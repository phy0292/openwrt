#!/bin/sh

[ -n "$INCLUDE_ONLY" ] || {
	. /lib/functions.sh
	. /lib/functions/network.sh
	. ../netifd-proto.sh
	init_proto "$@"
}

gre_generic_setup() {
	local cfg="$1"
	local mode="$2"
	local local="$3"
	local remote="$4"
	local link="$5"
	local mtu ttl zone ikey okey icsum ocsum iseqno oseqno
	json_get_vars mtu ttl zone ikey okey icsum ocsum iseqno oseqno

	[ -z "$zone" ] && zone="wan"

	proto_init_update "$link" 1

	proto_add_tunnel
	json_add_string mode "$mode"
	json_add_int mtu "${mtu:-1280}"
	[ -n "$df" ] && json_add_boolean df "$df"
	json_add_int ttl "${ttl:-64}"
	json_add_string local "$local"
	json_add_string remote "$remote"
	[ -n "$tunlink" ] && json_add_string link "$tunlink"
	json_add_string info "${ikey:-0},${okey:-0},${icsum:-0},${ocsum:-0},${iseqno:-0},${oseqno:-0}"
	proto_close_tunnel

	proto_add_data
	[ -n "$zone" ] && json_add_string zone "$zone"
	proto_close_data

	proto_send_update "$cfg"
}

gre_setup() {
	local cfg="$1"
	local mode="$2"

	local ipaddr peeraddr
	json_get_vars df ipaddr peeraddr tunlink

	[ -z "$peeraddr" ] && {
		proto_notify_error "$cfg" "MISSING_ADDRESS"
		proto_block_restart "$cfg"
		exit
	}

	( proto_add_host_dependency "$cfg" "0.0.0.0" "$tunlink" )

	[ -z "$ipaddr" ] && {
		local wanif="$tunlink"
		if [ -z $wanif ] && ! network_find_wan wanif; then
			proto_notify_error "$cfg" "NO_WAN_LINK"
			exit
		fi

		if ! network_get_ipaddr ipaddr "$wanif"; then
			proto_notify_error "$cfg" "NO_WAN_LINK"
			exit
		fi
	}

	[ -z "$df" ] && df="1"

	gre_generic_setup $cfg $mode $ipaddr $peeraddr "gre-$cfg"
}

proto_gre_setup() {
	local cfg="$1"

	gre_setup $cfg "greip"
}

proto_gretap_setup() {
	local cfg="$1"

	local network
	json_get_vars network

	gre_setup $cfg "gretapip"

	json_init
	json_add_string name "gre-$cfg"
	json_add_boolean link-ext 0
	json_close_object

	for i in $network; do
		ubus call network.interface."$i" add_device "$(json_dump)"
	done
}

grev6_setup() {
	local cfg="$1"
	local mode="$2"

	local ip6addr peer6addr weakif
	json_get_vars ip6addr peer6addr tunlink weakif

	[ -z "$peer6addr" ] && {
		proto_notify_error "$cfg" "MISSING_ADDRESS"
		proto_block_restart "$cfg"
		exit
	}

	( proto_add_host_dependency "$cfg" "::" "$tunlink" )

	[ -z "$ip6addr" ] && {
		local wanif="$tunlink"
		if [ -z $wanif ] && ! network_find_wan6 wanif; then
			proto_notify_error "$cfg" "NO_WAN_LINK"
			exit
		fi

		if ! network_get_ipaddr6 ip6addr "$wanif"; then
			[ -z "$weakif" ] && weakif="lan"
			if ! network_get_ipaddr6 ip6addr "$weakif"; then
				proto_notify_error "$cfg" "NO_WAN_LINK"
				exit
			fi
		fi
	}

	gre_generic_setup $cfg $mode $ip6addr $peer6addr "grev6-$cfg"
}

proto_grev6_setup() {
	local cfg="$1"

	grev6_setup $cfg "greip6"
}

proto_grev6tap_setup() {
	local cfg="$1"

	local network
	json_get_vars network

	grev6_setup $cfg "gretapip6"

	json_init
	json_add_string name "grev6-$cfg"
	json_add_boolean link-ext 0
	json_close_object

	for i in $network; do
		ubus call network.interface."$i" add_device "$(json_dump)"
	done
}

gretap_generic_teardown() {
	local network
	json_get_vars network

	json_init
	json_add_string name "$1"
	json_add_boolean link-ext 0
	json_close_object

	for i in $network; do
		ubus call network.interface."$i" remove_device "$(json_dump)"
	done
}

proto_gre_teardown() {
	local cfg="$1"
}

proto_gretap_teardown() {
	local cfg="$1"

	gretap_generic_teardown "gre-$cfg"
}

proto_grev6_teardown() {
	local cfg="$1"
}

proto_grev6tap_teardown() {
	local cfg="$1"

	gretap_generic_teardown "grev6-$cfg"
}

gre_generic_init_config() {
	no_device=1
	available=1

	proto_config_add_int "mtu"
	proto_config_add_int "ttl"
	proto_config_add_string "tunlink"
	proto_config_add_string "zone"
	proto_config_add_int "ikey"
	proto_config_add_int "okey"
	proto_config_add_boolean "icsum"
	proto_config_add_boolean "ocsum"
	proto_config_add_boolean "iseqno"
	proto_config_add_boolean "oseqno"
}

proto_gre_init_config() {
	gre_generic_init_config
	proto_config_add_string "ipaddr"
	proto_config_add_string "peeraddr"
	proto_config_add_boolean "df"
}

proto_gretap_init_config() {
	proto_gre_init_config
	proto_config_add_string "network"
}

proto_grev6_init_config() {
	gre_generic_init_config
	proto_config_add_string "ip6addr"
	proto_config_add_string "peer6addr"
	proto_config_add_string "weakif"
}

proto_grev6tap_init_config() {
	proto_grev6_init_config
	proto_config_add_string "network"
}

[ -n "$INCLUDE_ONLY" ] || {
	[ -f /lib/modules/$(uname -r)/gre.ko ] && add_protocol gre
	[ -f /lib/modules/$(uname -r)/gre.ko ] && add_protocol gretap
	[ -f /lib/modules/$(uname -r)/ip6_gre.ko ] && add_protocol grev6
	[ -f /lib/modules/$(uname -r)/ip6_gre.ko ] && add_protocol grev6tap
}
