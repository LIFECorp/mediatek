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
#include <linux/proc_fs.h>
#include <linux/xlog.h>
#include <linux/aee.h>

#include <asm/io.h>

#include <mach/irqs.h>
#include <mach/mt_reg_base.h>
#include <mach/mt_irq.h>
#include <mach/irqs.h>
#include <mach/mt_clkmgr.h>
#include <mach/mt_irq.h>
#include <mach/sync_write.h>
#include <mach/mt_smi.h>

#include "ddp_cmdq.h"
#include "ddp_drv.h"
#include "ddp_reg.h"
#include "ddp_path.h"
#include "ddp_pq.h"

extern unsigned long * cmdq_pBuffer;
extern wait_queue_head_t cmq_wait_queue[CMDQ_THREAD_NUM];
//extern unsigned char cmq_status[CMDQ_THREAD_NUM];
extern unsigned int dbg_log;

cmdq_buff_t cmdqBufTbl[CMDQ_BUFFER_NUM];

int taskIDStatusTbl[MAX_CMDQ_TASK_ID];

task_resource_t taskResOccuTbl[MAX_CMDQ_TASK_ID]; //0~255

unsigned int hwResTbl = 0; //0~20 bit 0-> usable HW, 1-> in use
unsigned int cmdqThreadResTbl[CMDQ_THREAD_NUM];

unsigned int cmdqThreadTaskList_R[CMDQ_THREAD_NUM]; //Read pointer for each thread
unsigned int cmdqThreadTaskList_W[CMDQ_THREAD_NUM]; //Write pointer for each thread
int cmdqThreadTaskList[CMDQ_THREAD_NUM][CMDQ_THREAD_LIST_LENGTH]; //each Thread's Current Job

//Add for ISR loss problem
int cmdqThreadTaskNumber[CMDQ_THREAD_NUM]; //Record Job Sequence
int cmdqThreadFirstJobID[CMDQ_THREAD_NUM]; //Record First Task ID
long cmdqThreadEofCnt[MAX_CMDQ_TASK_ID]; //Record / Predict CMDQ EOF cnt

// mdp error counter
int totalMdpResetCnt = 0;
int mdpMutexFailCnt = 0;
int mdpRdmaFailCnt = 0;
int mdpWrotFailCnt = 0;

// mdp RDMA register backup
int mdpRdmaPrevStatus[6];
int cmdqCurrInst;

// cmdq timeout counter
int totalTimeoutCnt = 0;
int totalExecCnt = 0;
int waitQueueTimeoutCnt = 0;
int cmdqIsrLossCnt = 0;
int mdpBusyLongCnt = 0;

// cmdq execution time monitor
struct timeval prevFrameExecTime;


spinlock_t gCmdqMgrLock;

#define CMDQ_WRN(string, args...) if(dbg_log) printk("[CMDQ]"string,##args)
//#define CMDQ_MSG(string, args...) printk(string,##args)
#define CMDQ_ERR(string, args...) if(1) printk("[CMDQ State]"string,##args)
#define CMDQ_MSG(string, args...) if(dbg_log) printk(string,##args)
//#define CMDQ_ERR(string, args...) if(dbg_log) printk(string,##args)
#define CMDQ_IRQ(string, args...) if(dbg_log) printk("[CMDQ]"string,##args)

#define CMDQ_MDP_AEE(string, args...) do{\
    xlog_printk(ANDROID_LOG_ERROR,  "MDP", "error: "string, ##args); \
    aee_kernel_warning("MDP", "error: "string, ##args);  \
}while(0)

#define CMDQ_ISP_AEE(string, args...) do{\
    xlog_printk(ANDROID_LOG_ERROR,  "ISP", "error: "string, ##args); \
    aee_kernel_warning("ISP", "error: "string, ##args);  \
}while(0)

#define CMDQ_MAX_LOOP_COUNT 0x100000

typedef struct {
    int moduleType[cbMAX];
    CMDQ_TIMEOUT_PTR cmdqTimeout_cb[cbMAX];
    CMDQ_RESET_PTR cmdqReset_cb[cbMAX];
} CMDQ_CONFIG_CB_ARRAY;


CMDQ_CONFIG_CB_ARRAY g_CMDQ_CB_Array = { {cbMDP, cbISP}, {NULL, NULL}, {NULL, NULL}};//if cbMAX == 2


void cmdqForceFreeAll(int cmdqThread);
void cmdqForceFree_SW(int taskID);

#if 1   // move to proc
// Hardware Mutex Variables
#define ENGINE_MUTEX_NUM 8
static DEFINE_SPINLOCK(gMutexLock);
int mutex_used[ENGINE_MUTEX_NUM] = {1, 0, 1, 1, 0, 0, 0, 0};    // 0 for FB, 1 for Bitblt, 2 for HDMI, 3 for BLS
static DECLARE_WAIT_QUEUE_HEAD(gMutexWaitQueue);

extern DISPLAY_SHP_T *get_SHP_index(void);

typedef struct
{
    pid_t open_pid;
    pid_t open_tgid;
    unsigned int u4LockedMutex;
    spinlock_t node_lock;
} cmdq_proc_node_struct;

int disp_lock_mutex(void)
{
    int id = -1;
    int i;
    spin_lock(&gMutexLock);

    for(i = 0 ; i < ENGINE_MUTEX_NUM ; i++)
        if(mutex_used[i] == 0)
        {
            id = i;
            mutex_used[i] = 1;
            //DISP_REG_SET_FIELD((1 << i) , DISP_REG_CONFIG_MUTEX_INTEN , 1);
            break;
        }
    spin_unlock(&gMutexLock);

    return id;
}

int disp_unlock_mutex(int id)
{
    if(id < 0 && id >= ENGINE_MUTEX_NUM) 
        return -1;

    spin_lock(&gMutexLock);
    
    mutex_used[id] = 0;
    //DISP_REG_SET_FIELD((1 << id) , DISP_REG_CONFIG_MUTEX_INTEN , 0);
    
    spin_unlock(&gMutexLock);

    return 0;
}

static long cmdq_proc_unlocked_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    DISP_EXEC_COMMAND cParams = {0};
    int mutex_id = 0;
    DISP_PQ_PARAM * pq_param = NULL;
    DISPLAY_SHP_T * shp_index = NULL;
    unsigned long flags = 0;
    int taskID = 0;
    cmdq_buff_t * pCmdqAddr = NULL;
    cmdq_proc_node_struct *pNode = (cmdq_proc_node_struct*)file->private_data;
    struct timeval tv;
    
    switch(cmd)
    {
        case DISP_IOCTL_RESOURCE_REQUIRE:

            CMDQ_MSG("\n=========DISP_IOCTL_RESOURCE_REQUIRE start!==========\n");

            spin_lock_irqsave(&gCmdqMgrLock,flags);
            taskID = cmdqResource_required();
            spin_unlock_irqrestore(&gCmdqMgrLock,flags);
            
            if (copy_to_user((void*)arg, &taskID , sizeof(int)))
            {
                CMDQ_ERR("DISP_IOCTL_RESOURCE_REQUIRE, copy_to_user failed\n");
                return -EFAULT;            
            }
  
            CMDQ_MSG("\n=========DISP_IOCTL_RESOURCE_REQUIRE taskID:%d==========\n",taskID);
            
            break;
            
        case DISP_IOCTL_EXEC_COMMAND:

            //CS++++++++++++++++++++++++++++++++++++++++++++++++++
    
           //spin_lock_irqsave(&gCmdqMgrLock,flags); 
            
            do_gettimeofday(&tv);

            if(copy_from_user(&cParams, (void *)arg, sizeof(DISP_EXEC_COMMAND)))
            {
                CMDQ_ERR("disp driver : Copy from user error\n");
           //     spin_unlock_irqrestore(&gCmdqMgrLock,flags);              
                return -EFAULT;
            }
            CMDQ_MSG(KERN_DEBUG "==========DISP_IOCTL_EXEC_COMMAND Task: %d start at SEC: %d MS: %d===========\n",cParams.taskID, (int)tv.tv_sec, (int)tv.tv_usec);

            //Get Related buffer VA/MVA

            pCmdqAddr = cmdqBufAddr(cParams.taskID);
            if(NULL == pCmdqAddr)
            {
                CMDQ_ERR("CmdQ buf address is NULL in DISP_IOCTL_EXEC_COMMAND ioctl\n");
                cmdqForceFree_SW(cParams.taskID);
                return -EFAULT;
            }

            CMDQ_MSG("CMDQ task %x buffer VA: 0x%x MVA: 0x%x \n", pCmdqAddr->Owner, (unsigned int)pCmdqAddr->VA, (unsigned int)pCmdqAddr->MVA);

            CMDQ_MSG(KERN_DEBUG "==========DISP_IOCTL_EXEC_COMMAND Params: FrameBase = 0x%08x, size = %x ===========\n",(unsigned int)cParams.pFrameBaseSW, cParams.blockSize);

            if (copy_from_user((unsigned long*)(pCmdqAddr->VA) ,cParams.pFrameBaseSW, cParams.blockSize))
            {
                CMDQ_ERR("disp driver : Copy from user error\n");
                return -EFAULT;
            }

            //spin_unlock_irqrestore(&gCmdqMgrLock,flags);

            //CS--------------------------------------------------
            
/*
            //ION Flush
*/
           // cmdq_ion_flush(); //FIXME!

            if (false == cmdqTaskAssigned(cParams.taskID, cParams.priority, cParams.engineFlag, cParams.blockSize))
            {
                do_gettimeofday(&tv);
                CMDQ_ERR(KERN_DEBUG "==========DISP_IOCTL_EXECUTE_COMMANDS Task: %d fail at SEC: %d MS: %d===========\n",cParams.taskID, (int)tv.tv_sec, (int)tv.tv_usec);
                return -EFAULT;
            }

            do_gettimeofday(&tv);
            CMDQ_MSG(KERN_DEBUG "==========DISP_IOCTL_EXEC_COMMAND Task: %d done at SEC: %d MS: %d===========\n",cParams.taskID, (int)tv.tv_sec, (int)tv.tv_usec);
            
            
            break;

            
        case DISP_IOCTL_LOCK_MUTEX:
        {
            wait_event_interruptible_timeout(
            gMutexWaitQueue, 
            (mutex_id = disp_lock_mutex()) != -1, 
            msecs_to_jiffies(200) );             

            if((-1) != mutex_id)
            {
                spin_lock(&pNode->node_lock);
                pNode->u4LockedMutex |= (1 << mutex_id);
                spin_unlock(&pNode->node_lock);
            }
            
            if(copy_to_user((void *)arg, &mutex_id, sizeof(int)))
            {
                CMDQ_ERR("disp driver : Copy to user error (mutex)\n");
                return -EFAULT;            
            }
            break;
        }
        case DISP_IOCTL_UNLOCK_MUTEX:
            if(copy_from_user(&mutex_id, (void*)arg , sizeof(int)))
            {
                CMDQ_ERR("DISP_IOCTL_UNLOCK_MUTEX, copy_from_user failed\n");
                return -EFAULT;
            }
            disp_unlock_mutex(mutex_id);

            if((-1) != mutex_id)
            {
                spin_lock(&pNode->node_lock);
                pNode->u4LockedMutex &= ~(1 << mutex_id);
                spin_unlock(&pNode->node_lock);
            }

            wake_up_interruptible(&gMutexWaitQueue);             

            break;
               
        case DISP_IOCTL_GET_SHPINDEX:
            // this is duplicated to disp_unlocked_ioctl
            // be careful when modify the definition
            shp_index = get_SHP_index();
            if(copy_to_user((void *)arg, shp_index, sizeof(DISPLAY_SHP_T)))
            {
                CMDQ_ERR("disp driver : DISP_IOCTL_GET_TDSHPINDEX Copy to user failed\n");
                return -EFAULT;            
            }  
            break;
            
        case DISP_IOCTL_GET_PQPARAM:
            // this is duplicated to cmdq_proc_unlocked_ioctl
            // be careful when modify the definition
            pq_param = get_PQ_config();
            if(copy_to_user((void *)arg, pq_param, sizeof(DISP_PQ_PARAM)))
            {
                CMDQ_ERR("disp driver : DISP_IOCTL_GET_PQPARAM Copy to user failed\n");
                return -EFAULT;            
            }

            break;
    }

    return 0;
}

static int cmdq_proc_open(struct inode *inode, struct file *file)
{
    cmdq_proc_node_struct *pNode = NULL;

    CMDQ_MSG("enter cmdq_proc_open() process:%s\n", current->comm);

    //Allocate and initialize private data
    file->private_data = kmalloc(sizeof(cmdq_proc_node_struct), GFP_ATOMIC);
    if(NULL == file->private_data)
    {
        CMDQ_MSG("Not enough entry for DDP open operation\n");
        return -ENOMEM;
    }
   
    pNode = (cmdq_proc_node_struct *)file->private_data;
    pNode->open_pid = current->pid;
    pNode->open_tgid = current->tgid;
    pNode->u4LockedMutex = 0;
    spin_lock_init(&pNode->node_lock);
    return 0;
}

static int cmdq_proc_release(struct inode *inode, struct file *file)
{
    cmdq_proc_node_struct *pNode = NULL;
    unsigned int index = 0;
    CMDQ_MSG("enter cmdq_proc_release() process:%s\n",current->comm);
    
    pNode = (cmdq_proc_node_struct *)file->private_data;

    spin_lock(&pNode->node_lock);

    if(pNode->u4LockedMutex)
    {
        CMDQ_ERR("Proccess terminated[Mutex] ! :%s , mutex:%u\n" 
            , current->comm , pNode->u4LockedMutex);

        for(index = 0 ; index < ENGINE_MUTEX_NUM ; index += 1)
        {
            if((1 << index) & pNode->u4LockedMutex)
            {
                disp_unlock_mutex(index);
                CMDQ_MSG("unlock index = %d ,mutex_used[ %d %d %d %d ]\n",index,mutex_used[0],mutex_used[1] ,mutex_used[2],mutex_used[3]);
            }
        }
        
    } 

    spin_unlock(&pNode->node_lock);

    if(NULL != file->private_data)
    {
        kfree(file->private_data);
        file->private_data = NULL;
    }
    
    return 0;
}

static ssize_t cmdq_proc_read(struct file *file, char __user *data, size_t len, loff_t *ppos)
{
    return 0;
}

static int cmdq_proc_flush(struct file * file , fl_owner_t a_id)
{
    return 0;
}

static struct file_operations cmdq_proc_fops = {
    .owner      = THIS_MODULE,
    .open		= cmdq_proc_open,
    .read       = cmdq_proc_read,
    .flush      = cmdq_proc_flush,
	.release	= cmdq_proc_release,
    .unlocked_ioctl = cmdq_proc_unlocked_ioctl,
};

#endif


//extern void smi_dumpDebugMsg(void);

int MDPTimeOutDump(int params)
{
    printk("\n\n\n MDP cmdqTimeout_cb Test %d\n\n\n", params);
    return 0;
}


int MDPResetProcess(int params)
{
    printk("\n\n\n MDP cmdqReset_cb Test %d\n\n\n", params);
    return 0;
}


void dumpDebugInfo(void)
{
    int i = 0;

    CMDQ_ERR("\n\n\ncmdqTaskAssigned R: %d W: %d \n\n\n", cmdqThreadTaskList_R[0], cmdqThreadTaskList_W[0]);

    for(i=0;i<CMDQ_BUFFER_NUM;i++)
    {
        CMDQ_ERR("cmdqBufTbl %x : [%x %lx %lx] \n", i, cmdqBufTbl[i].Owner, cmdqBufTbl[i].VA, cmdqBufTbl[i].MVA);
    }
}


bool checkMdpEngineStatus(unsigned int engineFlag)
{
    if (engineFlag & (0x1 << tIMGI))
    {
        #if 0 // TODO: correct isp SW CG configuration
        if(clock_is_on(MT_CG_MM_CAM_SW_CG))
        {
            //value = DISP_REG_GET(0xF5004160);
            DISP_REG_SET(0xF5004160, 0x06000);

            if (0x11 != (DISP_REG_GET(0xF5004164) & 0x11))
            {
                //pr_emerg("ISP engine is busy 0x%8x, 0x%8x\n", DISP_REG_GET(0xF5004160), DISP_REG_GET(0xF5004164));
                return true;
            }
        }
        #endif
    }

    if (engineFlag & (0x1 << tRDMA0))
    {
        if (clock_is_on(MT_CG_MDP_RDMA_SW_CG))
        {
            if (0x100 != (DISP_REG_GET(0xF4001408) & 0x7FF00))
            {
                //pr_emerg("RDMA engine is busy: %d\n", DISP_REG_GET(0xF4001408));
                return true;
            }
        }
    }

    if (engineFlag & (0x1 << tWROT))
    {
        if (clock_is_on(MT_CG_MDP_WROT_SW_CG))
        {
            DISP_REG_SET(0xF4004018, 0xB00);
            if (0x0 != (DISP_REG_GET(0xF40040D0) & 0x1F))
            {
                //pr_emerg("WROT engine is busy %d\n", DISP_REG_GET(0xF40040D0));
                return true;
            }
        }
    }

    return false;
}


void resetMdpEngine(unsigned int engineFlag)
{
    int loop_count;
//    int reg_val;
#if 1
    if (engineFlag & (0x01 << tIMGI))
    {
        CMDQ_MSG("Reset ISP Pass2 start\n");
#if 0
        if(clock_is_on(MT_CG_MM_CAM_SW_CG))
        {
            // Disable MDP Crop
            reg_val = DISP_REG_GET(0xF5004110);
            DISP_REG_SET(0xF5004110, (reg_val & ~0x08000));

            // Clear UV resampler
            DISP_REG_SET(0xF500408C, 0x00800000);

            DISP_REG_SET(0xF5004160, 0x06000);

            loop_count = 0;
            while(loop_count <= 50000)
            {
                if (0x11 == (DISP_REG_GET(0xF5004164) & 0x11))
                    break;

                loop_count++;
            }

            if (loop_count > 50000)
            {
                printk(KERN_DEBUG "Reset ISP failed\n");
            }

            // Set UV resampler
            DISP_REG_SET(0xF500408C, 0x00000000);
        }
#endif
        if(g_CMDQ_CB_Array.cmdqReset_cb[cbISP]!=NULL)
            g_CMDQ_CB_Array.cmdqReset_cb[cbISP](0);

        CMDQ_MSG("Reset ISP Pass2 end\n");
    }
#endif // 0

    if (engineFlag & (0x1 << tRDMA0))
    {
        if(clock_is_on(MT_CG_MDP_RDMA_SW_CG))
        {
            DISP_REG_SET(0xF4001008, 0x1);

            loop_count = 0;
            while(loop_count <= 50000)
            {
                if (0x100 == (DISP_REG_GET(0xF4001408) & 0x7FF00))
                    break;

                loop_count++;
            }

            if (loop_count > 50000)
            {
                printk(KERN_DEBUG "Reset RDMA failed\n");
            }

            DISP_REG_SET(0xF4001008, 0x0);
        }
    }

    if (engineFlag & (0x1 << tSCL0))
    {
        if(clock_is_on(MT_CG_MDP_RSZ_SW_CG))
        {
            DISP_REG_SET(0xF4002000, 0x0);
            DISP_REG_SET(0xF4002000, 0x10000);
            DISP_REG_SET(0xF4002000, 0x0);
        }
    }

    if (engineFlag & (0x1 << tMDPSHP))
    {
        if(clock_is_on(MT_CG_MDP_SHP_SW_CG))
        {
            DISP_REG_SET(0xF4003000, 0x0);
            DISP_REG_SET(0xF4003000, 0x2);
            DISP_REG_SET(0xF4003000, 0x0);
        }
    }

    if (engineFlag & (0x1 << tWROT))
    {
        if(clock_is_on(MT_CG_MDP_WROT_SW_CG))
        {
            DISP_REG_SET(0xF4004010, 0x1);

            loop_count = 0;
            while(loop_count <= 50000)
            {
                if (0x0 == (DISP_REG_GET(0xF4004014) & 0x1))
                    break;

                loop_count++;
            }

            if (loop_count > 50000)
            {
                printk(KERN_DEBUG "Reset ROT failed\n");
            }

            DISP_REG_SET(0xF4004010, 0x0);
        }
    }

    totalMdpResetCnt++;
}


void dumpMDPRegInfo(void)
{
    int reg_temp1, reg_temp2, reg_temp3;

    printk(KERN_DEBUG "[CMDQ]RDMA_SRC_CON: 0x%08x, RDMA_SRC_BASE_0: 0x%08x, RDMA_MF_BKGD_SIZE_IN_BYTE: 0x%08x\n",
           DISP_REG_GET(0xF4001030),
           DISP_REG_GET(0xF4001040),
           DISP_REG_GET(0xF4001060));
    printk(KERN_DEBUG "[CMDQ]RDMA_MF_SRC_SIZE: 0x%08x, RDMA_MF_CLIP_SIZE: 0x%08x, RDMA_MF_OFFSET_1: 0x%08x\n",
           DISP_REG_GET(0xF4001070),
           DISP_REG_GET(0xF4001078),
           DISP_REG_GET(0xF4001080));
    printk(KERN_DEBUG "[CMDQ]RDMA_SRC_END_0: 0x%08x, RDMA_SRC_OFFSET_0: 0x%08x, RDMA_SRC_OFFSET_W_0: 0x%08x\n",
           DISP_REG_GET(0xF4001100),
           DISP_REG_GET(0xF4001118),
           DISP_REG_GET(0xF4001130));
    printk(KERN_DEBUG "[CMDQ]RDMA_MON_STA_0: 0x%08x, RDMA_MON_STA_1: 0x%08x, RDMA_MON_STA_2: 0x%08x\n",
           DISP_REG_GET(0xF4001400),
           DISP_REG_GET(0xF4001408),
           DISP_REG_GET(0xF4001410));
    printk(KERN_DEBUG "[CMDQ]RDMA_MON_STA_4: 0x%08x, RDMA_MON_STA_6: 0x%08x, RDMA_MON_STA_26: 0x%08x\n",
           DISP_REG_GET(0xF4001420),
           DISP_REG_GET(0xF4001430),
           DISP_REG_GET(0xF40014D0));

    printk(KERN_DEBUG "[CMDQ]WDMA_CFG: 0x%08x, WDMA_SRC_SIZE: 0x%08x, WDMA_DST_W_IN_BYTE = 0x%08x\n",
           DISP_REG_GET(0xF4004014),
           DISP_REG_GET(0xF4004018),
           DISP_REG_GET(0xF4004028));
    printk(KERN_DEBUG "[CMDQ]WDMA_DST_ADDR0: 0x%08x, WDMA_DST_UV_PITCH: 0x%08x, WDMA_DST_ADDR_OFFSET0 = 0x%08x\n",
           DISP_REG_GET(0xF4004024),
           DISP_REG_GET(0xF4004078),
           DISP_REG_GET(0xF4004080));
    printk(KERN_DEBUG "[CMDQ]WDMA_STATUS: 0x%08x, WDMA_INPUT_CNT: 0x%08x\n",
           DISP_REG_GET(0xF40040A0),
           DISP_REG_GET(0xF40040A8));

    printk(KERN_DEBUG "[CMDQ]VIDO_CTRL: 0x%08x, VIDO_MAIN_BUF_SIZE: 0x%08x, VIDO_SUB_BUF_SIZE: 0x%08x\n",
           DISP_REG_GET(0xF4004000),
           DISP_REG_GET(0xF4004008),
           DISP_REG_GET(0xF400400C));

    printk(KERN_DEBUG "[CMDQ]VIDO_TAR_SIZE: 0x%08x, VIDO_BASE_ADDR: 0x%08x, VIDO_OFST_ADDR: 0x%08x\n",
           DISP_REG_GET(0xF4004024),
           DISP_REG_GET(0xF4004028),
           DISP_REG_GET(0xF400402C));

    printk(KERN_DEBUG "[CMDQ]VIDO_DMA_PERF: 0x%08x, VIDO_STRIDE: 0x%08x, VIDO_IN_SIZE: 0x%08x\n",
           DISP_REG_GET(0xF4004004),
           DISP_REG_GET(0xF4004030),
           DISP_REG_GET(0xF4004078));

    DISP_REG_SET(0xF4004018, 0x00000100);
    reg_temp1 = DISP_REG_GET(0xF40040D0);
    DISP_REG_SET(0xF4004018, 0x00000200);
    reg_temp2 = DISP_REG_GET(0xF40040D0);
    DISP_REG_SET(0xF4004018, 0x00000300);
    reg_temp3 = DISP_REG_GET(0xF40040D0);
    printk(KERN_DEBUG "[CMDQ]VIDO_DBG1: 0x%08x, VIDO_DBG2: 0x%08x, VIDO_DBG3: 0x%08x\n", reg_temp1, reg_temp2, reg_temp3);

    DISP_REG_SET(0xF4004018, 0x00000500);
    reg_temp1 = DISP_REG_GET(0xF40040D0);
    DISP_REG_SET(0xF4004018, 0x00000800);
    reg_temp2 = DISP_REG_GET(0xF40040D0);
    DISP_REG_SET(0xF4004018, 0x00000B00);
    reg_temp3 = DISP_REG_GET(0xF40040D0);
    printk(KERN_DEBUG "[CMDQ]VIDO_DBG5: 0x%08x, VIDO_DBG8: 0x%08x, VIDO_DBGB: 0x%08x\n", reg_temp1, reg_temp2, reg_temp3);
}


void cmdq_dump_mutex(void)
{
    int loop_count;

    // dump mutex status
    printk(KERN_DEBUG "=============== [CMDQ] Mutex Status ===============\n");
    printk(KERN_DEBUG "[CMDQ]DISP_MUTEX_INTSTA = 0x%08x, DISP_REG_COMMIT = 0x%08x\n",
           DISP_REG_GET(0xF400E004),
           DISP_REG_GET(0xF400E00C));

    if (0 != DISP_REG_GET(0xF400E00C))
    {
        mdpMutexFailCnt++;

        for (loop_count = 0; loop_count < 8; loop_count++)
        {
            printk(KERN_DEBUG "[CMDQ] Mutex reset  = 0x%08x\n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX_RST(loop_count)));
            printk(KERN_DEBUG "[CMDQ] Mutex module = 0x%08x\n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX_MOD(loop_count)));
        }
    }
}

void cmdq_dump_mmsys(void)
{
    printk(KERN_DEBUG "=============== [CMDQ] MMSys ===============\n");
    // dump MMSYS clock setting
    printk(KERN_DEBUG "MMSYS_CG_CON0 = 0x%08x\n", DISP_REG_GET(0xF4000100));
    printk(KERN_DEBUG "CAM_MDP_MOUT_EN: 0x%08x, MDP_RDMA_MOUT_EN: 0x%08x, MDP_RSZ0_MOUT_EN: 0x%08x\n",
        DISP_REG_GET(0xF4000000 + 0x01c),
        DISP_REG_GET(0xF4000000 + 0x020),
        DISP_REG_GET(0xF4000000 + 0x024));
    printk(KERN_DEBUG "MDP_SHP_MOUT_EN: 0x%08x, DISP_OVL_MOUT_EN: 0x%08x\n",
        DISP_REG_GET(0xF4000000 + 0x02c),
        DISP_REG_GET(0xF4000000 + 0x030));
    printk(KERN_DEBUG "MDP_RSZ0_SEL: 0x%08x, MDP_SHP_SEL: 0x%08x\n",
        DISP_REG_GET(0xF4000000 + 0x038),
        DISP_REG_GET(0xF4000000 + 0x040));
    printk(KERN_DEBUG "MDP_WROT_SEL: 0x%08x, DISP_OUT_SEL: 0x%08x\n",
        DISP_REG_GET(0xF4000000 + 0x044),
        DISP_REG_GET(0xF4000000 + 0x04c));
}


void cmdq_dump_rdma(void)
{
    printk(KERN_DEBUG "=============== [CMDQ] RDMA ===============\n");
    printk(KERN_DEBUG "[CMDQ]RDMA_SRC_CON: 0x%08x, RDMA_SRC_BASE_0: 0x%08x, RDMA_MF_BKGD_SIZE_IN_BYTE: 0x%08x\n",
           DISP_REG_GET(0xF4001030),
           DISP_REG_GET(0xF4001040),
           DISP_REG_GET(0xF4001060));
    printk(KERN_DEBUG "[CMDQ]RDMA_MF_SRC_SIZE: 0x%08x, RDMA_MF_CLIP_SIZE: 0x%08x, RDMA_MF_OFFSET_1: 0x%08x\n",
           DISP_REG_GET(0xF4001070),
           DISP_REG_GET(0xF4001078),
           DISP_REG_GET(0xF4001080));
    printk(KERN_DEBUG "[CMDQ]RDMA_SRC_END_0: 0x%08x, RDMA_SRC_OFFSET_0: 0x%08x, RDMA_SRC_OFFSET_W_0: 0x%08x\n",
           DISP_REG_GET(0xF4001100),
           DISP_REG_GET(0xF4001118),
           DISP_REG_GET(0xF4001130));
    printk(KERN_DEBUG "[CMDQ](R)RDMA_MON_STA_0: 0x%08x, RDMA_MON_STA_1: 0x%08x, RDMA_MON_STA_2: 0x%08x\n",
           mdpRdmaPrevStatus[0],
           mdpRdmaPrevStatus[1],
           mdpRdmaPrevStatus[2]);
    printk(KERN_DEBUG "[CMDQ](R)RDMA_MON_STA_4: 0x%08x, RDMA_MON_STA_6: 0x%08x, RDMA_MON_STA_26: 0x%08x\n",
           mdpRdmaPrevStatus[3],
           mdpRdmaPrevStatus[4],
           mdpRdmaPrevStatus[5]);
    printk(KERN_DEBUG "[CMDQ]RDMA_MON_STA_0: 0x%08x, RDMA_MON_STA_1: 0x%08x, RDMA_MON_STA_2: 0x%08x\n",
           DISP_REG_GET(0xF4001400),
           DISP_REG_GET(0xF4001408),
           DISP_REG_GET(0xF4001410));
    printk(KERN_DEBUG "[CMDQ]RDMA_MON_STA_4: 0x%08x, RDMA_MON_STA_6: 0x%08x, RDMA_MON_STA_26: 0x%08x\n",
           DISP_REG_GET(0xF4001420),
           DISP_REG_GET(0xF4001430),
           DISP_REG_GET(0xF40014D0));

    if (0x100 != (DISP_REG_GET(0xF4001408) & 0x100))
    {
        mdpRdmaFailCnt++;
    }
}


void cmdq_dump_rsz0(void)
{
    int reg_temp1, reg_temp2, reg_temp3;

    DISP_REG_SET(0xF4002040, 0x00000001);
    reg_temp1 = DISP_REG_GET(0xF4002044);
    DISP_REG_SET(0xF4002040, 0x00000002);
    reg_temp2 = DISP_REG_GET(0xF4002044);
    DISP_REG_SET(0xF4002040, 0x00000003);
    reg_temp3 = DISP_REG_GET(0xF4002044);

    printk(KERN_DEBUG "=============== [CMDQ] RSZ0 Status ====================================\n");
    printk(KERN_DEBUG "[CMDQ] RSZ0_CONTROL: 0x%08x, RSZ0_INPUT_IMAGE: 0x%08x RSZ0_OUTPUT_IMAGE: 0x%08x\n",
        DISP_REG_GET(0xF4002004), DISP_REG_GET(0xF400200C), DISP_REG_GET(0xF4002010));
    printk(KERN_DEBUG "[CMDQ] RSZ0_HORIZONTAL_COEFF_STEP: 0x%08x, RSZ0_VERTICAL_COEFF_STEP: 0x%08x\n",
        DISP_REG_GET(0xF4002014), DISP_REG_GET(0xF4002018));
    printk(KERN_DEBUG "[CMDQ] RSZ0_DEBUG_1: 0x%08x, RSZ0_DEBUG_2: 0x%08x, RSZ0_DEBUG_3: 0x%08x\n",
        reg_temp1, reg_temp2, reg_temp3);
}


void cmdq_dump_shp(void)
{
    printk(KERN_DEBUG "=============== [CMDQ] SHP Status ====================================\n");
    printk(KERN_DEBUG "[CMDQ] SHP_CONTROL: 0x%08x, SHP_INTSTA: 0x%08x SHP_STATUS: 0x%08x\n",
        DISP_REG_GET(0xF4003000), DISP_REG_GET(0xF4003008), DISP_REG_GET(0xF400300C));
    printk(KERN_DEBUG "[CMDQ] SHP_CFG: 0x%08x, SHP_IN_COUNT: 0x%08x SHP_CHKSUM: 0x%08x\n",
        DISP_REG_GET(0xF4003010), DISP_REG_GET(0xF400301C), DISP_REG_GET(0xF4003020));
    printk(KERN_DEBUG "[CMDQ] SHP_OUT_COUNT: 0x%08x, SHP_INPUT_SIZE: 0x%08x SHP_OUTPUT_OFFSET: 0x%08x\n",
        DISP_REG_GET(0xF4003024), DISP_REG_GET(0xF4003028), DISP_REG_GET(0xF4003030));
    printk(KERN_DEBUG "[CMDQ] SHP_OUTPUT_SIZE: 0x%08x\n",
        DISP_REG_GET(0xF4003034));
}


void cmdq_dump_wrot(void)
{
    int reg_temp1, reg_temp2, reg_temp3;

    printk(KERN_DEBUG "=============== [CMDQ] WROT ===============\n");
    printk(KERN_DEBUG "[CMDQ]VIDO_CTRL: 0x%08x, VIDO_MAIN_BUF_SIZE: 0x%08x, VIDO_SUB_BUF_SIZE: 0x%08x\n",
        DISP_REG_GET(0xF4004000),
        DISP_REG_GET(0xF4004008),
        DISP_REG_GET(0xF400400C));
    printk(KERN_DEBUG "[CMDQ]VIDO_TAR_SIZE: 0x%08x, VIDO_BASE_ADDR: 0x%08x, VIDO_OFST_ADDR: 0x%08x\n",
        DISP_REG_GET(0xF4004024),
        DISP_REG_GET(0xF4004028),
        DISP_REG_GET(0xF400402C));
    printk(KERN_DEBUG "[CMDQ]VIDO_DMA_PERF: 0x%08x, VIDO_STRIDE: 0x%08x, VIDO_IN_SIZE: 0x%08x\n",
        DISP_REG_GET(0xF4004004),
        DISP_REG_GET(0xF4004030),
        DISP_REG_GET(0xF4004078));
    printk(KERN_DEBUG "[CMDQ]VIDO_RSV_1: 0x%08x\n",
        DISP_REG_GET(0xF4004070));
    printk(KERN_DEBUG "[CMDQ] ROT_RST: 0x%08x, ROT_RST_STAT: 0x%08x, ROT_INIT: 0x%08x, [CMDQ] ROT_CROP_OFST: 0x%08x\n",
        DISP_REG_GET(0xF4004010),
        DISP_REG_GET(0xF4004014),
        DISP_REG_GET(0xF400401c),
        DISP_REG_GET(0xF4004020));
    printk(KERN_DEBUG "[CMDQ] ROT_BASE_ADDR_C: 0x%08x, ROT_OFST_ADDR_C: 0x%08x, ROT_STRIDE_C: 0x%08x, ROT_DITHER: 0x%08x\n",
        DISP_REG_GET(0xF4004034),
        DISP_REG_GET(0xF4004038),
        DISP_REG_GET(0xF400403c),
        DISP_REG_GET(0xF4004054));
    printk(KERN_DEBUG "[CMDQ] ROT_BASE_ADDR_V: 0x%08x, ROT_OFST_ADDR_V: 0x%08x, ROT_STRIDE_V: 0x%08x",
        DISP_REG_GET(0xF4004064),
        DISP_REG_GET(0xF4004068),
        DISP_REG_GET(0xF400406C));
    printk(KERN_DEBUG "[CMDQ] DMA_PREULTRA: 0x%08x, ROT_ROT_EN: 0x%08x, ROT_FIFO_TEST: 0x%08x\n",
        DISP_REG_GET(0xF4004074),
        DISP_REG_GET(0xF400407C),
        DISP_REG_GET(0xF4004080));
    printk(KERN_DEBUG "[CMDQ] ROT_MAT_CTRL: 0x%08x, ROT_MAT_RMY: 0x%08x, ROT_MAT_RMV: 0x%08x\n",
        DISP_REG_GET(0xF4004084),
        DISP_REG_GET(0xF4004088),
        DISP_REG_GET(0xF400408C));
    printk(KERN_DEBUG "[CMDQ] ROT_GMY: 0x%08x, ROT_BMY: 0x%08x, ROT_BMV: 0x%08x\n",
        DISP_REG_GET(0xF4004090),
        DISP_REG_GET(0xF4004094),
        DISP_REG_GET(0xF4004098));
    printk(KERN_DEBUG "[CMDQ] ROT_MAT_PREADD: 0x%08x, ROT_MAT_POSTADD: 0x%08x, DITHER_00: 0x%08x\n",
        DISP_REG_GET(0xF400409C),
        DISP_REG_GET(0xF40040A0),
        DISP_REG_GET(0xF40040A4));
    printk(KERN_DEBUG "[CMDQ] ROT_DITHER_02: 0x%08x, ROT_DITHER_03: 0x%08x, ROT_DITHER_04: 0x%08x\n",
        DISP_REG_GET(0xF40040AC),
        DISP_REG_GET(0xF40040B0),
        DISP_REG_GET(0xF40040B4));
    printk(KERN_DEBUG "[CMDQ] ROT_DITHER_05: 0x%08x, ROT_DITHER_06: 0x%08x, ROT_DITHER_07: 0x%08x\n",
        DISP_REG_GET(0xF40040B8),
        DISP_REG_GET(0xF40040BC),
        DISP_REG_GET(0xF40040C0));
    printk(KERN_DEBUG "[CMDQ] ROT_DITHER_08: 0x%08x, ROT_DITHER_09: 0x%08x, ROT_DITHER_10: 0x%08x\n",
        DISP_REG_GET(0xF40040C4),
        DISP_REG_GET(0xF40040C8),
        DISP_REG_GET(0xF40040CC));

    DISP_REG_SET(0xF4004018, 0x00000100);
    reg_temp1 = DISP_REG_GET(0xF40040D0);
    DISP_REG_SET(0xF4004018, 0x00000200);
    reg_temp2 = DISP_REG_GET(0xF40040D0);
    DISP_REG_SET(0xF4004018, 0x00000300);
    reg_temp3 = DISP_REG_GET(0xF40040D0);
    printk("[CMDQ]VIDO_DBG1: 0x%08x, VIDO_DBG2: 0x%08x, VIDO_DBG3: 0x%08x\n", reg_temp1, reg_temp2, reg_temp3);
    DISP_REG_SET(0xF4004018, 0x00000400);
    reg_temp1 = DISP_REG_GET(0xF40040D0);
    DISP_REG_SET(0xF4004018, 0x00000500);
    reg_temp2 = DISP_REG_GET(0xF40040D0);
    DISP_REG_SET(0xF4004018, 0x00000600);
    reg_temp3 = DISP_REG_GET(0xF40040D0);
    printk("[CMDQ]VIDO_DBG4: 0x%08x, VIDO_DBG5: 0x%08x, VIDO_DBG6: 0x%08x\n", reg_temp1, reg_temp2, reg_temp3);
    DISP_REG_SET(0xF4004018, 0x00000700);
    reg_temp1 = DISP_REG_GET(0xF40040D0);
    DISP_REG_SET(0xF4004018, 0x00000800);
    reg_temp2 = DISP_REG_GET(0xF40040D0);
    DISP_REG_SET(0xF4004018, 0x00000900);
    reg_temp3 = DISP_REG_GET(0xF40040D0);
    printk("[CMDQ]VIDO_DBG7: 0x%08x, VIDO_DBG8: 0x%08x, VIDO_DBG9: 0x%08x\n", reg_temp1, reg_temp2, reg_temp3);
    DISP_REG_SET(0xF4004018, 0x00000A00);
    reg_temp1 = DISP_REG_GET(0xF40040D0);
    DISP_REG_SET(0xF4004018, 0x00000B00);
    reg_temp2 = DISP_REG_GET(0xF40040D0);
    DISP_REG_SET(0xF4004018, 0x00000C00);
    reg_temp3 = DISP_REG_GET(0xF40040D0);
    printk("[CMDQ]VIDO_DBGA: 0x%08x, VIDO_DBGB: 0x%08x, VIDO_DBGC: 0x%08x\n", reg_temp1, reg_temp2, reg_temp3);
    DISP_REG_SET(0xF4004018, 0x00000D00);
    reg_temp1 = DISP_REG_GET(0xF40040D0);
    DISP_REG_SET(0xF4004018, 0x00000E00);
    reg_temp2 = DISP_REG_GET(0xF40040D0);
    DISP_REG_SET(0xF4004018, 0x00000F00);
    reg_temp3 = DISP_REG_GET(0xF40040D0);
    printk("[CMDQ]VIDO_DBGD: 0x%08x, VIDO_DBGE: 0x%08x, VIDO_DBGF: 0x%08x\n", reg_temp1, reg_temp2, reg_temp3);
    DISP_REG_SET(0xF4004018, 0x00001000);
    reg_temp1 = DISP_REG_GET(0xF40040D0);
    DISP_REG_SET(0xF4004018, 0x00001100);
    reg_temp2 = DISP_REG_GET(0xF40040D0);
    DISP_REG_SET(0xF4004018, 0x00001200);
    reg_temp3 = DISP_REG_GET(0xF40040D0);
    printk("[CMDQ]VIDO_DBG10: 0x%08x, VIDO_DBG11: 0x%08x, VIDO_DBG12: 0x%08x\n", reg_temp1, reg_temp2, reg_temp3);
    DISP_REG_SET(0xF4004018, 0x00001300);
    reg_temp1 = DISP_REG_GET(0xF40040D0);
    DISP_REG_SET(0xF4004018, 0x00001400);
    reg_temp2 = DISP_REG_GET(0xF40040D0);
    printk("[CMDQ]VIDO_DBG13: 0x%08x, VIDO_DBG14: 0x%08x\n", reg_temp1, reg_temp2);

    if (0x0 != (reg_temp3 & 0x1F))
    {
        mdpWrotFailCnt++;
    }

    printk("[CMDQ]mdpWrotFailCnt 0x%08x\n", mdpWrotFailCnt);
}

void cmdq_dump_isp(void)
{
    printk(KERN_DEBUG "=============== [CMDQ] ISP ===============\n");
    printk("[ISP/MDP][TPIPE_DumpReg] 0x10000000  = 0x%08x\n", DISP_REG_GET(0xF0000000));
    printk("[ISP/MDP][TPIPE_DumpReg] 0x10000020  = 0x%08x\n", DISP_REG_GET(0xF0000020));
    printk("[ISP/MDP][TPIPE_DumpReg] 0x10000030  = 0x%08x\n", DISP_REG_GET(0xF0000030));
    printk("[ISP/MDP][TPIPE_DumpReg] 0x10000034  = 0x%08x\n", DISP_REG_GET(0xF0000034));
    #if 0 // TODO: correct isp SW CG configuration
    printk(KERN_DEBUG "CLK_CFG_0  = 0x%08x\n", DISP_REG_GET(CLK_CFG_0));
    printk(KERN_DEBUG "CLK_CFG_3  = 0x%08x\n", DISP_REG_GET(CLK_CFG_3));
    printk(KERN_DEBUG "ISP_CLK_CG = 0x%08x\n", DISP_REG_GET(0xF5000000));

    if(clock_is_on(MT_CG_MM_CAM_SW_CG))
    {
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] start MT6582\n");
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] tpipe_id = 0x00000000\n");
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x15004000 = 0x%08x\n", DISP_REG_GET(0xF5004000));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x15004004 = 0x%08x\n", DISP_REG_GET(0xF5004004));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x15004008 = 0x%08x\n", DISP_REG_GET(0xF5004008));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x1500400C = 0x%08x\n", DISP_REG_GET(0xF500400C));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x15004010 = 0x%08x\n", DISP_REG_GET(0xF5004010));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x15004018 = 0x%08x\n", DISP_REG_GET(0xF5004018));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x1500401C = 0x%08x\n", DISP_REG_GET(0xF500401C));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x15004050 = 0x%08x\n", DISP_REG_GET(0xF5004050));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x15004054 = 0x%08x\n", DISP_REG_GET(0xF5004054));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x15004078 = 0x%08x\n", DISP_REG_GET(0xF5004078));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x15004110 = 0x%08x\n", DISP_REG_GET(0xF5004110));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x1500422C = 0x%08x\n", DISP_REG_GET(0xF500422C));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x15004240 = 0x%08x\n", DISP_REG_GET(0xF5004240));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x1500427C = 0x%08x\n", DISP_REG_GET(0xF500427C));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x150042B4 = 0x%08x\n", DISP_REG_GET(0xF50042B4));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x15004308 = 0x%08x\n", DISP_REG_GET(0xF5004308));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x1500430C = 0x%08x\n", DISP_REG_GET(0xF500430C));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x15004310 = 0x%08x\n", DISP_REG_GET(0xF5004310));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x1500431C = 0x%08x\n", DISP_REG_GET(0xF500431C));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x15004328 = 0x%08x\n", DISP_REG_GET(0xF5004328));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x1500432C = 0x%08x\n", DISP_REG_GET(0xF500432C));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x15004330 = 0x%08x\n", DISP_REG_GET(0xF5004330));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x1500433C = 0x%08x\n", DISP_REG_GET(0xF500433C));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x15004534 = 0x%08x\n", DISP_REG_GET(0xF5004534));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x15004538 = 0x%08x\n", DISP_REG_GET(0xF5004538));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x1500453C = 0x%08x\n", DISP_REG_GET(0xF500453C));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x15004800 = 0x%08x\n", DISP_REG_GET(0xF5004800));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x150048A0 = 0x%08x\n", DISP_REG_GET(0xF50048A0));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x150049C4 = 0x%08x\n", DISP_REG_GET(0xF50049C4));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x150049E4 = 0x%08x\n", DISP_REG_GET(0xF50049E4));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x150049E8 = 0x%08x\n", DISP_REG_GET(0xF50049E8));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x150049EC = 0x%08x\n", DISP_REG_GET(0xF50049EC));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x15004A20 = 0x%08x\n", DISP_REG_GET(0xF5004A20));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x15004ACC = 0x%08x\n", DISP_REG_GET(0xF5004ACC));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x15004B00 = 0x%08x\n", DISP_REG_GET(0xF5004B00));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x15004B04 = 0x%08x\n", DISP_REG_GET(0xF5004B04));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x15004B08 = 0x%08x\n", DISP_REG_GET(0xF5004B08));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x15004B0C = 0x%08x\n", DISP_REG_GET(0xF5004B0C));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x15004B10 = 0x%08x\n", DISP_REG_GET(0xF5004B10));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x15004B14 = 0x%08x\n", DISP_REG_GET(0xF5004B14));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x15004B18 = 0x%08x\n", DISP_REG_GET(0xF5004B18));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x15004B1C = 0x%08x\n", DISP_REG_GET(0xF5004B1C));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x15004B20 = 0x%08x\n", DISP_REG_GET(0xF5004B20));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x15004F50 = 0x%08x\n", DISP_REG_GET(0xF5004F50));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x1400001C = 0x%08x\n", DISP_REG_GET(0xF400001C));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x14000020 = 0x%08x\n", DISP_REG_GET(0xF4000020));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x14000024 = 0x%08x\n", DISP_REG_GET(0xF4000024));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x14000028 = 0x%08x\n", DISP_REG_GET(0xF4000028));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x1400002C = 0x%08x\n", DISP_REG_GET(0xF400002C));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x14000038 = 0x%08x\n", DISP_REG_GET(0xF4000038));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x1400003C = 0x%08x\n", DISP_REG_GET(0xF400003C));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x14000040 = 0x%08x\n", DISP_REG_GET(0xF4000040));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x14000044 = 0x%08x\n", DISP_REG_GET(0xF4000044));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x14000048 = 0x%08x\n", DISP_REG_GET(0xF4000048));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x14001000 = 0x%08x\n", DISP_REG_GET(0xF4001000));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x14001020 = 0x%08x\n", DISP_REG_GET(0xF4001020));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x14001030 = 0x%08x\n", DISP_REG_GET(0xF4001030));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x14001060 = 0x%08x\n", DISP_REG_GET(0xF4001060));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x14001090 = 0x%08x\n", DISP_REG_GET(0xF4001090));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x14002000 = 0x%08x\n", DISP_REG_GET(0xF4002000));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x14002004 = 0x%08x\n", DISP_REG_GET(0xF4002004));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x1400200C = 0x%08x\n", DISP_REG_GET(0xF400200C));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x14002010 = 0x%08x\n", DISP_REG_GET(0xF4002010));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x14002014 = 0x%08x\n", DISP_REG_GET(0xF4002014));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x14002018 = 0x%08x\n", DISP_REG_GET(0xF4002018));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x1400201C = 0x%08x\n", DISP_REG_GET(0xF400201C));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x14002020 = 0x%08x\n", DISP_REG_GET(0xF4002020));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x14002024 = 0x%08x\n", DISP_REG_GET(0xF4002024));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x14002028 = 0x%08x\n", DISP_REG_GET(0xF4002028));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x1400202C = 0x%08x\n", DISP_REG_GET(0xF400202C));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x14002030 = 0x%08x\n", DISP_REG_GET(0xF4002030));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x14002034 = 0x%08x\n", DISP_REG_GET(0xF4002034));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x14002038 = 0x%08x\n", DISP_REG_GET(0xF4002038));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x14003000 = 0x%08x\n", DISP_REG_GET(0xF4003000));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x14003004 = 0x%08x\n", DISP_REG_GET(0xF4003004));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x1400300C = 0x%08x\n", DISP_REG_GET(0xF400300C));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x14003010 = 0x%08x\n", DISP_REG_GET(0xF4003010));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x14003014 = 0x%08x\n", DISP_REG_GET(0xF4003014));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x14003018 = 0x%08x\n", DISP_REG_GET(0xF4003018));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x1400301C = 0x%08x\n", DISP_REG_GET(0xF400301C));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x14003020 = 0x%08x\n", DISP_REG_GET(0xF4003020));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x14003024 = 0x%08x\n", DISP_REG_GET(0xF4003024));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x14003028 = 0x%08x\n", DISP_REG_GET(0xF4003028));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x1400302C = 0x%08x\n", DISP_REG_GET(0xF400302C));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x14003030 = 0x%08x\n", DISP_REG_GET(0xF4003030));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x14003034 = 0x%08x\n", DISP_REG_GET(0xF4003034));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x14003038 = 0x%08x\n", DISP_REG_GET(0xF4003038));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x14004008 = 0x%08x\n", DISP_REG_GET(0xF4004008));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x14004014 = 0x%08x\n", DISP_REG_GET(0xF4004014));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x14004018 = 0x%08x\n", DISP_REG_GET(0xF4004018));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x14004028 = 0x%08x\n", DISP_REG_GET(0xF4004028));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x14004078 = 0x%08x\n", DISP_REG_GET(0xF4004078));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x14005000 = 0x%08x\n", DISP_REG_GET(0xF4005000));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x14005030 = 0x%08x\n", DISP_REG_GET(0xF4005030));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x1400503C = 0x%08x\n", DISP_REG_GET(0xF400503C));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x1400506C = 0x%08x\n", DISP_REG_GET(0xF400506C));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x1400507C = 0x%08x\n", DISP_REG_GET(0xF400507C));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x14005084 = 0x%08x\n", DISP_REG_GET(0xF4005084));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x14006000 = 0x%08x\n", DISP_REG_GET(0xF4006000));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] 0x14006110 = 0x%08x\n", DISP_REG_GET(0xF4006110));
        printk(KERN_DEBUG "[ISP/MDP][TPIPE_DumpReg] end MT6582\n");
    }
    else
    {
        printk(KERN_DEBUG "[CMDQ] Incorrectly, ISP clock is in off state\n");
    }
    #endif
}


void dumpRegDebugInfo(unsigned int engineFlag, int cmdqIndex, cmdq_buff_t bufferAddr)
{
    int reg_temp1, reg_temp2, reg_temp3;
    int index;

    totalTimeoutCnt++;

    // dump current instruction
    reg_temp1 = DISP_REG_GET(DISP_REG_CMDQ_THRx_PC(cmdqIndex));
    reg_temp2 = bufferAddr.VA + (reg_temp1 - bufferAddr.MVA);
    if ((bufferAddr.VA <= reg_temp2) && ((bufferAddr.VA + bufferAddr.blocksize) >= reg_temp2))
    {
        // dump current instruction
        if (reg_temp2 != (bufferAddr.VA + bufferAddr.blocksize))
        {
            cmdqCurrInst = DISP_REG_GET(reg_temp2+4);
            printk("[CMDQ]CMDQ current inst0 = 0x%08x0x%08x, inst1 = 0x%08x0x%08x\n",
                   DISP_REG_GET(reg_temp2-8),
                   DISP_REG_GET(reg_temp2-4),
                   DISP_REG_GET(reg_temp2),
                   cmdqCurrInst);
        }
        else
        {
            cmdqCurrInst = DISP_REG_GET(reg_temp2-4);
            printk("[CMDQ]CMDQ current inst0 = 0x%08x0x%08x, inst1 = 0x%08x0x%08x\n",
                   DISP_REG_GET(reg_temp2-16),
                   DISP_REG_GET(reg_temp2-12),
                   DISP_REG_GET(reg_temp2-8),
                   cmdqCurrInst);
        }
    }

    // dump CMDQ status
    printk("[CMDQ]CMDQ_THR%d_PC = 0x%08x, CMDQ_THR%d_END_ADDR = 0x%08x, CMDQ_THR%d_WAIT_TOKEN = 0x%08x\n",
           cmdqIndex, reg_temp1,
           cmdqIndex, DISP_REG_GET(DISP_REG_CMDQ_THRx_END_ADDR(cmdqIndex)),
           cmdqIndex, DISP_REG_GET(DISP_REG_CMDQ_THRx_WAIT_EVENTS0(cmdqIndex)));

    printk("[CMDQ] dump involved engine 0x%x\n", engineFlag);

    cmdq_dump_mmsys();
    cmdq_dump_mutex();

    if (engineFlag & (0x1 << tIMGI))
    {
        cmdq_dump_isp();
    }

    // dump RDMA debug registers
    if (engineFlag & (0x1 << tRDMA0))
    {
        cmdq_dump_rdma();
    }

    // dump RSZ debug registers
    if (engineFlag & (0x1 << tSCL0))
    {
        cmdq_dump_rsz0();
    }

    // dump SHP debug registers
    if (engineFlag & (0x01 << tMDPSHP))
    {
        cmdq_dump_shp();
    }

    // dump WROT debug registers
    if (engineFlag & (0x1 << tWROT))
    {
        cmdq_dump_wrot();
    }

    //SMI Dump
    //smi_dumpDebugMsg();

    //CMDQ Timeout Callbacks
    for (index = 0 ; index < cbMAX; index++)
    {
        if (g_CMDQ_CB_Array.cmdqTimeout_cb[index]!=NULL)
            g_CMDQ_CB_Array.cmdqTimeout_cb[index](0);
    }
}


void cmdqBufferTbl_init(unsigned long va_base, unsigned long mva_base)
{
    int i = 0;
    unsigned long flags;

    spin_lock_irqsave(&gCmdqMgrLock, flags);

    for(i=0;i<CMDQ_BUFFER_NUM;i++)
    {
        cmdqBufTbl[i].Owner = -1; //free buffer
        cmdqBufTbl[i].VA = va_base + (i*CMDQ_BUFFER_SIZE);
        cmdqBufTbl[i].MVA =  mva_base + (i*CMDQ_BUFFER_SIZE);

        CMDQ_MSG("cmdqBufferTbl_init %x : [%x %lx %lx] \n",i ,cmdqBufTbl[i].Owner, cmdqBufTbl[i].VA, cmdqBufTbl[i].MVA);
    }

    for(i=0;i<MAX_CMDQ_TASK_ID;i++)
    {
        taskIDStatusTbl[i] = -1; //mark as free ID
        taskResOccuTbl[i].cmdBufID = -1;
        taskResOccuTbl[i].cmdqThread= -1;
        cmdqThreadEofCnt[i] = -1;
    }

    for(i=0;i<CMDQ_THREAD_NUM;i++)
    {
        cmdqThreadResTbl[i] = 0;
        cmdqThreadTaskList_R[i] = 0;
        cmdqThreadTaskList_W[i] = 0;
        cmdqThreadTaskNumber[i] = 0;
        cmdqThreadFirstJobID[i] = -1;
    }

    spin_unlock_irqrestore(&gCmdqMgrLock, flags);
}


int cmdqResource_required(void)
{
    int i = 0;
    int assignedTaskID = -1;

    //Find free ID
    for(i=0;i<MAX_CMDQ_TASK_ID;i++)
    {
        if(taskIDStatusTbl[i]==-1)
        {
            assignedTaskID = i;
            taskIDStatusTbl[assignedTaskID] = 1; //mark as occupied
            break;
        }
    }

    if(assignedTaskID == -1)
    {
        CMDQ_ERR("No useable ID !!!\n");
        dumpDebugInfo();
        return -1;
    }

    //Find free Buffer
    for(i=0;i<CMDQ_BUFFER_NUM;i++)
    {
        if(cmdqBufTbl[i].Owner == -1)
        {
            cmdqBufTbl[i].Owner = assignedTaskID;
            taskResOccuTbl[assignedTaskID].cmdBufID = i;

            //printk(KERN_DEBUG "\n=========CMDQ Buffer %x is owned by %x==========\n", taskResOccuTbl[assignedTaskID].cmdBufID, cmdqBufTbl[i].Owner);
            break;
        }
    }

    if(taskResOccuTbl[assignedTaskID].cmdBufID == -1)
    {
        CMDQ_ERR("No Free Buffer !!! Total reset CMDQ Driver\n");
        dumpDebugInfo();
        //taskIDStatusTbl[assignedTaskID] = -1; //return ID, resource allocation fail
        cmdqForceFreeAll(0);

        for(i=0;i<MAX_CMDQ_TASK_ID;i++)
        {
            taskIDStatusTbl[i] = -1; //mark as CANCEL
            taskResOccuTbl[i].cmdBufID = -1;
            taskResOccuTbl[i].cmdqThread= -1;
        }
        return -1;
    }

    return assignedTaskID;
}


void cmdqResource_free(int taskID)
{
    int bufID = -1;

    if(taskID == -1 ||taskID>=MAX_CMDQ_TASK_ID)
    {
        CMDQ_ERR("\n=================Free Invalid Task ID================\n");
        dumpDebugInfo();
        return;
    }

    bufID = taskResOccuTbl[taskID].cmdBufID;

    CMDQ_MSG("=============Free Buf %x own by [%x=%x]===============\n",bufID,taskID,cmdqBufTbl[bufID].Owner);

    if(bufID != -1) //Free All resource and return ID
    {
        taskIDStatusTbl[taskID] = 3; //mark for complete
        taskResOccuTbl[taskID].cmdBufID = -1;
        taskResOccuTbl[taskID].cmdqThread= -1;
        cmdqBufTbl[bufID].Owner = -1;
    }
    else
    {
        CMDQ_ERR("\n=================Free Invalid Buffer ID================\n");
        dumpDebugInfo();
    }
}


cmdq_buff_t * cmdqBufAddr(int taskID)
{
    int bufID = -1;

    if ((-1 == taskID) || (taskID >= MAX_CMDQ_TASK_ID))
    {
        CMDQ_ERR("cmdqBufAddr Invalid ID %d\n", taskID);
        return NULL;
    }

    bufID = taskResOccuTbl[taskID].cmdBufID;

    if ((CMDQ_BUFFER_NUM < bufID) || (bufID < 0))
    {
        return NULL;
    }

    return &cmdqBufTbl[bufID];
}


void cmdqHwClockOn(unsigned int engineFlag, bool firstTask)
{
    if (firstTask)
    {
        #if 0 // TODO: need to check following SW CG configuration
        //if(!clock_is_on(MT_CG_DISP0_SMI_COMMON))
        {
            enable_clock(MT_CG_DISP0_SMI_COMMON, "SMI_COMMON");
        }

        //if(!clock_is_on(MT_CG_DISP0_SMI_LARB0))
        {
            enable_clock(MT_CG_DISP0_SMI_LARB0, "SMI_LARB0");
        }

        //if(!clock_is_on(MT_CG_DISP0_MM_CMDQ))
        {
            enable_clock(MT_CG_DISP0_MM_CMDQ, "MM_CMDQ");
        }

        //if(!clock_is_on(MT_CG_DISP0_MUTEX))
        {
            enable_clock(MT_CG_DISP0_MUTEX, "MUTEX");
        }
        #endif

        //TODO! Should reset All
        DISP_REG_SET(DISP_REG_CMDQ_THRx_RESET(0), 1);
        DISP_REG_SET(DISP_REG_CMDQ_THRx_RESET(1), 1);
    }

    if (engineFlag & (0x1 << tIMGI))
    {
        #if 0 // TODO: correct isp SW CG configuration
        if(!clock_is_on(MT_CG_DISP0_CAM_MDP))
        {
            enable_clock(MT_CG_IMAGE_CAM_SMI, "CAMERA");
            enable_clock(MT_CG_IMAGE_CAM_CAM, "CAMERA");
            enable_clock(MT_CG_IMAGE_SEN_TG,  "CAMERA");
            enable_clock(MT_CG_IMAGE_SEN_CAM, "CAMERA");
            enable_clock(MT_CG_IMAGE_LARB2_SMI, "CAMERA");

            enable_clock(MT_CG_DISP0_CAM_MDP, "CAM_MDP");
        }
        #endif
    }

    if (engineFlag & (0x1 << tRDMA0))
    {
        if(!clock_is_on(MT_CG_MDP_RDMA_SW_CG))
        {
            enable_clock(MT_CG_MDP_RDMA_SW_CG, "MDP_RDMA");
        }
    }

    if (engineFlag & (0x1 << tSCL0))
    {
        if(!clock_is_on(MT_CG_MDP_RSZ_SW_CG))
        {
            enable_clock(MT_CG_MDP_RSZ_SW_CG, "MDP_RSZ");
        }
    }

    if (engineFlag & (0x1 << tMDPSHP))
    {
        if(!clock_is_on(MT_CG_MDP_SHP_SW_CG))
        {
            enable_clock(MT_CG_MDP_SHP_SW_CG, "MDP_SHP");
        }
    }

    if (engineFlag & (0x1 << tWROT))
    {
        if(!clock_is_on(MT_CG_MDP_WROT_SW_CG))
        {
            enable_clock(MT_CG_MDP_WROT_SW_CG, "MDP_WROT");
        }
    }

    CMDQ_MSG("\n\n\n=========== Power On %x ==============\n\n\n",engineFlag);
}


void cmdqHwClockOff(unsigned int engineFlag)
{
    //Finished! Power off clock!
    //M4U
    //larb_clock_off(0, "MDP");
    //MDP
    if(engineFlag & (0x1 << tRDMA0))
    {
        if(clock_is_on(MT_CG_MDP_RDMA_SW_CG))
        {
            disable_clock(MT_CG_MDP_RDMA_SW_CG, "MDP_RDMA");
        }
    }

    if (engineFlag & (0x1 << tSCL0))
    {
        if(clock_is_on(MT_CG_MDP_RSZ_SW_CG))
        {
            disable_clock(MT_CG_MDP_RSZ_SW_CG, "MDP_RSZ");
        }
    }

    if (engineFlag & (0x1 << tMDPSHP))
    {
        if(clock_is_on(MT_CG_MDP_SHP_SW_CG))
        {
            disable_clock(MT_CG_MDP_SHP_SW_CG, "MDP_SHP");
        }
    }

    if (engineFlag & (0x1 << tWROT))
    {
        if(clock_is_on(MT_CG_MDP_WROT_SW_CG))
        {
            disable_clock(MT_CG_MDP_WROT_SW_CG, "MDP_WROT");
        }
    }

    if (engineFlag & (0x1 << tIMGI))
    {
        #if 0 // TODO: correct isp SW CG configuration
        if(clock_is_on(MT_CG_DISP0_CAM_MDP))
        {
            disable_clock(MT_CG_DISP0_CAM_MDP, "CAM_MDP");
            disable_clock(MT_CG_IMAGE_CAM_SMI, "CAMERA");
            disable_clock(MT_CG_IMAGE_CAM_CAM, "CAMERA");
            disable_clock(MT_CG_IMAGE_SEN_TG,  "CAMERA");
            disable_clock(MT_CG_IMAGE_SEN_CAM, "CAMERA");
            disable_clock(MT_CG_IMAGE_LARB2_SMI, "CAMERA");
        }
        #endif
    }

    #if 0 // TODO: need to check following SW CG configuration
    //if(clock_is_on(MT_CG_DISP0_MM_CMDQ))
    {
        disable_clock(MT_CG_DISP0_MM_CMDQ, "MM_CMDQ");
    }

    //if(clock_is_on(MT_CG_DISP0_SMI_LARB0))
    {
        disable_clock(MT_CG_DISP0_SMI_LARB0, "SMI_LARB0");
    }

    //if(clock_is_on(MT_CG_DISP0_SMI_COMMON))
    {
        disable_clock(MT_CG_DISP0_SMI_COMMON, "SMI_COMMON");
    }
    #endif

#if 0
    //disable_clock(MT_CG_INFRA_SMI, "M4U");
    //disable_clock(MT_CG_INFRA_M4U, "M4U");
#endif // 0

    CMDQ_MSG("\n\n\n===========Power Off %x ==============\n\n\n",engineFlag);
}


static int32_t cmdq_reset_hw_engine(int32_t engineFlag)
{
    int32_t  loopCount;
    uint32_t regValue;
    int32_t  err = 0;

    printk(KERN_DEBUG "Reset MDP hardware engine begin\n");

    if (engineFlag & (0x01 << tIMGI))
    {
        printk(KERN_DEBUG "Reset ISP pass2\n");
        if (NULL != g_CMDQ_CB_Array.cmdqReset_cb[cbISP])
        {
            g_CMDQ_CB_Array.cmdqReset_cb[cbISP](0);
        }
    }

    if (engineFlag & (0x1 << tRDMA0))
    {
        if (clock_is_on(MT_CG_MDP_RDMA_SW_CG))
        {
            printk(KERN_DEBUG "Reset RDMA\n");
            DISP_REG_SET(0xF4001008, 0x1);
            loopCount = 0;

            while (loopCount < CMDQ_MAX_LOOP_COUNT)
            {
                if (0x100 == (DISP_REG_GET(0xF4001408) & 0x7FF00))
                {
                    break;
                }
                loopCount++;
            }

            if (loopCount >= CMDQ_MAX_LOOP_COUNT)
            {
                CMDQ_ERR("Reset RDMA engine failed\n");
                err = -EFAULT;
            }

            DISP_REG_SET(0xF4001008, 0x0);
        }
    }

    if (engineFlag & (0x1 << tSCL0))
    {
        printk("Reset RSZ0\n");
        if (clock_is_on(MT_CG_MDP_RSZ_SW_CG))
        {
            DISP_REG_SET(0xF4002000, 0x0);
            DISP_REG_SET(0xF4002000, 0x10000);
            DISP_REG_SET(0xF4002000, 0x0);
        }
    }

    if (engineFlag & (0x1 << tMDPSHP))
    {
        printk("Reset MDPSHP\n");
        if (clock_is_on(MT_CG_MDP_SHP_SW_CG))
        {
            DISP_REG_SET(0xF4003000, 0x0);
            DISP_REG_SET(0xF4003000, 0x2);
            DISP_REG_SET(0xF4003000, 0x0);
        }
    }

    if (engineFlag & (0x1 << tWROT))
    {
        printk(KERN_DEBUG "Reset WROT\n");
        if (clock_is_on(MT_CG_MDP_WROT_SW_CG))
        {
            DISP_REG_SET(0xF4004010, 0x1);
            loopCount = 0;
            while (loopCount < CMDQ_MAX_LOOP_COUNT)
            {
                if (0x0 == (DISP_REG_GET(0xF4004014) & 0x1))
                {
                    break;
                }
                loopCount++;
            }
            DISP_REG_SET(0xF4004010, 0x0);

            if (loopCount >= CMDQ_MAX_LOOP_COUNT)
            {
                CMDQ_ERR("WROT need cold reset\n");
                regValue = DISP_REG_GET(0xF4000138);
                regValue &= 0xFFFFFBFF;
                DISP_REG_SET(0xF4000138, regValue);
                loopCount = 0;
                while (loopCount < CMDQ_MAX_LOOP_COUNT)
                {
                    loopCount++;
                }
                regValue |= 0x00000400;
                DISP_REG_SET(0xF4000138, regValue);

                if (0x0 != (DISP_REG_GET(0xF4004014) & 0x1))
                {
                    CMDQ_ERR("Reset WROT engine failed\n");
                    err = -EFAULT;
                }
            }
        }
    }

    printk("Reset MDP hardware engine end\n");
    return err;
}


bool cmdqTaskAssigned(int taskID, unsigned int priority, unsigned int engineFlag, unsigned int blocksize)
{
    int            i = 0;
    int            cmdqThread = -1;
    cmdq_buff_t    *pCmdqAddr = NULL;
    unsigned long  flags;
    unsigned long  ins_leng = 0;
    unsigned long  *cmdq_pc_head = 0;
    int            buf_id;
    int            pre_task_id;
    int            pre_w_ptr;
    cmdq_buff_t    *pPre_cmdqAddr = NULL;
    bool           ret = true;
    int            cmdq_polling_timeout = 0;
    long           wq_ret = 0;
    struct timeval start_t, end_t;
    int            ring = 0;

    ins_leng = blocksize >> 2; //CMDQ instruction: 4 byte

    totalExecCnt++;
    CMDQ_MSG("CMDQ_INFO: tExec: %d, tTimeout: %d, waitQ: %d, cmdqIsr: %d, mdpBusyL: %d\n",
           totalExecCnt, totalTimeoutCnt, waitQueueTimeoutCnt, cmdqIsrLossCnt, mdpBusyLongCnt);
    CMDQ_MSG("CMDQ_INFO: prevExec: %d us, MdpInfo: mdpMutex: %d, mdpRdma: %d, mdpWrot: %d, mdpReset: %d\n",
           prevFrameExecTime.tv_usec, mdpMutexFailCnt, mdpRdmaFailCnt, mdpWrotFailCnt, totalMdpResetCnt);

    //CS++++++++++++++++++++++++++++++++++++++++++++++++++
    spin_lock_irqsave(&gCmdqMgrLock, flags);

    do_gettimeofday(&start_t);

    CMDQ_MSG("\n\n\n==============cmdqTaskAssigned  %d %d %x %d =============\n\n\n", taskID, priority, engineFlag, blocksize);

    if ((engineFlag & hwResTbl) == 0) //Free HW available
    {
        for (i=0; i<CMDQ_THREAD_NUM; i++) //Find new free thread
        {
            if (cmdqThreadResTbl[i] == 0)
            {
                cmdqThread = i;
                break;
            }
        }

        if (cmdqThread != -1)
        {
            cmdqThreadResTbl[cmdqThread] = engineFlag;
            taskResOccuTbl[taskID].cmdqThread = cmdqThread;
        }
        else
        {
            CMDQ_ERR("Cannot find CMDQ thread\n");
            cmdqForceFree_SW(taskID);
            spin_unlock_irqrestore(&gCmdqMgrLock,flags);
            return false;
        }

        //Update HE resource TBL
        hwResTbl |= engineFlag;

        //Get Buffer info
        pCmdqAddr = cmdqBufAddr(taskID); //new Thread, current taskID must be first
        if (NULL == pCmdqAddr)
        {
            CMDQ_ERR("CmdQ buf address is NULL\n");
            cmdqForceFree_SW(taskID);
            spin_unlock_irqrestore(&gCmdqMgrLock, flags);
            return false;
        }

        //Start! Power on(TODO)!
        cmdqHwClockOn(cmdqThreadResTbl[cmdqThread], true);

        //Update EOC Cnt
        cmdqThreadFirstJobID[cmdqThread] = taskID;
        cmdqThreadTaskNumber[cmdqThread] = 1; //first Job

        // DISP_REG_CMDQ_THRx_EXEC_CMDS_CNT this means how many "frame" executed
        cmdqThreadEofCnt[taskID] = DISP_REG_GET(DISP_REG_CMDQ_THRx_EXEC_CMDS_CNT(cmdqThread)) + 1; //should be "1" always!!

        if (cmdqThreadEofCnt[taskID] > 65535)
        {
            cmdqThreadEofCnt[taskID] = cmdqThreadEofCnt[taskID] - 65536; //overflow
            ring = 1;
        }

        CMDQ_MSG("First Task %d done until %d equals %ld\n", taskID, DISP_REG_GET(DISP_REG_CMDQ_THRx_EXEC_CMDS_CNT(cmdqThread)), cmdqThreadEofCnt[taskID] );

        //Insert job to CMDQ thread
        cmdqThreadTaskList[cmdqThread][cmdqThreadTaskList_W[cmdqThread]] = taskID; //assign task to T' write pointer
        cmdqThreadTaskList_W[cmdqThread] = (cmdqThreadTaskList_W[cmdqThread] + 1) % CMDQ_THREAD_LIST_LENGTH; //increase write pointer

        CMDQ_MSG("\n\n\ncmdqTaskAssigned R: %d W: %d \n\n\n", cmdqThreadTaskList_R[cmdqThread], cmdqThreadTaskList_W[cmdqThread]);

        //Update CMDQ buffer parameter (CMDQ size / tail pointer)
        buf_id = taskResOccuTbl[taskID].cmdBufID;
        cmdq_pc_head = (unsigned long*)pCmdqAddr->VA;
        cmdqBufTbl[buf_id].blocksize = blocksize;
        cmdqBufTbl[buf_id].blockTailAddr = (cmdq_pc_head+ins_leng-1);

        //DBG message
        CMDQ_MSG("==========DISP_IOCTL_EXEC_COMMAND Task: %d ,Thread: %d, PC[0x%lx], EOC[0x%lx] =========\n",taskID, cmdqThread,(unsigned long)pCmdqAddr->MVA ,(unsigned long)(pCmdqAddr->MVA + cmdqBufTbl[buf_id].blocksize));

        // record RDMA status before start to run next frame
        mdpRdmaPrevStatus[0] = DISP_REG_GET(0xF4001400);
        mdpRdmaPrevStatus[1] = DISP_REG_GET(0xF4001408);
        mdpRdmaPrevStatus[2] = DISP_REG_GET(0xF4001410);
        mdpRdmaPrevStatus[3] = DISP_REG_GET(0xF4001420);
        mdpRdmaPrevStatus[4] = DISP_REG_GET(0xF4001430);
        mdpRdmaPrevStatus[5] = DISP_REG_GET(0xF40014D0);

        // enable CMDQ interrupt and set timeout cycles
        DISP_REG_SET(DISP_REG_CMDQ_THRx_EN(cmdqThread), 1);
        DISP_REG_SET(DISP_REG_CMDQ_THRx_IRQ_FLAG_EN(cmdqThread),0x1);  //Enable Each IRQ
        DISP_REG_SET(DISP_REG_CMDQ_THRx_INSTN_TIMEOUT_CYCLES(cmdqThread), CMDQ_TIMEOUT);  //Set time out IRQ: 2^20 cycle
        DISP_REG_SET(DISP_REG_CMDQ_THRx_SUSPEND(cmdqThread),1);

        // wait cmdq suspend
        cmdq_polling_timeout = 0;
        while ((DISP_REG_GET(DISP_REG_CMDQ_THRx_STATUS(cmdqThread))&0x2) == 0)
        {
            cmdq_polling_timeout++;
            if (cmdq_polling_timeout > 1000)
            {
                break;
            }
        }

        //Execuction
        DISP_REG_SET(DISP_REG_CMDQ_THRx_PC(cmdqThread), pCmdqAddr->MVA);
        DISP_REG_SET(DISP_REG_CMDQ_THRx_END_ADDR(cmdqThread), pCmdqAddr->MVA + cmdqBufTbl[buf_id].blocksize);
        DISP_REG_SET(DISP_REG_CMDQ_THRx_SUSPEND(cmdqThread), 0);

        spin_unlock_irqrestore(&gCmdqMgrLock, flags);
        //CS--------------------------------------------------

        //Schedule out
        if (ring == 0)
            wq_ret = wait_event_interruptible_timeout(cmq_wait_queue[cmdqThread], (DISP_REG_GET(DISP_REG_CMDQ_THRx_EXEC_CMDS_CNT(cmdqThread))>=cmdqThreadEofCnt[taskID]), HZ);
        else
            wq_ret = wait_event_interruptible_timeout(cmq_wait_queue[cmdqThread], ((DISP_REG_GET(DISP_REG_CMDQ_THRx_EXEC_CMDS_CNT(cmdqThread))<65500)&&(DISP_REG_GET(DISP_REG_CMDQ_THRx_EXEC_CMDS_CNT(cmdqThread))>=cmdqThreadEofCnt[taskID])), HZ);

        smp_rmb();

        do_gettimeofday(&end_t);

        //Clear Status
        spin_lock_irqsave(&gCmdqMgrLock, flags);

        if (wq_ret != 0)
        {
            cmdqThreadComplete(cmdqThread, taskID);
        }
        else
        {
            CMDQ_ERR("A Task %d [%d] CMDQ Status, PC: 0x%x 0x%x\n",taskID, taskIDStatusTbl[taskID], DISP_REG_GET(DISP_REG_CMDQ_THRx_PC(cmdqThread)), DISP_REG_GET(DISP_REG_CMDQ_THRx_END_ADDR(cmdqThread)));
            printk("Task %d : %d %ld", taskID, DISP_REG_GET(DISP_REG_CMDQ_THRx_EXEC_CMDS_CNT(cmdqThread)), cmdqThreadEofCnt[taskID]);

            if (pCmdqAddr != NULL)
                dumpRegDebugInfo(cmdqThreadResTbl[cmdqThread], cmdqThread, *pCmdqAddr);

            //Warm reset CMDQ!

            // CMDQ FD interrupt received, but wait queue timeout
            if ((ring == 0)&&(DISP_REG_GET(DISP_REG_CMDQ_THRx_EXEC_CMDS_CNT(cmdqThread))>=cmdqThreadEofCnt[taskID]))
            {
                waitQueueTimeoutCnt++;
                cmdqThreadComplete(cmdqThread, taskID);
                CMDQ_ERR("Status A\n");
            }
            else if ((ring == 1) && ((DISP_REG_GET(DISP_REG_CMDQ_THRx_EXEC_CMDS_CNT(cmdqThread))<65500) && (DISP_REG_GET(DISP_REG_CMDQ_THRx_EXEC_CMDS_CNT(cmdqThread)) >= cmdqThreadEofCnt[taskID])))
            {
                waitQueueTimeoutCnt++;
                cmdqThreadComplete(cmdqThread, taskID);
                CMDQ_ERR("Status B\n");
            }
            else // True Timeout
            {
                CMDQ_ERR("Status C\n");
                ret = false;

                if (taskIDStatusTbl[taskID] == 2)
                {
                    CMDQ_ERR("Status D\n");
                }
                else
                {
                    //assert aee
                    if ((cmdqCurrInst & 0x08000000) != 0)   //Polling some register
                    {
                        if ((cmdqCurrInst & 0x00010000) != 0)   // bit 16== 1 -> ISP
                        {
                            printk(KERN_ERR "Polling ISP timout in CMDQ, current inst = 0x%08x\n", cmdqCurrInst);
                        }
                        else // bit 16 == 0 -> MDP
                        {
                            printk(KERN_ERR "Polling MDP timout in CMDQ, current inst = 0x%08x\n", cmdqCurrInst);
                        }
                    }
                    else if ((cmdqCurrInst & 0x20000000) != 0)  //waiting some event
                    {
                        if ((cmdqCurrInst & 0xFFFFFF) == 12 || (cmdqCurrInst & 0xFFFFFF) == 13)
                        {
                            printk(KERN_ERR "Wait ISP event timout in CMDQ, current inst = 0x%08x\n", cmdqCurrInst);
                        }
                        else
                        {
                            printk(KERN_ERR "Wait MDP event timout in CMDQ, current inst = 0x%08x\n", cmdqCurrInst);
                        }
                    }

                    cmdqForceFreeAll(cmdqThread);
                }
            }

            if (-EFAULT == cmdq_reset_hw_engine(engineFlag))
            {
                printk(KERN_ERR "reset MDP HW engine fail\n");
            }
        }

        taskIDStatusTbl[taskID] = -1; //free at last

        spin_unlock_irqrestore(&gCmdqMgrLock,flags);
    }
    else // no free HW
    {
        CMDQ_MSG("======CMDQ: No Free HW resource====\n");

        // enable CMDQ interrupt and set timeout cycles
        DISP_REG_SET(DISP_REG_CMDQ_THRx_IRQ_FLAG_EN(cmdqThread), 0x1);  //Enable Each IRQ
        DISP_REG_SET(DISP_REG_CMDQ_THRx_INSTN_TIMEOUT_CYCLES(cmdqThread), CMDQ_TIMEOUT);  //Set time out IRQ: 2^20 cycle

        //Find Match HW in CMDQ
        for (i=0;i<CMDQ_THREAD_NUM;i++) //Find new free thread
        {
            //CMDQ_ERR("Findind....ThreadRes %x  engineFlag %x=========\n",cmdqThreadResTbl[i] , engineFlag);
            if (cmdqThreadResTbl[i] == engineFlag) //Use Same HW
            {
                cmdqThread = i;

                if (cmdqThreadTaskList_W[cmdqThread] == 0)
                    pre_w_ptr = (CMDQ_THREAD_LIST_LENGTH - 1); //round
                else
                    pre_w_ptr = cmdqThreadTaskList_W[cmdqThread]-1;

                pre_task_id = cmdqThreadTaskList[cmdqThread][pre_w_ptr];

                //Get Buffer info
                pCmdqAddr = cmdqBufAddr(taskID);
                pPre_cmdqAddr = cmdqBufAddr(pre_task_id);
                if ((NULL == pCmdqAddr))
                {
                    CMDQ_ERR("CmdQ buf address is NULL ,taskID %d, CMDQ 0x%x\n" , taskID, (unsigned int)pCmdqAddr);
                    dumpDebugInfo();
                    cmdqForceFree_SW(taskID);
                    spin_unlock_irqrestore(&gCmdqMgrLock,flags);
                    return false;
                }

                if (DISP_REG_GET(DISP_REG_CMDQ_THRx_EN(cmdqThread)) == 0)
                {
                        //Warm reset
                        DISP_REG_SET(DISP_REG_CMDQ_THRx_RESET(cmdqThread), 1);
                        //PC reset
                        DISP_REG_SET(DISP_REG_CMDQ_THRx_PC(cmdqThread), pCmdqAddr->MVA);
                        //printk("2 Set PC to 0x%x",pCmdqAddr->MVA);
                        //EN
                        DISP_REG_SET(DISP_REG_CMDQ_THRx_EN(cmdqThread), 1);
                }
                else
                {
                    DISP_REG_SET(DISP_REG_CMDQ_THRx_SUSPEND(cmdqThread),1);
                    cmdq_polling_timeout = 0;
                    while ((DISP_REG_GET(DISP_REG_CMDQ_THRx_STATUS(cmdqThread))&0x2) == 0)
                    {
                        cmdq_polling_timeout++;
                        if (cmdq_polling_timeout>1000)
                        {
                            CMDQ_ERR("CMDQ SUSPEND fail!%x %x %x %x\n", taskID ,DISP_REG_GET(DISP_REG_CMDQ_THRx_EN(cmdqThread)), DISP_REG_GET(DISPSYS_CMDQ_BASE+0x78), DISP_REG_GET(DISPSYS_CMDQ_BASE+0x7c));
                            //Warm reset
                            DISP_REG_SET(DISP_REG_CMDQ_THRx_RESET(cmdqThread), 1);
                            //PC reset
                            DISP_REG_SET(DISP_REG_CMDQ_THRx_PC(cmdqThread), pCmdqAddr->MVA);
                            //printk("3 Set PC to 0x%x",pCmdqAddr->MVA);
                            //EN
                            DISP_REG_SET(DISP_REG_CMDQ_THRx_EN(cmdqThread), 1);
                            break;
                        }
                    }
                }

                CMDQ_MSG("PC: 0x%x -> 0x%x\n",(unsigned int)DISP_REG_GET(DISP_REG_CMDQ_THRx_PC(cmdqThread)),(unsigned int)DISP_REG_GET(DISP_REG_CMDQ_THRx_END_ADDR(cmdqThread)));
                CMDQ_MSG("======CMDQ: Task %d [%x] Find Matched Thread: %d [%x] and suspend====\n",taskID,engineFlag,cmdqThread, cmdqThreadResTbl[cmdqThread]);

                break;
            }
            else if((cmdqThreadResTbl[i] & engineFlag) != 0) //Overlaped HW
            {
                cmdqThread = i;

                if (cmdqThreadTaskList_W[cmdqThread]==0)
                    pre_w_ptr = (CMDQ_THREAD_LIST_LENGTH - 1); //round
                else
                    pre_w_ptr = cmdqThreadTaskList_W[cmdqThread]-1;

                pre_task_id = cmdqThreadTaskList[cmdqThread][pre_w_ptr];

                //Get Buffer info
                pCmdqAddr = cmdqBufAddr(taskID);
                pPre_cmdqAddr = cmdqBufAddr(pre_task_id);

                if ((NULL == pCmdqAddr))
                {
                    CMDQ_ERR("CmdQ buf address is NULL ,taskID %d, CMDQ 0x%x\n", taskID, (unsigned int)pCmdqAddr);
                    dumpDebugInfo();
                    cmdqForceFree_SW(taskID);
                    spin_unlock_irqrestore(&gCmdqMgrLock,flags);
                    return false;
                }

                if (DISP_REG_GET(DISP_REG_CMDQ_THRx_EN(cmdqThread)) == 0)
                {
                    //Warm reset
                    DISP_REG_SET(DISP_REG_CMDQ_THRx_RESET(cmdqThread), 1);
                    //PC reset
                    DISP_REG_SET(DISP_REG_CMDQ_THRx_PC(cmdqThread), pCmdqAddr->MVA);
                    //printk("4 Set PC to 0x%x",pCmdqAddr->MVA);
                    //EN
                    DISP_REG_SET(DISP_REG_CMDQ_THRx_EN(cmdqThread), 1);
                }
                else
                {
                    DISP_REG_SET(DISP_REG_CMDQ_THRx_SUSPEND(cmdqThread),1);
                    cmdq_polling_timeout = 0;
                    while ((DISP_REG_GET(DISP_REG_CMDQ_THRx_STATUS(cmdqThread))&0x2) == 0)
                    {
                        cmdq_polling_timeout++;
                        if (cmdq_polling_timeout>1000)
                        {
                            CMDQ_ERR("CMDQ SUSPEND fail!%x %x %x %x\n", taskID ,DISP_REG_GET(DISP_REG_CMDQ_THRx_EN(cmdqThread)), DISP_REG_GET(DISPSYS_CMDQ_BASE+0x78), DISP_REG_GET(DISPSYS_CMDQ_BASE+0x7c));
                            //Warm reset
                            DISP_REG_SET(DISP_REG_CMDQ_THRx_RESET(cmdqThread), 1);
                            //PC reset
                            DISP_REG_SET(DISP_REG_CMDQ_THRx_PC(cmdqThread), pCmdqAddr->MVA);
                            //printk("5 Set PC to 0x%x",pCmdqAddr->MVA);
                            //EN
                            DISP_REG_SET(DISP_REG_CMDQ_THRx_EN(cmdqThread), 1);
                            break;
                        }
                    }
                }

                CMDQ_MSG("PC: 0x%x -> 0x%x\n",(unsigned int)DISP_REG_GET(DISP_REG_CMDQ_THRx_PC(cmdqThread)),(unsigned int)DISP_REG_GET(DISP_REG_CMDQ_THRx_END_ADDR(cmdqThread)));

                CMDQ_MSG("======CMDQ: Task %d [%x] Find Corresponding Thread: %d [%x] and suspend====\n",taskID,engineFlag,cmdqThread, cmdqThreadResTbl[cmdqThread]);
                if ((engineFlag&~(cmdqThreadResTbl[i]))!=0) //More Engine then Current CMDQ T
                {
                   //POWER ON! (TODO)
                    cmdqHwClockOn((engineFlag&~(cmdqThreadResTbl[i])),false);
                   //Update CMDQ Thread Table
                   cmdqThreadResTbl[i] |= engineFlag;
                   //Update HE resource TBL
                   hwResTbl |= engineFlag;

                   CMDQ_MSG("========update CMDQ T %d resource %x=======\n",cmdqThread, cmdqThreadResTbl[cmdqThread]);
                }

                break;
            }
        }

        if (cmdqThread == -1) //cannot find any thread
        {
            CMDQ_ERR("=========CMDQ Job append Error happen!! %x %x=================\n", engineFlag, hwResTbl);

            if ((engineFlag & hwResTbl) == 0) //Free HW available
            {
                CMDQ_ERR("=========DAMN!!!!!!!!=================\n");
            }

            for (i=0;i<CMDQ_THREAD_NUM;i++) //Find new free thread
            {
                CMDQ_ERR("ThreadRes %x engineFlag %x\n",cmdqThreadResTbl[i] , engineFlag);
            }

            cmdqForceFree_SW(taskID);
            spin_unlock_irqrestore(&gCmdqMgrLock, flags);

            return false;
        }

        //Update EOC Cnt
        cmdqThreadTaskNumber[cmdqThread]++; //n-th jobs

        if (cmdqThreadFirstJobID[cmdqThread] == -1)
        {
            CMDQ_ERR("cmdqThreadFirstJobID is NULL\n");
            cmdqForceFree_SW(taskID);
            spin_unlock_irqrestore(&gCmdqMgrLock,flags);
            return false;
        }

        //Because cmdqThreadEofCnt[cmdqThreadFirstJobID[cmdqThread]] == 1 always, cmdqThreadEofCnt[taskID] == cmdqThreadTaskNumber[cmdqThread]
        cmdqThreadEofCnt[taskID] = cmdqThreadTaskNumber[cmdqThread] ;

        if (cmdqThreadEofCnt[taskID] > 65535)
        {
            cmdqThreadEofCnt[taskID] = cmdqThreadEofCnt[taskID] - 65536; //overflow
            ring = 1;
        }

        CMDQ_MSG("%d-th Task %d done until %d equals %ld\n", cmdqThreadTaskNumber[cmdqThread] , taskID, DISP_REG_GET(DISP_REG_CMDQ_THRx_EXEC_CMDS_CNT(cmdqThread)), cmdqThreadEofCnt[taskID] );

        //Insert job to CMDQ thread

        cmdqThreadTaskList[cmdqThread][cmdqThreadTaskList_W[cmdqThread]] = taskID; //assign task to T' write pointer
        cmdqThreadTaskList_W[cmdqThread] = (cmdqThreadTaskList_W[cmdqThread] + 1) % CMDQ_THREAD_LIST_LENGTH; //increase write pointer

        CMDQ_MSG("\n\n\ncmdqTaskAssigned(Insert) R: %d W: %d \n\n\n",cmdqThreadTaskList_R[cmdqThread], cmdqThreadTaskList_W[cmdqThread]);

        //Update CMDQ buffer parameter (CMDQ size / tail pointer)
        buf_id = taskResOccuTbl[taskID].cmdBufID;
        cmdq_pc_head = (unsigned long *)pCmdqAddr->VA;
        cmdqBufTbl[buf_id].blocksize = blocksize;
        cmdqBufTbl[buf_id].blockTailAddr = (cmdq_pc_head+ins_leng-1);

        CMDQ_MSG("==========DISP_IOCTL_EXEC_COMMAND+ Task: %d ,Thread: %d, PC[0x%lx], EOC[0x%lx] =========\n",taskID, cmdqThread,(unsigned long)pCmdqAddr->MVA ,(unsigned long)(pCmdqAddr->MVA + cmdqBufTbl[buf_id].blocksize));

        //Thread is already complete, but ISR have not coming yet
        if (DISP_REG_GET(DISP_REG_CMDQ_THRx_PC(cmdqThread))==DISP_REG_GET(DISP_REG_CMDQ_THRx_END_ADDR(cmdqThread)))
        {
            DISP_REG_SET(DISP_REG_CMDQ_THRx_PC(cmdqThread), pCmdqAddr->MVA);
            //printk("6 Set PC to 0x%x",pCmdqAddr->MVA);
            printk(KERN_DEBUG "\n==============Reset %d's PC  to ADDR[0x%lx]===================\n",cmdqThread, pCmdqAddr->MVA);
        }
        else
        {
            if ((NULL == pPre_cmdqAddr))
            {
                CMDQ_ERR("CmdQ pre-buf address is NULL ,pre_task_id %d, pPre_cmdqAddr 0x%x\n" , pre_task_id, (unsigned int)pPre_cmdqAddr);
                dumpDebugInfo();
                cmdqForceFree_SW(taskID);
                spin_unlock_irqrestore(&gCmdqMgrLock,flags);
                return false;
            }

            *(pPre_cmdqAddr->blockTailAddr) = 0x10000001;//Jump: Absolute
            *(pPre_cmdqAddr->blockTailAddr-1) = pCmdqAddr->MVA; //Jump to here

            CMDQ_MSG("\n==============Modify %d's Pre-ID %d Jump ADDR[0x%lx] :0x%lx , 0x%lx ===================\n",cmdqThread, pre_task_id ,(unsigned long)(pPre_cmdqAddr->blockTailAddr) ,*(pPre_cmdqAddr->blockTailAddr) ,*(pPre_cmdqAddr->blockTailAddr-1) );
        }

        //Modify Thread END addr
        DISP_REG_SET(DISP_REG_CMDQ_THRx_END_ADDR(cmdqThread), pCmdqAddr->MVA + cmdqBufTbl[buf_id].blocksize);
        DISP_REG_SET(DISP_REG_CMDQ_THRx_SUSPEND(cmdqThread), 0);
        spin_unlock_irqrestore(&gCmdqMgrLock,flags);
        //CS--------------------------------------------------

        if (ring == 0)
            wq_ret = wait_event_interruptible_timeout(cmq_wait_queue[cmdqThread], (DISP_REG_GET(DISP_REG_CMDQ_THRx_EXEC_CMDS_CNT(cmdqThread))>=cmdqThreadEofCnt[taskID]), HZ);
        else
            wq_ret = wait_event_interruptible_timeout(cmq_wait_queue[cmdqThread], ((DISP_REG_GET(DISP_REG_CMDQ_THRx_EXEC_CMDS_CNT(cmdqThread))<65500)&&(DISP_REG_GET(DISP_REG_CMDQ_THRx_EXEC_CMDS_CNT(cmdqThread))>=cmdqThreadEofCnt[taskID])), HZ);

        smp_rmb();

        do_gettimeofday(&end_t);

        //Clear Status
        spin_lock_irqsave(&gCmdqMgrLock, flags);

        if (wq_ret != 0)
        {
            cmdqThreadComplete(cmdqThread, taskID);
        }
        else
        {
            CMDQ_ERR("B Task %d [%d] CMDQ Status, PC: 0x%x 0x%x\n", taskID, taskIDStatusTbl[taskID], DISP_REG_GET(DISP_REG_CMDQ_THRx_PC(cmdqThread)), DISP_REG_GET(DISP_REG_CMDQ_THRx_END_ADDR(cmdqThread)));
            printk("Task %d : %d %ld", taskID, DISP_REG_GET(DISP_REG_CMDQ_THRx_EXEC_CMDS_CNT(cmdqThread)), cmdqThreadEofCnt[taskID]);

            if (pCmdqAddr != NULL)
                dumpRegDebugInfo(cmdqThreadResTbl[cmdqThread], cmdqThread, *pCmdqAddr);

            // CMDQ FD interrupt received, but wait queue timeout
            if ((ring == 0)&&(DISP_REG_GET(DISP_REG_CMDQ_THRx_EXEC_CMDS_CNT(cmdqThread))>=cmdqThreadEofCnt[taskID]))
            {
                waitQueueTimeoutCnt++;
                cmdqThreadComplete(cmdqThread, taskID);
                CMDQ_ERR("Status E\n");
            }
            else if ((ring == 1)&&((DISP_REG_GET(DISP_REG_CMDQ_THRx_EXEC_CMDS_CNT(cmdqThread))<65500)&&(DISP_REG_GET(DISP_REG_CMDQ_THRx_EXEC_CMDS_CNT(cmdqThread))>=cmdqThreadEofCnt[taskID])))
            {
                waitQueueTimeoutCnt++;
                cmdqThreadComplete(cmdqThread, taskID);
                CMDQ_ERR("Status F\n");
            }
            else // True Timeout
            {
                if (DISP_REG_GET(DISP_REG_CMDQ_THRx_PC(cmdqThread))==DISP_REG_GET(DISP_REG_CMDQ_THRx_END_ADDR(cmdqThread)))
                {
                    cmdqIsrLossCnt++;
                }

                CMDQ_ERR("Status G\n");
                ret = false;

                if (taskIDStatusTbl[taskID]==2)
                {
                    CMDQ_ERR("Status H\n");
                }
                else
                {
                    //assert aee
                    if ((cmdqCurrInst & 0x08000000) != 0)   //Polling some register
                    {
                        if ((cmdqCurrInst & 0x00010000) != 0)   // bit 16== 1 -> ISP
                        {
                            printk(KERN_ERR "Polling ISP timout in CMDQ, current inst = 0x%08x\n", cmdqCurrInst);
                        }
                        else // bit 16 == 0 -> MDP
                        {
                            printk(KERN_ERR "Polling MDP timout in CMDQ, current inst = 0x%08x\n", cmdqCurrInst);
                        }
                    }
                    else if ((cmdqCurrInst & 0x20000000) != 0)  //waiting some event
                    {
                        if ((cmdqCurrInst & 0xFFFFFF) == 12 || (cmdqCurrInst & 0xFFFFFF) == 13)
                        {
                            printk(KERN_ERR "Wait ISP event timout in CMDQ, current inst = 0x%08x\n", cmdqCurrInst);
                        }
                        else
                        {
                            printk(KERN_ERR "Wait MDP event timout in CMDQ, current inst = 0x%08x\n", cmdqCurrInst);
                        }
                    }

                    cmdqForceFreeAll(cmdqThread);
                }
            }

            if (-EFAULT == cmdq_reset_hw_engine(engineFlag))
            {
                printk(KERN_ERR "reset MDP HW engine fail\n");
            }
        }

        taskIDStatusTbl[taskID] = -1; //free at last
        spin_unlock_irqrestore(&gCmdqMgrLock,flags);
    }

    // execution time monitor
    if (end_t.tv_usec < start_t.tv_usec)
        prevFrameExecTime.tv_usec = (end_t.tv_usec + 1000000) - start_t.tv_usec;
    else
        prevFrameExecTime.tv_usec = end_t.tv_usec - start_t.tv_usec;

    //spin_unlock(&gCmdqMgrLock);
    return ret;
}


void cmdqThreadPowerOff(int cmdqThread)
{
    // check MDP engine busy
    if(true == checkMdpEngineStatus(cmdqThreadResTbl[cmdqThread]))
    {
        printk("cmdqThreadPowerOff when MDP Busy!\n");
        mdpBusyLongCnt++;
        resetMdpEngine(cmdqThreadResTbl[cmdqThread]);
    }

    cmdqHwClockOff(cmdqThreadResTbl[cmdqThread]);

    //Return HW resource
    hwResTbl &= ~(cmdqThreadResTbl[cmdqThread]);

    //clear T' res table
    cmdqThreadResTbl[cmdqThread] = 0;
    CMDQ_MSG("\n======All job complete in cmdqThread: %d ====\n", cmdqThread);
}


void cmdqThreadComplete(int cmdqThread, int taskID)
{
    cmdqResource_free(taskID); //task complete

    //CMDQ read pointer ++
    cmdqThreadTaskList[cmdqThread][cmdqThreadTaskList_R[cmdqThread]] = 0; //Mark for complete at T' read pointer
    cmdqThreadTaskList_R[cmdqThread] = (cmdqThreadTaskList_R[cmdqThread] + 1) % CMDQ_THREAD_LIST_LENGTH; //increase Read pointer

    //0322
    if (cmdqThreadTaskList_R[cmdqThread] == cmdqThreadTaskList_W[cmdqThread]) //no task needed
    {
        //power off!
        CMDQ_MSG("============cmdqThreadComplete Task! %d R: %d W: %d \n",taskID,cmdqThreadTaskList_R[cmdqThread], cmdqThreadTaskList_W[cmdqThread]);
        cmdqThreadPowerOff(cmdqThread);
    }
}


void cmdqForceFreeAll(int cmdqThread)
{
    //SW force init
    int i = 0;
    unsigned int cmdq_polling_timeout;

    printk("Status X\n");

    for (i=0;i<CMDQ_BUFFER_NUM;i++)
    {
        cmdqBufTbl[i].Owner = -1; //free buffer
    }

    for (i=0;i<MAX_CMDQ_TASK_ID;i++)
    {
        if(taskIDStatusTbl[i]!=-1)
            taskIDStatusTbl[i] = 2; //mark as CANCEL
        taskResOccuTbl[i].cmdBufID = -1;
        taskResOccuTbl[i].cmdqThread= -1;
    }

    //Warm reset CMDQ
    DISP_REG_SET(DISP_REG_CMDQ_THRx_RESET(cmdqThread), 1);
    cmdq_polling_timeout = 0;
    while (DISP_REG_GET(DISP_REG_CMDQ_THRx_RESET(cmdqThread)) == 1)
    {
        cmdq_polling_timeout++;
        if (cmdq_polling_timeout>1000)
        {
            CMDQ_ERR("CMDQ warm reset status%x %x %x\n",DISP_REG_GET(DISP_REG_CMDQ_THRx_EN(cmdqThread)), DISP_REG_GET(DISPSYS_CMDQ_BASE+0x78), DISP_REG_GET(DISPSYS_CMDQ_BASE+0x7c));
            break;
        }
    }

    //Warm reset MDP
    resetMdpEngine(cmdqThreadResTbl[cmdqThread]);

    //Resource Free
    for (i=0;i<CMDQ_THREAD_NUM;i++)
    {
        cmdqThreadResTbl[i] = 0;
        cmdqThreadTaskList_R[i] = 0;
        cmdqThreadTaskList_W[i] = 0;
        cmdqThreadTaskNumber[i] = 0;
        cmdqThreadFirstJobID[i] = -1;
    }

    hwResTbl = 0;

    if (clock_is_on(MT_CG_MDP_RDMA_SW_CG))
        disable_clock(MT_CG_MDP_RDMA_SW_CG, "MDP_RDMA");
    if (clock_is_on(MT_CG_MDP_RSZ_SW_CG))
        disable_clock(MT_CG_MDP_RSZ_SW_CG, "MDP_RSZ");
    if (clock_is_on(MT_CG_MDP_SHP_SW_CG))
        disable_clock(MT_CG_MDP_SHP_SW_CG, "MDP_SHP");
    if (clock_is_on(MT_CG_MDP_WROT_SW_CG))
        disable_clock(MT_CG_MDP_WROT_SW_CG, "MDP_WROT");
}


void cmdqForceFree_SW(int taskID)
{
    //SW force init
    int bufID = -1;

    if ((taskID == -1) || (taskID >= MAX_CMDQ_TASK_ID))
    {
        printk("\n cmdqForceFree_SW Free Invalid Task ID \n");
        dumpDebugInfo();
        return;
    }

    taskIDStatusTbl[taskID] = -1; //mark for free
    taskResOccuTbl[taskID].cmdBufID = -1;
    taskResOccuTbl[taskID].cmdqThread = -1;
    cmdqThreadEofCnt[taskID]  = -1;

    bufID = taskResOccuTbl[taskID].cmdBufID;

    printk("\n cmdqForceFree_SW Free Buf %x own by [%x=%x]\n",bufID,taskID,cmdqBufTbl[bufID].Owner);

    if(bufID != -1) //Free All resource and return ID
    {
        cmdqBufTbl[bufID].Owner = -1;
    }
    else
    {
        CMDQ_ERR("\n cmdqForceFree_SW Safely Free Invalid Buffer ID\n");
        dumpDebugInfo();
    }

    //FIXME! work around in 6572 (only one thread in 6572)
    hwResTbl = 0;
}


void cmdqTerminated(void)
{
    unsigned long flags;

    spin_lock_irqsave(&gCmdqMgrLock,flags);

    if((cmdqThreadTaskList_R[0] == cmdqThreadTaskList_W[0]) &&  (cmdqThreadResTbl[0] !=0)) //no task needed, but resource leaked!
    {
        CMDQ_ERR("\n======CMDQ Process terminated handling : %d ====\n", cmdqThreadResTbl[0]);

        cmdqForceFreeAll(0);
    }

    spin_unlock_irqrestore(&gCmdqMgrLock, flags);
}


int cmdqRegisterCallback(int index, CMDQ_TIMEOUT_PTR pTimeoutFunc , CMDQ_RESET_PTR pResetFunc)
{
    if((index>=2)||(NULL == pTimeoutFunc) || (NULL == pResetFunc))
    {
        printk("Warning! [Func]%s register NULL function : %p,%p\n", __func__ , pTimeoutFunc , pResetFunc);
        return -1;
    }

    g_CMDQ_CB_Array.cmdqTimeout_cb[index] = pTimeoutFunc;
    g_CMDQ_CB_Array.cmdqReset_cb[index] = pResetFunc;


    return 0;
}

static int __init cmdq_mdp_init(void)
{
    struct proc_dir_entry *pEntry = NULL;
    
    // Mount proc entry for non-specific group ioctls
    pEntry = create_proc_entry("mtk_mdp_cmdq", 0, NULL);
    if (NULL != pEntry)
    {
        pEntry->proc_fops = &cmdq_proc_fops;
    }
    else
    {
        printk("[CMDQ] cannot create mtk_mdp_cmdq!!!!!\n");
    }
    return 0;
}

static void __exit cmdq_mdp_exit(void)
{
    return;
}
    
module_init(cmdq_mdp_init);
module_exit(cmdq_mdp_exit);

MODULE_AUTHOR("Pablo Sun <pablo.sun@mediatek.com>");
MODULE_DESCRIPTION("CMDQ device Driver");
MODULE_LICENSE("GPL");

