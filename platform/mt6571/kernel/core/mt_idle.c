#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/cpu.h>

#include <linux/types.h>
#include <linux/string.h>
#include <mach/mt_cirq.h>
#include <asm/system_misc.h>
#include <mach/mt_spm.h>

#include <mach/mt_typedefs.h>
#include <mach/sync_write.h>
#include <mach/mt_idle.h>
#include <mach/mt_clkmgr.h>
#include <mach/mt_dcm.h>
#include <mach/mt_gpt.h>
#include <mach/mt_spm_api.h>
#include <mach/mt_spm_pcm.h>
#include <mach/hotplug.h>
#include <mach/mt_clkmgr.h>


#define USING_XLOG

#define SPM_SUSPEND_GPT_EN

#ifdef USING_XLOG 
#include <linux/xlog.h>

#define TAG     "Power/swap"

#define idle_err(fmt, args...)       \
    xlog_printk(ANDROID_LOG_ERROR, TAG, fmt, ##args)
#define idle_warn(fmt, args...)      \
    xlog_printk(ANDROID_LOG_WARN, TAG, fmt, ##args)
#define idle_info(fmt, args...)      \
    xlog_printk(ANDROID_LOG_INFO, TAG, fmt, ##args)
#define idle_dbg(fmt, args...)       \
    xlog_printk(ANDROID_LOG_DEBUG, TAG, fmt, ##args)
#define idle_ver(fmt, args...)       \
    xlog_printk(ANDROID_LOG_VERBOSE, TAG, fmt, ##args)

#else /* !USING_XLOG */
#define TAG     "[Power/swap] "

#define idle_err(fmt, args...)       \
    printk(KERN_ERR TAG);           \
    printk(KERN_CONT fmt, ##args) 
#define idle_warn(fmt, args...)      \
    printk(KERN_WARNING TAG);       \
    printk(KERN_CONT fmt, ##args)
#define idle_info(fmt, args...)      \
    printk(KERN_NOTICE TAG);        \
    printk(KERN_CONT fmt, ##args)
#define idle_dbg(fmt, args...)       \
    printk(KERN_INFO TAG);          \
    printk(KERN_CONT fmt, ##args)
#define idle_ver(fmt, args...)       \
    printk(KERN_DEBUG TAG);         \
    printk(KERN_CONT fmt, ##args)

#endif


#define INVALID_GRP_ID(grp) (grp < 0 || grp >= NR_GRPS)


extern unsigned long localtimer_get_counter(void);
extern int localtimer_set_next_event(unsigned long evt);


bool __attribute__((weak)) 
clkmgr_idle_can_enter(unsigned int *condition_mask, unsigned int *block_mask)
{
    return false;
}

static long int idle_get_current_time_ms(void)
{
    struct timeval t;
    do_gettimeofday(&t);
    return ((t.tv_sec & 0xFFF) * 1000000 + t.tv_usec) / 1000;
}


enum {
    BY_CPU = 0,
    BY_CLK = 1,
    BY_TMR = 2,
    BY_OTH = 3,
    NR_REASONS = 4,
};

static const char *idle_name[NR_TYPES] = {
    "dpidle",
    "mcidle",
    "soidle",
    "rgidle",
};

static const char *reason_name[NR_REASONS] = {
    "by_cpu",
    "by_clk",
    "by_tmr",
    "by_oth",
};

static int idle_switch[NR_TYPES] = {
    1,  //dpidle switch
    0,  //mcidle switch 
    1,  //soidle switch
    1,  //rgidle switch
};

/************************************************
 * Screen On idle part
 ************************************************/
static unsigned int soidle_condition_mask[NR_GRPS]= 
{
    //CG_MIXED
    0x0,

    //CG_MPLL
    0x0,

    //CG_UPLL
    0x0,

    //CG_CTRL0
    MFG_MM_SW_CG_BIT|
    SPM_52M_SW_CG_BIT|
    DBI_BCLK_SW_CG_BIT,

    //CG_CTRL1
    EFUSE_SW_CG_BIT|
    //THEM_SW_CG_BIT|// //Thermal no need check from Arvin(no access dram)
    APDMA_SW_CG_BIT|//TY no check, but check for UART0/1 from Lomen
    I2C0_SW_CG_BIT|
    I2C1_SW_CG_BIT|
    NFI_SW_CG_BIT|
    PWM_SW_CG_BIT|
    //UART0_SW_CG_BIT|
    //UART1_SW_CG_BIT|
    BTIF_SW_CG_BIT|
    USB_SW_CG_BIT|
    SPINFI_SW_CG_BIT|
    //MSDC0_SW_CG_BIT|
    //MSDC1_SW_CG_BIT|
    NFI2X_SW_CG_BIT|
    SPI_SW_CG_BIT,

    //CG_CTRL7
    NFI_HCLK_SW_CG_BIT,//71 add from YY

    //CG_MMSYS0
    DISP_FAKE_ENG_SW_CG_BIT|
    MM_CODEC_SW_CG_BIT|
    MM_CAMTG_SW_CG_BIT|
    MM_SENINF_SW_CG_BIT|
    MM_CAM_SMI_SW_CG_BIT|
    MM_CAM_DP2_SW_CG_BIT|
    MM_CAM_DP_SW_CG_BIT|
    MDP_RDMA_SW_CG_BIT|
    MDP_RSZ_SW_CG_BIT|
    MDP_WROT_SW_CG_BIT|
    MDP_SHP_SW_CG_BIT|
    MM_CMDQ_SMI_SW_CG_BIT|
    MM_CMDQ_ENGINE_SW_CG_BIT|
    SMI_LARB0_SW_CG_BIT|
    SMI_COMMON_SW_CG_BIT,

    //CG_MMSYS1
    DISP_DBI_IF_SW_CG_BIT|
    DISP_DBI_ENGINE_SW_CG_BIT|
    DISP_DPI_IF_SW_CG_BIT|
    DISP_DPI_ENGINE_SW_CG_BIT, 

    //CG_MFG
    0x0,

    //CG_AUDIO
    0x0,

    //CG_VIRTUAL
    0
};

static unsigned int soidle_block_mask[NR_GRPS] = {0x0};
#ifdef CONFIG_SMP
static signed int soidle_timer_left;
static signed int soidle_timer_left2;
#else
static unsigned int soidle_timer_left;
static unsigned int soidle_timer_left2;
static unsigned int soidle_timer_cmp;
#endif
static signed int soidle_time_critera = 26000;

static unsigned long soidle_cnt[NR_CPUS] = {0};
static unsigned long soidle_block_cnt[NR_CPUS][NR_REASONS] = {{0}};

void enable_soidle_by_bit(int id)
{
    unsigned int grp = clk_id_to_grp_id(id);
    unsigned int mask = clk_id_to_mask(id);

    if(( grp == NR_GRPS )||( mask == NR_GRPS ))
        idle_info("[%s]wrong clock id\n", __func__);
    else
    soidle_condition_mask[grp] &= ~mask;

}
EXPORT_SYMBOL(enable_soidle_by_bit);

void disable_soidle_by_bit(int id)
{
    unsigned int grp = clk_id_to_grp_id(id);
    unsigned int mask = clk_id_to_mask(id);

    if(( grp == NR_GRPS )||( mask == NR_GRPS ))
        idle_info("[%s]wrong clock id\n", __func__);
    else
        soidle_condition_mask[grp] |= mask;   
}
EXPORT_SYMBOL(disable_soidle_by_bit);


unsigned char soidle_can_enter(int cpu)
{
    int reason = NR_REASONS;


#ifdef CONFIG_SMP
    if ((atomic_read(&is_in_hotplug) == 1)||(atomic_read(&hotplug_cpu_count) != 1)) {
        reason = BY_CPU;
        goto soidle_out;
    }   
#endif	

    memset(soidle_block_mask, 0, NR_GRPS * sizeof(unsigned int));
    if (!clkmgr_idle_can_enter(soidle_condition_mask, soidle_block_mask)) {
        reason = BY_CLK;
        goto soidle_out;
    }

#ifdef CONFIG_SMP
    soidle_timer_left = localtimer_get_counter();
    if (soidle_timer_left < soidle_time_critera || 
            ((int)soidle_timer_left) < 0) {
        reason = BY_TMR;
        goto soidle_out;
    }
#else    
    gpt_get_cnt(GPT1, &soidle_timer_left);
    gpt_get_cmp(GPT1, &soidle_timer_cmp);
    if((soidle_timer_cmp-soidle_timer_left)<soidle_time_critera)
    {
        reason = BY_TMR;
        goto soidle_out;
    }
#endif


soidle_out:
    if (reason < NR_REASONS)
        soidle_block_cnt[cpu][reason]++;
    return reason;
}


void soidle_before_wfi(void)
{
#ifdef CONFIG_SMP
    int err = 0; 

    soidle_timer_left2 = localtimer_get_counter(); 

    free_gpt(GPT4);
    err = request_gpt(GPT4, GPT_ONE_SHOT, GPT_CLK_SRC_SYS, GPT_CLK_DIV_1, 
                0, NULL, GPT_NOAUTOEN);
    if (err) {
        idle_info("[%s]fail to request GPT4\n", __func__);
    }

    soidle_timer_left2 = localtimer_get_counter();

    if( soidle_timer_left2 <=0 )
        gpt_set_cmp(GPT4, 1);//Trigger GPT4 Timerout imediately
    else
    gpt_set_cmp(GPT4, soidle_timer_left2);

    start_gpt(GPT4);
#else
    gpt_get_cnt(GPT1, &soidle_timer_left2);
#endif
   
}

void soidle_after_wfi(bool sodi_en)
{
#ifdef CONFIG_SMP
    if (gpt_check_and_ack_irq(GPT4)) {
        localtimer_set_next_event(1);
    } else {
        /* waked up by other wakeup source */
        unsigned int cnt, cmp;
        gpt_get_cnt(GPT4, &cnt);
        gpt_get_cmp(GPT4, &cmp);
        if (unlikely(cmp < cnt)) {
            idle_err("[%s]GPT%d: counter = %10u, compare = %10u\n", __func__, 
                    GPT4 + 1, cnt, cmp);
            BUG();
        }    
        localtimer_set_next_event(cmp-cnt);
        stop_gpt(GPT4);
        free_gpt(GPT4);
    }
#endif    
    if(sodi_en)
        soidle_cnt[0]++;
}
static void go_to_soidle(bool sodi_en, int cpu)
{

    idle_ver("[SPM]SODI EN: %x, CPU %x\n",sodi_en,cpu);

    spm_go_to_sodi(sodi_en,cpu);

    idle_ver("SODI %s\n",spm_get_wake_up_result(SPM_PCM_SODI));

#ifdef CONFIG_SMP
    idle_ver("timer_left=%d, timer_left2=%d, delta=%d\n", 
        soidle_timer_left, soidle_timer_left2, soidle_timer_left-soidle_timer_left2);
#else
    idle_ver("timer_left=%d, timer_left2=%d, delta=%d,timeout val=%d\n", 
        soidle_timer_left, soidle_timer_left2, soidle_timer_left2-soidle_timer_left,soidle_timer_cmp-soidle_timer_left);
#endif

}


/************************************************
 * multi-core idle part
 ************************************************/
#ifdef SPM_MCDI_FUNC
extern u32 En_SPM_MCDI;

static unsigned int mcidle_gpt_percpu[NR_CPUS] = {
    NR_GPTS,//Core0
    GPT4,//Core1
};

#ifdef CONFIG_SMP
static signed int mcidle_timer_left[NR_CPUS];
static signed int mcidle_timer_left2[NR_CPUS];
#else
static unsigned int mcidle_timer_left[NR_CPUS];
static unsigned int mcidle_timer_left2[NR_CPUS];
#endif
static signed int mcidle_time_critera = 12000;
//static signed int mcidle_time_critera = 12000;


static unsigned long mcidle_cnt[NR_CPUS] = {0};
static unsigned long mcidle_block_cnt[NR_CPUS][NR_REASONS] = {{0}};

static DEFINE_MUTEX(mcidle_locked);

bool mcidle_can_enter(int cpu)
{
    int reason = NR_REASONS;
    
#ifndef CONFIG_SMP
    unsigned int cmp;
#endif

    /* MCDI Only */
    if (cpu == 0) {
        reason = BY_OTH;
        goto mcidle_out;
    }

#ifdef CONFIG_SMP
    if (atomic_read(&hotplug_cpu_count) == 1) {
        reason = BY_CPU;
        goto mcidle_out;
    }
#endif	

    if (atomic_read(&is_in_hotplug) == 1) {
        reason = BY_CPU;
        goto mcidle_out;
    }    
#ifdef CONFIG_SMP
    mcidle_timer_left[cpu] = localtimer_get_counter();
    if (mcidle_timer_left[cpu] < mcidle_time_critera || 
            ((int)mcidle_timer_left[cpu]) < 0) {
        reason = BY_TMR;
        goto mcidle_out;
    }
#else
    gpt_get_cnt(GPT1, &mcidle_timer_left[cpu]);
    gpt_get_cmp(GPT1, &cmp);
    if((cmp-mcidle_timer_left[cpu])<mcidle_time_critera)
    {
        reason = BY_TMR;
        goto mcidle_out;
    }
#endif

mcidle_out:
    if (reason < NR_REASONS) {
        mcidle_block_cnt[cpu][reason]++;
        return false;
    } else {
        return true;
    }
}

void mcidle_before_wfi(int cpu)
{
#ifdef CONFIG_SMP
    int err = 0;

    unsigned int id = mcidle_gpt_percpu[cpu];

    mcidle_timer_left2[cpu] = localtimer_get_counter(); 

    free_gpt(id);
    err = request_gpt(id, GPT_ONE_SHOT, GPT_CLK_SRC_SYS, GPT_CLK_DIV_1, 
                0, NULL, GPT_NOAUTOEN);
    if (err) {
        idle_info("[%s]fail to request GPT4\n", __func__);
    }

    mcidle_timer_left2[cpu] = localtimer_get_counter();


    if(cpu!=0)//core1~n, avoid gpt clear by core0
    {
        if( mcidle_timer_left2[cpu] <=2600 ) //200us(todo)
        {
                if(mcidle_timer_left2[cpu]<=0)
                    gpt_set_cmp(id, 1);//Trigger GPT4 Timerout imediately
                else
                    gpt_set_cmp(id, mcidle_timer_left2[cpu]);
                
                spm_write(SPM_SLEEP_CPU_WAKEUP_EVENT,spm_read(SPM_SLEEP_CPU_WAKEUP_EVENT)|0x1);//spm wake up directly
                
        }
        else
            gpt_set_cmp(id, mcidle_timer_left2[cpu]);

        start_gpt(id);
    }
#else
    gpt_get_cnt(GPT1, &mcidle_timer_left2);
#endif
}

void mcidle_after_wfi(int cpu)
{
#ifdef CONFIG_SMP
    unsigned int id = mcidle_gpt_percpu[cpu];
    //if (cpu != 0) {
        if (gpt_check_and_ack_irq(id)) {
            localtimer_set_next_event(1);
        } else {
            /* waked up by other wakeup source */
            unsigned int cnt, cmp;
            gpt_get_cnt(id, &cnt);
            gpt_get_cmp(id, &cmp);
            if (unlikely(cmp < cnt)) {
                idle_err("[%s]GPT%d: counter = %10u, compare = %10u\n", __func__, 
                        id + 1, cnt, cmp);
                BUG();
            }
        
            localtimer_set_next_event(cmp-cnt);
            stop_gpt(id);

            free_gpt(id);

        }
    //}
#endif
    mcidle_cnt[cpu]++;
}

extern u32 spm_pre_SPM_PCM_SW_INT_SET;
extern u32 spm_post_SPM_PCM_SW_INT_SET;
extern u32 spm_pre_SPM_SLEEP_ISR_STATUS;
extern u32 spm_post_SPM_SLEEP_ISR_STATUS;
extern u32 spm_post_SPM_PCM_RESERVE;
static void go_to_mcidle(int cpu)
{
    spm_go_to_sodi(0,cpu);   
}
#endif

/************************************************
 * deep idle part
 ************************************************/
static unsigned int dpidle_condition_mask[NR_GRPS]= 
{
    //CG_MIXED
    0x0,
    
    //CG_MPLL
    0x0,
    
    //CG_UPLL
    0x0,
    
    //CG_CTRL0
    PWM_MM_SW_CG_BIT|
    CAM_MM_SW_CG_BIT|
    MFG_MM_SW_CG_BIT|
    SPM_52M_SW_CG_BIT|
    DBI_BCLK_SW_CG_BIT|
    DBI_PAD0_SW_CG_BIT|
    DBI_PAD1_SW_CG_BIT|
    DBI_PAD2_SW_CG_BIT|
    DBI_PAD3_SW_CG_BIT,
    
    //CG_CTRL1
     EFUSE_SW_CG_BIT|
    //THEM_SW_CG_BIT|// //Thermal working from Arvin.Wang
    APDMA_SW_CG_BIT|
    I2C0_SW_CG_BIT|
    I2C1_SW_CG_BIT|
    AUX_SW_CG_MD_BIT|
    NFI_SW_CG_BIT|
    NFIECC_SW_CG_BIT|
    PWM_SW_CG_BIT|
    UART0_SW_CG_BIT|
    UART1_SW_CG_BIT|
    BTIF_SW_CG_BIT|
    USB_SW_CG_BIT|
    FHCTL_SW_CG_BIT|
    AUX_SW_CG_THERM_BIT|
    SPINFI_SW_CG_BIT|
    //MSDC0_SW_CG_BIT|
    //MSDC1_SW_CG_BIT|
    NFI2X_SW_CG_BIT | //71 add from YY
    SEJ_SW_CG_BIT|
    //MEMSLP_DLYER_SW_CG_BIT| //dpidle must enable
    SPI_SW_CG_BIT | // 71 add from YY
    //AUX_SW_CG_ADC_BIT| //Thermal working from Arvin.Wang
    AUX_SW_CG_TP_BIT,

    //CG_CTRL7
    NFI_HCLK_SW_CG_BIT,//71 add from YY

    //CG_MMSYS0
    MUTEX_SLOW_CLOCK_SW_CG_BIT| 
    DISP_FAKE_ENG_SW_CG_BIT|
    MM_CODEC_SW_CG_BIT|
    MM_CAMTG_SW_CG_BIT|
    MM_SENINF_SW_CG_BIT|
    MM_CAM_SMI_SW_CG_BIT|
    MM_CAM_DP2_SW_CG_BIT|
    MM_CAM_DP_SW_CG_BIT|
    DISP_PWM_26M_SW_CG_BIT|
    DISP_PWM_SW_CG_BIT|
    MDP_RDMA_SW_CG_BIT|
    MDP_RSZ_SW_CG_BIT|
    MDP_WROT_SW_CG_BIT|
    MDP_SHP_SW_CG_BIT|
    DISP_OVL_SW_CG_BIT|
    DISP_RDMA_SW_CG_BIT|
    DISP_WDMA_SW_CG_BIT|
    DISP_BLS_SW_CG_BIT|
    DISP_PQ_SW_CG_BIT|
    MM_CMDQ_SMI_SW_CG_BIT|
    MM_CMDQ_ENGINE_SW_CG_BIT|
    SMI_LARB0_SW_CG_BIT|
    SMI_COMMON_SW_CG_BIT,

    //CG_MMSYS1
    DISP_DBI_IF_SW_CG_BIT|
    DISP_DBI_ENGINE_SW_CG_BIT|
    DISP_DPI_IF_SW_CG_BIT|
    DISP_DPI_ENGINE_SW_CG_BIT|
    DSI_DIGITAL_SW_CG_BIT|
    DSI_ENGINE_SW_CG_BIT,
    
    //CG_MFG
    0x0,

    //CG_AUDIO
    0x0,

    //CG_VIRTUAL
    0
};



static unsigned long rgidle_cnt[NR_CPUS] = {0};
static unsigned int dpidle_block_mask[NR_GRPS] = {0x0};
static unsigned long dpidle_cnt[NR_CPUS] = {0};
static unsigned long dpidle_block_cnt[NR_REASONS] = {0};


static DEFINE_MUTEX(dpidle_locked);

static void enable_dpidle_by_mask(int grp, unsigned int mask)
{
    mutex_lock(&dpidle_locked);
    dpidle_condition_mask[grp] &= ~mask;
    mutex_unlock(&dpidle_locked);
}

static void disable_dpidle_by_mask(int grp, unsigned int mask)
{
    mutex_lock(&dpidle_locked);
    dpidle_condition_mask[grp] |= mask;
    mutex_unlock(&dpidle_locked);
}

void enable_dpidle_by_bit(int id)
{
    unsigned int grp = clk_id_to_grp_id(id);
    unsigned int mask = clk_id_to_mask(id);

    if(( grp == NR_GRPS )||( mask == NR_GRPS ))
        idle_info("[%s]wrong clock id\n", __func__);
    else
        dpidle_condition_mask[grp] &= ~mask;   
}
EXPORT_SYMBOL(enable_dpidle_by_bit);

void disable_dpidle_by_bit(int id)
{

    unsigned int grp = clk_id_to_grp_id(id);
    unsigned int mask = clk_id_to_mask(id);

    if(( grp == NR_GRPS )||( mask == NR_GRPS ))
        idle_info("[%s]wrong clock id\n", __func__);
    else
        dpidle_condition_mask[grp] |= mask;
}
EXPORT_SYMBOL(disable_dpidle_by_bit);

#ifdef CONFIG_SMP
signed int dpidle_timer_left;
signed int dpidle_timer_left2;
#else
unsigned int dpidle_timer_left;
unsigned int dpidle_timer_left2;
unsigned int dpidle_timer_cmp;
#endif
static signed int dpidle_time_critera = 26000;
static unsigned long long dpidle_block_prev_time = 0;
static unsigned long long dpidle_block_time_critera = 30000;//default 30sec

static bool dpidle_can_enter(void)
{

    int reason = NR_REASONS;
    int i;
    unsigned long long dpidle_block_curr_time = 0;
#ifdef SPM_MCDI_FUNC
    if (En_SPM_MCDI != 0) {
        reason = BY_OTH;
        goto out;
    }
#endif

#ifdef CONFIG_SMP
    if ((atomic_read(&is_in_hotplug) == 1)||(atomic_read(&hotplug_cpu_count) != 1)) {
        reason = BY_CPU;
        goto out;
    }
#endif


    memset(dpidle_block_mask, 0, NR_GRPS * sizeof(unsigned int));
    if (!clkmgr_idle_can_enter(dpidle_condition_mask, dpidle_block_mask)) {
        reason = BY_CLK;
        goto out;
    }

#ifdef CONFIG_SMP
    dpidle_timer_left = localtimer_get_counter();
    if (dpidle_timer_left < dpidle_time_critera || 
            ((int)dpidle_timer_left) < 0) {
        reason = BY_TMR;
        goto out;
    }
#else
    gpt_get_cnt(GPT1, &dpidle_timer_left);
    gpt_get_cmp(GPT1, &dpidle_timer_cmp);
    if((dpidle_timer_cmp-dpidle_timer_left)<dpidle_time_critera)
    {
        reason = BY_TMR;
        goto out;
    }
#endif
  

out:
    if (reason < NR_REASONS) {
        if( dpidle_block_prev_time == 0 )
            dpidle_block_prev_time = idle_get_current_time_ms();

        dpidle_block_curr_time = idle_get_current_time_ms();
        if((dpidle_block_curr_time - dpidle_block_prev_time) > dpidle_block_time_critera)
        {
            if ((smp_processor_id() == 0))
            {
                for (i = 0; i < nr_cpu_ids; i++) {
                    idle_ver("dpidle_cnt[%d]=%lu, rgidle_cnt[%d]=%lu\n", 
                            i, dpidle_cnt[i], i, rgidle_cnt[i]);
                } 
                
                for (i = 0; i < NR_REASONS; i++) {
                    idle_ver("[%d]dpidle_block_cnt[%s]=%lu\n", i, reason_name[i], 
                            dpidle_block_cnt[i]);
                }

                for (i = 0; i < NR_GRPS; i++) {
                    idle_ver("[%02d]dpidle_condition_mask[%-8s]=0x%08x\t\t"
                            "dpidle_block_mask[%-8s]=0x%08x\n", i, 
                            grp_get_name(i), dpidle_condition_mask[i],
                            grp_get_name(i), dpidle_block_mask[i]);
                }

                //printk("dpidle_block_prev_time =%lu, dpidle_block_curr_time = %lu\n",dpidle_block_prev_time,dpidle_block_curr_time);

                memset(dpidle_block_cnt, 0, sizeof(dpidle_block_cnt));
                dpidle_block_prev_time = idle_get_current_time_ms();
                
            }
           
            
        }       
        dpidle_block_cnt[reason]++;
        return false;
    } else {
        dpidle_block_prev_time = idle_get_current_time_ms();
        return true;
    }

}

static unsigned int g_clk_aud_intbus_sel = 0;
void spm_dpidle_before_wfi(void)
{

    int err = 0;

 
    g_clk_aud_intbus_sel = clkmux_get(MT_CLKMUX_AUD_INTBUS_SEL,"Deep_Idle");
    clkmux_sel(MT_CLKMUX_AUD_INTBUS_SEL, MT_CG_SYS_26M,"Deep_Idle");

    enable_clock(MT_CG_PMIC_SW_CG_AP, "DEEP_IDLE");//PMIC CG bit for AP. SPM need PMIC wrapper clock to change Vcore voltage
#ifdef CONFIG_SMP

    free_gpt(GPT4);
    err = request_gpt(GPT4, GPT_ONE_SHOT, GPT_CLK_SRC_SYS, GPT_CLK_DIV_1, 
                0, NULL, GPT_NOAUTOEN);

    if (err) {
        idle_info("[%s]fail to request GPT4\n", __func__);
    }
    
    dpidle_timer_left2 = localtimer_get_counter();

    if( dpidle_timer_left2 <=0 )
        gpt_set_cmp(GPT4, 1);//Trigger GPT4 Timerout imediately
    else
        gpt_set_cmp(GPT4, dpidle_timer_left2);
    
    start_gpt(GPT4);
#else
    gpt_get_cnt(GPT1, &dpidle_timer_left2);
#endif

}

void spm_dpidle_after_wfi(void)
{

#ifdef CONFIG_SMP
    //if (gpt_check_irq(GPT4)) {
    if (gpt_check_and_ack_irq(GPT4)) {
        /* waked up by WAKEUP_GPT */
        localtimer_set_next_event(1);
    } else {
        /* waked up by other wakeup source */
        unsigned int cnt, cmp;
        gpt_get_cnt(GPT4, &cnt);
        gpt_get_cmp(GPT4, &cmp);
        if (unlikely(cmp < cnt)) {
            idle_err("[%s]GPT%d: counter = %10u, compare = %10u\n", __func__, 
                    GPT4 + 1, cnt, cmp);
            BUG();
        }

        localtimer_set_next_event(cmp-cnt);
        stop_gpt(GPT4);
        //GPT_ClearCount(WAKEUP_GPT);
        free_gpt(GPT4);

    }
#endif    
    disable_clock(MT_CG_PMIC_SW_CG_AP, "DEEP_IDLE");  
    clkmux_sel(MT_CLKMUX_AUD_INTBUS_SEL,g_clk_aud_intbus_sel,"Deep_Idle");

    dpidle_cnt[0]++;
}



/************************************************
 * regular idle part
 ************************************************/

static void rgidle_before_wfi(int cpu)
{
}

static void rgidle_after_wfi(int cpu)
{
    rgidle_cnt[cpu]++;
}

static void noinline go_to_rgidle(int cpu)
{
    rgidle_before_wfi(cpu);

    dsb();
    __asm__ __volatile__("wfi" ::: "memory");

    rgidle_after_wfi(cpu);
}

/************************************************
 * idle task flow part
 ************************************************/

/*
 * xxidle_handler return 1 if enter and exit the low power state
 */
#ifdef SPM_MCDI_FUNC
extern wake_reason_t spm_go_to_mcdi_ipi_test(int cpu);
extern void spm_check_core_status_before(u32 target_core);
extern void spm_check_core_status_after(u32 target_core);
static inline int mcidle_handler(int cpu)
{

#if 1
    if (idle_switch[IDLE_TYPE_MC]) {
        if (mcidle_can_enter(cpu)) {
            go_to_mcidle(cpu);
            //go_to_soidle(0,cpu);
            return 1;
        }
    } 
#else

    if (idle_switch[IDLE_TYPE_MC]) {
        mtk_wdt_suspend();
        for(;;)
        {
            if(cpu==1)
            {
                printk("MCDI start\n");
                spm_go_to_mcdi_ipi_test(cpu);
                printk("MCDI %s\n",spm_get_wake_up_result(SPM_PCM_SODI));
            }
            if(cpu==0)
            {


                #if 1

                if(spm_read(SPM_FC1_PWR_CON)==0x32)
                {
                    //printk("IPI start\n");
                    spm_check_core_status_before(2);
                     //printk("1. SPM_PCM_EVENT_VECTOR2 = 0x%x, SPM_PCM_REG15_DATA = 0x%x \n", spm_read(SPM_PCM_EVENT_VECTOR2),spm_read(SPM_PCM_REG15_DATA));
                    //mdelay(1);
                    spm_check_core_status_after(2);
                    //printk("2. SPM_PCM_EVENT_VECTOR2 = 0x%x, SPM_PCM_REG15_DATA = 0x%x \n", spm_read(SPM_PCM_EVENT_VECTOR2),spm_read(SPM_PCM_REG15_DATA));
                }
                #endif
                
            }
        }
     }

#endif
    return 0;
}
#else
static inline int mcidle_handler(int cpu)
{
    return 0;
}
#endif
static unsigned long doidle_cnt[NR_CPUS] = {0};

static inline int soidle_handler(int cpu)
{
    unsigned char sodi_ret = 0;
    if (idle_switch[IDLE_TYPE_SO]) {
        if( soidle_can_enter(cpu)== NR_REASONS ) {
            go_to_soidle(1 , cpu);
            return 1;
        }
    } 

    return 0;
}


static int dpidle_cpu_pdn = 1;
static int dpidle_cpu_pwrlevel =0;//level1
//bool spm_change_fw = 0;

int dpidle_handler(int cpu)
{
    int ret = 0;
    wake_reason_t wr = WR_NONE;
    if (idle_switch[IDLE_TYPE_DP]) {
        if (dpidle_can_enter()) {
            idle_ver("idle-enter SPM: Deepidle\n");
            wr = spm_go_to_dpidle(dpidle_cpu_pdn, dpidle_cpu_pwrlevel);
            idle_ver("deepidle %s\n",spm_get_wake_up_result(SPM_PCM_DEEP_IDLE));
#ifdef CONFIG_SMP            
            idle_ver("timer_left=%d, timer_left2=%d, delta=%d\n", 
                dpidle_timer_left, dpidle_timer_left2, dpidle_timer_left-dpidle_timer_left2);
#else
            idle_ver("timer_left=%d, timer_left2=%d, delta=%d, timeout val=%d\n", 
                dpidle_timer_left, dpidle_timer_left2, dpidle_timer_left2-dpidle_timer_left,dpidle_timer_cmp-dpidle_timer_left);
#endif
            ret = 1;
        }
        
    }

    return ret;
}

static inline int rgidle_handler(int cpu)
{
    int ret = 0;
    if (idle_switch[IDLE_TYPE_RG]) {
        go_to_rgidle(cpu);
        ret = 1;
    }

    return ret;
}

static int (*idle_handlers[NR_TYPES])(int) = {    
    dpidle_handler,
    mcidle_handler,
    soidle_handler,    
    rgidle_handler,
};


void arch_idle(void)
{
    int cpu = smp_processor_id();
    int i;

    for (i = 0; i < NR_TYPES; i++) {
        if (idle_handlers[i](cpu))
            break;
    }
}

#define idle_attr(_name)                         \
static struct kobj_attribute _name##_attr = {   \
    .attr = {                                   \
        .name = __stringify(_name),             \
        .mode = 0644,                           \
    },                                          \
    .show = _name##_show,                       \
    .store = _name##_store,                     \
}

extern struct kobject *power_kobj;

#ifdef SPM_MCDI_FUNC
static ssize_t mcidle_state_show(struct kobject *kobj, 
                struct kobj_attribute *attr, char *buf)
{
    int len = 0;
    char *p = buf;

    int cpus, reason, i;

    p += sprintf(p, "*********** multi-core idle state ************\n");
    p += sprintf(p, "mcidle_time_critera=%u\n", mcidle_time_critera);

    for (cpus = 0; cpus < nr_cpu_ids; cpus++) {
        p += sprintf(p, "cpu:%d\n", cpus);
        for (reason = 0; reason < NR_REASONS; reason++) {
            p += sprintf(p, "[%d]mcidle_block_cnt[%s]=%lu\n", reason, 
                    reason_name[reason], mcidle_block_cnt[cpus][reason]);
        }
        p += sprintf(p, "\n");
    }

    for (i = 0; i < NR_GRPS; i++) {
        p += sprintf(p, "[%02d]soidle_condition_mask[%-8s]=0x%08x\t\t"
                "soidle_block_mask[%08x]=0x%08x\n", i, 
                grp_get_name(i), soidle_condition_mask[i],
                grp_get_name(i), soidle_block_mask[i]);
    }

    p += sprintf(p, "\n********** mcidle command help **********\n");
    p += sprintf(p, "mcidle help:   cat /sys/power/mcidle_state\n");
    p += sprintf(p, "switch on/off: echo [mcidle] 1/0 > /sys/power/mcidle_state\n");
    p += sprintf(p, "modify tm_cri: echo time value(dec) > /sys/power/mcidle_state\n");

    len = p - buf;
    return len;
}

static ssize_t mcidle_state_store(struct kobject *kobj, 
                struct kobj_attribute *attr, const char *buf, size_t n)
{
    char cmd[32];
    int param;

    if (sscanf(buf, "%s %d", cmd, &param) == 2) {
        if (!strcmp(cmd, "mcidle")) {
            idle_switch[IDLE_TYPE_MC] = param;
        }
        else if (!strcmp(cmd, "time")) {
            mcidle_time_critera = param;
        }
        return n;
    } else if (sscanf(buf, "%d", &param) == 1) {
        idle_switch[IDLE_TYPE_MC] = param;
        return n;
    }

    return -EINVAL;
}
idle_attr(mcidle_state);
#endif

static ssize_t soidle_state_show(struct kobject *kobj, 
                struct kobj_attribute *attr, char *buf)
{
    int len = 0;
    char *p = buf;

    int cpus, reason, i;

    p += sprintf(p, "*********** multi-core idle state ************\n");
    p += sprintf(p, "soidle_time_critera=%u\n", soidle_time_critera);

    for (cpus = 0; cpus < nr_cpu_ids; cpus++) {
        p += sprintf(p, "cpu:%d\n", cpus);
        for (reason = 0; reason < NR_REASONS; reason++) {
            p += sprintf(p, "[%d]soidle_block_cnt[%s]=%lu\n", reason, 
                    reason_name[reason], soidle_block_cnt[cpus][reason]);
        }
        p += sprintf(p, "\n");
    }

    for (i = 0; i < NR_GRPS; i++) {
        p += sprintf(p, "[%02d]soidle_condition_mask[%-8s]=0x%08x\t\t"
                "soidle_block_mask[%08x]=0x%08x\n", i, 
                grp_get_name(i), soidle_condition_mask[i],
                grp_get_name(i), soidle_block_mask[i]);
    }

    p += sprintf(p, "\n********** mcidle command help **********\n");
    p += sprintf(p, "soidle help:   cat /sys/power/soidle_state\n");
    p += sprintf(p, "switch on/off: echo [soidle] 1/0 > /sys/power/soidle_state\n");
    p += sprintf(p, "en_so_by_bit:  echo enable id > /sys/power/soidle_state\n");
    p += sprintf(p, "dis_so_by_bit: echo disable id > /sys/power/soidle_state\n");
    p += sprintf(p, "modify tm_cri: echo time value(dec) > /sys/power/soidle_state\n");

    len = p - buf;
    return len;
}

static ssize_t soidle_state_store(struct kobject *kobj, 
                struct kobj_attribute *attr, const char *buf, size_t n)
{
    char cmd[32];
    int param;

    if (sscanf(buf, "%s %d", cmd, &param) == 2) {
        if (!strcmp(cmd, "soidle")) {
            idle_switch[IDLE_TYPE_SO] = param;
        } else if (!strcmp(cmd, "enable")) {
            enable_soidle_by_bit(param);
        } else if (!strcmp(cmd, "disable")) {
            disable_soidle_by_bit(param);
        } else if (!strcmp(cmd, "time")) {
            soidle_time_critera = param;
        } else if (!strcmp(cmd, "bypass")) {
             memset(soidle_condition_mask, 0, NR_GRPS * sizeof(unsigned int));
        }
        return n;
    } else if (sscanf(buf, "%d", &param) == 1) {
        idle_switch[IDLE_TYPE_SO] = param;
        return n;
    }

    return -EINVAL;
}
idle_attr(soidle_state);


static ssize_t dpidle_state_show(struct kobject *kobj, 
                struct kobj_attribute *attr, char *buf)
{
    int len = 0;
    char *p = buf;

    int i;

    p += sprintf(p, "*********** deep idle state ************\n");
    p += sprintf(p, "dpidle_cpu_pdn = %d\n", dpidle_cpu_pdn);
    p += sprintf(p, "dpidle_time_critera=%u\n", dpidle_time_critera);
    p += sprintf(p, "dpidle_block_time_critera=%u\n", dpidle_block_time_critera);

    

    for (i = 0; i < NR_REASONS; i++) {
        p += sprintf(p, "[%d]dpidle_block_cnt[%s]=%lu\n", i, reason_name[i], 
                dpidle_block_cnt[i]);
    }

    p += sprintf(p, "\n");

    for (i = 0; i < NR_GRPS; i++) {
        p += sprintf(p, "[%02d]dpidle_condition_mask[%-8s]=0x%08x\t\t"
                "dpidle_block_mask[%-8s]=0x%08x\n", i, 
                grp_get_name(i), dpidle_condition_mask[i],
                grp_get_name(i), dpidle_block_mask[i]);
    }

    p += sprintf(p, "\n*********** dpidle command help  ************\n");
    p += sprintf(p, "dpidle help:   cat /sys/power/dpidle_state\n");
    p += sprintf(p, "switch on/off: echo [dpidle] 1/0 > /sys/power/dpidle_state\n");
    p += sprintf(p, "cpupdn on/off: echo cpupdn 1/0 > /sys/power/dpidle_state\n");
    p += sprintf(p, "en_dp_by_bit:  echo enable id > /sys/power/dpidle_state\n");
    p += sprintf(p, "dis_dp_by_bit: echo disable id > /sys/power/dpidle_state\n");
    p += sprintf(p, "modify tm_cri: echo time value(dec) > /sys/power/dpidle_state\n");
    p += sprintf(p, "modify block_tm_cri: echo block_time value(ms) > /sys/power/dpidle_state\n");

    len = p - buf;
    return len;
}


static ssize_t dpidle_state_store(struct kobject *kobj, 
                struct kobj_attribute *attr, const char *buf, size_t n)
{
    char cmd[32];
    int param;

    if (sscanf(buf, "%s %d", cmd, &param) == 2) {
        if (!strcmp(cmd, "dpidle")) {
            idle_switch[IDLE_TYPE_DP] = param;
        } else if (!strcmp(cmd, "enable")) {
            enable_dpidle_by_bit(param);
        } else if (!strcmp(cmd, "disable")) {
            disable_dpidle_by_bit(param);
        } else if (!strcmp(cmd, "cpupdn")) {
            dpidle_cpu_pdn = !!param;
        } else if (!strcmp(cmd, "time")) {
            dpidle_time_critera = param;
        } else if (!strcmp(cmd, "block_time")) {
            dpidle_block_time_critera = param;            
        } else if (!strcmp(cmd, "bypass")) {
             memset(dpidle_condition_mask, 0, NR_GRPS * sizeof(unsigned int));
        }        
        return n;
    } else if (sscanf(buf, "%d", &param) == 1) {
        idle_switch[IDLE_TYPE_DP] = param;
        return n;
    }

    return -EINVAL;
}
idle_attr(dpidle_state);


static ssize_t rgidle_state_show(struct kobject *kobj, 
                struct kobj_attribute *attr, char *buf)
{
    int len = 0;
    char *p = buf;

    p += sprintf(p, "*********** regular idle state ************\n");
    p += sprintf(p, "\n********** rgidle command help **********\n");
    p += sprintf(p, "rgidle help:   cat /sys/power/rgidle_state\n");
    p += sprintf(p, "switch on/off: echo [rgidle] 1/0 > /sys/power/rgidle_state\n");

    len = p - buf;
    return len;
}

static ssize_t rgidle_state_store(struct kobject *kobj, 
                struct kobj_attribute *attr, const char *buf, size_t n)
{
    char cmd[32];
    int param;

    if (sscanf(buf, "%s %d", cmd, &param) == 2) {
        if (!strcmp(cmd, "rgidle")) {
            idle_switch[IDLE_TYPE_RG] = param;
        }
        return n;
    } else if (sscanf(buf, "%d", &param) == 1) {
        idle_switch[IDLE_TYPE_RG] = param;
        return n;
    }

    return -EINVAL;
}
idle_attr(rgidle_state);

static ssize_t idle_state_show(struct kobject *kobj, 
                struct kobj_attribute *attr, char *buf)
{
    int len = 0;
    char *p = buf;
    
    int i;

    p += sprintf(p, "********** idle state dump **********\n");
#ifdef SPM_MCDI_FUNC
    for (i = 0; i < nr_cpu_ids; i++) {
        p += sprintf(p, "mcidle_cnt[%d]=%lu, soidle_cnt[%d]=%lu, doidle_cnt[%d]=%lu, dpidle_cnt[%d]=%lu, "
                "rgidle_cnt[%d]=%lu\n", 
                i, mcidle_cnt[i], i, soidle_cnt[i],  i, doidle_cnt[i],  i, dpidle_cnt[i], 
                i, rgidle_cnt[i]);
    }
#else
    for (i = 0; i < nr_cpu_ids; i++) {
        p += sprintf(p, "dpidle_cnt[%d]=%lu, rgidle_cnt[%d]=%lu\n", 
                i, dpidle_cnt[i], i, rgidle_cnt[i]);
    }
#endif
    
    p += sprintf(p, "\n********** variables dump **********\n");
    for (i = 0; i < NR_TYPES; i++) {
        p += sprintf(p, "%s_switch=%d, ", idle_name[i], idle_switch[i]);
    }
    p += sprintf(p, "\n");

    p += sprintf(p, "\n********** idle command help **********\n");
    p += sprintf(p, "status help:   cat /sys/power/idle_state\n");
    p += sprintf(p, "switch on/off: echo switch mask > /sys/power/idle_state\n");

    p += sprintf(p, "mcidle help:   cat /sys/power/soidle_state\n");
#ifdef SPM_MCDI_FUNC
    p += sprintf(p, "mcidle help:   cat /sys/power/mcidle_state\n");
#else
    p += sprintf(p, "mcidle help:   mcidle is unavailable\n");
#endif
    p += sprintf(p, "dpidle help:   cat /sys/power/dpidle_state\n");
    p += sprintf(p, "rgidle help:   cat /sys/power/rgidle_state\n");

    len = p - buf;
    return len;
}

static ssize_t idle_state_store(struct kobject *kobj, 
                struct kobj_attribute *attr, const char *buf, size_t n)
{
    char cmd[32];
    int idx;
    int param;

    if (sscanf(buf, "%s %x", cmd, &param) == 2) {
        if (!strcmp(cmd, "switch")) {
            for (idx = 0; idx < NR_TYPES; idx++) {
#ifndef SPM_MCDI_FUNC
                if (idx == IDLE_TYPE_MC) {
                    continue;
                }
#endif
                idle_switch[idx] = (param & (1U << idx)) ? 1 : 0;
            }
        }
        return n;
    }

    return -EINVAL;
}
idle_attr(idle_state);


bool idle_state_get(u8 idx)
{  
    return idle_switch[idx];
}

bool idle_state_en(u8 idx, bool en)
{
    if(idx >= NR_TYPES)
        return FALSE;
    idle_switch[idx] = en;
    return false;
}

void mt_idle_init(void)
{
    int err = 0;

    idle_info("[%s]entry!!\n", __func__);
    arm_pm_idle = arch_idle;

#ifndef SPM_SUSPEND_GPT_EN    
    err = request_gpt(GPT4, GPT_ONE_SHOT, GPT_CLK_SRC_SYS, GPT_CLK_DIV_1, 
                0, NULL, GPT_NOAUTOEN);
    if (err) {
        idle_info("[%s]fail to request GPT4\n", __func__);
    }
#endif

    err = sysfs_create_file(power_kobj, &idle_state_attr.attr);
#ifdef SPM_MCDI_FUNC
    err |= sysfs_create_file(power_kobj, &mcidle_state_attr.attr);
#endif
    err |= sysfs_create_file(power_kobj, &soidle_state_attr.attr);
    err |= sysfs_create_file(power_kobj, &dpidle_state_attr.attr);
    err |= sysfs_create_file(power_kobj, &rgidle_state_attr.attr);

    if (err) {
        idle_err("[%s]: fail to create sysfs\n", __func__);
    }
}
