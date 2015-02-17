#ifndef __MTK_PTP_H__
#define __MTK_PTP_H__

#include <mach/sync_write.h>
#include <linux/xlog.h>

#define PTP_GIC_SIGNALS         (32)
#define PTP_FSM_IRQ_ID          (PTP_GIC_SIGNALS + 30)
#define PTP_THERM_IRQ_ID        (PTP_GIC_SIGNALS + 25)
#define MAX_OPP_NUM             (8)
#define STANDARD_FREQ           (1000000)  // khz
#define PTP_LEVEL_INDEX         (18)
#define PTP_LEVEL_0             (0x3)
#define PTP_LEVEL_1             (0x2)
#define PTP_LEVEL_2             (0x1)

/* ====== PTP register ====== */
#define PTP_base_addr           (0xf100D000)
#define PTP_ctr_reg_addr        (PTP_base_addr+0x200)

#define PTP_DESCHAR             (PTP_ctr_reg_addr)
#define PTP_TEMPCHAR            (PTP_ctr_reg_addr+0x04)
#define PTP_DETCHAR             (PTP_ctr_reg_addr+0x08)
#define PTP_AGECHAR             (PTP_ctr_reg_addr+0x0c)

#define PTP_DCCONFIG            (PTP_ctr_reg_addr+0x10)
#define PTP_AGECONFIG           (PTP_ctr_reg_addr+0x14)
#define PTP_FREQPCT30           (PTP_ctr_reg_addr+0x18)
#define PTP_FREQPCT74           (PTP_ctr_reg_addr+0x1c)

#define PTP_LIMITVALS           (PTP_ctr_reg_addr+0x20)
#define PTP_VBOOT               (PTP_ctr_reg_addr+0x24)
#define PTP_DETWINDOW           (PTP_ctr_reg_addr+0x28)
#define PTP_PTPCONFIG           (PTP_ctr_reg_addr+0x2c)

#define PTP_TSCALCS             (PTP_ctr_reg_addr+0x30)
#define PTP_RUNCONFIG           (PTP_ctr_reg_addr+0x34)
#define PTP_PTPEN               (PTP_ctr_reg_addr+0x38)
#define PTP_INIT2VALS           (PTP_ctr_reg_addr+0x3c)

#define PTP_DCVALUES            (PTP_ctr_reg_addr+0x40)
#define PTP_AGEVALUES           (PTP_ctr_reg_addr+0x44)
#define PTP_VOP30               (PTP_ctr_reg_addr+0x48)
#define PTP_VOP74               (PTP_ctr_reg_addr+0x4c)

#define PTP_TEMP                (PTP_ctr_reg_addr+0x50)
#define PTP_PTPINTSTS           (PTP_ctr_reg_addr+0x54)
#define PTP_PTPINTSTSRAW        (PTP_ctr_reg_addr+0x58)
#define PTP_PTPINTEN            (PTP_ctr_reg_addr+0x5c)
#define PTP_AGECOUNT            (PTP_ctr_reg_addr+0x7c)
#define PTP_SMSTATE0            (PTP_ctr_reg_addr+0x80)
#define PTP_SMSTATE1            (PTP_ctr_reg_addr+0x84)

/* ====== Thermal Controller register ======= */
#define PTP_Thermal_ctr_reg_addr (PTP_base_addr)

#define TEMPMONCTL0             (PTP_base_addr)
#define TEMPMONCTL1             (PTP_base_addr+0x04)
#define TEMPMONCTL2             (PTP_base_addr+0x08)
#define TEMPMONINT              (PTP_base_addr+0x0c)

#define TEMPMONINTSTS           (PTP_base_addr+0x10)
#define TEMPMONIDET0            (PTP_base_addr+0x14)
#define TEMPMONIDET1            (PTP_base_addr+0x18)
#define TEMPMONIDET2            (PTP_base_addr+0x1c)

#define TEMPH2NTHRE             (PTP_base_addr+0x24)
#define TEMPHTHRE               (PTP_base_addr+0x28)
#define TEMPCTHRE               (PTP_base_addr+0x2c)

#define TEMPOFFSETH             (PTP_base_addr+0x30)
#define TEMPOFFSETL             (PTP_base_addr+0x34)
#define TEMPMSRCTL0             (PTP_base_addr+0x38)
#define TEMPMSRCTL1             (PTP_base_addr+0x3c)

#define TEMPAHBPOLL             (PTP_base_addr+0x40)
#define TEMPAHBTO               (PTP_base_addr+0x44)
#define TEMPADCPNP0             (PTP_base_addr+0x48)
#define TEMPADCPNP1             (PTP_base_addr+0x4c)

#define TEMPADCPNP2             (PTP_base_addr+0x50)
#define TEMPADCMUX              (PTP_base_addr+0x54)
#define TEMPADCEXT              (PTP_base_addr+0x58)
#define TEMPADCEXT1             (PTP_base_addr+0x5c)

#define TEMPADCEN               (PTP_base_addr+0x60)
#define TEMPPNPMUXADDR          (PTP_base_addr+0x64)
#define TEMPADCMUXADDR          (PTP_base_addr+0x68)
#define TEMPADCEXTADDR          (PTP_base_addr+0x6c)

#define TEMPADCEXT1ADDR         (PTP_base_addr+0x70)
#define TEMPADCENADDR           (PTP_base_addr+0x74)
#define TEMPADCVALIDADDR        (PTP_base_addr+0x78)
#define TEMPADCVOLTADDR         (PTP_base_addr+0x7c)

#define TEMPRDCTRL              (PTP_base_addr+0x80)
#define TEMPADCVALIDMASK        (PTP_base_addr+0x84)
#define TEMPADCVOLTAGESHIFT     (PTP_base_addr+0x88)
#define TEMPADCWRITECTRL        (PTP_base_addr+0x8c)

#define TEMPMSR0                (PTP_base_addr+0x90)
#define TEMPMSR1                (PTP_base_addr+0x94)
#define TEMPMSR2                (PTP_base_addr+0x98)

#define TEMPIMMD0               (PTP_base_addr+0xa0)
#define TEMPIMMD1               (PTP_base_addr+0xa4)
#define TEMPIMMD2               (PTP_base_addr+0xa8)

#define TEMPMONIDET3            (PTP_base_addr+0xb0)
#define TEMPADCPNP3             (PTP_base_addr+0xb4)
#define TEMPMSR3                (PTP_base_addr+0xb8)
#define TEMPIMMD3               (PTP_base_addr+0xbc)

#define TEMPPROTCTL             (PTP_base_addr+0xc0)
#define TEMPPROTTA              (PTP_base_addr+0xc4)
#define TEMPPROTTB              (PTP_base_addr+0xc8)
#define TEMPPROTTC              (PTP_base_addr+0xcc)

#define TEMPSPARE0              (PTP_base_addr+0xf0)
#define TEMPSPARE1              (PTP_base_addr+0xf4)
#define TEMPSPARE2              (PTP_base_addr+0xf8)
#define TEMPSPARE3              (PTP_base_addr+0xfc)

#define ptp_print(fmt, args...) xlog_printk(ANDROID_LOG_INFO, "PTPOD", fmt, ##args)
#define ptp_isr_print(fmt, args...) printk(KERN_DEBUG "[PTPOD] " fmt, ##args)

#define ptp_read(addr)		    (*(volatile u32 *)(addr))
#define ptp_write(addr, val)	mt65xx_reg_sync_writel(val, addr)

typedef struct {
    unsigned int ADC_CALI_EN;
    unsigned int PTPINITEN;
    unsigned int PTPMONEN;
    
    unsigned int MDES;
    unsigned int BDES;
    unsigned int DCCONFIG;
    unsigned int DCMDET;
    unsigned int DCBDET;
    unsigned int AGECONFIG;
    unsigned int AGEM;
    unsigned int AGEDELTA;
    unsigned int DVTFIXED;
    unsigned int VCO;
    unsigned int MTDES;
    unsigned int MTS;
    unsigned int BTS;

    char FREQPCT[MAX_OPP_NUM];

    unsigned int DETWINDOW;
    unsigned int VMAX;
    unsigned int VMIN;
    unsigned int DTHI;
    unsigned int DTLO;
    unsigned int VBOOT;
    unsigned int DETMAX;

    unsigned int DCVOFFSETIN;
    unsigned int AGEVOFFSETIN;
} PTP_Init_T;

enum init_type{
    INIT1_MODE = 0,
    INIT2_MODE,
    MONITOR_MODE,
};

void init_PTP_interrupt(void);
void init_PTP_Therm_interrupt(void);
void ptp_init1(void);
unsigned int get_ptp_level(void);

#endif /* __MTK_PTP_H__  */
