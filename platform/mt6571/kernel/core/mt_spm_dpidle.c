#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/delay.h>
#include <linux/string.h>
#include <linux/aee.h>

#include <mach/irqs.h>
#include <mach/mt_cirq.h>
#include <mach/mt_spm.h>
#include <mach/mt_spm_pcm.h>
#include <mach/mt_clkmgr.h>
#include <mach/mt_dcm.h>
#include <mach/mt_dormant.h>
//#include <mach/eint.h>
#include <mach/mtk_ccci_helper.h>
#include <board-custom.h>


//#define SPM_DPIDLE_CLK_DBG_OUT
//#define SPM_DPIDLE_SECONDARY_KICK_IMPROVE

extern void spm_dpidle_before_wfi(void);        /* can be redefined */
extern void spm_dpidle_after_wfi(void);         /* can be redefined */
#ifdef SPM_DPIDLE_CLK_DBG_OUT
extern void gpiodbg_emi_dbg_out(void);
extern void gpiodbg_armcore_dbg_out(void);
#endif

#define SPM_DPIDLE_FIRMWARE_VER "2013-09-10B"
 
static const u32 spm_pcm_dpidle[] = {
    0x80c01801, 0xd82000a3, 0x17c07c1f, 0x1800001f, 0x038e0e1e, 0xd8000123,
    0x17c07c1f, 0x1800001f, 0x038e0e16, 0xc0c016e0, 0x10c0041f, 0x1b00001f,
    0x7ffff7ff, 0xf0000000, 0x17c07c1f, 0x1b00001f, 0x3fffe7ff, 0x1b80001f,
    0x20000004, 0xd800044c, 0x17c07c1f, 0xc0c016e0, 0x10c07c1f, 0xc2801ac0,
    0x1291041f, 0x1800001f, 0x038e0e36, 0x80c01801, 0xd8200403, 0x17c07c1f,
    0x1800001f, 0x038e0e3e, 0x1b00001f, 0x3fffefff, 0xf0000000, 0x17c07c1f,
    0x18c0001f, 0x10006240, 0x1200041f, 0xc0c00fc0, 0x17c07c1f, 0x19c0001f,
    0x001c4ba7, 0x1b80001f, 0x20000030, 0x1800001f, 0x038a0e16, 0x1b80001f,
    0x20000300, 0x1800001f, 0x028a0e16, 0x1800001f, 0x028a0a16, 0x1800001f,
    0x028a0a12, 0x19c0001f, 0x00144ba7, 0x19c0001f, 0x00104ba5, 0x1b80001f,
    0x20000030, 0xe8208000, 0x10006354, 0xfffff806, 0x19c0001f, 0x00114aa5,
    0x1b00001f, 0xbfffe7ff, 0xf0000000, 0x17c07c1f, 0x1b80001f, 0x20000fdf,
    0x1890001f, 0x10006608, 0x80c98801, 0x810a0801, 0x10928c1f, 0xa0911002,
    0x8080080d, 0x1b00001f, 0xbfffe7ff, 0xd8000f82, 0x17c07c1f, 0x1b00001f,
    0x3fffe7ff, 0x1b80001f, 0x20000004, 0xd8000f8c, 0x17c07c1f, 0xe8208000,
    0x10006354, 0xffffff97, 0x19c0001f, 0x00184ba5, 0x19c0001f, 0x001c4ba5,
    0x1b80001f, 0x20000008, 0x1880001f, 0x10006320, 0xc0c015c0, 0xe080000f,
    0xd8000f83, 0x17c07c1f, 0xe080001f, 0x18c0001f, 0x10006240, 0x1200041f,
    0xc0c01180, 0x17c07c1f, 0x1800001f, 0x028a0a16, 0x1800001f, 0x028a0e16,
    0x1800001f, 0x038a0e16, 0x1800001f, 0x038e0e16, 0xc0c018a0, 0x10c0041f,
    0xc2801ac0, 0x1290841f, 0x1b00001f, 0x7ffff7ff, 0xf0000000, 0x17c07c1f,
    0xe0e00f16, 0x1380201f, 0xe0e00f1e, 0x1380201f, 0xe0e00f0e, 0x1380201f,
    0xe0e00f0c, 0xe0e00f0d, 0xe0e00e0d, 0xe0e00c0d, 0xe0e0080d, 0xe0e0000d,
    0xf0000000, 0x17c07c1f, 0xe0e00f0d, 0xe0e00f1e, 0xe0e00f12, 0xf0000000,
    0x17c07c1f, 0xd800132a, 0x17c07c1f, 0xe2e00036, 0x1380201f, 0xe2e0003e,
    0x1380201f, 0xe2e0002e, 0x1380201f, 0xd820142a, 0x17c07c1f, 0xe2e0006e,
    0xe2e0004e, 0xe2e0004c, 0x1b80001f, 0x20000020, 0xe2e0004d, 0xf0000000,
    0x17c07c1f, 0xd80014ea, 0x17c07c1f, 0xe2e0006d, 0xe2e0002d, 0xd820158a,
    0x17c07c1f, 0xe2e0002f, 0xe2e0003e, 0xe2e00032, 0xf0000000, 0x17c07c1f,
    0xa1d08407, 0x1b80001f, 0x20000080, 0x80eab401, 0x1a00001f, 0x10006814,
    0xe2000003, 0xf0000000, 0x17c07c1f, 0x1a00001f, 0x10006604, 0xd8001803,
    0x17c07c1f, 0xe2200002, 0x1b80001f, 0x20000020, 0xd8201863, 0x17c07c1f,
    0xe2200001, 0x1b80001f, 0x20000020, 0xf0000000, 0x17c07c1f, 0xd80019a3,
    0x17c07c1f, 0x1a10001f, 0x11004058, 0x1a80001f, 0x11004058, 0xd8201a23,
    0x17c07c1f, 0x1a10001f, 0x1100406c, 0x1a80001f, 0x1100406c, 0xaa000008,
    0x80000000, 0xe2800008, 0xf0000000, 0x17c07c1f, 0x18d0001f, 0x10006b00,
    0xa1002803, 0x18c0001f, 0x10006b00, 0xe0c00004, 0xf0000000, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x1840001f, 0x00000001,
    0xa1d48407, 0xe8208000, 0x10006b00, 0x00000000, 0x1990001f, 0x10006600,
    0x1b00001f, 0x3fffe7ff, 0x1b80001f, 0xd00f0000, 0x8880000c, 0x3fffe7ff,
    0xd80032a2, 0x1140041f, 0xe8208000, 0x10006354, 0xfffff806, 0xc0c03420,
    0x17c07c1f, 0x1950001f, 0x10006400, 0x80d70405, 0xd8002603, 0x17c07c1f,
    0x89c00007, 0xffffefff, 0x18c0001f, 0x10006200, 0xc0c01460, 0x12807c1f,
    0xe8208000, 0x1000625c, 0x00000001, 0xc0c01460, 0x1280041f, 0x18c0001f,
    0x10006208, 0xc0c01460, 0x12807c1f, 0xe8208000, 0x10006248, 0x00000000,
    0xc0c01460, 0x1280041f, 0xc2801ac0, 0x1290041f, 0x19c0001f, 0x00014aa5,
    0x1b80001f, 0x20000030, 0x1800001f, 0x00000012, 0x1800001f, 0x00000a12,
    0x1800001f, 0x000a0a12, 0x1800001f, 0x028a0a12, 0xc0c018a0, 0x10c07c1f,
    0xe8208000, 0x10006310, 0x0b1600f8, 0x1b00001f, 0xbfffe7ff, 0x1b80001f,
    0x90100000, 0x80881c01, 0xd8002d02, 0x17c07c1f, 0x1800001f, 0x038e0e16,
    0xc0c016e0, 0x10c0041f, 0x18c0001f, 0x10006240, 0xc0c00fc0, 0x1200041f,
    0x1800001f, 0x038e0e16, 0x1800001f, 0x03800e16, 0x1b80001f, 0x20000300,
    0x1800001f, 0x00000e16, 0x1800001f, 0x00000016, 0x10007c1f, 0x19c0001f,
    0x00144ba7, 0x19c0001f, 0x00104ba5, 0x1b80001f, 0x20000030, 0xe8208000,
    0x10006354, 0xfffff806, 0x19c0001f, 0x00114aa5, 0xd0002e20, 0x17c07c1f,
    0x1800001f, 0x02800a12, 0x1b80001f, 0x20000300, 0x1800001f, 0x00000a12,
    0x1800001f, 0x00000012, 0x10007c1f, 0x19c0001f, 0x00014a25, 0x80d70405,
    0xd80032a3, 0x17c07c1f, 0x18c0001f, 0x10006208, 0x1212841f, 0xc0c01220,
    0x12807c1f, 0xe8208000, 0x10006248, 0x00000001, 0x1890001f, 0x10006248,
    0x81040801, 0xd8202fc4, 0x17c07c1f, 0x1b80001f, 0x20000020, 0xc0c01220,
    0x1280041f, 0x18c0001f, 0x10006200, 0xc0c01220, 0x12807c1f, 0xe8208000,
    0x1000625c, 0x00000000, 0x1890001f, 0x1000625c, 0x81040801, 0xd80031c4,
    0x17c07c1f, 0xc0c01220, 0x1280041f, 0x19c0001f, 0x00015820, 0x10007c1f,
    0x80cab001, 0x808cb401, 0x80800c02, 0xd82033c2, 0x17c07c1f, 0xa1d78407,
    0x1ac0001f, 0x55aa55aa, 0xf0000000, 0xa1d10407, 0x1b80001f, 0x20000020,
    0x10c07c1f, 0xf0000000, 0x17c07c1f

};
#define PCM_DPIDLE_BASE         __pa(spm_pcm_dpidle)
#define PCM_DPIDLE_LEN          (423)
#define PCM_DPIDLE_VEC0         EVENT_VEC(WAKE_ID_26M_WAKE, 1, 0, 0)      /* MD-wake event */
#define PCM_DPIDLE_VEC1         EVENT_VEC(WAKE_ID_26M_SLP, 1, 0, 15)     /* MD-sleep event */
#define PCM_DPIDLE_VEC2         EVENT_VEC(WAKE_ID_AP_WAKE, 1, 0, 36)      /* MD-wake event */
#define PCM_DPIDLE_VEC3         EVENT_VEC(WAKE_ID_AP_SLEEP, 1, 0, 70)     /* MD-sleep event */


#define WAKE_SRC_FOR_DPIDLE                                                 \
    (WAKE_SRC_KP | WAKE_SRC_GPT | WAKE_SRC_EINT | WAKE_SRC_CCIF |      \
     WAKE_SRC_USB_CD | WAKE_SRC_USB_PDN | WAKE_SRC_AFE |                 \
     WAKE_SRC_MD_WDT | WAKE_SRC_CONN_WDT | WAKE_SRC_CONN | WAKE_SRC_THERM)


SPM_PCM_CONFIG pcm_config_dpidle={
    .scenario = SPM_PCM_DEEP_IDLE,
    .ver = SPM_DPIDLE_FIRMWARE_VER,
	.spm_turn_off_26m = false,
	.pcm_firmware_addr =  PCM_DPIDLE_BASE,
	.pcm_firmware_len = PCM_DPIDLE_LEN,
	.pcm_pwrlevel = PWR_LV0,
	.spm_request_uart_sleep = true,
    .pcm_vsr = {PCM_DPIDLE_VEC0,PCM_DPIDLE_VEC1,PCM_DPIDLE_VEC2,PCM_DPIDLE_VEC3,0,0,0,0},
    
    /*Wake up event mask*/
    .md_mask = MDCONN_UNMASK,   /* unmask MD1 and MD2 */
#ifndef FPGA_EARLY_PORTING     
    .mm_mask = MMALL_UNMASK,   /* unmask DISP and MFG */
#else
    .mm_mask = MMALL_MASK,
#endif 
    
    .sync_r0r7=true,
    
    /*AP Sleep event mask*/
    .wfi_scu_mask=false,  /* check SCU idle */
	.wfi_l2c_mask=false,  /* check L2C idle */
	.wfi_op=REDUCE_AND,
	.wfi_sel = {true,true},
    .timer_val_ms = 0,
    .wake_src = WAKE_SRC_FOR_DPIDLE,
    .infra_pdn = false,  /* keep INFRA/DDRPHY power */
    .reserved = 0x0,
    .coclock_en = false,
    .lock_infra_dcm=true,
    
    /* debug related*/
    .dbg_wfi_cnt=0,
    .wakesta_idx=0
	 };
//extern bool spm_change_fw;
extern SPM_PCM_SCENARIO spm_last_senario;
extern int spm_request_uart_to_sleep(void);

wake_reason_t spm_go_to_dpidle(bool cpu_pdn, u8 pwrlevel)
{
    wake_status_t *wakesta;
    unsigned long flags;
    struct mtk_irq_mask mask;
    wake_reason_t wr = WR_NONE;
    u32 con0;
    u32 top_ckgen_val = 0;

    spin_lock_irqsave(&spm_lock, flags);
    spm_stop_normal();
    mt_irq_mask_all(&mask);
    mt_irq_unmask_for_sleep(MT_SPM0_IRQ_ID);
    mt_cirq_clone_gic();
    mt_cirq_enable();

#ifdef SPM_DPIDLE_SECONDARY_KICK_IMPROVE
    if( spm_last_senario != pcm_config_dpidle.scenario)//dpidle SPM Initialize
#endif    
    {   
        if(pwrlevel>2) 
        {
            spm_crit2("Hey!! wrong PWR Level: %x",pwrlevel);//ASSERT Wrong Para!!
            goto RESTORE_IRQ;
        }
        pcm_config_dpidle.pcm_pwrlevel = 1 << pwrlevel ;
        pcm_config_dpidle.spm_request_uart_sleep = (pwrlevel == 0 ? true : false);
        pcm_config_dpidle.cpu_pdn = cpu_pdn;

        if (spm_init_pcm(&pcm_config_dpidle)==false)
            goto  RESTORE_IRQ;

#ifdef SPM_CLOCK_INIT
        enable_clock(MT_CG_MEMSLP_DLYER_SW_CG, "SPM_DPIDLE");
        enable_clock(MT_CG_SPM_SW_CG, "SPM_DPIDLE");
#endif

#ifdef SPM_DPIDLE_CLK_DBG_OUT
        //gpiodbg_monitor();
        gpiodbg_emi_dbg_out();
        gpiodbg_armcore_dbg_out();
#endif

        spm_kick_pcm(&pcm_config_dpidle);
        //spm_change_fw = 1;

    }
#ifdef SPM_DPIDLE_SECONDARY_KICK_IMPROVE    
    else       
    {
        if( spm_secondary_kick(&pcm_config_dpidle)==false)
            goto RESTORE_IRQ;

        //spm_change_fw = 0;
    }  
#endif     
        
    spm_dpidle_before_wfi();

    //hopping 33M-->fixed 26M, 20130902 HG.Wei
    top_ckgen_val = spm_read(TOPCKGEN_BASE);
    spm_write(TOPCKGEN_BASE,top_ckgen_val&(~(1<<26)));
        
    snapshot_golden_setting(__FUNCTION__, __LINE__);

    spm_trigger_wfi(&pcm_config_dpidle);

    //hopping fixed 26M-->hopping 33M, 20130902 HG.Wei
    spm_write(TOPCKGEN_BASE,top_ckgen_val);

    spm_dpidle_after_wfi();

    wakesta = spm_get_wakeup_status(&pcm_config_dpidle);

    wr = wakesta->wake_reason;

    spm_clean_after_wakeup();

#ifdef SPM_CLOCK_INIT
    disable_clock(MT_CG_SPM_SW_CG, "SPM_DPIDLE");
    disable_clock(MT_CG_MEMSLP_DLYER_SW_CG, "SPM_DPIDLE");
#endif

RESTORE_IRQ:
    mt_cirq_flush();
    mt_cirq_disable();
    mt_irq_mask_restore(&mask);
    spm_go_to_normal();
    spin_unlock_irqrestore(&spm_lock, flags);
    return wr;
}
