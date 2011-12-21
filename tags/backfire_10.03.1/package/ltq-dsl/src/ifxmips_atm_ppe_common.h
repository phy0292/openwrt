#ifndef IFXMIPS_ATM_PPE_COMMON_H
#define IFXMIPS_ATM_PPE_COMMON_H

#if defined(CONFIG_LANTIQ)
  #include "ifxmips_atm_ppe_danube.h"
  #define CONFIG_DANUBE
#elif defined(CONFIG_DANUBE) 
  #include "ifxmips_atm_ppe_danube.h"
#elif defined(CONFIG_AMAZON_SE)
  #include "ifxmips_atm_ppe_amazon_se.h"
#elif defined(CONFIG_AR9)
  #include "ifxmips_atm_ppe_ar9.h"
#elif defined(CONFIG_VR9)
  #include "ifxmips_atm_ppe_vr9.h"
#else
  #error Platform is not specified!
#endif


/*
 *  Code/Data Memory (CDM) Interface Configuration Register
 */
#define CDM_CFG                         PPE_REG_ADDR(0x0100)

#define CDM_CFG_RAM1                    GET_BITS(*CDM_CFG, 3, 2)
#define CDM_CFG_RAM0                    (*CDM_CFG & (1 << 1))

#define CDM_CFG_RAM1_SET(value)         SET_BITS(0, 3, 2, value)
#define CDM_CFG_RAM0_SET(value)         ((value) ? (1 << 1) : 0)

/*
 *  QSB Internal Cell Delay Variation Register
 */
#define QSB_ICDV                        QSB_CONF_REG_ADDR(0x0007)

#define QSB_ICDV_TAU                    GET_BITS(*QSB_ICDV, 5, 0)

#define QSB_ICDV_TAU_SET(value)         SET_BITS(0, 5, 0, value)

/*
 *  QSB Scheduler Burst Limit Register
 */
#define QSB_SBL                         QSB_CONF_REG_ADDR(0x0009)

#define QSB_SBL_SBL                     GET_BITS(*QSB_SBL, 3, 0)

#define QSB_SBL_SBL_SET(value)          SET_BITS(0, 3, 0, value)

/*
 *  QSB Configuration Register
 */
#define QSB_CFG                         QSB_CONF_REG_ADDR(0x000A)

#define QSB_CFG_TSTEPC                  GET_BITS(*QSB_CFG, 1, 0)

#define QSB_CFG_TSTEPC_SET(value)       SET_BITS(0, 1, 0, value)

/*
 *  QSB RAM Transfer Table Register
 */
#define QSB_RTM                         QSB_CONF_REG_ADDR(0x000B)

#define QSB_RTM_DM                      (*QSB_RTM)

#define QSB_RTM_DM_SET(value)           ((value) & 0xFFFFFFFF)

/*
 *  QSB RAM Transfer Data Register
 */
#define QSB_RTD                         QSB_CONF_REG_ADDR(0x000C)

#define QSB_RTD_TTV                     (*QSB_RTD)

#define QSB_RTD_TTV_SET(value)          ((value) & 0xFFFFFFFF)

/*
 *  QSB RAM Access Register
 */
#define QSB_RAMAC                       QSB_CONF_REG_ADDR(0x000D)

#define QSB_RAMAC_RW                    (*QSB_RAMAC & (1 << 31))
#define QSB_RAMAC_TSEL                  GET_BITS(*QSB_RAMAC, 27, 24)
#define QSB_RAMAC_LH                    (*QSB_RAMAC & (1 << 16))
#define QSB_RAMAC_TESEL                 GET_BITS(*QSB_RAMAC, 9, 0)

#define QSB_RAMAC_RW_SET(value)         ((value) ? (1 << 31) : 0)
#define QSB_RAMAC_TSEL_SET(value)       SET_BITS(0, 27, 24, value)
#define QSB_RAMAC_LH_SET(value)         ((value) ? (1 << 16) : 0)
#define QSB_RAMAC_TESEL_SET(value)      SET_BITS(0, 9, 0, value)

/*
 *  QSB Queue Scheduling and Shaping Definitions
 */
#define QSB_WFQ_NONUBR_MAX              0x3f00
#define QSB_WFQ_UBR_BYPASS              0x3fff
#define QSB_TP_TS_MAX                   65472
#define QSB_TAUS_MAX                    64512
#define QSB_GCR_MIN                     18

/*
 *  QSB Constant
 */
#define QSB_RAMAC_RW_READ               0
#define QSB_RAMAC_RW_WRITE              1

#define QSB_RAMAC_TSEL_QPT              0x01
#define QSB_RAMAC_TSEL_SCT              0x02
#define QSB_RAMAC_TSEL_SPT              0x03
#define QSB_RAMAC_TSEL_VBR              0x08

#define QSB_RAMAC_LH_LOW                0
#define QSB_RAMAC_LH_HIGH               1

#define QSB_QPT_SET_MASK                0x0
#define QSB_QVPT_SET_MASK               0x0
#define QSB_SET_SCT_MASK                0x0
#define QSB_SET_SPT_MASK                0x0
#define QSB_SET_SPT_SBVALID_MASK        0x7FFFFFFF

#define QSB_SPT_SBV_VALID               (1 << 31)
#define QSB_SPT_PN_SET(value)           (((value) & 0x01) ? (1 << 16) : 0)
#define QSB_SPT_INTRATE_SET(value)      SET_BITS(0, 13, 0, value)

/*
 *  QSB Queue Parameter Table Entry and Queue VBR Parameter Table Entry
 */
#if defined(__BIG_ENDIAN)
    union qsb_queue_parameter_table {
        struct {
            unsigned int    res1    :1;
            unsigned int    vbr     :1;
            unsigned int    wfqf    :14;
            unsigned int    tp      :16;
        }               bit;
        u32             dword;
    };

    union qsb_queue_vbr_parameter_table {
        struct {
            unsigned int    taus    :16;
            unsigned int    ts      :16;
        }               bit;
        u32             dword;
    };
#else
    union qsb_queue_parameter_table {
        struct {
            unsigned int    tp      :16;
            unsigned int    wfqf    :14;
            unsigned int    vbr     :1;
            unsigned int    res1    :1;
        }               bit;
        u32             dword;
    };

    union qsb_queue_vbr_parameter_table {
        struct {
            unsigned int    ts      :16;
            unsigned int    taus    :16;
        }               bit;
        u32             dword;
    };
#endif  //  defined(__BIG_ENDIAN)

/*
 *  Mailbox IGU0 Registers
 */
#define MBOX_IGU0_ISRS                  PPE_REG_ADDR(0x0200)
#define MBOX_IGU0_ISRC                  PPE_REG_ADDR(0x0201)
#define MBOX_IGU0_ISR                   PPE_REG_ADDR(0x0202)
#define MBOX_IGU0_IER                   PPE_REG_ADDR(0x0203)

#define MBOX_IGU0_ISRS_SET(n)           (1 << (n))
#define MBOX_IGU0_ISRC_CLEAR(n)         (1 << (n))
#define MBOX_IGU0_ISR_ISR(n)            (*MBOX_IGU0_ISR & (1 << (n)))
#define MBOX_IGU0_IER_EN(n)             (*MBOX_IGU0_IER & (1 << (n)))
#define MBOX_IGU0_IER_EN_SET(n)         (1 << (n))

/*
 *  Mailbox IGU1 Registers
 */
#define MBOX_IGU1_ISRS                  PPE_REG_ADDR(0x0204)
#define MBOX_IGU1_ISRC                  PPE_REG_ADDR(0x0205)
#define MBOX_IGU1_ISR                   PPE_REG_ADDR(0x0206)
#define MBOX_IGU1_IER                   PPE_REG_ADDR(0x0207)

#define MBOX_IGU1_ISRS_SET(n)           (1 << (n))
#define MBOX_IGU1_ISRC_CLEAR(n)         (1 << (n))
#define MBOX_IGU1_ISR_ISR(n)            (*MBOX_IGU1_ISR & (1 << (n)))
#define MBOX_IGU1_IER_EN(n)             (*MBOX_IGU1_IER & (1 << (n)))
#define MBOX_IGU1_IER_EN_SET(n)         (1 << (n))

/*
 *  Mailbox IGU3 Registers
 */
#define MBOX_IGU3_ISRS                  PPE_REG_ADDR(0x0214)
#define MBOX_IGU3_ISRC                  PPE_REG_ADDR(0x0215)
#define MBOX_IGU3_ISR                   PPE_REG_ADDR(0x0216)
#define MBOX_IGU3_IER                   PPE_REG_ADDR(0x0217)

#define MBOX_IGU3_ISRS_SET(n)           (1 << (n))
#define MBOX_IGU3_ISRC_CLEAR(n)         (1 << (n))
#define MBOX_IGU3_ISR_ISR(n)            (*MBOX_IGU3_ISR & (1 << (n)))
#define MBOX_IGU3_IER_EN(n)             (*MBOX_IGU3_IER & (1 << (n)))
#define MBOX_IGU3_IER_EN_SET(n)         (1 << (n))

/*
 *  RTHA/TTHA Registers
 */
#define SFSM_STATE0                             PPE_REG_ADDR(0x0410)
#define SFSM_STATE1                             PPE_REG_ADDR(0x0411)
#define SFSM_DBA0                               PPE_REG_ADDR(0x0412)
#define SFSM_DBA1                               PPE_REG_ADDR(0x0413)
#define SFSM_CBA0                               PPE_REG_ADDR(0x0414)
#define SFSM_CBA1                               PPE_REG_ADDR(0x0415)
#define SFSM_CFG0                               PPE_REG_ADDR(0x0416)
#define SFSM_CFG1                               PPE_REG_ADDR(0x0417)
#define SFSM_PGCNT0                             PPE_REG_ADDR(0x041C)
#define SFSM_PGCNT1                             PPE_REG_ADDR(0x041D)
#define FFSM_DBA0                               PPE_REG_ADDR(0x0508)
#define FFSM_DBA1                               PPE_REG_ADDR(0x0509)
#define FFSM_CFG0                               PPE_REG_ADDR(0x050A)
#define FFSM_CFG1                               PPE_REG_ADDR(0x050B)
#define FFSM_IDLE_HEAD_BC0                      PPE_REG_ADDR(0x050E)
#define FFSM_IDLE_HEAD_BC1                      PPE_REG_ADDR(0x050F)
#define FFSM_PGCNT0                             PPE_REG_ADDR(0x0514)
#define FFSM_PGCNT1                             PPE_REG_ADDR(0x0515)



#endif  //  IFXMIPS_ATM_PPE_COMMON_H
