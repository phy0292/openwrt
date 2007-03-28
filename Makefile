# Makefile for OpenWrt
#
# Copyright (C) 2006 OpenWrt.org
# Copyright (C) 2006 by Felix Fietkau <openwrt@nbd.name>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

RELEASE:=Kamikaze
#VERSION:=2.0 # uncomment for final release

#--------------------------------------------------------------
# Just run 'make menuconfig', configure stuff, then run 'make'.
# You shouldn't need to mess with anything beyond this point...
#--------------------------------------------------------------

all: world

SHELL:=/usr/bin/env bash
export LC_ALL=C
export LANG=C
export TOPDIR=${CURDIR}
ifeq ($(KBUILD_VERBOSE),99)
  MAKE:=3>/dev/null $(MAKE)
endif
export IS_TTY=$(shell tty -s && echo 1 || echo 0)

include $(TOPDIR)/include/verbose.mk

OPENWRTVERSION:=$(RELEASE)
ifneq ($(VERSION),)
  OPENWRTVERSION:=$(VERSION) ($(OPENWRTVERSION))
else
  REV:=$(shell LANG=C svn info | awk '/^Revision:/ { print$$2 }' )
  ifneq ($(REV),)
    OPENWRTVERSION:=$(OPENWRTVERSION)/r$(REV)
  endif
endif
export OPENWRTVERSION

ifeq ($(FORCE),)
  .config scripts/config/conf scripts/config/mconf: tmp/.prereq-build
  world: tmp/.prereq-packages tmp/.prereq-target
endif

tmp/.pkginfo: FORCE
	@mkdir -p tmp/info
	@$(NO_TRACE_MAKE) -s -f include/scan.mk SCAN_TARGET="pkginfo" SCAN_DIR="package" SCAN_NAME="package" SCAN_DEPS="$(shell ls include/package*.mk) include/kernel.mk" SCAN_EXTRA=""

tmp/.targetinfo: FORCE
	@mkdir -p tmp/info
	@$(NO_TRACE_MAKE) -s -f include/scan.mk SCAN_TARGET="targetinfo" SCAN_DIR="target/linux" SCAN_NAME="target" SCAN_DEPS="$(shell ls include/kernel*.mk)" SCAN_EXTRA=""

tmpinfo-clean: FORCE
	@-rm -rf tmp/.pkginfo tmp/.targetinfo

tmp/.config.in: tmp/.pkginfo
	@./scripts/metadata.pl package_config < $< > $@ || rm -f $@

tmp/.config-target.in: tmp/.targetinfo
	@./scripts/metadata.pl target_config < $< > $@ || rm -f $@

.config: ./scripts/config/conf tmp/.config.in tmp/.config-target.in
	@[ -f .config ] || $(NO_TRACE_MAKE) menuconfig
	@$< -D .config Config.in &> /dev/null

scripts/config/mconf:
	@$(MAKE) -C scripts/config all

scripts/config/conf:
	@$(MAKE) -C scripts/config conf

config: scripts/config/conf tmp/.config.in tmp/.config-target.in FORCE
	$< Config.in

config-clean: FORCE
	$(NO_TRACE_MAKE) -C scripts/config clean

defconfig: scripts/config/conf tmp/.config.in tmp/.config-target.in FORCE
	touch .config
	$< -D .config Config.in

oldconfig: scripts/config/conf tmp/.config.in tmp/.config-target.in FORCE
	$< -o Config.in

menuconfig: scripts/config/mconf tmp/.config.in tmp/.config-target.in FORCE
	$< Config.in

kernel_menuconfig: .config FORCE
	-$(MAKE) target/linux-prepare
	$(NO_TRACE_MAKE) -C target/linux menuconfig

package/%: 
	@$(NO_TRACE_MAKE) -s tmp/.pkginfo tmp/.targetinfo
	$(MAKE) -C package $(patsubst package/%,%,$@)

target/%:
	@$(NO_TRACE_MAKE) -s tmp/.pkginfo tmp/.targetinfo
	$(MAKE) -C target $(patsubst target/%,%,$@)

tools/%: FORCE
	$(MAKE) -C tools $(patsubst tools/%,%,$@)

toolchain/%: tmp/.targetinfo FORCE
	$(MAKE) -C toolchain $(patsubst toolchain/%,%,$@)

tmp/.prereq-build: include/prereq-build.mk
	@mkdir -p tmp
	@rm -f tmp/.host.mk
	@$(NO_TRACE_MAKE) -s -f $(TOPDIR)/include/prereq-build.mk prereq 2>/dev/null || { \
		echo "Prerequisite check failed. Use FORCE=1 to override."; \
		false; \
	}
	@touch $@

tmp/.prereq-packages: include/prereq.mk tmp/.pkginfo .config
	@mkdir -p tmp
	@rm -f tmp/.host.mk
	@$(NO_TRACE_MAKE) -s -C package prereq 2>/dev/null || { \
		echo "Prerequisite check failed. Use FORCE=1 to override."; \
		false; \
	}
	@touch $@

tmp/.prereq-target: include/prereq.mk tmp/.targetinfo .config
	@mkdir -p tmp
	@rm -f tmp/.host.mk
	@$(NO_TRACE_MAKE) -s -C target prereq 2>/dev/null || { \
		echo "Prerequisite check failed. Use FORCE=1 to override."; \
		false; \
	}
	@touch $@

prereq: tmp/.prereq-build tmp/.prereq-packages tmp/.prereq-target FORCE

download: .config FORCE
	$(MAKE) tools/download
	$(MAKE) toolchain/download
	$(MAKE) package/download
	$(MAKE) target/download

world: .config FORCE
	$(MAKE) tools/install
	$(MAKE) toolchain/install
	$(MAKE) target/compile
	$(MAKE) package/compile
	$(MAKE) package/install
	$(MAKE) target/install
	$(MAKE) package/index

clean: FORCE
	rm -rf build_* bin tmp

dirclean: clean
	rm -rf staging_dir_* toolchain_build_* tool_build

distclean: dirclean config-clean symlinkclean docclean
	rm -rf dl

help:
	cat README

doc:
	$(MAKE) -C docs/ openwrt.pdf

docclean:
	$(MAKE) -C docs/ clean

symlinkclean:
	find package -type l | xargs rm -f

.SILENT: clean dirclean distclean symlinkclean config-clean download world
FORCE: ;
.PHONY: FORCE help
%: ;
