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
#include <linux/cpumask.h>

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
#define SPM_MCDI_BYPASS_SYSPWREQ 0//for FPGA attach jtag
#else
#define SPM_MCDI_BYPASS_SYSPWREQ 1
#endif

#define MCDI_KICK_PCM 1

#define SPM_MCDI_LDVT_EN

#define clc_debug spm_debug
#define clc_notice spm_notice



// ===================================


u32 MCDI_Test_Mode = 0;


#define WFI_OP        4
#define WFI_L2C      5
#define WFI_SCU      6
#define WFI_MM      16
#define WFI_MD      19

#define SPM_PCM_HOTPLUG          (1U << 16)
#define SPM_PCM_IPI              (1U << 17)

#define PCM_MCDI_BASE            0//MCDI F/W merge to SODI F/W
#define PCM_MCDI_LEN              ( 0 )//MCDI F/W merge to SODI F/W
#define SPM_MCDI_FIRMWARE_VER ""//MCDI F/W merge to SODI F/W


extern void mt_irq_mask_for_sleep(unsigned int irq);
extern void mt_irq_unmask_for_sleep(unsigned int irq);
#if defined( SPM_MCDI_LDVT_EN )
extern void spm_mcdi_LDVT_mcdi(void);
extern void spm_mcdi_LDVT_sodi(void);
#endif
extern char *get_env(char *name);

/*
extern void //mt_cirq_enable(void);
extern void //mt_cirq_disable(void);
extern void //mt_cirq_clone_gic(void);
extern void //mt_cirq_flush(void);
extern void //mt_cirq_mask(unsigned int cirq_num);
*/
extern spinlock_t spm_lock;
extern u32 En_SPM_MCDI;

#if SPM_MCDI_BYPASS_SYSPWREQ    
    #define WAKE_SRC_FOR_MCDI                     \
        (WAKE_SRC_PCM_TIMER | WAKE_SRC_GPT | WAKE_SRC_THERM | WAKE_SRC_CIRQ | WAKE_SRC_CPU0_IRQ | WAKE_SRC_CPU1_IRQ | WAKE_SRC_SYSPWREQ )
#else    
    #define WAKE_SRC_FOR_MCDI                     \
        (WAKE_SRC_PCM_TIMER | WAKE_SRC_GPT | WAKE_SRC_THERM | WAKE_SRC_CIRQ | WAKE_SRC_CPU0_IRQ | WAKE_SRC_CPU1_IRQ )
#endif


SPM_PCM_CONFIG pcm_config_mcdi={
    .scenario = SPM_PCM_MCDI,
    .ver = SPM_MCDI_FIRMWARE_VER,
    .spm_turn_off_26m = false,
    .pcm_firmware_addr =  PCM_MCDI_BASE,
    .pcm_firmware_len = PCM_MCDI_LEN,
    .pcm_pwrlevel = PWR_LVNA,
    .spm_request_uart_sleep = false,
    .sodi_en = false,
    .pcm_vsr = {0,0,0,0,0,0,0,0},

    //spm_write(SPM_AP_STANBY_CON, ((0x0<<WFI_OP) | (0x1<<WFI_L2C) | (0x1<<WFI_SCU)));  // operand or, mask l2c, mask scu 

    /*Wake up event mask*/
    .md_mask = MDCONN_UNMASK,   /* mask MD1 and MD2 */
    .mm_mask = MFG_MASK,   /* mask DISP and MFG */

    .sync_r0r7=true,

    /*AP Sleep event mask*/
    .wfi_scu_mask=false,  /* check SCU idle */
    .wfi_l2c_mask=false,  /* check L2C idle */
    .wfi_op=REDUCE_OR,
    .wfi_sel = {true,true},

    .timer_val_ms = 0*1000,
    .wake_src = WAKE_SRC_FOR_MCDI,
    .infra_pdn = false,  /* keep INFRA/DDRPHY power */
    .cpu_pdn = true,
    .coclock_en = false,
    .lock_infra_dcm=false,
    
    /* debug related*/
    .reserved = 0x0,
    .dbg_wfi_cnt=0,
    .wakesta_idx=0
	 };


extern void mcidle_before_wfi(int cpu);
extern void mcidle_after_wfi(int cpu);

//extern u32 cpu_pdn_cnt;
void spm_mcdi_wfi(void)
{   
        volatile u32 core_id;
        //u32 clc_counter;
        unsigned long flags;  
        //u32 temp_address;
#if 0 

        core_id = (u32)smp_processor_id();
        
       
        if(core_id == 0)
        {

            if(MCDI_Test_Mode == 1)
            {
                //clc_notice("SPM_FC1_PWR_CON %x, cpu_pdn_cnt %d.\n",spm_read(SPM_FC1_PWR_CON),cpu_pdn_cnt);
                clc_notice("core_%d set wfi_sel.\n", core_id);   
            }

            spm_wfi_sel(pcm_config_mcdi.wfi_sel, SPM_CORE1_WFI_SEL_SW_MASK );

            spm_mcdi_poll_mask(core_id,pcm_config_mcdi.wfi_sel);  
            if(MCDI_Test_Mode == 1)
            {
                clc_notice("core_%d mask polling done.\n", core_id);   
            }
            wfi_with_sync(); // enter wfi 

            //spm_get_wakeup_status(&pcm_config_mcdi);
            
            if(MCDI_Test_Mode == 1)
                clc_notice("core_%d exit wfi.\n", core_id);                 

            //if(MCDI_Test_Mode == 1)
                //mdelay(10);  // delay 10 ms    
       

        }
        else // Core 1 Keep original IRQ
#endif
        {
            if(MCDI_Test_Mode == 1)
            {
                clc_notice("core_%d set wfi_sel.\n", core_id);   
            }
            spm_wfi_sel(pcm_config_mcdi.wfi_sel, SPM_CORE0_WFI_SEL_SW_MASK );

            ////clc_notice("core_%d enter wfi.\n", core_id);
            spm_mcdi_poll_mask(core_id,pcm_config_mcdi.wfi_sel); 
            if(MCDI_Test_Mode == 1)
            {
                clc_notice("core_%d mask polling done.\n", core_id);   
            }      

            mcidle_before_wfi(1);
            if (!cpu_power_down(DORMANT_MODE)) 
            {
                switch_to_amp();  
                
                /* do not add code here */
                wfi_with_sync();
            }
            switch_to_smp(); 
            
            cpu_check_dormant_abort();
            mcidle_after_wfi(1);
            spm_get_wakeup_status(&pcm_config_mcdi);
            if(MCDI_Test_Mode == 1)
                clc_notice("core_%d exit wfi.\n", core_id);
      
        }

}       


static int spm_mcdi_probe(struct platform_device *pdev)
{
    //remove for MT6571
    return 0;
}



/***************************
* show current SPM-MCDI stauts
****************************/
static int spm_mcdi_debug_read(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
    int len = 0;
    char *p = buf;

    if (En_SPM_MCDI)
        p += sprintf(p, "SPM MCDI+Thermal Protect enabled.\n");
    else
        p += sprintf(p, "SPM MCDI disabled, Thermal Protect only.\n");

    len = p - buf;
    return len;
}

/************************************
* set SPM-MCDI stauts by sysfs interface
*************************************/
static ssize_t spm_mcdi_debug_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
    int enabled = 0;

    if (sscanf(buffer, "%d", &enabled) == 1)
    {
        if (enabled == 0)
        {
            En_SPM_MCDI = 1;
        }
        else
        {
            clc_notice("bad argument_0!! (argument_0 = %d)\n", enabled);
        }
    }
    else
    {
            clc_notice("bad argument_1!! \n");
    }

    return count;

}
static int __init spm_mcdi_init(void)
{
    struct proc_dir_entry *mt_entry = NULL;    
    struct proc_dir_entry *mt_mcdi_dir = NULL;
    int mcdi_err = 0;

    mt_mcdi_dir = proc_mkdir("mcdi", NULL);
    if (!mt_mcdi_dir)
    {
        clc_notice("[%s]: mkdir /proc/mcdi failed\n", __FUNCTION__);
    }
    else
    {
        mt_entry = create_proc_entry("mcdi_debug", S_IRUGO | S_IWUSR | S_IWGRP, mt_mcdi_dir);
        if (mt_entry)
        {
            mt_entry->read_proc = spm_mcdi_debug_read;
            mt_entry->write_proc = spm_mcdi_debug_write;
        }
    }
    return 0;

}

/*
    CORE0_AUX  :  PCM_EVENT_VECTOR2[24:16]
    CORE1_AUX  :  PCM_EVENT_VECTOR3[24:16]
    CORE2_AUX  :  PCM_EVENT_VECTOR4[24:16]
    CORE3_AUX  :  PCM_EVENT_VECTOR5[24:16]
    CORE4_AUX  :  PCM_EVENT_VECTOR6[24:16]
    CORE5_AUX  :  PCM_EVENT_VECTOR7[24:16]
    CORE6_AUX  :  PCM_RESERVED[24:16]
    CORE7_AUX  :  { SLEEP_APMCU_PWRCTL[7:0],SLEEP_L2C_TAG_SLEEP[0] }


    Bit0 :  COREn  hot-plugged ?   1: hot-plugged ;  0: non hot-plugged
    Bit1 : Assert IPI to CORE0
    Bit2 : Assert IPI to CORE1
    Bit3 : Assert IPI to CORE2
    Bit4 : Assert IPI to CORE3
    Bit5 : Assert IPI to CORE4
    Bit6 : Assert IPI to CORE5
    Bit7 : Assert IPI to CORE6
    Bit8 : Assert IPI to CORE7

*/
u32 En_SPM_MCDI = 0;
void spm_check_core_status_before(u32 target_core)
{
    //remove spm IPI
}


void spm_check_core_status_after(u32 target_core)
{
    //clear SPM IPI & wait FCn mtcmos move to F/W
}


void spm_hot_plug_in_before(u32 target_core)
{
//#ifndef SPM_SODI_ONLY

    spm_notice("spm_hot_plug_in_before()........ target_core = 0x%x\n", target_core);

    switch (target_core)
    {
        case 0 : spm_write(SPM_PCM_EVENT_VECTOR2, (spm_read(SPM_PCM_EVENT_VECTOR2) & (~SPM_PCM_HOTPLUG)));  break;
        case 1 : spm_write(SPM_PCM_EVENT_VECTOR3, (spm_read(SPM_PCM_EVENT_VECTOR3) & (~SPM_PCM_HOTPLUG)));  break;
        default : break;
    }

//#endif
}

void spm_hot_plug_out_after(u32 target_core)
{
//#ifndef SPM_SODI_ONLY
    spm_notice("spm_hot_plug_out_after()........ target_core = 0x%x\n", target_core);    

    switch (target_core)
    {
        case 0 : spm_write(SPM_PCM_EVENT_VECTOR2, (spm_read(SPM_PCM_EVENT_VECTOR2) | SPM_PCM_HOTPLUG));  break;                     
        case 1 : spm_write(SPM_PCM_EVENT_VECTOR3, (spm_read(SPM_PCM_EVENT_VECTOR3) | SPM_PCM_HOTPLUG));  break;    
        default : break;
    }
//#endif
}

void spm_mcdi_init_core_mux(void)
{
    int i = 0;
    // set SPM_MP_CORE0_AUX
    spm_write(SPM_PCM_EVENT_VECTOR2, spm_read(SPM_PCM_EVENT_VECTOR2)&0xfe00ffff);
    spm_write(SPM_PCM_EVENT_VECTOR3, spm_read(SPM_PCM_EVENT_VECTOR3)&0xfe00ffff); 
    
    for (i = (num_possible_cpus() - 1); i > 0; i--)
    {
        if (cpu_online(i)==0)
        {
            switch(i)
            {
                case 1://for 72, only core1 hotplug out
                    spm_write(SPM_PCM_EVENT_VECTOR3, spm_read(SPM_PCM_EVENT_VECTOR3)|SPM_PCM_HOTPLUG);
                default: break;                    
            //cpu_down(i);
            }
        }
    }
    
}

void spm_mcdi_poll_mask(u32 core_id,bool core_wfi_sel[])
{
    while((spm_read(SPM_SLEEP_CPU_IRQ_MASK) & (0x1 << core_id) )==0);
#if 0        
    {
        if(core_id==0)
            spm_wfi_sel(core_wfi_sel, SPM_CORE1_WFI_SEL_SW_MASK );//check Core IRQ Mask  
        else    
            spm_wfi_sel(core_wfi_sel, SPM_CORE0_WFI_SEL_SW_MASK );;//check Core IRQ Mask  
    }
#endif

}


//late_initcall(spm_mcdi_init);

//MODULE_DESCRIPTION("MT6589 SPM-Idle Driver v0.1");
