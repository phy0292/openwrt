/*
 *  ADM5120 specific setup
 *
 *  Copyright (C) 2007-2008 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This code was based on the ADM5120 specific port of the Linux 2.6.10 kernel
 *  done by Jeroen Vreeken
 *	Copyright (C) 2005 Jeroen Vreeken (pe1rxq@amsat.org)
 *
 *  Jeroen's code was based on the Linux 2.4.xx source codes found in various
 *  tarballs released by Edimax for it's ADM5120 based devices
 *	Copyright (C) ADMtek Incorporated
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/io.h>

#include <asm/reboot.h>
#include <asm/time.h>

#include <asm/mach-adm5120/adm5120_info.h>
#include <asm/mach-adm5120/adm5120_defs.h>
#include <asm/mach-adm5120/adm5120_switch.h>
#include <asm/mach-adm5120/adm5120_board.h>

void (*adm5120_board_reset)(void);

static char *prom_names[ADM5120_PROM_LAST+1] __initdata = {
	[ADM5120_PROM_GENERIC]		= "Generic",
	[ADM5120_PROM_CFE]		= "CFE",
	[ADM5120_PROM_UBOOT]		= "U-Boot",
	[ADM5120_PROM_MYLOADER]		= "MyLoader",
	[ADM5120_PROM_ROUTERBOOT]	= "RouterBOOT",
	[ADM5120_PROM_BOOTBASE]		= "Bootbase"
};

static void __init adm5120_report(void)
{
	printk(KERN_INFO "SoC      : ADM%04X%s revision %d, running "
		"at %ldMHz\n",
		adm5120_product_code,
		adm5120_package_bga() ? "" : "P",
		adm5120_revision, (adm5120_speed / 1000000));
	printk(KERN_INFO "Bootdev  : %s flash\n",
		adm5120_nand_boot ? "NAND":"NOR");
	printk(KERN_INFO "Prom     : %s\n", prom_names[adm5120_prom_type]);
}

static void adm5120_restart(char *command)
{
	/* TODO: stop switch before reset */

	if (adm5120_board_reset)
		adm5120_board_reset();

	SW_WRITE_REG(SWITCH_REG_SOFT_RESET, 1);
}

static void adm5120_halt(void)
{
	local_irq_disable();

	while (1) {
		if (cpu_wait)
			cpu_wait();
	}
}

void __init plat_time_init(void)
{
	mips_hpt_frequency = adm5120_speed / 2;
}

void __init plat_mem_setup(void)
{
	adm5120_soc_init();
	adm5120_mem_init();
	adm5120_report();

	_machine_restart = adm5120_restart;
	_machine_halt = adm5120_halt;
	pm_power_off = adm5120_halt;

	set_io_port_base(KSEG1);
}
