#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/suspend.h>
#include <linux/console.h>
#include <linux/xlog.h>
#include <linux/aee.h>
#include <linux/delay.h>

#include <mach/mt_sleep.h>
#include <mach/mt_spm.h>
#include <mach/mt_spm_api.h>
#include <mach/mt_spm_mtcmos.h>
#include <mach/mt_clkmgr.h>
#include <mach/mt_gpio.h>
#include <mach/mt_gpt.h>

/**************************************
 * only for internal debug
 **************************************/
#ifdef CONFIG_MTK_LDVT
#define SLP_SLEEP_DPIDLE_EN         1
#define SLP_REPLACE_DEF_WAKESRC     1
#define SLP_SUSPEND_LOG_EN          1
#else
#define SLP_SLEEP_DPIDLE_EN         0
#define SLP_REPLACE_DEF_WAKESRC     0
#define SLP_SUSPEND_LOG_EN          0
#endif


/**************************************
 * SW code for suspend
 **************************************/
#define slp_read(addr)              (*(volatile u32 *)(addr))
#define slp_write(addr, val)        (*(volatile u32 *)(addr) = (u32)(val))

#define slp_write_sync()            mb()

#if SLP_SUSPEND_LOG_EN
#define slp_xverb(fmt, args...)     \
    xlog_printk(ANDROID_LOG_INFO, "Power/Sleep", fmt, ##args)
#else
#define slp_xverb(fmt, args...)     \
    xlog_printk(ANDROID_LOG_VERBOSE, "Power/Sleep", fmt, ##args)
#endif

#define slp_xinfo(fmt, args...)     \
    xlog_printk(ANDROID_LOG_INFO, "Power/Sleep", fmt, ##args)

#define slp_xerror(fmt, args...)    \
    xlog_printk(ANDROID_LOG_ERROR, "Power/Sleep", fmt, ##args)

static DEFINE_SPINLOCK(slp_lock);

static wake_reason_t slp_wake_reason;

extern SPM_PCM_CONFIG pcm_config_suspend;

static bool slp_ck26m_on = false;

static bool slp_dump_gpio = 0;
static bool slp_dump_regs = 1;

/* Sleep Mode Test*/
static int slp_test_mode = 0; /*0: Legacy Mode, 1: Shui down mode*/
static int slp_min_time = 0;
static int slp_max_time = 0;

static int slp_suspend_ops_valid(suspend_state_t state)
{
    return state == PM_SUSPEND_MEM;
}

static int slp_suspend_ops_begin(suspend_state_t state)
{
    /* legacy log */
    slp_xinfo("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
    slp_xinfo("_Chip_pm_begin (%u)(%u) @@@@@@@@@@@@@@@\n", pcm_config_suspend.cpu_pdn,  pcm_config_suspend.infra_pdn);
    slp_xinfo(" @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");

    slp_wake_reason = WR_NONE;

    return 0;
}

static int slp_suspend_ops_prepare(void)
{
    /* legacy log */
    aee_sram_printk("_Chip_pm_prepare\n");
    slp_xinfo("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
    slp_xinfo("_Chip_pm_prepare @@@@@@@@@@@@@@@@@@@@\n");
    slp_xinfo(" @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");

    return 0;
}

//#if defined(LDVT_SPM_SUSPEND_STRESS_TEST) || defined(ADVT_SPM_SUSPEND_STRESS_TEST)
#if 1
unsigned int slp_abort_cnt=0,slp_normal_cnt=0,slp_abnormal_cnt=0, slp_test_cnt;
int slp_test_times = 20000;
wake_status_t* slp_wakeup_status;
PCM_DBG_REG  *pcm_dbg_reg;
extern SPM_PCM_CONFIG pcm_config_suspend;
#endif

extern void gpiodbg_emi_dbg_out(void);

static int slp_suspend_ops_enter(suspend_state_t state)
{
    const char *result;
    int timer_val_ms=1;
    bool gpt_err;

    /* check MM & MFG MTCMOS status */
    if (   PWR_ON == subsys_is_on(SYS_DIS)
        || PWR_ON == subsys_is_on(SYS_MFG)
        )
    {
        slp_xerror("WARNING: MMSYS_CG_CON0 = 0x%08X, MMSYS_CG_CON1 = 0x%08X, MFG_CG_CON = 0x%08X\n", spm_read(MMSYS_CG_CON0), spm_read(MMSYS_CG_CON1), spm_read(MFG_CG_CON));
    }

    /* legacy log */
    aee_sram_printk("_Chip_pm_enter\n");
    slp_xinfo("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
    slp_xinfo("_Chip_pm_enter @@@@@@@@@@@@@@@@@@@@@@\n");
    slp_xinfo(" @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");

     if (slp_dump_gpio)
        gpio_dump_regs();

    //if (slp_dump_regs)
        spm_dump_pll_regs();

    if (pcm_config_suspend.cpu_pdn && !spm_cpusys_can_power_down()) {
        slp_xerror("!!! CANNOT POWER DOWN CPU0/CPUSYS DUE TO CPUx ON !!!\n");
    }

    /*Output emi clock to check if PLL is off when suspend(output to LP09)*/
    //gpiodbg_emi_dbg_out();

    

#if defined( LDVT_SPM_SUSPEND_STRESS_TEST)//NOTE, in stress mode the timer tick is 32K NOT 1ms
    for(slp_test_cnt=0;;slp_test_cnt++)
    {
     pcm_config_suspend.timer_val_ms=timer_val_ms;
     pcm_config_suspend.spm_timer_opt = SPM_USE_GPT4_TIMER;
     
     if(timer_val_ms >= 100000)
          timer_val_ms=1;
     else
          timer_val_ms+=100;
          
     mdelay(1000);
     
#endif

    #if defined(LDVT_SPM_SUSPEND_WDT_TEST)
    pcm_config_suspend.timer_val_ms=10000;
    pcm_config_suspend.wdt_val_ms=5000;
    spm_wakesrc_set(SPM_PCM_KERNEL_SUSPEND,WAKE_ID_KP);
    spm_write(0xF0007034 , (spm_read(0xF0007034) &(~0x03))|0x44000000);//Force RGU to reset mode
    #endif
    
    slp_wake_reason = spm_go_to_sleep();
    slp_wakeup_status= spm_get_last_wakeup_status();
    pcm_dbg_reg= &slp_wakeup_status->pcm_dbg_reg;
    result = spm_get_wake_up_result(SPM_PCM_KERNEL_SUSPEND);
    slp_xinfo("Wakeup Succefully",slp_wake_reason);
    slp_xinfo("%s",result);

#if 0
    if(pcm_dbg_reg->PCM_EVENT_REG_STA & PCM_EVENT_ABORT)
        slp_abort_cnt++;
    else if(pcm_dbg_reg->PCM_EVENT_REG_STA & PCM_EVENT_NORMAL)
        slp_normal_cnt++;
    else
        slp_abnormal_cnt++;
    
    slp_xinfo("slp_abort_cnt:%d,slp_normal_cnt:%d,slp_abnormal_cnt:%d",slp_abort_cnt,slp_normal_cnt,slp_abnormal_cnt);
#endif    

#if defined( LDVT_SPM_SUSPEND_STRESS_TEST)
     }
#endif
/*
#elif defined(LDVT_SPM_SUSPEND_WDT_TEST)
    pcm_config_suspend.timer_val_ms=10240;
    pcm_config_suspend.wdt_val_ms=3000;
    spm_wakesrc_set(SPM_PCM_KERNEL_SUSPEND,WAKE_ID_KP);
    spm_write(0xF0007034 , (spm_read(0xF0007034) &(~0x03))|0x44000000);//Force RGU to reset mode

#elif defined(ADVT_SPM_SUSPEND_STRESS_TEST)
    pcm_config_suspend.timer_val_ms+=1;
    if(pcm_config_suspend.timer_val_ms >= 10)
      pcm_config_suspend.timer_val_ms=1;
    
        if(slp_wake_reason == WR_PCM_SLEEP_ABORT)
            slp_abort_cnt++;
        else if(slp_wake_reason == WR_WAKE_SRC)
            slp_normal_cnt++;
        else
            slp_abnormal_cnt++;

    slp_xinfo("slp_test_cnt:%d,timer:%d",slp_test_cnt,pcm_config_suspend.timer_val_ms);
    slp_xinfo("slp_abort_cnt:%d,slp_normal_cnt:%d,slp_abnormal_cnt:%d",slp_abort_cnt,slp_normal_cnt,slp_abnormal_cnt);
#endif

  // If Fuel Guage is enabled
  if(pcm_config_suspend.reserved & SPM_SUSPEND_GET_FGUAGE)
      pcm_config_suspend.timer_val_ms = spm_get_wake_period(WR_NONE)*1000;

#if 0
    spm_wdt_config(true);
    while(1)
    {
        mdelay(4000);
        slp_xinfo("Kick Dog!!");
        spm_wdt_restart();
    }
#endif

    slp_wake_reason = spm_go_to_sleep();
    slp_wakeup_status= spm_get_last_wakeup_status();
    result = spm_get_wake_up_result(SPM_PCM_KERNEL_SUSPEND);
    slp_xinfo("Wakeup Succefully");
    slp_xinfo("%s",result);
    if(slp_wake_reason != WR_WAKE_SRC && slp_wake_reason != WR_PCM_TIMER){
       slp_abort_cnt++;
     }
    else{
       slp_normal_cnt++;
     }

     slp_xinfo("slp_abort_cnt:%d,slp_normal_cnt:%d",slp_abort_cnt,slp_normal_cnt);
     slp_xinfo("slp_wake_reason: %d",slp_wake_reason);*/
    return 0;
}

static void slp_suspend_ops_finish(void)
{
    /* legacy log */
    aee_sram_printk("_Chip_pm_finish\n");
    slp_xinfo("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
    slp_xinfo("_Chip_pm_finish @@@@@@@@@@@@@@@@@@@@@\n");
    slp_xinfo(" @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");

    /* debug help */
    //slp_xinfo("Battery_Voltage = %lu\n", BAT_Get_Battery_Voltage(0));
}

static void slp_suspend_ops_end(void)
{
    /* legacy log */
    slp_xinfo("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
    slp_xinfo("_Chip_pm_end @@@@@@@@@@@@@@@@@@@@@@@@\n");
    slp_xinfo(" @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
}

static struct platform_suspend_ops slp_suspend_ops = {
    .valid      = slp_suspend_ops_valid,
    .begin      = slp_suspend_ops_begin,
    .prepare    = slp_suspend_ops_prepare,
    .enter      = slp_suspend_ops_enter,
    .finish     = slp_suspend_ops_finish,
    .end        = slp_suspend_ops_end,
};

wake_reason_t slp_get_wake_reason(void)
{
    return slp_wake_reason;
}

bool slp_will_infra_pdn(void)
{
    return pcm_config_suspend.infra_pdn;
}


void slp_module_init(void)
{
   // spm_output_sleep_option();

    slp_xinfo("SLEEP_DPIDLE_EN:%d, REPLACE_DEF_WAKESRC:%d, SUSPEND_LOG_EN:%d\n",
              SLP_SLEEP_DPIDLE_EN, SLP_REPLACE_DEF_WAKESRC, SLP_SUSPEND_LOG_EN);

    suspend_set_ops(&slp_suspend_ops);

//#if SLP_SUSPEND_LOG_EN
    console_suspend_enabled = 0;
//#endif
}


module_param(slp_dump_gpio, bool, 0644);
module_param(slp_dump_regs, bool, 0644);

MODULE_AUTHOR("Terry Chang <terry.chang@mediatek.com>");
MODULE_DESCRIPTION("MT65xx Sleep Driver v0.1");
