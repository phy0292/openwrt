/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 *
 *   Copyright (C) 2005 infineon
 *   Copyright (C) 2007 John Crispin <blogic@openwrt.org> 
 *
 */
#ifndef _IFXMIPS_H__
#define _IFXMIPS_H__

#define ifxmips_r32(reg) __raw_readl(reg)
#define ifxmips_w32(val,reg) __raw_writel(val,reg)
#define ifxmips_w32_mask(clear,set,reg)	ifxmips_w32((ifxmips_r32(reg) & ~clear) | set, reg)

/*------------ GENERAL */

#define BOARD_SYSTEM_TYPE		"IFXMIPS"

#define IOPORT_RESOURCE_START	0x10000000
#define IOPORT_RESOURCE_END		0xffffffff
#define IOMEM_RESOURCE_START	0x10000000
#define IOMEM_RESOURCE_END		0xffffffff

#define IFXMIPS_FLASH_START     0x10000000
#define IFXMIPS_FLASH_MAX       0x2000000


/*------------ ASC1 */

#define IFXMIPS_ASC_BASE_ADDR	(KSEG1 + 0x1E100400)
#define IFXMIPS_ASC_BASE_DIFF	(0x1E100C00 - 0x1E100400)

#define IFXMIPS_ASC_FSTAT		0x0048
#define IFXMIPS_ASC_TBUF		0x0020
#define IFXMIPS_ASC_WHBSTATE	0x0018
#define IFXMIPS_ASC_RBUF		0x0024
#define IFXMIPS_ASC_STATE		0x0014
#define IFXMIPS_ASC_IRNCR		0x00F8
#define IFXMIPS_ASC_CLC			0x0000
#define IFXMIPS_ASC_PISEL		0x0004
#define IFXMIPS_ASC_TXFCON		0x0044
#define IFXMIPS_ASC_RXFCON		0x0040
#define IFXMIPS_ASC_CON			0x0010
#define IFXMIPS_ASC_BG			0x0050
#define IFXMIPS_ASC_IRNREN		0x00F4

#define IFXMIPS_ASC_CLC_DISS	0x2
#define ASC_IRNREN_RX_BUF		0x8
#define ASC_IRNREN_TX_BUF		0x4
#define ASC_IRNREN_ERR			0x2
#define ASC_IRNREN_TX			0x1
#define ASC_IRNCR_TIR			0x4
#define ASC_IRNCR_RIR			0x2
#define ASC_IRNCR_EIR			0x4
#define ASCOPT_CSIZE			0x3
#define ASCOPT_CS7				0x1
#define ASCOPT_CS8				0x2
#define ASCOPT_PARENB			0x4
#define ASCOPT_STOPB			0x8
#define ASCOPT_PARODD			0x0
#define ASCOPT_CREAD			0x20
#define ASCFSTAT_TXFFLMASK		0x3F00
#define ASCFSTAT_TXFFLOFF		8



/*------------ RCU */
#define IFXMIPS_RCU_BASE_ADDR	0xBF203000

/* reset request */
#define IFXMIPS_RCU_RST			((u32*)(IFXMIPS_RCU_BASE_ADDR + 0x0010))
#define IFXMIPS_RCU_RST_CPU1	(1 << 3)
#define IFXMIPS_RCU_RST_ALL		0x40000000

#define IFXMIPS_RCU_RST_REQ_DFE	(1 << 7)
#define IFXMIPS_RCU_RST_REQ_AFE	(1 << 11)
#define IFXMIPS_RCU_RST_REQ_ARC_JTAG	(1 << 20)


/*------------ GPTU */

#define IFXMIPS_GPTU_BASE_ADDR	0xB8000300

/* clock control register */
#define IFXMIPS_GPTU_GPT_CLC		((u32*)(IFXMIPS_GPTU_BASE_ADDR + 0x0000))

/* captur reload register */
#define IFXMIPS_GPTU_GPT_CAPREL	((u32*)(IFXMIPS_GPTU_BASE_ADDR + 0x0030))

/* timer 6 control register */
#define IFXMIPS_GPTU_GPT_T6CON	((u32*)(IFXMIPS_GPTU_BASE_ADDR + 0x0020))


/*------------ EBU */

#define IFXMIPS_EBU_BASE_ADDR	0xBE105300

/* bus configuration register */
#define IFXMIPS_EBU_BUSCON0		((u32*)(IFXMIPS_EBU_BASE_ADDR + 0x0060))
#define IFXMIPS_EBU_PCC_CON		((u32*)(IFXMIPS_EBU_BASE_ADDR + 0x0090))
#define IFXMIPS_EBU_PCC_IEN		((u32*)(IFXMIPS_EBU_BASE_ADDR + 0x00A4))
#define IFXMIPS_EBU_PCC_ISTAT	((u32*)(IFXMIPS_EBU_BASE_ADDR + 0x00A0))


/*------------ CGU */
#define IFXMIPS_CGU_BASE_ADDR		(KSEG1 + 0x1F103000)
#define IFXMIPS_CGU_PLL0_CFG		((u32*)(IFXMIPS_CGU_BASE_ADDR + 0x0004))
#define IFXMIPS_CGU_PLL1_CFG		((u32*)(IFXMIPS_CGU_BASE_ADDR + 0x0008))
#define IFXMIPS_CGU_PLL2_CFG		((u32*)(IFXMIPS_CGU_BASE_ADDR + 0x000C))
#define IFXMIPS_CGU_SYS				((u32*)(IFXMIPS_CGU_BASE_ADDR + 0x0010))
#define IFXMIPS_CGU_UPDATE			((u32*)(IFXMIPS_CGU_BASE_ADDR + 0x0014))
#define IFXMIPS_CGU_IF_CLK			((u32*)(IFXMIPS_CGU_BASE_ADDR + 0x0018))
#define IFXMIPS_CGU_OSC_CON			((u32*)(IFXMIPS_CGU_BASE_ADDR + 0x001C))
#define IFXMIPS_CGU_SMD				((u32*)(IFXMIPS_CGU_BASE_ADDR + 0x0020))
#define IFXMIPS_CGU_CT1SR			((u32*)(IFXMIPS_CGU_BASE_ADDR + 0x0028))
#define IFXMIPS_CGU_CT2SR			((u32*)(IFXMIPS_CGU_BASE_ADDR + 0x002C))
#define IFXMIPS_CGU_PCMCR			((u32*)(IFXMIPS_CGU_BASE_ADDR + 0x0030))
#define IFXMIPS_CGU_PCI_CR			((u32*)(IFXMIPS_CGU_BASE_ADDR + 0x0034))
#define IFXMIPS_CGU_PD_PC			((u32*)(IFXMIPS_CGU_BASE_ADDR + 0x0038))
#define IFXMIPS_CGU_FMR				((u32*)(IFXMIPS_CGU_BASE_ADDR + 0x003C))

/* clock mux */
#define IFXMIPS_CGU_SYS			((u32*)(IFXMIPS_CGU_BASE_ADDR + 0x0010))
#define IFXMIPS_CGU_IFCCR		((u32*)(IFXMIPS_CGU_BASE_ADDR + 0x0018))
#define IFXMIPS_CGU_PCICR		((u32*)(IFXMIPS_CGU_BASE_ADDR + 0x0034))

#define CLOCK_60M				60000000
#define CLOCK_83M				83333333
#define CLOCK_111M				111111111
#define CLOCK_133M				133333333
#define CLOCK_167M				166666667
#define CLOCK_333M				333333333


/*------------ CGU */

#define IFXMIPS_PMU_BASE_ADDR	(KSEG1 + 0x1F102000)

#define IFXMIPS_PMU_PWDCR		((u32*)(IFXMIPS_PMU_BASE_ADDR + 0x001C))
#define IFXMIPS_PMU_PWDSR		((u32*)(IFXMIPS_PMU_BASE_ADDR + 0x0020))


/*------------ ICU */

#define IFXMIPS_ICU_BASE_ADDR	0xBF880200


#define IFXMIPS_ICU_IM0_ISR		((u32*)(IFXMIPS_ICU_BASE_ADDR + 0x0000))
#define IFXMIPS_ICU_IM0_IER		((u32*)(IFXMIPS_ICU_BASE_ADDR + 0x0008))
#define IFXMIPS_ICU_IM0_IOSR	((u32*)(IFXMIPS_ICU_BASE_ADDR + 0x0010))
#define IFXMIPS_ICU_IM0_IRSR	((u32*)(IFXMIPS_ICU_BASE_ADDR + 0x0018))
#define IFXMIPS_ICU_IM0_IMR		((u32*)(IFXMIPS_ICU_BASE_ADDR + 0x0020))

#define IFXMIPS_ICU_IM1_ISR		((u32*)(IFXMIPS_ICU_BASE_ADDR + 0x0028))
#define IFXMIPS_ICU_IM5_IER		((u32*)(IFXMIPS_ICU_BASE_ADDR + 0x00D0))

#define IFXMIPS_ICU_OFFSET		(IFXMIPS_ICU_IM1_ISR - IFXMIPS_ICU_IM0_ISR)


/*------------ ETOP */

#define IFXMIPS_PPE32_BASE_ADDR	0xBE180000

#define ETHERNET_PACKET_DMA_BUFFER_SIZE		0x600

#define IFXMIPS_PPE32_MEM_MAP	((u32*)(IFXMIPS_PPE32_BASE_ADDR + 0x10000))
#define IFXMIPS_PPE32_SRST		((u32*)(IFXMIPS_PPE32_BASE_ADDR + 0x10080))

#define MII_MODE 1
#define REV_MII_MODE 2

/* mdio access */
#define IFXMIPS_PPE32_MDIO_CFG	((u32*)(IFXMIPS_PPE32_BASE_ADDR + 0x11800))
#define IFXMIPS_PPE32_MDIO_ACC	((u32*)(IFXMIPS_PPE32_BASE_ADDR + 0x11804))

#define MDIO_ACC_REQUEST		0x80000000
#define MDIO_ACC_READ			0x40000000
#define MDIO_ACC_ADDR_MASK		0x1f
#define MDIO_ACC_ADDR_OFFSET	0x15
#define MDIO_ACC_REG_MASK		0xff
#define MDIO_ACC_REG_OFFSET		0x10
#define MDIO_ACC_VAL_MASK		0xffff

/* configuration */
#define IFXMIPS_PPE32_CFG		((u32*)(IFXMIPS_PPE32_MEM_MAP + 0x1808))

#define PPE32_MII_MASK			0xfffffffc
#define PPE32_MII_NORMAL		0x8
#define PPE32_MII_REVERSE		0xe

/* packet length */
#define IFXMIPS_PPE32_IG_PLEN_CTRL	((u32*)(IFXMIPS_PPE32_MEM_MAP + 0x1820))

#define PPE32_PLEN_OVER			0x5ee
#define PPE32_PLEN_UNDER		0x400000

/* enet */
#define IFXMIPS_PPE32_ENET_MAC_CFG	((u32*)(IFXMIPS_PPE32_MEM_MAP + 0x1840))

#define PPE32_CGEN				0x800


/*------------ DMA */
#define IFXMIPS_DMA_BASE_ADDR	0xBE104100

#define IFXMIPS_DMA_CS			((u32*)(IFXMIPS_DMA_BASE_ADDR + 0x18))
#define IFXMIPS_DMA_CIE			((u32*)(IFXMIPS_DMA_BASE_ADDR + 0x2C))
#define IFXMIPS_DMA_IRNEN		((u32*)(IFXMIPS_DMA_BASE_ADDR + 0xf4))
#define IFXMIPS_DMA_CCTRL		((u32*)(IFXMIPS_DMA_BASE_ADDR + 0x1C))
#define IFXMIPS_DMA_CIS			((u32*)(IFXMIPS_DMA_BASE_ADDR + 0x28))
#define IFXMIPS_DMA_CDLEN		((u32*)(IFXMIPS_DMA_BASE_ADDR + 0x24))
#define IFXMIPS_DMA_PS			((u32*)(IFXMIPS_DMA_BASE_ADDR + 0x40))
#define IFXMIPS_DMA_PCTRL		((u32*)(IFXMIPS_DMA_BASE_ADDR + 0x44))
#define IFXMIPS_DMA_CTRL			((u32*)(IFXMIPS_DMA_BASE_ADDR + 0x10))
#define IFXMIPS_DMA_CPOLL		((u32*)(IFXMIPS_DMA_BASE_ADDR + 0x14))
#define IFXMIPS_DMA_CDBA			((u32*)(IFXMIPS_DMA_BASE_ADDR + 0x20))


/*------------ PCI */
#define PCI_CR_PR_BASE_ADDR		(KSEG1 + 0x1E105400)

#define PCI_CR_FCI_ADDR_MAP0	((u32*)(PCI_CR_PR_BASE_ADDR + 0x00C0))
#define PCI_CR_FCI_ADDR_MAP1	((u32*)(PCI_CR_PR_BASE_ADDR + 0x00C4))
#define PCI_CR_FCI_ADDR_MAP2	((u32*)(PCI_CR_PR_BASE_ADDR + 0x00C8))
#define PCI_CR_FCI_ADDR_MAP3	((u32*)(PCI_CR_PR_BASE_ADDR + 0x00CC))
#define PCI_CR_FCI_ADDR_MAP4	((u32*)(PCI_CR_PR_BASE_ADDR + 0x00D0))
#define PCI_CR_FCI_ADDR_MAP5	((u32*)(PCI_CR_PR_BASE_ADDR + 0x00D4))
#define PCI_CR_FCI_ADDR_MAP6	((u32*)(PCI_CR_PR_BASE_ADDR + 0x00D8))
#define PCI_CR_FCI_ADDR_MAP7	((u32*)(PCI_CR_PR_BASE_ADDR + 0x00DC))
#define PCI_CR_CLK_CTRL			((u32*)(PCI_CR_PR_BASE_ADDR + 0x0000))
#define PCI_CR_PCI_MOD			((u32*)(PCI_CR_PR_BASE_ADDR + 0x0030))
#define PCI_CR_PC_ARB			((u32*)(PCI_CR_PR_BASE_ADDR + 0x0080))
#define PCI_CR_FCI_ADDR_MAP11hg	((u32*)(PCI_CR_PR_BASE_ADDR + 0x00E4))
#define PCI_CR_BAR11MASK		((u32*)(PCI_CR_PR_BASE_ADDR + 0x0044))
#define PCI_CR_BAR12MASK		((u32*)(PCI_CR_PR_BASE_ADDR + 0x0048))
#define PCI_CR_BAR13MASK		((u32*)(PCI_CR_PR_BASE_ADDR + 0x004C))
#define PCI_CS_BASE_ADDR1		((u32*)(PCI_CS_PR_BASE_ADDR + 0x0010))
#define PCI_CR_PCI_ADDR_MAP11	((u32*)(PCI_CR_PR_BASE_ADDR + 0x0064))
#define PCI_CR_FCI_BURST_LENGTH	((u32*)(PCI_CR_PR_BASE_ADDR + 0x00E8))
#define PCI_CR_PCI_EOI			((u32*)(PCI_CR_PR_BASE_ADDR + 0x002C))

#define PCI_CS_PR_BASE_ADDR		(KSEG1 + 0x17000000)

#define PCI_CS_STS_CMD			((u32*)(PCI_CS_PR_BASE_ADDR + 0x0004))

#define PCI_MASTER0_REQ_MASK_2BITS	8
#define PCI_MASTER1_REQ_MASK_2BITS	10
#define PCI_MASTER2_REQ_MASK_2BITS	12
#define INTERNAL_ARB_ENABLE_BIT		0


/*------------ WDT */

#define IFXMIPS_WDT_BASE_ADDR	(KSEG1 + 0x1F880000)

#define IFXMIPS_BIU_WDT_CR		((u32*)(IFXMIPS_WDT_BASE_ADDR + 0x03F0))
#define IFXMIPS_BIU_WDT_SR		((u32*)(IFXMIPS_WDT_BASE_ADDR + 0x03F8))


/*------------ LED */

#define IFXMIPS_LED_BASE_ADDR	(KSEG1 + 0x1E100BB0)
#define IFXMIPS_LED_CON0			((u32*)(IFXMIPS_LED_BASE_ADDR + 0x0000))
#define IFXMIPS_LED_CON1			((u32*)(IFXMIPS_LED_BASE_ADDR + 0x0004))
#define IFXMIPS_LED_CPU0			((u32*)(IFXMIPS_LED_BASE_ADDR + 0x0008))
#define IFXMIPS_LED_CPU1			((u32*)(IFXMIPS_LED_BASE_ADDR + 0x000C))
#define IFXMIPS_LED_AR			((u32*)(IFXMIPS_LED_BASE_ADDR + 0x0010))

#define LED_CON0_SWU			(1 << 31)
#define LED_CON0_AD1			(1 << 25)
#define LED_CON0_AD0			(1 << 24)

#define IFXMIPS_LED_2HZ          (0)
#define IFXMIPS_LED_4HZ          (1 << 23)
#define IFXMIPS_LED_8HZ          (2 << 23)
#define IFXMIPS_LED_10HZ         (3 << 23)
#define IFXMIPS_LED_MASK         (0xf << 23)

#define IFXMIPS_LED_UPD_SRC_FPI  (1 << 31)
#define IFXMIPS_LED_UPD_MASK     (3 << 30)
#define IFXMIPS_LED_ADSL_SRC		(3 << 24)

#define IFXMIPS_LED_GROUP0		(1 << 0)
#define IFXMIPS_LED_GROUP1		(1 << 1)
#define IFXMIPS_LED_GROUP2		(1 << 2)

#define IFXMIPS_LED_RISING		0
#define IFXMIPS_LED_FALLING		(1 << 26)
#define IFXMIPS_LED_EDGE_MASK	(1 << 26)


/*------------ GPIO */

#define IFXMIPS_GPIO_BASE_ADDR	(0xBE100B00)

#define IFXMIPS_GPIO_P0_OUT		((u32*)(IFXMIPS_GPIO_BASE_ADDR + 0x0010))
#define IFXMIPS_GPIO_P1_OUT		((u32*)(IFXMIPS_GPIO_BASE_ADDR + 0x0040))
#define IFXMIPS_GPIO_P0_IN		((u32*)(IFXMIPS_GPIO_BASE_ADDR + 0x0014))
#define IFXMIPS_GPIO_P1_IN		((u32*)(IFXMIPS_GPIO_BASE_ADDR + 0x0044))
#define IFXMIPS_GPIO_P0_DIR		((u32*)(IFXMIPS_GPIO_BASE_ADDR + 0x0018))
#define IFXMIPS_GPIO_P1_DIR		((u32*)(IFXMIPS_GPIO_BASE_ADDR + 0x0048))
#define IFXMIPS_GPIO_P0_ALTSEL0	((u32*)(IFXMIPS_GPIO_BASE_ADDR + 0x001C))
#define IFXMIPS_GPIO_P1_ALTSEL0	((u32*)(IFXMIPS_GPIO_BASE_ADDR + 0x004C))
#define IFXMIPS_GPIO_P0_ALTSEL1	((u32*)(IFXMIPS_GPIO_BASE_ADDR + 0x0020))
#define IFXMIPS_GPIO_P1_ALTSEL1	((u32*)(IFXMIPS_GPIO_BASE_ADDR + 0x0050))
#define IFXMIPS_GPIO_P0_OD		((u32*)(IFXMIPS_GPIO_BASE_ADDR + 0x0024))
#define IFXMIPS_GPIO_P1_OD		((u32*)(IFXMIPS_GPIO_BASE_ADDR + 0x0054))
#define IFXMIPS_GPIO_P0_STOFF	((u32*)(IFXMIPS_GPIO_BASE_ADDR + 0x0028))
#define IFXMIPS_GPIO_P1_STOFF	((u32*)(IFXMIPS_GPIO_BASE_ADDR + 0x0058))
#define IFXMIPS_GPIO_P0_PUDSEL	((u32*)(IFXMIPS_GPIO_BASE_ADDR + 0x002C))
#define IFXMIPS_GPIO_P1_PUDSEL	((u32*)(IFXMIPS_GPIO_BASE_ADDR + 0x005C))
#define IFXMIPS_GPIO_P0_PUDEN	((u32*)(IFXMIPS_GPIO_BASE_ADDR + 0x0030))
#define IFXMIPS_GPIO_P1_PUDEN	((u32*)(IFXMIPS_GPIO_BASE_ADDR + 0x0060))


/*------------ SSC */

#define IFXMIPS_SSC_BASE_ADDR	(KSEG1 + 0x1e100800)


#define IFXMIPS_SSC_CLC			((u32*)(IFXMIPS_SSC_BASE_ADDR + 0x0000))
#define IFXMIPS_SSC_IRN			((u32*)(IFXMIPS_SSC_BASE_ADDR + 0x00F4))
#define IFXMIPS_SSC_SFCON		((u32*)(IFXMIPS_SSC_BASE_ADDR + 0x0060))
#define IFXMIPS_SSC_WHBGPOSTAT	((u32*)(IFXMIPS_SSC_BASE_ADDR + 0x0078))
#define IFXMIPS_SSC_STATE    	((u32*)(IFXMIPS_SSC_BASE_ADDR + 0x0014))
#define IFXMIPS_SSC_WHBSTATE	((u32*)(IFXMIPS_SSC_BASE_ADDR + 0x0018))
#define IFXMIPS_SSC_FSTAT		((u32*)(IFXMIPS_SSC_BASE_ADDR + 0x0038))
#define IFXMIPS_SSC_ID			((u32*)(IFXMIPS_SSC_BASE_ADDR + 0x0008))
#define IFXMIPS_SSC_TB			((u32*)(IFXMIPS_SSC_BASE_ADDR + 0x0020))
#define IFXMIPS_SSC_RXFCON		((u32*)(IFXMIPS_SSC_BASE_ADDR + 0x0030))
#define IFXMIPS_SSC_TXFCON		((u32*)(IFXMIPS_SSC_BASE_ADDR + 0x0034))
#define IFXMIPS_SSC_CON			((u32*)(IFXMIPS_SSC_BASE_ADDR + 0x0010))
#define IFXMIPS_SSC_GPOSTAT		((u32*)(IFXMIPS_SSC_BASE_ADDR + 0x0074))
#define IFXMIPS_SSC_RB			((u32*)(IFXMIPS_SSC_BASE_ADDR + 0x0024))
#define IFXMIPS_SSC_RXCNT		((u32*)(IFXMIPS_SSC_BASE_ADDR + 0x0084))
#define IFXMIPS_SSC_GPOCON		((u32*)(IFXMIPS_SSC_BASE_ADDR + 0x0070))
#define IFXMIPS_SSC_BR			((u32*)(IFXMIPS_SSC_BASE_ADDR + 0x0040))
#define IFXMIPS_SSC_RXREQ		((u32*)(IFXMIPS_SSC_BASE_ADDR + 0x0080))
#define IFXMIPS_SSC_SFSTAT		((u32*)(IFXMIPS_SSC_BASE_ADDR + 0x0064))
#define IFXMIPS_SSC_RXCNT		((u32*)(IFXMIPS_SSC_BASE_ADDR + 0x0084))


/*------------ MEI */

#define IFXMIPS_MEI_BASE_ADDR	(0xBE116000)

#define MEI_DATA_XFR			((u32*)(IFXMIPS_MEI_BASE_ADDR + 0x0000))
#define MEI_VERSION				((u32*)(IFXMIPS_MEI_BASE_ADDR + 0x0004))
#define MEI_ARC_GP_STAT			((u32*)(IFXMIPS_MEI_BASE_ADDR + 0x0008))
#define MEI_DATA_XFR_STAT		((u32*)(IFXMIPS_MEI_BASE_ADDR + 0x000C))
#define MEI_XFR_ADDR			((u32*)(IFXMIPS_MEI_BASE_ADDR + 0x0010))
#define MEI_MAX_WAIT			((u32*)(IFXMIPS_MEI_BASE_ADDR + 0x0014))
#define MEI_TO_ARC_INT			((u32*)(IFXMIPS_MEI_BASE_ADDR + 0x0018))
#define ARC_TO_MEI_INT			((u32*)(IFXMIPS_MEI_BASE_ADDR + 0x001C))
#define ARC_TO_MEI_INT_MASK		((u32*)(IFXMIPS_MEI_BASE_ADDR + 0x0020))
#define MEI_DEBUG_WAD			((u32*)(IFXMIPS_MEI_BASE_ADDR + 0x0024))
#define MEI_DEBUG_RAD			((u32*)(IFXMIPS_MEI_BASE_ADDR + 0x0028))
#define MEI_DEBUG_DATA			((u32*)(IFXMIPS_MEI_BASE_ADDR + 0x002C))
#define MEI_DEBUG_DEC			((u32*)(IFXMIPS_MEI_BASE_ADDR + 0x0030))
#define MEI_CONFIG				((u32*)(IFXMIPS_MEI_BASE_ADDR + 0x0034))
#define MEI_RST_CONTROL			((u32*)(IFXMIPS_MEI_BASE_ADDR + 0x0038))
#define MEI_DBG_MASTER			((u32*)(IFXMIPS_MEI_BASE_ADDR + 0x003C))
#define MEI_CLK_CONTROL			((u32*)(IFXMIPS_MEI_BASE_ADDR + 0x0040))
#define MEI_BIST_CONTROL		((u32*)(IFXMIPS_MEI_BASE_ADDR + 0x0044))
#define MEI_BIST_STAT			((u32*)(IFXMIPS_MEI_BASE_ADDR + 0x0048))
#define MEI_XDATA_BASE_SH		((u32*)(IFXMIPS_MEI_BASE_ADDR + 0x004c))
#define MEI_XDATA_BASE			((u32*)(IFXMIPS_MEI_BASE_ADDR + 0x0050))
#define MEI_XMEM_BAR_BASE		((u32*)(IFXMIPS_MEI_BASE_ADDR + 0x0054))
#define MEI_XMEM_BAR0			((u32*)(IFXMIPS_MEI_BASE_ADDR + 0x0054))
#define MEI_XMEM_BAR1			((u32*)(IFXMIPS_MEI_BASE_ADDR + 0x0058))
#define MEI_XMEM_BAR2			((u32*)(IFXMIPS_MEI_BASE_ADDR + 0x005C))
#define MEI_XMEM_BAR3			((u32*)(IFXMIPS_MEI_BASE_ADDR + 0x0060))
#define MEI_XMEM_BAR4			((u32*)(IFXMIPS_MEI_BASE_ADDR + 0x0064))
#define MEI_XMEM_BAR5			((u32*)(IFXMIPS_MEI_BASE_ADDR + 0x0068))
#define MEI_XMEM_BAR6			((u32*)(IFXMIPS_MEI_BASE_ADDR + 0x006C))
#define MEI_XMEM_BAR7			((u32*)(IFXMIPS_MEI_BASE_ADDR + 0x0070))
#define MEI_XMEM_BAR8			((u32*)(IFXMIPS_MEI_BASE_ADDR + 0x0074))
#define MEI_XMEM_BAR9			((u32*)(IFXMIPS_MEI_BASE_ADDR + 0x0078))
#define MEI_XMEM_BAR10			((u32*)(IFXMIPS_MEI_BASE_ADDR + 0x007C))
#define MEI_XMEM_BAR11			((u32*)(IFXMIPS_MEI_BASE_ADDR + 0x0080))
#define MEI_XMEM_BAR12			((u32*)(IFXMIPS_MEI_BASE_ADDR + 0x0084))
#define MEI_XMEM_BAR13			((u32*)(IFXMIPS_MEI_BASE_ADDR + 0x0088))
#define MEI_XMEM_BAR14			((u32*)(IFXMIPS_MEI_BASE_ADDR + 0x008C))
#define MEI_XMEM_BAR15			((u32*)(IFXMIPS_MEI_BASE_ADDR + 0x0090))
#define MEI_XMEM_BAR16			((u32*)(IFXMIPS_MEI_BASE_ADDR + 0x0094))


/*------------ DEU */

#define IFXMIPS_DEU_BASE     (KSEG1 + 0x1E103100)
#define IFXMIPS_DEU_CLK			((u32 *)(IFXMIPS_DEU_BASE + 0x0000))
#define IFXMIPS_DEU_ID			((u32 *)(IFXMIPS_DEU_BASE + 0x0008))

#define IFXMIPS_DES_CON			((u32 *)(IFXMIPS_DEU_BASE + 0x0010))
#define IFXMIPS_DES_IHR			((u32 *)(IFXMIPS_DEU_BASE + 0x0014))
#define IFXMIPS_DES_ILR			((u32 *)(IFXMIPS_DEU_BASE + 0x0018))
#define IFXMIPS_DES_K1HR		((u32 *)(IFXMIPS_DEU_BASE + 0x001C))
#define IFXMIPS_DES_K1LR		((u32 *)(IFXMIPS_DEU_BASE + 0x0020))
#define IFXMIPS_DES_K3HR		((u32 *)(IFXMIPS_DEU_BASE + 0x0024))
#define IFXMIPS_DES_K3LR		((u32 *)(IFXMIPS_DEU_BASE + 0x0028))
#define IFXMIPS_DES_IVHR		((u32 *)(IFXMIPS_DEU_BASE + 0x002C))
#define IFXMIPS_DES_IVLR		((u32 *)(IFXMIPS_DEU_BASE + 0x0030))
#define IFXMIPS_DES_OHR			((u32 *)(IFXMIPS_DEU_BASE + 0x0040))
#define IFXMIPS_DES_OLR			((u32 *)(IFXMIPS_DEU_BASE + 0x0050))
#define IFXMIPS_AES_CON			((u32 *)(IFXMIPS_DEU_BASE + 0x0050))
#define IFXMIPS_AES_ID3R		((u32 *)(IFXMIPS_DEU_BASE + 0x0054))
#define IFXMIPS_AES_ID2R		((u32 *)(IFXMIPS_DEU_BASE + 0x0058))
#define IFXMIPS_AES_ID1R		((u32 *)(IFXMIPS_DEU_BASE + 0x005C))
#define IFXMIPS_AES_ID0R		((u32 *)(IFXMIPS_DEU_BASE + 0x0060))
#define IFXMIPS_AES_K7R			((u32 *)(IFXMIPS_DEU_BASE + 0x0064))
#define IFXMIPS_AES_K6R			((u32 *)(IFXMIPS_DEU_BASE + 0x0068))
#define IFXMIPS_AES_K5R			((u32 *)(IFXMIPS_DEU_BASE + 0x006C))
#define IFXMIPS_AES_K4R			((u32 *)(IFXMIPS_DEU_BASE + 0x0070))
#define IFXMIPS_AES_K3R			((u32 *)(IFXMIPS_DEU_BASE + 0x0074))
#define IFXMIPS_AES_K2R			((u32 *)(IFXMIPS_DEU_BASE + 0x0078))
#define IFXMIPS_AES_K1R			((u32 *)(IFXMIPS_DEU_BASE + 0x007C))
#define IFXMIPS_AES_K0R			((u32 *)(IFXMIPS_DEU_BASE + 0x0080))
#define IFXMIPS_AES_IV3R		((u32 *)(IFXMIPS_DEU_BASE + 0x0084))
#define IFXMIPS_AES_IV2R		((u32 *)(IFXMIPS_DEU_BASE + 0x0088))
#define IFXMIPS_AES_IV1R		((u32 *)(IFXMIPS_DEU_BASE + 0x008C))
#define IFXMIPS_AES_IV0R		((u32 *)(IFXMIPS_DEU_BASE + 0x0090))
#define IFXMIPS_AES_0D3R		((u32 *)(IFXMIPS_DEU_BASE + 0x0094))
#define IFXMIPS_AES_0D2R		((u32 *)(IFXMIPS_DEU_BASE + 0x0098))
#define IFXMIPS_AES_OD1R		((u32 *)(IFXMIPS_DEU_BASE + 0x009C))
#define IFXMIPS_AES_OD0R		((u32 *)(IFXMIPS_DEU_BASE + 0x00A0))

/*------------ FUSE */

#define IFXMIPS_FUSE_BASE_ADDR	(KSEG1 + 0x1F107354)


/*------------ MPS */

#define IFXMIPS_MPS_BASE_ADDR	(KSEG1 + 0x1F107000)
#define IFXMIPS_MPS_SRAM		((u32*)(KSEG1 + 0x1F200000))

#define IFXMIPS_MPS_CHIPID		((u32*)(IFXMIPS_MPS_BASE_ADDR + 0x0344))
#define IFXMIPS_MPS_VC0ENR		((u32*)(IFXMIPS_MPS_BASE_ADDR + 0x0000))
#define IFXMIPS_MPS_VC1ENR		((u32*)(IFXMIPS_MPS_BASE_ADDR + 0x0004))
#define IFXMIPS_MPS_VC2ENR		((u32*)(IFXMIPS_MPS_BASE_ADDR + 0x0008))
#define IFXMIPS_MPS_VC3ENR		((u32*)(IFXMIPS_MPS_BASE_ADDR + 0x000C))
#define IFXMIPS_MPS_RVC0SR		((u32*)(IFXMIPS_MPS_BASE_ADDR + 0x0010))
#define IFXMIPS_MPS_RVC1SR		((u32*)(IFXMIPS_MPS_BASE_ADDR + 0x0014))
#define IFXMIPS_MPS_RVC2SR		((u32*)(IFXMIPS_MPS_BASE_ADDR + 0x0018))
#define IFXMIPS_MPS_RVC3SR		((u32*)(IFXMIPS_MPS_BASE_ADDR + 0x001C))
#define IFXMIPS_MPS_SVC0SR		((u32*)(IFXMIPS_MPS_BASE_ADDR + 0x0020))
#define IFXMIPS_MPS_SVC1SR		((u32*)(IFXMIPS_MPS_BASE_ADDR + 0x0024))
#define IFXMIPS_MPS_SVC2SR		((u32*)(IFXMIPS_MPS_BASE_ADDR + 0x0028))
#define IFXMIPS_MPS_SVC3SR		((u32*)(IFXMIPS_MPS_BASE_ADDR + 0x002C))
#define IFXMIPS_MPS_CVC0SR		((u32*)(IFXMIPS_MPS_BASE_ADDR + 0x0030))
#define IFXMIPS_MPS_CVC1SR		((u32*)(IFXMIPS_MPS_BASE_ADDR + 0x0034))
#define IFXMIPS_MPS_CVC2SR		((u32*)(IFXMIPS_MPS_BASE_ADDR + 0x0038))
#define IFXMIPS_MPS_CVC3SR		((u32*)(IFXMIPS_MPS_BASE_ADDR + 0x003C))
#define IFXMIPS_MPS_RAD0SR		((u32*)(IFXMIPS_MPS_BASE_ADDR + 0x0040))
#define IFXMIPS_MPS_RAD1SR		((u32*)(IFXMIPS_MPS_BASE_ADDR + 0x0044))
#define IFXMIPS_MPS_SAD0SR		((u32*)(IFXMIPS_MPS_BASE_ADDR + 0x0048))
#define IFXMIPS_MPS_SAD1SR		((u32*)(IFXMIPS_MPS_BASE_ADDR + 0x004C))
#define IFXMIPS_MPS_CAD0SR		((u32*)(IFXMIPS_MPS_BASE_ADDR + 0x0050))
#define IFXMIPS_MPS_CAD1SR		((u32*)(IFXMIPS_MPS_BASE_ADDR + 0x0054))
#define IFXMIPS_MPS_AD0ENR		((u32*)(IFXMIPS_MPS_BASE_ADDR + 0x0058))
#define IFXMIPS_MPS_AD1ENR		((u32*)(IFXMIPS_MPS_BASE_ADDR + 0x005C))

#define IFXMIPS_MPS_CHIPID_VERSION_GET(value)	 (((value) >> 28) & ((1 << 4) - 1))
#define IFXMIPS_MPS_CHIPID_VERSION_SET(value)	(((( 1 << 4) - 1) & (value)) << 28)
#define IFXMIPS_MPS_CHIPID_PARTNUM_GET(value)	(((value) >> 12) & ((1 << 16) - 1))
#define IFXMIPS_MPS_CHIPID_PARTNUM_SET(value)	(((( 1 << 16) - 1) & (value)) << 12)
#define IFXMIPS_MPS_CHIPID_MANID_GET(value)		(((value) >> 1) & ((1 << 10) - 1))
#define IFXMIPS_MPS_CHIPID_MANID_SET(value)		(((( 1 << 10) - 1) & (value)) << 1)

#endif
