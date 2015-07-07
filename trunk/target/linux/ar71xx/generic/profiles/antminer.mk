#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/ANTMINER_S1
	NAME:=Antminer S1
	PACKAGES:=kmod-usb-core kmod-usb2 kmod-crypto-manager kmod-i2c-gpio-custom kmod-usb-hid
endef


define Profile/ANTMINER_S1/Description
	Package set optimized for the Bitmain Antminer S1.
endef
$(eval $(call Profile,ANTMINER_S1))

define Profile/ANTMINER_S3
	NAME:=Antminer S3
	PACKAGES:=kmod-usb-core kmod-usb2 kmod-crypto-manager kmod-i2c-gpio-custom kmod-usb-hid
endef


define Profile/ANTMINER_S1/Description
	Package set optimized for the Bitmain Antminer S3.
endef
$(eval $(call Profile,ANTMINER_S3))
