/*
 *  NETGEAR WNR2000 board support
 *
 *  Copyright (C) 2008-2009 Gabor Juhos <juhosg@openwrt.org>
 *  Copyright (C) 2008 Imre Kaloz <kaloz@openwrt.org>
 *  Copyright (C) 2008-2009 Andy Boyett <agb@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>

#include <asm/mach-ar71xx/ar71xx.h>

#include "machtype.h"
#include "devices.h"
#include "dev-m25p80.h"
#include "dev-ar9xxx-wmac.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"

#define WNR2000_GPIO_LED_PWR_GREEN	14
#define WNR2000_GPIO_LED_PWR_AMBER	7
#define WNR2000_GPIO_LED_WPS		4
#define WNR2000_GPIO_LED_WLAN		6
#define WNR2000_GPIO_BTN_RESET		21
#define WNR2000_GPIO_BTN_WPS		8

#define WNR2000_KEYS_POLL_INTERVAL	20	/* msecs */
#define WNR2000_KEYS_DEBOUNCE_INTERVAL	(3 * WNR2000_KEYS_POLL_INTERVAL)

#ifdef CONFIG_MTD_PARTITIONS
static struct mtd_partition wnr2000_partitions[] = {
	{
		.name		= "u-boot",
		.offset		= 0,
		.size		= 0x040000,
		.mask_flags	= MTD_WRITEABLE,
	}, {
		.name		= "u-boot-env",
		.offset		= 0x040000,
		.size		= 0x010000,
	}, {
		.name		= "rootfs",
		.offset		= 0x050000,
		.size		= 0x240000,
	}, {
		.name		= "user-config",
		.offset		= 0x290000,
		.size		= 0x010000,
	}, {
		.name		= "uImage",
		.offset		= 0x2a0000,
		.size		= 0x120000,
	}, {
		.name		= "language_table",
		.offset		= 0x3c0000,
		.size		= 0x020000,
	}, {
		.name		= "rootfs_checksum",
		.offset		= 0x3e0000,
		.size		= 0x010000,
	}, {
		.name		= "art",
		.offset		= 0x3f0000,
		.size		= 0x010000,
		.mask_flags	= MTD_WRITEABLE,
	}
};
#endif /* CONFIG_MTD_PARTITIONS */

static struct flash_platform_data wnr2000_flash_data = {
#ifdef CONFIG_MTD_PARTITIONS
	.parts		= wnr2000_partitions,
	.nr_parts	= ARRAY_SIZE(wnr2000_partitions),
#endif
};

static struct gpio_led wnr2000_leds_gpio[] __initdata = {
	{
		.name		= "wnr2000:green:power",
		.gpio		= WNR2000_GPIO_LED_PWR_GREEN,
		.active_low	= 1,
	}, {
		.name		= "wnr2000:amber:power",
		.gpio		= WNR2000_GPIO_LED_PWR_AMBER,
		.active_low	= 1,
	}, {
		.name		= "wnr2000:green:wps",
		.gpio		= WNR2000_GPIO_LED_WPS,
		.active_low	= 1,
	}, {
		.name		= "wnr2000:blue:wlan",
		.gpio		= WNR2000_GPIO_LED_WLAN,
		.active_low	= 1,
	}
};

static struct gpio_keys_button wnr2000_gpio_keys[] __initdata = {
	{
		.desc		= "reset",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = WNR2000_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= WNR2000_GPIO_BTN_RESET,
	}, {
		.desc		= "wps",
		.type		= EV_KEY,
		.code		= KEY_WPS_BUTTON,
		.debounce_interval = WNR2000_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= WNR2000_GPIO_BTN_WPS,
	}
};

static void __init wnr2000_setup(void)
{
	u8 *eeprom = (u8 *) KSEG1ADDR(0x1fff1000);

	ar71xx_add_device_mdio(0, 0x0);

	ar71xx_init_mac(ar71xx_eth0_data.mac_addr, eeprom, 0);
	ar71xx_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RMII;
	ar71xx_eth0_data.speed = SPEED_100;
	ar71xx_eth0_data.duplex = DUPLEX_FULL;
	ar71xx_eth0_data.has_ar8216 = 1;

	ar71xx_init_mac(ar71xx_eth1_data.mac_addr, eeprom, 1);
	ar71xx_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_RMII;
	ar71xx_eth1_data.phy_mask = 0x10;

	ar71xx_add_device_eth(0);
	ar71xx_add_device_eth(1);

	ar71xx_add_device_m25p80(&wnr2000_flash_data);

	ar71xx_add_device_leds_gpio(-1, ARRAY_SIZE(wnr2000_leds_gpio),
					wnr2000_leds_gpio);

	ar71xx_register_gpio_keys_polled(-1, WNR2000_KEYS_POLL_INTERVAL,
					 ARRAY_SIZE(wnr2000_gpio_keys),
					 wnr2000_gpio_keys);


	ar9xxx_add_device_wmac(eeprom, NULL);
}

MIPS_MACHINE(AR71XX_MACH_WNR2000, "WNR2000", "NETGEAR WNR2000", wnr2000_setup);
