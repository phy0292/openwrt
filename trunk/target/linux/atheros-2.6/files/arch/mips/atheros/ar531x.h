#ifndef __AR531X_H
#define __AR531X_H

#include <ar531x_platform.h>
#include "ar5312.h"
#include "ar5315.h"

#define MIPS_CPU_IRQ_BASE		0x00
#define AR531X_HIGH_PRIO                0x10
#define AR531X_MISC_IRQ_BASE		0x20
#define AR531X_GPIO_IRQ_BASE            0x30

/* Software's idea of interrupts handled by "CPU Interrupt Controller" */
#define AR531X_IRQ_NONE		MIPS_CPU_IRQ_BASE+0
#define AR531X_IRQ_CPU_CLOCK	MIPS_CPU_IRQ_BASE+7 /* C0_CAUSE: 0x8000 */

/* Miscellaneous interrupts, which share IP6 */
#define AR531X_MISC_IRQ_NONE		AR531X_MISC_IRQ_BASE+0
#define AR531X_MISC_IRQ_TIMER		AR531X_MISC_IRQ_BASE+1
#define AR531X_MISC_IRQ_AHB_PROC	AR531X_MISC_IRQ_BASE+2
#define AR531X_MISC_IRQ_AHB_DMA		AR531X_MISC_IRQ_BASE+3
#define AR531X_MISC_IRQ_GPIO		AR531X_MISC_IRQ_BASE+4
#define AR531X_MISC_IRQ_UART0		AR531X_MISC_IRQ_BASE+5
#define AR531X_MISC_IRQ_UART0_DMA	AR531X_MISC_IRQ_BASE+6
#define AR531X_MISC_IRQ_WATCHDOG	AR531X_MISC_IRQ_BASE+7
#define AR531X_MISC_IRQ_LOCAL		AR531X_MISC_IRQ_BASE+8
#define AR531X_MISC_IRQ_COUNT		9

/* GPIO Interrupts [0..7], share AR531X_MISC_IRQ_GPIO */
#define AR531X_GPIO_IRQ_NONE            AR531X_MISC_IRQ_BASE+0
#define AR531X_GPIO_IRQ(n)              AR531X_MISC_IRQ_BASE+(n)+1
#define AR531X_GPIO_IRQ_COUNT           22

#define sysRegRead(phys)	\
	(*(volatile u32 *)KSEG1ADDR(phys))

#define sysRegWrite(phys, val)	\
	((*(volatile u32 *)KSEG1ADDR(phys)) = (val))

/*
 * This is board-specific data that is stored in a "fixed" location in flash.
 * It is shared across operating systems, so it should not be changed lightly.
 * The main reason we need it is in order to extract the ethernet MAC
 * address(es).
 */
struct ar531x_boarddata {
    u32 magic;                       /* board data is valid */
#define AR531X_BD_MAGIC 0x35333131   /* "5311", for all 531x platforms */
    u16 cksum;                       /* checksum (starting with BD_REV 2) */
    u16 rev;                         /* revision of this struct */
#define BD_REV  4
    char   boardName[64];            /* Name of board */
    u16 major;                       /* Board major number */
    u16 minor;                       /* Board minor number */
    u32 config;                      /* Board configuration */
#define BD_ENET0        0x00000001   /* ENET0 is stuffed */
#define BD_ENET1        0x00000002   /* ENET1 is stuffed */
#define BD_UART1        0x00000004   /* UART1 is stuffed */
#define BD_UART0        0x00000008   /* UART0 is stuffed (dma) */
#define BD_RSTFACTORY   0x00000010   /* Reset factory defaults stuffed */
#define BD_SYSLED       0x00000020   /* System LED stuffed */
#define BD_EXTUARTCLK   0x00000040   /* External UART clock */
#define BD_CPUFREQ      0x00000080   /* cpu freq is valid in nvram */
#define BD_SYSFREQ      0x00000100   /* sys freq is set in nvram */
#define BD_WLAN0        0x00000200   /* Enable WLAN0 */
#define BD_MEMCAP       0x00000400   /* CAP SDRAM @ memCap for testing */
#define BD_DISWATCHDOG  0x00000800   /* disable system watchdog */
#define BD_WLAN1        0x00001000   /* Enable WLAN1 (ar5212) */
#define BD_ISCASPER     0x00002000   /* FLAG for AR2312 */
#define BD_WLAN0_2G_EN  0x00004000   /* FLAG for radio0_2G */
#define BD_WLAN0_5G_EN  0x00008000   /* FLAG for radio0_2G */
#define BD_WLAN1_2G_EN  0x00020000   /* FLAG for radio0_2G */
#define BD_WLAN1_5G_EN  0x00040000   /* FLAG for radio0_2G */
    u16 resetConfigGpio;             /* Reset factory GPIO pin */
    u16 sysLedGpio;                  /* System LED GPIO pin */

    u32 cpuFreq;                     /* CPU core frequency in Hz */
    u32 sysFreq;                     /* System frequency in Hz */
    u32 cntFreq;                     /* Calculated C0_COUNT frequency */

    u8  wlan0Mac[6];
    u8  enet0Mac[6];
    u8  enet1Mac[6];

    u16 pciId;                       /* Pseudo PCIID for common code */
    u16 memCap;                      /* cap bank1 in MB */

    /* version 3 */
    u8  wlan1Mac[6];                 /* (ar5212) */
};


extern char *board_config;
extern char *radio_config;
extern void serial_setup(unsigned long mapbase, unsigned int uartclk);
extern int ar531x_find_config(char *flash_limit);

extern void ar5312_misc_intr_init(int irq_base);
extern void ar5312_irq_dispatch(void);
extern void ar5312_plat_setup(void);

extern void ar5315_misc_intr_init(int irq_base);
extern asmlinkage void ar5315_irq_dispatch(void);
extern void ar5315_plat_setup(void);

#endif
