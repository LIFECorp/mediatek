#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/delay.h>
#include <linux/string.h>
#include <linux/aee.h>
#include <linux/interrupt.h>

#include <mach/irqs.h>
#include <mach/mt_spm.h>
#include <mach/mt_clkmgr.h>
#include <mach/mt_dcm.h>
#include <mach/mt_dormant.h>
#include <mach/eint.h>
#include <mach/mtk_ccci_helper.h>
#include <board-custom.h>

#define SPM_NORMAL_FIRMWARE_VER   "pcm_normal_20131015A"
 
static const u32 spm_pcm_normal[] = {
    0x1840001f, 0x00000001, 0x1b00001f, 0x00202000, 0x1b80001f, 0x80001000,
    0x8880000c, 0x00200000, 0xd80001e2, 0x17c07c1f, 0xe8208000, 0x100063e0,
    0x00000002, 0x1b80001f, 0x00001000, 0x809c840d, 0xd8200042, 0x17c07c1f,
    0xa1d78407, 0x1890001f, 0x10006014, 0x18c0001f, 0x10006014, 0xa0978402,
    0xe0c00002, 0x1b80001f, 0x00001000, 0xf0000000
};

#define PCM_NORMAL_BASE        __pa(spm_pcm_normal)
#define PCM_NORMAL_LEN         (28)                       /* # words */

#define NORMAL_PCM_TIMER_VAL   (0)
#define NORMAL_PCM_WDT_VAL     (0) //need larger than PCM Timer

#define WAKE_SRC_FOR_NORMAL  WAKE_SRC_THERM

SPM_PCM_CONFIG pcm_config_normal ={
    .scenario = SPM_PCM_NORMAL,
    .ver = SPM_NORMAL_FIRMWARE_VER,
    .spm_turn_off_26m = false,
    .pcm_firmware_addr =  PCM_NORMAL_BASE,
    .pcm_firmware_len = PCM_NORMAL_LEN,
    .pcm_pwrlevel = PWR_LVNA,         //no necessary to set pwr level when suspend
    .spm_request_uart_sleep = false,
    .pcm_vsr = {0,0,0,0,0,0,0,0},
    
    .sync_r0r7=false,
    
    /*Wake up event mask*/
    .md_mask = CONN_MASK,   /* unmask MD1 and MD2 */
    .mm_mask = MMALL_MASK,   /* unmask DISP and MFG */
    
    /*AP Sleep event mask*/
    .wfi_scu_mask=false ,   
    .wfi_l2c_mask=false, 
    .wfi_op=REDUCE_OR,//We need to ignore CPU 1 in FPGA platform
    .wfi_sel = {false,false},
    .timer_val_ms = NORMAL_PCM_TIMER_VAL,
    .wdt_val_ms = NORMAL_PCM_TIMER_VAL,
    .wake_src = WAKE_SRC_FOR_NORMAL,
    .reserved = 0x0,
    
    .cpu_pdn = false,
    .infra_pdn = false,
    .coclock_en = false,
    .lock_infra_dcm=false,
    
    /* debug related*/
    .dbg_wfi_cnt=0,
    .wakesta_idx=0
     };

/***********************Exposed API*************************/
     
wake_reason_t spm_go_to_normal(void)
{
#ifndef CONFIG_KICK_SPM_WDT
    unsigned long flags;
    static wake_reason_t last_wr = WR_NONE;
    
    if (spm_init_pcm(&pcm_config_normal)==false)
      goto  ERROR;
    
    spm_kick_pcm(&pcm_config_normal);
        
ERROR:

    return last_wr;
#endif
}

wake_reason_t spm_stop_normal(void)
{   
    #ifndef CONFIG_KICK_SPM_WDT
        spm_write(SPM_PCM_SW_INT_CLEAR,0xf);
        spm_write(SPM_PCM_CON1, (spm_read(SPM_PCM_CON1) & ~CON1_PCM_WDT_EN) | CON1_CFG_KEY);
    #endif
}
