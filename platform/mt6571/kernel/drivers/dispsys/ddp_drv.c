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
#include <linux/sched.h>
#include <linux/kthread.h>

#include <linux/xlog.h>
#include <linux/proc_fs.h>  //proc file use
//ION
#include <linux/ion.h>
#include <linux/ion_drv.h>
#include <mach/m4u.h>

#include <linux/vmalloc.h>
#include <linux/dma-mapping.h>

#include <asm/io.h>


#include <mach/irqs.h>
#include <mach/mt_reg_base.h>
#include <mach/mt_irq.h>
#include <mach/irqs.h>
#include <mach/mt_clkmgr.h> // ????
#include <mach/mt_irq.h>
#include <mach/sync_write.h>

#include "ddp_drv.h"
#include "ddp_reg.h"
#include "ddp_path.h"
#include "ddp_debug.h"
#include "ddp_pq.h"
#include "ddp_bls.h"
#include "disp_drv.h"

#include "ddp_wdma.h"
#include "ddp_cmdq.h"

//#include <asm/tcm.h>


unsigned int dbg_log = 0;
unsigned int irq_log = 0;

#define DISP_DEVNAME "mtk_disp"
// device and driver
static dev_t disp_devno;
static struct cdev *disp_cdev;
static struct class *disp_class = NULL;

//ION

unsigned char ion_init=0;
unsigned char dma_init=0;

//NCSTool for PQ Tuning
unsigned char ncs_tuning_mode = 0;

//flag for gamma lut update
unsigned char bls_gamma_dirty = 0;

struct ion_client *cmdqIONClient;
struct ion_handle *cmdqIONHandle;
struct ion_mm_data mm_data;
unsigned long * cmdq_pBuffer;
unsigned int cmdq_pa;
unsigned int cmdq_pa_len;
struct ion_sys_data sys_data;
M4U_PORT_STRUCT m4uPort;
//irq
#define DISP_REGISTER_IRQ(irq_num){\
    if(request_irq( irq_num , (irq_handler_t)disp_irq_handler, IRQF_TRIGGER_LOW, DISP_DEVNAME , NULL))\
    { DDP_DRV_ERR("ddp register irq failed! %d\n", irq_num); }}

//-------------------------------------------------------------------------------//
// global variables
typedef struct
{
    spinlock_t irq_lock;
    unsigned int irq_src;  //one bit represent one module
} disp_irq_struct;

typedef struct
{
    pid_t open_pid;
    pid_t open_tgid;
    struct list_head testList;
    unsigned int u4LockedResource;
    unsigned int u4Clock;
    spinlock_t node_lock;
} disp_node_struct;

#define DISP_MAX_IRQ_CALLBACK   10
static DDP_IRQ_CALLBACK g_disp_irq_table[DISP_MODULE_MAX][DISP_MAX_IRQ_CALLBACK];

disp_irq_struct g_disp_irq;
static DECLARE_WAIT_QUEUE_HEAD(g_disp_irq_done_queue);

// cmdq thread

unsigned char cmdq_thread[CMDQ_THREAD_NUM] = {1, 1, 1, 1, 1, 1, 1};
spinlock_t gCmdqLock;
extern spinlock_t gCmdqMgrLock;
extern unsigned int gMutexID;

wait_queue_head_t cmq_wait_queue[CMDQ_THREAD_NUM];


//G2d Variables
spinlock_t gResourceLock;
unsigned int gLockedResource;//lock dpEngineType_6572

static DECLARE_WAIT_QUEUE_HEAD(gResourceWaitQueue);

// Overlay Variables
spinlock_t gOvlLock;
int disp_run_dp_framework = 0;
int disp_layer_enable = 0;
int disp_mutex_status = 0;

DISP_OVL_INFO disp_layer_info[DDP_OVL_LAYER_MUN];

//AAL variables
static unsigned long u4UpdateFlag = 0;

//Register update lock
spinlock_t gRegisterUpdateLock;
spinlock_t gPowerOperateLock;
//Clock gate management
//static unsigned long g_u4ClockOnTbl = 0;

//PQ variables
extern UINT32 fb_width;
extern UINT32 fb_height;

// IRQ log print kthread
static struct task_struct *disp_irq_log_task = NULL;
static wait_queue_head_t disp_irq_log_wq;
static int disp_irq_log_module = 0;

static DISPLAY_SHP_T g_SHP_Index;

DISPLAY_SHP_T *get_SHP_index(void)
{
    return &g_SHP_Index;
}


extern void cmdqForceFreeAll(int cmdqThread);
extern void cmdqForceFree_SW(int taskID);
bool checkMdpEngineStatus(unsigned int engineFlag);



unsigned int* pRegBackup = NULL;

//-------------------------------------------------------------------------------//
// functions
#if 0
static void cmdq_ion_init(void)
{
    //ION
    DISP_MSG("DISP ION: ion_client_create 1:%x",(unsigned int)g_ion_device);

    cmdqIONClient = ion_client_create(g_ion_device,-1, "cmdq");
    DISP_MSG("DISP ION: ion_client_create cmdqIONClient...%x.\n", (unsigned int)cmdqIONClient);
    if (IS_ERR_OR_NULL(cmdqIONClient))
    {
        DISP_ERR("DISP ION: Couldn't create ion client\n");
    }

    DISP_MSG("DISP ION: Create ion client done\n");

    cmdqIONHandle = ion_alloc(cmdqIONClient, CMDQ_ION_BUFFER_SIZE, 4, ION_HEAP_MULTIMEDIA_MASK, 0);

    if (IS_ERR_OR_NULL(cmdqIONHandle))
    {
        DISP_ERR("DISP ION: Couldn't alloc ion buffer\n");
    }

    DISP_MSG("DISP ION:  ion alloc done\n");

    mm_data.mm_cmd = ION_MM_CONFIG_BUFFER;
    mm_data.config_buffer_param.handle = cmdqIONHandle;
    mm_data.config_buffer_param.eModuleID = M4U_CLNTMOD_CMDQ;
    mm_data.config_buffer_param.security = 0;
    mm_data.config_buffer_param.coherent = 0;

    if(ion_kernel_ioctl(cmdqIONClient, ION_CMD_MULTIMEDIA, &mm_data))
    {
        DISP_ERR("DISP ION: Couldn't config ion buffer\n");
    }

    DISP_MSG("DISP ION:  ion config done\n");

    cmdq_pBuffer = ion_map_kernel(cmdqIONClient, cmdqIONHandle);
    if(IS_ERR_OR_NULL(cmdq_pBuffer))
    {
        DISP_ERR("DISP ION: Couldn't get ion buffer VA\n");
    }

    DISP_MSG("DISP ION:  ion VA done\n");

    if(ion_phys(cmdqIONClient,cmdqIONHandle, &cmdq_pa, &cmdq_pa_len))
    {
        DISP_ERR("DISP ION: Couldn't get ion buffer MVA\n");
    }

    DISP_MSG("DISP ION:  ion MVA done\n");

    m4uPort.ePortID = M4U_PORT_CMDQ;
    m4uPort.Virtuality = 1;
    m4uPort.Security = 0;
    m4uPort.Distance = 1;
    m4uPort.Direction = 0;
    //m4uPort.Domain = 3;
    m4u_config_port(&m4uPort);

    DISP_MSG("DISP ION:  Config MVA port done\n");

    //ION TEST
    DISP_MSG("CMDQ ION buffer VA: 0x%lx MVA: 0x%x \n", (unsigned long)cmdq_pBuffer, (unsigned int)cmdq_pa);

    cmdqBufferTbl_init((unsigned long) cmdq_pBuffer, (unsigned int) cmdq_pa);
}
#endif


void cmdq_ion_flush(void)
{
    sys_data.sys_cmd = ION_SYS_CACHE_SYNC;
    sys_data.cache_sync_param.handle = cmdqIONHandle;
    sys_data.cache_sync_param.sync_type = ION_CACHE_FLUSH_ALL;

    if(ion_kernel_ioctl(cmdqIONClient, ION_CMD_SYSTEM ,(int)&sys_data))
    {
        DISP_ERR("CMDQ ION flush fail\n");
    }
}


static void cmdq_dma_init(void)
{
    cmdq_pBuffer= dma_alloc_coherent(NULL, CMDQ_ION_BUFFER_SIZE, &cmdq_pa, GFP_KERNEL);

    if(!cmdq_pBuffer)
    {
        DISP_MSG("dma_alloc_coherent error!  dma memory not available.\n");
        return ;
    }

    memset((void*)cmdq_pBuffer, 0, CMDQ_ION_BUFFER_SIZE);
    DISP_MSG("dma_alloc_coherent success VA:%x PA:%x \n", (unsigned int)cmdq_pBuffer, (unsigned int)cmdq_pa);


    cmdqBufferTbl_init((unsigned long) cmdq_pBuffer, (unsigned int) cmdq_pa);
}


static int disp_irq_log_kthread_func(void *data)
{
    unsigned int i=0;
    while(1)
    {
        wait_event_interruptible(disp_irq_log_wq, disp_irq_log_module);
        DDP_DRV_INFO("disp_irq_log_kthread_func dump intr register: disp_irq_log_module=%d \n", disp_irq_log_module);
        for(i=0;i<DISP_MODULE_MAX;i++)
        {
            if( (disp_irq_log_module&(1<<i))!=0 )
            {
                disp_dump_reg(i);
            }
        }
        // reset wakeup flag
        disp_irq_log_module = 0;
    }

    return 0;
}

unsigned int disp_ms2jiffies(unsigned long ms)
{
    return ((ms*HZ + 512) >> 10);
}

int disp_lock_cmdq_thread(void)
{
    int i=0;

    printk("disp_lock_cmdq_thread()called \n");

    spin_lock(&gCmdqLock);
    for (i = 0; i < CMDQ_THREAD_NUM; i++)
    {
        if (cmdq_thread[i] == 1)
        {
            cmdq_thread[i] = 0;
            break;
        }
    }
    spin_unlock(&gCmdqLock);

    printk("disp_lock_cmdq_thread(), i=%d \n", i);

    return (i>=CMDQ_THREAD_NUM)? -1 : i;

}

int disp_unlock_cmdq_thread(unsigned int idx)
{
    if(idx >= CMDQ_THREAD_NUM)
        return -1;

    spin_lock(&gCmdqLock);
    cmdq_thread[idx] = 1;  // free thread availbility
    spin_unlock(&gCmdqLock);

    return 0;
}

// if return is not 0, should wait again
int disp_wait_intr(DISP_MODULE_ENUM module, unsigned int timeout_ms)
{
    int ret;
    unsigned long flags;
#if 0
    unsigned long long end_time = 0;
    unsigned long long start_time = sched_clock();
#endif

    MMProfileLogEx(DDP_MMP_Events.WAIT_INTR, MMProfileFlagStart, 0, module);
    // wait until irq done or timeout
    ret = wait_event_interruptible_timeout(
                    g_disp_irq_done_queue,
                    g_disp_irq.irq_src & (1<<module),
                    disp_ms2jiffies(timeout_ms) );

    /*wake-up from sleep*/
    if(ret==0) // timeout
    {
        MMProfileLogEx(DDP_MMP_Events.WAIT_INTR, MMProfileFlagPulse, 0, module);
        MMProfileLog(DDP_MMP_Events.WAIT_INTR, MMProfileFlagEnd);
        DDP_DRV_ERR("Wait Done Timeout! pid=%d, module=%d \n", current->pid ,module);
        if(module==DISP_MODULE_WDMA0)
        {
            printk("======== WDMA0 timeout, dump all registers! \n");
            disp_dump_reg(DISP_MODULE_WDMA0);
            disp_dump_reg(DISP_MODULE_CONFIG);
        }
        else
        {
            disp_dump_reg(module);
        }
        ASSERT(0);

        return -EAGAIN;
    }
    else if(ret<0) // intr by a signal
    {
        MMProfileLogEx(DDP_MMP_Events.WAIT_INTR, MMProfileFlagPulse, 1, module);
        MMProfileLog(DDP_MMP_Events.WAIT_INTR, MMProfileFlagEnd);
        DDP_DRV_ERR("Wait Done interrupted by a signal! pid=%d, module=%d \n", current->pid ,module);
        disp_dump_reg(module);
        ASSERT(0);
        return -EAGAIN;
    }

    MMProfileLogEx(DDP_MMP_Events.WAIT_INTR, MMProfileFlagEnd, 0, module);
    spin_lock_irqsave( &g_disp_irq.irq_lock , flags );
    g_disp_irq.irq_src &= ~(1<<module);
    spin_unlock_irqrestore( &g_disp_irq.irq_lock , flags );

#if 0
    end_time = sched_clock();
   	//DDP_DRV_INFO("**ROT_SCL_WDMA0 execute %d us\n", ((unsigned int)end_time-(unsigned int)start_time)/1000);
#endif

    return 0;
}

int disp_register_irq(DISP_MODULE_ENUM module, DDP_IRQ_CALLBACK cb)
{
    int i;
    if (module >= DISP_MODULE_MAX)
    {
        DDP_DRV_ERR("Register IRQ with invalid module ID. module=%d\n", module);
        return -1;
    }
    if (cb == NULL)
    {
        DDP_DRV_ERR("Register IRQ with invalid cb.\n");
        return -1;
    }
    for (i=0; i<DISP_MAX_IRQ_CALLBACK; i++)
    {
        if (g_disp_irq_table[module][i] == cb)
            break;
    }
    if (i < DISP_MAX_IRQ_CALLBACK)
    {
        // Already registered.
        return 0;
    }
    for (i=0; i<DISP_MAX_IRQ_CALLBACK; i++)
    {
        if (g_disp_irq_table[module][i] == NULL)
            break;
    }
    if (i == DISP_MAX_IRQ_CALLBACK)
    {
        DDP_DRV_ERR("No enough callback entries for module %d.\n", module);
        return -1;
    }
    g_disp_irq_table[module][i] = cb;
    return 0;
}

int disp_unregister_irq(DISP_MODULE_ENUM module, DDP_IRQ_CALLBACK cb)
{
    int i;
    for (i=0; i<DISP_MAX_IRQ_CALLBACK; i++)
    {
        if (g_disp_irq_table[module][i] == cb)
        {
            g_disp_irq_table[module][i] = NULL;
            break;
        }
    }
    if (i == DISP_MAX_IRQ_CALLBACK)
    {
        DDP_DRV_ERR("Try to unregister callback function with was not registered. module=%d cb=0x%08X\n", module, (unsigned int)cb);
        return -1;
    }
    return 0;
}

void disp_invoke_irq_callbacks(DISP_MODULE_ENUM module, unsigned int param)
{
    int i;
    for (i=0; i<DISP_MAX_IRQ_CALLBACK; i++)
    {
        if (g_disp_irq_table[module][i])
        {
            //DDP_DRV_ERR("Invoke callback function. module=%d param=0x%X\n", module, param);
            g_disp_irq_table[module][i](param);
        }
    }
}
#if defined(MTK_HDMI_SUPPORT)
extern void hdmi_setorientation(int orientation);
void hdmi_power_on(void);
void hdmi_power_off(void);
extern void hdmi_update_buffer_switch(void);
extern bool is_hdmi_active(void);
extern void hdmi_update(void);
extern void hdmi_source_buffer_switch(void);
#endif
#if  defined(MTK_WFD_SUPPORT)
extern void hdmi_setorientation(int orientation);
void hdmi_power_on(void);
void hdmi_power_off(void);
extern void wfd_update_buffer_switch(void);
extern bool is_wfd_active(void);
extern void wfd_update(void);
extern void wfd_source_buffer_switch(void);
#endif

//extern void hdmi_test_switch_buffer(void);

unsigned int cnt_rdma_underflow = 1;
unsigned int cnt_rdma_abnormal = 1;
unsigned int cnt_ovl_underflow = 1;
unsigned int cnt_ovl_eof = 1;

static /*__tcmfunc*/ irqreturn_t disp_irq_handler(int irq, void *dev_id)
{
    unsigned long reg_val;
    int i;
    int taskid;
    extern int cmdqThreadTaskList[CMDQ_THREAD_NUM][CMDQ_THREAD_LIST_LENGTH];
    extern unsigned int cmdqThreadTaskList_R[CMDQ_THREAD_NUM];

    DDP_DRV_IRQ("irq=%d, 0x%x, 0x%x, 0x%x, 0x%x \n",
                     irq,
                     DISP_REG_GET(DISP_REG_OVL_INTSTA),
                     DISP_REG_GET(DISP_REG_WDMA_INTSTA),
                     DISP_REG_GET(DISP_REG_RDMA_INT_STATUS),
                     DISP_REG_GET(DISP_REG_CONFIG_MUTEX_INTSTA));


    /*1. Process ISR*/
    switch(irq)
    {

        case MT_DISP_OVL_IRQ_ID:
                reg_val = DISP_REG_GET(DISP_REG_OVL_INTSTA);
                if(reg_val&(1<<0))
                {
                      DDP_DRV_IRQ("IRQ: OVL reg update done! \n");
                }
                if(reg_val&(1<<1))
                {
                      DDP_DRV_IRQ("IRQ: OVL frame done! \n");
                      g_disp_irq.irq_src |= (1<<DISP_MODULE_OVL);
                }
                if(reg_val&(1<<2))
                {
                      DDP_DRV_ERR("IRQ: OVL frame underrun! \n");
                }
                if(reg_val&(1<<3))
                {
                      DDP_DRV_IRQ("IRQ: OVL SW reset done! \n");
                }
                if(reg_val&(1<<4))
                {
                      DDP_DRV_IRQ("IRQ: OVL HW reset done! \n");
                }
                if(reg_val&(1<<5))
                {
                      DDP_DRV_ERR("IRQ: OVL-RDMA0 not complete untill EOF! \n");
                }
                if(reg_val&(1<<6))
                {
                      DDP_DRV_ERR("IRQ: OVL-RDMA1 not complete untill EOF! \n");
                }
                if(reg_val&(1<<7))
                {
                      DDP_DRV_ERR("IRQ: OVL-RDMA2 not complete untill EOF! \n");
                }
                if(reg_val&(1<<8))
                {
                      DDP_DRV_ERR("IRQ: OVL-RDMA3 not complete untill EOF! \n");
                }
                if(reg_val&(1<<9))
                {
                      DDP_DRV_ERR("IRQ: OVL-RDMA0 fifo underflow! \n");
                }
                if(reg_val&(1<<10))
                {
                      DDP_DRV_ERR("IRQ: OVL-RDMA1 fifo underflow! \n");
                }
                if(reg_val&(1<<11))
                {
                      DDP_DRV_ERR("IRQ: OVL-RDMA2 fifo underflow! \n");
                }
                if(reg_val&(1<<12))
                {
                      DDP_DRV_ERR("IRQ: OVL-RDMA3 fifo underflow! \n");
                }
                //clear intr
                DISP_REG_SET(DISP_REG_OVL_INTSTA, ~reg_val);
                MMProfileLogEx(DDP_MMP_Events.OVL_IRQ, MMProfileFlagPulse, reg_val, 0);
                disp_invoke_irq_callbacks(DISP_MODULE_OVL, reg_val);
            break;

        case MT_DISP_WDMA_IRQ_ID:
                reg_val = DISP_REG_GET(DISP_REG_WDMA_INTSTA);
                if(reg_val&(1<<0))
                {
                    DDP_DRV_IRQ("IRQ: WDMA0 frame done! \n");
                    g_disp_irq.irq_src |= (1<<DISP_MODULE_WDMA0);
                }
                if(reg_val&(1<<1))
                {
                      DDP_DRV_ERR("IRQ: WDMA0 underrun! \n");

                }
                //clear intr
                DISP_REG_SET(DISP_REG_WDMA_INTSTA, ~reg_val);
                MMProfileLogEx(DDP_MMP_Events.WDMA0_IRQ, MMProfileFlagPulse, reg_val, DISP_REG_GET(DISP_REG_WDMA_CLIP_SIZE));
                disp_invoke_irq_callbacks(DISP_MODULE_WDMA0, reg_val);
            break;


        case MT_DISP_RDMA_IRQ_ID:
                reg_val = DISP_REG_GET(DISP_REG_RDMA_INT_STATUS);
                if(reg_val&(1<<0))
                {
                      DDP_DRV_IRQ("IRQ: RDMA0 reg update done! \n");
                }
                if(reg_val&(1<<1))
                {
                      DDP_DRV_IRQ("IRQ: RDMA0 frame start! \n");
                      on_disp_aal_alarm_set();
                }
                if(reg_val&(1<<2))
                {
                      DDP_DRV_IRQ("IRQ: RDMA0 frame done! \n");
                      g_disp_irq.irq_src |= (1<<DISP_MODULE_RDMA0);
                }
                if(reg_val&(1<<3))
                {
                      if(cnt_rdma_abnormal)
                      {
                          DDP_DRV_ERR("IRQ: RDMA0 abnormal! \n");
                          cnt_rdma_abnormal = 0;
                      }
                }
                if(reg_val&(1<<4))
                {
                      if(cnt_rdma_underflow)
                      {
                          DDP_DRV_ERR("IRQ: RDMA0 underflow! \n");
                          cnt_rdma_underflow = 0;
                      }
                }
                //clear intr
                DISP_REG_SET(DISP_REG_RDMA_INT_STATUS, ~reg_val);
                MMProfileLogEx(DDP_MMP_Events.RDMA0_IRQ, MMProfileFlagPulse, reg_val, 0);
                disp_invoke_irq_callbacks(DISP_MODULE_RDMA0, reg_val);
            break;

        case MT_DISP_PQ_IRQ_ID:
            reg_val = DISP_REG_GET(DISP_REG_PQ_INTSTA);

            // read LUMA histogram
            if (reg_val & 0x2)
            {
//TODO : might want to move to other IRQ~ -S
                //disp_update_hist();
                //disp_wakeup_aal();
//TODO : might want to move to other IRQ~ -E
            }

            //clear intr
            DISP_REG_SET(DISP_REG_PQ_INTSTA, ~reg_val);

            MMProfileLogEx(DDP_MMP_Events.COLOR_IRQ, MMProfileFlagPulse, reg_val, 0);
//            disp_invoke_irq_callbacks(DISP_MODULE_PQ, reg_val);
            break;

        case MT_DISP_BLS_IRQ_ID:
            reg_val = DISP_REG_GET(DISP_REG_BLS_INTSTA);

            // read LUMA & MAX(R,G,B) histogram
            if (reg_val & 0x1)
            {
                disp_update_hist();
                disp_wakeup_aal();
            }

            //clear intr
            DISP_REG_SET(DISP_REG_BLS_INTSTA, ~reg_val);
            MMProfileLogEx(DDP_MMP_Events.BLS_IRQ, MMProfileFlagPulse, reg_val, 0);
            break;

        case MT_MM_MUTEX_IRQ_ID:  // can not do reg update done status after release mutex(for ECO requirement),
                                        // so we have to check update timeout intr here
            reg_val = DISP_REG_GET(DISP_REG_CONFIG_MUTEX_INTSTA);

#if 0
            if(reg_val&0xFF00) // udpate timeout intr triggered
            {
                unsigned int reg = 0;
                unsigned int mutexID = 0;

                for(mutexID=0;mutexID<4;mutexID++)
                {
                    if((DISP_REG_GET(DISP_REG_CONFIG_MUTEX_INTSTA) & (1<<(mutexID+8))) == (1<<(mutexID+8)))
                    {
                        DDP_DRV_ERR("disp_path_release_mutex() timeout! \n");
                        disp_dump_reg(DISP_MODULE_CONFIG);
                        disp_dump_reg(DISP_MODULE_MUTEX);
                        //print error engine
                        reg = DISP_REG_GET(DISP_REG_CONFIG_REG_COMMIT);
                        if(reg!=0)
                        {
                            if (reg & DDP_MOD_DISP_OVL) { DDP_DRV_INFO(" OVL update reg timeout! \n"); disp_dump_reg(DISP_MODULE_OVL); }
                            if (reg & DDP_MOD_DISP_PQ) { DDP_DRV_INFO(" PQ update reg timeout! \n");    disp_dump_reg(DISP_MODULE_PQ); }
                            if (reg & DDP_MOD_DISP_WDMA) { DDP_DRV_INFO(" WDMA update reg timeout! \n"); disp_dump_reg(DISP_MODULE_WDMA0); }
                            if (reg & DDP_MOD_DISP_RDMA) { DDP_DRV_INFO(" RDMA update reg timeout! \n"); disp_dump_reg(DISP_MODULE_RDMA0); }
                            if (reg & DDP_MOD_DISP_BLS) { DDP_DRV_INFO(" BLS update reg timeout! \n"); disp_dump_reg(DISP_MODULE_BLS); }
                        }
                        ASSERT(0);

                        //reset mutex
                        DISP_REG_SET(DISP_REG_CONFIG_MUTEX_RST(mutexID), 1);
                        DISP_REG_SET(DISP_REG_CONFIG_MUTEX_RST(mutexID), 0);
                        DDP_DRV_INFO("mutex reset done! \n");
                    }
                 }
            }
#else
            if(reg_val&0xFF00) // udpate timeout intr triggered
            {
                unsigned int reg = 0;

                 if((DISP_REG_GET(DISP_REG_CONFIG_MUTEX_INTSTA) & (1<<(gMutexID+8))) == (1<<(gMutexID+8)))
                 {
                     DDP_DRV_ERR("disp_path_release_mutex() timeout! \n");
                     //disp_dump_reg(DISP_MODULE_CONFIG);

                     //disp_dump_reg(DISP_MODULE_MUTEX);
                     //print error engine
                     reg = DISP_REG_GET(DISP_REG_CONFIG_REG_COMMIT);
                     if(reg!=0)
                     {
                         if (reg & DDP_MOD_DISP_OVL) { DDP_DRV_INFO(" OVL update reg timeout! \n"); }
                         if (reg & DDP_MOD_DISP_PQ) { DDP_DRV_INFO(" PQ update reg timeout! \n"); }
                         if (reg & DDP_MOD_DISP_WDMA) { DDP_DRV_INFO(" WDMA update reg timeout! \n"); }
                         if (reg & DDP_MOD_DISP_RDMA) { DDP_DRV_INFO(" RDMA update reg timeout! \n"); }
                         if (reg & DDP_MOD_DISP_BLS) { DDP_DRV_INFO(" BLS update reg timeout! \n"); }
                     }
                     //ASSERT(0);

                     //reset ovl
                     OVLReset();

                     //reset mutex
                     DISP_REG_SET(DISP_REG_CONFIG_MUTEX_RST(gMutexID), 1);
                     DISP_REG_SET(DISP_REG_CONFIG_MUTEX_RST(gMutexID), 0);

                     reg = DISP_REG_GET(DISP_REG_CONFIG_MUTEX_INTSTA);
                     reg &= ~(1 << gMutexID);
                     reg &= ~(1 << (gMutexID + 8));
                     DISP_REG_SET(DISP_REG_CONFIG_MUTEX_INTSTA, reg);

                     DDP_DRV_INFO("mutex reset done! \n");
                 }
            }
#endif

            DISP_REG_SET(DISP_REG_CONFIG_MUTEX_INTSTA, ~reg_val);
            MMProfileLogEx(DDP_MMP_Events.Mutex_IRQ, MMProfileFlagPulse, reg_val, 0);
            disp_invoke_irq_callbacks(DISP_MODULE_MUTEX, reg_val);
            break;

        case MT_CMDQ_IRQ_ID:
            for(i = 0 ; i < CMDQ_THREAD_NUM ; i++)
            {
                reg_val = DISP_REG_GET(DISP_REG_CMDQ_THRx_IRQ_FLAG(i));
                DISP_REG_SET(DISP_REG_CMDQ_THRx_IRQ_FLAG(i), ~reg_val);
                if( reg_val != 0 )
                {

                    taskid = cmdqThreadTaskList[i][cmdqThreadTaskList_R[i]];
                    //printk("\n\n!!!!!!!!!!!!!!!!!!!!!CMQ Thread %d Complete Task %d!!!!!!!!!!!!!!!!!!!\n\n\n", i, taskid);
                    if(reg_val & (1 << 1))
                    {
                        DISP_ERR("\n\n\n!!!!!!!!!!!!!!!IRQ: CMQ %d Time out! !!!!!!!!!!!!!!!!\n\n\n", i);
                        spin_lock(&gCmdqMgrLock);
                        cmdqForceFreeAll(i);
                        smp_wmb();
                        spin_unlock(&gCmdqMgrLock);

                    }
                    else if(reg_val & (1 << 4))
                    {
                        DISP_ERR("IRQ: CMQ thread%d Invalid Command Instruction! \n", i);
                        spin_lock(&gCmdqMgrLock);
                        cmdqForceFreeAll(i);
                        smp_wmb();
                        spin_unlock(&gCmdqMgrLock);

                    }
                    else if(reg_val == 0x1)// Normal EOF end
                    {
                        //spin_lock(&gCmdqMgrLock);
                        //cmdqThreadComplete(i, true); //Thread i complete!
                        //smp_wmb();
                        wake_up_interruptible(&cmq_wait_queue[i]);
                        //spin_unlock(&gCmdqMgrLock);
                    }


                    MMProfileLogEx(DDP_MMP_Events.CMDQ_IRQ, MMProfileFlagPulse, reg_val, i);
                }
            }
            break;

        default: DDP_DRV_ERR("invalid irq=%d \n ", irq); break;
    }

    // Wakeup event
    mb();   // Add memory barrier before the other CPU (may) wakeup
    wake_up_interruptible(&g_disp_irq_done_queue);

    return IRQ_HANDLED;
}


void disp_power_on(DISP_MODULE_ENUM eModule , unsigned int * pu4Record)
{
#if 0
    unsigned long flag;
    unsigned int ret = 0;
    spin_lock_irqsave(&gPowerOperateLock , flag);

    if((1 << eModule) & g_u4ClockOnTbl)
    {
        DDP_DRV_INFO("DDP power %lu is already enabled\n" , (unsigned long)eModule);
    }
    else
    {
        switch(eModule)
        {

            case DISP_MODULE_WDMA0 :
                enable_clock(MT_CG_DISP0_WDMA0_ENGINE , "DDP_DRV");
                enable_clock(MT_CG_DISP0_WDMA0_SMI , "DDP_DRV");
            break;

            default :
                DDP_DRV_ERR("disp_power_on:unknown module:%d\n" , eModule);
                ret = -1;
            break;
        }

        if(0 == ret)
        {
            if(0 == g_u4ClockOnTbl)
            {
                enable_clock(MT_CG_DISP0_LARB2_SMI , "DDP_DRV");
            }
            g_u4ClockOnTbl |= (1 << eModule);
            *pu4Record |= (1 << eModule);
        }
    }

    spin_unlock_irqrestore(&gPowerOperateLock , flag);
#endif
}

void disp_power_off(DISP_MODULE_ENUM eModule , unsigned int * pu4Record)
{
#if 0
    unsigned long flag;
    unsigned int ret = 0;
    spin_lock_irqsave(&gPowerOperateLock , flag);

//    DDP_DRV_INFO("power off : %d\n" , eModule);

    if((1 << eModule) & g_u4ClockOnTbl)
    {
        switch(eModule)
        {
            case DISP_MODULE_WDMA0 :
                WDMAStop(0);
                WDMAReset(0);
                disable_clock(MT_CG_DISP0_WDMA0_ENGINE , "DDP_DRV");
                disable_clock(MT_CG_DISP0_WDMA0_SMI , "DDP_DRV");
            break;

            default :
                DDP_DRV_ERR("disp_power_off:unsupported format:%d\n" , eModule);
                ret = -1;
            break;
        }

        if(0 == ret)
        {
            g_u4ClockOnTbl &= (~(1 << eModule));
            *pu4Record &= (~(1 << eModule));

            if(0 == g_u4ClockOnTbl)
            {
                disable_clock(MT_CG_DISP0_LARB2_SMI , "DDP_DRV");
            }

        }
    }
    else
    {
        DDP_DRV_INFO("DDP power %lu is already disabled\n" , (unsigned long)eModule);
    }

    spin_unlock_irqrestore(&gPowerOperateLock , flag);
#endif
}

unsigned int inAddr=0, outAddr=0;

int disp_set_needupdate(DISP_MODULE_ENUM eModule , unsigned long u4En)
{
    unsigned long flag;
    spin_lock_irqsave(&gRegisterUpdateLock , flag);

    if(u4En)
    {
        u4UpdateFlag |= (1 << eModule);
    }
    else
    {
        u4UpdateFlag &= ~(1 << eModule);
    }

    spin_unlock_irqrestore(&gRegisterUpdateLock , flag);

    return 0;
}

void DISP_REG_SET_FIELD(unsigned long field, unsigned long reg32, unsigned long val)
{
    unsigned long flag;
    spin_lock_irqsave(&gRegisterUpdateLock , flag);
    //*(volatile unsigned int*)(reg32) = ((*(volatile unsigned int*)(reg32) & ~(REG_FLD_MASK(field))) |  REG_FLD_VAL((field), (val)));
     mt65xx_reg_sync_writel( (*(volatile unsigned int*)(reg32) & ~(REG_FLD_MASK(field)))|REG_FLD_VAL((field), (val)), reg32);
     spin_unlock_irqrestore(&gRegisterUpdateLock , flag);
}

int CheckAALUpdateFunc(int i4IsNewFrame)
{
    return (((1 << DISP_MODULE_BLS) & u4UpdateFlag) || i4IsNewFrame || is_disp_aal_alarm_on()) ? 1 : 0;
}

extern int g_AAL_LumaUpdated;

int ConfAALFunc(int i4IsNewFrame)
{
    disp_onConfig_aal(i4IsNewFrame);
    
    disp_set_needupdate(DISP_MODULE_BLS , 0);
    return 0;
}

static int AAL_init = 0;
void disp_aal_lock()
{
    if(0 == AAL_init)
    {
        //printk("disp_aal_lock: register update func\n");
        DISP_RegisterExTriggerSource(CheckAALUpdateFunc , ConfAALFunc);
        AAL_init = 1;
    }
    GetUpdateMutex();
}

void disp_aal_unlock()
{
    ReleaseUpdateMutex();
    disp_set_needupdate(DISP_MODULE_BLS , 1);
}

int CheckPQUpdateFunc(int i4NotUsed)
{
    return (((1 << DISP_MODULE_PQ) & u4UpdateFlag) || bls_gamma_dirty) ? 1 : 0;
}

int ConfPQFunc(int i4NotUsed)
{
    DISP_MSG("ConfPQFunc: BLS_EN=0x%x, bls_gamma_dirty=%d\n", DISP_REG_GET(DISP_REG_BLS_EN), bls_gamma_dirty);
    if(bls_gamma_dirty != 0)
    {
        // disable BLS
        if (DISP_REG_GET(DISP_REG_BLS_EN) & 0x1)
        {
            DISP_MSG("ConfPQFunc: Disable BLS\n");
            DISP_REG_SET(DISP_REG_BLS_EN, 0x0);
        }
    }
    else
    {
        if(ncs_tuning_mode == 0) //normal mode
        {
            disp_pq_config(fb_width,fb_height);
        }
        else
        {
            ncs_tuning_mode = 0;
        }
        // enable BLS
        DISP_REG_SET(DISP_REG_BLS_EN, 0x1);
        disp_set_needupdate(DISP_MODULE_PQ , 0);
    }
    DISP_MSG("ConfPQFunc done: BLS_EN=0x%x, bls_gamma_dirty=%d\n", DISP_REG_GET(DISP_REG_BLS_EN), bls_gamma_dirty);
    return 0;
}


static long disp_unlocked_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    DISP_WRITE_REG wParams;
    DISP_READ_REG rParams;
    unsigned int ret = 0;
    unsigned int value;
    int taskID;
    DISP_MODULE_ENUM module;
    DISP_OVL_INFO ovl_info;
    DISP_PQ_PARAM * pq_param;
    DISP_PQ_PARAM * pq_cam_param;
    DISP_PQ_PARAM * pq_gal_param;
    DISPLAY_PQ_T * pq_index;
    DISPLAY_SHP_T * shp_index;
    DISPLAY_GAMMA_T * gamma_index;
    //DISPLAY_PWM_T * pwm_lut;
    int layer, mutex_id;
    disp_wait_irq_struct wait_irq_struct;
    unsigned long lcmindex = 0;
    unsigned long flags;
    int count;
    struct timeval tv;

#if defined(MTK_AAL_SUPPORT)
    DISP_AAL_PARAM * aal_param;
#endif

#ifdef DDP_DBG_DDP_PATH_CONFIG
    struct disp_path_config_struct config;
#endif

    disp_node_struct *pNode = (disp_node_struct *)file->private_data;

#if 0
    if(inAddr==0)
    {
        inAddr = kmalloc(800*480*4, GFP_KERNEL);
        memset((void*)inAddr, 0x55, 800*480*4);
        DDP_DRV_INFO("inAddr=0x%x \n", inAddr);
    }
    if(outAddr==0)
    {
        outAddr = kmalloc(800*480*4, GFP_KERNEL);
        memset((void*)outAddr, 0xff, 800*480*4);
        DDP_DRV_INFO("outAddr=0x%x \n", outAddr);
    }
#endif

#if 0
    DDP_DRV_DBG("cmd=0x%x, arg=0x%x \n", cmd, (unsigned int)arg);
#endif

    switch (cmd)
    {
        case DISP_IOCTL_WRITE_REG:

            if(copy_from_user(&wParams, (void *)arg, sizeof(DISP_WRITE_REG )))
            {
                DDP_DRV_ERR("DISP_IOCTL_WRITE_REG, copy_from_user failed\n");
                return -EFAULT;
            }

            DDP_DRV_DBG("write  0x%x = 0x%x (0x%x)\n", wParams.reg, wParams.val, wParams.mask);
            if(wParams.reg>DISPSYS_REG_ADDR_MAX || wParams.reg<DISPSYS_REG_ADDR_MIN)
            {
                DDP_DRV_ERR("reg write, addr invalid, addr min=0x%x, max=0x%x, addr=0x%x \n",
                    DISPSYS_REG_ADDR_MIN,
                    DISPSYS_REG_ADDR_MAX,
                    wParams.reg);
                return -EFAULT;
            }

            *(volatile unsigned int*)wParams.reg = (*(volatile unsigned int*)wParams.reg & ~wParams.mask) | (wParams.val & wParams.mask);
            //mt65xx_reg_sync_writel(wParams.reg, value);
            break;

        case DISP_IOCTL_READ_REG:
            if(copy_from_user(&rParams, (void *)arg, sizeof(DISP_READ_REG)))
            {
                DDP_DRV_ERR("DISP_IOCTL_READ_REG, copy_from_user failed\n");
                return -EFAULT;
            }
            if(0) //wParams.reg>DISPSYS_REG_ADDR_MAX || wParams.reg<DISPSYS_REG_ADDR_MIN)
            {
                DDP_DRV_ERR("reg read, addr invalid, addr min=0x%x, max=0x%x, addr=0x%x \n",
                    DISPSYS_REG_ADDR_MIN,
                    DISPSYS_REG_ADDR_MAX,
                    wParams.reg);
                return -EFAULT;
            }

            value = (*(volatile unsigned int*)rParams.reg) & rParams.mask;

            DDP_DRV_DBG("read 0x%x = 0x%x (0x%x)\n", rParams.reg, value, rParams.mask);

            if(copy_to_user(rParams.val, &value, sizeof(unsigned int)))
            {
                DDP_DRV_ERR("DISP_IOCTL_READ_REG, copy_to_user failed\n");
                return -EFAULT;
            }
            break;

        case DISP_IOCTL_WAIT_IRQ:
            if(copy_from_user(&wait_irq_struct, (void*)arg , sizeof(wait_irq_struct)))
            {
                DDP_DRV_ERR("DISP_IOCTL_WAIT_IRQ, copy_from_user failed\n");
                return -EFAULT;
            }
            ret = disp_wait_intr(wait_irq_struct.module, wait_irq_struct.timeout_ms);
            break;

        case DISP_IOCTL_DUMP_REG:
            if(copy_from_user(&module, (void*)arg , sizeof(module)))
            {
                DDP_DRV_ERR("DISP_IOCTL_DUMP_REG, copy_from_user failed\n");
                return -EFAULT;
            }
            ret = disp_dump_reg(module);
            break;

        case DISP_IOCTL_LOCK_THREAD:
            printk("DISP_IOCTL_LOCK_THREAD! \n");
            value = disp_lock_cmdq_thread();
            if (copy_to_user((void*)arg, &value , sizeof(unsigned int)))
            {
                DDP_DRV_ERR("DISP_IOCTL_LOCK_THREAD, copy_to_user failed\n");
                return -EFAULT;
            }
            break;

        case DISP_IOCTL_UNLOCK_THREAD:
            if(copy_from_user(&value, (void*)arg , sizeof(value)))
            {
                    DDP_DRV_ERR("DISP_IOCTL_UNLOCK_THREAD, copy_from_user failed\n");
                    return -EFAULT;
            }
            ret = disp_unlock_cmdq_thread(value);
            break;

        case DISP_IOCTL_MARK_CMQ:
            if(copy_from_user(&value, (void*)arg , sizeof(value)))
            {
                    DDP_DRV_ERR("DISP_IOCTL_MARK_CMQ, copy_from_user failed\n");
                    return -EFAULT;
            }
            if(value >= CMDQ_THREAD_NUM) return -EFAULT;
//            cmq_status[value] = 1;
            break;

        case DISP_IOCTL_WAIT_CMQ:
            if(copy_from_user(&value, (void*)arg , sizeof(value)))
            {
                    DDP_DRV_ERR("DISP_IOCTL_WAIT_CMQ, copy_from_user failed\n");
                    return -EFAULT;
            }
            if(value >= CMDQ_THREAD_NUM) return -EFAULT;
            /*
            wait_event_interruptible_timeout(cmq_wait_queue[value], cmq_status[value], 3 * HZ);
            if(cmq_status[value] != 0)
            {
                cmq_status[value] = 0;
                return -EFAULT;
            }
        */
            break;

        case DISP_IOCTL_LOCK_RESOURCE:
            if(copy_from_user(&mutex_id, (void*)arg , sizeof(int)))
            {
                DDP_DRV_ERR("DISP_IOCTL_LOCK_RESOURCE, copy_from_user failed\n");
                return -EFAULT;
            }
            if((-1) != mutex_id)
            {
                int ret = wait_event_interruptible_timeout(
                gResourceWaitQueue,
                (gLockedResource & (1 << mutex_id)) == 0,
                disp_ms2jiffies(50) );

                if(ret <= 0)
                {
                    DDP_DRV_ERR("DISP_IOCTL_LOCK_RESOURCE, mutex_id 0x%x failed\n",gLockedResource);
                    return -EFAULT;
                }

                spin_lock(&gResourceLock);
                gLockedResource |= (1 << mutex_id);
                spin_unlock(&gResourceLock);

                spin_lock(&pNode->node_lock);
                pNode->u4LockedResource = gLockedResource;
                spin_unlock(&pNode->node_lock);
            }
            else
            {
                DDP_DRV_ERR("DISP_IOCTL_LOCK_RESOURCE, mutex_id = -1 failed\n");
                return -EFAULT;
            }
            break;


        case DISP_IOCTL_UNLOCK_RESOURCE:
            if(copy_from_user(&mutex_id, (void*)arg , sizeof(int)))
            {
                DDP_DRV_ERR("DISP_IOCTL_UNLOCK_RESOURCE, copy_from_user failed\n");
                return -EFAULT;
            }
            if((-1) != mutex_id)
            {
                spin_lock(&gResourceLock);
                gLockedResource &= ~(1 << mutex_id);
                spin_unlock(&gResourceLock);

                spin_lock(&pNode->node_lock);
                pNode->u4LockedResource = gLockedResource;
                spin_unlock(&pNode->node_lock);

                wake_up_interruptible(&gResourceWaitQueue);
            }
            else
            {
                DDP_DRV_ERR("DISP_IOCTL_UNLOCK_RESOURCE, mutex_id = -1 failed\n");
                return -EFAULT;
            }
            break;

        case DISP_IOCTL_SYNC_REG:
            mb();
            break;

        case DISP_IOCTL_SET_INTR:
            DDP_DRV_DBG("DISP_IOCTL_SET_INTR! \n");
            if(copy_from_user(&value, (void*)arg , sizeof(int)))
            {
                DDP_DRV_ERR("DISP_IOCTL_SET_INTR, copy_from_user failed\n");
                return -EFAULT;
            }

            // enable intr
            if( (value&0xffff0000) !=0)
            {
                disable_irq(value&0xff);
                printk("disable_irq %d \n", value&0xff);
            }
            else
            {
                DISP_REGISTER_IRQ(value&0xff);
                printk("enable irq: %d \n", value&0xff);
            }
            break;

        case DISP_IOCTL_RUN_DPF:
            DDP_DRV_DBG("DISP_IOCTL_RUN_DPF! \n");
            if(copy_from_user(&value, (void*)arg , sizeof(int)))
            {
                DDP_DRV_ERR("DISP_IOCTL_SET_INTR, copy_from_user failed, %d\n", ret);
                return -EFAULT;
            }

            spin_lock(&gOvlLock);

            disp_run_dp_framework = value;

            spin_unlock(&gOvlLock);

            if(value == 1)
            {
                while(disp_get_mutex_status() != 0)
                {
                    DDP_DRV_ERR("disp driver : wait fb release hw mutex\n");
                    msleep(3);
                }
            }
            break;

        case DISP_IOCTL_CHECK_OVL:
            DDP_DRV_DBG("DISP_IOCTL_CHECK_OVL! \n");
            value = disp_layer_enable;

            if(copy_to_user((void *)arg, &value, sizeof(int)))
            {
                DDP_DRV_ERR("disp driver : Copy to user error (result)\n");
                return -EFAULT;
            }
            break;

        case DISP_IOCTL_GET_OVL:
            DDP_DRV_DBG("DISP_IOCTL_GET_OVL! \n");
            if(copy_from_user(&ovl_info, (void*)arg , sizeof(DISP_OVL_INFO)))
            {
                DDP_DRV_ERR("DISP_IOCTL_SET_INTR, copy_from_user failed, %d\n", ret);
                return -EFAULT;
            }

            layer = ovl_info.layer;

            spin_lock(&gOvlLock);
            ovl_info = disp_layer_info[layer];
            spin_unlock(&gOvlLock);

            if(copy_to_user((void *)arg, &ovl_info, sizeof(DISP_OVL_INFO)))
            {
                DDP_DRV_ERR("disp driver : Copy to user error (result)\n");
                return -EFAULT;
            }

            break;

        case DISP_IOCTL_AAL_EVENTCTL:
#if !defined(MTK_AAL_SUPPORT)
            printk("Invalid operation DISP_IOCTL_AAL_EVENTCTL since AAL is not turned on, in %s\n" , __FUNCTION__);
            return -EFAULT;
#else
            if(copy_from_user(&value, (void *)arg, sizeof(int)))
            {
                printk("disp driver : DISP_IOCTL_AAL_EVENTCTL Copy from user failed\n");
                return -EFAULT;
            }
            disp_set_aal_alarm(value);
            disp_set_needupdate(DISP_MODULE_BLS , 1);
            ret = 0;
#endif
            break;

        case DISP_IOCTL_GET_AALSTATISTICS:
#if !defined(MTK_AAL_SUPPORT)
            printk("Invalid operation DISP_IOCTL_GET_AALSTATISTICS since AAL is not turned on, in %s\n" , __FUNCTION__);
            return -EFAULT;
#else
            // 1. Wait till new interrupt comes
            if(disp_wait_hist_update(60))
            {
                printk("disp driver : DISP_IOCTL_GET_AALSTATISTICS wait time out\n");
                return -EFAULT;
            }

            // 2. read out pq engine histogram
            disp_set_hist_readlock(1);
            if(copy_to_user((void*)arg, (void *)(disp_get_hist_ptr()) , sizeof(DISP_AAL_STATISTICS)))
            {
                printk("disp driver : DISP_IOCTL_GET_AALSTATISTICS Copy to user failed\n");
                return -EFAULT;
            }
            disp_set_hist_readlock(0);
            ret = 0;
#endif
            break;

        case DISP_IOCTL_SET_AALPARAM:
#if !defined(MTK_AAL_SUPPORT)
            printk("Invalid operation : DISP_IOCTL_SET_AALPARAM since AAL is not turned on, in %s\n" , __FUNCTION__);
            return -EFAULT;
#else
//            disp_set_needupdate(DISP_MODULE_BLS , 0);

            disp_aal_lock();

            aal_param = get_aal_config();

            if(copy_from_user(aal_param , (void *)arg, sizeof(DISP_AAL_PARAM)))
            {
                printk("disp driver : DISP_IOCTL_SET_AALPARAM Copy from user failed\n");
                return -EFAULT;
            }

            g_AAL_LumaUpdated = 1;

            disp_aal_unlock();
#endif
            break;

        case DISP_IOCTL_SET_PQPARAM:

            DISP_RegisterExTriggerSource(CheckPQUpdateFunc , ConfPQFunc);

            GetUpdateMutex();

            pq_param = get_PQ_config();
            if(copy_from_user(pq_param, (void *)arg, sizeof(DISP_PQ_PARAM)))
            {
                printk("disp driver : DISP_IOCTL_SET_PQPARAM Copy from user failed\n");
                ReleaseUpdateMutex();
                return -EFAULT;
            }

            ReleaseUpdateMutex();

            disp_set_needupdate(DISP_MODULE_PQ, 1);

            break;

        case DISP_IOCTL_SET_PQINDEX:

            pq_index = get_PQ_index();
            if(copy_from_user(pq_index, (void *)arg, sizeof(DISPLAY_PQ_T)))
            {
                printk("disp driver : DISP_IOCTL_SET_PQINDEX Copy from user failed\n");
                return -EFAULT;
            }

            break;

        case DISP_IOCTL_GET_PQPARAM:

            pq_param = get_PQ_config();
            if(copy_to_user((void *)arg, pq_param, sizeof(DISP_PQ_PARAM)))
            {
                printk("disp driver : DISP_IOCTL_GET_PQPARAM Copy to user failed\n");
                return -EFAULT;
            }

            break;

        case DISP_IOCTL_SET_SHPINDEX:

            shp_index = get_SHP_index();
            if(copy_from_user(shp_index, (void *)arg, sizeof(DISPLAY_SHP_T)))
            {
                printk("disp driver : DISP_IOCTL_SET_SHPINDEX Copy from user failed\n");
                return -EFAULT;
            }

            break;

        case DISP_IOCTL_GET_SHPINDEX:

                shp_index = get_SHP_index();
                if(copy_to_user((void *)arg, shp_index, sizeof(DISPLAY_SHP_T)))
                {
                    printk("disp driver : DISP_IOCTL_GET_SHPINDEX Copy to user failed\n");
                    return -EFAULT;
                }

                break;

         case DISP_IOCTL_SET_GAMMALUT:

            DISP_MSG("DISP_IOCTL_SET_GAMMALUT\n");

            gamma_index = get_gamma_index();
            if(copy_from_user(gamma_index, (void *)arg, sizeof(DISPLAY_GAMMA_T)))
            {
                printk("disp driver : DISP_IOCTL_SET_GAMMALUT Copy from user failed\n");
                return -EFAULT;
            }

            // disable BLS
            GetUpdateMutex();
            bls_gamma_dirty = 1;
            ReleaseUpdateMutex();

            disp_set_needupdate(DISP_MODULE_PQ, 1);

            count = 0;
            while(DISP_REG_GET(DISP_REG_BLS_EN) & 0x1) {
                msleep(1);
                count++;
                if (count > 1000) {
                    DISP_ERR("fail to disable BLS (0x%x)\n", DISP_REG_GET(DISP_REG_BLS_EN));
                    break;
                }
            }

            // update gamma lut
            // enable BLS
            GetUpdateMutex();
            disp_bls_update_gamma_lut();
            bls_gamma_dirty = 0;
            ReleaseUpdateMutex(); 

            disp_set_needupdate(DISP_MODULE_PQ, 1);

            break;

         case DISP_IOCTL_SET_CLKON:
            if(copy_from_user(&module, (void *)arg, sizeof(DISP_MODULE_ENUM)))
            {
                printk("disp driver : DISP_IOCTL_SET_CLKON Copy from user failed\n");
                return -EFAULT;
            }

            disp_power_on(module , &(pNode->u4Clock));
            break;

        case DISP_IOCTL_SET_CLKOFF:
            if(copy_from_user(&module, (void *)arg, sizeof(DISP_MODULE_ENUM)))
            {
                printk("disp driver : DISP_IOCTL_SET_CLKOFF Copy from user failed\n");
                return -EFAULT;
            }

            disp_power_off(module , &(pNode->u4Clock));
            break;

        case DISP_IOCTL_MUTEX_CONTROL:
            if(copy_from_user(&value, (void *)arg, sizeof(int)))
            {
                printk("disp driver : DISP_IOCTL_MUTEX_CONTROL Copy from user failed\n");
                return -EFAULT;
            }

            DISP_MSG("DISP_IOCTL_MUTEX_CONTROL: %d, BLS_EN = %d\n", value, DISP_REG_GET(DISP_REG_BLS_EN));

            if(value == 1)
            {

                // disable BLS
                GetUpdateMutex();
                bls_gamma_dirty = 1;
                ReleaseUpdateMutex();

                disp_set_needupdate(DISP_MODULE_PQ, 1);

                count = 0;
                while(DISP_REG_GET(DISP_REG_BLS_EN) & 0x1) {
                    msleep(1);
                    count++;
                    if (count > 1000) {
                        DISP_ERR("fail to disable BLS (0x%x)\n", DISP_REG_GET(DISP_REG_BLS_EN));
                        break;
                    }
                }

                ncs_tuning_mode = 1;
                GetUpdateMutex();
            }
            else if(value == 2)
            {
                // enable BLS
                bls_gamma_dirty = 0;
                ReleaseUpdateMutex();

                disp_set_needupdate(DISP_MODULE_PQ, 1);
            }
            else
            {
                printk("disp driver : DISP_IOCTL_MUTEX_CONTROL invalid control\n");
                return -EFAULT;
            }

            DISP_MSG("DISP_IOCTL_MUTEX_CONTROL done: %d, BLS_EN = %d\n", value, DISP_REG_GET(DISP_REG_BLS_EN));

            break;

        case DISP_IOCTL_GET_LCMINDEX:

                lcmindex = DISP_GetLCMIndex();
                if(copy_to_user((void *)arg, &lcmindex, sizeof(unsigned long)))
                {
                    printk("disp driver : DISP_IOCTL_GET_LCMINDEX Copy to user failed\n");
                    return -EFAULT;
                }

                break;

            break;

        case DISP_IOCTL_SET_PQ_CAM_PARAM:

            pq_cam_param = get_PQ_Cam_config();
            if(copy_from_user(pq_cam_param, (void *)arg, sizeof(DISP_PQ_PARAM)))
            {
                printk("disp driver : DISP_IOCTL_SET_PQ_CAM_PARAM Copy from user failed\n");
                return -EFAULT;
            }

            break;

        case DISP_IOCTL_GET_PQ_CAM_PARAM:

            pq_cam_param = get_PQ_Cam_config();
            if(copy_to_user((void *)arg, pq_cam_param, sizeof(DISP_PQ_PARAM)))
            {
                printk("disp driver : DISP_IOCTL_GET_PQ_CAM_PARAM Copy to user failed\n");
                return -EFAULT;
            }

            break;

        case DISP_IOCTL_SET_PQ_GAL_PARAM:

            pq_gal_param = get_PQ_Gal_config();
            if(copy_from_user(pq_gal_param, (void *)arg, sizeof(DISP_PQ_PARAM)))
            {
                printk("disp driver : DISP_IOCTL_SET_PQ_GAL_PARAM Copy from user failed\n");
                return -EFAULT;
            }

            break;

        case DISP_IOCTL_GET_PQ_GAL_PARAM:

            pq_gal_param = get_PQ_Gal_config();
            if(copy_to_user((void *)arg, pq_gal_param, sizeof(DISP_PQ_PARAM)))
            {
                printk("disp driver : DISP_IOCTL_GET_PQ_GAL_PARAM Copy to user failed\n");
                return -EFAULT;
            }

            break;

        case DISP_IOCTL_TEST_PATH:
#ifdef DDP_DBG_DDP_PATH_CONFIG
            if(copy_from_user(&value, (void*)arg , sizeof(value)))
            {
                    DDP_DRV_ERR("DISP_IOCTL_MARK_CMQ, copy_from_user failed\n");
                    return -EFAULT;
            }

            config.layer = 0;
            config.layer_en = 1;
            config.source = OVL_LAYER_SOURCE_MEM;
            config.addr = virt_to_phys(inAddr);
            config.inFormat = OVL_INPUT_FORMAT_RGB565;
            config.pitch = 480;
            config.srcROI.x = 0;        // ROI
            config.srcROI.y = 0;
            config.srcROI.width = 480;
            config.srcROI.height = 800;
            config.bgROI.x = config.srcROI.x;
            config.bgROI.y = config.srcROI.y;
            config.bgROI.width = config.srcROI.width;
            config.bgROI.height = config.srcROI.height;
            config.bgColor = 0xff;  // background color
            config.key = 0xff;     // color key
            config.aen = 0;             // alpha enable
            config.alpha = 0;
            DDP_DRV_INFO("value=%d \n", value);
            if(value==0) // mem->ovl->rdma0->dpi0
            {
                config.srcModule = DISP_MODULE_OVL;
                config.outFormat = RDMA_OUTPUT_FORMAT_ARGB;
                config.dstModule = DISP_MODULE_DPI0;
                config.dstAddr = 0;
            }
            else if(value==1) // mem->ovl-> wdma1->mem
            {
                config.srcModule = DISP_MODULE_OVL;
                config.outFormat = WDMA_OUTPUT_FORMAT_RGB888;
                config.dstModule = DISP_MODULE_WDMA0;
                config.dstAddr = virt_to_phys(outAddr);
            }
            else if(value==2)  // mem->rdma0 -> dpi0
            {
                config.srcModule = DISP_MODULE_RDMA0;
                config.outFormat = RDMA_OUTPUT_FORMAT_ARGB;
                config.dstModule = DISP_MODULE_DPI0;
                config.dstAddr = 0;
            }
            disp_path_config(&config);
            disp_path_enable();
#endif
            break;
#if 0
        case DISP_IOCTL_G_WAIT_REQUEST:
            ret = ddp_bitblt_ioctl_wait_reequest(arg);
            break;

        case DISP_IOCTL_T_INFORM_DONE:
            ret = ddp_bitblt_ioctl_inform_done(arg);
            break;
#endif

        default :
            DDP_DRV_ERR("Ddp drv dose not have such command : %d\n" , cmd);
            break;
    }

    return ret;
}

static int disp_open(struct inode *inode, struct file *file)
{
    disp_node_struct *pNode = NULL;

    DDP_DRV_DBG("enter disp_open() process:%s\n",current->comm);

    //Allocate and initialize private data
    file->private_data = kmalloc(sizeof(disp_node_struct) , GFP_ATOMIC);
    if(NULL == file->private_data)
    {
        DDP_DRV_INFO("Not enough entry for DDP open operation\n");
        return -ENOMEM;
    }

    pNode = (disp_node_struct *)file->private_data;
    pNode->open_pid = current->pid;
    pNode->open_tgid = current->tgid;
    INIT_LIST_HEAD(&(pNode->testList));
    pNode->u4LockedResource = 0;
    pNode->u4Clock = 0;
    spin_lock_init(&pNode->node_lock);

    return 0;

}

static ssize_t disp_read(struct file *file, char __user *data, size_t len, loff_t *ppos)
{
    return 0;
}

static int disp_release(struct inode *inode, struct file *file)
{
    disp_node_struct *pNode = NULL;
    unsigned int index = 0;
    DDP_DRV_DBG("enter disp_release() process:%s\n",current->comm);

    pNode = (disp_node_struct *)file->private_data;

    spin_lock(&pNode->node_lock);

    if(pNode->u4LockedResource)
    {
        DDP_DRV_ERR("Proccess terminated[REsource] ! :%s , resource:%d\n"
            , current->comm , pNode->u4LockedResource);
        spin_lock(&gResourceLock);
        gLockedResource = 0;
        spin_unlock(&gResourceLock);
    }

    if(pNode->u4Clock)
    {
        DDP_DRV_ERR("Process safely terminated [Clock] !:%s , clock:%u\n"
            , current->comm , pNode->u4Clock);

        for(index  = 0 ; index < DISP_MODULE_MAX; index += 1)
        {
            if((1 << index) & pNode->u4Clock)
            {
                disp_power_off((DISP_MODULE_ENUM)index , &pNode->u4Clock);
            }
        }
    }

    cmdqTerminated();

    spin_unlock(&pNode->node_lock);

    if(NULL != file->private_data)
    {
        kfree(file->private_data);
        file->private_data = NULL;
    }

    return 0;
}

static int disp_flush(struct file * file , fl_owner_t a_id)
{
    return 0;
}

// remap register to user space
static int disp_mmap(struct file * file, struct vm_area_struct * a_pstVMArea)
{
    unsigned long size = a_pstVMArea->vm_end - a_pstVMArea->vm_start;
    unsigned long paStart = a_pstVMArea->vm_pgoff << PAGE_SHIFT;
    unsigned long paEnd = paStart + size;
    unsigned long MAX_SIZE = DISPSYS_REG_ADDR_MAX - DISPSYS_REG_ADDR_MIN;
    if (size > MAX_SIZE)
    {
        DISP_MSG("MMAP Size Range OVERFLOW!!\n");
        return -1;
    }
    if (paStart < (DISPSYS_REG_ADDR_MIN-0xE0000000) || paEnd > (DISPSYS_REG_ADDR_MAX-0xE0000000)) {
        DISP_MSG("MMAP Address Range OVERFLOW!!\n");
        return -1;
    }

    a_pstVMArea->vm_page_prot = pgprot_noncached(a_pstVMArea->vm_page_prot);
    if(remap_pfn_range(a_pstVMArea , 
                 a_pstVMArea->vm_start , 
                 a_pstVMArea->vm_pgoff , 
                 (a_pstVMArea->vm_end - a_pstVMArea->vm_start) , 
                 a_pstVMArea->vm_page_prot))
    {
        DDP_DRV_INFO("MMAP failed!!\n");
        return -1;
    }

    return 0;
}


/* Kernel interface */
static struct file_operations disp_fops = {
    .owner		= THIS_MODULE,
    .unlocked_ioctl = disp_unlocked_ioctl,
    .open		= disp_open,
    .release	= disp_release,
    .flush		= disp_flush,
    .read       = disp_read,
    .mmap       = disp_mmap
};


static int disp_probe(struct platform_device *pdev)
{
    struct class_device;

    int ret;
    int i;
    struct class_device *class_dev = NULL;

    DDP_DRV_INFO("\ndisp driver probe...\n\n");
    ret = alloc_chrdev_region(&disp_devno, 0, 1, DISP_DEVNAME);

    if(ret)
    {
        DDP_DRV_ERR("Error: Can't Get Major number for DISP Device\n");
    }
    else
    {
        DDP_DRV_INFO("Get DISP Device Major number (%d)\n", disp_devno);
    }

    disp_cdev = cdev_alloc();
    disp_cdev->owner = THIS_MODULE;
    disp_cdev->ops = &disp_fops;

    ret = cdev_add(disp_cdev, disp_devno, 1);

    disp_class = class_create(THIS_MODULE, DISP_DEVNAME);
    class_dev = (struct class_device *)device_create(disp_class, NULL, disp_devno, NULL, DISP_DEVNAME);

    // initial wait queue
    for(i = 0 ; i < CMDQ_THREAD_NUM ; i++)
    {
        init_waitqueue_head(&cmq_wait_queue[i]);

        // enable CMDQ interrupt
        DISP_REG_SET(DISP_REG_CMDQ_THRx_IRQ_FLAG_EN(i),0x13); //SL TEST CMDQ time out
    }

    // Register IRQ
    DISP_REGISTER_IRQ(MT_DISP_PQ_IRQ_ID);
    DISP_REGISTER_IRQ(MT_DISP_BLS_IRQ_ID);
    DISP_REGISTER_IRQ(MT_DISP_OVL_IRQ_ID  );
    DISP_REGISTER_IRQ(MT_DISP_WDMA_IRQ_ID);
    DISP_REGISTER_IRQ(MT_DISP_RDMA_IRQ_ID);
    DISP_REGISTER_IRQ(MT_CMDQ_IRQ_ID);
    DISP_REGISTER_IRQ(MT_MM_MUTEX_IRQ_ID);

    spin_lock_init(&gCmdqLock);
    spin_lock_init(&gResourceLock);
    spin_lock_init(&gOvlLock);
    spin_lock_init(&gRegisterUpdateLock);
    spin_lock_init(&gPowerOperateLock);
    spin_lock_init(&g_disp_irq.irq_lock);


    init_waitqueue_head(&disp_irq_log_wq);
    disp_irq_log_task = kthread_create(disp_irq_log_kthread_func, NULL, "disp_config_update_kthread");
    if (IS_ERR(disp_irq_log_task))
    {
        DDP_DRV_ERR("DISP_InitVSYNC(): Cannot create disp_irq_log_task kthread\n");
    }
    wake_up_process(disp_irq_log_task);

    spin_lock_init(&gCmdqMgrLock);

    DDP_DRV_INFO("DISP Probe Done\n");

    NOT_REFERENCED(class_dev);
    return 0;
}

static int disp_remove(struct platform_device *pdev)
{
    disable_irq(MT_DISP_OVL_IRQ_ID);
    disable_irq(MT_DISP_WDMA_IRQ_ID);
    disable_irq(MT_DISP_RDMA_IRQ_ID);
    disable_irq(MT_CMDQ_IRQ_ID);
    disable_irq(MT_DISP_PQ_IRQ_ID);
    disable_irq(MT_DISP_BLS_IRQ_ID);

    return 0;
}

static void disp_shutdown(struct platform_device *pdev)
{
	/* Nothing yet */
}

/* PM suspend */
static int disp_suspend(struct platform_device *pdev, pm_message_t mesg)
{
    printk("\n\n\n!!!!!!!disp_suspend: cmdqForceFreeAll !!!!!!!!!!!\n\n\n");

    //Only Thread 0 available in 6572
    cmdqForceFreeAll(0);

    return 0;
}

/* PM resume */
static int disp_resume(struct platform_device *pdev)
{
    // clear cmdq event for MDP engine
    DISP_REG_SET(DISP_REG_CMDQ_SYNC_TOKEN_UPDATE, 0x14);
    DISP_REG_SET(DISP_REG_CMDQ_SYNC_TOKEN_UPDATE, 0x15);
    DISP_REG_SET(DISP_REG_CMDQ_SYNC_TOKEN_UPDATE, 0x16);

    return 0;
}


static struct platform_driver disp_driver = {
    .probe		= disp_probe,
    .remove		= disp_remove,
    .shutdown	= disp_shutdown,
    .suspend	= disp_suspend,
    .resume		= disp_resume,
    .driver     = {
    .name = DISP_DEVNAME,
    },
};

static void disp_device_release(struct device *dev)
{
    // Nothing to release?
}

static u64 disp_dmamask = ~(u32)0;

static struct platform_device disp_device = {
    .name	 = DISP_DEVNAME,
    .id      = 0,
    .dev     = {
    .release = disp_device_release,
    .dma_mask = &disp_dmamask,
    .coherent_dma_mask = 0xffffffff,
    },
    .num_resources = 0,
};

static int __init disp_init(void)
{
    int ret;

    DDP_DRV_INFO("Register disp device\n");
    if(platform_device_register(&disp_device))
    {
        DDP_DRV_ERR("failed to register disp device\n");
        ret = -ENODEV;
        return ret;
    }

    DDP_DRV_INFO("Register the disp driver\n");
    if(platform_driver_register(&disp_driver))
    {
        DDP_DRV_ERR("failed to register disp driver\n");
        platform_device_unregister(&disp_device);
        ret = -ENODEV;
        return ret;
    }

    ddp_debug_init();

    pRegBackup = kmalloc(DDP_BACKUP_REG_NUM*sizeof(int), GFP_KERNEL);
    ASSERT(pRegBackup!=NULL);
    *pRegBackup = DDP_UNBACKED_REG_MEM;

    cmdq_dma_init();
    // clear cmdq event for MDP engine
    DISP_REG_SET(DISP_REG_CMDQ_SYNC_TOKEN_UPDATE, 0x14);
    DISP_REG_SET(DISP_REG_CMDQ_SYNC_TOKEN_UPDATE, 0x15);
    DISP_REG_SET(DISP_REG_CMDQ_SYNC_TOKEN_UPDATE, 0x16);

    return 0;
}

static void __exit disp_exit(void)
{
    cdev_del(disp_cdev);
    unregister_chrdev_region(disp_devno, 1);

    platform_driver_unregister(&disp_driver);
    platform_device_unregister(&disp_device);

    device_destroy(disp_class, disp_devno);
    class_destroy(disp_class);

    ddp_debug_exit();

    DDP_DRV_INFO("Done\n");
}

int disp_set_overlay_roi(int layer, int x, int y, int w, int h, int pitch)
{
    // DDP_DRV_INFO(" disp_set_overlay_roi %d\n", layer );

    if(layer < 0 || layer >= DDP_OVL_LAYER_MUN) return -1;
    spin_lock(&gOvlLock);

    disp_layer_info[layer].x = x;
    disp_layer_info[layer].y = y;
    disp_layer_info[layer].w = w;
    disp_layer_info[layer].h = h;
    disp_layer_info[layer].pitch = pitch;

    spin_unlock(&gOvlLock);

    return 0;
}

int disp_set_overlay_addr(int layer, unsigned int addr, DDP_OVL_FORMAT fmt)
{
    // DDP_DRV_INFO(" disp_set_overlay_addr %d\n", layer );
    if(layer < 0 || layer >= DDP_OVL_LAYER_MUN) return -1;

    spin_lock(&gOvlLock);

    disp_layer_info[layer].addr = addr;
    if(fmt != DDP_NONE_FMT)
        disp_layer_info[layer].fmt = fmt;

    spin_unlock(&gOvlLock);

    return 0;
}

int disp_set_overlay(int layer, int enable)
{
    // DDP_DRV_INFO(" disp_set_overlay %d %d\n", layer, enable );
    if(layer < 0 || layer >= DDP_OVL_LAYER_MUN) return -1;

    spin_lock(&gOvlLock);

    if(enable == 0)
        disp_layer_enable = disp_layer_enable & ~(1 << layer);
    else
        disp_layer_enable = disp_layer_enable | (1 << layer);

    spin_unlock(&gOvlLock);

    return 0;
}

int disp_is_dp_framework_run()
{
    // DDP_DRV_INFO(" disp_is_dp_framework_run " );
    return disp_run_dp_framework;
}

int disp_set_mutex_status(int enable)
{
    // DDP_DRV_INFO(" disp_set_mutex_status %d\n", enable );
    spin_lock(&gOvlLock);

    disp_mutex_status = enable;

    spin_unlock(&gOvlLock);
    return 0;
}

int disp_get_mutex_status()
{
    return disp_mutex_status;
}

int disp_dump_reg(DISP_MODULE_ENUM module)
{
    switch (module)
    {
        case DISP_MODULE_CONFIG :
            DDP_DRV_INFO("===== DISP CFG Reg Dump: ============\n");
            DDP_DRV_INFO("(0x01C)DISP_REG_CONFIG_CAM_MDP_MOUT_EN          = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_CAM_MDP_MOUT_EN));
            DDP_DRV_INFO("(0x020)DISP_REG_CONFIG_MDP_RDMA_MOUT_EN         = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MDP_RDMA_MOUT_EN));
            DDP_DRV_INFO("(0x024)DISP_REG_CONFIG_MDP_RSZ0_MOUT_EN         = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MDP_RSZ0_MOUT_EN));
            DDP_DRV_INFO("(0x028)DISP_REG_CONFIG_MDP_RSZ1_MOUT_EN         = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MDP_RSZ1_MOUT_EN));
            DDP_DRV_INFO("(0x02C)DISP_REG_CONFIG_MDP_SHP_MOUT_EN          = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MDP_SHP_MOUT_EN));
            DDP_DRV_INFO("(0x030)DISP_REG_CONFIG_DISP_OVL_MOUT_EN         = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_DISP_OVL_MOUT_EN));
            DDP_DRV_INFO("(0x034)DISP_REG_CONFIG_MMSYS_MOUT_RST           = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MMSYS_MOUT_RST));
            DDP_DRV_INFO("(0x038)DISP_REG_CONFIG_MDP_RSZ0_SEL             = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MDP_RSZ0_SEL));
            DDP_DRV_INFO("(0x03C)DISP_REG_CONFIG_MDP_RSZ1_SEL             = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MDP_RSZ1_SEL));
            DDP_DRV_INFO("(0x040)DISP_REG_CONFIG_MDP_SHP_SEL              = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MDP_SHP_SEL));
            DDP_DRV_INFO("(0x044)DISP_REG_CONFIG_MDP_WROT_SEL             = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MDP_WROT_SEL));
            DDP_DRV_INFO("(0x048)DISP_REG_CONFIG_MDP_WDMA_SEL             = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MDP_WDMA_SEL));
            DDP_DRV_INFO("(0x04C)DISP_REG_CONFIG_DISP_BLS_SOUT_SEL        = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_DISP_BLS_SOUT_SEL));
            DDP_DRV_INFO("(0x050)DISP_REG_CONFIG_DISP_RDMA_SOUT_SEL       = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_DISP_RDMA_SOUT_SEL));
            DDP_DRV_INFO("(0x054)DISP_REG_CONFIG_DISP_PQ_SEL              = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_DISP_PQ_SEL));
            DDP_DRV_INFO("(0x058)DISP_REG_CONFIG_DISP_DSI_SEL             = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_DISP_DSI_SEL));
            DDP_DRV_INFO("(0x05C)DISP_REG_CONFIG_DISP_DPI_SEL             = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_DISP_DPI_SEL));
            DDP_DRV_INFO("(0x060)DISP_REG_CONFIG_DBI_PAD_DELAY_SEL        = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_DBI_PAD_DELAY_SEL));
            DDP_DRV_INFO("(0x064)DISP_REG_CONFIG_DBI_DPI_IO_SEL           = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_DBI_DPI_IO_SEL));
            DDP_DRV_INFO("(0x070)DISP_REG_CONFIG_LCD_RESET_CON            = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_LCD_RESET_CON));
            DDP_DRV_INFO("(0x100)DISP_REG_CONFIG_MMSYS_CG_CON0            = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MMSYS_CG_CON0));
            DDP_DRV_INFO("(0x104)DISP_REG_CONFIG_MMSYS_CG_SET0            = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MMSYS_CG_SET0));
            DDP_DRV_INFO("(0x108)DISP_REG_CONFIG_MMSYS_CG_CLR0            = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MMSYS_CG_CLR0));
            DDP_DRV_INFO("(0x110)DISP_REG_CONFIG_MMSYS_CG_CON1            = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MMSYS_CG_CON1));
            DDP_DRV_INFO("(0x114)DISP_REG_CONFIG_MMSYS_CG_SET1            = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MMSYS_CG_SET1));
            DDP_DRV_INFO("(0x118)DISP_REG_CONFIG_MMSYS_CG_CLR1            = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MMSYS_CG_CLR1));
            DDP_DRV_INFO("(0x120)DISP_REG_CONFIG_MMSYS_HW_DCM_DIS0        = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MMSYS_HW_DCM_DIS0));
            DDP_DRV_INFO("(0x124)DISP_REG_CONFIG_MMSYS_HW_DCM_DIS_SET0    = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MMSYS_HW_DCM_DIS_SET0));
            DDP_DRV_INFO("(0x128)DISP_REG_CONFIG_MMSYS_HW_DCM_DIS_CLR0    = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MMSYS_HW_DCM_DIS_CLR0));
            DDP_DRV_INFO("(0x12C)DISP_REG_CONFIG_MMSYS_HW_DCM_DIS1        = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MMSYS_HW_DCM_DIS1));
            DDP_DRV_INFO("(0x130)DISP_REG_CONFIG_MMSYS_HW_DCM_DIS_SET1    = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MMSYS_HW_DCM_DIS_SET1));
            DDP_DRV_INFO("(0x134)DISP_REG_CONFIG_MMSYS_HW_DCM_DIS_CLR1    = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MMSYS_HW_DCM_DIS_CLR1));
            DDP_DRV_INFO("(0x138)DISP_REG_CONFIG_MMSYS_SW_RST_B           = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MMSYS_SW_RST_B));
            DDP_DRV_INFO("(0x200)DISP_REG_CONFIG_DISP_FAKE_ENG_EN         = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_DISP_FAKE_ENG_EN));
            DDP_DRV_INFO("(0x204)DISP_REG_CONFIG_DISP_FAKE_ENG_RST        = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_DISP_FAKE_ENG_RST));
            DDP_DRV_INFO("(0x208)DISP_REG_CONFIG_DISP_FAKE_ENG_CON0       = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_DISP_FAKE_ENG_CON0));
            DDP_DRV_INFO("(0x20C)DISP_REG_CONFIG_DISP_FAKE_ENG_CON1       = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_DISP_FAKE_ENG_CON1));
            DDP_DRV_INFO("(0x210)DISP_REG_CONFIG_DISP_FAKE_ENG_RD_ADDR    = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_DISP_FAKE_ENG_RD_ADDR));
            DDP_DRV_INFO("(0x214)DISP_REG_CONFIG_DISP_FAKE_ENG_WR_ADDR    = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_DISP_FAKE_ENG_WR_ADDR));
            DDP_DRV_INFO("(0x218)DISP_REG_CONFIG_DISP_FAKE_ENG_STATE      = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_DISP_FAKE_ENG_STATE));
            DDP_DRV_INFO("(0x800)DISP_REG_CONFIG_MMSYS_MBIST_MODE         = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MMSYS_MBIST_MODE));
            DDP_DRV_INFO("(0x804)DISP_REG_CONFIG_MMSYS_MBIST_HOLDB        = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MMSYS_MBIST_HOLDB));
            DDP_DRV_INFO("(0x808)DISP_REG_CONFIG_MMSYS_MBIST_CON          = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MMSYS_MBIST_CON));
            DDP_DRV_INFO("(0x80C)DISP_REG_CONFIG_MMSYS_MBIST_DONE         = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MMSYS_MBIST_DONE));
            DDP_DRV_INFO("(0x810)DISP_REG_CONFIG_MMSYS_MBIST_FAIL0        = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MMSYS_MBIST_FAIL0));
            DDP_DRV_INFO("(0x814)DISP_REG_CONFIG_MMSYS_MBIST_FAIL1        = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MMSYS_MBIST_FAIL1));
            DDP_DRV_INFO("(0x818)DISP_REG_CONFIG_MMSYS_MBIST_FAIL2        = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MMSYS_MBIST_FAIL2));
            DDP_DRV_INFO("(0x820)DISP_REG_CONFIG_MMSYS_MBIST_BSEL0        = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MMSYS_MBIST_BSEL0));
            DDP_DRV_INFO("(0x824)DISP_REG_CONFIG_MMSYS_MBIST_BSEL1        = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MMSYS_MBIST_BSEL1));
            DDP_DRV_INFO("(0x828)DISP_REG_CONFIG_MMSYS_MBIST_BSEL2        = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MMSYS_MBIST_BSEL2));
            DDP_DRV_INFO("(0x830)DISP_REG_CONFIG_MMSYS_MEM_DELSEL0        = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MMSYS_MEM_DELSEL0));
            DDP_DRV_INFO("(0x834)DISP_REG_CONFIG_MMSYS_MEM_DELSEL1        = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MMSYS_MEM_DELSEL1));
            DDP_DRV_INFO("(0x838)DISP_REG_CONFIG_MMSYS_MEM_DELSEL2        = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MMSYS_MEM_DELSEL2));
            DDP_DRV_INFO("(0x83C)DISP_REG_CONFIG_MMSYS_MEM_DELSEL3        = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MMSYS_MEM_DELSEL3));
            DDP_DRV_INFO("(0x840)DISP_REG_CONFIG_MMSYS_MEM_DELSEL4        = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MMSYS_MEM_DELSEL4));
            DDP_DRV_INFO("(0x844)DISP_REG_CONFIG_MMSYS_MEM_DELSEL5        = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MMSYS_MEM_DELSEL5));
            DDP_DRV_INFO("(0x848)DISP_REG_CONFIG_MMSYS_MEM_DELSEL6        = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MMSYS_MEM_DELSEL6));
            DDP_DRV_INFO("(0x84C)DISP_REG_CONFIG_MMSYS_MEM_DELSEL7        = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MMSYS_MEM_DELSEL7));
            DDP_DRV_INFO("(0x850)DISP_REG_CONFIG_MDP_WROT_MBISR_RESET     = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MDP_WROT_MBISR_RESET));
            DDP_DRV_INFO("(0x854)DISP_REG_CONFIG_MDP_WROT_MBISR_FAIL      = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MDP_WROT_MBISR_FAIL));
            DDP_DRV_INFO("(0x858)DISP_REG_CONFIG_MDP_WROT_MBISR_OK        = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MDP_WROT_MBISR_OK));
            DDP_DRV_INFO("(0x860)DISP_REG_CONFIG_MMSYS_DEBUG_OUT_SEL      = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MMSYS_DEBUG_OUT_SEL));
            DDP_DRV_INFO("(0x864)DISP_REG_CONFIG_MMSYS_DUMMY              = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MMSYS_DUMMY));
            DDP_DRV_INFO("(0x870)DISP_REG_CONFIG_MMSYS_DL_VALID_0         = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MMSYS_DL_VALID_0));
            DDP_DRV_INFO("(0x874)DISP_REG_CONFIG_MMSYS_DL_READY_0         = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MMSYS_DL_READY_0));
            DDP_DRV_INFO("(0x880)DISP_REG_CONFIG_MMSYS_DL_VALID_1         = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MMSYS_DL_VALID_1));
            DDP_DRV_INFO("(0x884)DISP_REG_CONFIG_MMSYS_DL_READY_1         = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MMSYS_DL_READY_1));
            break;

        case DISP_MODULE_OVL :
            DDP_DRV_INFO("===== DISP OVL Reg Dump: ============\n");
            DDP_DRV_INFO("(0x000)DISP_REG_OVL_STA                         = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_STA));
            DDP_DRV_INFO("(0x004)DISP_REG_OVL_INTEN                       = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_INTEN));
            DDP_DRV_INFO("(0x008)DISP_REG_OVL_INTSTA                      = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_INTSTA));
            DDP_DRV_INFO("(0x00C)DISP_REG_OVL_EN                          = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_EN));
            DDP_DRV_INFO("(0x010)DISP_REG_OVL_TRIG                        = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_TRIG));
            DDP_DRV_INFO("(0x014)DISP_REG_OVL_RST                         = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_RST));
            DDP_DRV_INFO("(0x020)DISP_REG_OVL_ROI_SIZE                    = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_ROI_SIZE));
            DDP_DRV_INFO("(0x024)DISP_REG_OVL_DATAPATH_CON                = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_DATAPATH_CON));
            DDP_DRV_INFO("(0x028)DISP_REG_OVL_ROI_BGCLR                   = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_ROI_BGCLR));
            DDP_DRV_INFO("(0x02C)DISP_REG_OVL_SRC_CON                     = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_SRC_CON));
            DDP_DRV_INFO("(0x030)DISP_REG_OVL_L0_CON                      = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L0_CON));
            DDP_DRV_INFO("(0x034)DISP_REG_OVL_L0_SRCKEY                   = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L0_SRCKEY));
            DDP_DRV_INFO("(0x038)DISP_REG_OVL_L0_SRC_SIZE                 = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L0_SRC_SIZE));
            DDP_DRV_INFO("(0x03C)DISP_REG_OVL_L0_OFFSET                   = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L0_OFFSET));
            DDP_DRV_INFO("(0x040)DISP_REG_OVL_L0_ADDR                     = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L0_ADDR));
            DDP_DRV_INFO("(0x044)DISP_REG_OVL_L0_PITCH                    = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L0_PITCH));
            DDP_DRV_INFO("(0x048)DISP_REG_OVL_L0_TILE                     = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L0_TILE));
            DDP_DRV_INFO("(0x050)DISP_REG_OVL_L1_CON                      = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L1_CON));
            DDP_DRV_INFO("(0x054)DISP_REG_OVL_L1_SRCKEY                   = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L1_SRCKEY));
            DDP_DRV_INFO("(0x058)DISP_REG_OVL_L1_SRC_SIZE                 = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L1_SRC_SIZE));
            DDP_DRV_INFO("(0x05C)DISP_REG_OVL_L1_OFFSET                   = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L1_OFFSET));
            DDP_DRV_INFO("(0x060)DISP_REG_OVL_L1_ADDR                     = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L1_ADDR));
            DDP_DRV_INFO("(0x064)DISP_REG_OVL_L1_PITCH                    = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L1_PITCH));
            DDP_DRV_INFO("(0x068)DISP_REG_OVL_L1_TILE                     = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L1_TILE));
            DDP_DRV_INFO("(0x070)DISP_REG_OVL_L2_CON                      = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L2_CON));
            DDP_DRV_INFO("(0x074)DISP_REG_OVL_L2_SRCKEY                   = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L2_SRCKEY));
            DDP_DRV_INFO("(0x078)DISP_REG_OVL_L2_SRC_SIZE                 = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L2_SRC_SIZE));
            DDP_DRV_INFO("(0x07C)DISP_REG_OVL_L2_OFFSET                   = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L2_OFFSET));
            DDP_DRV_INFO("(0x080)DISP_REG_OVL_L2_ADDR                     = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L2_ADDR));
            DDP_DRV_INFO("(0x084)DISP_REG_OVL_L2_PITCH                    = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L2_PITCH));
            DDP_DRV_INFO("(0x088)DISP_REG_OVL_L2_TILE                     = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L2_TILE));
            DDP_DRV_INFO("(0x090)DISP_REG_OVL_L3_CON                      = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L3_CON));
            DDP_DRV_INFO("(0x094)DISP_REG_OVL_L3_SRCKEY                   = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L3_SRCKEY));
            DDP_DRV_INFO("(0x098)DISP_REG_OVL_L3_SRC_SIZE                 = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L3_SRC_SIZE));
            DDP_DRV_INFO("(0x09C)DISP_REG_OVL_L3_OFFSET                   = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L3_OFFSET));
            DDP_DRV_INFO("(0x0A0)DISP_REG_OVL_L3_ADDR                     = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L3_ADDR));
            DDP_DRV_INFO("(0x0A4)DISP_REG_OVL_L3_PITCH                    = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L3_PITCH));
            DDP_DRV_INFO("(0x0A8)DISP_REG_OVL_L3_TILE                     = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L3_TILE));
            DDP_DRV_INFO("(0x0C0)DISP_REG_OVL_RDMA0_CTRL                  = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_RDMA0_CTRL));
            DDP_DRV_INFO("(0x0C8)DISP_REG_OVL_RDMA0_MEM_GMC_SETTING1      = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_RDMA0_MEM_GMC_SETTING1));
            DDP_DRV_INFO("(0x0CC)DISP_REG_OVL_RDMA0_MEM_SLOW_CON          = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_RDMA0_MEM_SLOW_CON));
            DDP_DRV_INFO("(0x0D0)DISP_REG_OVL_RDMA0_FIFO_CTRL             = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_RDMA0_FIFO_CTRL));
            DDP_DRV_INFO("(0x0E0)DISP_REG_OVL_RDMA1_CTRL                  = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_RDMA1_CTRL));
            DDP_DRV_INFO("(0x0E8)DISP_REG_OVL_RDMA1_MEM_GMC_SETTING1      = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_RDMA1_MEM_GMC_SETTING1));
            DDP_DRV_INFO("(0x0EC)DISP_REG_OVL_RDMA1_MEM_SLOW_CON          = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_RDMA1_MEM_SLOW_CON));
            DDP_DRV_INFO("(0x0F0)DISP_REG_OVL_RDMA1_FIFO_CTRL             = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_RDMA1_FIFO_CTRL));
            DDP_DRV_INFO("(0x100)DISP_REG_OVL_RDMA2_CTRL                  = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_RDMA2_CTRL));
            DDP_DRV_INFO("(0x108)DISP_REG_OVL_RDMA2_MEM_GMC_SETTING1      = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_RDMA2_MEM_GMC_SETTING1));
            DDP_DRV_INFO("(0x10C)DISP_REG_OVL_RDMA2_MEM_SLOW_CON          = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_RDMA2_MEM_SLOW_CON));
            DDP_DRV_INFO("(0x110)DISP_REG_OVL_RDMA2_FIFO_CTRL             = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_RDMA2_FIFO_CTRL));
            DDP_DRV_INFO("(0x120)DISP_REG_OVL_RDMA3_CTRL                  = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_RDMA3_CTRL));
            DDP_DRV_INFO("(0x128)DISP_REG_OVL_RDMA3_MEM_GMC_SETTING1      = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_RDMA3_MEM_GMC_SETTING1));
            DDP_DRV_INFO("(0x12C)DISP_REG_OVL_RDMA3_MEM_SLOW_CON          = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_RDMA3_MEM_SLOW_CON));
            DDP_DRV_INFO("(0x130)DISP_REG_OVL_RDMA3_FIFO_CTRL             = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_RDMA3_FIFO_CTRL));
            DDP_DRV_INFO("(0x134)DISP_REG_OVL_L0_Y2R_PARA_R0              = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L0_Y2R_PARA_R0));
            DDP_DRV_INFO("(0x138)DISP_REG_OVL_L0_Y2R_PARA_R1              = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L0_Y2R_PARA_R1));
            DDP_DRV_INFO("(0x13C)DISP_REG_OVL_L0_Y2R_PARA_G0              = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L0_Y2R_PARA_G0));
            DDP_DRV_INFO("(0x140)DISP_REG_OVL_L0_Y2R_PARA_G1              = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L0_Y2R_PARA_G1));
            DDP_DRV_INFO("(0x144)DISP_REG_OVL_L0_Y2R_PARA_B0              = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L0_Y2R_PARA_B0));
            DDP_DRV_INFO("(0x148)DISP_REG_OVL_L0_Y2R_PARA_B1              = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L0_Y2R_PARA_B1));
            DDP_DRV_INFO("(0x14C)DISP_REG_OVL_L0_Y2R_PARA_YUV_A_0         = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L0_Y2R_PARA_YUV_A_0));
            DDP_DRV_INFO("(0x150)DISP_REG_OVL_L0_Y2R_PARA_YUV_A_1         = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L0_Y2R_PARA_YUV_A_1));
            DDP_DRV_INFO("(0x154)DISP_REG_OVL_L0_Y2R_PARA_RGB_A_0         = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L0_Y2R_PARA_RGB_A_0));
            DDP_DRV_INFO("(0x158)DISP_REG_OVL_L0_Y2R_PARA_RGB_A_1         = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L0_Y2R_PARA_RGB_A_1));
            DDP_DRV_INFO("(0x15C)DISP_REG_OVL_L1_Y2R_PARA_R0              = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L1_Y2R_PARA_R0));
            DDP_DRV_INFO("(0x160)DISP_REG_OVL_L1_Y2R_PARA_R1              = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L1_Y2R_PARA_R1));
            DDP_DRV_INFO("(0x164)DISP_REG_OVL_L1_Y2R_PARA_G0              = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L1_Y2R_PARA_G0));
            DDP_DRV_INFO("(0x168)DISP_REG_OVL_L1_Y2R_PARA_G1              = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L1_Y2R_PARA_G1));
            DDP_DRV_INFO("(0x16C)DISP_REG_OVL_L1_Y2R_PARA_B0              = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L1_Y2R_PARA_B0));
            DDP_DRV_INFO("(0x170)DISP_REG_OVL_L1_Y2R_PARA_B1              = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L1_Y2R_PARA_B1));
            DDP_DRV_INFO("(0x174)DISP_REG_OVL_L1_Y2R_PARA_YUV_A_0         = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L1_Y2R_PARA_YUV_A_0));
            DDP_DRV_INFO("(0x178)DISP_REG_OVL_L1_Y2R_PARA_YUV_A_1         = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L1_Y2R_PARA_YUV_A_1));
            DDP_DRV_INFO("(0x17C)DISP_REG_OVL_L1_Y2R_PARA_RGB_A_0         = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L1_Y2R_PARA_RGB_A_0));
            DDP_DRV_INFO("(0x180)DISP_REG_OVL_L1_Y2R_PARA_RGB_A_1         = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L1_Y2R_PARA_RGB_A_1));
            DDP_DRV_INFO("(0x184)DISP_REG_OVL_L2_Y2R_PARA_R0              = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L2_Y2R_PARA_R0));
            DDP_DRV_INFO("(0x188)DISP_REG_OVL_L2_Y2R_PARA_R1              = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L2_Y2R_PARA_R1));
            DDP_DRV_INFO("(0x18C)DISP_REG_OVL_L2_Y2R_PARA_G0              = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L2_Y2R_PARA_G0));
            DDP_DRV_INFO("(0x190)DISP_REG_OVL_L2_Y2R_PARA_G1              = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L2_Y2R_PARA_G1));
            DDP_DRV_INFO("(0x194)DISP_REG_OVL_L2_Y2R_PARA_B0              = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L2_Y2R_PARA_B0));
            DDP_DRV_INFO("(0x198)DISP_REG_OVL_L2_Y2R_PARA_B1              = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L2_Y2R_PARA_B1));
            DDP_DRV_INFO("(0x19C)DISP_REG_OVL_L2_Y2R_PARA_YUV_A_0         = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L2_Y2R_PARA_YUV_A_0));
            DDP_DRV_INFO("(0x1A0)DISP_REG_OVL_L2_Y2R_PARA_YUV_A_1         = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L2_Y2R_PARA_YUV_A_1));
            DDP_DRV_INFO("(0x1A4)DISP_REG_OVL_L2_Y2R_PARA_RGB_A_0         = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L2_Y2R_PARA_RGB_A_0));
            DDP_DRV_INFO("(0x1A8)DISP_REG_OVL_L2_Y2R_PARA_RGB_A_1         = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L2_Y2R_PARA_RGB_A_1));
            DDP_DRV_INFO("(0x1AC)DISP_REG_OVL_L3_Y2R_PARA_R0              = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L3_Y2R_PARA_R0));
            DDP_DRV_INFO("(0x1B0)DISP_REG_OVL_L3_Y2R_PARA_R1              = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L3_Y2R_PARA_R1));
            DDP_DRV_INFO("(0x1B4)DISP_REG_OVL_L3_Y2R_PARA_G0              = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L3_Y2R_PARA_G0));
            DDP_DRV_INFO("(0x1B8)DISP_REG_OVL_L3_Y2R_PARA_G1              = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L3_Y2R_PARA_G1));
            DDP_DRV_INFO("(0x1BC)DISP_REG_OVL_L3_Y2R_PARA_B0              = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L3_Y2R_PARA_B0));
            DDP_DRV_INFO("(0x1C0)DISP_REG_OVL_L3_Y2R_PARA_B1              = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L3_Y2R_PARA_B1));
            DDP_DRV_INFO("(0x1C4)DISP_REG_OVL_L3_Y2R_PARA_YUV_A_0         = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L3_Y2R_PARA_YUV_A_0));
            DDP_DRV_INFO("(0x1C8)DISP_REG_OVL_L3_Y2R_PARA_YUV_A_1         = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L3_Y2R_PARA_YUV_A_1));
            DDP_DRV_INFO("(0x1CC)DISP_REG_OVL_L3_Y2R_PARA_RGB_A_0         = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L3_Y2R_PARA_RGB_A_0));
            DDP_DRV_INFO("(0x1D0)DISP_REG_OVL_L3_Y2R_PARA_RGB_A_1         = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_L3_Y2R_PARA_RGB_A_1));
            DDP_DRV_INFO("(0x1D4)DISP_REG_OVL_DEBUG_MON_SEL               = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_DEBUG_MON_SEL));
            DDP_DRV_INFO("(0x1E0)DISP_REG_OVL_RDMA0_MEM_GMC_SETTING2      = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_RDMA0_MEM_GMC_SETTING2));
            DDP_DRV_INFO("(0x1E4)DISP_REG_OVL_RDMA1_MEM_GMC_SETTING2      = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_RDMA1_MEM_GMC_SETTING2));
            DDP_DRV_INFO("(0x1E8)DISP_REG_OVL_RDMA2_MEM_GMC_SETTING2      = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_RDMA2_MEM_GMC_SETTING2));
            DDP_DRV_INFO("(0x1EC)DISP_REG_OVL_RDMA3_MEM_GMC_SETTING2      = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_RDMA3_MEM_GMC_SETTING2));
            DDP_DRV_INFO("(0x200)DISP_REG_OVL_DUMMY_REG                   = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_DUMMY_REG));
            DDP_DRV_INFO("(0x240)DISP_REG_OVL_FLOW_CTRL_DBG               = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_FLOW_CTRL_DBG));
            DDP_DRV_INFO("(0x244)DISP_REG_OVL_ADDCON_DBG                  = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_ADDCON_DBG));
            DDP_DRV_INFO("(0x24C)DISP_REG_OVL_RDMA0_DBG                   = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_RDMA0_DBG));
            DDP_DRV_INFO("(0x250)DISP_REG_OVL_RDMA1_DBG                   = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_RDMA1_DBG));
            DDP_DRV_INFO("(0x254)DISP_REG_OVL_RDMA2_DBG                   = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_RDMA2_DBG));
            DDP_DRV_INFO("(0x258)DISP_REG_OVL_RDMA3_DBG                   = 0x%08X\n", DISP_REG_GET(DISP_REG_OVL_RDMA3_DBG));
            break;

        case DISP_MODULE_PQ :
            DDP_DRV_INFO("===== DISP PQ Reg Dump: ============\n");
            DDP_DRV_INFO("(0x000)DISP_REG_PQ_CTRL                         = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_CTRL));
            DDP_DRV_INFO("(0x004)DISP_REG_PQ_INTEN                        = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_INTEN));
            DDP_DRV_INFO("(0x008)DISP_REG_PQ_INTSTA                       = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_INTSTA));
            DDP_DRV_INFO("(0x00C)DISP_REG_PQ_STATUS                       = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_STATUS));
            DDP_DRV_INFO("(0x010)DISP_REG_PQ_CFG                          = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_CFG));
            DDP_DRV_INFO("(0x014)DISP_REG_PQ_INPUT_COUNT                  = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_INPUT_COUNT));
            DDP_DRV_INFO("(0x018)DISP_REG_PQ_CHKSUM                       = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_CHKSUM));
            DDP_DRV_INFO("(0x01C)DISP_REG_PQ_OUTPUT_COUNT                 = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_OUTPUT_COUNT));
            DDP_DRV_INFO("(0x020)DISP_REG_PQ_INPUT_SIZE                   = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_INPUT_SIZE));
            DDP_DRV_INFO("(0x024)DISP_REG_PQ_OUTPUT_OFFSET                = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_OUTPUT_OFFSET));
            DDP_DRV_INFO("(0x028)DISP_REG_PQ_OUTPUT_SIZE                  = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_OUTPUT_SIZE));
            DDP_DRV_INFO("(0x02C)DISP_REG_PQ_HSYNC                        = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_HSYNC));
            DDP_DRV_INFO("(0x030)DISP_REG_PQ_DEMO_HMASK                   = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_DEMO_HMASK));
            DDP_DRV_INFO("(0x034)DISP_REG_PQ_DEMO_VMASK                   = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_DEMO_VMASK));
            DDP_DRV_INFO("(0x040)DISP_REG_PQ_SHP_CON_00                   = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_SHP_CON_00));
            DDP_DRV_INFO("(0x044)DISP_REG_PQ_SHP_CON_01                   = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_SHP_CON_01));
            DDP_DRV_INFO("(0x050)DISP_REG_PQ_CONT_CP                      = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_CONT_CP));
            DDP_DRV_INFO("(0x054)DISP_REG_PQ_CONT_SLOPE                   = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_CONT_SLOPE));
            DDP_DRV_INFO("(0x058)DISP_REG_PQ_CONT_OFFSET                  = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_CONT_OFFSET));
            DDP_DRV_INFO("(0x060)DISP_REG_PQ_SAT_CON_00                   = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_SAT_CON_00));
            DDP_DRV_INFO("(0x064)DISP_REG_PQ_SAT_CON_01                   = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_SAT_CON_01));
            DDP_DRV_INFO("(0x068)DISP_REG_PQ_SAT_GAIN                     = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_SAT_GAIN));
            DDP_DRV_INFO("(0x06C)DISP_REG_PQ_SAT_SLOPE                    = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_SAT_SLOPE));
            DDP_DRV_INFO("(0x070)DISP_REG_PQ_HIST_X_CFG                   = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_HIST_X_CFG));
            DDP_DRV_INFO("(0x074)DISP_REG_PQ_HIST_Y_CFG                   = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_HIST_Y_CFG));
            DDP_DRV_INFO("(0x078)DISP_REG_PQ_LUMA_HIST_00                 = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_LUMA_HIST_00));
            DDP_DRV_INFO("(0x07C)DISP_REG_PQ_LUMA_HIST_01                 = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_LUMA_HIST_01));
            DDP_DRV_INFO("(0x080)DISP_REG_PQ_LUMA_HIST_02                 = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_LUMA_HIST_02));
            DDP_DRV_INFO("(0x084)DISP_REG_PQ_LUMA_HIST_03                 = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_LUMA_HIST_03));
            DDP_DRV_INFO("(0x088)DISP_REG_PQ_LUMA_HIST_04                 = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_LUMA_HIST_04));
            DDP_DRV_INFO("(0x08C)DISP_REG_PQ_LUMA_HIST_05                 = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_LUMA_HIST_05));
            DDP_DRV_INFO("(0x090)DISP_REG_PQ_LUMA_HIST_06                 = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_LUMA_HIST_06));
            DDP_DRV_INFO("(0x094)DISP_REG_PQ_LUMA_HIST_07                 = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_LUMA_HIST_07));
            DDP_DRV_INFO("(0x098)DISP_REG_PQ_LUMA_HIST_08                 = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_LUMA_HIST_08));
            DDP_DRV_INFO("(0x09C)DISP_REG_PQ_LUMA_HIST_09                 = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_LUMA_HIST_09));
            DDP_DRV_INFO("(0x0A0)DISP_REG_PQ_LUMA_HIST_10                 = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_LUMA_HIST_10));
            DDP_DRV_INFO("(0x0A4)DISP_REG_PQ_LUMA_HIST_11                 = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_LUMA_HIST_11));
            DDP_DRV_INFO("(0x0A8)DISP_REG_PQ_LUMA_HIST_12                 = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_LUMA_HIST_12));
            DDP_DRV_INFO("(0x0AC)DISP_REG_PQ_LUMA_HIST_13                 = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_LUMA_HIST_13));
            DDP_DRV_INFO("(0x0B0)DISP_REG_PQ_LUMA_HIST_14                 = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_LUMA_HIST_14));
            DDP_DRV_INFO("(0x0B4)DISP_REG_PQ_LUMA_HIST_15                 = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_LUMA_HIST_15));
            DDP_DRV_INFO("(0x0B8)DISP_REG_PQ_LUMA_HIST_16                 = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_LUMA_HIST_16));
            DDP_DRV_INFO("(0x0C0)DISP_REG_PQ_LUMA_SUM                     = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_LUMA_SUM));
            DDP_DRV_INFO("(0x0C4)DISP_REG_PQ_LUMA_MIN_MAX                 = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_LUMA_MIN_MAX));
            DDP_DRV_INFO("(0x0D0)DISP_REG_PQ_Y_FTN_1_0                    = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_Y_FTN_1_0));
            DDP_DRV_INFO("(0x0D4)DISP_REG_PQ_Y_FTN_3_2                    = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_Y_FTN_3_2));
            DDP_DRV_INFO("(0x0D8)DISP_REG_PQ_Y_FTN_5_4                    = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_Y_FTN_5_4));
            DDP_DRV_INFO("(0x0DC)DISP_REG_PQ_Y_FTN_7_6                    = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_Y_FTN_7_6));
            DDP_DRV_INFO("(0x0E0)DISP_REG_PQ_Y_FTN_9_8                    = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_Y_FTN_9_8));
            DDP_DRV_INFO("(0x0E4)DISP_REG_PQ_Y_FTN_11_10                  = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_Y_FTN_11_10));
            DDP_DRV_INFO("(0x0E8)DISP_REG_PQ_Y_FTN_13_12                  = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_Y_FTN_13_12));
            DDP_DRV_INFO("(0x0EC)DISP_REG_PQ_Y_FTN_15_14                  = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_Y_FTN_15_14));
            DDP_DRV_INFO("(0x0F0)DISP_REG_PQ_Y_FTN_16                     = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_Y_FTN_16));
            DDP_DRV_INFO("(0x100)DISP_REG_PQ_C_BOOST_CON                  = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_C_BOOST_CON));
            DDP_DRV_INFO("(0x110)DISP_REG_PQ_C_HIST_X_CFG                 = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_C_HIST_X_CFG));
            DDP_DRV_INFO("(0x114)DISP_REG_PQ_C_HIST_Y_CFG                 = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_C_HIST_Y_CFG));
            DDP_DRV_INFO("(0x118)DISP_REG_PQ_C_HIST_CON                   = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_C_HIST_CON));
            DDP_DRV_INFO("(0x11C)DISP_REG_PQ_C_HIST_BIN                   = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_C_HIST_BIN));
            DDP_DRV_INFO("(0x200)DISP_REG_PQ_DEMO_MAIN                    = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_DEMO_MAIN));
            DDP_DRV_INFO("(0x208)DISP_REG_PQ_INK_LUMA                     = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_INK_LUMA));
            DDP_DRV_INFO("(0x20C)DISP_REG_PQ_INK_CHROMA                   = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_INK_CHROMA));
            DDP_DRV_INFO("(0x210)DISP_REG_PQ_CAP_POS                      = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_CAP_POS));
            DDP_DRV_INFO("(0x218)DISP_REG_PQ_CAP_IN_Y                     = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_CAP_IN_Y));
            DDP_DRV_INFO("(0x21C)DISP_REG_PQ_CAP_IN_C                     = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_CAP_IN_C));
            DDP_DRV_INFO("(0x220)DISP_REG_PQ_CAP_OUT_Y                    = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_CAP_OUT_Y));
            DDP_DRV_INFO("(0x224)DISP_REG_PQ_CAP_OUT_C                    = 0x%08X\n", DISP_REG_GET(DISP_REG_PQ_CAP_OUT_C));
            break;

        case DISP_MODULE_BLS :
            DDP_DRV_INFO("===== DISP PWM Reg Dump: ============\n");
            DDP_DRV_INFO("(0x000)DISP_REG_PWM_EN                          = 0x%08X\n", DISP_REG_GET(DISP_REG_PWM_EN));
            DDP_DRV_INFO("(0x004)DISP_REG_PWM_RST                         = 0x%08X\n", DISP_REG_GET(DISP_REG_PWM_RST));
            DDP_DRV_INFO("(0x010)DISP_REG_PWM_CON_0                       = 0x%08X\n", DISP_REG_GET(DISP_REG_PWM_CON_0));
            DDP_DRV_INFO("(0x014)DISP_REG_PWM_CON_1                       = 0x%08X\n", DISP_REG_GET(DISP_REG_PWM_CON_1));
            DDP_DRV_INFO("(0x018)DISP_REG_PWM_GRADUAL                     = 0x%08X\n", DISP_REG_GET(DISP_REG_PWM_GRADUAL));
            DDP_DRV_INFO("(0x01C)DISP_REG_PWM_GRADUAL_RO                  = 0x%08X\n", DISP_REG_GET(DISP_REG_PWM_GRADUAL_RO));
            DDP_DRV_INFO("(0x020)DISP_REG_PWM_DEBUG                       = 0x%08X\n", DISP_REG_GET(DISP_REG_PWM_DEBUG));
            DDP_DRV_INFO("(0x030)DISP_REG_PWM_DUMMY                       = 0x%08X\n", DISP_REG_GET(DISP_REG_PWM_DUMMY));

            DDP_DRV_INFO("===== DISP BLS Reg Dump: ============\n");
            DDP_DRV_INFO("(0x000)DISP_REG_BLS_EN                          = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_EN));
            DDP_DRV_INFO("(0x004)DISP_REG_BLS_RST                         = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_RST));
            DDP_DRV_INFO("(0x008)DISP_REG_BLS_INTEN                       = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_INTEN));
            DDP_DRV_INFO("(0x00C)DISP_REG_BLS_INTSTA                      = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_INTSTA));
            DDP_DRV_INFO("(0x010)DISP_REG_BLS_BLS_SETTING                 = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_BLS_SETTING));
            DDP_DRV_INFO("(0x014)DISP_REG_BLS_FANA_SETTING                = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_FANA_SETTING));
            DDP_DRV_INFO("(0x018)DISP_REG_BLS_SRC_SIZE                    = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_SRC_SIZE));
            DDP_DRV_INFO("(0x020)DISP_REG_BLS_GAIN_SETTING                = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_GAIN_SETTING));
            DDP_DRV_INFO("(0x024)DISP_REG_BLS_MANUAL_GAIN                 = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_MANUAL_GAIN));
            DDP_DRV_INFO("(0x028)DISP_REG_BLS_MANUAL_MAXCLR               = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_MANUAL_MAXCLR));
            DDP_DRV_INFO("(0x030)DISP_REG_BLS_GAMMA_SETTING               = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_GAMMA_SETTING));
            DDP_DRV_INFO("(0x038)DISP_REG_BLS_LUT_UPDATE                  = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_LUT_UPDATE));
            DDP_DRV_INFO("(0x060)DISP_REG_BLS_MAXCLR_THD                  = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_MAXCLR_THD));
            DDP_DRV_INFO("(0x064)DISP_REG_BLS_DISTPT_THD                  = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_DISTPT_THD));
            DDP_DRV_INFO("(0x068)DISP_REG_BLS_MAXCLR_LIMIT                = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_MAXCLR_LIMIT));
            DDP_DRV_INFO("(0x06C)DISP_REG_BLS_DISTPT_LIMIT                = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_DISTPT_LIMIT));
            DDP_DRV_INFO("(0x070)DISP_REG_BLS_AVE_SETTING                 = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_AVE_SETTING));
            DDP_DRV_INFO("(0x074)DISP_REG_BLS_AVE_LIMIT                   = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_AVE_LIMIT));
            DDP_DRV_INFO("(0x078)DISP_REG_BLS_DISTPT_SETTING              = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_DISTPT_SETTING));
            DDP_DRV_INFO("(0x07C)DISP_REG_BLS_HIS_CLEAR                   = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_HIS_CLEAR));
            DDP_DRV_INFO("(0x080)DISP_REG_BLS_SC_DIFF_THD                 = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_SC_DIFF_THD));
            DDP_DRV_INFO("(0x084)DISP_REG_BLS_SC_BIN_THD                  = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_SC_BIN_THD));
            DDP_DRV_INFO("(0x088)DISP_REG_BLS_MAXCLR_GRADUAL              = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_MAXCLR_GRADUAL));
            DDP_DRV_INFO("(0x08C)DISP_REG_BLS_DISTPT_GRADUAL              = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_DISTPT_GRADUAL));
            DDP_DRV_INFO("(0x090)DISP_REG_BLS_FAST_IIR_XCOEFF             = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_FAST_IIR_XCOEFF));
            DDP_DRV_INFO("(0x094)DISP_REG_BLS_FAST_IIR_YCOEFF             = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_FAST_IIR_YCOEFF));
            DDP_DRV_INFO("(0x098)DISP_REG_BLS_SLOW_IIR_XCOEFF             = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_SLOW_IIR_XCOEFF));
            DDP_DRV_INFO("(0x09C)DISP_REG_BLS_SLOW_IIR_YCOEFF             = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_SLOW_IIR_YCOEFF));
            DDP_DRV_INFO("(0x0A0)DISP_REG_BLS_PWM_DUTY                    = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_PWM_DUTY));
            DDP_DRV_INFO("(0x0B0)DISP_REG_BLS_DEBUG                       = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_DEBUG));
            DDP_DRV_INFO("(0x0B4)DISP_REG_BLS_PATTERN                     = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_PATTERN));
            DDP_DRV_INFO("(0x0B8)DISP_REG_BLS_CHKSUM                      = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_CHKSUM));
            DDP_DRV_INFO("(0x0C0)DISP_REG_BLS_SAFE_MEASURE                = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_SAFE_MEASURE));
            DDP_DRV_INFO("(0x100)DISP_REG_BLS_HIS_BIN                     = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_HIS_BIN_));
            DDP_DRV_INFO("(0x200)DISP_REG_BLS_PWM_DUTY_RD                 = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_PWM_DUTY_RD));
            DDP_DRV_INFO("(0x204)DISP_REG_BLS_FRAME_AVE_RD                = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_FRAME_AVE_RD));
            DDP_DRV_INFO("(0x208)DISP_REG_BLS_MAXCLR_RD                   = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_MAXCLR_RD));
            DDP_DRV_INFO("(0x20C)DISP_REG_BLS_DISTPT_RD                   = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_DISTPT_RD));
            DDP_DRV_INFO("(0x210)DISP_REG_BLS_GAIN_RD                     = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_GAIN_RD));
            DDP_DRV_INFO("(0x214)DISP_REG_BLS_SC_RD                       = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_SC_RD));
            DDP_DRV_INFO("(0x300)DISP_REG_BLS_LUMINANCE                   = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_LUMINANCE_));
            DDP_DRV_INFO("(0x384)DISP_REG_BLS_LUMINANCE_255               = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_LUMINANCE_255));
            DDP_DRV_INFO("(0x400)DISP_REG_BLS_GAMMA_LUT                   = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_GAMMA_LUT_));
            DDP_DRV_INFO("(0xE00)DISP_REG_BLS_DITHER_0                    = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_DITHER_0));
            DDP_DRV_INFO("(0xE14)DISP_REG_BLS_DITHER_5                    = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_DITHER_5));
            DDP_DRV_INFO("(0xE18)DISP_REG_BLS_DITHER_6                    = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_DITHER_6));
            DDP_DRV_INFO("(0xE1C)DISP_REG_BLS_DITHER_7                    = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_DITHER_7));
            DDP_DRV_INFO("(0xE20)DISP_REG_BLS_DITHER_8                    = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_DITHER_8));
            DDP_DRV_INFO("(0xE24)DISP_REG_BLS_DITHER_9                    = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_DITHER_9));
            DDP_DRV_INFO("(0xE28)DISP_REG_BLS_DITHER_10                   = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_DITHER_10));
            DDP_DRV_INFO("(0xE2C)DISP_REG_BLS_DITHER_11                   = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_DITHER_11));
            DDP_DRV_INFO("(0xE30)DISP_REG_BLS_DITHER_12                   = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_DITHER_12));
            DDP_DRV_INFO("(0xE34)DISP_REG_BLS_DITHER_13                   = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_DITHER_13));
            DDP_DRV_INFO("(0xE38)DISP_REG_BLS_DITHER_14                   = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_DITHER_14));
            DDP_DRV_INFO("(0xE3C)DISP_REG_BLS_DITHER_15                   = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_DITHER_15));
            DDP_DRV_INFO("(0xE40)DISP_REG_BLS_DITHER_16                   = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_DITHER_16));
            DDP_DRV_INFO("(0xE44)DISP_REG_BLS_DITHER_17                   = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_DITHER_17));
            DDP_DRV_INFO("(0xF00)DISP_REG_BLS_DUMMY                       = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_DUMMY));
            break;

        case DISP_MODULE_WDMA0 :
            DDP_DRV_INFO("===== DISP WDMA Reg Dump: ============\n");
            DDP_DRV_INFO("(0x000)DISP_REG_WDMA_INTEN                      = 0x%08X\n", DISP_REG_GET(DISP_REG_WDMA_INTEN));
            DDP_DRV_INFO("(0x004)DISP_REG_WDMA_INTSTA                     = 0x%08X\n", DISP_REG_GET(DISP_REG_WDMA_INTSTA));
            DDP_DRV_INFO("(0x008)DISP_REG_WDMA_EN                         = 0x%08X\n", DISP_REG_GET(DISP_REG_WDMA_EN));
            DDP_DRV_INFO("(0x00C)DISP_REG_WDMA_RST                        = 0x%08X\n", DISP_REG_GET(DISP_REG_WDMA_RST));
            DDP_DRV_INFO("(0x010)DISP_REG_WDMA_SMI_CON                    = 0x%08X\n", DISP_REG_GET(DISP_REG_WDMA_SMI_CON));
            DDP_DRV_INFO("(0x014)DISP_REG_WDMA_CFG                        = 0x%08X\n", DISP_REG_GET(DISP_REG_WDMA_CFG));
            DDP_DRV_INFO("(0x018)DISP_REG_WDMA_SRC_SIZE                   = 0x%08X\n", DISP_REG_GET(DISP_REG_WDMA_SRC_SIZE));
            DDP_DRV_INFO("(0x01C)DISP_REG_WDMA_CLIP_SIZE                  = 0x%08X\n", DISP_REG_GET(DISP_REG_WDMA_CLIP_SIZE));
            DDP_DRV_INFO("(0x020)DISP_REG_WDMA_CLIP_COORD                 = 0x%08X\n", DISP_REG_GET(DISP_REG_WDMA_CLIP_COORD));
            DDP_DRV_INFO("(0x024)DISP_REG_WDMA_DST_ADDR0                  = 0x%08X\n", DISP_REG_GET(DISP_REG_WDMA_DST_ADDR0));
            DDP_DRV_INFO("(0x028)DISP_REG_WDMA_DST_W_IN_BYTE              = 0x%08X\n", DISP_REG_GET(DISP_REG_WDMA_DST_W_IN_BYTE));
            DDP_DRV_INFO("(0x02C)DISP_REG_WDMA_ALPHA                      = 0x%08X\n", DISP_REG_GET(DISP_REG_WDMA_ALPHA));
            DDP_DRV_INFO("(0x038)DISP_REG_WDMA_BUF_CON1                   = 0x%08X\n", DISP_REG_GET(DISP_REG_WDMA_BUF_CON1));
            DDP_DRV_INFO("(0x03C)DISP_REG_WDMA_BUF_CON2                   = 0x%08X\n", DISP_REG_GET(DISP_REG_WDMA_BUF_CON2));
            DDP_DRV_INFO("(0x040)DISP_REG_WDMA_C00                        = 0x%08X\n", DISP_REG_GET(DISP_REG_WDMA_C00));
            DDP_DRV_INFO("(0x044)DISP_REG_WDMA_C02                        = 0x%08X\n", DISP_REG_GET(DISP_REG_WDMA_C02));
            DDP_DRV_INFO("(0x048)DISP_REG_WDMA_C10                        = 0x%08X\n", DISP_REG_GET(DISP_REG_WDMA_C10));
            DDP_DRV_INFO("(0x04C)DISP_REG_WDMA_C12                        = 0x%08X\n", DISP_REG_GET(DISP_REG_WDMA_C12));
            DDP_DRV_INFO("(0x050)DISP_REG_WDMA_C20                        = 0x%08X\n", DISP_REG_GET(DISP_REG_WDMA_C20));
            DDP_DRV_INFO("(0x054)DISP_REG_WDMA_C22                        = 0x%08X\n", DISP_REG_GET(DISP_REG_WDMA_C22));
            DDP_DRV_INFO("(0x058)DISP_REG_WDMA_PRE_ADD0                   = 0x%08X\n", DISP_REG_GET(DISP_REG_WDMA_PRE_ADD0));
            DDP_DRV_INFO("(0x05C)DISP_REG_WDMA_PRE_ADD2                   = 0x%08X\n", DISP_REG_GET(DISP_REG_WDMA_PRE_ADD2));
            DDP_DRV_INFO("(0x060)DISP_REG_WDMA_POST_ADD0                  = 0x%08X\n", DISP_REG_GET(DISP_REG_WDMA_POST_ADD0));
            DDP_DRV_INFO("(0x064)DISP_REG_WDMA_POST_ADD2                  = 0x%08X\n", DISP_REG_GET(DISP_REG_WDMA_POST_ADD2));
            DDP_DRV_INFO("(0x070)DISP_REG_WDMA_DST_ADDR1                  = 0x%08X\n", DISP_REG_GET(DISP_REG_WDMA_DST_ADDR1));
            DDP_DRV_INFO("(0x074)DISP_REG_WDMA_DST_ADDR2                  = 0x%08X\n", DISP_REG_GET(DISP_REG_WDMA_DST_ADDR2));
            DDP_DRV_INFO("(0x078)DISP_REG_WDMA_DST_UV_PITCH               = 0x%08X\n", DISP_REG_GET(DISP_REG_WDMA_DST_UV_PITCH));
            DDP_DRV_INFO("(0x080)DISP_REG_WDMA_DST_ADDR_OFFSET0           = 0x%08X\n", DISP_REG_GET(DISP_REG_WDMA_DST_ADDR_OFFSET0));
            DDP_DRV_INFO("(0x084)DISP_REG_WDMA_DST_ADDR_OFFSET1           = 0x%08X\n", DISP_REG_GET(DISP_REG_WDMA_DST_ADDR_OFFSET1));
            DDP_DRV_INFO("(0x088)DISP_REG_WDMA_DST_ADDR_OFFSET2           = 0x%08X\n", DISP_REG_GET(DISP_REG_WDMA_DST_ADDR_OFFSET2));
            DDP_DRV_INFO("(0x0A0)DISP_REG_WDMA_FLOW_CTRL_DBG              = 0x%08X\n", DISP_REG_GET(DISP_REG_WDMA_FLOW_CTRL_DBG));
            DDP_DRV_INFO("(0x0A4)DISP_REG_WDMA_EXEC_DBG                   = 0x%08X\n", DISP_REG_GET(DISP_REG_WDMA_EXEC_DBG));
            DDP_DRV_INFO("(0x0A8)DISP_REG_WDMA_CT_DBG                     = 0x%08X\n", DISP_REG_GET(DISP_REG_WDMA_CT_DBG));
            DDP_DRV_INFO("(0x0AC)DISP_REG_WDMA_DEBUG                      = 0x%08X\n", DISP_REG_GET(DISP_REG_WDMA_DEBUG));
            DDP_DRV_INFO("(0x100)DISP_REG_WDMA_DUMMY                      = 0x%08X\n", DISP_REG_GET(DISP_REG_WDMA_DUMMY));
            DDP_DRV_INFO("(0xE00)DISP_REG_WDMA_DITHER_0                   = 0x%08X\n", DISP_REG_GET(DISP_REG_WDMA_DITHER_0));
            DDP_DRV_INFO("(0xE14)DISP_REG_WDMA_DITHER_5                   = 0x%08X\n", DISP_REG_GET(DISP_REG_WDMA_DITHER_5));
            DDP_DRV_INFO("(0xE18)DISP_REG_WDMA_DITHER_6                   = 0x%08X\n", DISP_REG_GET(DISP_REG_WDMA_DITHER_6));
            DDP_DRV_INFO("(0xE1C)DISP_REG_WDMA_DITHER_7                   = 0x%08X\n", DISP_REG_GET(DISP_REG_WDMA_DITHER_7));
            DDP_DRV_INFO("(0xE20)DISP_REG_WDMA_DITHER_8                   = 0x%08X\n", DISP_REG_GET(DISP_REG_WDMA_DITHER_8));
            DDP_DRV_INFO("(0xE24)DISP_REG_WDMA_DITHER_9                   = 0x%08X\n", DISP_REG_GET(DISP_REG_WDMA_DITHER_9));
            DDP_DRV_INFO("(0xE28)DISP_REG_WDMA_DITHER_10                  = 0x%08X\n", DISP_REG_GET(DISP_REG_WDMA_DITHER_10));
            DDP_DRV_INFO("(0xE2C)DISP_REG_WDMA_DITHER_11                  = 0x%08X\n", DISP_REG_GET(DISP_REG_WDMA_DITHER_11));
            DDP_DRV_INFO("(0xE30)DISP_REG_WDMA_DITHER_12                  = 0x%08X\n", DISP_REG_GET(DISP_REG_WDMA_DITHER_12));
            DDP_DRV_INFO("(0xE34)DISP_REG_WDMA_DITHER_13                  = 0x%08X\n", DISP_REG_GET(DISP_REG_WDMA_DITHER_13));
            DDP_DRV_INFO("(0xE38)DISP_REG_WDMA_DITHER_14                  = 0x%08X\n", DISP_REG_GET(DISP_REG_WDMA_DITHER_14));
            DDP_DRV_INFO("(0xE3C)DISP_REG_WDMA_DITHER_15                  = 0x%08X\n", DISP_REG_GET(DISP_REG_WDMA_DITHER_15));
            DDP_DRV_INFO("(0xE40)DISP_REG_WDMA_DITHER_16                  = 0x%08X\n", DISP_REG_GET(DISP_REG_WDMA_DITHER_16));
            DDP_DRV_INFO("(0xE44)DISP_REG_WDMA_DITHER_17                  = 0x%08X\n", DISP_REG_GET(DISP_REG_WDMA_DITHER_17));
            break;

        case DISP_MODULE_RDMA0 :
            DDP_DRV_INFO("===== DISP RDMA Reg Dump: ======== \n");
            DDP_DRV_INFO("(0x000)DISP_REG_RDMA_INT_ENABLE                 = 0x%08X\n", DISP_REG_GET(DISP_REG_RDMA_INT_ENABLE));
            DDP_DRV_INFO("(0x004)DISP_REG_RDMA_INT_STATUS                 = 0x%08X\n", DISP_REG_GET(DISP_REG_RDMA_INT_STATUS));
            DDP_DRV_INFO("(0x010)DISP_REG_RDMA_GLOBAL_CON                 = 0x%08X\n", DISP_REG_GET(DISP_REG_RDMA_GLOBAL_CON));
            DDP_DRV_INFO("(0x014)DISP_REG_RDMA_SIZE_CON_0                 = 0x%08X\n", DISP_REG_GET(DISP_REG_RDMA_SIZE_CON_0));
            DDP_DRV_INFO("(0x018)DISP_REG_RDMA_SIZE_CON_1                 = 0x%08X\n", DISP_REG_GET(DISP_REG_RDMA_SIZE_CON_1));
            DDP_DRV_INFO("(0x01C)DISP_REG_RDMA_TARGET_LINE                = 0x%08X\n", DISP_REG_GET(DISP_REG_RDMA_TARGET_LINE));
            DDP_DRV_INFO("(0x024)DISP_REG_RDMA_MEM_CON                    = 0x%08X\n", DISP_REG_GET(DISP_REG_RDMA_MEM_CON));
            DDP_DRV_INFO("(0x028)DISP_REG_RDMA_MEM_START_ADDR             = 0x%08X\n", DISP_REG_GET(DISP_REG_RDMA_MEM_START_ADDR));
            DDP_DRV_INFO("(0x02C)DISP_REG_RDMA_MEM_SRC_PITCH              = 0x%08X\n", DISP_REG_GET(DISP_REG_RDMA_MEM_SRC_PITCH));
            DDP_DRV_INFO("(0x030)DISP_REG_RDMA_MEM_GMC_SETTING_0          = 0x%08X\n", DISP_REG_GET(DISP_REG_RDMA_MEM_GMC_SETTING_0));
            DDP_DRV_INFO("(0x034)DISP_REG_RDMA_MEM_SLOW_CON               = 0x%08X\n", DISP_REG_GET(DISP_REG_RDMA_MEM_SLOW_CON));
            DDP_DRV_INFO("(0x038)DISP_REG_RDMA_MEM_GMC_SETTING_1          = 0x%08X\n", DISP_REG_GET(DISP_REG_RDMA_MEM_GMC_SETTING_1));
            DDP_DRV_INFO("(0x040)DISP_REG_RDMA_FIFO_CON                   = 0x%08X\n", DISP_REG_GET(DISP_REG_RDMA_FIFO_CON));
            DDP_DRV_INFO("(0x044)DISP_REG_RDMA_FIFO_LOG                   = 0x%08X\n", DISP_REG_GET(DISP_REG_RDMA_FIFO_LOG));
            DDP_DRV_INFO("(0x054)DISP_REG_RDMA_C00                        = 0x%08X\n", DISP_REG_GET(DISP_REG_RDMA_C00));
            DDP_DRV_INFO("(0x058)DISP_REG_RDMA_C01                        = 0x%08X\n", DISP_REG_GET(DISP_REG_RDMA_C01));
            DDP_DRV_INFO("(0x05C)DISP_REG_RDMA_C02                        = 0x%08X\n", DISP_REG_GET(DISP_REG_RDMA_C02));
            DDP_DRV_INFO("(0x060)DISP_REG_RDMA_C10                        = 0x%08X\n", DISP_REG_GET(DISP_REG_RDMA_C10));
            DDP_DRV_INFO("(0x064)DISP_REG_RDMA_C11                        = 0x%08X\n", DISP_REG_GET(DISP_REG_RDMA_C11));
            DDP_DRV_INFO("(0x068)DISP_REG_RDMA_C12                        = 0x%08X\n", DISP_REG_GET(DISP_REG_RDMA_C12));
            DDP_DRV_INFO("(0x06C)DISP_REG_RDMA_C20                        = 0x%08X\n", DISP_REG_GET(DISP_REG_RDMA_C20));
            DDP_DRV_INFO("(0x070)DISP_REG_RDMA_C21                        = 0x%08X\n", DISP_REG_GET(DISP_REG_RDMA_C21));
            DDP_DRV_INFO("(0x074)DISP_REG_RDMA_C22                        = 0x%08X\n", DISP_REG_GET(DISP_REG_RDMA_C22));
            DDP_DRV_INFO("(0x078)DISP_REG_RDMA_PRE_ADD_0                  = 0x%08X\n", DISP_REG_GET(DISP_REG_RDMA_PRE_ADD_0));
            DDP_DRV_INFO("(0x07C)DISP_REG_RDMA_PRE_ADD_1                  = 0x%08X\n", DISP_REG_GET(DISP_REG_RDMA_PRE_ADD_1));
            DDP_DRV_INFO("(0x080)DISP_REG_RDMA_PRE_ADD_2                  = 0x%08X\n", DISP_REG_GET(DISP_REG_RDMA_PRE_ADD_2));
            DDP_DRV_INFO("(0x084)DISP_REG_RDMA_POST_ADD_0                 = 0x%08X\n", DISP_REG_GET(DISP_REG_RDMA_POST_ADD_0));
            DDP_DRV_INFO("(0x088)DISP_REG_RDMA_POST_ADD_1                 = 0x%08X\n", DISP_REG_GET(DISP_REG_RDMA_POST_ADD_1));
            DDP_DRV_INFO("(0x08C)DISP_REG_RDMA_POST_ADD_2                 = 0x%08X\n", DISP_REG_GET(DISP_REG_RDMA_POST_ADD_2));
            DDP_DRV_INFO("(0x090)DISP_REG_RDMA_DUMMY                      = 0x%08X\n", DISP_REG_GET(DISP_REG_RDMA_DUMMY));
            DDP_DRV_INFO("(0x094)DISP_REG_RDMA_DEBUG_OUT_SEL              = 0x%08X\n", DISP_REG_GET(DISP_REG_RDMA_DEBUG_OUT_SEL));
            break;

        case DISP_MODULE_MUTEX :
            DDP_DRV_INFO("===== DISP DISP_REG_MUTEX_CONFIG Reg Dump: ============\n");
            DDP_DRV_INFO("(0x000)DISP_REG_CONFIG_MUTEX_INTEN              = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX_INTEN));
            DDP_DRV_INFO("(0x004)DISP_REG_CONFIG_MUTEX_INTSTA             = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX_INTSTA));
            DDP_DRV_INFO("(0x008)DISP_REG_CONFIG_REG_UPD_TIMEOUT          = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_REG_UPD_TIMEOUT));
            DDP_DRV_INFO("(0x00C)DISP_REG_CONFIG_REG_COMMIT               = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_REG_COMMIT));
            DDP_DRV_INFO("(0x020)DISP_REG_CONFIG_MUTEX0_EN                = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX0_EN));
            DDP_DRV_INFO("(0x024)DISP_REG_CONFIG_MUTEX0                   = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX0));
            DDP_DRV_INFO("(0x028)DISP_REG_CONFIG_MUTEX0_RST               = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX0_RST));
            DDP_DRV_INFO("(0x02C)DISP_REG_CONFIG_MUTEX0_MOD               = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX0_MOD));
            DDP_DRV_INFO("(0x030)DISP_REG_CONFIG_MUTEX0_SOF               = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX0_SOF));
            DDP_DRV_INFO("(0x040)DISP_REG_CONFIG_MUTEX1_EN                = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX1_EN));
            DDP_DRV_INFO("(0x044)DISP_REG_CONFIG_MUTEX1                   = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX1));
            DDP_DRV_INFO("(0x048)DISP_REG_CONFIG_MUTEX1_RST               = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX1_RST));
            DDP_DRV_INFO("(0x04C)DISP_REG_CONFIG_MUTEX1_MOD               = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX1_MOD));
            DDP_DRV_INFO("(0x050)DISP_REG_CONFIG_MUTEX1_SOF               = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX1_SOF));
            DDP_DRV_INFO("(0x060)DISP_REG_CONFIG_MUTEX2_EN                = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX2_EN));
            DDP_DRV_INFO("(0x064)DISP_REG_CONFIG_MUTEX2                   = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX2));
            DDP_DRV_INFO("(0x068)DISP_REG_CONFIG_MUTEX2_RST               = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX2_RST));
            DDP_DRV_INFO("(0x06C)DISP_REG_CONFIG_MUTEX2_MOD               = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX2_MOD));
            DDP_DRV_INFO("(0x070)DISP_REG_CONFIG_MUTEX2_SOF               = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX2_SOF));
            DDP_DRV_INFO("(0x080)DISP_REG_CONFIG_MUTEX3_EN                = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX3_EN));
            DDP_DRV_INFO("(0x084)DISP_REG_CONFIG_MUTEX3                   = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX3));
            DDP_DRV_INFO("(0x088)DISP_REG_CONFIG_MUTEX3_RST               = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX3_RST));
            DDP_DRV_INFO("(0x08C)DISP_REG_CONFIG_MUTEX3_MOD               = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX3_MOD));
            DDP_DRV_INFO("(0x090)DISP_REG_CONFIG_MUTEX3_SOF               = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX3_SOF));
            DDP_DRV_INFO("(0x0A0)DISP_REG_CONFIG_MUTEX4_EN                = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX4_EN));
            DDP_DRV_INFO("(0x0A4)DISP_REG_CONFIG_MUTEX4                   = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX4));
            DDP_DRV_INFO("(0x0A8)DISP_REG_CONFIG_MUTEX4_RST               = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX4_RST));
            DDP_DRV_INFO("(0x0AC)DISP_REG_CONFIG_MUTEX4_MOD               = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX4_MOD));
            DDP_DRV_INFO("(0x0B0)DISP_REG_CONFIG_MUTEX4_SOF               = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX4_SOF));
            DDP_DRV_INFO("(0x0C0)DISP_REG_CONFIG_MUTEX5_EN                = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX5_EN));
            DDP_DRV_INFO("(0x0C4)DISP_REG_CONFIG_MUTEX5                   = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX5));
            DDP_DRV_INFO("(0x0C8)DISP_REG_CONFIG_MUTEX5_RST               = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX5_RST));
            DDP_DRV_INFO("(0x0CC)DISP_REG_CONFIG_MUTEX5_MOD               = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX5_MOD));
            DDP_DRV_INFO("(0x0D0)DISP_REG_CONFIG_MUTEX5_SOF               = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX5_SOF));
            DDP_DRV_INFO("(0x0E0)DISP_REG_CONFIG_MUTEX6_EN                = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX6_EN));
            DDP_DRV_INFO("(0x0E4)DISP_REG_CONFIG_MUTEX6                   = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX6));
            DDP_DRV_INFO("(0x0E8)DISP_REG_CONFIG_MUTEX6_RST               = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX6_RST));
            DDP_DRV_INFO("(0x0EC)DISP_REG_CONFIG_MUTEX6_MOD               = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX6_MOD));
            DDP_DRV_INFO("(0x0F0)DISP_REG_CONFIG_MUTEX6_SOF               = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX6_SOF));
            DDP_DRV_INFO("(0x100)DISP_REG_CONFIG_MUTEX7_EN                = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX7_EN));
            DDP_DRV_INFO("(0x104)DISP_REG_CONFIG_MUTEX7                   = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX7));
            DDP_DRV_INFO("(0x108)DISP_REG_CONFIG_MUTEX7_RST               = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX7_RST));
            DDP_DRV_INFO("(0x10C)DISP_REG_CONFIG_MUTEX7_MOD               = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX7_MOD));
            DDP_DRV_INFO("(0x110)DISP_REG_CONFIG_MUTEX7_SOF               = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX7_SOF));
            DDP_DRV_INFO("(0x200)DISP_REG_CONFIG_MUTEX_DEBUG_OUT_SEL      = 0x%08X\n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX_DEBUG_OUT_SEL));
            break;

        default :
            DDP_DRV_INFO("disp_dump_reg() invalid module id=%d \n", module);
    }

    return 0;
}

void disp_m4u_dump_reg(void)
{
    // dump display info
    disp_dump_reg(DISP_MODULE_OVL);
    disp_dump_reg(DISP_MODULE_WDMA0);
    disp_dump_reg(DISP_MODULE_MUTEX);
    disp_dump_reg(DISP_MODULE_CONFIG);

    // dump mdp info
    dumpMDPRegInfo();
}

int disp_module_clock_on(DISP_MODULE_ENUM module, char* caller_name)
{
    return 0;
}

int disp_module_clock_off(DISP_MODULE_ENUM module, char* caller_name)
{
    return 0;
}


module_init(disp_init);
module_exit(disp_exit);
MODULE_AUTHOR("Tzu-Meng, Chung <Tzu-Meng.Chung@mediatek.com>");
MODULE_DESCRIPTION("Display subsystem Driver");
MODULE_LICENSE("GPL");
