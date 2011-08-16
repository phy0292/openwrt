#
# Copyright (C) 2008-2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

W1_MENU:=W1 support
W1_MASTERS_DIR:=$(LINUX_DIR)/drivers/w1/masters
W1_SLAVES_DIR:=$(LINUX_DIR)/drivers/w1/slaves

define KernelPackage/w1
  SUBMENU:=$(W1_MENU)
  TITLE:=Dallas's 1-wire support
  KCONFIG:=CONFIG_W1
  FILES:=$(LINUX_DIR)/drivers/w1/wire.ko
  AUTOLOAD:=$(call AutoLoad,50,wire)
endef

define KernelPackage/w1/description
  Kernel module for Dallas's 1-wire support
endef

$(eval $(call KernelPackage,w1))


define AddDepends/w1
  SUBMENU:=$(W1_MENU)
  DEPENDS+=kmod-w1 $(1)
endef


#
# 1-wire masters
#
define KernelPackage/w1-master-gpio
  TITLE:=GPIO 1-wire bus master driver
  DEPENDS:=@GPIO_SUPPORT
  KCONFIG:=CONFIG_W1_MASTER_GPIO
  FILES:=$(W1_MASTERS_DIR)/w1-gpio.ko
  AUTOLOAD:=$(call AutoLoad,60,w1-gpio)
  $(call AddDepends/w1)
endef

define KernelPackage/w1-master-gpio/description
  Kernel module for the GPIO 1-wire bus master driver
endef

$(eval $(call KernelPackage,w1-master-gpio))

define KernelPackage/w1-master-ds2482
  TITLE:=DS2482 1-wire i2c bus master driver
  KCONFIG:=CONFIG_W1_MASTER_DS2482
  FILES:=$(W1_MASTERS_DIR)/ds2482.ko
  AUTOLOAD:=$(call AutoLoad,60,ds2482)
  $(call AddDepends/w1,+kmod-i2c-core)
endef

define KernelPackage/w1-master-ds2482/description
  Kernel module for the DS2482 i2c 1-wire bus master driver
  NOTE: Init with: echo ds2482 0x18 > /sys/bus/i2c/devices/i2c-0/new_device
  or use owfs
endef

$(eval $(call KernelPackage,w1-master-ds2482))


define KernelPackage/w1-master-ds2490
  TITLE:=DS2490 1-wire usb bus master driver
  DEPENDS:=@USB_SUPPORT +kmod-usb-core
  KCONFIG:=CONFIG_W1_MASTER_DS2490
  FILES:=$(W1_MASTERS_DIR)/ds2490.ko
  AUTOLOAD:=$(call AutoLoad,60,ds2490)
  $(call AddDepends/w1)
endef
 
define KernelPackage/w1-master-ds2490/description
  Kernel module for the DS2490 usb 1-wire bus master driver
endef

$(eval $(call KernelPackage,w1-master-ds2490))

#
# 1-wire slaves
#
define KernelPackage/w1-slave-therm
  TITLE:=Thermal family implementation
  KCONFIG:=CONFIG_W1_SLAVE_THERM
  FILES:=$(W1_SLAVES_DIR)/w1_therm.ko
  AUTOLOAD:=$(call AutoLoad,70,w1_therm)
  $(call AddDepends/w1)
endef

define KernelPackage/w1-slave-therm/description
  Kernel module for 1-wire thermal sensors
endef

$(eval $(call KernelPackage,w1-slave-therm))


define KernelPackage/w1-slave-smem
  TITLE:=Simple 64bit memory family implementation
  KCONFIG:=CONFIG_W1_SLAVE_SMEM
  FILES:=$(W1_SLAVES_DIR)/w1_smem.ko
  AUTOLOAD:=$(call AutoLoad,70,w1_smem)
  $(call AddDepends/w1)
endef

define KernelPackage/w1-slave-smem/description
  Kernel module for 1-wire simple 64bit memory rom(ds2401/ds2411/ds1990*)
endef

$(eval $(call KernelPackage,w1-slave-smem))


define KernelPackage/w1-slave-ds2433
  TITLE:=Simple 64bit memory family implementation
  KCONFIG:= \
	CONFIG_W1_SLAVE_DS2433 \
	CONFIG_W1_SLAVE_DS2433_CRC=n
  FILES:=$(W1_SLAVES_DIR)/w1_ds2433.ko
  AUTOLOAD:=$(call AutoLoad,70,w1_ds2433)
  $(call AddDepends/w1)
endef

define KernelPackage/w1-slave-ds2433/description
  Kernel module for 1-wire simple 64bit memory rom(ds2401/ds2411/ds1990*)
endef

$(eval $(call KernelPackage,w1-slave-ds2433))


define KernelPackage/w1-slave-ds2760
  TITLE:=Dallas 2760 battery monitor chip (HP iPAQ & others)
  KCONFIG:= \
	CONFIG_W1_SLAVE_DS2760 \
	CONFIG_W1_SLAVE_DS2433_CRC=n
  FILES:=$(W1_SLAVES_DIR)/w1_ds2760.ko
  AUTOLOAD:=$(call AutoLoad,70,w1_ds2760)
  $(call AddDepends/w1)
endef

define KernelPackage/w1-slave-ds2760/description
  Kernel module for 1-wire DS2760 battery monitor chip support
endef

$(eval $(call KernelPackage,w1-slave-ds2760))
