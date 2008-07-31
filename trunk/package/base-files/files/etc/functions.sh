#!/bin/sh
# Copyright (C) 2006 OpenWrt.org
# Copyright (C) 2006 Fokus Fraunhofer <carsten.tittel@fokus.fraunhofer.de>

alias debug=${DEBUG:-:}

# newline
N="
"

_C=0
NO_EXPORT=1
LOAD_STATE=1

hotplug_dev() {
	env -i ACTION=$1 INTERFACE=$2 /sbin/hotplug-call net
}

append() {
	local var="$1"
	local value="$2"
	local sep="${3:- }"
	
	eval "export ${NO_EXPORT:+-n} -- \"$var=\${$var:+\${$var}\${value:+\$sep}}\$value\""
}

list_contains() {
	local var="$1"
	local str="$2"
	local val

	eval "val=\" \${$var} \""
	[ "${val%% $str *}" != "$val" ]
}

list_remove() {
	local var="$1"
	local remove="$2"
	local val

	eval "val=\" \${$var} \""
	val1="${val%% $remove *}"
	[ "$val1" = "$val" ] && return
	val2="${val##* $remove }"
	[ "$val2" = "$val" ] && return
	val="${val1## } ${val2%% }"
	val="${val%% }"
	eval "export ${NO_EXPORT:+-n} -- \"$var=\$val\""
}

config_load() {
	[ -n "$IPKG_INSTROOT" ] && return 0
	uci_load "$@"
}

reset_cb() {
	config_cb() { return 0; }
	option_cb() { return 0; }
}
reset_cb

package() {
	return 0
}

config () {
	local cfgtype="$1"
	local name="$2"
	
	export ${NO_EXPORT:+-n} CONFIG_NUM_SECTIONS=$(($CONFIG_NUM_SECTIONS + 1))
	name="${name:-cfg$CONFIG_NUM_SECTIONS}"
	append CONFIG_SECTIONS "$name"
	[ -n "$NO_CALLBACK" ] || config_cb "$cfgtype" "$name"
	export ${NO_EXPORT:+-n} CONFIG_SECTION="$name"
	export ${NO_EXPORT:+-n} "CONFIG_${CONFIG_SECTION}_TYPE=$cfgtype"
}

option () {
	local varname="$1"; shift
	local value="$*"
	
	export ${NO_EXPORT:+-n} "CONFIG_${CONFIG_SECTION}_${varname}=$value"
	[ -n "$NO_CALLBACK" ] || option_cb "$varname" "$*"
}

config_rename() {
	local OLD="$1"
	local NEW="$2"
	local oldvar
	local newvar
	
	[ -n "$OLD" -a -n "$NEW" ] || return
	for oldvar in `set | grep ^CONFIG_${OLD}_ | \
		sed -e 's/\(.*\)=.*$/\1/'` ; do
		newvar="CONFIG_${NEW}_${oldvar##CONFIG_${OLD}_}"
		eval "export ${NO_EXPORT:+-n} \"$newvar=\${$oldvar}\""
		unset "$oldvar"
	done
	export ${NO_EXPORT:+-n} CONFIG_SECTIONS="$(echo " $CONFIG_SECTIONS " | sed -e "s, $OLD , $NEW ,")"
	
	[ "$CONFIG_SECTION" = "$OLD" ] && export ${NO_EXPORT:+-n} CONFIG_SECTION="$NEW"
}

config_unset() {
	config_set "$1" "$2" ""
}

config_clear() {
	local SECTION="$1"
	local oldvar

	list_remove CONFIG_SECTIONS "$SECTION"
	export ${NO_EXPORT:+-n} CONFIG_SECTIONS="${SECTION:+$CONFIG_SECTIONS}"

	for oldvar in `set | grep ^CONFIG_${SECTION:+${SECTION}_} | \
		sed -e 's/\(.*\)=.*$/\1/'` ; do 
		unset $oldvar 
	done
}

config_get() {
	case "$3" in
		"") eval "echo \"\${CONFIG_${1}_${2}}\"";;
		*)  eval "export ${NO_EXPORT:+-n} -- \"$1=\${CONFIG_${2}_${3}}\"";;
	esac
}

# config_get_bool <variable> <section> <option> [<default>]
config_get_bool() {
	local _tmp
	config_get "_tmp" "$2" "$3"
	case "$_tmp" in
		1|on|true|enabled) export ${NO_EXPORT:+-n} "$1=1";;
		0|off|false|disabled) export ${NO_EXPORT:+-n} "$1=0";;
		*) eval "$1=$4";;
	esac
}

config_set() {
	local section="$1"
	local option="$2"
	local value="$3"
	local old_section="$CONFIG_SECTION"

	CONFIG_SECTION="$section"
	option "$option" "$value"
	CONFIG_SECTION="$old_section"
}

config_foreach() {
	local function="$1"
	[ "$#" -ge 1 ] && shift
	local type="$1"
	[ "$#" -ge 1 ] && shift
	local section cfgtype
	
	[ -z "$CONFIG_SECTIONS" ] && return 0
	for section in ${CONFIG_SECTIONS}; do
		config_get cfgtype "$section" TYPE
		[ -n "$type" -a "x$cfgtype" != "x$type" ] && continue
		eval "$function \"\$section\" \"\$@\""
	done
}

load_modules() {
	[ -d /etc/modules.d ] && {
		cd /etc/modules.d
		sed 's/^[^#]/insmod &/' $* | ash 2>&- || :
	}
}

include() {
	local file
	
	for file in $(ls $1/*.sh 2>/dev/null); do
		. $file
	done
}

find_mtd_part() {
	local PART="$(grep "\"$1\"" /proc/mtd | awk -F: '{print $1}')"
	local PREFIX=/dev/mtdblock
	
	PART="${PART##mtd}"
	[ -d /dev/mtdblock ] && PREFIX=/dev/mtdblock/
	echo "${PART:+$PREFIX$PART}"
}

strtok() { # <string> { <variable> [<separator>] ... }
	local tmp
	local val="$1"
	local count=0

	shift

	while [ $# -gt 1 ]; do
		tmp="${val%%$2*}"

		[ "$tmp" = "$val" ] && break

		val="${val#$tmp$2}"

		export ${NO_EXPORT:+-n} "$1=$tmp"; count=$((count+1))
		shift 2
	done

	if [ $# -gt 0 -a -n "$val" ]; then
		export ${NO_EXPORT:+-n} "$1=$val"; count=$((count+1))
	fi

	return $count
}


jffs2_mark_erase() {
	local part="$(find_mtd_part "$1")"
	[ -z "$part" ] && {
		echo Partition not found.
		return 1
	}
	echo -e "\xde\xad\xc0\xde" | mtd -qq write - "$1"
}

uci_apply_defaults() {(
	cd /etc/uci-defaults || return 0
	files="$(ls)"
	[ -z "$files" ] && return 0
	mkdir -p /tmp/.uci
	for file in $files; do
		( . "./$(basename $file)" ) && rm -f "$file"
	done
	uci commit
)}

[ -z "$IPKG_INSTROOT" -a -f /lib/config/uci.sh ] && . /lib/config/uci.sh
