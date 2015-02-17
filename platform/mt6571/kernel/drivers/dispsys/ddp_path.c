#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/mm_types.h>
#include <linux/module.h>
#include <linux/autoconf.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/kdev_t.h>
#include <linux/delay.h>
#include <linux/ioport.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/wait.h>
#include <linux/spinlock.h>
#include <linux/param.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <linux/slab.h>

#include <linux/xlog.h>
#include <asm/io.h>

#include <mach/irqs.h>
#include <mach/mt_reg_base.h>
#include <mach/mt_irq.h>
#include <mach/irqs.h>
#include <mach/mt_clkmgr.h>
#include <mach/mt_irq.h>
#include <mach/sync_write.h>
#include <mach/m4u.h>
#include "smi_common.h"
#include "devinfo.h"

#include "ddp_drv.h"
#include "ddp_reg.h"
#include "ddp_path.h"
#include "ddp_debug.h"

#include "ddp_rdma.h"
#include "ddp_wdma.h"
#include "ddp_ovl.h"
#include "ddp_pq.h"

#define DISP_INT_MUTEX_BIT_MASK 0x00000002

#define ENABLE_MUTEX_INTERRUPT 1

unsigned int gMutexID = 0;
unsigned int gTdshpStatus[OVL_LAYER_NUM] = {0};
unsigned int gMemOutMutexID = 2;
extern unsigned int decouple_addr;
extern BOOL DISP_IsDecoupleMode(void);
extern UINT32 DISP_GetScreenWidth(void);
extern UINT32 DISP_GetScreenHeight(void);
extern unsigned int disp_ms2jiffies(unsigned long ms);
static DEFINE_MUTEX(DpEngineMutexLock);

extern unsigned char pq_debug_flag;
extern unsigned char ddp_debug_flag;
static DECLARE_WAIT_QUEUE_HEAD(g_disp_mutex_wq);
static unsigned int g_disp_mutex_reg_update = 0;
DECLARE_WAIT_QUEUE_HEAD(mem_out_wq);
static unsigned int mem_out_done = 0;
extern volatile int g_HistogramValid;
extern int g_AAL_LumaUpdated;

static void _disp_path_mutex_reg_update_cb(unsigned int param)
{
    if (param & (1 << gMutexID))
    {
        g_disp_mutex_reg_update = 1;
        wake_up_interruptible(&g_disp_mutex_wq);
    }
}


unsigned int disp_mutex_lock_cnt = 0;
unsigned int disp_mutex_unlock_cnt = 0;

int disp_path_get_mutex()
{
    // ddp force stopping flag
    if ((pq_debug_flag == 3) || (ddp_debug_flag == 1))
    {
        return 0;
    }
    else
    {
        disp_register_irq(DISP_MODULE_MUTEX, _disp_path_mutex_reg_update_cb);
        return disp_path_get_mutex_(gMutexID);
    }
}


int disp_path_get_mutex_(int mutexID)
{
    unsigned int cnt = 0;

    DDP_DRV_DBG("disp_path_get_mutex %d \n", disp_mutex_lock_cnt++);
    mutex_lock(&DpEngineMutexLock);
    MMProfileLog(DDP_MMP_Events.Mutex[mutexID], MMProfileFlagStart);
    DISP_REG_SET(DISP_REG_CONFIG_MUTEX_EN(mutexID), 1);
    DISP_REG_SET(DISP_REG_CONFIG_MUTEX(mutexID), 1);
    DISP_REG_SET_FIELD(REG_FLD(1, mutexID), DISP_REG_CONFIG_MUTEX_INTSTA, 0);

    while ((DISP_REG_GET(DISP_REG_CONFIG_MUTEX(mutexID)) & DISP_INT_MUTEX_BIT_MASK) != DISP_INT_MUTEX_BIT_MASK)
    {
        if (cnt > 2000)
        {
            DDP_DRV_ERR("disp_path_get_mutex(), get mutex timeout!\n");
            disp_dump_reg(DISP_MODULE_MUTEX);
            disp_dump_reg(DISP_MODULE_CONFIG);
            MMProfileLogEx(DDP_MMP_Events.Mutex[mutexID], MMProfileFlagPulse, 0, 0);
            ASSERT(0);
            break;
        }
        mdelay(1);
        cnt ++;
    }

    return 0;
}


int disp_path_release_mutex(void)
{
    // ddp force stopping flag
    if ((pq_debug_flag == 3) || (ddp_debug_flag == 1))
    {
        return 0;
    }
    else
    {
        g_disp_mutex_reg_update = 0;
        return disp_path_release_mutex_(gMutexID);
    }
}

int disp_check_engine_status(int mutexID)
{
    int result = 0;
    unsigned int engine = DISP_REG_GET(DISP_REG_CONFIG_MUTEX_MOD(mutexID));

    if ((DISP_REG_GET(DISP_REG_CONFIG_MMSYS_CG_CON0) & SMI_LARB0_SW_CG_BIT) != 0)
    {
        result = -1;
        DDP_DRV_ERR("smi clk abnormal, clk=0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MMSYS_CG_CON0));
    }

    if (engine & DDP_MOD_DISP_OVL)
    {
        if (DISP_REG_GET(DISP_REG_OVL_EN) == 0 ||
           (DISP_REG_GET(DISP_REG_CONFIG_MMSYS_CG_CON0) & DISP_OVL_SW_CG_BIT) != 0)
        {
            result = -1;
            DDP_DRV_ERR("ovl abnormal, en=%d, clk=0x%x \n",
                             DISP_REG_GET(DISP_REG_OVL_EN),
                             DISP_REG_GET(DISP_REG_CONFIG_MMSYS_CG_CON0));
        }
    }

    if (engine & DDP_MOD_DISP_WDMA)
    {
        if (DISP_REG_GET(DISP_REG_WDMA_EN) == 0 ||
           (DISP_REG_GET(DISP_REG_CONFIG_MMSYS_CG_CON0) & DISP_WDMA_SW_CG_BIT) != 0)
        {
            result = -1;
            DDP_DRV_ERR("wdma abnormal, en=%d, clk=0x%x \n",
                             DISP_REG_GET(DISP_REG_WDMA_EN),
                             DISP_REG_GET(DISP_REG_CONFIG_MMSYS_CG_CON0));
        }
    }

    if (engine & DDP_MOD_DISP_PQ)
    {
        if ((DISP_REG_GET(DISP_REG_PQ_CTRL) & 0x1) == 0 ||
           (DISP_REG_GET(DISP_REG_CONFIG_MMSYS_CG_CON0) & DISP_PQ_SW_CG_BIT) != 0)
        {
            result = -1;
            DDP_DRV_ERR("pq abnormal, en=%d, clk=0x%x \n",
                             DISP_REG_GET(DISP_REG_PQ_CTRL),
                             DISP_REG_GET(DISP_REG_CONFIG_MMSYS_CG_CON0));
        }
    }

    if (engine & DDP_MOD_DISP_BLS)
    {
        if (DISP_REG_GET(DISP_REG_BLS_EN) == 0 ||
           (DISP_REG_GET(DISP_REG_CONFIG_MMSYS_CG_CON0) & DISP_BLS_SW_CG_BIT) != 0)
        {
            result = -1;
            DDP_DRV_ERR("bls abnormal, en=%d, clk=0x%x \n",
                             DISP_REG_GET(DISP_REG_BLS_EN),
                             DISP_REG_GET(DISP_REG_CONFIG_MMSYS_CG_CON0));
        }
    }

    if (engine & DDP_MOD_DISP_RDMA)
    {
        if ((DISP_REG_GET(DISP_REG_RDMA_GLOBAL_CON) & 0x1) == 0 ||
           (DISP_REG_GET(DISP_REG_CONFIG_MMSYS_CG_CON0) & DISP_RDMA_SW_CG_BIT) != 0)
        {
            result = -1;
            DDP_DRV_ERR("rdma abnormal, en=%d, clk=0x%x \n",
                             DISP_REG_GET(DISP_REG_RDMA_GLOBAL_CON),
                             DISP_REG_GET(DISP_REG_CONFIG_MMSYS_CG_CON0));
        }
    }

    if (result != 0)
    {
        DDP_DRV_ERR("engine status error before release mutex, engine=0x%x, mutexID=%d \n", engine, mutexID);
    }

    return result;
}


int disp_path_release_mutex_(int mutexID)
{
    unsigned int reg = 0;
    unsigned int cnt = 0;

    disp_check_engine_status(mutexID);

    DISP_REG_SET(DISP_REG_CONFIG_MUTEX(mutexID), 0);

    while ((DISP_REG_GET(DISP_REG_CONFIG_MUTEX(mutexID)) & DISP_INT_MUTEX_BIT_MASK) != 0)
    {
        if (cnt > 2000)
        {
            DDP_DRV_ERR("disp_path_release_mutex() timeout!\n");
            break;
        }
        mdelay(1);
        cnt ++;
    }

    if (cnt > 2000)
    {
        if ((DISP_REG_GET(DISP_REG_CONFIG_MUTEX_INTSTA) & (1<<(mutexID+8))) == (1<<(mutexID+8)))
        {
            DDP_DRV_ERR("disp_path_release_mutex() timeout! \n");
            disp_dump_reg(DISP_MODULE_CONFIG);

            //print error engine
            reg = DISP_REG_GET(DISP_REG_CONFIG_REG_COMMIT);
            if (reg != 0)
            {
                if (reg & DDP_MOD_DISP_OVL) { DDP_DRV_INFO(" OVL update reg timeout! \n"); disp_dump_reg(DISP_MODULE_OVL); }
                if (reg & DDP_MOD_DISP_WDMA) { DDP_DRV_INFO(" WDMA update reg timeout! \n"); disp_dump_reg(DISP_MODULE_WDMA0); }
                if (reg & DDP_MOD_DISP_PQ) { DDP_DRV_INFO(" PQ update reg timeout! \n"); disp_dump_reg(DISP_MODULE_PQ); }
                if (reg & DDP_MOD_DISP_BLS) { DDP_DRV_INFO(" BLS update reg timeout! \n"); disp_dump_reg(DISP_MODULE_BLS); }
                if (reg & DDP_MOD_DISP_RDMA) { DDP_DRV_INFO(" RDMA update reg timeout! \n"); disp_dump_reg(DISP_MODULE_RDMA0); }
            }

            disp_dump_reg(DISP_MODULE_MUTEX);
            disp_dump_reg(DISP_MODULE_CONFIG);
            //ASSERT(0);

            return - 1;
        }
    }

    MMProfileLog(DDP_MMP_Events.Mutex[mutexID], MMProfileFlagEnd);
    mutex_unlock(&DpEngineMutexLock);
    DDP_DRV_DBG("disp_path_release_mutex %d \n", disp_mutex_unlock_cnt++);

    return 0;
}


int disp_path_wait_reg_update(void)
{
    wait_event_interruptible_timeout(
                    g_disp_mutex_wq,
                    g_disp_mutex_reg_update,
                    HZ/10);
    return 0;
}


int disp_path_change_tdshp_status(unsigned int layer, unsigned int enable)
{
    ASSERT(layer<DDP_OVL_LAYER_MUN);
    DDP_DRV_INFO("disp_path_change_tdshp_status(), layer=%d, enable=%d", layer, enable);

    gTdshpStatus[layer] = enable;
    return 0;
}


///============================================================================
// OVL decouple @{
///========================
void _disp_path_wdma_callback(unsigned int param);

int disp_path_get_mem_read_mutex (void) 
{
    disp_path_get_mutex_(gMutexID);

    return 0;
}

int disp_path_release_mem_read_mutex (void) 
{
    disp_path_release_mutex_(gMutexID);

    return 0;
}

int disp_path_get_mem_write_mutex (void) 
{
    disp_path_get_mutex_(gMemOutMutexID);

    return 0;
}

int disp_path_release_mem_write_mutex (void) 
{
    disp_path_release_mutex_(gMemOutMutexID);

    return 0;
}

#if 0
static int disp_path_get_m4u_moduleId (DISP_MODULE_ENUM module) 
{
    int m4u_module = M4U_PORT_UNKNOWN;

    switch (module) 
    {
        case DISP_MODULE_OVL:
            m4u_module = M4U_PORT_LCD_OVL;
            break;
        case DISP_MODULE_RDMA0:
            m4u_module = M4U_PORT_LCD_R;
            break;
        case DISP_MODULE_WDMA0:
            m4u_module = M4U_PORT_LCD_W;
            break;
        default:
            break;
    }
    
    return m4u_module;
}
#endif

static int disp_path_init_m4u_port (DISP_MODULE_ENUM module) 
{
    int m4u_module = M4U_PORT_UNKNOWN;
    M4U_PORT_STRUCT portStruct;

    switch (module) 
    {
        case DISP_MODULE_OVL:
            m4u_module = M4U_PORT_LCD_OVL;
            break;
        case DISP_MODULE_RDMA0:
            m4u_module = M4U_PORT_LCD_R;
            break;
        case DISP_MODULE_WDMA0:
            m4u_module = M4U_PORT_LCD_W;
            break;
        default:
            break;
    }

    portStruct.ePortID = m4u_module;		   //hardware port ID, defined in M4U_PORT_ID_ENUM
    portStruct.Virtuality = 1;
    portStruct.Security = 0;
    portStruct.domain = 0;            //domain : 0 1 2 3
    portStruct.Distance = 1;
    portStruct.Direction = 0;

    m4u_config_port(&portStruct);

    return 0;
}

#if 0
static int disp_path_deinit_m4u_port (DISP_MODULE_ENUM module) 
{
    int m4u_module = M4U_PORT_UNKNOWN;
    M4U_PORT_STRUCT portStruct;

    switch (module) 
    {
        case DISP_MODULE_OVL:
            m4u_module = M4U_PORT_LCD_OVL;
            break;
        case DISP_MODULE_RDMA0:
            m4u_module = M4U_PORT_LCD_R;
            break;
        case DISP_MODULE_WDMA0:
            m4u_module = M4U_PORT_LCD_W;
            break;
        default:
            break;
    }

    portStruct.ePortID = m4u_module;		   //hardware port ID, defined in M4U_PORT_ID_ENUM
    portStruct.Virtuality = 0;
    portStruct.Security = 0;
    portStruct.domain = 0;            //domain : 0 1 2 3
    portStruct.Distance = 1;
    portStruct.Direction = 0;

    m4u_config_port(&portStruct);

    return 0;
}
#endif

/**
 * In decouple mode, disp_config_update_kthread will call this to reconfigure
 * next buffer to RDMA->LCM
 */
int disp_path_config_rdma (RDMA_CONFIG_STRUCT* pRdmaConfig) 
{
    DISP_DBG("[DDP] config_rdma(), idx=%d, mode=%d, in_fmt=%d, addr=0x%x, out_fmt=%d, pitch=%d, x=%d, y=%d, byteSwap=%d, rgbSwap=%d \n ",
                     pRdmaConfig->idx,
                     pRdmaConfig->mode,       	 // direct link mode
                     pRdmaConfig->inputFormat,    // inputFormat
                     pRdmaConfig->address,        // address
                     pRdmaConfig->outputFormat,   // output format
                     pRdmaConfig->pitch,       	 // pitch
                     pRdmaConfig->width,       	 // width
                     pRdmaConfig->height,      	 // height
                     pRdmaConfig->isByteSwap,  	 // byte swap
                     pRdmaConfig->isRGBSwap);

    // TODO: just need to reconfig buffer address now! because others are configured before
    //RDMASetAddress(pRdmaConfig->idx, pRdmaConfig->address);
    RDMAConfig(pRdmaConfig->idx, pRdmaConfig->mode, pRdmaConfig->inputFormat, pRdmaConfig->address,
    pRdmaConfig->outputFormat, pRdmaConfig->pitch, pRdmaConfig->width,
    pRdmaConfig->height, pRdmaConfig->isByteSwap, pRdmaConfig->isRGBSwap);

    return 0;
}

/**
 * In decouple mode, disp_ovl_worker_kthread will call this to reconfigure
 * next output buffer to OVL-->WDMA
 */
int disp_path_config_wdma (struct disp_path_config_mem_out_struct* pConfig) 
{
    DISP_DBG("[DDP] config_wdma(), idx=%d, dstAddr=%x, out_fmt=%d\n",
                      0,
                      pConfig->dstAddr,
                      pConfig->outFormat);

    // TODO: just need to reconfig buffer address now! because others are configured before
    WDMAConfigAddress(0, pConfig->dstAddr);
    WDMAStart(0);
    
    return 0;
}

/**
 * switch decouple:
 * 1. save mutex0(gMutexID) contents
 * 2. reconfigure OVL->WDMA within mutex0(gMutexID)
 * 3. move engines in mutex0(gMutexID) to mutex1(gLcmMutexID)
 * 4. reconfigure MMSYS_OVL_MOUT
 * 5. reconfigure RDMA mode
 */
int disp_path_switch_ovl_mode (struct disp_path_config_ovl_mode_t *pConfig) 
{
    int reg_mutex_mod;
    //int reg_mutex_sof;

    if (DISP_IsDecoupleMode()) 
    {
        //reg_mutex_sof = DISP_REG_GET(DISP_REG_CONFIG_MUTEX_SOF(gMutexID));
        reg_mutex_mod = DISP_REG_GET(DISP_REG_CONFIG_MUTEX_MOD(gMutexID));
        // ovl mout
        DISP_REG_SET(DISP_REG_CONFIG_DISP_OVL_MOUT_EN, 1<<1);   // ovl_mout output to wdma0
        DISP_REG_SET(DISP_REG_CONFIG_MUTEX_INTEN , 0x0303);
        DISP_REG_SET(DISP_REG_CONFIG_MUTEX_INTSTA, (1<<gMemOutMutexID)|(1<<gMutexID));
        // mutex0
        DISP_REG_SET(DISP_REG_CONFIG_MUTEX_MOD(gMutexID), reg_mutex_mod & (~DDP_MOD_DISP_OVL)); 	//remove OVL
        // mutex1
        DISP_REG_SET(DISP_REG_CONFIG_MUTEX_EN(gMemOutMutexID), 1); 					//enable mutex1
        DISP_REG_SET(DISP_REG_CONFIG_MUTEX_MOD(gMemOutMutexID), DDP_MOD_DISP_OVL | DDP_MOD_DISP_WDMA); 	//OVL, WDMA
        DISP_REG_SET(DISP_REG_CONFIG_MUTEX_SOF(gMemOutMutexID), 0);					//single mode

        disp_register_irq(DISP_MODULE_WDMA0, _disp_path_wdma_callback);

        // config wdma0
        WDMAReset(0);
        WDMAConfig(0,
                   WDMA_INPUT_FORMAT_ARGB,
                   pConfig->roi.width,
                   pConfig->roi.height,
                   pConfig->roi.x,
                   pConfig->roi.y,
                   pConfig->roi.width,
                   pConfig->roi.height,
                   pConfig->format,
                   pConfig->address,
                   pConfig->roi.width,
                   1,
                   0);
        WDMAStart(0);

        // config rdma
        RDMAConfig(0,
                   RDMA_MODE_MEMORY,            // memory mode
                   pConfig->format,             // inputFormat
                   pConfig->address,            // address, will be reset later
                   RDMA_OUTPUT_FORMAT_ARGB,     // output format
                   pConfig->pitch,              // pitch
                   pConfig->roi.width,
                   pConfig->roi.height,
                   0,                           //byte swap
                   0);                          // is RGB swap
        RDMAStart(0);
        //disp_dump_reg(DISP_MODULE_WDMA0);
    } 
    else 
    {
        //reg_mutex_sof = DISP_REG_GET(DISP_REG_CONFIG_MUTEX_SOF(gMemOutMutexID));
        reg_mutex_mod = DISP_REG_GET(DISP_REG_CONFIG_MUTEX_MOD(gMutexID));
        // ovl mout
        DISP_REG_SET(DISP_REG_CONFIG_DISP_OVL_MOUT_EN, 1<<0);   // ovl_mout output to rdma
        //DISP_REG_SET(DISP_REG_CONFIG_MUTEX_EN(gMutexID), 1);
        DISP_REG_SET(DISP_REG_CONFIG_MUTEX_INTEN , 0x0101);
        DISP_REG_SET(DISP_REG_CONFIG_MUTEX_INTSTA, 1<<gMutexID);
        // mutex0
        DISP_REG_SET(DISP_REG_CONFIG_MUTEX_MOD(gMutexID), reg_mutex_mod | DDP_MOD_DISP_OVL); //Add OVL
        //DISP_REG_SET(DISP_REG_CONFIG_MUTEX_SOF(gMutexID), reg_mutex_sof);
        // mutex1
        DISP_REG_SET(DISP_REG_CONFIG_MUTEX_MOD(gMemOutMutexID), 0);
        DISP_REG_SET(DISP_REG_CONFIG_MUTEX_EN(gMemOutMutexID), 0); //disable mutex1

        // config rdma
        RDMAConfig(0,
                   RDMA_MODE_DIRECT_LINK,       // direct link mode
                   pConfig->format,             // inputFormat
                   pConfig->address,            // address
                   RDMA_OUTPUT_FORMAT_ARGB,     // output format
                   pConfig->pitch,              // pitch
                   pConfig->roi.width,
                   pConfig->roi.height,
                   0,                           // byte swap
                   0);                          // is RGB swap

        disp_unregister_irq(DISP_MODULE_WDMA0, _disp_path_wdma_callback);
        RDMAStart(0);
    }

    return 0;
}

int disp_path_wait_frame_done(void)
{
    //wait_event_interruptible(mem_out_wq, mem_out_done);
    //mem_out_done = 0;

    int ret = 0;
    // use timeout version instead of wait-forever
    ret = wait_event_interruptible_timeout(
                    mem_out_wq, 
                    mem_out_done, 
                    disp_ms2jiffies(100) ); // timeout after 2s
                    
    /*wake-up from sleep*/
    if(ret==0) // timeout
    {
        if (DISP_REG_GET(DISP_REG_WDMA_EN))
        {
            DISP_ERR("disp_path_wait_frame_done timeout \n");

            // reset DSI & WDMA engine
            WDMAReset(0);
        }
    }
    else if(ret<0) // intr by a signal
    {
        DISP_ERR("disp_path_wait_frame_done intr by a signal ret=%d \n", ret);
    }

    mem_out_done = 0;   
    return ret;
}
// OVL decouple @}
///========================


//#define MANUAL_DEBUG
/**
 * In D-link mode, disp_config_update_kthread will call this to reconfigure next
 * buffer to OVL
 * In Decouple mode, disp_ovl_worker_kthread will call this to reconfigure next
 * buffer to OVL
 */
int disp_path_config_layer(OVL_CONFIG_STRUCT* pOvlConfig)
{
//    unsigned int reg_addr;

    // ddp force stopping flag
    if (ddp_debug_flag == 1)
    {
        return 0;
    }

    DDP_DRV_DBG("[DDP] config_layer(), layer=%d, en=%d, source=%d, fmt=%d, addr=0x%x, (%d, %d, %d, %d), pitch=%d, keyEn=%d, key=%d, aen=%d, alpha=%d\n ", 
                     pOvlConfig->layer,   // layer
                     pOvlConfig->layer_en,   
                     pOvlConfig->source,   // data source (0=memory)
                     pOvlConfig->fmt, 
                     pOvlConfig->addr, // addr 
                     pOvlConfig->src_w,  // width
                     pOvlConfig->src_h,  // height
                     pOvlConfig->dst_w, // width
                     pOvlConfig->dst_h, // height
                     pOvlConfig->src_pitch, //pitch, pixel number
                     pOvlConfig->keyEn,  //color key
                     pOvlConfig->key,  //color key
                     pOvlConfig->aen, // alpha enable
                     pOvlConfig->alpha);


    // config overlay
    OVLLayerSwitch(pOvlConfig->layer, pOvlConfig->layer_en);

    if(pOvlConfig->layer_en!=0)
    {
        OVLLayerConfig(pOvlConfig->layer,   // layer
                       pOvlConfig->source,   // data source (0=memory)
                       pOvlConfig->fmt, 
                       pOvlConfig->addr, // addr 
                       pOvlConfig->src_x,  // x
                       pOvlConfig->src_y,  // y
                       pOvlConfig->src_w,  // width
                       pOvlConfig->src_h,  // height
                       pOvlConfig->src_pitch, //pitch, pixel number
                       pOvlConfig->dst_x,  // x
                       pOvlConfig->dst_y,  // y
                       pOvlConfig->dst_w, // width
                       pOvlConfig->dst_h, // height
                       pOvlConfig->keyEn,  //color key
                       pOvlConfig->key,  //color key
                       pOvlConfig->aen, // alpha enable
                       pOvlConfig->alpha); // alpha
    }    
    if(pOvlConfig->isTdshp==0)
    {
        gTdshpStatus[pOvlConfig->layer] = 0;
    }
    else
    {
        int i=0;
        for(i=0;i<OVL_LAYER_NUM;i++)
        {
            if(gTdshpStatus[i]==1 && i!=pOvlConfig->layer)  //other layer has already enable tdshp
            {
                DDP_DRV_ERR("enable layer=%d tdshp, but layer=%d has already enable tdshp \n", i, pOvlConfig->layer);
                return -1;
            }
            gTdshpStatus[pOvlConfig->layer] = 1;
        }
    }
    //OVLLayerTdshpEn(pOvlConfig->layer, pOvlConfig->isTdshp);
    OVLLayerTdshpEn(pOvlConfig->layer, 0); //Cvs: de-couple

    //printk("[DDP]disp_path_config_layer() done, addr=0x%x \n", pOvlConfig->addr);

    return 0;
}


int disp_path_config_layer_addr(unsigned int layer, unsigned int addr)
{
    unsigned int reg_addr;

    DDP_DRV_DBG("[DDP]disp_path_config_layer_addr(), layer=%d, addr=0x%x\n ", layer, addr);

    switch(layer)
    {
        case 0:
            DISP_REG_SET(DISP_REG_OVL_L0_ADDR, addr);
            reg_addr = DISP_REG_OVL_L0_ADDR;
            break;
        case 1:
            DISP_REG_SET(DISP_REG_OVL_L1_ADDR, addr);
            reg_addr = DISP_REG_OVL_L1_ADDR;
            break;
        case 2:
            DISP_REG_SET(DISP_REG_OVL_L2_ADDR, addr);
            reg_addr = DISP_REG_OVL_L2_ADDR;
            break;
        case 3:
            DISP_REG_SET(DISP_REG_OVL_L3_ADDR, addr);
            reg_addr = DISP_REG_OVL_L3_ADDR;
            break;
        default:
            DDP_DRV_ERR("unknow layer=%d \n", layer);
    }
   
    return 0;
}

void _disp_path_wdma_callback(unsigned int param)
{
    mem_out_done = 1;
    wake_up_interruptible(&mem_out_wq);
}


void disp_path_wait_mem_out_done(void)
{
    //wait_event_interruptible(mem_out_wq, mem_out_done);
    //mem_out_done = 0;

    int ret = 0;
    // use timeout version instead of wait-forever
    ret = wait_event_interruptible_timeout(
                    mem_out_wq, 
                    mem_out_done, 
                    disp_ms2jiffies(2000) ); // timeout after 2s
                    
    /*wake-up from sleep*/
    if(ret==0) // timeout
    {
        if (DISP_REG_GET(DISP_REG_WDMA_EN))
        {
            DISP_ERR("disp_path_wait_mem_out_done timeout \n");

            // reset DSI & WDMA engine
            WDMAReset(0);
        }
    }
    else if(ret<0) // intr by a signal
    {
        DISP_ERR("disp_path_wait_mem_out_done intr by a signal ret=%d \n", ret);
    }

    mem_out_done = 0;
}


// for video mode, if there are more than one frame between memory_out done and disable WDMA
// mem_out_done will be set to 1, next time user trigger disp_path_wait_mem_out_done() will return 
// directly, in such case, screen capture will dose not work for one time. 
// so we add this func to make sure disp_path_wait_mem_out_done() will be execute everytime.
void disp_path_clear_mem_out_done_flag(void)
{
    mem_out_done = 0;    
}


// just mem->ovl->wdma1->mem, used in suspend mode screen capture
// have to call this function set pConfig->enable=0 to reset configuration
// should call clock_on()/clock_off() if use this function in suspend mode
int disp_path_config_mem_out_without_lcd(struct disp_path_config_mem_out_struct* pConfig)
{
    static unsigned int reg_mutex_mod;
    static unsigned int reg_mutex_sof;
    static unsigned int reg_mout;
    
    DDP_DRV_DBG(" disp_path_config_mem_out(), enable = %d, outFormat=%d, dstAddr=0x%x, ROI(%d,%d,%d,%d) \n",
            pConfig->enable,
            pConfig->outFormat,            
            pConfig->dstAddr,  
            pConfig->srcROI.x, 
            pConfig->srcROI.y, 
            pConfig->srcROI.width, 
            pConfig->srcROI.height);
            
    if(pConfig->enable==1 && pConfig->dstAddr==0)
    {
          DDP_DRV_ERR("pConfig->dstAddr==0! \n");
    }

    if(pConfig->enable==1)
    {
        mem_out_done = 0;
        disp_register_irq(DISP_MODULE_WDMA0, _disp_path_wdma_callback);

        // config wdma0
        WDMAInit(0);
        
        WDMAReset(0);

        if(pConfig->dstAddr==0 ||
           pConfig->srcROI.width==0    ||
           pConfig->srcROI.height==0    )
        {
            DDP_DRV_ERR("wdma parameter invalidate, addr=0x%x, w=%d, h=%d \n",
                   pConfig->dstAddr, 
                   pConfig->srcROI.width,
                   pConfig->srcROI.height);

            disp_unregister_irq(DISP_MODULE_WDMA0, _disp_path_wdma_callback);
            return -1;
        }

        //disp_dump_reg(DISP_MODULE_WDMA0);
        
        WDMAConfig(0, 
                            WDMA_INPUT_FORMAT_ARGB, 
                            pConfig->srcROI.width,
                            pConfig->srcROI.height,
                            0,
                            0,
                            pConfig->srcROI.width, 
                            pConfig->srcROI.height, 
                            pConfig->outFormat, 
                            pConfig->dstAddr, 
                            pConfig->srcROI.width,
                            1, 
                            0);      
        WDMAStart(0);

        // mutex module
        reg_mutex_mod = DISP_REG_GET(DISP_REG_CONFIG_MUTEX_MOD(gMutexID));
        DISP_REG_SET(DISP_REG_CONFIG_MUTEX_MOD(gMutexID), DDP_MOD_DISP_OVL | DDP_MOD_DISP_WDMA); //ovl, wdma0

        // mutex sof
        reg_mutex_sof = DISP_REG_GET(DISP_REG_CONFIG_MUTEX_SOF(gMutexID));
        DISP_REG_SET(DISP_REG_CONFIG_MUTEX_SOF(gMutexID), 0); //single mode
                
        // ovl mout
        reg_mout = DISP_REG_GET(DISP_REG_CONFIG_DISP_OVL_MOUT_EN);
        DISP_REG_SET(DISP_REG_CONFIG_DISP_OVL_MOUT_EN, 0x2);   // ovl_mout output to wdma0
        
        //disp_dump_reg(DISP_MODULE_WDMA0);
    }
    else
    {
        WDMAStop(0);
        WDMAReset(0);
        
        OVLStop();
        OVLReset();    
    
        // mutex
        DISP_REG_SET(DISP_REG_CONFIG_MUTEX_MOD(gMutexID), reg_mutex_mod);
        DISP_REG_SET(DISP_REG_CONFIG_MUTEX_SOF(gMutexID), reg_mutex_sof);         
        // ovl mout
        DISP_REG_SET(DISP_REG_CONFIG_DISP_OVL_MOUT_EN, reg_mout);        

        disp_unregister_irq(DISP_MODULE_WDMA0, _disp_path_wdma_callback);
    }

    return 0;
}


// add wdma0 into the path
// should call get_mutex() / release_mutex for this func
int disp_path_config_mem_out(struct disp_path_config_mem_out_struct* pConfig)
{
    unsigned int reg;


    DDP_DRV_DBG(" disp_path_config_mem_out(), enable = %d, outFormat=%d, dstAddr=0x%x, ROI(%d,%d,%d,%d) \n",
            pConfig->enable,
            pConfig->outFormat,            
            pConfig->dstAddr,  
            pConfig->srcROI.x, 
            pConfig->srcROI.y, 
            pConfig->srcROI.width, 
            pConfig->srcROI.height);

    if(pConfig->enable==1 && pConfig->dstAddr==0)
    {
          DDP_DRV_ERR("pConfig->dstAddr==0! \n");
    }

    if(pConfig->enable==1)
    {
        mem_out_done = 0;
        disp_register_irq(DISP_MODULE_WDMA0, _disp_path_wdma_callback);

        // config wdma0
        WDMAInit(0);
        
        WDMAReset(0);

        if(pConfig->dstAddr==0 ||
           pConfig->srcROI.width==0    ||
           pConfig->srcROI.height==0    )
        {
            DDP_DRV_ERR("wdma parameter invalidate, addr=0x%x, w=%d, h=%d \n",
                   pConfig->dstAddr, 
                   pConfig->srcROI.width,
                   pConfig->srcROI.height);

            disp_unregister_irq(DISP_MODULE_WDMA0, _disp_path_wdma_callback);
            return -1;
        }
                
        WDMAConfig(0, 
                   WDMA_INPUT_FORMAT_ARGB, 
                   pConfig->srcROI.width,
                   pConfig->srcROI.height,
                   0,
                   0,
                   pConfig->srcROI.width, 
                   pConfig->srcROI.height, 
                   pConfig->outFormat, 
                   pConfig->dstAddr, 
                   pConfig->srcROI.width,
                   1, 
                   0);      
        WDMAStart(0);

        // mutex
        reg = DISP_REG_GET(DISP_REG_CONFIG_MUTEX_MOD(gMutexID));
        DISP_REG_SET(DISP_REG_CONFIG_MUTEX_MOD(gMutexID), reg | DDP_MOD_DISP_WDMA); //wdma0=6
        
        // ovl mout
        reg = DISP_REG_GET(DISP_REG_CONFIG_DISP_OVL_MOUT_EN);
        DISP_REG_SET(DISP_REG_CONFIG_DISP_OVL_MOUT_EN, reg|(0x2));   // ovl_mout output to bls
        
        //disp_dump_reg(DISP_MODULE_WDMA0);
    }
    else
    {
        // mutex
        reg = DISP_REG_GET(DISP_REG_CONFIG_MUTEX_MOD(gMutexID));
        DISP_REG_SET(DISP_REG_CONFIG_MUTEX_MOD(gMutexID), reg & (~DDP_MOD_DISP_WDMA)); //wdma0=6
        
        // ovl mout
        reg = DISP_REG_GET(DISP_REG_CONFIG_DISP_OVL_MOUT_EN);
        DISP_REG_SET(DISP_REG_CONFIG_DISP_OVL_MOUT_EN, reg&(~0x2));   // ovl_mout output to bls
        
        // config wdma0
        //WDMAReset(1);
        disp_unregister_irq(DISP_MODULE_WDMA0, _disp_path_wdma_callback);
    }

    return 0;
}

UINT32 fb_width = 0;
UINT32 fb_height = 0;
UINT32 fb_vaddr = 0;

int disp_path_config(struct disp_path_config_struct* pConfig)
{
    fb_width = pConfig->srcROI.width;
    fb_height = pConfig->srcROI.height;
    fb_vaddr = pConfig->ovl_config.vaddr;
    return disp_path_config_(pConfig, gMutexID);
}


DISP_MODULE_ENUM g_dst_module;

int disp_path_config_(struct disp_path_config_struct* pConfig, int mutexId)
{
    unsigned int mutexMode;

    DDP_DRV_DBG("[DDP] disp_path_config(), srcModule=%d, addr=0x%x, inFormat=%d, \n\
                     pitch=%d, bgROI(%d,%d,%d,%d), bgColor=%d, outFormat=%d, dstModule=%d, dstAddr=0x%x, dstPitch=%d, mutexId=%d \n",
                     pConfig->srcModule,
                     pConfig->addr,
                     pConfig->inFormat,
                     pConfig->pitch,
                     pConfig->bgROI.x,
                     pConfig->bgROI.y,
                     pConfig->bgROI.width,
                     pConfig->bgROI.height,
                     pConfig->bgColor,
                     pConfig->outFormat,
                     pConfig->dstModule,
                     pConfig->dstAddr,
                     pConfig->dstPitch,
                     mutexId);
    g_dst_module = pConfig->dstModule;

        switch(pConfig->dstModule)
        {
            case DISP_MODULE_DSI_VDO:
                mutexMode = 1;
            break;

            case DISP_MODULE_DPI0:
                mutexMode = 2;
            break;

            case DISP_MODULE_DBI:
            case DISP_MODULE_DSI_CMD:
            case DISP_MODULE_WDMA0:
                mutexMode = 0;
            break;

            default:
                mutexMode = 0;
               DDP_DRV_ERR("unknown dstModule=%d \n", pConfig->dstModule); 
               return -1;
        }

       
        if (pConfig->srcModule == DISP_MODULE_RDMA0)
        {
           DISP_REG_SET(DISP_REG_CONFIG_MUTEX_MOD(gMutexID), DDP_MOD_DISP_RDMA | DDP_MOD_DISP_PQ | DDP_MOD_DISP_BLS | DDP_MOD_DISP_PWM);
        }
        else
        {
            if (pConfig->dstModule == DISP_MODULE_WDMA0)
            {
               DISP_REG_SET(DISP_REG_CONFIG_MUTEX_MOD(gMutexID), DDP_MOD_DISP_OVL | DDP_MOD_DISP_WDMA);
            }
            else
            {
               DISP_REG_SET(DISP_REG_CONFIG_MUTEX_MOD(gMutexID), DDP_MOD_DISP_OVL | DDP_MOD_DISP_RDMA | DDP_MOD_DISP_PQ | DDP_MOD_DISP_BLS | DDP_MOD_DISP_PWM);
            }
        }		
        DISP_REG_SET(DISP_REG_CONFIG_MUTEX_SOF(gMutexID), mutexMode);
        DISP_REG_SET(DISP_REG_CONFIG_MUTEX_INTSTA, (1 << gMutexID));
        DISP_REG_SET(DISP_REG_CONFIG_MUTEX_INTEN , 0x0d0d);   // enable mutex 0,2,3's intr, other mutex will be used by MDP     
        DISP_REG_SET(DISP_REG_CONFIG_MUTEX_EN(gMutexID), 1);        
 
        if (DISP_IsDecoupleMode()) 
        {
            disp_path_init_m4u_port(DISP_MODULE_RDMA0);
            disp_path_init_m4u_port(DISP_MODULE_WDMA0);
            DISP_REG_SET(DISP_REG_CONFIG_MUTEX_EN(gMemOutMutexID), 1);
            DISP_REG_SET(DISP_REG_CONFIG_MUTEX_INTEN , 0x0F0F);
            /// config OVL-WDMA data path with mutex0
            DISP_REG_SET(DISP_REG_CONFIG_MUTEX_MOD(gMemOutMutexID), DDP_MOD_DISP_OVL | DDP_MOD_DISP_WDMA); // OVL-->WDMA
            DISP_REG_SET(DISP_REG_CONFIG_MUTEX_SOF(gMemOutMutexID), 0x0);// single mode
            DISP_REG_SET(DISP_REG_CONFIG_MUTEX_INTSTA, (1 << gMemOutMutexID)|(1 << mutexId));
        }

        ///> config config reg
        switch(pConfig->dstModule)
        {
            case DISP_MODULE_DSI_VDO:
            case DISP_MODULE_DSI_CMD:
               if (DISP_IsDecoupleMode()) 
               {
                   DISP_REG_SET(DISP_REG_CONFIG_DISP_OVL_MOUT_EN, 0x2);  // OVL output, [0]: DISP_RDMA, [1]: DISP_WDMA, [2]: DISP_PQ
               } 
               else 
               {
                   DISP_REG_SET(DISP_REG_CONFIG_DISP_OVL_MOUT_EN, 0x1);  // OVL output, [0]: DISP_RDMA, [1]: DISP_WDMA, [2]: DISP_PQ
               }
               DISP_REG_SET(DISP_REG_CONFIG_DISP_BLS_SOUT_SEL, 0x0);  // display output, 0: DSI, 1: DPI, 2: DBI
               DISP_REG_SET(DISP_REG_CONFIG_DISP_RDMA_SOUT_SEL, 0x0);  // RDMA output, 0: DISP_PQ, 1: DSI, 2: DPI
               DISP_REG_SET(DISP_REG_CONFIG_DISP_PQ_SEL, 0x0);  // PQ input, 0: DISP_RDMA, 1: DISP_OVL
               DISP_REG_SET(DISP_REG_CONFIG_DISP_DSI_SEL, 0x0);  // DSI input, 0: DISP_BLS, 1: DISP_RDMA
               break;

            case DISP_MODULE_DPI0:
               //printk("DISI_MODULE_DPI0\n");
               if (DISP_IsDecoupleMode()) 
               {
                   DISP_REG_SET(DISP_REG_CONFIG_DISP_OVL_MOUT_EN, 0x2);  // OVL output, [0]: DISP_RDMA, [1]: DISP_WDMA, [2]: DISP_PQ
               } 
               else 
               {
                   DISP_REG_SET(DISP_REG_CONFIG_DISP_OVL_MOUT_EN, 0x1);  // OVL output, [0]: DISP_RDMA, [1]: DISP_WDMA, [2]: DISP_PQ
               }
               DISP_REG_SET(DISP_REG_CONFIG_DISP_BLS_SOUT_SEL, 0x1);  // display output, 0: DSI, 1: DPI, 2: DBI
               DISP_REG_SET(DISP_REG_CONFIG_DISP_RDMA_SOUT_SEL, 0x0);  // RDMA output, 0: DISP_PQ, 1: DSI, 2: DPI
               DISP_REG_SET(DISP_REG_CONFIG_DISP_DPI_SEL, 0);  // DPI input, 0: DISP_BLS, 1: DISP_RDMA
               DISP_REG_SET(DISP_REG_CONFIG_DISP_PQ_SEL, 0x0);  // PQ input, 0: DISP_RDMA, 1: DISP_OVL
               break;

            case DISP_MODULE_DBI:
               if (DISP_IsDecoupleMode()) 
               {
                   DISP_REG_SET(DISP_REG_CONFIG_DISP_OVL_MOUT_EN, 0x2);  // OVL output, [0]: DISP_RDMA, [1]: DISP_WDMA, [2]: DISP_PQ
               } 
               else 
               {
                   DISP_REG_SET(DISP_REG_CONFIG_DISP_OVL_MOUT_EN, 0x1);  // OVL output, [0]: DISP_RDMA, [1]: DISP_WDMA, [2]: DISP_PQ
               }
               DISP_REG_SET(DISP_REG_CONFIG_DISP_BLS_SOUT_SEL, 0x2);  // display output, 0: DSI, 1: DPI, 2: DBI
               DISP_REG_SET(DISP_REG_CONFIG_DISP_RDMA_SOUT_SEL, 0x0);  // RDMA output, 0: DISP_PQ, 1: DSI, 2: DPI
               DISP_REG_SET(DISP_REG_CONFIG_DISP_PQ_SEL, 0x0);  // PQ input, 0: DISP_RDMA, 1: DISP_OVL
               break;

            case DISP_MODULE_WDMA0:
                DISP_REG_SET(DISP_REG_CONFIG_DISP_OVL_MOUT_EN, 0x1<<1);   // wdma0
            break;

            default:
               printk("[DDP] error! unknown dstModule=%d \n", pConfig->dstModule); 
        }    
        
        ///> config engines
        {            // config OVL
            // ovl initialization
            OVLInit();
            
            OVLROI(pConfig->bgROI.width, // width
                   pConfig->bgROI.height, // height
                   pConfig->bgColor);// background B

            if(pConfig->dstModule!=DISP_MODULE_DSI_VDO && pConfig->dstModule!=DISP_MODULE_DPI0)
            {
                OVLStop();
                OVLReset();
            }
            if(pConfig->ovl_config.layer<4)
            {
                OVLLayerSwitch(pConfig->ovl_config.layer, pConfig->ovl_config.layer_en);
                if(pConfig->ovl_config.layer_en!=0)
                {
                    if(pConfig->ovl_config.addr==0 ||
                       pConfig->ovl_config.dst_w==0    ||
                       pConfig->ovl_config.dst_h==0    )
                    {
                        DDP_DRV_ERR("ovl parameter invalidate, addr=0x%x, w=%d, h=%d \n",
                               pConfig->ovl_config.addr, 
                               pConfig->ovl_config.dst_w,
                               pConfig->ovl_config.dst_h);
                        return -1;
                    }
                
                    OVLLayerConfig(pConfig->ovl_config.layer,   // layer
                                   pConfig->ovl_config.source,   // data source (0=memory)
                                   pConfig->ovl_config.fmt, 
                                   pConfig->ovl_config.addr, // addr 
                                   pConfig->ovl_config.src_x,  // x
                                   pConfig->ovl_config.src_y,  // y
                                   pConfig->ovl_config.src_w,  // width
                                   pConfig->ovl_config.src_h,  // height
                                   pConfig->ovl_config.src_pitch, //pitch, pixel number
                                   pConfig->ovl_config.dst_x,  // x
                                   pConfig->ovl_config.dst_y,  // y
                                   pConfig->ovl_config.dst_w, // width
                                   pConfig->ovl_config.dst_h, // height
                                   pConfig->ovl_config.keyEn,  //color key
                                   pConfig->ovl_config.key,  //color key
                                   pConfig->ovl_config.aen, // alpha enable
                                   pConfig->ovl_config.alpha); // alpha
                }
            }
            else
            {
                DDP_DRV_ERR("layer ID undefined! %d \n", pConfig->ovl_config.layer);
            }
            OVLStart();

            if(pConfig->dstModule==DISP_MODULE_WDMA0)  //1. mem->ovl->wdma0->mem
            {
                WDMAInit(0);

                WDMAReset(0);

                if(pConfig->dstAddr==0 ||
                   pConfig->srcROI.width==0    ||
                   pConfig->srcROI.height==0    )
                {
                    DDP_DRV_ERR("wdma parameter invalidate, addr=0x%x, w=%d, h=%d \n",
                           pConfig->dstAddr, 
                           pConfig->srcROI.width,
                           pConfig->srcROI.height);
                    return -1;
                }

                WDMAConfig(0, 
                           WDMA_INPUT_FORMAT_ARGB, 
                           pConfig->srcROI.width, 
                           pConfig->srcROI.height, 
                           0, 
                           0, 
                           pConfig->srcROI.width, 
                           pConfig->srcROI.height, 
                           pConfig->outFormat, 
                           pConfig->dstAddr, 
                           pConfig->srcROI.width, 
                           1, 
                           0);      
                WDMAStart(0);
            }
            else    //2. ovl->bls->rdma0->lcd
            {
                disp_bls_init(pConfig->srcROI.width, pConfig->srcROI.height);

               //=============================config PQ start==================================				 
                disp_pq_init();
                disp_pq_config(pConfig->srcROI.width, pConfig->srcROI.height);


                //=============================config PQ end==================================
                ///config RDMA
                if(pConfig->dstModule!=DISP_MODULE_DSI_VDO && pConfig->dstModule!=DISP_MODULE_DPI0)
                {
                    RDMAInit(0);
                    RDMAStop(0);
                    RDMAReset(0);
                }
                if(pConfig->srcROI.width==0    ||
                   pConfig->srcROI.height==0    )
                {
                    DDP_DRV_ERR("rdma parameter invalidate, w=%d, h=%d \n",
                           pConfig->srcROI.width,
                           pConfig->srcROI.height);
                    return -1;
                }
                if (DISP_IsDecoupleMode())
                {
                    printk("from de-couple\n");
                    WDMAReset(0);

                    if (decouple_addr == 0 ||
                        pConfig->srcROI.width == 0 ||
                        pConfig->srcROI.height == 0)
                    {
                        DISP_ERR("wdma parameter invalidate, addr=0x%x, w=%d, h=%d \n",
                                         decouple_addr,
                                         pConfig->srcROI.width,
                                         pConfig->srcROI.height);

                        return -1;
                    }

                    WDMAConfig(0,
                               WDMA_INPUT_FORMAT_ARGB,
                               pConfig->srcROI.width,
                               pConfig->srcROI.height,
                               0,
                               0,
                               pConfig->srcROI.width,
                               pConfig->srcROI.height,
                               eRGB888,
                               decouple_addr,
                               pConfig->srcROI.width,
                               1,
                               0);
                    WDMAStart(0);
                    // Register WDMA intr
                    mem_out_done = 0;
                    disp_register_irq(DISP_MODULE_WDMA0, _disp_path_wdma_callback);
                    
                    RDMAConfig(0,
                               RDMA_MODE_MEMORY,            // mem mode
                               eRGB888,                     // inputFormat
                               decouple_addr,               // display lk logo when entering kernel
                               RDMA_OUTPUT_FORMAT_ARGB,     // output format
                               pConfig->srcROI.width*3,     // pitch, eRGB888
                               pConfig->srcROI.width,       // width
                               pConfig->srcROI.height,      // height
                               0,                           // byte swap
                               0);
                    RDMASetTargetLine(0, DISP_GetScreenHeight()*4/5);
                    RDMAStart(0);
                }
                else
                {
                    RDMAConfig(0, 
                               RDMA_MODE_DIRECT_LINK,       ///direct link mode
                               eRGB888,                     // inputFormat
                               (unsigned int)NULL,          // address
                               pConfig->outFormat,          // output format
                               pConfig->pitch,              // pitch
                               pConfig->srcROI.width,       // width
                               pConfig->srcROI.height,      // height
                               0,                           //byte swap
                               0);                          // is RGB swap        
                    RDMASetTargetLine(0, pConfig->srcROI.height*4/5);
                    RDMAStart(0);
                }
            }
        }

    printk("DDP Config = 0x%08X\n", get_devinfo_with_index(16));

#if 0
        disp_dump_reg(DISP_MODULE_OVL);
        disp_dump_reg(DISP_MODULE_WDMA0);
        disp_dump_reg(DISP_MODULE_PQ);
        disp_dump_reg(DISP_MODULE_BLS);
        disp_dump_reg(DISP_MODULE_DPI0);
        disp_dump_reg(DISP_MODULE_RDMA0);
        disp_dump_reg(DISP_MODULE_CONFIG);
        disp_dump_reg(DISP_MODULE_MUTEX);
#endif

/*************************************************/
// Ultra config
    // ovl ultra 0x40402020
    DISP_REG_SET(DISP_REG_OVL_RDMA0_MEM_GMC_SETTING1, 0x40402020);
    DISP_REG_SET(DISP_REG_OVL_RDMA1_MEM_GMC_SETTING1, 0x40402020);
    DISP_REG_SET(DISP_REG_OVL_RDMA2_MEM_GMC_SETTING1, 0x40402020);
    DISP_REG_SET(DISP_REG_OVL_RDMA3_MEM_GMC_SETTING1, 0x40402020);
    // disp_rdma1 ultra
    DISP_REG_SET(DISP_REG_RDMA_MEM_GMC_SETTING_0, 0x20402040);
    // disp_wdma0 ultra
    //DISP_REG_SET(DISP_REG_WDMA_BUF_CON1, 0x10000000);
    //DISP_REG_SET(DISP_REG_WDMA_BUF_CON2, 0x20402020);

    return 0;
}

extern unsigned int* pRegBackup;
unsigned int reg_offset = 0;
unsigned int disp_intr_status[DISP_MODULE_MAX] = {0};

// #define DDP_RECORD_REG_BACKUP_RESTORE  // print the reg value before backup and after restore

static void _reg_backup(unsigned int reg_addr)
{
   *(pRegBackup+reg_offset) = DISP_REG_GET(reg_addr);
#ifdef DDP_RECORD_REG_BACKUP_RESTORE
      printk("0x%08x(0x%08x), ", reg_addr, *(pRegBackup+reg_offset));
      if((reg_offset+1)%8==0)
          printk("\n");
#endif      
      reg_offset++;
      if(reg_offset>=DDP_BACKUP_REG_NUM)
      {
          DDP_DRV_ERR("_reg_backup fail, reg_offset=%d, regBackupSize=%d \n", reg_offset, DDP_BACKUP_REG_NUM);        
      }
}


static void _reg_restore(unsigned int reg_addr)
{
      DISP_REG_SET(reg_addr, *(pRegBackup+reg_offset));
#ifdef DDP_RECORD_REG_BACKUP_RESTORE
      printk("0x%08x(0x%08x), ", reg_addr, DISP_REG_GET(reg_addr));
      if((reg_offset+1)%8==0)
          printk("\n");
#endif                
      reg_offset++;
      
      if(reg_offset>=DDP_BACKUP_REG_NUM)
      {
          DDP_DRV_ERR("_reg_backup fail, reg_offset=%d, regBackupSize=%d \n", reg_offset, DDP_BACKUP_REG_NUM);        
      }
}


static int _disp_intr_restore(void)
{
    // restore intr enable reg 
    DISP_REG_SET(DISP_REG_OVL_INTEN, disp_intr_status[DISP_MODULE_OVL]);
    DISP_REG_SET(DISP_REG_WDMA_INTEN, disp_intr_status[DISP_MODULE_WDMA0]);
    DISP_REG_SET(DISP_REG_RDMA_INT_ENABLE, disp_intr_status[DISP_MODULE_RDMA0]);
    DISP_REG_SET(DISP_REG_CONFIG_MUTEX_INTEN, disp_intr_status[DISP_MODULE_MUTEX]);

    return 0;
}


// TODO: color, tdshp, gamma, bls, cmdq intr management should add later
static int _disp_intr_disable_and_clear(void)
{
    // backup intr enable reg
    disp_intr_status[DISP_MODULE_OVL] = DISP_REG_GET(DISP_REG_OVL_INTEN);
    disp_intr_status[DISP_MODULE_WDMA0] = DISP_REG_GET(DISP_REG_WDMA_INTEN);
    disp_intr_status[DISP_MODULE_RDMA0] = DISP_REG_GET(DISP_REG_RDMA_INT_ENABLE);
    disp_intr_status[DISP_MODULE_MUTEX] = DISP_REG_GET(DISP_REG_CONFIG_MUTEX_INTEN);

    // disable intr
    DISP_REG_SET(DISP_REG_OVL_INTEN, 0);
    DISP_REG_SET(DISP_REG_WDMA_INTEN, 0);
    DISP_REG_SET(DISP_REG_RDMA_INT_ENABLE, 0);
    DISP_REG_SET(DISP_REG_CONFIG_MUTEX_INTEN, 0);

    // clear intr status
    DISP_REG_SET(DISP_REG_OVL_INTSTA, 0);
    DISP_REG_SET(DISP_REG_WDMA_INTSTA, 0);
    DISP_REG_SET(DISP_REG_RDMA_INT_STATUS, 0);
    DISP_REG_SET(DISP_REG_CONFIG_MUTEX_INTSTA, 0);

    return 0;
}


int disp_mutex_backup(void)
{
    int i;

    _reg_backup(DISP_REG_CONFIG_REG_UPD_TIMEOUT);

    for (i=0; i<DDP_MAX_MUTEX_MUN; i++)
    {
        _reg_backup(DISP_REG_CONFIG_MUTEX_EN(i));
        _reg_backup(DISP_REG_CONFIG_MUTEX_MOD(i));
        _reg_backup(DISP_REG_CONFIG_MUTEX_SOF(i));
    }

    return 0;
}


int disp_bls_backup(void)
{
    int i;

    _reg_backup(DISP_REG_BLS_DEBUG);
    _reg_backup(DISP_REG_BLS_PWM_DUTY);

    _reg_backup(DISP_REG_BLS_BLS_SETTING);
    _reg_backup(DISP_REG_BLS_SRC_SIZE);
    _reg_backup(DISP_REG_BLS_GAMMA_SETTING);

    /* BLS Luminance LUT */
    for (i=0; i<=32; i++)
    {
        _reg_backup(DISP_REG_BLS_LUMINANCE(i));
    }

    /* BLS Luminance 255 */
    _reg_backup(DISP_REG_BLS_LUMINANCE_255);

    /* Dither */
    for (i=0; i<18; i++)
    {
        _reg_backup(DISP_REG_BLS_DITHER(i));
    }

    _reg_backup(DISP_REG_BLS_INTEN);
    _reg_backup(DISP_REG_BLS_EN);

    return 0;
}


int disp_reg_backup(void)
{
    reg_offset = 0;
    DDP_DRV_INFO("disp_reg_backup() start, *pRegBackup=0x%x, reg_offset=%d  \n", *pRegBackup, reg_offset);

    // Mutex
    disp_mutex_backup();

    // Config
    _reg_backup(DISP_REG_CONFIG_DISP_OVL_MOUT_EN);
    _reg_backup(DISP_REG_CONFIG_DISP_BLS_SOUT_SEL);
    _reg_backup(DISP_REG_CONFIG_DISP_RDMA_SOUT_SEL);
    _reg_backup(DISP_REG_CONFIG_DISP_PQ_SEL);
    _reg_backup(DISP_REG_CONFIG_DISP_DSI_SEL);
    _reg_backup(DISP_REG_CONFIG_DISP_DPI_SEL);

    // OVL
    _reg_backup(DISP_REG_OVL_STA);
    _reg_backup(DISP_REG_OVL_INTEN);
    _reg_backup(DISP_REG_OVL_INTSTA);
    _reg_backup(DISP_REG_OVL_EN);
    _reg_backup(DISP_REG_OVL_TRIG);
    _reg_backup(DISP_REG_OVL_RST);
    _reg_backup(DISP_REG_OVL_ROI_SIZE);
    _reg_backup(DISP_REG_OVL_DATAPATH_CON);
    _reg_backup(DISP_REG_OVL_ROI_BGCLR);
    _reg_backup(DISP_REG_OVL_L0_CON);
    _reg_backup(DISP_REG_OVL_L0_SRCKEY);
    _reg_backup(DISP_REG_OVL_L0_SRC_SIZE);
    _reg_backup(DISP_REG_OVL_L0_OFFSET);
    _reg_backup(DISP_REG_OVL_L0_ADDR);
    _reg_backup(DISP_REG_OVL_L0_PITCH);
    _reg_backup(DISP_REG_OVL_L1_CON);
    _reg_backup(DISP_REG_OVL_L1_SRCKEY);
    _reg_backup(DISP_REG_OVL_L1_SRC_SIZE);
    _reg_backup(DISP_REG_OVL_L1_OFFSET);
    _reg_backup(DISP_REG_OVL_L1_ADDR);
    _reg_backup(DISP_REG_OVL_L1_PITCH);
    _reg_backup(DISP_REG_OVL_L2_CON);
    _reg_backup(DISP_REG_OVL_L2_SRCKEY);
    _reg_backup(DISP_REG_OVL_L2_SRC_SIZE);
    _reg_backup(DISP_REG_OVL_L2_OFFSET);
    _reg_backup(DISP_REG_OVL_L2_ADDR);
    _reg_backup(DISP_REG_OVL_L2_PITCH);
    _reg_backup(DISP_REG_OVL_L3_CON);
    _reg_backup(DISP_REG_OVL_L3_SRCKEY);
    _reg_backup(DISP_REG_OVL_L3_SRC_SIZE);
    _reg_backup(DISP_REG_OVL_L3_OFFSET);
    _reg_backup(DISP_REG_OVL_L3_ADDR);
    _reg_backup(DISP_REG_OVL_L3_PITCH);
    _reg_backup(DISP_REG_OVL_RDMA0_CTRL);
    _reg_backup(DISP_REG_OVL_RDMA0_MEM_GMC_SETTING1);
    _reg_backup(DISP_REG_OVL_RDMA0_MEM_SLOW_CON);
    _reg_backup(DISP_REG_OVL_RDMA0_FIFO_CTRL);
    _reg_backup(DISP_REG_OVL_RDMA1_CTRL);
    _reg_backup(DISP_REG_OVL_RDMA1_MEM_GMC_SETTING1);
    _reg_backup(DISP_REG_OVL_RDMA1_MEM_SLOW_CON);
    _reg_backup(DISP_REG_OVL_RDMA1_FIFO_CTRL);
    _reg_backup(DISP_REG_OVL_RDMA2_CTRL);
    _reg_backup(DISP_REG_OVL_RDMA2_MEM_GMC_SETTING1);
    _reg_backup(DISP_REG_OVL_RDMA2_MEM_SLOW_CON);
    _reg_backup(DISP_REG_OVL_RDMA2_FIFO_CTRL);
    _reg_backup(DISP_REG_OVL_RDMA3_CTRL);
    _reg_backup(DISP_REG_OVL_RDMA3_MEM_GMC_SETTING1);
    _reg_backup(DISP_REG_OVL_RDMA3_MEM_SLOW_CON);
    _reg_backup(DISP_REG_OVL_RDMA3_FIFO_CTRL);
    _reg_backup(DISP_REG_OVL_L0_Y2R_PARA_R0);
    _reg_backup(DISP_REG_OVL_L0_Y2R_PARA_R1);
    _reg_backup(DISP_REG_OVL_L0_Y2R_PARA_G0);
    _reg_backup(DISP_REG_OVL_L0_Y2R_PARA_G1);
    _reg_backup(DISP_REG_OVL_L0_Y2R_PARA_B0);
    _reg_backup(DISP_REG_OVL_L0_Y2R_PARA_B1);
    _reg_backup(DISP_REG_OVL_L0_Y2R_PARA_YUV_A_0);
    _reg_backup(DISP_REG_OVL_L0_Y2R_PARA_YUV_A_1);
    _reg_backup(DISP_REG_OVL_L0_Y2R_PARA_RGB_A_0);
    _reg_backup(DISP_REG_OVL_L0_Y2R_PARA_RGB_A_1);
    _reg_backup(DISP_REG_OVL_L1_Y2R_PARA_R0);
    _reg_backup(DISP_REG_OVL_L1_Y2R_PARA_R1);
    _reg_backup(DISP_REG_OVL_L1_Y2R_PARA_G0);
    _reg_backup(DISP_REG_OVL_L1_Y2R_PARA_G1);
    _reg_backup(DISP_REG_OVL_L1_Y2R_PARA_B0);
    _reg_backup(DISP_REG_OVL_L1_Y2R_PARA_B1);
    _reg_backup(DISP_REG_OVL_L1_Y2R_PARA_YUV_A_0);
    _reg_backup(DISP_REG_OVL_L1_Y2R_PARA_YUV_A_1);
    _reg_backup(DISP_REG_OVL_L1_Y2R_PARA_RGB_A_0);
    _reg_backup(DISP_REG_OVL_L1_Y2R_PARA_RGB_A_1);
    _reg_backup(DISP_REG_OVL_L2_Y2R_PARA_R0);
    _reg_backup(DISP_REG_OVL_L2_Y2R_PARA_R1);
    _reg_backup(DISP_REG_OVL_L2_Y2R_PARA_G0);
    _reg_backup(DISP_REG_OVL_L2_Y2R_PARA_G1);
    _reg_backup(DISP_REG_OVL_L2_Y2R_PARA_B0);
    _reg_backup(DISP_REG_OVL_L2_Y2R_PARA_B1);
    _reg_backup(DISP_REG_OVL_L2_Y2R_PARA_YUV_A_0);
    _reg_backup(DISP_REG_OVL_L2_Y2R_PARA_YUV_A_1);
    _reg_backup(DISP_REG_OVL_L2_Y2R_PARA_RGB_A_0);
    _reg_backup(DISP_REG_OVL_L2_Y2R_PARA_RGB_A_1);
    _reg_backup(DISP_REG_OVL_L3_Y2R_PARA_R0);
    _reg_backup(DISP_REG_OVL_L3_Y2R_PARA_R1);
    _reg_backup(DISP_REG_OVL_L3_Y2R_PARA_G0);
    _reg_backup(DISP_REG_OVL_L3_Y2R_PARA_G1);
    _reg_backup(DISP_REG_OVL_L3_Y2R_PARA_B0);
    _reg_backup(DISP_REG_OVL_L3_Y2R_PARA_B1);
    _reg_backup(DISP_REG_OVL_L3_Y2R_PARA_YUV_A_0);
    _reg_backup(DISP_REG_OVL_L3_Y2R_PARA_YUV_A_1);
    _reg_backup(DISP_REG_OVL_L3_Y2R_PARA_RGB_A_0);
    _reg_backup(DISP_REG_OVL_L3_Y2R_PARA_RGB_A_1);
    _reg_backup(DISP_REG_OVL_DEBUG_MON_SEL);
    _reg_backup(DISP_REG_OVL_RDMA0_MEM_GMC_SETTING2);
    _reg_backup(DISP_REG_OVL_RDMA1_MEM_GMC_SETTING2);
    _reg_backup(DISP_REG_OVL_RDMA2_MEM_GMC_SETTING2);
    _reg_backup(DISP_REG_OVL_RDMA3_MEM_GMC_SETTING2);
    _reg_backup(DISP_REG_OVL_FLOW_CTRL_DBG);
    _reg_backup(DISP_REG_OVL_ADDCON_DBG);

    // RDMA
    _reg_backup(DISP_REG_RDMA_INT_ENABLE);
    _reg_backup(DISP_REG_RDMA_INT_STATUS);
    _reg_backup(DISP_REG_RDMA_GLOBAL_CON);
    _reg_backup(DISP_REG_RDMA_SIZE_CON_0);
    _reg_backup(DISP_REG_RDMA_SIZE_CON_1);
    _reg_backup(DISP_REG_RDMA_TARGET_LINE);
    _reg_backup(DISP_REG_RDMA_MEM_CON);
    _reg_backup(DISP_REG_RDMA_MEM_START_ADDR);
    _reg_backup(DISP_REG_RDMA_MEM_SRC_PITCH);
    _reg_backup(DISP_REG_RDMA_MEM_GMC_SETTING_0);
    _reg_backup(DISP_REG_RDMA_MEM_SLOW_CON);
    _reg_backup(DISP_REG_RDMA_MEM_GMC_SETTING_1);
    _reg_backup(DISP_REG_RDMA_FIFO_CON);
    _reg_backup(DISP_REG_RDMA_FIFO_LOG);
    _reg_backup(DISP_REG_RDMA_C00);
    _reg_backup(DISP_REG_RDMA_C01);
    _reg_backup(DISP_REG_RDMA_C02);
    _reg_backup(DISP_REG_RDMA_C10);
    _reg_backup(DISP_REG_RDMA_C11);
    _reg_backup(DISP_REG_RDMA_C12);
    _reg_backup(DISP_REG_RDMA_C20);
    _reg_backup(DISP_REG_RDMA_C21);
    _reg_backup(DISP_REG_RDMA_C22);
    _reg_backup(DISP_REG_RDMA_PRE_ADD_0);
    _reg_backup(DISP_REG_RDMA_PRE_ADD_1);
    _reg_backup(DISP_REG_RDMA_PRE_ADD_2);
    _reg_backup(DISP_REG_RDMA_POST_ADD_0);
    _reg_backup(DISP_REG_RDMA_POST_ADD_1);
    _reg_backup(DISP_REG_RDMA_POST_ADD_2);
    _reg_backup(DISP_REG_RDMA_DUMMY);
    _reg_backup(DISP_REG_RDMA_DEBUG_OUT_SEL);

    // WDMA
    _reg_backup(DISP_REG_WDMA_INTEN);
    _reg_backup(DISP_REG_WDMA_INTSTA);
    _reg_backup(DISP_REG_WDMA_EN);
    _reg_backup(DISP_REG_WDMA_RST);
    _reg_backup(DISP_REG_WDMA_SMI_CON);
    _reg_backup(DISP_REG_WDMA_CFG);
    _reg_backup(DISP_REG_WDMA_SRC_SIZE);
    _reg_backup(DISP_REG_WDMA_CLIP_SIZE);
    _reg_backup(DISP_REG_WDMA_CLIP_COORD);
    _reg_backup(DISP_REG_WDMA_DST_ADDR0);
    _reg_backup(DISP_REG_WDMA_DST_W_IN_BYTE);
    _reg_backup(DISP_REG_WDMA_ALPHA);
    _reg_backup(DISP_REG_WDMA_BUF_CON1);
    _reg_backup(DISP_REG_WDMA_BUF_CON2);
    _reg_backup(DISP_REG_WDMA_C00);
    _reg_backup(DISP_REG_WDMA_C02);
    _reg_backup(DISP_REG_WDMA_C10);
    _reg_backup(DISP_REG_WDMA_C12);
    _reg_backup(DISP_REG_WDMA_C20);
    _reg_backup(DISP_REG_WDMA_C22);
    _reg_backup(DISP_REG_WDMA_PRE_ADD0);
    _reg_backup(DISP_REG_WDMA_PRE_ADD2);
    _reg_backup(DISP_REG_WDMA_POST_ADD0);
    _reg_backup(DISP_REG_WDMA_POST_ADD2);
    _reg_backup(DISP_REG_WDMA_DST_ADDR1);
    _reg_backup(DISP_REG_WDMA_DST_ADDR2);
    _reg_backup(DISP_REG_WDMA_DST_UV_PITCH);
    _reg_backup(DISP_REG_WDMA_FLOW_CTRL_DBG);
    _reg_backup(DISP_REG_WDMA_EXEC_DBG);
    _reg_backup(DISP_REG_WDMA_CT_DBG);

    DDP_DRV_INFO("disp_reg_backup() end, *pRegBackup=0x%x, reg_offset=%d \n", *pRegBackup, reg_offset);

    return 0; 
}


int disp_mutex_restore(void)
{
    int i;

    _reg_restore(DISP_REG_CONFIG_REG_UPD_TIMEOUT);

    for (i=0; i<DDP_MAX_MUTEX_MUN; i++)
    {
        _reg_restore(DISP_REG_CONFIG_MUTEX_EN(i));
        _reg_restore(DISP_REG_CONFIG_MUTEX_MOD(i));
        _reg_restore(DISP_REG_CONFIG_MUTEX_SOF(i));
    }

    return 0;
}


int disp_bls_restore(void)
{
    int i;

    _reg_restore(DISP_REG_BLS_DEBUG);
    _reg_restore(DISP_REG_BLS_PWM_DUTY);

    _reg_restore(DISP_REG_BLS_BLS_SETTING);
    _reg_restore(DISP_REG_BLS_SRC_SIZE);
    _reg_restore(DISP_REG_BLS_GAMMA_SETTING);

    /* BLS Luminance LUT */
    for (i=0; i<=32; i++)
    {
        _reg_restore(DISP_REG_BLS_LUMINANCE(i));
    }

    /* BLS Luminance 255 */
    _reg_restore(DISP_REG_BLS_LUMINANCE_255);

    /* Dither */
    for (i=0; i<18; i++)
    {
        _reg_restore(DISP_REG_BLS_DITHER(i));
    }

    _reg_restore(DISP_REG_BLS_INTEN);
    _reg_restore(DISP_REG_BLS_EN);

    return 0; 
}


int disp_reg_restore(void)
{
    reg_offset = 0;
    DDP_DRV_INFO("disp_reg_restore(*) start, *pRegBackup=0x%x, reg_offset=%d  \n", *pRegBackup, reg_offset);

    // Mutex
    disp_mutex_restore();

    // Config
    _reg_restore(DISP_REG_CONFIG_DISP_OVL_MOUT_EN);
    _reg_restore(DISP_REG_CONFIG_DISP_BLS_SOUT_SEL);
    _reg_restore(DISP_REG_CONFIG_DISP_RDMA_SOUT_SEL);
    _reg_restore(DISP_REG_CONFIG_DISP_PQ_SEL);
    _reg_restore(DISP_REG_CONFIG_DISP_DSI_SEL);
    _reg_restore(DISP_REG_CONFIG_DISP_DPI_SEL);

    // OVL
    _reg_restore(DISP_REG_OVL_STA);
    _reg_restore(DISP_REG_OVL_INTEN);
    _reg_restore(DISP_REG_OVL_INTSTA);
    _reg_restore(DISP_REG_OVL_EN);
    _reg_restore(DISP_REG_OVL_TRIG);
    _reg_restore(DISP_REG_OVL_RST);
    _reg_restore(DISP_REG_OVL_ROI_SIZE);
    _reg_restore(DISP_REG_OVL_DATAPATH_CON);
    _reg_restore(DISP_REG_OVL_ROI_BGCLR);
    _reg_restore(DISP_REG_OVL_L0_CON);
    _reg_restore(DISP_REG_OVL_L0_SRCKEY);
    _reg_restore(DISP_REG_OVL_L0_SRC_SIZE);
    _reg_restore(DISP_REG_OVL_L0_OFFSET);
    _reg_restore(DISP_REG_OVL_L0_ADDR);
    _reg_restore(DISP_REG_OVL_L0_PITCH);
    _reg_restore(DISP_REG_OVL_L1_CON);
    _reg_restore(DISP_REG_OVL_L1_SRCKEY);
    _reg_restore(DISP_REG_OVL_L1_SRC_SIZE);
    _reg_restore(DISP_REG_OVL_L1_OFFSET);
    _reg_restore(DISP_REG_OVL_L1_ADDR);
    _reg_restore(DISP_REG_OVL_L1_PITCH);
    _reg_restore(DISP_REG_OVL_L2_CON);
    _reg_restore(DISP_REG_OVL_L2_SRCKEY);
    _reg_restore(DISP_REG_OVL_L2_SRC_SIZE);
    _reg_restore(DISP_REG_OVL_L2_OFFSET);
    _reg_restore(DISP_REG_OVL_L2_ADDR);
    _reg_restore(DISP_REG_OVL_L2_PITCH);
    _reg_restore(DISP_REG_OVL_L3_CON);
    _reg_restore(DISP_REG_OVL_L3_SRCKEY);
    _reg_restore(DISP_REG_OVL_L3_SRC_SIZE);
    _reg_restore(DISP_REG_OVL_L3_OFFSET);
    _reg_restore(DISP_REG_OVL_L3_ADDR);
    _reg_restore(DISP_REG_OVL_L3_PITCH);
    _reg_restore(DISP_REG_OVL_RDMA0_CTRL);
    _reg_restore(DISP_REG_OVL_RDMA0_MEM_GMC_SETTING1);
    _reg_restore(DISP_REG_OVL_RDMA0_MEM_SLOW_CON);
    _reg_restore(DISP_REG_OVL_RDMA0_FIFO_CTRL);
    _reg_restore(DISP_REG_OVL_RDMA1_CTRL);
    _reg_restore(DISP_REG_OVL_RDMA1_MEM_GMC_SETTING1);
    _reg_restore(DISP_REG_OVL_RDMA1_MEM_SLOW_CON);
    _reg_restore(DISP_REG_OVL_RDMA1_FIFO_CTRL);
    _reg_restore(DISP_REG_OVL_RDMA2_CTRL);
    _reg_restore(DISP_REG_OVL_RDMA2_MEM_GMC_SETTING1);
    _reg_restore(DISP_REG_OVL_RDMA2_MEM_SLOW_CON);
    _reg_restore(DISP_REG_OVL_RDMA2_FIFO_CTRL);
    _reg_restore(DISP_REG_OVL_RDMA3_CTRL);
    _reg_restore(DISP_REG_OVL_RDMA3_MEM_GMC_SETTING1);
    _reg_restore(DISP_REG_OVL_RDMA3_MEM_SLOW_CON);
    _reg_restore(DISP_REG_OVL_RDMA3_FIFO_CTRL);
    _reg_restore(DISP_REG_OVL_L0_Y2R_PARA_R0);
    _reg_restore(DISP_REG_OVL_L0_Y2R_PARA_R1);
    _reg_restore(DISP_REG_OVL_L0_Y2R_PARA_G0);
    _reg_restore(DISP_REG_OVL_L0_Y2R_PARA_G1);
    _reg_restore(DISP_REG_OVL_L0_Y2R_PARA_B0);
    _reg_restore(DISP_REG_OVL_L0_Y2R_PARA_B1);
    _reg_restore(DISP_REG_OVL_L0_Y2R_PARA_YUV_A_0);
    _reg_restore(DISP_REG_OVL_L0_Y2R_PARA_YUV_A_1);
    _reg_restore(DISP_REG_OVL_L0_Y2R_PARA_RGB_A_0);
    _reg_restore(DISP_REG_OVL_L0_Y2R_PARA_RGB_A_1);
    _reg_restore(DISP_REG_OVL_L1_Y2R_PARA_R0);
    _reg_restore(DISP_REG_OVL_L1_Y2R_PARA_R1);
    _reg_restore(DISP_REG_OVL_L1_Y2R_PARA_G0);
    _reg_restore(DISP_REG_OVL_L1_Y2R_PARA_G1);
    _reg_restore(DISP_REG_OVL_L1_Y2R_PARA_B0);
    _reg_restore(DISP_REG_OVL_L1_Y2R_PARA_B1);
    _reg_restore(DISP_REG_OVL_L1_Y2R_PARA_YUV_A_0);
    _reg_restore(DISP_REG_OVL_L1_Y2R_PARA_YUV_A_1);
    _reg_restore(DISP_REG_OVL_L1_Y2R_PARA_RGB_A_0);
    _reg_restore(DISP_REG_OVL_L1_Y2R_PARA_RGB_A_1);
    _reg_restore(DISP_REG_OVL_L2_Y2R_PARA_R0);
    _reg_restore(DISP_REG_OVL_L2_Y2R_PARA_R1);
    _reg_restore(DISP_REG_OVL_L2_Y2R_PARA_G0);
    _reg_restore(DISP_REG_OVL_L2_Y2R_PARA_G1);
    _reg_restore(DISP_REG_OVL_L2_Y2R_PARA_B0);
    _reg_restore(DISP_REG_OVL_L2_Y2R_PARA_B1);
    _reg_restore(DISP_REG_OVL_L2_Y2R_PARA_YUV_A_0);
    _reg_restore(DISP_REG_OVL_L2_Y2R_PARA_YUV_A_1);
    _reg_restore(DISP_REG_OVL_L2_Y2R_PARA_RGB_A_0);
    _reg_restore(DISP_REG_OVL_L2_Y2R_PARA_RGB_A_1);
    _reg_restore(DISP_REG_OVL_L3_Y2R_PARA_R0);
    _reg_restore(DISP_REG_OVL_L3_Y2R_PARA_R1);
    _reg_restore(DISP_REG_OVL_L3_Y2R_PARA_G0);
    _reg_restore(DISP_REG_OVL_L3_Y2R_PARA_G1);
    _reg_restore(DISP_REG_OVL_L3_Y2R_PARA_B0);
    _reg_restore(DISP_REG_OVL_L3_Y2R_PARA_B1);
    _reg_restore(DISP_REG_OVL_L3_Y2R_PARA_YUV_A_0);
    _reg_restore(DISP_REG_OVL_L3_Y2R_PARA_YUV_A_1);
    _reg_restore(DISP_REG_OVL_L3_Y2R_PARA_RGB_A_0);
    _reg_restore(DISP_REG_OVL_L3_Y2R_PARA_RGB_A_1);
    _reg_restore(DISP_REG_OVL_DEBUG_MON_SEL);
    _reg_restore(DISP_REG_OVL_RDMA0_MEM_GMC_SETTING2);
    _reg_restore(DISP_REG_OVL_RDMA1_MEM_GMC_SETTING2);
    _reg_restore(DISP_REG_OVL_RDMA2_MEM_GMC_SETTING2);
    _reg_restore(DISP_REG_OVL_RDMA3_MEM_GMC_SETTING2);
    _reg_restore(DISP_REG_OVL_FLOW_CTRL_DBG);
    _reg_restore(DISP_REG_OVL_ADDCON_DBG);

    // RDMA
    _reg_restore(DISP_REG_RDMA_INT_ENABLE);
    _reg_restore(DISP_REG_RDMA_INT_STATUS);
    _reg_restore(DISP_REG_RDMA_GLOBAL_CON);
    _reg_restore(DISP_REG_RDMA_SIZE_CON_0);
    _reg_restore(DISP_REG_RDMA_SIZE_CON_1);
    _reg_restore(DISP_REG_RDMA_TARGET_LINE);
    _reg_restore(DISP_REG_RDMA_MEM_CON);
    _reg_restore(DISP_REG_RDMA_MEM_START_ADDR);
    _reg_restore(DISP_REG_RDMA_MEM_SRC_PITCH);
    _reg_restore(DISP_REG_RDMA_MEM_GMC_SETTING_0);
    _reg_restore(DISP_REG_RDMA_MEM_SLOW_CON);
    _reg_restore(DISP_REG_RDMA_MEM_GMC_SETTING_1);
    _reg_restore(DISP_REG_RDMA_FIFO_CON);
    _reg_restore(DISP_REG_RDMA_FIFO_LOG);
    _reg_restore(DISP_REG_RDMA_C00);
    _reg_restore(DISP_REG_RDMA_C01);
    _reg_restore(DISP_REG_RDMA_C02);
    _reg_restore(DISP_REG_RDMA_C10);
    _reg_restore(DISP_REG_RDMA_C11);
    _reg_restore(DISP_REG_RDMA_C12);
    _reg_restore(DISP_REG_RDMA_C20);
    _reg_restore(DISP_REG_RDMA_C21);
    _reg_restore(DISP_REG_RDMA_C22);
    _reg_restore(DISP_REG_RDMA_PRE_ADD_0);
    _reg_restore(DISP_REG_RDMA_PRE_ADD_1);
    _reg_restore(DISP_REG_RDMA_PRE_ADD_2);
    _reg_restore(DISP_REG_RDMA_POST_ADD_0);
    _reg_restore(DISP_REG_RDMA_POST_ADD_1);
    _reg_restore(DISP_REG_RDMA_POST_ADD_2);
    _reg_restore(DISP_REG_RDMA_DUMMY);
    _reg_restore(DISP_REG_RDMA_DEBUG_OUT_SEL);

    // WDMA
    _reg_restore(DISP_REG_WDMA_INTEN);
    _reg_restore(DISP_REG_WDMA_INTSTA);
    _reg_restore(DISP_REG_WDMA_EN);
    _reg_restore(DISP_REG_WDMA_RST);
    _reg_restore(DISP_REG_WDMA_SMI_CON);
    _reg_restore(DISP_REG_WDMA_CFG);
    _reg_restore(DISP_REG_WDMA_SRC_SIZE);
    _reg_restore(DISP_REG_WDMA_CLIP_SIZE);
    _reg_restore(DISP_REG_WDMA_CLIP_COORD);
    _reg_restore(DISP_REG_WDMA_DST_ADDR0);
    _reg_restore(DISP_REG_WDMA_DST_W_IN_BYTE);
    _reg_restore(DISP_REG_WDMA_ALPHA);
    _reg_restore(DISP_REG_WDMA_BUF_CON1);
    _reg_restore(DISP_REG_WDMA_BUF_CON2);
    _reg_restore(DISP_REG_WDMA_C00);
    _reg_restore(DISP_REG_WDMA_C02);
    _reg_restore(DISP_REG_WDMA_C10);
    _reg_restore(DISP_REG_WDMA_C12);
    _reg_restore(DISP_REG_WDMA_C20);
    _reg_restore(DISP_REG_WDMA_C22);
    _reg_restore(DISP_REG_WDMA_PRE_ADD0);
    _reg_restore(DISP_REG_WDMA_PRE_ADD2);
    _reg_restore(DISP_REG_WDMA_POST_ADD0);
    _reg_restore(DISP_REG_WDMA_POST_ADD2);
    _reg_restore(DISP_REG_WDMA_DST_ADDR1);
    _reg_restore(DISP_REG_WDMA_DST_ADDR2);
    _reg_restore(DISP_REG_WDMA_DST_UV_PITCH);
    _reg_restore(DISP_REG_WDMA_FLOW_CTRL_DBG);
    _reg_restore(DISP_REG_WDMA_EXEC_DBG);
    _reg_restore(DISP_REG_WDMA_CT_DBG);

    DDP_DRV_INFO("disp_reg_restore() done \n");
    printk("DDP Config = 0x%08X\n", get_devinfo_with_index(16));

//#if defined(MTK_AAL_SUPPORT)
    disp_bls_init(fb_width, fb_height);
//#endif

    disp_pq_init();
    disp_pq_config(fb_width,fb_height);

    return 0;
}


int disp_path_clock_on(char* name)
{
    if(name != NULL)
    {
        DDP_DRV_INFO("disp_path_power_on, caller:%s \n", name);
    }

    larb_clock_on(0, "DDP");
    if (!clock_is_on(MT_CG_DISP_PQ_SW_CG))
        enable_clock(MT_CG_DISP_PQ_SW_CG, "DDP");
    if (!clock_is_on(MT_CG_DISP_BLS_SW_CG))
        enable_clock(MT_CG_DISP_BLS_SW_CG, "DDP");
    if (!clock_is_on(MT_CG_DISP_WDMA_SW_CG))
        enable_clock(MT_CG_DISP_WDMA_SW_CG, "DDP");
    if (!clock_is_on(MT_CG_DISP_RDMA_SW_CG))
        enable_clock(MT_CG_DISP_RDMA_SW_CG, "DDP");
    if (!clock_is_on(MT_CG_DISP_OVL_SW_CG))
        enable_clock(MT_CG_DISP_OVL_SW_CG, "DDP");
    if (!clock_is_on(MT_CG_DISP_PWM_SW_CG))
        enable_clock(MT_CG_DISP_PWM_SW_CG, "DDP");
    if (!clock_is_on(MT_CG_DISP_PWM_26M_SW_CG))
    {
        enable_clock(MT_CG_DISP_PWM_26M_SW_CG, "DDP");
        enable_clock(MT_CG_MUTEX_SLOW_CLOCK_SW_CG, "DDP");
    }

    // restore ddp related registers
    if (0 == strncmp(name, "mtkfb", 5))
    {
        if(*pRegBackup != DDP_UNBACKED_REG_MEM)
        {
            DDP_DRV_INFO("reg_restore, caller:%s \n", name);
        
            // restore ddp engine registers
            disp_reg_restore();

            // restore intr enable registers
            _disp_intr_restore();
        }
        else
        {
            DDP_DRV_INFO("disp_path_clock_on(), dose not call disp_reg_restore, cause mem not inited! \n");
        }
    }

    g_HistogramValid = 0;
    g_AAL_LumaUpdated = 0;
    
    return 0;
}


int disp_path_clock_off(char* name)
{
    if(name != NULL)
    {
        DDP_DRV_INFO("disp_path_power_off, caller:%s \n", name);
    }

    if (0 == strncmp(name, "mtkfb", 5))
    {
        DDP_DRV_INFO("reg_backup, caller:%s \n", name);

        // disable intr and clear intr status
        _disp_intr_disable_and_clear();
    
        // backup ddp engine registers
        disp_reg_backup();
    }

    if (clock_is_on(MT_CG_DISP_PWM_26M_SW_CG))
    {
        disable_clock(MT_CG_MUTEX_SLOW_CLOCK_SW_CG, "DDP");
        disable_clock(MT_CG_DISP_PWM_26M_SW_CG, "DDP");
    }
    if (clock_is_on(MT_CG_DISP_PWM_SW_CG))
        disable_clock(MT_CG_DISP_PWM_SW_CG, "DDP");
    if (clock_is_on(MT_CG_DISP_OVL_SW_CG))
        disable_clock(MT_CG_DISP_OVL_SW_CG, "DDP");
    if (clock_is_on(MT_CG_DISP_RDMA_SW_CG))
        disable_clock(MT_CG_DISP_RDMA_SW_CG, "DDP");
    if (clock_is_on(MT_CG_DISP_WDMA_SW_CG))
        disable_clock(MT_CG_DISP_WDMA_SW_CG, "DDP");
    if (clock_is_on(MT_CG_DISP_BLS_SW_CG))
        disable_clock(MT_CG_DISP_BLS_SW_CG, "DDP");
    if (clock_is_on(MT_CG_DISP_PQ_SW_CG))
        disable_clock(MT_CG_DISP_PQ_SW_CG, "DDP");
    larb_clock_off(0, "DDP");

    return 0;
}

