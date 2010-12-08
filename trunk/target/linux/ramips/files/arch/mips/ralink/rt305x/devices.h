/*
 * Ralink RT305x SoC specific platform device definitions
 *
 * Copyright (C) 2009-2010 Gabor Juhos <juhosg@openwrt.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 */

#ifndef __RT305X_DEVICES_H
#define __RT305X_DEVICES_H

#include <asm/mach-ralink/rt305x_esw_platform.h>

struct physmap_flash_data;

extern struct rt305x_esw_platform_data rt305x_esw_data;

extern void rt305x_register_flash(unsigned int id,
				  struct physmap_flash_data *pdata) __init;
extern void rt305x_register_ethernet(void) __init;
extern void rt305x_register_wifi(void) __init;

#endif  /* __RT305X_DEVICES_H */

