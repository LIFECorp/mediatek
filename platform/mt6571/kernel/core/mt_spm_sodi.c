#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/proc_fs.h>
#include <linux/platform_device.h>
#include <linux/earlysuspend.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/err.h>
#include <linux/delay.h>  

#include <mach/irqs.h>
#include <mach/mt_cirq.h>
#include <mach/mt_spm.h>
#include <mach/mt_dormant.h>
#include <mach/mt_gpt.h>
#include <mach/mt_spm_pcm.h>
#include <mach/mt_irq.h>
#include <mach/mt_spm_api.h>
#include <mach/mt_clkmgr.h>


//#include <mach/env.h> // require from hibboot flag
#include <asm/hardware/gic.h>

//===================================
#if defined (CONFIG_MT6572_FPGA_CA7)
#define SPM_SODI_BYPASS_SYSPWREQ 0//for FPGA attach jtag
#else
#define SPM_SODI_BYPASS_SYSPWREQ 1
#endif

#define clc_debug spm_debug
#define clc_notice spm_notice


#define WFI_OP        4
#define WFI_L2C      5
#define WFI_SCU      6
#define WFI_MM      16
#define WFI_MD      19

//#define SPM_SODI_SECONDARY_KICK_IMPROVE


#define SPM_SODI_FIRMWARE_VER "20131029A-debug0"
// ==========================================
// PCM code for SODI (Screen On Deep Idle)  v0.19 2013/09/06
//
// core 0 : GPT 4
// ==========================================
static u32 spm_pcm_sodi[] = {
    0x88000000, 0xfffffffb, 0x89c00007, 0xfffffffd, 0x89c00007, 0xfffbfeff,
    0xa9c00007, 0x00010000, 0x1b00001f, 0xbfffe7ff, 0xf0000000, 0x17c07c1f,
    0x1b80001f, 0x20000020, 0x8980000d, 0x00000010, 0xd8200286, 0x17c07c1f,
    0x1b80001f, 0x20000fdf, 0x1890001f, 0x10006608, 0x80c98801, 0x810a0801,
    0x10928c1f, 0xa0911002, 0xa0940402, 0x8080080d, 0xd80007e2, 0x17c07c1f,
    0x1b00001f, 0x3fffe7ff, 0x8880000c, 0x3fffe7ff, 0xd80007e2, 0x17c07c1f,
    0x89c00007, 0xfffeffff, 0x1950001f, 0x10006b00, 0x80d90405, 0xd80005e3,
    0x17c07c1f, 0xa1d40407, 0x1b80001f, 0x20000008, 0xa1d90407, 0x1950001f,
    0x10006b00, 0x80d98405, 0xd80007e3, 0x17c07c1f, 0xc0c013e0, 0x17c07c1f,
    0xd80007e3, 0x17c07c1f, 0x1950001f, 0x10006b00, 0x80da0405, 0xd80007e3,
    0x17c07c1f, 0xa8000000, 0x00000004, 0x1b00001f, 0x7fffe7ff, 0xf0000000,
    0x17c07c1f, 0xe0e00f16, 0x1392841f, 0xe0e00f1e, 0x1392841f, 0xe0e00f0e,
    0x1392841f, 0xe0e00f0c, 0xe0e00f0d, 0xe0e00e0d, 0xe0e00c0d, 0xe0e0080d,
    0xe0e0000d, 0xf0000000, 0x17c07c1f, 0x6aa00003, 0x10006234, 0xd8000b6a,
    0x17c07c1f, 0xe0e00f0d, 0xe0e00f0f, 0xe0e00f1e, 0xe0e00f12, 0xd8200cea,
    0x17c07c1f, 0xe8208000, 0x10006234, 0x000f0f0d, 0xe8208000, 0x10006234,
    0x000f0f0f, 0xe8208000, 0x10006234, 0x000f0f1e, 0xe8208000, 0x10006234,
    0x000f0f12, 0xf0000000, 0x17c07c1f, 0xd8000dea, 0x17c07c1f, 0xe2e00036,
    0x1392841f, 0xe2e0003e, 0x1393841f, 0xd8200f0a, 0x17c07c1f, 0xe2e0007e,
    0xe2e0006e, 0xe2e0006c, 0xe2e0007c, 0xe2e0005c, 0xe2e0005d, 0xe2e0004d,
    0xf0000000, 0x17c07c1f, 0xd800102a, 0x17c07c1f, 0xe2e0005d, 0xe2e0007d,
    0xe2e0007f, 0xe2e0003f, 0xe2e0003e, 0xd82010ea, 0x17c07c1f, 0xe2e0003a,
    0x1392841f, 0xe2e00032, 0x1392841f, 0xf0000000, 0x17c07c1f, 0x1a50001f,
    0x10006348, 0x82402c09, 0x1a00001f, 0x10006348, 0xe2000009, 0x1a50001f,
    0x1000634c, 0x82402c09, 0x1a00001f, 0x1000634c, 0xe2000009, 0xf0000000,
    0x17c07c1f, 0x80cab001, 0x808cb401, 0x80800c02, 0xd82013a2, 0x17c07c1f,
    0xa1d78407, 0xf0000000, 0x17c07c1f, 0xa1d08407, 0x1b80001f, 0x20000040,
    0x80eab401, 0x1a00001f, 0x10006814, 0xe2000003, 0xf0000000, 0x17c07c1f,
    0x1a00001f, 0x10006604, 0xd80015c3, 0x17c07c1f, 0xd82015c3, 0x17c07c1f,
    0xf0000000, 0x17c07c1f, 0x1a10001f, 0x11004058, 0x1a80001f, 0x11004058,
    0xaa000008, 0x80000000, 0xe2800008, 0xf0000000, 0x17c07c1f, 0xa1d40407,
    0x1b80001f, 0x20000008, 0xa1d90407, 0xf0000000, 0x17c07c1f, 0x18d0001f,
    0x10006b00, 0xa1002803, 0x18c0001f, 0x10006b00, 0xe0c00004, 0xf0000000,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x1840001f, 0x00000001,
    0x1b00001f, 0x3fffe7ff, 0x1b80001f, 0xd00f0000, 0x8880000c, 0x3fffe7ff,
    0xd80037a2, 0x17c07c1f, 0x1950001f, 0x10006400, 0x80d70405, 0xd80027c3,
    0x17c07c1f, 0x89c00007, 0xffffefff, 0x18c0001f, 0x10006200, 0xc0c00f40,
    0x12807c1f, 0xe8208000, 0x1000625c, 0x00000001, 0xc0c00f40, 0x1280041f,
    0x18c0001f, 0x10006208, 0xc0c00f40, 0x12807c1f, 0xe8208000, 0x10006248,
    0x00000000, 0xc0c00f40, 0x1280041f, 0xc28017e0, 0x1290041f, 0x1950001f,
    0x10006b00, 0x80d80405, 0xd80026e3, 0x17c07c1f, 0xa9c00007, 0x00000080,
    0xa8000000, 0x00000002, 0xa8000000, 0x00800200, 0x80d88405, 0xd80026e3,
    0x17c07c1f, 0xa8000000, 0x00020000, 0xc0c01600, 0x17c07c1f, 0x80dc0405,
    0xd82027c3, 0x17c07c1f, 0x1b00001f, 0x00000000, 0x1b80001f, 0x90100000,
    0x1890001f, 0x10006400, 0x80868801, 0xd8202922, 0x17c07c1f, 0x1b00001f,
    0x3fffe7ff, 0x1b80001f, 0xd0100000, 0xd0003200, 0x17c07c1f, 0x1b00001f,
    0xffffffff, 0x8880000c, 0x3fffe7ff, 0xaa40000c, 0x00800000, 0xd8003202,
    0x17c07c1f, 0x8880000c, 0x40000000, 0xd8002922, 0x17c07c1f, 0xc28017e0,
    0x1290841f, 0x89c00007, 0xfffeffff, 0x1950001f, 0x10006b00, 0x80d90405,
    0xd8002c03, 0x17c07c1f, 0xc0c01720, 0x17c07c1f, 0x1950001f, 0x10006b00,
    0x80d98405, 0xd8002f63, 0x17c07c1f, 0xc0c013e0, 0x17c07c1f, 0xd8003103,
    0x17c07c1f, 0x1950001f, 0x10006b00, 0x80da0405, 0xd8002f63, 0x17c07c1f,
    0xa8000000, 0x00000004, 0xc28017e0, 0x1291041f, 0x1950001f, 0x10006b00,
    0x80dc8405, 0xd8202f63, 0x17c07c1f, 0x1b00001f, 0x00000000, 0x1b80001f,
    0x90100000, 0xe8208000, 0x10006310, 0x0b160038, 0xa9c00007, 0x00000200,
    0x1b00001f, 0x7fffe7ff, 0x1b80001f, 0x90100000, 0x1240301f, 0xe8208000,
    0x10006310, 0x0b160008, 0x88000000, 0xfffffffb, 0x89c00007, 0xfffffffd,
    0x89c00007, 0xfffbfeff, 0xa9c00007, 0x00010000, 0x1950001f, 0x10006400,
    0x80d70405, 0xd80037a3, 0x17c07c1f, 0x1950001f, 0x10006b00, 0x80d80405,
    0xd80034e3, 0x17c07c1f, 0x80d88405, 0xd80033e3, 0x17c07c1f, 0x88000000,
    0xfffdffff, 0x1b80001f, 0x20000300, 0x88000000, 0xff7ffdff, 0x88000000,
    0xfffffffd, 0x89c00007, 0xffffff7f, 0x18c0001f, 0x10006208, 0xc0c00d20,
    0x12807c1f, 0xe8208000, 0x10006248, 0x00000001, 0x1b80001f, 0x20000080,
    0xc0c00d20, 0x1280041f, 0x18c0001f, 0x10006200, 0xc0c00d20, 0x12807c1f,
    0xe8208000, 0x1000625c, 0x00000000, 0x1b80001f, 0x20000080, 0xc0c00d20,
    0x1280041f, 0xe8208000, 0x100063e0, 0x00000001, 0x19c0001f, 0x00015820,
    0x10007c1f, 0xc0c012e0, 0x17c07c1f, 0xe8208000, 0x10006b10, 0x00000000,
    0x1b00001f, 0x00202000, 0x1b80001f, 0x80001000, 0x8880000c, 0x00200000,
    0xd8003aa2, 0x17c07c1f, 0xe8208000, 0x100063e0, 0x00000002, 0x1b80001f,
    0x00001000, 0x809c840d, 0xd8203902, 0x17c07c1f, 0xa1d78407, 0x1890001f,
    0x10006014, 0x18c0001f, 0x10006014, 0xa0978402, 0xe0c00002, 0x1b80001f,
    0x00001000, 0xf0000000

};

#define PCM_SODI_BASE            __pa(spm_pcm_sodi)
#define PCM_SODI_LEN              ( 482)
#define SODI_pcm_pc_0      0
#define SODI_pcm_pc_1      12

#define PCM_SODI_VEC0        EVENT_VEC(WAKE_ID_AP_WAKE, 1, 0, SODI_pcm_pc_0)      /* MD-wake event */
#define PCM_SODI_VEC1        EVENT_VEC(WAKE_ID_AP_SLEEP, 1, 0, SODI_pcm_pc_1)      /* MD-sleep event */

s32 spm_sodi_disable_counter = 0;
extern spinlock_t spm_lock;

#ifndef FPGA_EARLY_PORTING  
#define WAKE_SRC_FOR_SODI \
    (WAKE_SRC_KP | WAKE_SRC_GPT | WAKE_SRC_EINT | WAKE_SRC_CCIF |      \
     WAKE_SRC_USB_CD | WAKE_SRC_USB_PDN | WAKE_SRC_AFE |                 \
     WAKE_SRC_SYSPWREQ | WAKE_SRC_MD_WDT | WAKE_SRC_CONN_WDT | WAKE_SRC_CONN | WAKE_SRC_THERM| WAKE_SRC_CPU0_IRQ)
#else
#define WAKE_SRC_FOR_SODI \
    (WAKE_SRC_KP | WAKE_SRC_GPT | WAKE_SRC_EINT | WAKE_SRC_CCIF |      \
     WAKE_SRC_USB_CD | WAKE_SRC_USB_PDN | WAKE_SRC_AFE |                 \
     WAKE_SRC_MD_WDT | WAKE_SRC_CONN_WDT | WAKE_SRC_CONN | WAKE_SRC_THERM| WAKE_SRC_CPU0_IRQ)
#endif
#define WAKE_SRC_FOR_MCDI                     \
        (WAKE_SRC_PCM_TIMER | WAKE_SRC_GPT | WAKE_SRC_THERM | WAKE_SRC_CPU1_IRQ|WAKE_SRC_SYSPWREQ )


SPM_PCM_CONFIG pcm_config_sodi={
    .scenario = SPM_PCM_SODI,
    .ver = SPM_SODI_FIRMWARE_VER,
    .spm_turn_off_26m = false,
    .pcm_firmware_addr =  PCM_SODI_BASE,
    .pcm_firmware_len = PCM_SODI_LEN,
    .pcm_pwrlevel = PWR_LVNA,
    .spm_request_uart_sleep = true,
    .sodi_en = false,
    .pcm_vsr = {PCM_SODI_VEC0,PCM_SODI_VEC1,0,0,0,0,0,0},

    //spm_write(SPM_AP_STANBY_CON, ((0x0<<WFI_OP) | (0x1<<WFI_L2C) | (0x1<<WFI_SCU)));  // operand or, mask l2c, mask scu 

    /*Wake up event mask*/
    .md_mask = MDCONN_UNMASK,   /* mask MD1 and MD2 */
#ifndef FPGA_EARLY_PORTING     
    .mm_mask = MFG_MASK,   /* mask DISP and MFG */
#else
    .mm_mask = MMALL_MASK,
#endif

    .sync_r0r7=true,

    /*AP Sleep event mask*/
    .wfi_scu_mask=false,  /* check SCU idle */
    .wfi_l2c_mask=false,  /* check L2C idle */
    .wfi_op=REDUCE_AND,
    .wfi_sel = {true,true},

    .timer_val_ms = 0*1000,
    .wake_src = WAKE_SRC_FOR_SODI,
    .infra_pdn = false,  /* keep INFRA/DDRPHY power */
    .cpu_pdn = true,
    .coclock_en = false,
    .lock_infra_dcm=true,
    .pcm_reserved = 0,

    /* debug related*/
    .reserved = 0x0,
    .dbg_wfi_cnt=0,
    .wakesta_idx=0
};

// ==============================================================================
DEFINE_SPINLOCK(spm_sodi_lock);

void spm_disable_sodi(void)
{
    spin_lock(&spm_sodi_lock);

    spm_sodi_disable_counter++;
    clc_debug("spm_disable_sodi() : spm_sodi_disable_counter = 0x%x\n", spm_sodi_disable_counter);    

    if(spm_sodi_disable_counter > 0)
    {
        spm_direct_disable_sodi();
    }

    spin_unlock(&spm_sodi_lock);
}

void spm_enable_sodi(void)
{
    spin_lock(&spm_sodi_lock);

    spm_sodi_disable_counter--;
    clc_debug("spm_enable_sodi() : spm_sodi_disable_counter = 0x%x\n", spm_sodi_disable_counter);    
    
    if(spm_sodi_disable_counter <= 0)
    {
        spm_direct_enable_sodi();
    }

    spin_unlock(&spm_sodi_lock);
}

bool spm_is_sodi_user_en(void)
{
    return pcm_config_sodi.sodi_en;
}

void spm_direct_disable_sodi(void)
{
    u32 clc_temp;

    clc_temp = spm_read(SPM_CLK_CON);
    clc_temp |= (0x1<<13);
    
    spm_write(SPM_CLK_CON, clc_temp);  
}

void spm_direct_enable_sodi(void)
{
    u32 clc_temp;

    clc_temp = spm_read(SPM_CLK_CON);
    clc_temp &= 0xffffdfff; // ~(0x1<<13);

    spm_write(SPM_CLK_CON, clc_temp);    
}


extern void spm_trigger_wfi_for_dpidle(bool cpu_pdn);
extern void spm_init_pcm_register(void);
extern SPM_PCM_SCENARIO spm_last_senario;
extern bool spm_change_fw;
extern void soidle_before_wfi(void);
extern void soidle_after_wfi(bool sodi_en);
extern void mcidle_before_wfi(int cpu);
extern void mcidle_after_wfi(int cpu);
extern void spm_mcdi_init_core_mux(void);

u32 power_on_val1 = 0;
/*
    status
    RESERVED[0] :  mcusys dormant 
    RESERVED[1] :  APSRCSLEEP - (pass CHECK_APSRCWAKE)
    RESERVED[2] :  EMI self-refresh & mem ck off
     
    Options
    RESERVED[16] :  skip arm pll cg/pd 
    RESERVED[17] :  skip arm pll pd 
    RESERVED[18] :  skip AXI 26 SWITCH 
    RESERVED[19] :  skip EMI self-refresh & mem ck off                         
    RESERVED[20] :  skip mem ck off
                                                                    
    RESERVED[24] :  stop after cpu power down / cpu pll off
    RESERVED[25] :  stop after emi  down / mem ck off

*/
wake_reason_t spm_go_to_sodi(bool sodi_en,int cpu)
{
    wake_status_t *wakesta;
    unsigned long flags;
    struct mtk_irq_mask mask;
    wake_reason_t wr = WR_NONE;
    u32 con0;

    spin_lock_irqsave(&spm_lock, flags);
    spm_stop_normal();    

    mt_irq_mask_all(&mask);
    mt_irq_unmask_for_sleep(MT_SPM0_IRQ_ID);    
    mt_cirq_clone_gic();
    mt_cirq_enable();
    spm_direct_enable_sodi();

    spm_write(SPM_PCM_RESERVE,pcm_config_sodi.pcm_reserved);

#ifdef SPM_SODI_SECONDARY_KICK_IMPROVE
    if( spm_last_senario != pcm_config_sodi.scenario)//MCDI SPM Initialize
#endif
    {
        //spm_direct_enable_sodi();
        if (spm_init_pcm(&pcm_config_sodi)==false)
            goto  RESTORE_IRQ;
        
        spm_write(SPM_SLEEP_CPU_IRQ_MASK,0x1); 

        spm_write(SPM_PCM_CON1,(spm_read(SPM_PCM_CON1)| CON1_CFG_KEY )&0xffffffdf);//temporary disable pcm timer

        spm_wfi_sel(pcm_config_sodi.wfi_sel, SPM_CORE0_WFI_SEL_SW_MASK );
        spm_wfi_sel(pcm_config_sodi.wfi_sel, SPM_CORE1_WFI_SEL_SW_MASK ); 

        spm_kick_pcm(&pcm_config_sodi);

        spm_change_fw = 1;
    }
#ifdef SPM_SODI_SECONDARY_KICK_IMPROVE
    else
    {
        spm_write(SPM_SLEEP_CPU_IRQ_MASK,0x1);
        spm_wfi_sel(pcm_config_sodi.wfi_sel, SPM_CORE0_WFI_SEL_SW_MASK );
        spm_wfi_sel(pcm_config_sodi.wfi_sel, SPM_CORE1_WFI_SEL_SW_MASK ); 
        if( spm_secondary_kick(&pcm_config_sodi)==false)
            goto RESTORE_IRQ;

        spm_change_fw = 0;
    }
#endif    

    //gpiodbg_infra_dbg_out();//measure bus clock to LPD15
    soidle_before_wfi();

    spm_trigger_wfi_for_dpidle(1);//SODI with cpu dormant

    soidle_after_wfi(sodi_en);

    wakesta = spm_get_wakeup_status(&pcm_config_sodi);

    wr = wakesta->wake_reason;

    spm_clean_after_wakeup();
      
    spm_write(SPM_PCM_CON1,(spm_read(SPM_PCM_CON1)| CON1_CFG_KEY )|(CON1_PCM_TIMER_EN)); 

RESTORE_IRQ:
    mt_cirq_flush();
    mt_cirq_disable();
    mt_irq_mask_restore(&mask);
    spm_go_to_normal();
    spin_unlock_irqrestore(&spm_lock, flags);
    return wr;

}


#if 0//for MCDI LDVT testcase
u32 spm_pre_SPM_PCM_SW_INT_SET=0;
u32 spm_post_SPM_PCM_SW_INT_SET=0;
u32 spm_pre_SPM_SLEEP_ISR_STATUS=0;
u32 spm_post_SPM_SLEEP_ISR_STATUS=0;
u32 spm_post_SPM_PCM_RESERVE = 0;

wake_reason_t spm_go_to_mcdi_ipi_test(int cpu)
{
    wake_status_t *wakesta;
    unsigned long flags;
    struct mtk_irq_mask mask;
    wake_reason_t wr = WR_NONE;
    u32 con0;
    bool sodi_en = 0;

    spin_lock_irqsave(&spm_lock, flags);
    //mt_irq_mask_all(&mask);
    //mt_irq_unmask_for_sleep(MT_SPM0_IRQ_ID);       
    
    if(cpu==0)//except mcdi
    {
        mt_irq_mask_all(&mask);
        mt_irq_unmask_for_sleep(MT_SPM0_IRQ_ID);
        
        //spm_wdt_config(false);
        
        mt_cirq_clone_gic();
        mt_cirq_enable();
    }

    pcm_config_sodi.spm_request_uart_sleep = true;
    pcm_config_sodi.wake_src=WAKE_SRC_FOR_SODI;


    if((sodi_en==1)&&(cpu==0))//SODI
    {
        spm_direct_enable_sodi();
        pcm_config_sodi.wfi_sel[0]=true;
        pcm_config_sodi.wfi_sel[1]=true;        
        pcm_config_sodi.wfi_scu_mask=false;
        pcm_config_sodi.wfi_l2c_mask=false;

        
        spm_write(SPM_PCM_RESERVE,spm_read(SPM_PCM_RESERVE)&~0x1);
    }
    else if((sodi_en==0)&&(cpu==0))//CPU Dormant
    {
        pcm_config_sodi.wfi_sel[0]=true;
        pcm_config_sodi.wfi_sel[1]=true;    
        pcm_config_sodi.wfi_scu_mask=false;
        pcm_config_sodi.wfi_l2c_mask=false;        
        spm_direct_disable_sodi();
        spm_write(SPM_PCM_RESERVE,spm_read(SPM_PCM_RESERVE)&~0x1);
    }
    else if((sodi_en==0)&&(cpu!=0))//MCDI
    {
        //pcm_config_sodi.wake_src=WAKE_SRC_FOR_MCDI;
        pcm_config_sodi.wake_src=0x0;//for test IPI only
        pcm_config_sodi.wfi_sel[0]=false;
        pcm_config_sodi.wfi_sel[1]=true;
        pcm_config_sodi.wfi_scu_mask=true;
        pcm_config_sodi.wfi_l2c_mask=true;        
        pcm_config_sodi.spm_request_uart_sleep = false;
        spm_direct_disable_sodi();
        spm_write(SPM_PCM_RESERVE,spm_read(SPM_PCM_RESERVE)|0x1);
    }
    else
    {
        printk("[SPM]Wrong para!!\n");
        goto RESTORE_IRQ;
    }

    if( spm_last_senario != pcm_config_sodi.scenario)//MCDI SPM Initialize
    {
        //spm_direct_enable_sodi();
        if (spm_init_pcm(&pcm_config_sodi)==false)
            goto  RESTORE_IRQ;

        if(cpu==1)//except mcdi
        {
            spm_write(SPM_SLEEP_CPU_IRQ_MASK,0x2); 

        spm_mcdi_init_core_mux();
        }
        else
            spm_write(SPM_SLEEP_CPU_IRQ_MASK,0x1);


        spm_write(SPM_PCM_CON1,(spm_read(SPM_PCM_CON1)| CON1_CFG_KEY )&0xffffffdf);//temporary disable pcm timer
        
        spm_wfi_sel(pcm_config_sodi.wfi_sel, SPM_CORE0_WFI_SEL_SW_MASK );
        spm_wfi_sel(pcm_config_sodi.wfi_sel, SPM_CORE1_WFI_SEL_SW_MASK ); 


        spm_kick_pcm(&pcm_config_sodi);

        //spm_wfi_sel(pcm_config_sodi.wfi_sel, SPM_CORE0_WFI_SEL_SW_MASK );
        //spm_wfi_sel(pcm_config_sodi.wfi_sel, SPM_CORE1_WFI_SEL_SW_MASK );
        spm_change_fw = 1;
    }
    else
    {
        if(cpu==1)//except mcdi
        {
            spm_write(SPM_SLEEP_CPU_IRQ_MASK,0x2); 

            //spm_mcdi_init_core_mux();
        }
        else
            spm_write(SPM_SLEEP_CPU_IRQ_MASK,0x1);

        spm_wfi_sel(pcm_config_sodi.wfi_sel, SPM_CORE0_WFI_SEL_SW_MASK );
        spm_wfi_sel(pcm_config_sodi.wfi_sel, SPM_CORE1_WFI_SEL_SW_MASK );     
        if( spm_secondary_kick(&pcm_config_sodi)==false)
            goto RESTORE_IRQ;

        spm_change_fw = 0;
    }
   

    if(cpu==0)
    {
        soidle_before_wfi();

        spm_trigger_wfi_for_dpidle(1);//SODI with cpu dormant

        soidle_after_wfi(sodi_en);
    }
    else
    {
    spm_pre_SPM_PCM_SW_INT_SET=spm_read(SPM_PCM_SW_INT_SET);

    spm_pre_SPM_SLEEP_ISR_STATUS=spm_read(SPM_SLEEP_ISR_STATUS);
    spm_write(SPM_PCM_SW_INT_CLEAR,0xf);

        //mcidle_before_wfi(cpu);
        if (!cpu_power_down(DORMANT_MODE)) 
        {
            switch_to_amp();  
            
            /* do not add code here */
            wfi_with_sync();
        }
        switch_to_smp(); 
        cpu_check_dormant_abort();
        //mcidle_after_wfi(cpu);

    spm_post_SPM_PCM_SW_INT_SET=spm_read(SPM_PCM_SW_INT_SET);

    spm_post_SPM_SLEEP_ISR_STATUS=spm_read(SPM_SLEEP_ISR_STATUS);  
    spm_post_SPM_PCM_RESERVE = spm_read(SPM_PCM_RESERVE );
    } 

#if 1
    wakesta = spm_get_wakeup_status(&pcm_config_sodi);

    wr = wakesta->wake_reason;
#endif
    spm_clean_after_wakeup();

    spm_write(SPM_PCM_CON1,(spm_read(SPM_PCM_CON1)| CON1_CFG_KEY )|(CON1_PCM_TIMER_EN)); 

RESTORE_IRQ:
    if(cpu==0)//except mcdi
    {
        mt_cirq_flush();
        mt_cirq_disable();
        mt_irq_mask_restore(&mask);
    }

    //mt_irq_mask_restore(&mask);
    spin_unlock_irqrestore(&spm_lock, flags);
    return wr;

}
#endif




