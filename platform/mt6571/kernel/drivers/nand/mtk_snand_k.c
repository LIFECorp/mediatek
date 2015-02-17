/******************************************************************************
* mtk_snand.c - MTK NAND Flash Device Driver
 *
* Copyright 2009-2012 MediaTek Co.,Ltd.
 *
* DESCRIPTION:
* 	This file provid the other drivers nand relative functions
 *
* modification history
* ----------------------------------------
* v3.0, 11 Feb 2010, mtk
* ----------------------------------------
******************************************************************************/

#include <linux/slab.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/wait.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/partitions.h>
#include <linux/mtd/nand_ecc.h>
#include <linux/dma-mapping.h>
#include <linux/jiffies.h>
#include <linux/platform_device.h>
#include <linux/proc_fs.h>
#include <linux/time.h>
#include <linux/mm.h>
#include <linux/xlog.h>
#include <asm/io.h>
#include <asm/cacheflush.h>
#include <asm/uaccess.h>
#include <linux/miscdevice.h>
#include <mach/mtk_nand.h>
#include <mach/mtk_snand_k.h>
#include <mach/dma.h>
#include <mach/devs.h>
#include <mach/mt_reg_base.h>
#include <mach/mt_typedefs.h>
#include <mach/mt_clkmgr.h>
#include <mach/mtk_nand.h>
#include <mach/bmt.h>
#include <mach/mt_irq.h>
#include "partition.h"
#include <asm/system.h>
#include "partition_define.h"
#include <mach/mt_boot.h>
//#include "../../../../../../source/kernel/drivers/aee/ipanic/ipanic.h"
#include <linux/rtc.h>

#define VERSION  	"v1.0"
#define MODULE_NAME	"# MTK SNAND #"
#define PROCNAME    "driver/nand"
#define PMT 							1
#define _MTK_NAND_DUMMY_DRIVER_
#define __INTERNAL_USE_AHB_MODE__ 	(1)

//#define _SNAND_DEBUG

/*
 * Access Pattern Logger
 *
 * Enable the feacility to record MTD/DRV read/program/erase pattern
 */
//#define CFG_SNAND_ACCESS_PATTERN_LOGGER
//#define CFG_SNAND_ACCESS_PATTERN_LOGGER_BOOTUP    // NOTE. Must define CFG_SNAND_ACCESS_PATTERN_LOGGER first!

//#define CFG_SNAND_SLEEP_WHILE_BUSY              // stanley chu
//#define CFG_SNAND_SLEEP_WHILE_BUSY_PROC_TEST          // stanley chu

//#define CFG_SNAND_PAGE_READ_NFI_MODE_AUTO       // stanley chu

//#define CFG_SNAND_DRV_RW_BREAKDOWN
//#define CFG_SNAND_DRV_RW_BREAKDOWN_DMA_READ

#ifdef CFG_SNAND_DRV_RW_BREAKDOWN
struct timeval g_stimer[5], g_etimer[16];
#endif

#if defined(CFG_SNAND_SLEEP_WHILE_BUSY)
    #if defined(CFG_SNAND_PAGE_READ_NFI_MODE_AUTO)
        u32 g_snand_sleep_us_page_read_min = 10;
        u32 g_snand_sleep_us_page_read_max = 10;
    #else   // custom read
        u32 g_snand_sleep_us_page_read_min = 10;
        u32 g_snand_sleep_us_page_read_max = 20;
    #endif
u32 g_snand_sleep_temp_val1, g_snand_sleep_temp_val2;
#endif

void show_stack(struct task_struct *tsk, unsigned long *sp);
extern void mt_irq_set_sens(unsigned int irq, unsigned int sens);
extern void mt_irq_set_polarity(unsigned int irq,unsigned int polarity);

static void mtk_snand_reset_dev(void);

void        mtk_snand_dump_reg(void);
static void mtk_snand_dev_read_id(u8 id[]);
void        mtk_snand_dump_mem(u32 * buf, u32 size);
void        mtk_snand_dump_reg(void);
static bool mtk_snand_ready_for_read_custom(struct nand_chip *nand, u32 u4RowAddr, u32 u4ColAddr, u32 u4SecNum, u8 * buf, u8 mtk_ecc, u8 auto_fmt, u8 ahb_mode);
static bool mtk_snand_reset_con(void);
static void mtk_snand_stop_read_custom(u8 mtk_ecc);

#if 0
static void mtk_snand_stop_read_auto(void);
#endif

// deprecated declarations
//static bool mtk_snand_ready_for_read_auto(struct nand_chip *nand, u32 u4RowAddr, u32 u4ColAddr, u32 u4SecNum, bool full, u8 * buf);

#if !defined(CFG_SNAND_PAGE_READ_NFI_MODE_AUTO) // custom read
//#define mtk_snand_ready_for_read    mtk_snand_ready_for_read_custom
//#define mtk_snand_stop_read         mtk_snand_stop_read_custom
#else   // auto read
//#define mtk_snand_ready_for_read    mtk_snand_ready_for_read_auto
//#define mtk_snand_stop_read         mtk_snand_stop_read_auto
#endif

typedef enum
{
     SNAND_RB_DEFAULT    = 0
    ,SNAND_RB_READ_ID    = 1
    ,SNAND_RB_CMD_STATUS = 2
    ,SNAND_RB_PIO        = 3
} SNAND_Read_Byte_Mode;

typedef enum
{
     SNAND_IDLE                 = 0
    ,SNAND_DEV_ERASE_DONE
    ,SNAND_DEV_PROG_DONE
    ,SNAND_BUSY
    ,SNAND_NFI_CUST_READING
    ,SNAND_NFI_AUTO_ERASING
    ,SNAND_DEV_PROG_EXECUTING
} SNAND_Status;

SNAND_Read_Byte_Mode    g_snand_read_byte_mode 	= SNAND_RB_DEFAULT;
SNAND_Status            g_snand_dev_status     	= SNAND_IDLE;    // used for mtk_snand_dev_ready() and interrupt waiting service
int						g_snand_cmd_status		= NAND_STATUS_READY;

u8 g_snand_id_data[SNAND_MAX_ID + 1];
u8 g_snand_id_data_idx = 0;

// Read Split related definitions and variables
#define SNAND_RS_BOUNDARY_BLOCK                     (2)
#define SNAND_RS_BOUNDARY_KB                        (1024)

#define SNAND_RS_SPARE_PER_SECTOR_PART0_VAL         (16)                                        // MT6571 shoud fix this as 0
#define SNAND_RS_SPARE_PER_SECTOR_PART0_NFI         (PAGEFMT_SPARE_16 << PAGEFMT_SPARE_SHIFT)   // MT6571 shoud fix this as 16
#define SNAND_RS_ECC_BIT_PART0                      (0)                                         // MT6571 use device ECC in part 1

u32 g_snand_rs_ecc_bit_second_part;
u32 g_snand_rs_spare_per_sector_second_part_nfi;
u32 g_snand_rs_num_page = 0;
u32 g_snand_rs_cur_part = 0xFFFFFFFF;
u32 g_snand_rs_ecc_bit = 0;

#ifdef _SNAND_DEBUG
u8  g_snand_nfi_bypass_enabled = 0;
#endif

u32 g_snand_k_spare_per_sector = 0;   // because Read Split feature will change spare_per_sector in run-time, thus use a variable to indicate current spare_per_sector

#define SNAND_WriteReg(reg, value) \
do { \
    *(reg) = (value); \
} while (0)

#define SNAND_REG_SET_BITS(reg, value) \
do { \
    *(reg) = *(reg) | (value); \
} while (0)

#define SNAND_REG_CLN_BITS(reg, value) \
do { \
    *(reg) = *(reg) & (~(value)); \
} while (0)

#define SNAND_MAX_PAGE_SIZE             (4096)
#define SNAND_FDM_DATA_SIZE_PER_SECTOR  (8)
#define NAND_SECTOR_SIZE                (512)
#define OOB_PER_SECTOR                  (28)
#define OOB_AVAI_PER_SECTOR             (8)

__attribute__((aligned(64))) unsigned char g_snand_k_spare[128];
__attribute__((aligned(64))) unsigned char g_snand_k_temp[4096 + 128];

#if defined(MTK_COMBO_NAND_SUPPORT)
	// BMT_POOL_SIZE is not used anymore
#else
	#ifndef PART_SIZE_BMTPOOL
	#define BMT_POOL_SIZE (80)
	#else
	#define BMT_POOL_SIZE (PART_SIZE_BMTPOOL)
	#endif
#endif

#define PMT_POOL_SIZE	(2)
/*******************************************************************************
 * Gloable Varible Definition
 *******************************************************************************/
struct nand_perf_log
{
    unsigned int ReadPageCount;
    suseconds_t  ReadPageTotalTime;
    unsigned int ReadBusyCount;
    suseconds_t  ReadBusyTotalTime;
    unsigned int ReadDMACount;
    suseconds_t  ReadDMATotalTime;

    unsigned int WritePageCount;
    suseconds_t  WritePageTotalTime;
    unsigned int WriteBusyCount;
    suseconds_t  WriteBusyTotalTime;
    unsigned int WriteDMACount;
    suseconds_t  WriteDMATotalTime;

    unsigned int EraseBlockCount;
    suseconds_t  EraseBlockTotalTime;

    unsigned int ReadSectorCount;
    suseconds_t  ReadSectorTotalTime;
};

static struct nand_perf_log g_NandPerfLog={0};
#ifdef NAND_PFM
static suseconds_t g_PFM_R = 0;
static suseconds_t g_PFM_W = 0;
static suseconds_t g_PFM_E = 0;
static u32 g_PFM_RNum = 0;
static u32 g_PFM_RD = 0;
static u32 g_PFM_WD = 0;
static struct timeval g_now;

#define PFM_BEGIN(time) \
do_gettimeofday(&g_now); \
(time) = g_now;

#define PFM_END_R(time, n) \
do_gettimeofday(&g_now); \
g_PFM_R += (g_now.tv_sec * 1000000 + g_now.tv_usec) - (time.tv_sec * 1000000 + time.tv_usec); \
g_PFM_RNum += 1; \
g_PFM_RD += n; \
MSG(PERFORMANCE, "%s - Read PFM: %lu, data: %d, ReadOOB: %d (%d, %d)\n", MODULE_NAME , g_PFM_R, g_PFM_RD, g_kCMD.pureReadOOB, g_kCMD.pureReadOOBNum, g_PFM_RNum);

#define PFM_END_W(time, n) \
do_gettimeofday(&g_now); \
g_PFM_W += (g_now.tv_sec * 1000000 + g_now.tv_usec) - (time.tv_sec * 1000000 + time.tv_usec); \
g_PFM_WD += n; \
MSG(PERFORMANCE, "%s - Write PFM: %lu, data: %d\n", MODULE_NAME, g_PFM_W, g_PFM_WD);

#define PFM_END_E(time) \
do_gettimeofday(&g_now); \
g_PFM_E += (g_now.tv_sec * 1000000 + g_now.tv_usec) - (time.tv_sec * 1000000 + time.tv_usec); \
MSG(PERFORMANCE, "%s - Erase PFM: %lu\n", MODULE_NAME, g_PFM_E);
#else
#define PFM_BEGIN(time)
#define PFM_END_R(time, n)
#define PFM_END_W(time, n)
#define PFM_END_E(time)
#endif

#define TIMEOUT_1   0x1fff
#define TIMEOUT_2   0x8ff
#define TIMEOUT_3   0xffff
#define TIMEOUT_4   0xffff      //5000   //PIO

#define NFI_ISSUE_COMMAND(cmd, col_addr, row_addr, col_num, row_num) \
   do { \
      SNAND_WriteReg(NFI_CMD_REG16,cmd);\
      while (*(NFI_STA_REG32) & STA_CMD_STATE);\
      SNAND_WriteReg(NFI_COLADDR_REG32, col_addr);\
      SNAND_WriteReg(NFI_ROWADDR_REG32, row_addr);\
      SNAND_WriteReg(NFI_ADDRNOB_REG16, col_num | (row_num<<ADDR_ROW_NOB_SHIFT));\
      while (*(NFI_STA_REG32) & STA_ADDR_STATE);\
   }while(0);

//-------------------------------------------------------------------------------
static struct completion g_comp_AHB_Done;
static struct NAND_CMD g_kCMD;
bool g_bInitDone;
static int g_i4Interrupt;
static int g_page_size;

#if __INTERNAL_USE_AHB_MODE__
BOOL g_bHwEcc = 1;
#else
BOOL g_bHwEcc = 0;
#endif

#if defined(MT6571)
#define _SNAND_CACHE_LINE_SIZE  (64)
#else
#error "[K-SNAND] Please defince _SNAND_CACHE_LINE_SIZE!"
#endif

static u8 *local_buffer_cache_size_align;   // _SNAND_CACHE_LINE_SIZE byte aligned buffer, for HW issue
__attribute__((aligned(_SNAND_CACHE_LINE_SIZE))) static u8 local_buffer[SNAND_MAX_PAGE_SIZE + _SNAND_CACHE_LINE_SIZE];

extern void nand_release_device(struct mtd_info *mtd);
extern int nand_get_device(struct nand_chip *chip, struct mtd_info *mtd, int new_state);

static bmt_struct *g_bmt;
struct mtk_nand_host *host;
static u8 g_running_dma = 0;
#ifdef DUMP_NATIVE_BACKTRACE
static u32 g_dump_count = 0;
#endif
extern struct mtd_partition g_pasStatic_Partition[];
int part_num = NUM_PARTITIONS;
#ifdef PMT
extern void part_init_pmt(struct mtd_info *mtd, u8 * buf);
extern struct mtd_partition g_exist_Partition[];
#endif
int manu_id;
int dev_id;

static u8 local_oob_buf[NAND_MAX_OOBSIZE];

#ifdef _MTK_NAND_DUMMY_DRIVER_
int dummy_driver_debug;
#endif

snand_flashdev_info devinfo;

#ifdef CFG_SNAND_ACCESS_PATTERN_LOGGER

typedef enum
{
     _SNAND_PM_OP_READ_PAGE             = 1
    ,_SNAND_PM_OP_READ_PAGE_CACHE_HIT
    ,_SNAND_PM_OP_READ_OOB
    ,_SNAND_PM_OP_READ_SEC
    ,_SNAND_PM_OP_READ_SEC_CACHE_HIT
    ,_SNAND_PM_OP_PROGRAM
    ,_SNAND_PM_OP_ERASE
    ,_SNAND_PM_OP_END
} SNAND_PM_OP;

typedef enum
{
     _SNAND_PM_LAYER_MTD    = 1
    ,_SNAND_PM_LAYER_DRIVER = 2
} SNAND_PM_LAYER;

typedef struct
{
    u8 op;
    u8 layer;
    u32 pid;
    u32 parent_pid;
    u32 val[2];
    u32 time_s;
    u32 time_ns;
    u32 duration;
} SNAND_PM_RECORD;

#define _SNAND_PM_RECORD_CNT    (200000)    // bootup needs about 120000 records

SNAND_PM_RECORD g_snand_pm_data[_SNAND_PM_RECORD_CNT];

#ifdef CFG_SNAND_ACCESS_PATTERN_LOGGER_BOOTUP
int g_snand_dbg_ap_on = 1;
#else
int g_snand_dbg_ap_on = 0;
#endif

u32 g_snand_pm_cnt = 0;
u32 g_snand_pm_wrapped = 0;
u32 g_snand_pm_initial_tvsec = 0xFFFFFFFF;

char g_snand_pm_part_name_bmt[4] = {'b', 'm', 't', '\0'};

u32 mtk_snand_pm_get_part_idx(u32 row_addr)
{
    u32 byte;
    u32 i;
    u32 byte_left = 0x20000000 - (0x50 * 64 * 2048);    // minus BMT size

    byte = row_addr * 2048;

    for (i = 0; (g_exist_Partition[i].size != 0) || (g_exist_Partition[i].offset != 0); i++)
    {
        if (0 == g_exist_Partition[i].size) // reach USRDATA
        {
            if ((byte >= g_exist_Partition[i].offset) && (byte < (g_exist_Partition[i].offset + byte_left)))
            {
                return i;
            }
        }
        else
        {
            if ((byte >= g_exist_Partition[i].offset) && (byte < (g_exist_Partition[i].offset + g_exist_Partition[i].size)))
            {
                return i;
            }
        }

        byte_left -= g_exist_Partition[i].size;
    }

    return i;
}

char * mtk_snand_pm_get_part_name(u32 row_addr)
{
    u32 i;

    i = mtk_snand_pm_get_part_idx(row_addr);

    if ((g_exist_Partition[i].size != 0) || (g_exist_Partition[i].offset != 0))
    {
        return g_exist_Partition[i].name;
    }
    else
    {
        return &(g_snand_pm_part_name_bmt[0]);
    }
}

void mtk_snand_pm_init()
{
    struct timespec tv;

    getnstimeofday(&tv);

    g_snand_pm_initial_tvsec = tv.tv_sec - ((jiffies - INITIAL_JIFFIES) / HZ);
}

void mtk_snand_pm_add_drv_record(u32 op, u32 row, u32 sec, u32 duration)
{
    struct timespec tv;
    struct task_struct * cur_thread;

    cur_thread = get_current();

    getnstimeofday(&tv);

    if ((0xFFFFFFFF == g_snand_pm_initial_tvsec) && (tv.tv_sec != 0))
    {
        mtk_snand_pm_init();
    }

    if (g_snand_pm_cnt >= _SNAND_PM_RECORD_CNT)
    {
        g_snand_pm_cnt = 0;
        g_snand_pm_wrapped++;
    }

    g_snand_pm_data[g_snand_pm_cnt].pid = cur_thread->pid;;

    if (cur_thread->real_parent != NULL)
    {
        g_snand_pm_data[g_snand_pm_cnt].parent_pid = cur_thread->real_parent->pid;
    }
    else
    {
        g_snand_pm_data[g_snand_pm_cnt].parent_pid = 0;
    }

    g_snand_pm_data[g_snand_pm_cnt].op = op;
    g_snand_pm_data[g_snand_pm_cnt].layer = _SNAND_PM_LAYER_DRIVER;
    g_snand_pm_data[g_snand_pm_cnt].val[0] = row;
    g_snand_pm_data[g_snand_pm_cnt].duration = duration;

    if (op <= _SNAND_PM_OP_READ_PAGE_CACHE_HIT)
    {
        g_snand_pm_data[g_snand_pm_cnt].val[1] = 0;
    }
    else
    {
        g_snand_pm_data[g_snand_pm_cnt].val[1] = sec;
    }

    if (0xFFFFFFFF != g_snand_pm_initial_tvsec)
    {
        g_snand_pm_data[g_snand_pm_cnt].time_s = tv.tv_sec - g_snand_pm_initial_tvsec;
    }
    else
    {
        g_snand_pm_data[g_snand_pm_cnt].time_s = tv.tv_sec; // must be 0
    }

    g_snand_pm_data[g_snand_pm_cnt].time_ns = tv.tv_nsec;

    g_snand_pm_cnt++;
}

void mtk_snand_pm_add_mtd_record(u32 op, u32 from, u32 len)
{
    struct timespec tv;
    struct task_struct * cur_thread;

    cur_thread = get_current();

    getnstimeofday(&tv);

    if ((0xFFFFFFFF == g_snand_pm_initial_tvsec) && (tv.tv_sec != 0))
    {
        mtk_snand_pm_init();
    }

    if (g_snand_pm_cnt >= _SNAND_PM_RECORD_CNT)
    {
        g_snand_pm_cnt = 0;
        g_snand_pm_wrapped++;
    }

    g_snand_pm_data[g_snand_pm_cnt].pid = cur_thread->pid;;

    if (cur_thread->real_parent != NULL)
    {
        g_snand_pm_data[g_snand_pm_cnt].parent_pid = cur_thread->real_parent->pid;
    }
    else
    {
        g_snand_pm_data[g_snand_pm_cnt].parent_pid = 0;
    }

    g_snand_pm_data[g_snand_pm_cnt].op = op;
    g_snand_pm_data[g_snand_pm_cnt].layer = _SNAND_PM_LAYER_MTD;
    g_snand_pm_data[g_snand_pm_cnt].val[0] = from;
    g_snand_pm_data[g_snand_pm_cnt].val[1] = len;

    getnstimeofday(&tv);

    if (0xFFFFFFFF != g_snand_pm_initial_tvsec)
    {
        g_snand_pm_data[g_snand_pm_cnt].time_s = tv.tv_sec - g_snand_pm_initial_tvsec;
    }
    else
    {
        g_snand_pm_data[g_snand_pm_cnt].time_s = tv.tv_sec; // must be 0
    }

    g_snand_pm_data[g_snand_pm_cnt].time_ns = tv.tv_nsec;

    g_snand_pm_cnt++;
}

void mtk_snand_pm_dump_one_record(u32 idx)
{
    static u32 mtd_len = 0;

    if (g_snand_pm_data[idx].layer == _SNAND_PM_LAYER_DRIVER)   // DRIVER
    {
        if (g_snand_pm_data[idx].op == _SNAND_PM_OP_READ_SEC)
        {
            printk("[%d:%d][%d.%d][%d]  ++-DRV RS r=%d, b=%d, p=%d, s=%d, mtd_len=%d, t=%dus, %s\n", g_snand_pm_data[idx].parent_pid, g_snand_pm_data[idx].pid, g_snand_pm_data[idx].time_s, g_snand_pm_data[idx].time_ns / 1000, idx, g_snand_pm_data[idx].val[0], g_snand_pm_data[idx].val[0] / ((devinfo.blocksize << 10) / devinfo.pagesize), g_snand_pm_data[idx].val[0] % ((devinfo.blocksize << 10) / devinfo.pagesize), g_snand_pm_data[idx].val[1], mtd_len, g_snand_pm_data[idx].duration, mtk_snand_pm_get_part_name(g_snand_pm_data[idx].val[0]));
        }
        else if (g_snand_pm_data[idx].op == _SNAND_PM_OP_READ_SEC_CACHE_HIT)
        {
            printk("[%d:%d][%d.%d][%d]  ++-DRV RSC r=%d, b=%d, p=%d, s=%d, mtd_len=%d, %s\n", g_snand_pm_data[idx].parent_pid, g_snand_pm_data[idx].pid, g_snand_pm_data[idx].time_s, g_snand_pm_data[idx].time_ns / 1000, idx, g_snand_pm_data[idx].val[0], g_snand_pm_data[idx].val[0] / ((devinfo.blocksize << 10) / devinfo.pagesize), g_snand_pm_data[idx].val[0] % ((devinfo.blocksize << 10) / devinfo.pagesize), g_snand_pm_data[idx].val[1], mtd_len, mtk_snand_pm_get_part_name(g_snand_pm_data[idx].val[0]));
        }
        else if (g_snand_pm_data[idx].op == _SNAND_PM_OP_READ_PAGE)
        {
            printk("[%d:%d][%d.%d][%d]  ++-DRV RP r=%d, b=%d, p=%d, mtd_len=%d, t=%dus, %s\n", g_snand_pm_data[idx].parent_pid, g_snand_pm_data[idx].pid, g_snand_pm_data[idx].time_s, g_snand_pm_data[idx].time_ns / 1000, idx, g_snand_pm_data[idx].val[0], g_snand_pm_data[idx].val[0] / ((devinfo.blocksize << 10) / devinfo.pagesize), g_snand_pm_data[idx].val[0] % ((devinfo.blocksize << 10) / devinfo.pagesize), mtd_len, g_snand_pm_data[idx].duration, mtk_snand_pm_get_part_name(g_snand_pm_data[idx].val[0]));
        }
        else if (g_snand_pm_data[idx].op == _SNAND_PM_OP_READ_PAGE_CACHE_HIT)
        {
            printk("[%d:%d][%d.%d][%d]  ++-DRV RPC r=%d, b=%d, p=%d, mtd_len=%d, %s\n", g_snand_pm_data[idx].parent_pid, g_snand_pm_data[idx].pid, g_snand_pm_data[idx].time_s, g_snand_pm_data[idx].time_ns / 1000, idx, g_snand_pm_data[idx].val[0], g_snand_pm_data[idx].val[0] / ((devinfo.blocksize << 10) / devinfo.pagesize), g_snand_pm_data[idx].val[0] % ((devinfo.blocksize << 10) / devinfo.pagesize), mtd_len, mtk_snand_pm_get_part_name(g_snand_pm_data[idx].val[0]));
        }
        else if (g_snand_pm_data[idx].op == _SNAND_PM_OP_PROGRAM)
        {
            printk("[%d:%d][%d.%d][%d]  ++-DRV PP r=%d, b=%d, t=%dus, %s\n", g_snand_pm_data[idx].parent_pid, g_snand_pm_data[idx].pid, g_snand_pm_data[idx].time_s, g_snand_pm_data[idx].time_ns / 1000, idx, g_snand_pm_data[idx].val[0], g_snand_pm_data[idx].val[0] / ((devinfo.blocksize << 10) / devinfo.pagesize), g_snand_pm_data[idx].duration, mtk_snand_pm_get_part_name(g_snand_pm_data[idx].val[0]));
        }
        else if (g_snand_pm_data[idx].op == _SNAND_PM_OP_ERASE)
        {
            printk("[%d:%d][%d.%d][%d]  ++-DRV EB r=%d, b=%d, t=%dus, %s\n", g_snand_pm_data[idx].parent_pid, g_snand_pm_data[idx].pid, g_snand_pm_data[idx].time_s, g_snand_pm_data[idx].time_ns / 1000, idx, g_snand_pm_data[idx].val[0], g_snand_pm_data[idx].val[0] / ((devinfo.blocksize << 10) / devinfo.pagesize), g_snand_pm_data[idx].duration, mtk_snand_pm_get_part_name(g_snand_pm_data[idx].val[0]));
        }
        else
        {
            printk("[%d:%d][%d.%d][%d]  ++-DRV ERROR! Unknown OP\n", g_snand_pm_data[idx].parent_pid, g_snand_pm_data[idx].pid, g_snand_pm_data[idx].time_s, g_snand_pm_data[idx].time_ns / 1000, idx);
        }
    }
    else    // MTD
    {
        if (g_snand_pm_data[idx].op == _SNAND_PM_OP_READ_PAGE)
        {
            printk("[%d:%d][%d.%d][%d] +++MTD RD from=%d, b=%d, p=%d, col=%d, len=%d\n", g_snand_pm_data[idx].parent_pid, g_snand_pm_data[idx].pid, g_snand_pm_data[idx].time_s, g_snand_pm_data[idx].time_ns / 1000, idx, g_snand_pm_data[idx].val[0], g_snand_pm_data[idx].val[0] / (devinfo.blocksize << 10), (g_snand_pm_data[idx].val[0] % (devinfo.blocksize << 10)) / devinfo.pagesize, g_snand_pm_data[idx].val[0] % devinfo.pagesize, g_snand_pm_data[idx].val[1]);
        }
        else if (g_snand_pm_data[idx].op == _SNAND_PM_OP_READ_OOB)
        {
            printk("[%d:%d][%d.%d][%d] +++MTD RO from=%d, b=%d, p=%d, col=%d, len=%d\n", g_snand_pm_data[idx].parent_pid, g_snand_pm_data[idx].pid, g_snand_pm_data[idx].time_s, g_snand_pm_data[idx].time_ns / 1000, idx, g_snand_pm_data[idx].val[0], g_snand_pm_data[idx].val[0] / (devinfo.blocksize << 10), (g_snand_pm_data[idx].val[0] % (devinfo.blocksize << 10)) / devinfo.pagesize, g_snand_pm_data[idx].val[0] % devinfo.pagesize, g_snand_pm_data[idx].val[1]);
        }
        else if (g_snand_pm_data[idx].op == _SNAND_PM_OP_PROGRAM)
        {
            printk("[%d:%d][%d.%d][%d] +++MTD PG from=%d, b=%d, p=%d, col=%d, len=%d\n", g_snand_pm_data[idx].parent_pid, g_snand_pm_data[idx].pid, g_snand_pm_data[idx].time_s, g_snand_pm_data[idx].time_ns / 1000, idx, g_snand_pm_data[idx].val[0], g_snand_pm_data[idx].val[0] / (devinfo.blocksize << 10), (g_snand_pm_data[idx].val[0] % (devinfo.blocksize << 10)) / devinfo.pagesize, g_snand_pm_data[idx].val[0] % devinfo.pagesize, g_snand_pm_data[idx].val[1]);
        }
        else if (g_snand_pm_data[idx].op == _SNAND_PM_OP_ERASE)
        {
            printk("[%d:%d][%d.%d][%d] +++MTD ER b=%d\n", g_snand_pm_data[idx].parent_pid, g_snand_pm_data[idx].pid, g_snand_pm_data[idx].time_s, g_snand_pm_data[idx].time_ns / 1000, idx, g_snand_pm_data[idx].val[0]);
        }
        else
        {
            printk("[%d:%d][%d.%d][%d] +++MTD ERROR! Unknown OP\n", g_snand_pm_data[idx].parent_pid, g_snand_pm_data[idx].pid, g_snand_pm_data[idx].time_s, g_snand_pm_data[idx].time_ns / 1000, idx);
        }

        mtd_len = g_snand_pm_data[idx].val[1];
    }
}

void mtk_snand_pm_dump_record()
{
    u32 i;
    u32 mtd_op_cnt[_SNAND_PM_OP_END] = {0};
    u32 drv_op_cnt[_SNAND_PM_OP_END] = {0};

    printk("[SNAND] ====== Performance Monitor Dump (begin) ======\n");

    if (g_snand_pm_wrapped)
    {
        for (i = g_snand_pm_cnt; i < _SNAND_PM_RECORD_CNT; i++)
        {
            mtk_snand_pm_dump_one_record(i);

            if (g_snand_pm_data[i].layer == _SNAND_PM_LAYER_DRIVER)
            {
                drv_op_cnt[g_snand_pm_data[i].op]++;
            }
            else
            {
                mtd_op_cnt[g_snand_pm_data[i].op]++;
            }

            usleep_range(5000, 5000);
        }
    }

    for (i = 0; i < g_snand_pm_cnt; i++)
    {
        mtk_snand_pm_dump_one_record(i);

        if (g_snand_pm_data[i].layer == _SNAND_PM_LAYER_DRIVER)
        {
            drv_op_cnt[g_snand_pm_data[i].op]++;
        }
        else
        {
            mtd_op_cnt[g_snand_pm_data[i].op]++;
        }

        usleep_range(5000, 5000);
    }

    printk("#wrap: %d\n", g_snand_pm_wrapped);
    printk("#cnt : %d\n", g_snand_pm_cnt);

    printk("#MTD: _SNAND_PM_OP_READ_PAGE: %d\n", mtd_op_cnt[_SNAND_PM_OP_READ_PAGE]);
    printk("#MTD: _SNAND_PM_OP_PROGRAM: %d\n", mtd_op_cnt[_SNAND_PM_OP_PROGRAM]);

    printk("#DRV: _SNAND_PM_OP_READ_PAGE: %d\n", drv_op_cnt[_SNAND_PM_OP_READ_PAGE]);
    printk("#DRV: _SNAND_PM_OP_READ_PAGE_CACHE_HIT: %d\n", drv_op_cnt[_SNAND_PM_OP_READ_PAGE_CACHE_HIT]);
    printk("#DRV: _SNAND_PM_OP_READ_SEC: %d\n", drv_op_cnt[_SNAND_PM_OP_READ_SEC]);
    printk("#DRV: _SNAND_PM_OP_READ_SEC_CACHE_HIT: %d\n", drv_op_cnt[_SNAND_PM_OP_READ_SEC_CACHE_HIT]);
    printk("#DRV: _SNAND_PM_OP_PROGRAM: %d\n", drv_op_cnt[_SNAND_PM_OP_PROGRAM]);
    printk("#DRV: _SNAND_PM_OP_ERASE: %d\n", drv_op_cnt[_SNAND_PM_OP_ERASE]);

    printk("[SNAND] ====== Performance Monitor Dump (end) ======\n");
}

#else

// TODO: emptize PM APIs

#endif

void nand_enable_clock(void)
{
    enable_clock(MT_CG_NFI_SW_CG, "NAND");
    enable_clock(MT_CG_SPINFI_SW_CG, "NAND");
	enable_clock(MT_CG_NFI_HCLK_SW_CG, "NAND"); // necessary for SPI-NAND (by Yanli He)

    //enable_clock(MT_CG_NFIECC_SW_CG, "NAND"); // not necessary in MT6571. ECC is enabled when MT_CG_NFI_SW_CG is enabled.
    //enable_clock(MT_CG_NFI2X_SW_CG, "NAND"); // not necessary for SPI-NAND (by Yanli He)
}

void nand_disable_clock(void)
{
    disable_clock(MT_CG_NFI_SW_CG, "NAND");
    disable_clock(MT_CG_SPINFI_SW_CG, "NAND");
	disable_clock(MT_CG_NFI_HCLK_SW_CG, "NAND"); // necessary for SPI-NAND (by Yanli He)

    //disable_clock(MT_CG_NFIECC_SW_CG, "NAND"); // not necessary in MT6571. ECC is disabled when MT_CG_NFI_SW_CG is disabled.
    //disable_clock(MT_CG_NFI2X_SW_CG, "NAND"); // not necessary for SPI-NAND (by Yanli He)
}

static struct nand_ecclayout nand_oob_16 = {
    .eccbytes = 8,
    .eccpos = {8, 9, 10, 11, 12, 13, 14, 15},
    .oobfree = {{1, 6}, {0, 0}}
};

struct nand_ecclayout nand_oob_64 = {
    .eccbytes = 32,
    .eccpos = {32, 33, 34, 35, 36, 37, 38, 39,
               40, 41, 42, 43, 44, 45, 46, 47,
               48, 49, 50, 51, 52, 53, 54, 55,
               56, 57, 58, 59, 60, 61, 62, 63},
    .oobfree = {{1, 7}, {9, 7}, {17, 7}, {25, 6}, {0, 0}}
};

struct nand_ecclayout nand_oob_128 = {
    .eccbytes = 64,
    .eccpos = {
               64, 65, 66, 67, 68, 69, 70, 71,
               72, 73, 74, 75, 76, 77, 78, 79,
               80, 81, 82, 83, 84, 85, 86, 86,
               88, 89, 90, 91, 92, 93, 94, 95,
               96, 97, 98, 99, 100, 101, 102, 103,
               104, 105, 106, 107, 108, 109, 110, 111,
               112, 113, 114, 115, 116, 117, 118, 119,
               120, 121, 122, 123, 124, 125, 126, 127},
    .oobfree = {{1, 7}, {9, 7}, {17, 7}, {25, 7}, {33, 7}, {41, 7}, {49, 7}, {57, 6}}
};

/*---------------------------------------------
 * Spare Format
 * NOTE: Each member specify (start location, bytes) of each sector's spare data
 *---------------------------------------------*/
u32 g_snand_k_spare_format_1[][2] =    {
                                             {4, 8}
                                            ,{20, 8}
                                            ,{36, 8}
                                            ,{52, 8}
                                            ,{0, 0}
                                        };

static suseconds_t Cal_timediff(struct timeval *end_time,struct timeval *start_time )
{
  struct timeval difference;

  difference.tv_sec =end_time->tv_sec -start_time->tv_sec ;
  difference.tv_usec=end_time->tv_usec-start_time->tv_usec;

  /* Using while instead of if below makes the code slightly more robust. */

  while(difference.tv_usec<0)
  {
    difference.tv_usec+=1000000;
    difference.tv_sec -=1;
  }

  return 1000000LL*difference.tv_sec+
                   difference.tv_usec;

} /* timeval_diff() */

#define RW_DRV_CFG          ((volatile P_U32)(0xF0015060))
#define DRV_CFG_MC_0_MASK   (0x00000FFF)

u8 NFI_DMA_status(void)
{
    return g_running_dma;
}

u32 NFI_DMA_address(void)
{
    return *(NFI_STRADDR_REG32);
}

EXPORT_SYMBOL(NFI_DMA_status);
EXPORT_SYMBOL(NFI_DMA_address);

u32 nand_virt_to_phys_add(u32 va)
{
    u32 pageOffset = (va & (PAGE_SIZE - 1));
    pgd_t *pgd;
    pmd_t *pmd;
    pte_t *pte;
    u32 pa;

    if (virt_addr_valid(va))
    {
        return __virt_to_phys(va);
    }

    if (NULL == current)
    {
        printk(KERN_ERR "[nand_virt_to_phys_add] ERROR ,current is NULL! \n");
        return 0;
    }

    if (NULL == current->mm)
    {
        printk(KERN_ERR "[nand_virt_to_phys_add] ERROR current->mm is NULL! tgid=0x%x, name=%s \n", current->tgid, current->comm);
        return 0;
    }

    pgd = pgd_offset(current->mm, va);  /* what is tsk->mm */
    if (pgd_none(*pgd) || pgd_bad(*pgd))
    {
        printk(KERN_ERR "[nand_virt_to_phys_add] ERROR, va=0x%x, pgd invalid! \n", va);
        return 0;
    }

    pmd = pmd_offset((pud_t *)pgd, va);
    if (pmd_none(*pmd) || pmd_bad(*pmd))
    {
        printk(KERN_ERR "[nand_virt_to_phys_add] ERROR, va=0x%x, pmd invalid! \n", va);
        return 0;
    }

    pte = pte_offset_map(pmd, va);
    if (pte_present(*pte))
    {
        pa = (pte_val(*pte) & (PAGE_MASK)) | pageOffset;
        return pa;
    }

    printk(KERN_ERR "[nand_virt_to_phys_add] ERROR va=0x%x, pte invalid! \n", va);
    return 0;
}

EXPORT_SYMBOL(nand_virt_to_phys_add);

bool mtk_snand_get_device_info(u8*id, snand_flashdev_info *devinfo)
{
    u32 i,m,n,mismatch;
    int target=-1;
    u8 target_id_len=0;

    for (i = 0; i < SNAND_CHIP_CNT; i++)
    {
		mismatch=0;

		for(m=0;m<gen_snand_FlashTable[i].id_length;m++)
		{
			if(id[m]!=gen_snand_FlashTable[i].id[m])
			{
				mismatch=1;
				break;
			}
		}

		if(mismatch == 0 && gen_snand_FlashTable[i].id_length > target_id_len)
		{
				target=i;
				target_id_len=gen_snand_FlashTable[i].id_length;
		}
    }

    if(target != -1)
    {
		MSG(INIT, "Recognize NAND: ID [");

		for(n=0;n<gen_snand_FlashTable[target].id_length;n++)
		{
			devinfo->id[n] = gen_snand_FlashTable[target].id[n];
			MSG(INIT, "%x ",devinfo->id[n]);
		}

		MSG(INIT, "], Device Name [%s], Page Size [%d]B Spare Size [%d]B Total Size [%d]MB\n",gen_snand_FlashTable[target].devicename,gen_snand_FlashTable[target].pagesize,gen_snand_FlashTable[target].sparesize,gen_snand_FlashTable[target].totalsize);
		devinfo->id_length=gen_snand_FlashTable[target].id_length;
		devinfo->blocksize = gen_snand_FlashTable[target].blocksize;    // KB
		devinfo->advancedmode = gen_snand_FlashTable[target].advancedmode;
		devinfo->spareformat = gen_snand_FlashTable[target].spareformat;

		// SW workaround for SNAND_ADV_READ_SPLIT
        if (0xC8 == devinfo->id[0] && 0xF4 == devinfo->id[1])
        {
            devinfo->advancedmode |= (SNAND_ADV_READ_SPLIT | SNAND_ADV_VENDOR_RESERVED_BLOCKS);
        }

		devinfo->pagesize = gen_snand_FlashTable[target].pagesize;
		devinfo->sparesize = gen_snand_FlashTable[target].sparesize;
		devinfo->totalsize = gen_snand_FlashTable[target].totalsize;

		memcpy(devinfo->devicename, gen_snand_FlashTable[target].devicename, sizeof(devinfo->devicename));

		devinfo->SNF_DLY_CTL1 = gen_snand_FlashTable[target].SNF_DLY_CTL1;
		devinfo->SNF_DLY_CTL2 = gen_snand_FlashTable[target].SNF_DLY_CTL2;
		devinfo->SNF_DLY_CTL3 = gen_snand_FlashTable[target].SNF_DLY_CTL3;
		devinfo->SNF_DLY_CTL4 = gen_snand_FlashTable[target].SNF_DLY_CTL4;
		devinfo->SNF_MISC_CTL = gen_snand_FlashTable[target].SNF_MISC_CTL;
		devinfo->SNF_DRIVING = gen_snand_FlashTable[target].SNF_DRIVING;

        // init read split boundary
        g_snand_rs_num_page = SNAND_RS_BOUNDARY_KB * 1024 / devinfo->pagesize;
        g_snand_k_spare_per_sector = devinfo->sparesize / (devinfo->pagesize / NAND_SECTOR_SIZE);

    	return 1;
	}
	else
	{
	    MSG(INIT, "Not Found NAND: ID [");
		for(n=0;n<SNAND_MAX_ID;n++){
			MSG(INIT, "%x ",id[n]);
		}
		MSG(INIT, "]\n");
        return 0;
	}
}
#ifdef DUMP_NATIVE_BACKTRACE
#define NFI_NATIVE_LOG_SD    "/sdcard/NFI_native_log_%s-%02d-%02d-%02d_%02d-%02d-%02d.log"
#define NFI_NATIVE_LOG_DATA "/data/NFI_native_log_%s-%02d-%02d-%02d_%02d-%02d-%02d.log"
static int nfi_flush_log(char *s)
{
    mm_segment_t old_fs;
    struct rtc_time tm;
    struct timeval tv = { 0 };
    struct file *filp = NULL;
    char name[256];
    unsigned int re = 0;
    int data_write = 0;

    do_gettimeofday(&tv);
    rtc_time_to_tm(tv.tv_sec, &tm);
    memset(name, 0, sizeof(name));
    sprintf(name, NFI_NATIVE_LOG_DATA, s, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

    old_fs = get_fs();
    set_fs(KERNEL_DS);
    filp = filp_open(name, O_WRONLY | O_CREAT, 0777);
    if (IS_ERR(filp))
    {
        printk("[NFI_flush_log]error create file in %s, IS_ERR:%ld, PTR_ERR:%ld\n", name, IS_ERR(filp), PTR_ERR(filp));
        memset(name, 0, sizeof(name));
        sprintf(name, NFI_NATIVE_LOG_SD, s, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
        filp = filp_open(name, O_WRONLY | O_CREAT, 0777);
        if (IS_ERR(filp))
        {
            printk("[NFI_flush_log]error create file in %s, IS_ERR:%ld, PTR_ERR:%ld\n", name, IS_ERR(filp), PTR_ERR(filp));
            set_fs(old_fs);
            return -1;
        }
    }
    printk("[NFI_flush_log]log file:%s\n", name);
    set_fs(old_fs);

    if (!(filp->f_op) || !(filp->f_op->write))
    {
        printk("[NFI_flush_log] No operation\n");
        re = -1;
        goto ClOSE_FILE;
    }

    DumpNativeInfo();
    old_fs = get_fs();
    set_fs(KERNEL_DS);
    data_write = vfs_write(filp, (char __user *)NativeInfo, strlen(NativeInfo), &filp->f_pos);
    if (!data_write)
    {
        printk("[nfi_flush_log] write fail\n");
        re = -1;
    }
    set_fs(old_fs);

  ClOSE_FILE:
    if (filp)
    {
        filp_close(filp, current->files);
        filp = NULL;
    }
    return re;
}
#endif

bool mtk_snand_is_vendor_reserved_blocks(u32 row_addr)
{
    u32 page_per_block = (devinfo.blocksize << 10) / devinfo.pagesize;
    u32 target_block = row_addr / page_per_block;

    if (devinfo.advancedmode & SNAND_ADV_VENDOR_RESERVED_BLOCKS)
    {
        if (target_block >= 2045 && target_block <= 2048)
        {
            return 1;
        }
    }

    return 0;
}

static void mtk_snand_wait_us(u32 us)
{
    udelay(us);
}

static void mtk_snand_dev_mac_enable(SNAND_Mode mode)
{
    u32 mac;

    mac = *(RW_SNAND_MAC_CTL);

    // SPI
    if (mode == SPI)
    {
        mac &= ~SNAND_MAC_SIO_SEL;   // Clear SIO_SEL to send command in SPI style
        mac |= SNAND_MAC_EN;         // Enable Macro Mode
    }
    // QPI
    else
    {
        /*
         * SFI V2: QPI_EN only effects direct read mode, and it is moved into DIRECT_CTL in V2
         *         There's no need to clear the bit again.
         */
        mac |= (SNAND_MAC_SIO_SEL | SNAND_MAC_EN);  // Set SIO_SEL to send command in QPI style, and enable Macro Mode
    }

    SNAND_WriteReg(RW_SNAND_MAC_CTL, mac);
}

/**
 * @brief This funciton triggers SFI to issue command to serial flash, wait SFI until ready.
 *
 * @remarks: !NOTE! This function must be used with mtk_snand_dev_mac_enable in pair!
 */
static void mtk_snand_dev_mac_trigger(void)
{
    u32 mac;

    mac = *(RW_SNAND_MAC_CTL);

    // Trigger SFI: Set TRIG and enable Macro mode
    mac |= (SNAND_TRIG | SNAND_MAC_EN);
    SNAND_WriteReg(RW_SNAND_MAC_CTL, mac);

    /*
     * Wait for SFI ready
     * Step 1. Wait for WIP_READY becoming 1 (WIP register is ready)
     */
    while (!(*(RW_SNAND_MAC_CTL) & SNAND_WIP_READY));

    /*
     * Step 2. Wait for WIP becoming 0 (Controller finishes command write process)
     */
    while ((*(RW_SNAND_MAC_CTL) & SNAND_WIP));


}

/**
 * @brief This funciton leaves Macro mode and enters Direct Read mode
 *
 * @remarks: !NOTE! This function must be used after mtk_snand_dev_mac_trigger
 */
static void mtk_snand_dev_mac_leave(void)
{
    u32 mac;

    // clear SF_TRIG and leave mac mode
    mac = *(RW_SNAND_MAC_CTL);

    /*
     * Clear following bits
     * SF_TRIG: Confirm the macro command sequence is completed
     * SNAND_MAC_EN: Leaves macro mode, and enters direct read mode
     * SNAND_MAC_SIO_SEL: Always reset quad macro control bit at the end
     */
    mac &= ~(SNAND_TRIG | SNAND_MAC_EN | SNAND_MAC_SIO_SEL);
    SNAND_WriteReg(RW_SNAND_MAC_CTL, mac);
}

static void mtk_snand_dev_mac_op(SNAND_Mode mode)
{
    #ifdef _SNAND_DEBUG
    if (1 == g_snand_nfi_bypass_enabled)
    {
        printk("[K-SNAND][%s] ERROR! NFI BYPASS must be disabled during SNF's MAC operations!\n", __FUNCTION__);
        //dump_stack();
        while (1);
    }
    #endif

    mtk_snand_dev_mac_enable(mode);
    mtk_snand_dev_mac_trigger();
    mtk_snand_dev_mac_leave();
}

static void mtk_snand_dev_command_ext(SNAND_Mode mode, const U8 cmd[], U8 data[], const u32 outl, const u32 inl)
{
    u32   tmp;
    u32   i, j;
    P_U8  p_data, p_tmp;

    p_tmp = (P_U8)(&tmp);

    // Moving commands into SFI GPRAM
    for (i = 0, p_data = ((P_U8)RW_SNAND_GPRAM_DATA); i < outl; p_data += 4)
    {
        // Using 4 bytes aligned copy, by moving the data into the temp buffer and then write to GPRAM
        for (j = 0, tmp = 0; i < outl && j < 4; i++, j++)
        {
            p_tmp[j] = cmd[i];
        }

        SNAND_WriteReg(p_data, tmp);
    }

    SNAND_WriteReg(RW_SNAND_MAC_OUTL, outl);
    SNAND_WriteReg(RW_SNAND_MAC_INL, inl);
    mtk_snand_dev_mac_op(mode);

    // for NULL data, this loop will be skipped
    for (i = 0, p_data = ((P_U8)RW_SNAND_GPRAM_DATA + outl); i < inl; ++i, ++data, ++p_data)
    {
        *data = DRV_Reg8(p_data);
    }

    return;
}

static void mtk_snand_dev_command(const u32 cmd, u8 outlen)
{
    SNAND_WriteReg(RW_SNAND_GPRAM_DATA, cmd);
    SNAND_WriteReg(RW_SNAND_MAC_OUTL, outlen);
    SNAND_WriteReg(RW_SNAND_MAC_INL , 0);
    mtk_snand_dev_mac_op(SPI);

    return;
}

static void mtk_snand_reset_dev()
{
    u8 cmd = SNAND_CMD_SW_RESET;

    // issue SW RESET command to device
    mtk_snand_dev_command_ext(SPI, &cmd, NULL, 1, 0);

    // wait for awhile, then polling status register (required by spec)
    mtk_snand_wait_us(SNAND_DEV_RESET_LATENCY_US);

    *RW_SNAND_GPRAM_DATA = (SNAND_CMD_GET_FEATURES | (SNAND_CMD_FEATURES_STATUS << 8));
    *RW_SNAND_MAC_OUTL = 2;
    *RW_SNAND_MAC_INL = 1;

    // polling status register

    for (;;)
    {
        mtk_snand_dev_mac_op(SPI);

        cmd = DRV_Reg8(((P_U8)RW_SNAND_GPRAM_DATA + 2));

        if (0 == (cmd & SNAND_STATUS_OIP))
        {
            break;
        }
    }
}

static u32 mtk_snand_reverse_byte_order(u32 num)
{
   u32 ret = 0;

   ret |= ((num >> 24) & 0x000000FF);
   ret |= ((num >> 8)  & 0x0000FF00);
   ret |= ((num << 8)  & 0x00FF0000);
   ret |= ((num << 24) & 0xFF000000);

   return ret;
}

static u32 mtk_snand_gen_c1a3(const u32 cmd, const u32 address)
{
    return ((mtk_snand_reverse_byte_order(address) & 0xFFFFFF00) | (cmd & 0xFF));
}

static void mtk_snand_dev_enable_spiq(bool enable)
{
    u8   regval;
    u32  cmd;

    // read QE in status register
    cmd = SNAND_CMD_GET_FEATURES | (SNAND_CMD_FEATURES_OTP << 8);
    SNAND_WriteReg(RW_SNAND_GPRAM_DATA, cmd);
    SNAND_WriteReg(RW_SNAND_MAC_OUTL, 2);
    SNAND_WriteReg(RW_SNAND_MAC_INL , 1);

    mtk_snand_dev_mac_op(SPI);

    regval = DRV_Reg8(((volatile u8 *)RW_SNAND_GPRAM_DATA + 2));

    if (0 == enable)    // disable x4
    {
        if ((regval & SNAND_OTP_QE) == 0)
        {
            return;
        }
        else
        {
            regval = regval & ~SNAND_OTP_QE;
        }
    }
    else    // enable x4
    {
        if ((regval & SNAND_OTP_QE) == 1)
        {
            return;
        }
        else
        {
            regval = regval | SNAND_OTP_QE;
        }
    }

    // if goes here, it means QE needs to be set as new different value

    // write status register
    cmd = SNAND_CMD_SET_FEATURES | (SNAND_CMD_FEATURES_OTP << 8) | (regval << 16);
    SNAND_WriteReg(RW_SNAND_GPRAM_DATA, cmd);
    SNAND_WriteReg(RW_SNAND_MAC_OUTL, 3);
    SNAND_WriteReg(RW_SNAND_MAC_INL , 0);

    mtk_snand_dev_mac_op(SPI);
}


/******************************************************************************
 * mtk_nand_irq_handler
 *
 * DESCRIPTION:
 *   NAND interrupt handler!
 *
 * PARAMETERS:
 *   int irq
 *   void *dev_id
 *
 * RETURNS:
 *   IRQ_HANDLED : Successfully handle the IRQ
 *
 * NOTES:
 *   None
 *
 ******************************************************************************/
/* Modified for TCM used */
static irqreturn_t mtk_nand_irq_handler(int irqno, void *dev_id)
{
    u16 u16IntStatus = *(NFI_INTR_REG16);
    (void)irqno;

    if (SNAND_NFI_CUST_READING == g_snand_dev_status)
    {
        if (u16IntStatus & (u16) INTR_AHB_DONE_EN)
        {
            complete(&g_comp_AHB_Done);
        }
    }
    else if (SNAND_NFI_AUTO_ERASING == g_snand_dev_status)
    {
        if (u16IntStatus & (u16) INTR_AUTO_BLKER_INTR_EN)
        {
            complete(&g_comp_AHB_Done);
        }
    }
    else if (SNAND_DEV_PROG_EXECUTING == g_snand_dev_status)
    {
        if (u16IntStatus & (u16) INTR_AHB_DONE_EN)
        {
            complete(&g_comp_AHB_Done);
        }
    }
    else
    {
        if (u16IntStatus)
        {
            complete(&g_comp_AHB_Done);
        }
    }

    return IRQ_HANDLED;
}

// Read Split related APIs
static bool mtk_snand_rs_if_require_split(void) // must be executed after snand_rs_reconfig_nfiecc()
{
    if (devinfo.advancedmode & SNAND_ADV_READ_SPLIT)
    {
        if (g_snand_rs_cur_part != 0)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        return 0;
    }
}

void mtk_snand_dev_ecc_control(u8 enable)
{
    u32 cmd;
    u8  otp;
    u8  otp_new;

    // read original otp settings

    cmd = SNAND_CMD_GET_FEATURES | (SNAND_CMD_FEATURES_OTP << 8);
    SNAND_WriteReg(RW_SNAND_GPRAM_DATA, cmd);
    SNAND_WriteReg(RW_SNAND_MAC_OUTL, 2);
    SNAND_WriteReg(RW_SNAND_MAC_INL , 1);

    mtk_snand_dev_mac_op(SPI);

    otp = DRV_Reg8(((P_U8)RW_SNAND_GPRAM_DATA + 2));

    if (enable == 1)
    {
        otp_new = otp | SNAND_OTP_ECC_ENABLE;
    }
    else
    {
        otp_new = otp & ~SNAND_OTP_ECC_ENABLE;
    }

    if (otp != otp_new)
    {
        // write enable

        mtk_snand_dev_command(SNAND_CMD_WRITE_ENABLE, 1);


        // set features
        cmd = SNAND_CMD_SET_FEATURES | (SNAND_CMD_FEATURES_OTP << 8) | (otp_new << 16);

        mtk_snand_dev_command(cmd, 3);
    }
}

static void mtk_snand_rs_reconfig_nfiecc(u32 row_addr)
{
    // 1. only decode part should be re-configured
    // 2. only re-configure essential part (fixed register will not be re-configured)

    u16 reg16;
    u32 ecc_bit_cfg = 0;
    u32 u4DECODESize;
    struct mtd_info * mtd = &host->mtd;

    if (0 == (devinfo.advancedmode & SNAND_ADV_READ_SPLIT))
    {
        return;
    }

    if (row_addr < g_snand_rs_num_page)
    {
        if (g_snand_rs_cur_part != 0)
        {
            g_snand_rs_ecc_bit = SNAND_RS_ECC_BIT_PART0;

            reg16 = *(NFI_PAGEFMT_REG16);
            reg16 &= ~PAGEFMT_SPARE_MASK;
            reg16 |= SNAND_RS_SPARE_PER_SECTOR_PART0_NFI;
            SNAND_WriteReg(NFI_PAGEFMT_REG16, reg16);

            g_snand_k_spare_per_sector = SNAND_RS_SPARE_PER_SECTOR_PART0_VAL;

            g_snand_rs_cur_part = 0;
        }
        else
        {
            return;
        }
    }
    else
    {
        if (g_snand_rs_cur_part != 1)
        {
            g_snand_rs_ecc_bit = g_snand_rs_ecc_bit_second_part;

            reg16 = *(NFI_PAGEFMT_REG16);
            reg16 &= ~PAGEFMT_SPARE_MASK;
            reg16 |= g_snand_rs_spare_per_sector_second_part_nfi;
            SNAND_WriteReg(NFI_PAGEFMT_REG16, reg16);

            g_snand_k_spare_per_sector = mtd->oobsize / (mtd->writesize / NAND_SECTOR_SIZE);

            g_snand_rs_cur_part = 1;
        }
        else
        {
            return;
        }
    }

    if (0 == g_snand_rs_ecc_bit)
    {
        mtk_snand_dev_ecc_control(1);   // enable device ECC

        return; // Use device in this partition. ECC re-configuration is not required
    }
    else
    {
        mtk_snand_dev_ecc_control(0);   // disable device ECC
    }

    switch(g_snand_rs_ecc_bit)
    {
    	case 4:
    		ecc_bit_cfg = ECC_CNFG_ECC4;
    		break;
    	case 8:
    		ecc_bit_cfg = ECC_CNFG_ECC8;
    		break;
    	case 10:
    		ecc_bit_cfg = ECC_CNFG_ECC10;
    		break;
    	case 12:
    		ecc_bit_cfg = ECC_CNFG_ECC12;
    		break;
    	default:
    		break;
    }
    SNAND_WriteReg(ECC_DECCON_REG16, DEC_DE);
    do
    {;
    }
    while (!*(ECC_DECIDLE_REG16));

    u4DECODESize = ((NAND_SECTOR_SIZE + OOB_AVAI_PER_SECTOR) << 3) + g_snand_rs_ecc_bit * 14;

    /* configure ECC decoder && encoder */
    SNAND_WriteReg(ECC_DECCNFG_REG32, DEC_CNFG_CORRECT | ecc_bit_cfg | DEC_CNFG_NFI | DEC_CNFG_EMPTY_EN | (u4DECODESize << DEC_CNFG_CODE_SHIFT));
}


/******************************************************************************
 * mtk_snand_ecc_config
 *
 * DESCRIPTION:
 *   Configure HW ECC!
 *
 * PARAMETERS:
 *   struct mtk_nand_host_hw *hw
 *
 * RETURNS:
 *   None
 *
 * NOTES:
 *   None
 *
 ******************************************************************************/
static void mtk_snand_ecc_config(struct mtk_nand_host_hw *hw, u32 ecc_bit)
{
    u32 u4ENCODESize;
    u32 u4DECODESize;
    u32 ecc_bit_cfg = ECC_CNFG_ECC4;

    switch(ecc_bit){
  	case 4:
  		ecc_bit_cfg = ECC_CNFG_ECC4;
  		break;
  	case 8:
  		ecc_bit_cfg = ECC_CNFG_ECC8;
  		break;
  	case 10:
  		ecc_bit_cfg = ECC_CNFG_ECC10;
  		break;
  	case 12:
  		ecc_bit_cfg = ECC_CNFG_ECC12;
  		break;
        default:
  			break;

    }
    SNAND_WriteReg(ECC_DECCON_REG16, DEC_DE);
    do
    {;
    }
    while (!*(ECC_DECIDLE_REG16));

    SNAND_WriteReg(ECC_ENCCON_REG16, ENC_DE);
    do
    {;
    }
    while (!*(ECC_ENCIDLE_REG32));

    /* setup FDM register base */
    //SNAND_WriteReg(ECC_FDMADDR_REG32, NFI_FDM0L_REG32);

    /* Sector + FDM */
    u4ENCODESize = (hw->nand_sec_size + 8) << 3;
    /* Sector + FDM + YAFFS2 meta data bits */
    u4DECODESize = ((hw->nand_sec_size + 8) << 3) + ecc_bit * 14;

    /* configure ECC decoder && encoder */
    SNAND_WriteReg(ECC_DECCNFG_REG32, ecc_bit_cfg | DEC_CNFG_NFI | DEC_CNFG_EMPTY_EN | (u4DECODESize << DEC_CNFG_CODE_SHIFT));

    SNAND_WriteReg(ECC_ENCCNFG_REG32, ecc_bit_cfg | ENC_CNFG_NFI | (u4ENCODESize << ENC_CNFG_MSG_SHIFT));
#ifndef MANUAL_CORRECT
    SNAND_REG_SET_BITS(ECC_DECCNFG_REG32, DEC_CNFG_CORRECT);
#else
    SNAND_REG_SET_BITS(ECC_DECCNFG_REG32, DEC_CNFG_EL);
#endif
}

/******************************************************************************
 * mtk_snand_ecc_decode_start
 *
 * DESCRIPTION:
 *   HW ECC Decode Start !
 *
 * PARAMETERS:
 *   None
 *
 * RETURNS:
 *   None
 *
 * NOTES:
 *   None
 *
 ******************************************************************************/
static void mtk_snand_ecc_decode_start(void)
{
    /* wait for device returning idle */
    while (!(*(ECC_DECIDLE_REG16) & DEC_IDLE)) ;
    SNAND_WriteReg(ECC_DECCON_REG16, DEC_EN);
}

/******************************************************************************
 * mtk_snand_ecc_decode_end
 *
 * DESCRIPTION:
 *   HW ECC Decode End !
 *
 * PARAMETERS:
 *   None
 *
 * RETURNS:
 *   None
 *
 * NOTES:
 *   None
 *
 ******************************************************************************/
static void mtk_snand_ecc_decode_end(void)
{
    u32 timeout = 0xFFFF;

    /* wait for device returning idle */
    while (!(*(ECC_DECIDLE_REG16) & DEC_IDLE))
    {
        timeout--;

        if (timeout == 0)   // Stanley Chu (need check timeout value again)
        {
            break;
        }
    }

    SNAND_WriteReg(ECC_DECCON_REG16, DEC_DE);
}

/******************************************************************************
 * mtk_snand_ecc_encode_start
 *
 * DESCRIPTION:
 *   HW ECC Encode Start !
 *
 * PARAMETERS:
 *   None
 *
 * RETURNS:
 *   None
 *
 * NOTES:
 *   None
 *
 ******************************************************************************/
static void mtk_snand_ecc_encode_start(void)
{
    /* wait for device returning idle */
    while (!(*(ECC_ENCIDLE_REG32) & ENC_IDLE)) ;
    mb();
    SNAND_WriteReg(ECC_ENCCON_REG16, ENC_EN);
}

/******************************************************************************
 * mtk_snand_ecc_encode_end
 *
 * DESCRIPTION:
 *   HW ECC Encode End !
 *
 * PARAMETERS:
 *   None
 *
 * RETURNS:
 *   None
 *
 * NOTES:
 *   None
 *
 ******************************************************************************/
static void mtk_snand_ecc_encode_end(void)
{
    /* wait for device returning idle */
    while (!(*(ECC_ENCIDLE_REG32) & ENC_IDLE)) ;
    mb();
    SNAND_WriteReg(ECC_ENCCON_REG16, ENC_DE);
}

#if 0
static bool is_empty_page(u8 * spare_buf, u32 sec_num){
	u32 i=0;
	bool is_empty=1;
#if 0
	for(i=0;i<sec_num*8;i++){
		if(spare_buf[i]!=0xFF){
			is_empty=0;
			break;
		}
	}
	printk("\n");
#else
	for(i=0;i<OOB_INDEX_SIZE;i++){
		xlog_printk(ANDROID_LOG_INFO,"NFI", "flag byte: %x ",spare_buf[OOB_INDEX_OFFSET+i] );
		if(spare_buf[OOB_INDEX_OFFSET+i] !=0xFF){
			is_empty=0;
			break;
		}
	}
#endif
	xlog_printk(ANDROID_LOG_INFO,"NFI", "This page is %s!\n",is_empty?"empty":"not empty");
	return is_empty;
}

static bool return_fake_buf(u8 * data_buf, u32 page_size, u32 sec_num,u32 u4PageAddr){
	u32 i=0,j=0;
	u32 sec_zero_count=0;
	u8 t=0;
	u8 *p=data_buf;
	bool ret=1;
	for(j=0;j<sec_num;j++){
		p=data_buf+j*512;
		sec_zero_count=0;
		for(i=0;i<512;i++){
			t=p[i];
			t=~t;
			t=((t&0xaa)>>1) + (t&0x55);
			t=((t&0xcc)>>2)+(t&0x33);
			t=((t&0xf0f0)>>4)+(t&0x0f0f);
			sec_zero_count+=t;
			if(t>0){
				xlog_printk(ANDROID_LOG_INFO,"NFI", "there is %d bit filp at sector(%d): %d in empty page \n ",t,j,i);
			}
		}
		if(sec_zero_count > 2){
			xlog_printk(ANDROID_LOG_ERROR,"NFI","too many bit filp=%d @ page addr=0x%x, we can not return fake buf\n",sec_zero_count,u4PageAddr);
			ret=0;
		}
	}
	return ret;
}
#endif

/******************************************************************************
 * mtk_snand_check_bch_error
 *
 * DESCRIPTION:
 *   Check BCH error or not !
 *
 * PARAMETERS:
 *   struct mtd_info *mtd
 *	 u8* pDataBuf
 *	 u32 u4SecIndex
 *	 u32 u4PageAddr
 *
 * RETURNS:
 *   None
 *
 * NOTES:
 *   None
 *
 ******************************************************************************/
void mtk_snand_nfi_enable_bypass(u8 enable)
{
    if (1 == enable)
    {
        *NFI_DEBUG_CON1_REG16 |= DEBUG_CON1_BYPASS_MASTER_EN;
        *ECC_BYPASS_REG32 |= ECC_BYPASS;

        #ifdef _SNAND_DEBUG
        g_snand_nfi_bypass_enabled = 1;
        #endif
    }
    else
    {
        *NFI_DEBUG_CON1_REG16 &= ~DEBUG_CON1_BYPASS_MASTER_EN;
        *ECC_BYPASS_REG32 &= ~ECC_BYPASS;

        #ifdef _SNAND_DEBUG
        g_snand_nfi_bypass_enabled = 0;
        #endif
    }
}

u32 mtk_snand_nfi_if_empty_page(void)
{
   u32 reg_val = 0;

   reg_val = *(NFI_STA_REG32);

   if (0 != (reg_val & STA_READ_EMPTY)) // empty page
   {
      reg_val = 1;
   }
   else
   {
      reg_val = 0;
   }

   mtk_snand_reset_con();

   return reg_val;
}

static bool mtk_snand_check_bch_error(struct mtd_info *mtd, u8 * pDataBuf,u8 * spareBuf,u32 u4SecIndex, u32 u4PageAddr)
{
    bool ret = 1;
    u16 u2SectorDoneMask = 1 << u4SecIndex;
    u32 u4ErrorNumDebug0,u4ErrorNumDebug1, i, u4ErrNum;
    u32 timeout = 0xFFFF;
    u32 correct_count = 0;
    u32 page_size=(u4SecIndex+1)*512;
    u32 sec_num=u4SecIndex+1;
	u32 failed_sec=0;

#ifdef MANUAL_CORRECT
    u32 au4ErrBitLoc[6];
    u32 u4ErrByteLoc, u4BitOffset;
    u32 u4ErrBitLoc1th, u4ErrBitLoc2nd;
#endif

    while (0 == (u2SectorDoneMask & *(ECC_DECDONE_REG16)))
    {
        timeout--;
        if (0 == timeout)
        {
            return 0;
        }
    }
#ifndef MANUAL_CORRECT
    u4ErrorNumDebug0 = *(ECC_DECENUM0_REG32);
    u4ErrorNumDebug1 = *(ECC_DECENUM1_REG32);

    if (0 != (u4ErrorNumDebug0 & 0x3F3F3F3F) || 0 != (u4ErrorNumDebug1 & 0x3F3F3F3F))
    {
        failed_sec = 0;

        for (i = 0; i <= u4SecIndex; ++i)
        {
            if (i < 4)
            {
                u4ErrNum = *(ECC_DECENUM0_REG32) >> (i * 8);
            }
            else
            {
                u4ErrNum = *(ECC_DECENUM1_REG32) >> ((i - 4) * 8);
            }

            u4ErrNum &= 0x3F;

            if (0x3F == u4ErrNum)
            {
                // check if all data are empty (MT6571 does not support DEC_EMPTY_EN)

                failed_sec++;

                #if 1

                #else   // manual check empty

                for (j = NAND_SECTOR_SIZE * i; j < NAND_SECTOR_SIZE * (i + 1); j++)
                {
                    if (pDataBuf[j] != 0xFF)
                    {
                        err_confirmed = 1;

                        break;
                    }
                }

                if (spareBuf)
                {
                    for (j = SNAND_FDM_DATA_SIZE_PER_SECTOR * i; j < SNAND_FDM_DATA_SIZE_PER_SECTOR * (i + 1); j++)
                    {
                        if (spareBuf[i] != 0xFF)
                        {
                            err_confirmed = 1;

                            break;
                        }
                    }
                }
                #endif
            }
            else
            {
				if (u4ErrNum)
                {
					correct_count += u4ErrNum;
                    xlog_printk(ANDROID_LOG_INFO,"K-SNAND"," ECC-C, #err:%d, PA=%d, S=%d\n", u4ErrNum, u4PageAddr, i);
				}
            }
        }

        if (failed_sec != 0)
        {
            if ((failed_sec == (u4SecIndex + 1)) &&
                (1 == mtk_snand_nfi_if_empty_page()))   // all sectors are ECC-U, however it's an empty page
            {
                // false alarm

                #ifdef _SNAND_DEBUG
                //printk("[K-SNAND] false ECC-U alarm in PA=%d\n", u4PageAddr);
                #endif

                /*
                 * False ECC-U alarm (Empty page || less than 1 bit-flipping)
                 *
                 * Report fake buffer contents which should be empty to user.
                 * Reset return value to "NO ERROR"
                 */

                ret = 1;
                failed_sec = 0;

                memset(pDataBuf, 0xff, page_size);
                memset(spareBuf, 0xff, sec_num * 8);

                //xlog_printk(ANDROID_LOG_WARN,"K-SNAND", "Fake ECC-U, PA=%d, u4SecIndex=%d\n", u4PageAddr, u4SecIndex);
            }
            else
            {
                ret = 0;
                xlog_printk(ANDROID_LOG_WARN,"K-SNAND", "ECC-U, PA=%d\n", u4PageAddr);
            }
        }

        #if 0
        if (ret == 0)
        {
		    if (is_empty_page(spareBuf,sec_num) && return_fake_buf(pDataBuf,page_size,sec_num,u4PageAddr))
		    {
    			ret=1;
    			xlog_printk(ANDROID_LOG_INFO,"K-SNAND", "empty page have few filped bit(s) , fake buffer returned\n");
    			memset(pDataBuf,0xff,page_size);
    			memset(spareBuf,0xff,sec_num*8);
    			failed_sec=0;
		    }
		    else
    		{
    		    /*
    		     * ECC-U and (Not an empty page || more than 1 bit-flipping)
    		     *
    		     * Report fake buffer contents with all 0xFF to user. Thus user (e.g., UBIFS) will not touch physical contents
    		     * to avoid data corruption for bit-flipping case.
    		     *
    		     * PS. Keep the return value (ECC-U)
    		     */

    		    memset(pDataBuf,0xff,page_size);
    			memset(spareBuf,0xff,sec_num*8);
    		}
    	}
    	#endif

	    mtd->ecc_stats.failed += failed_sec;

        if (correct_count > 2 && ret)
        {
            mtd->ecc_stats.corrected++;
        }
        else
        {
            //xlog_printk(ANDROID_LOG_INFO,"NFI",  "Less than 2 bit error, ignore\n");
        }
    }
#else
    /* We will manually correct the error bits in the last sector, not all the sectors of the page! */
    memset(au4ErrBitLoc, 0x0, sizeof(au4ErrBitLoc));
    u4ErrorNumDebug = *(ECC_DECENUM_REG32);
    u4ErrNum = *(ECC_DECENUM_REG32) >> (u4SecIndex << 2);
    u4ErrNum &= 0xF;

    if (u4ErrNum)
    {
        if (0xF == u4ErrNum)
        {
            mtd->ecc_stats.failed++;
            ret = 0;
            //printk(KERN_ERR"UnCorrectable at PageAddr=%d\n", u4PageAddr);
        } else
        {
            for (i = 0; i < ((u4ErrNum + 1) >> 1); ++i)
            {
                au4ErrBitLoc[i] = *(ECC_DECEL0_REG32 + i);
                u4ErrBitLoc1th = au4ErrBitLoc[i] & 0x1FFF;

                if (u4ErrBitLoc1th < 0x1000)
                {
                    u4ErrByteLoc = u4ErrBitLoc1th / 8;
                    u4BitOffset = u4ErrBitLoc1th % 8;
                    pDataBuf[u4ErrByteLoc] = pDataBuf[u4ErrByteLoc] ^ (1 << u4BitOffset);
                    mtd->ecc_stats.corrected++;
                } else
                {
                    mtd->ecc_stats.failed++;
                    //printk(KERN_ERR"UnCorrectable ErrLoc=%d\n", au4ErrBitLoc[i]);
                }
                u4ErrBitLoc2nd = (au4ErrBitLoc[i] >> 16) & 0x1FFF;
                if (0 != u4ErrBitLoc2nd)
                {
                    if (u4ErrBitLoc2nd < 0x1000)
                    {
                        u4ErrByteLoc = u4ErrBitLoc2nd / 8;
                        u4BitOffset = u4ErrBitLoc2nd % 8;
                        pDataBuf[u4ErrByteLoc] = pDataBuf[u4ErrByteLoc] ^ (1 << u4BitOffset);
                        mtd->ecc_stats.corrected++;
                    } else
                    {
                        mtd->ecc_stats.failed++;
                        //printk(KERN_ERR"UnCorrectable High ErrLoc=%d\n", au4ErrBitLoc[i]);
                    }
                }
            }
        }
        if (0 == (*(ECC_DECFER_REG16) & (1 << u4SecIndex)))
        {
            ret = 0;
        }
    }
#endif
    return ret;
}

/******************************************************************************
 * mtk_snand_RFIFOValidSize
 *
 * DESCRIPTION:
 *   Check the Read FIFO data bytes !
 *
 * PARAMETERS:
 *   u16 u2Size
 *
 * RETURNS:
 *   None
 *
 * NOTES:
 *   None
 *
 ******************************************************************************/
static bool mtk_snand_RFIFOValidSize(u16 u2Size)
{
    u32 timeout = 0xFFFF;
    while (FIFO_RD_REMAIN(*(NFI_FIFOSTA_REG16)) < u2Size)
    {
        timeout--;
        if (0 == timeout)
        {
            return 0;
        }
    }
    return 1;
}

/******************************************************************************
 * mtk_snand_WFIFOValidSize
 *
 * DESCRIPTION:
 *   Check the Write FIFO data bytes !
 *
 * PARAMETERS:
 *   u16 u2Size
 *
 * RETURNS:
 *   None
 *
 * NOTES:
 *   None
 *
 ******************************************************************************/
static bool mtk_snand_WFIFOValidSize(u16 u2Size)
{
    u32 timeout = 0xFFFF;
    while (FIFO_WR_REMAIN(*(NFI_FIFOSTA_REG16)) > u2Size)
    {
        timeout--;
        if (0 == timeout)
        {
            return 0;
        }
    }
    return 1;
}

/******************************************************************************
 * mtk_snand_status_ready
 *
 * DESCRIPTION:
 *   Indicate the NAND device is ready or not !
 *
 * PARAMETERS:
 *   u32 u4Status
 *
 * RETURNS:
 *   None
 *
 * NOTES:
 *   None
 *
 ******************************************************************************/
static bool mtk_snand_status_ready(u32 u4Status)
{
    u32 timeout = 0xFFFF;

	u4Status &= ~STA_NAND_BUSY;

    while ((*(NFI_STA_REG32) & u4Status) != 0)
    {
        timeout--;

        if (0 == timeout)
        {
            return 0;
        }
    }

    return 1;
}

/******************************************************************************
 * mtk_snand_reset_con
 *
 * DESCRIPTION:
 *   Reset the NAND device hardware component !
 *
 * PARAMETERS:
 *   struct mtk_nand_host *host (Initial setting data)
 *
 * RETURNS:
 *   None
 *
 * NOTES:
 *   None
 *
 ******************************************************************************/
static bool mtk_snand_reset_con(void)
{
    u32 reg32;

    // HW recommended reset flow
    int timeout = 0xFFFF;

    // part 1. SNF

    reg32 = *RW_SNAND_MISC_CTL;
    reg32 |= SNAND_SW_RST;
    SNAND_WriteReg(RW_SNAND_MISC_CTL, reg32);
    reg32 &= ~SNAND_SW_RST;
    SNAND_WriteReg(RW_SNAND_MISC_CTL, reg32);

    // part 2. NFI

    for (;;)
    {
        if (0 == (*(NFI_MASTERSTA_REG16) & MASTERSTA_MASK))
        {
            break;
        }
    }

    mb();

    SNAND_WriteReg(NFI_CON_REG16, CON_FIFO_FLUSH | CON_NFI_RST);

    for (;;)
    {
        if (0 == (*(NFI_STA_REG32) & (STA_NFI_FSM_MASK | STA_NAND_FSM_MASK)))
        {
            break;
        }

        timeout--;

        if (!timeout)
        {
            printk("[%s] NFI_MASTERSTA_REG16 timeout!!\n", __FUNCTION__);
        }
    }

    /* issue reset operation */

    mb();

    return mtk_snand_status_ready(STA_NFI_FSM_MASK | STA_NAND_BUSY) && mtk_snand_RFIFOValidSize(0) && mtk_snand_WFIFOValidSize(0);
}

/******************************************************************************
 * mtk_snand_set_mode
 *
 * DESCRIPTION:
 *    Set the oepration mode !
 *
 * PARAMETERS:
 *   u16 u2OpMode (read/write)
 *
 * RETURNS:
 *   None
 *
 * NOTES:
 *   None
 *
 ******************************************************************************/
static void mtk_snand_set_mode(u16 u2OpMode)
{
    u16 u2Mode = *(NFI_CNFG_REG16);
    u2Mode &= ~CNFG_OP_MODE_MASK;
    u2Mode |= u2OpMode;
    SNAND_WriteReg(NFI_CNFG_REG16, u2Mode);
}

/******************************************************************************
 * mtk_snand_set_autoformat
 *
 * DESCRIPTION:
 *    Enable/Disable hardware autoformat !
 *
 * PARAMETERS:
 *   bool bEnable (Enable/Disable)
 *
 * RETURNS:
 *   None
 *
 * NOTES:
 *   None
 *
 ******************************************************************************/
static void mtk_snand_set_autoformat(bool bEnable)
{
    if (bEnable)
    {
        SNAND_REG_SET_BITS(NFI_CNFG_REG16, CNFG_AUTO_FMT_EN);
    }
    else
    {
        SNAND_REG_CLN_BITS(NFI_CNFG_REG16, CNFG_AUTO_FMT_EN);
    }
}

/******************************************************************************
 * mtk_snand_configure_fdm
 *
 * DESCRIPTION:
 *   Configure the FDM data size !
 *
 * PARAMETERS:
 *   u16 u2FDMSize
 *
 * RETURNS:
 *   None
 *
 * NOTES:
 *   None
 *
 ******************************************************************************/
static void mtk_snand_configure_fdm(u16 u2FDMSize)
{
    SNAND_REG_CLN_BITS(NFI_PAGEFMT_REG16, PAGEFMT_FDM_MASK | PAGEFMT_FDM_ECC_MASK);
    SNAND_REG_SET_BITS(NFI_PAGEFMT_REG16, u2FDMSize << PAGEFMT_FDM_SHIFT);
    SNAND_REG_SET_BITS(NFI_PAGEFMT_REG16, u2FDMSize << PAGEFMT_FDM_ECC_SHIFT);
}

/******************************************************************************
 * mtk_snand_check_RW_count
 *
 * DESCRIPTION:
 *    Check the RW how many sectors !
 *
 * PARAMETERS:
 *   u16 u2WriteSize
 *
 * RETURNS:
 *   None
 *
 * NOTES:
 *   None
 *
 ******************************************************************************/
static bool mtk_snand_check_RW_count(u16 u2WriteSize)
{
    u32 timeout = 0xFFFF;
    u16 u2SecNum = u2WriteSize >> 9;

    while (ADDRCNTR_CNTR(*(NFI_ADDRCNTR_REG16)) < u2SecNum)
    {
        timeout--;
        if (0 == timeout)
        {
            printk(KERN_INFO "[%s] timeout\n", __FUNCTION__);
            return 0;
        }
    }
    return 1;
}

static bool mtk_snand_ready_for_read_sector_custom(struct nand_chip *nand, u32 u4RowAddr, u32 u4ColAddr)
{
    /* Reset NFI HW internal state machine and flush NFI in/out FIFO */
    u8  ret = 1;
    u32 cmd, reg;

    if (!mtk_snand_reset_con())
    {
        ret = 0;
        goto cleanup;
    }

    // 1. Read page to cache

    cmd = mtk_snand_gen_c1a3(SNAND_CMD_PAGE_READ, u4RowAddr); // PAGE_READ command + 3-byte address

    SNAND_WriteReg(RW_SNAND_GPRAM_DATA, cmd);
    SNAND_WriteReg(RW_SNAND_MAC_OUTL, 1 + 3);
    SNAND_WriteReg(RW_SNAND_MAC_INL , 0);

    mtk_snand_dev_mac_op(SPI);

    // 2. Get features (status polling)

    cmd = SNAND_CMD_GET_FEATURES | (SNAND_CMD_FEATURES_STATUS << 8);

    SNAND_WriteReg(RW_SNAND_GPRAM_DATA, cmd);
    SNAND_WriteReg(RW_SNAND_MAC_OUTL, 2);
    SNAND_WriteReg(RW_SNAND_MAC_INL , 1);

    for (;;)
    {
        mtk_snand_dev_mac_op(SPI);

        cmd = DRV_Reg8(((P_U8)RW_SNAND_GPRAM_DATA + 2));

        if ((cmd & SNAND_STATUS_OIP) == 0)
        {
            // use MTK ECC, not device ECC
            //if (SNAND_STATUS_TOO_MANY_ERROR_BITS == (cmd & SNAND_STATUS_ECC_STATUS_MASK) )
            //{
            //   bRet = 0;
            //}

            break;
        }
    }

    //------ SNF Part ------

    // set PAGE READ command & address
    reg = (SNAND_CMD_PAGE_READ << SNAND_PAGE_READ_CMD_OFFSET) | (u4RowAddr & SNAND_PAGE_READ_ADDRESS_MASK);
    SNAND_WriteReg(RW_SNAND_RD_CTL1, reg);

    // set DATA READ dummy cycle and command (use default value, ignored) (SPIQ)
    mtk_snand_dev_enable_spiq(1);

    reg = *(RW_SNAND_RD_CTL2);
    reg &= ~SNAND_DATA_READ_CMD_MASK;
    reg |= SNAND_CMD_RANDOM_READ_SPIQ & SNAND_DATA_READ_CMD_MASK;
    SNAND_WriteReg(RW_SNAND_RD_CTL2, reg);

  cleanup:

    return ret;
}

#if 0
static bool mtk_snand_ready_for_read_auto(struct nand_chip *nand, u32 u4RowAddr, u32 u4ColAddr, u32 u4SecNum, bool full, u8 * buf)
{
    /* Reset NFI HW internal state machine and flush NFI in/out FIFO */
    bool bRet = 1;
    u32 reg;
    u32 col_addr = u4ColAddr;
#if __INTERNAL_USE_AHB_MODE__
    u32 phys = 0;
#endif
    struct timeval stimer,etimer;

    do_gettimeofday(&stimer);

    if (!mtk_snand_reset_con())
    {
        bRet = 0;
        goto cleanup;
    }

    //------ SNF Part ------

    // set PAGE READ command & address
    reg = (SNAND_CMD_PAGE_READ << SNAND_PAGE_READ_CMD_OFFSET) | (u4RowAddr & SNAND_PAGE_READ_ADDRESS_MASK);
    SNAND_WriteReg(RW_SNAND_RD_CTL1, reg);

    // set DATA READ dummy cycle and command (use default value, ignored)
    mtk_snand_dev_enable_spiq(1);

    reg = *(RW_SNAND_RD_CTL2);
    reg &= ~SNAND_DATA_READ_CMD_MASK;
    reg |= SNAND_CMD_RANDOM_READ_SPIQ & SNAND_DATA_READ_CMD_MASK;
    SNAND_WriteReg(RW_SNAND_RD_CTL2, reg);

    // set DATA READ address
    SNAND_WriteReg(RW_SNAND_RD_CTL3, (col_addr & SNAND_DATA_READ_ADDRESS_MASK));

    // set SNF timing
    reg = *(RW_SNAND_MISC_CTL);

    reg &= ~SNAND_DATA_READ_MODE_MASK;
    reg |= ((SNAND_DATA_READ_MODE_X4 << SNAND_DATA_READ_MODE_OFFSET) & SNAND_DATA_READ_MODE_MASK);

    SNAND_WriteReg(RW_SNAND_MISC_CTL, reg);

    // set SNF data length

    reg = u4SecNum * (NAND_SECTOR_SIZE + g_snand_k_spare_per_sector);

    SNAND_WriteReg(RW_SNAND_MISC_CTL2, (reg | (reg << SNAND_PROGRAM_LOAD_BYTE_LEN_OFFSET)));

    /*
      * SNF get feature polling cycles
      * 1 unit = 128 * 10 ns (for 104 MHz) = 1.28 us
      */

    reg = *(RW_SNAND_GF_CTL3);
    reg = (reg & ~SNAND_GF_POLLING_CYCLE_MASK) | (1 & SNAND_GF_POLLING_CYCLE_MASK);
    SNAND_WriteReg(RW_SNAND_GF_CTL3, reg);

    //------ NFI Part ------

    mtk_snand_set_mode(CNFG_OP_CUST);
    SNAND_REG_SET_BITS(NFI_CNFG_REG16, CNFG_READ_EN);
    SNAND_WriteReg(NFI_CON_REG16, u4SecNum << CON_NFI_SEC_SHIFT);

    if (g_bHwEcc)
    {
        /* Enable HW ECC */
        SNAND_REG_SET_BITS(NFI_CNFG_REG16, CNFG_HW_ECC_EN);
    } else
    {
        SNAND_REG_CLN_BITS(NFI_CNFG_REG16, CNFG_HW_ECC_EN);
    }

    if (full)
    {
#if __INTERNAL_USE_AHB_MODE__
        SNAND_REG_SET_BITS(NFI_CNFG_REG16, CNFG_AHB);

        phys = nand_virt_to_phys_add((u32) buf);

        if (!phys)
        {
            printk(KERN_ERR "[mtk_snand_ready_for_read_auto] convert virt addr (%x) to phys add (%x)fail!!!", (u32) buf, phys);
            return 0;
        }
        else
        {
            SNAND_WriteReg(NFI_STRADDR_REG32, phys);
        }
#else   // use MCU
        SNAND_REG_CLN_BITS(NFI_CNFG_REG16, CNFG_AHB);
#endif

        if (g_bHwEcc)
        {
            SNAND_REG_SET_BITS(NFI_CNFG_REG16, CNFG_HW_ECC_EN);
        } else
        {
            SNAND_REG_CLN_BITS(NFI_CNFG_REG16, CNFG_HW_ECC_EN);
        }

    }
    else    // raw data
    {
        SNAND_REG_CLN_BITS(NFI_CNFG_REG16, CNFG_HW_ECC_EN);
        SNAND_REG_CLN_BITS(NFI_CNFG_REG16, CNFG_AHB);
    }

    mtk_snand_set_autoformat(full);

    if (full)
    {
        if (g_bHwEcc)
        {
            mtk_snand_ecc_decode_start();
        }
    }

  cleanup:

    do_gettimeofday(&etimer);
    g_NandPerfLog.ReadBusyTotalTime+= Cal_timediff(&etimer,&stimer);
    g_NandPerfLog.ReadBusyCount++;

    return bRet;
}
#endif


/******************************************************************************
 * mtk_snand_ready_for_read_custom
 *
 * DESCRIPTION:
 *    Prepare hardware environment for read !
 *
 * PARAMETERS:
 *   struct nand_chip *nand, u32 u4RowAddr
 *
 * RETURNS:
 *   None
 *
 * NOTES:
 *   None
 *
 ******************************************************************************/
static bool mtk_snand_ready_for_read_custom(struct nand_chip *nand, u32 u4RowAddr, u32 u4ColAddr, u32 u4SecNum, u8 * buf, u8 mtk_ecc, u8 auto_fmt, u8 ahb_mode)
{
    /* Reset NFI HW internal state machine and flush NFI in/out FIFO */
    u8  ret = 1;
    u32 cmd, reg;
    u32 col_addr = u4ColAddr;
#if __INTERNAL_USE_AHB_MODE__
    u32 phys = 0;
#endif
    struct timeval stimer,etimer;
#ifdef CFG_SNAND_SLEEP_WHILE_BUSY_PROC_TEST
    struct timeval stimer1, stimer2, etimer1;
#endif

    do_gettimeofday(&stimer);

    #ifdef _SNAND_DEBUG
    if (u4RowAddr >= 9217 && u4RowAddr <= 9219)
    {
        printk("[K-SNAND][%s] row:%d\n", __FUNCTION__, u4RowAddr);
        //printk("[K-SNAND] buf:\n");
        //mtk_snand_dump_mem((u32 *)buf, 2048);
        //printk("[K-SNAND] fdm:\n");
        //mtk_snand_dump_mem((u32 *)pFDMBuf, 64);
        //dump_stack();
    }
    #endif

    if (!mtk_snand_reset_con())
    {
        ret = 0;
        goto cleanup;
    }

    // 1. Read page to cache

    cmd = mtk_snand_gen_c1a3(SNAND_CMD_PAGE_READ, u4RowAddr); // PAGE_READ command + 3-byte address

    SNAND_WriteReg(RW_SNAND_GPRAM_DATA, cmd);
    SNAND_WriteReg(RW_SNAND_MAC_OUTL, 1 + 3);
    SNAND_WriteReg(RW_SNAND_MAC_INL , 0);

    mtk_snand_dev_mac_op(SPI);

    #ifdef CFG_SNAND_SLEEP_WHILE_BUSY_PROC_TEST
    do_gettimeofday(&stimer1);
    #endif

    #ifdef CFG_SNAND_SLEEP_WHILE_BUSY
    usleep_range(g_snand_sleep_us_page_read_min, g_snand_sleep_us_page_read_max);
    #endif

    #ifdef CFG_SNAND_SLEEP_WHILE_BUSY_PROC_TEST
    do_gettimeofday(&etimer1);
    g_snand_sleep_temp_val1 = Cal_timediff(&etimer1,&stimer1);
    #endif

    // 2. Get features (status polling)

    cmd = SNAND_CMD_GET_FEATURES | (SNAND_CMD_FEATURES_STATUS << 8);

    SNAND_WriteReg(RW_SNAND_GPRAM_DATA, cmd);
    SNAND_WriteReg(RW_SNAND_MAC_OUTL, 2);
    SNAND_WriteReg(RW_SNAND_MAC_INL , 1);

    for (;;)
    {
        mtk_snand_dev_mac_op(SPI);

        cmd = DRV_Reg8(((P_U8)RW_SNAND_GPRAM_DATA + 2));

        if ((cmd & SNAND_STATUS_OIP) == 0)
        {
            // use MTK ECC, not device ECC
            //if (SNAND_STATUS_TOO_MANY_ERROR_BITS == (cmd & SNAND_STATUS_ECC_STATUS_MASK) )
            //{
            //   bRet = 0;
            //}

            break;
        }
    }

    #ifdef CFG_SNAND_SLEEP_WHILE_BUSY_PROC_TEST
    do_gettimeofday(&etimer1);
    g_snand_sleep_temp_val2 = Cal_timediff(&etimer1,&stimer1);
    #endif

    //------ SNF Part ------

    // set PAGE READ command & address
    reg = (SNAND_CMD_PAGE_READ << SNAND_PAGE_READ_CMD_OFFSET) | (u4RowAddr & SNAND_PAGE_READ_ADDRESS_MASK);
    SNAND_WriteReg(RW_SNAND_RD_CTL1, reg);

    // set DATA READ dummy cycle and command (use default value, ignored)
    mtk_snand_dev_enable_spiq(1);

    mtk_snand_nfi_enable_bypass(1);

    reg = *(RW_SNAND_RD_CTL2);
    reg &= ~SNAND_DATA_READ_CMD_MASK;
    reg |= SNAND_CMD_RANDOM_READ_SPIQ & SNAND_DATA_READ_CMD_MASK;
    SNAND_WriteReg(RW_SNAND_RD_CTL2, reg);

    // set DATA READ address
    SNAND_WriteReg(RW_SNAND_RD_CTL3, (col_addr & SNAND_DATA_READ_ADDRESS_MASK));

    // set SNF timing
    reg = *(RW_SNAND_MISC_CTL);

    reg |= SNAND_DATARD_CUSTOM_EN;

    reg &= ~SNAND_DATA_READ_MODE_MASK;
    reg |= ((SNAND_DATA_READ_MODE_X4 << SNAND_DATA_READ_MODE_OFFSET) & SNAND_DATA_READ_MODE_MASK);

    SNAND_WriteReg(RW_SNAND_MISC_CTL, reg);

    // set SNF data length

    reg = u4SecNum * (NAND_SECTOR_SIZE + g_snand_k_spare_per_sector);

    SNAND_WriteReg(RW_SNAND_MISC_CTL2, (reg | (reg << SNAND_PROGRAM_LOAD_BYTE_LEN_OFFSET)));

    //------ NFI Part ------

    mtk_snand_set_mode(CNFG_OP_CUST);
    SNAND_REG_SET_BITS(NFI_CNFG_REG16, CNFG_READ_EN);
    SNAND_WriteReg(NFI_CON_REG16, u4SecNum << CON_NFI_SEC_SHIFT);

    #if 0
    if (g_bHwEcc)
    {
        /* Enable HW ECC */
        SNAND_REG_SET_BITS(NFI_CNFG_REG16, CNFG_HW_ECC_EN);
    } else
    {
        SNAND_REG_CLN_BITS(NFI_CNFG_REG16, CNFG_HW_ECC_EN);
    }
    #endif

    if (ahb_mode)
    {
        SNAND_REG_SET_BITS(NFI_CNFG_REG16, CNFG_AHB);

        phys = nand_virt_to_phys_add((u32) buf);

        if (!phys)
        {
            printk("[%s]convert virt addr (%x) to phys add (%x)fail!!!", __FUNCTION__, (u32) buf, phys);

            mtk_snand_nfi_enable_bypass(0);

            return 0;
        }
        else
        {
            SNAND_WriteReg(NFI_STRADDR_REG32, phys);
        }
    }
    else
    {
        SNAND_REG_CLN_BITS(NFI_CNFG_REG16, CNFG_AHB);
    }

    if (g_bHwEcc && mtk_ecc)
    {
        SNAND_REG_SET_BITS(NFI_CNFG_REG16, CNFG_HW_ECC_EN);

        mtk_snand_ecc_decode_start();
    }
    else
    {
        SNAND_REG_CLN_BITS(NFI_CNFG_REG16, CNFG_HW_ECC_EN);
    }

    mtk_snand_set_autoformat(auto_fmt);

    #if 0
    if (full)
    {
#if __INTERNAL_USE_AHB_MODE__
        SNAND_REG_SET_BITS(NFI_CNFG_REG16, CNFG_AHB);

        phys = nand_virt_to_phys_add((u32) buf);

        if (!phys)
        {
            printk(KERN_ERR "[mtk_snand_ready_for_read_custom]convert virt addr (%x) to phys add (%x)fail!!!", (u32) buf, phys);
            return 0;
        }
        else
        {
            SNAND_WriteReg(NFI_STRADDR_REG32, phys);
        }
#else   // use MCU
        SNAND_REG_CLN_BITS(NFI_CNFG_REG16, CNFG_AHB);
#endif

        if (g_bHwEcc)
        {
            SNAND_REG_SET_BITS(NFI_CNFG_REG16, CNFG_HW_ECC_EN);
        } else
        {
            SNAND_REG_CLN_BITS(NFI_CNFG_REG16, CNFG_HW_ECC_EN);
        }

    }
    else    // raw data
    {
        SNAND_REG_CLN_BITS(NFI_CNFG_REG16, CNFG_HW_ECC_EN);
        SNAND_REG_CLN_BITS(NFI_CNFG_REG16, CNFG_AHB);
    }

    mtk_snand_set_autoformat(full);

    if (full)
    {
        if (g_bHwEcc)
        {
            mtk_snand_ecc_decode_start();
        }
    }
    #endif

  cleanup:

    do_gettimeofday(&etimer);
    g_NandPerfLog.ReadBusyTotalTime+= Cal_timediff(&etimer,&stimer);
    g_NandPerfLog.ReadBusyCount++;

    return ret;
}

/******************************************************************************
 * mtk_snand_ready_for_write
 *
 * DESCRIPTION:
 *    Prepare hardware environment for write !
 *
 * PARAMETERS:
 *   struct nand_chip *nand, u32 u4RowAddr
 *
 * RETURNS:
 *   None
 *
 * NOTES:
 *   None
 *
 ******************************************************************************/
static bool mtk_snand_ready_for_write(struct nand_chip *nand, u32 u4RowAddr, u32 col_addr, u8 * buf, u8 mtk_ecc, u8 auto_fmt, u8 ahb_mode)
{
    u32 sec_num = 1 << (nand->page_shift - 9);
    u32 reg;
    SNAND_Mode mode = SPIQ;
#if __INTERNAL_USE_AHB_MODE__
    u32 phys = 0;
#endif

    #ifdef _SNAND_DEBUG
    if (u4RowAddr >= 9217 && u4RowAddr <= 9219)
    {
        printk("[K-SNAND][%s] row:%d\n", __FUNCTION__, u4RowAddr);
        printk("[K-SNAND] buf:\n");
        mtk_snand_dump_mem((u32 *)buf, 2048);
        //printk("[K-SNAND] fdm:\n");
        //mtk_snand_dump_mem((u32 *)pFDMBuf, 64);
        //dump_stack();
    }
    #endif

    /* Reset NFI HW internal state machine and flush NFI in/out FIFO */
    if (!mtk_snand_reset_con())
    {
        return 0;
    }

    // 1. Write Enable
    mtk_snand_dev_command(SNAND_CMD_WRITE_ENABLE, 1);

    //------ SNF Part ------

    // set SPI-NAND command
    if (SPI == mode)
    {
        reg = SNAND_CMD_WRITE_ENABLE | (SNAND_CMD_PROGRAM_LOAD << SNAND_PG_LOAD_CMD_OFFSET) | (SNAND_CMD_PROGRAM_EXECUTE << SNAND_PG_EXE_CMD_OFFSET);
        SNAND_WriteReg(RW_SNAND_PG_CTL1, reg);
    }
    else if (SPIQ == mode)
    {
        reg = SNAND_CMD_WRITE_ENABLE | (SNAND_CMD_PROGRAM_LOAD_X4<< SNAND_PG_LOAD_CMD_OFFSET) | (SNAND_CMD_PROGRAM_EXECUTE << SNAND_PG_EXE_CMD_OFFSET);
        SNAND_WriteReg(RW_SNAND_PG_CTL1, reg);
        mtk_snand_dev_enable_spiq(1);
    }

    mtk_snand_nfi_enable_bypass(1);

    // set program load address
    SNAND_WriteReg(RW_SNAND_PG_CTL2, col_addr & SNAND_PG_LOAD_ADDR_MASK);

    // set program execution address
    SNAND_WriteReg(RW_SNAND_PG_CTL3, u4RowAddr);

    // set SNF data length  (set in snand_xxx_write_data)

    // set SNF timing
    reg = *(RW_SNAND_MISC_CTL);

    reg |= SNAND_PG_LOAD_CUSTOM_EN;    // use custom program

    if (SPI == mode)
    {
        reg &= ~SNAND_DATA_READ_MODE_MASK;
        reg |= ((SNAND_DATA_READ_MODE_X1 << SNAND_DATA_READ_MODE_OFFSET) & SNAND_DATA_READ_MODE_MASK);
        reg &=~ SNAND_PG_LOAD_X4_EN;
    }
    else if (SPIQ == mode)
    {
        reg &= ~SNAND_DATA_READ_MODE_MASK;
        reg |= ((SNAND_DATA_READ_MODE_X4 << SNAND_DATA_READ_MODE_OFFSET) & SNAND_DATA_READ_MODE_MASK);
        reg |= SNAND_PG_LOAD_X4_EN;
    }

    SNAND_WriteReg(RW_SNAND_MISC_CTL, reg);

    // set SNF data length

    reg = sec_num * (NAND_SECTOR_SIZE + g_snand_k_spare_per_sector);

    SNAND_WriteReg(RW_SNAND_MISC_CTL2, reg | (reg << SNAND_PROGRAM_LOAD_BYTE_LEN_OFFSET));

    //------ NFI Part ------

    mtk_snand_set_mode(CNFG_OP_PRGM);

    SNAND_REG_CLN_BITS(NFI_CNFG_REG16, CNFG_READ_EN);

    SNAND_WriteReg(NFI_CON_REG16, sec_num << CON_NFI_SEC_SHIFT);

    if (ahb_mode)
    {
        SNAND_REG_SET_BITS(NFI_CNFG_REG16, CNFG_AHB);

        phys = nand_virt_to_phys_add((u32) buf);

        if (!phys)
        {
            printk(KERN_ERR "[mtk_snand_ready_for_write] convert virt addr (%x) to phys add fail!!!", (u32) buf);
            return 0;
        }
        else
        {
            SNAND_WriteReg(NFI_STRADDR_REG32, phys);
        }
    }
    else
    {
        SNAND_REG_CLN_BITS(NFI_CNFG_REG16, CNFG_AHB);
    }

    mtk_snand_set_autoformat(auto_fmt);

    if (mtk_ecc)
    {
        SNAND_REG_SET_BITS(NFI_CNFG_REG16, CNFG_HW_ECC_EN);

        mtk_snand_ecc_encode_start();
    }
    else
    {
        SNAND_REG_CLN_BITS(NFI_CNFG_REG16, CNFG_HW_ECC_EN);
    }

    return 1;
}

static bool mtk_snand_check_dececc_done(u32 u4SecNum)
{
    u32 timeout, dec_mask;
    timeout = 0xffff;
    dec_mask = (1 << u4SecNum) - 1;

    while ((dec_mask != *(ECC_DECDONE_REG16)) && timeout > 0)
    {
        timeout--;

        if (timeout == 0)
        {
            MSG(VERIFY, "ECC_DECDONE: timeout\n");
            return 0;
        }
    }

    return 1;
}

/******************************************************************************
 * mtk_snand_read_page_data
 *
 * DESCRIPTION:
 *   Fill the page data into buffer !
 *
 * PARAMETERS:
 *   u8* pDataBuf, u32 u4Size
 *
 * RETURNS:
 *   None
 *
 * NOTES:
 *   None
 *
 ******************************************************************************/
static bool mtk_snand_dma_read_data(struct mtd_info *mtd, u8 * buf, u32 num_sec)
{
    int interrupt_en = g_i4Interrupt;
    int timeout = 0xffff;
    struct scatterlist sg;
    enum dma_data_direction dir = DMA_FROM_DEVICE;

    struct timeval stimer,etimer;

    #ifdef CFG_SNAND_SLEEP_WHILE_BUSY_PROC_TEST
    struct timeval stimer1, stimer2, etimer1;
    #endif

    #ifdef CFG_SNAND_DRV_RW_BREAKDOWN_DMA_READ
    do_gettimeofday(&g_stimer[1]);
    #endif

    do_gettimeofday(&stimer);

    sg_init_one(&sg, buf, num_sec * (NAND_SECTOR_SIZE + g_snand_k_spare_per_sector));
    dma_map_sg(&(mtd->dev), &sg, 1, dir);

    #ifdef CFG_SNAND_DRV_RW_BREAKDOWN_DMA_READ
    do_gettimeofday(&g_etimer[10]);
    #endif

    SNAND_REG_CLN_BITS(NFI_CNFG_REG16, CNFG_BYTE_RW);

    if ((unsigned int)buf % 16) // TODO: can not use AHB mode here
    {
        printk(KERN_INFO "Un-16-aligned address\n");
        SNAND_REG_CLN_BITS(NFI_CNFG_REG16, CNFG_DMA_BURST_EN);
    } else
    {
        SNAND_REG_SET_BITS(NFI_CNFG_REG16, CNFG_DMA_BURST_EN);
    }

    // set dummy command to trigger NFI enter custom mode
    SNAND_WriteReg(NFI_CMD_REG16, NAND_CMD_DUMMYREAD);

    mb();

    *(NFI_INTR_REG16);  // read clear

    SNAND_WriteReg(NFI_INTR_EN_REG16, INTR_AHB_DONE_EN);

    if (interrupt_en)
    {
        init_completion(&g_comp_AHB_Done);
    }

    mb();

    SNAND_REG_SET_BITS(NFI_CON_REG16, CON_NFI_BRD);

    mb();

    g_snand_dev_status = SNAND_NFI_CUST_READING;
    g_snand_cmd_status = 0; // busy
    g_running_dma = 1;

    #ifdef CFG_SNAND_DRV_RW_BREAKDOWN_DMA_READ
    do_gettimeofday(&g_etimer[11]);
    #endif

    #ifdef CFG_SNAND_SLEEP_WHILE_BUSY_PROC_TEST
    do_gettimeofday(&stimer1);
    #endif

    #ifdef CFG_SNAND_SLEEP_WHILE_BUSY
    usleep_range(g_snand_sleep_us_page_read_min, g_snand_sleep_us_page_read_max);
    #endif

    #ifdef CFG_SNAND_SLEEP_WHILE_BUSY_PROC_TEST
    do_gettimeofday(&etimer1);
    g_snand_sleep_temp_val1 = Cal_timediff(&etimer1,&stimer1);
    #endif

    if (interrupt_en)
    {
        // Wait 10ms for AHB done
        if (!wait_for_completion_timeout(&g_comp_AHB_Done, 20))
        {
            MSG(READ, "wait for completion timeout happened @ [%s]: %d\n", __FUNCTION__, __LINE__);
            mtk_snand_dump_reg();
            g_snand_dev_status = SNAND_IDLE;
            g_running_dma = 0;

            goto mtk_snand_dma_read_data_failed;
        }

        g_snand_dev_status = SNAND_IDLE;
        g_running_dma = 0;

        while (num_sec > ((*(NFI_BYTELEN_REG16) & 0x1f000) >> 12))
        {
            timeout--;

            if (0 == timeout)
            {
                printk(KERN_ERR "[%s] poll BYTELEN error\n", __FUNCTION__);
                g_snand_dev_status = SNAND_IDLE;
                g_running_dma = 0;

                goto mtk_snand_dma_read_data_failed;
            }
        }
    }
    else
    {
        while (!*(NFI_INTR_REG16))
        {
            timeout--;

            if (0 == timeout)   // time out
            {
                printk(KERN_ERR "[%s] poll nfi_intr error\n", __FUNCTION__);
                mtk_snand_dump_reg();
                g_snand_dev_status = SNAND_IDLE;
                g_running_dma = 0;

                goto mtk_snand_dma_read_data_failed;
            }
        }

        #ifdef CFG_SNAND_DRV_RW_BREAKDOWN_DMA_READ
        do_gettimeofday(&g_etimer[13]);
        #endif

        while (num_sec > ((*(NFI_BYTELEN_REG16) & 0x1f000) >> 12))
        {
            timeout--;

            if (0 == timeout)
            {
                printk(KERN_ERR "[%s] poll BYTELEN error\n", __FUNCTION__);
                mtk_snand_dump_reg();
                g_snand_dev_status = SNAND_IDLE;
                g_running_dma = 0;

                goto mtk_snand_dma_read_data_failed;
            }
        }

        g_snand_dev_status = SNAND_IDLE;
        g_running_dma = 0;
    }

    #ifdef CFG_SNAND_SLEEP_WHILE_BUSY_PROC_TEST
    do_gettimeofday(&etimer1);
    g_snand_sleep_temp_val2 = Cal_timediff(&etimer1,&stimer1);
    #endif

    #ifdef CFG_SNAND_DRV_RW_BREAKDOWN_DMA_READ
    do_gettimeofday(&g_etimer[14]);
    #endif

    dma_unmap_sg(&(mtd->dev), &sg, 1, dir);
    do_gettimeofday(&etimer);
    g_NandPerfLog.ReadDMATotalTime+= Cal_timediff(&etimer,&stimer);
    g_NandPerfLog.ReadDMACount++;

    #ifdef CFG_SNAND_DRV_RW_BREAKDOWN_DMA_READ
    do_gettimeofday(&g_etimer[15]);

    printk("perf-dr, s:%d, 10:%d, 11:%d, 12:%d, 13:%d, F:%d, STA_CTL3:0x%X\n"
            ,num_sec
            ,Cal_timediff(&g_etimer[10],&g_stimer[1])
            ,Cal_timediff(&g_etimer[11],&g_stimer[1])
            ,Cal_timediff(&g_etimer[12],&g_stimer[1])
            ,Cal_timediff(&g_etimer[13],&g_stimer[1])
            ,Cal_timediff(&g_etimer[14],&g_stimer[1])
            ,Cal_timediff(&g_etimer[15],&g_stimer[1])
            ,*(RW_SNAND_STA_CTL3)
            );
    #endif

    mtk_snand_nfi_enable_bypass(0);

    return 1;

mtk_snand_dma_read_data_failed:

    mtk_snand_nfi_enable_bypass(0);

    return 0;
}

#if !defined(__INTERNAL_USE_AHB_MODE__)
#if 0
static bool mtk_snand_mcu_read_data(u8 * buf, u32 u4SecNum, u8 full)
{
    int timeout = 0xffff;
    u32 i;
    u32 *buf32 = (u32 *) buf;
    u32 snf_len;
    u32 length;
#ifdef TESTTIME
    unsigned long long time1, time2;
    time1 = sched_clock();
#endif

    length = u4SecNum * NAND_SECTOR_SIZE;

    // set SNF data length
    if (full)
	{
		snf_len = length + devinfo.sparesize;
	}
	else
	{
		snf_len = length;
	}

    SNAND_WriteReg(RW_SNAND_MISC_CTL2, (snf_len | (snf_len << SNAND_PROGRAM_LOAD_BYTE_LEN_OFFSET)));

    // set dummy command to trigger NFI enter custom mode
    SNAND_WriteReg(NFI_CMD_REG16, NAND_CMD_DUMMYREAD);

    if ((u32) buf % 4 || length % 4)
        SNAND_REG_SET_BITS(NFI_CNFG_REG16, CNFG_BYTE_RW);
    else
        SNAND_REG_CLN_BITS(NFI_CNFG_REG16, CNFG_BYTE_RW);

    mb();

    SNAND_REG_SET_BITS(NFI_CON_REG16, CON_NFI_BRD);

    if ((u32) buf % 4 || length % 4)
    {
        for (i = 0; (i < (length)) && (timeout > 0);)
        {
            if (*(NFI_PIO_DIRDY_REG16) & 1)
            {
                *buf++ = (u8) *(NFI_DATAR_REG32);
                i++;
            }
            else
            {
                timeout--;
            }

            if (0 == timeout)
            {
                printk(KERN_ERR "[%s] timeout\n", __FUNCTION__);
                mtk_snand_dump_reg();
                return 0;
            }
        }
    }
    else
    {
        for (i = 0; (i < (length >> 2)) && (timeout > 0);)
        {
            if (*(NFI_PIO_DIRDY_REG16) & 1)
            {
                *buf32++ = *(NFI_DATAR_REG32);
                i++;
            }
            else
            {
                timeout--;
            }

            if (0 == timeout)
            {
                printk(KERN_ERR "[%s] timeout\n", __FUNCTION__);
                mtk_snand_dump_reg();
                return 0;
            }
        }
    }
#ifdef TESTTIME
    time2 = sched_clock() - time1;

    if (!readdatatime)
    {
        readdatatime = (time2);
    }
#endif
    return 1;
}
#endif
#endif

static bool mtk_snand_read_page_data(struct mtd_info *mtd, u8 * pDataBuf, u32 u4SecNum, u8 full)
{
#if (__INTERNAL_USE_AHB_MODE__)
    return mtk_snand_dma_read_data(mtd, pDataBuf, u4SecNum);
#else
    #error "[K-SNAND] MCU r/w is prohibited for SPI-NAND"
    //return mtk_snand_mcu_read_data(mtd, pDataBuf, u4SecNum, full);
#endif
}

/******************************************************************************
 * mtk_snand_write_page_data
 *
 * DESCRIPTION:
 *   Fill the page data into buffer !
 *
 * PARAMETERS:
 *   u8* pDataBuf, u32 u4Size
 *
 * RETURNS:
 *   None
 *
 * NOTES:
 *   None
 *
 ******************************************************************************/
static bool mtk_snand_dma_write_data(struct mtd_info *mtd, u8 * pDataBuf, u32 u4Size, u8 full)
{
    int i4Interrupt = 0;        //g_i4Interrupt;
    u32 snf_len;
    struct scatterlist sg;
    enum dma_data_direction dir = DMA_TO_DEVICE;
    struct timeval stimer,etimer;
    do_gettimeofday(&stimer);

    sg_init_one(&sg, pDataBuf, u4Size);
    dma_map_sg(&(mtd->dev), &sg, 1, dir);

    // set SNF data length
    if (full)
	{
		snf_len = u4Size + devinfo.sparesize;
	}
	else
	{
		snf_len = u4Size;
	}

    SNAND_WriteReg(RW_SNAND_MISC_CTL2, ((snf_len) | (snf_len << SNAND_PROGRAM_LOAD_BYTE_LEN_OFFSET)));

    SNAND_REG_CLN_BITS(NFI_CNFG_REG16, CNFG_BYTE_RW);

    // set dummy command to trigger NFI enter custom mode
    SNAND_WriteReg(NFI_CMD_REG16, NAND_CMD_DUMMYPROG);

    *(NFI_INTR_REG16);
    SNAND_WriteReg(NFI_INTR_EN_REG16, INTR_CUSTOM_PROG_DONE_INTR_EN);

    if ((unsigned int)pDataBuf % 16)    // TODO: can not use AHB mode here
    {
        printk(KERN_INFO "Un-16-aligned address\n");
        SNAND_REG_CLN_BITS(NFI_CNFG_REG16, CNFG_DMA_BURST_EN);
    } else
    {
        SNAND_REG_SET_BITS(NFI_CNFG_REG16, CNFG_DMA_BURST_EN);
    }

    if (i4Interrupt)
    {
        init_completion(&g_comp_AHB_Done);
        *(NFI_INTR_REG16);  // read clear
        SNAND_WriteReg(NFI_INTR_EN_REG16, INTR_CUSTOM_PROG_DONE_INTR_EN);
    }

    mtk_snand_nfi_enable_bypass(0);

    mb();

    SNAND_REG_SET_BITS(NFI_CON_REG16, CON_NFI_BWR);

    g_running_dma = 3;

    if (i4Interrupt)
    {
        // Wait 10ms for AHB done
        if (!wait_for_completion_timeout(&g_comp_AHB_Done, 20))
        {
            MSG(READ, "wait for completion timeout happened @ [%s]: %d\n", __FUNCTION__, __LINE__);
            mtk_snand_dump_reg();
            g_running_dma = 0;
            return 0;
        }

        g_running_dma = 0;
    }
    else
    {
        while (!(*(RW_SNAND_STA_CTL1) & SNAND_CUSTOM_PROGRAM))
        {
            #if 0   // FIXME: Stanley Chu, use a suitable timeout value
            timeout--;

            if (0 == timeout)
            {
                printk(KERN_ERR "[%s] poll BYTELEN error\n", __FUNCTION__);
                g_running_dma = 0;
                return 0;   //4  // AHB Mode Time Out!
            }
            #endif
        }

        g_running_dma = 0;
    }

    dma_unmap_sg(&(mtd->dev), &sg, 1, dir);
    do_gettimeofday(&etimer);
    g_NandPerfLog.WriteDMATotalTime+= Cal_timediff(&etimer,&stimer);
    g_NandPerfLog.WriteDMACount++;

    return 1;
}

#if 0
static bool mtk_snand_mcu_write_data(struct mtd_info *mtd, const u8 * buf, u32 length, u8 full)
{
    u32 timeout = 0xFFFF;
    u32 i;
    u32 *pBuf32;
    u32 snf_len;

	// set SNF data length
    if (full)
	{
		snf_len += length + devinfo.sparesize;
	}
	else
	{
		snf_len = length;
	}

    SNAND_WriteReg(RW_SNAND_MISC_CTL2, ((snf_len) | (snf_len << SNAND_PROGRAM_LOAD_BYTE_LEN_OFFSET)));

    // set dummy command to trigger NFI enter custom mode
    SNAND_WriteReg(NFI_CMD_REG16, NAND_CMD_DUMMYPROG);

    SNAND_REG_CLN_BITS(NFI_CNFG_REG16, CNFG_BYTE_RW);

    mb();

    SNAND_REG_SET_BITS(NFI_CON_REG16, CON_NFI_BWR);

    pBuf32 = (u32 *) buf;

    if ((u32) buf % 4 || length % 4)
        SNAND_REG_SET_BITS(NFI_CNFG_REG16, CNFG_BYTE_RW);
    else
        SNAND_REG_CLN_BITS(NFI_CNFG_REG16, CNFG_BYTE_RW);

    if ((u32) buf % 4 || length % 4)
    {
        for (i = 0; (i < (length)) && (timeout > 0);)
        {
            if (*(NFI_PIO_DIRDY_REG16) & 1)
            {
                SNAND_WriteReg(NFI_DATAW_REG32, *buf++);
                i++;
            } else
            {
                timeout--;
            }
            if (0 == timeout)
            {
                printk(KERN_ERR "[%s] timeout\n", __FUNCTION__);
                mtk_snand_dump_reg();
                return 0;
            }
        }
    } else
    {
        for (i = 0; (i < (length >> 2)) && (timeout > 0);)
        {
            // if (FIFO_WR_REMAIN(*(NFI_FIFOSTA_REG16)) <= 12)
            if (*(NFI_PIO_DIRDY_REG16) & 1)
            {
                SNAND_WriteReg(NFI_DATAW_REG32, *pBuf32++);
                i++;
            } else
            {
                timeout--;
            }
            if (0 == timeout)
            {
                printk(KERN_ERR "[%s] timeout\n", __FUNCTION__);
                mtk_snand_dump_reg();
                return 0;
            }
        }
    }

    return 1;
}
#endif

static bool mtk_snand_write_page_data(struct mtd_info *mtd, u8 * buf, u32 size, u8 full)
{
#if (__INTERNAL_USE_AHB_MODE__)
    return mtk_snand_dma_write_data(mtd, buf, size, full);
#else
    #error "[K-SNAND] MCU r/w is prohibited for SPI-NAND"
    //return mtk_snand_mcu_write_data(mtd, buf, size, full);
#endif
}

/******************************************************************************
 * mtk_snand_read_fdm_data
 *
 * DESCRIPTION:
 *   Read a fdm data !
 *
 * PARAMETERS:
 *   u8* pDataBuf, u32 u4SecNum
 *
 * RETURNS:
 *   None
 *
 * NOTES:
 *   None
 *
 ******************************************************************************/
static u32 mtk_snand_read_fdm_data_devecc(u8 * dest, u8 * src, u32 start_sec, u32 num_sec)
{
    u32 i, j;
    u32 dest_idx, cur_dest_sec, byte_cnt_in_cur_sec;

    if (0 != (SNAND_SPARE_FORMAT_1 & devinfo.spareformat)) // spare format 1
    {
        dest_idx = 0;
        cur_dest_sec = 0;
        byte_cnt_in_cur_sec = 0;

        for (i = start_sec; g_snand_k_spare_format_1[i][0] != 0; i++)
        {
            for (j = 0; j < g_snand_k_spare_format_1[i][1]; j++)
            {
                dest[dest_idx] = src[devinfo.pagesize + (g_snand_k_spare_format_1[i][0] + j)];

                dest_idx++;
                byte_cnt_in_cur_sec++;

                if (SNAND_FDM_DATA_SIZE_PER_SECTOR == byte_cnt_in_cur_sec)  // all spare bytes in this sector are copied, go to next sector
                {
                    cur_dest_sec++;

                    if (num_sec == cur_dest_sec)   // all sectors' spare are filled
                    {
                        return 1;
                    }
                    else
                    {
                        byte_cnt_in_cur_sec = 0;
                    }
                }
            }
        }
    }
    else
    {
        printk("[K-SNAND][%s] ERROR: Invalid spare format!\n", __FUNCTION__);

        ASSERT(0);

        return 0;
    }

    return 0;
}

static void mtk_snand_read_fdm_data(u8 * pDataBuf, u32 u4SecNum)
{
    u32 i;
    u32 *pBuf32 = (u32 *) pDataBuf;

    if (pBuf32)
    {
        for (i = 0; i < u4SecNum; ++i)
        {
            *pBuf32++ = *(NFI_FDM0L_REG32 + (i << 1));
            *pBuf32++ = *(NFI_FDM0M_REG32 + (i << 1));
        }
    }
}

/******************************************************************************
 * mtk_snand_write_fdm_data
 *
 * DESCRIPTION:
 *   Write a fdm data !
 *
 * PARAMETERS:
 *   u8* pDataBuf, u32 u4SecNum
 *
 * RETURNS:
 *   None
 *
 * NOTES:
 *   None
 *
 ******************************************************************************/
static u8 mtk_snand_write_fdm_data_devecc(u8 * dest, u8 * src, u32 u4SecNum)
{
    u32 i, j, src_idx, cur_sec_index;

    if (0 != (SNAND_SPARE_FORMAT_1 & devinfo.spareformat)) // spare format 1
    {
        // convert from continuous spare data to DEVICE ECC's spare format

        memset(dest, 0xFF, devinfo.sparesize);

        src_idx = 0;
        cur_sec_index = 0;

        for (i = 0; g_snand_k_spare_format_1[i][0] != 0; i++)
        {
            for (j = 0; j < g_snand_k_spare_format_1[i][1]; j++)
            {
                dest[g_snand_k_spare_format_1[i][0] + j] = src[(cur_sec_index * SNAND_FDM_DATA_SIZE_PER_SECTOR) + src_idx];

                src_idx++;

                if (SNAND_FDM_DATA_SIZE_PER_SECTOR == src_idx)
                {
                    cur_sec_index++;

                    if (cur_sec_index == u4SecNum)
                    {
                        return 1;
                    }
                }
            }
        }
    }
    else
    {
        return 0;
    }

    return 0;
}

__attribute__((aligned(64))) static u8 fdm_buf[64];

static void mtk_snand_write_fdm_data(struct nand_chip *chip, u8 * pDataBuf, u32 u4SecNum)
{
    u32 i, j;
    u8 checksum = 0;
    bool empty = 1;
    struct nand_oobfree *free_entry;
    u32 *pBuf32;

    memcpy(fdm_buf, pDataBuf, u4SecNum * 8);

    free_entry = chip->ecc.layout->oobfree;

    for (i = 0; i < MTD_MAX_OOBFREE_ENTRIES && free_entry[i].length; i++)
    {
        for (j = 0; j < free_entry[i].length; j++)
        {
            if (pDataBuf[free_entry[i].offset + j] != 0xFF)
            {
                empty = 0;
            }

            checksum ^= pDataBuf[free_entry[i].offset + j];
        }
    }

    if (!empty)
    {
        fdm_buf[free_entry[i - 1].offset + free_entry[i - 1].length] = checksum;
    }

    pBuf32 = (u32 *) fdm_buf;

    for (i = 0; i < u4SecNum; ++i)
    {
        SNAND_WriteReg(NFI_FDM0L_REG32 + (i << 1), *pBuf32++);
        SNAND_WriteReg(NFI_FDM0M_REG32 + (i << 1), *pBuf32++);
    }
}

/******************************************************************************
 * mtk_snand_stop_read_custom
 *
 * DESCRIPTION:
 *   Stop read operation !
 *
 * PARAMETERS:
 *   None
 *
 * RETURNS:
 *   None
 *
 * NOTES:
 *   None
 *
 ******************************************************************************/
/*
static void mtk_snand_stop_read_auto(void)
{
    //------ NFI Part
    SNAND_REG_CLN_BITS(NFI_CON_REG16, CON_NFI_BRD);

    //------ SNF Part

    // set 1 then set 0 to clear done flag
    SNAND_WriteReg(RW_SNAND_STA_CTL1, SNAND_AUTO_READ);
    SNAND_WriteReg(RW_SNAND_STA_CTL1, 0);

    if (g_bHwEcc)
    {
        mtk_snand_ecc_decode_end();
    }

    SNAND_WriteReg(NFI_INTR_EN_REG16, 0);

    mtk_snand_reset_con();
}
*/

static void mtk_snand_stop_read_custom(u8 mtk_ecc)
{
    //------ NFI Part
    SNAND_REG_CLN_BITS(NFI_CON_REG16, CON_NFI_BRD);

    //------ SNF Part

    // set 1 then set 0 to clear done flag
    SNAND_WriteReg(RW_SNAND_STA_CTL1, SNAND_CUSTOM_READ);
    SNAND_WriteReg(RW_SNAND_STA_CTL1, 0);

    // clear essential SNF setting
    SNAND_REG_CLN_BITS(RW_SNAND_MISC_CTL, SNAND_PG_LOAD_CUSTOM_EN);

    if (mtk_ecc)
    {
        mtk_snand_ecc_decode_end();
    }

    SNAND_WriteReg(NFI_INTR_EN_REG16, 0);

    mtk_snand_nfi_enable_bypass(0);

    mtk_snand_reset_con();
}

/******************************************************************************
 * mtk_snand_stop_write
 *
 * DESCRIPTION:
 *   Stop write operation !
 *
 * PARAMETERS:
 *   None
 *
 * RETURNS:
 *   None
 *
 * NOTES:
 *   None
 *
 ******************************************************************************/
static void mtk_snand_stop_write(u8 mtk_ecc)
{
    //------ NFI Part

    SNAND_REG_CLN_BITS(NFI_CON_REG16, CON_NFI_BWR);

    //------ SNF Part

    // set 1 then set 0 to clear done flag
    SNAND_WriteReg(RW_SNAND_STA_CTL1, SNAND_CUSTOM_PROGRAM);
    SNAND_WriteReg(RW_SNAND_STA_CTL1, 0);

    // clear essential SNF setting
    SNAND_REG_CLN_BITS(RW_SNAND_MISC_CTL, SNAND_PG_LOAD_CUSTOM_EN);

    mtk_snand_nfi_enable_bypass(0);

    mtk_snand_dev_enable_spiq(0);

    if (mtk_ecc)
    {
        mtk_snand_ecc_encode_end();
    }

    SNAND_WriteReg(NFI_INTR_EN_REG16, 0);
}

static bool mtk_snand_read_page_part2(struct mtd_info *mtd, u32 row_addr, u32 num_sec, u8 * buf)
{
    bool    bRet = 1;
    u32     reg;
    u32     col_part2, i, len;
    u32     spare_per_sector;
    P_U8    buf_part2;
    u32     timeout = 0xFFFF;
    u32     old_dec_mode = 0;
    u32     buf_phy;
    struct scatterlist sg;
    enum dma_data_direction dir = DMA_FROM_DEVICE;

    //printk("r-par2, r:%d, sec:%d\n", row_addr, num_sec); // stanley chu

    sg_init_one(&sg, buf, NAND_SECTOR_SIZE + OOB_PER_SECTOR);
    dma_map_sg(&(mtd->dev), &sg, 1, dir);

    spare_per_sector = mtd->oobsize / (mtd->writesize / NAND_SECTOR_SIZE);

    for (i = 0; i < 2 ; i++)
    {
        mtk_snand_reset_con();

        mtk_snand_nfi_enable_bypass(1);

        if (0 == i)
        {
            col_part2 = (NAND_SECTOR_SIZE + spare_per_sector) * (num_sec - 1);

            buf_part2 = buf;

            len = 2112 - col_part2;
        }
        else
        {
            col_part2 = 2112;

            buf_part2 += len;   // append to first round

            len = (num_sec * (NAND_SECTOR_SIZE + spare_per_sector)) - 2112;
        }

        //------ SNF Part ------

        // set DATA READ address
        SNAND_WriteReg(RW_SNAND_RD_CTL3, (col_part2 & SNAND_DATA_READ_ADDRESS_MASK));

        if (0 == i)
        {
            // set RW_SNAND_MISC_CTL
            reg = *(RW_SNAND_MISC_CTL);

            reg |= SNAND_DATARD_CUSTOM_EN;

            reg &= ~SNAND_DATA_READ_MODE_MASK;
            reg |= ((SNAND_DATA_READ_MODE_X4 << SNAND_DATA_READ_MODE_OFFSET) & SNAND_DATA_READ_MODE_MASK);

            SNAND_WriteReg(RW_SNAND_MISC_CTL, reg);
        }

        // set SNF data length
        reg = len | (len << SNAND_PROGRAM_LOAD_BYTE_LEN_OFFSET);

        SNAND_WriteReg(RW_SNAND_MISC_CTL2, reg);

        //------ NFI Part ------

        if (0 == i)
        {
            mtk_snand_set_mode(CNFG_OP_CUST);
            SNAND_REG_SET_BITS(NFI_CNFG_REG16, CNFG_READ_EN);
            SNAND_REG_SET_BITS(NFI_CNFG_REG16, CNFG_AHB);
            SNAND_REG_CLN_BITS(NFI_CNFG_REG16, CNFG_HW_ECC_EN);
            mtk_snand_set_autoformat(0);
        }

        //SNAND_WriteReg(NFI_CON_REG16, 1 << CON_NFI_SEC_SHIFT);

        buf_phy = nand_virt_to_phys_add((u32)buf_part2);

        SNAND_WriteReg(NFI_STRADDR_REG32, buf_phy);

        SNAND_WriteReg(NFI_SPIDMA_REG32, SPIDMA_SEC_EN | (len & SPIDMA_SEC_SIZE_MASK));


        SNAND_REG_CLN_BITS(NFI_CNFG_REG16, CNFG_BYTE_RW);

        // set dummy command to trigger NFI enter custom mode
        SNAND_WriteReg(NFI_CMD_REG16, NAND_CMD_DUMMYREAD);

        *(NFI_INTR_REG16);  // read clear
        SNAND_WriteReg(NFI_INTR_EN_REG16, INTR_AHB_DONE_EN);

        SNAND_REG_SET_BITS(NFI_CON_REG16, (1 << CON_NFI_SEC_SHIFT) | CON_NFI_BRD);     // fixed to sector number 1

        timeout = 0xFFFF;

        while (!(*(NFI_INTR_REG16) & INTR_AHB_DONE))    // for custom read, wait NFI's INTR_AHB_DONE done to ensure all data are transferred to buffer
        {
            timeout--;

            if (0 == timeout)
            {
                bRet = 0;
                goto unmap_and_cleanup;
            }
        }

        timeout = 0xFFFF;

        while (((*(NFI_BYTELEN_REG16) & 0x1f000) >> 12) != 1)
        {
            timeout--;

            if (0 == timeout)
            {
                bRet = 0;
                goto unmap_and_cleanup;
            }
        }

        //------ NFI Part

        SNAND_REG_CLN_BITS(NFI_CON_REG16, CON_NFI_BRD);

        //------ SNF Part

        // set 1 then set 0 to clear done flag
        SNAND_WriteReg(RW_SNAND_STA_CTL1, SNAND_CUSTOM_READ);
        SNAND_WriteReg(RW_SNAND_STA_CTL1, 0);

        // clear essential SNF setting
        if (1 == i)
        {
            SNAND_REG_CLN_BITS(RW_SNAND_MISC_CTL, SNAND_DATARD_CUSTOM_EN);
        }

        mtk_snand_nfi_enable_bypass(0);
    }

    dma_unmap_sg(&(mtd->dev), &sg, 1, dir);

    if (g_bHwEcc)
    {
        dir = DMA_BIDIRECTIONAL;

        sg_init_one(&sg, buf, NAND_SECTOR_SIZE + OOB_PER_SECTOR);
        dma_map_sg(&(mtd->dev), &sg, 1, dir);

        mtk_snand_nfi_enable_bypass(1);

        /* configure ECC decoder && encoder */
        reg = *(ECC_DECCNFG_REG32);
        old_dec_mode = reg & DEC_CNFG_DEC_MODE_MASK;
        reg &= ~DEC_CNFG_DEC_MODE_MASK;
        reg |= DEC_CNFG_AHB;
        reg |= DEC_CNFG_DEC_BURST_EN;
        SNAND_WriteReg(ECC_DECCNFG_REG32, reg);

        buf_phy = nand_virt_to_phys_add((u32)buf);

        SNAND_WriteReg(ECC_DECDIADDR_REG32, (u32)buf_phy);

        SNAND_WriteReg(ECC_DECCON_REG16, DEC_DE);
        SNAND_WriteReg(ECC_DECCON_REG16, DEC_EN);

        while(!((*(ECC_DECDONE_REG16)) & (1 << 0)));

        mtk_snand_nfi_enable_bypass(0);

        dma_unmap_sg(&(mtd->dev), &sg, 1, dir);

        reg = *(ECC_DECENUM0_REG32);

        if (0 != reg)
        {
            reg &= 0x1F;

            if (0x1F == reg)
            {
                bRet = 0;
                xlog_printk(ANDROID_LOG_WARN,"NFI", "ECC-U(2), PA=%d, S=%d\n", row_addr, num_sec - 1);
            }
            else
            {
				if (reg)
                {
                    xlog_printk(ANDROID_LOG_INFO,"NFI"," ECC-C(2), #err:%d, PA=%d, S=%d\n", reg, row_addr, num_sec - 1);
				}
            }
        }

        // restore essential NFI and ECC registers
        SNAND_WriteReg(NFI_SPIDMA_REG32, 0);
        reg = *(ECC_DECCNFG_REG32);
        reg &= ~DEC_CNFG_DEC_MODE_MASK;
        reg |= old_dec_mode;
        reg &= ~DEC_CNFG_DEC_BURST_EN;
        SNAND_WriteReg(ECC_DECCNFG_REG32, reg);
        SNAND_WriteReg(ECC_DECCON_REG16, DEC_DE);
        SNAND_WriteReg(ECC_DECDIADDR_REG32, 0);
    }

unmap_and_cleanup:

    dma_unmap_sg(&(mtd->dev), &sg, 1, dir);

    mtk_snand_nfi_enable_bypass(0);

    return bRet;
}

//#define _SNAND_SUBPAGE_READ_DBG

bool mtk_nand_exec_read_subpage(struct mtd_info *mtd, u32 u4RowAddr, u32 sector_begin, u32 sector_cnt, u8 * pPageBuf, u8 * pFDMBuf)
{
    u8 * buf;
    bool bRet = 1;
    u8 retry;
    struct nand_chip *nand = mtd->priv;
    u32 u4SecNum = 1;
    u32 col_addr;
    u8  manual_sector_read = 0;
    u8  manual_sector_read_this_loop;
    u32 sector_to_read_this_loop;
    u32 sector_cnt_done = 0;
#ifdef NAND_PFM
    struct timeval pfm_time_read;
#endif

    PFM_BEGIN(pfm_time_read);

    if (((u32) pPageBuf % _SNAND_CACHE_LINE_SIZE) && local_buffer_cache_size_align)
    {
        buf = local_buffer_cache_size_align;
    }
    else
    {
        buf = pPageBuf;
    }

    mtk_snand_rs_reconfig_nfiecc(u4RowAddr);

    sector_to_read_this_loop = sector_cnt;

    manual_sector_read_this_loop = 0;

    if (((sector_begin + sector_cnt) * NAND_SECTOR_SIZE) == mtd->writesize)
    {
        if (mtk_snand_rs_if_require_split())
        {
            manual_sector_read = 1;

            if ((sector_begin + 1) * NAND_SECTOR_SIZE != mtd->writesize)
            {
                manual_sector_read_this_loop = 0;

                sector_to_read_this_loop--;
            }
            else
            {
                manual_sector_read_this_loop = 1;
            }
        }
    }

    while (sector_cnt)
    {
        col_addr = sector_begin * (NAND_SECTOR_SIZE + g_snand_k_spare_per_sector);

        #ifdef _SNAND_SUBPAGE_READ_DBG
        printk("[K-SNAND] rs,r %d,b %d,sector_begin %d,sector_cnt %d,manual_sector_read_this_loop %d,sector_to_read_this_loop %d\n", u4RowAddr, u4RowAddr/64, sector_begin, sector_cnt, manual_sector_read_this_loop, sector_to_read_this_loop);
        #endif

        retry = 0;

mtk_nand_exec_read_page_retry:

        bRet = 1;

        if (0 == manual_sector_read_this_loop)
        {
            if (0 != g_snand_rs_ecc_bit)
            {
                if (mtk_snand_ready_for_read_custom(nand, u4RowAddr, col_addr, sector_to_read_this_loop, buf, 1, 1, 1))
                {
                    if (!mtk_snand_read_page_data(mtd, buf, sector_to_read_this_loop, 1))
                    {
                        bRet = 0;
                    }

                    if (g_bHwEcc)
                    {
                        if (!mtk_snand_check_dececc_done(sector_to_read_this_loop))
                        {
                            bRet = 0;
                        }
                    }

                    mtk_snand_read_fdm_data(pFDMBuf, sector_to_read_this_loop);

                    if (g_bHwEcc)
                    {
                        if (!mtk_snand_check_bch_error(mtd, buf, pFDMBuf, sector_to_read_this_loop - 1, u4RowAddr))
                        {
                            bRet = 0;
                        }
                    }

                    mtk_snand_stop_read_custom(1);
                }
                else
                {
                    mtk_snand_stop_read_custom(0);
                }
            }
            else    // mtk_ecc off, use device ECC
            {
                // use internal buffer for read with device ECC on, then transform the format of data and spare

                if (mtk_snand_ready_for_read_custom(nand, u4RowAddr, col_addr, sector_to_read_this_loop, g_snand_k_temp, 0, 0, 1))   // MTK ECC off, AUTO FMT off, AHB on
                {
                    if (!mtk_snand_read_page_data(mtd, g_snand_k_temp, sector_to_read_this_loop, 1))
                    {
                        bRet = 0;
                    }

                    // copy data
                    memcpy((void *)buf, (void *)g_snand_k_temp, sector_to_read_this_loop * NAND_SECTOR_SIZE);

                    // copy spare
                    if (!mtk_snand_read_fdm_data_devecc(pFDMBuf, g_snand_k_temp, col_addr / NAND_SECTOR_SIZE, sector_to_read_this_loop))
                    {
                        bRet = 0;
                    }

                    mtk_snand_stop_read_custom(0);
                }
                else
                {
                    mtk_snand_stop_read_custom(0);

                    bRet = 0;
                }
            }

            if (0 == bRet)
            {
                if (retry <= 3)
                {
                    retry++;
                    printk(KERN_ERR "[%s] read retrying ... (the %d time)\n", __FUNCTION__, retry);
                    mtk_snand_reset_dev();

                    goto mtk_nand_exec_read_page_retry;
                }
            }
        }
        else    // manual read (for the last sector)
        {
            #ifdef _SNAND_SUBPAGE_READ_DBG
            if (sector_to_read_this_loop != 1)
            {
                printk("[K-SNAND][ERROR] sector_to_read_this_loop should be 1! (%d)\n", sector_to_read_this_loop);

                while (1);
            }
            #endif

            mtk_snand_ready_for_read_sector_custom(nand, u4RowAddr, col_addr);

            u4SecNum = mtd->writesize >> 9;

            // note: use local temp buffer to read part 2
            if (!mtk_snand_read_page_part2(mtd, u4RowAddr, u4SecNum, g_snand_k_temp))
            {
                bRet = 0;
            }
        }

        mtk_snand_dev_enable_spiq(0);

        // copy data

        if (1 == manual_sector_read_this_loop)
        {
            memcpy(buf + (sector_cnt_done * NAND_SECTOR_SIZE), g_snand_k_temp, NAND_SECTOR_SIZE);

            // copy FDM data

            memcpy(pFDMBuf + (sector_cnt_done * OOB_AVAI_PER_SECTOR), (g_snand_k_temp + NAND_SECTOR_SIZE), OOB_AVAI_PER_SECTOR);
        }

        if (buf == local_buffer_cache_size_align)
        {
            memcpy(pPageBuf + (sector_cnt_done * NAND_SECTOR_SIZE), buf, sector_to_read_this_loop * NAND_SECTOR_SIZE);
        }

        sector_cnt -= sector_to_read_this_loop;

        sector_begin += sector_to_read_this_loop;

        sector_cnt_done += sector_to_read_this_loop;

        if (manual_sector_read) // next loop should use manual sector read
        {
            manual_sector_read_this_loop = 1;

            sector_to_read_this_loop = 1;
        }

        #ifdef _SNAND_SUBPAGE_READ_DBG
        if (sector_cnt)
        {
            if (sector_cnt != 1)
            {
                printk("[K-SNAND][ERROR] sector_cnt should be 1! (%d)\n", sector_cnt);

                while (1);
            }

            if (sector_to_read_this_loop != 1)
            {
                printk("[K-SNAND][ERROR] sector_to_read_this_loop should be 1! (%d)\n", sector_to_read_this_loop);

                while (1);
            }

            if (manual_sector_read_this_loop != 1)
            {
                printk("[K-SNAND][ERROR] manual_sector_read_this_loop should be 1! (%d)\n", manual_sector_read_this_loop);

                while (1);
            }

            if ((sector_begin + 1) != (mtd->writesize / NAND_SECTOR_SIZE))
            {
                printk("[K-SNAND][ERROR] sector_begin should be %d! (%d)\n", sector_begin, (mtd->writesize / NAND_SECTOR_SIZE));

                while (1);
            }
        }
        #endif

        if (0 == bRet)  // read error, abort
        {
            #ifdef _SNAND_SUBPAGE_READ_DBG
            printk("[K-SNAND][ERROR] %s read error!\n", __FUNCTION__);
            while (1);
            #endif

            break;
        }
    }

    PFM_END_R(pfm_time_read, sector_cnt_done * (NAND_SECTOR_SIZE + OOB_AVAI_PER_SECTOR));

    return bRet;
}


/******************************************************************************
 * mtk_nand_exec_read_page
 *
 * DESCRIPTION:
 *   Read a page data !
 *
 * PARAMETERS:
 *   struct mtd_info *mtd, u32 u4RowAddr, u32 u4PageSize,
 *   u8* pPageBuf, u8* pFDMBuf
 *
 * RETURNS:
 *   None
 *
 * NOTES:
 *   None
 *
 ******************************************************************************/
int mtk_nand_exec_read_page(struct mtd_info *mtd, u32 u4RowAddr, u32 u4PageSize, u8 * pPageBuf, u8 * pFDMBuf)
{
    u8 * buf;
    u8 * p_buf_local;
    bool bRet = 1;
    u8 retry;
    struct nand_chip *nand = mtd->priv;
    u32 u4SecNum = u4PageSize >> 9;
#ifdef NAND_PFM
    struct timeval pfm_time_read;
#endif

    #ifdef CFG_SNAND_DRV_RW_BREAKDOWN
    do_gettimeofday(&g_stimer[0]);
    #endif

    PFM_BEGIN(pfm_time_read);

    if (((u32) pPageBuf % _SNAND_CACHE_LINE_SIZE) && local_buffer_cache_size_align)
    {
        buf = local_buffer_cache_size_align;

        #ifdef _SNAND_DEBUG
        //printk("[K-SNAND][%s] use local_buffer_cache_size_align!\n", __FUNCTION__);
        #endif
    }
    else
    {
        buf = pPageBuf;
    }

    mtk_snand_rs_reconfig_nfiecc(u4RowAddr);

    //printk("[K-SNAND][%s] p:%d, b:%d, s:4, part:%d, spare:%d, ecc:%d\n", __FUNCTION__, u4RowAddr, u4RowAddr / 64, g_snand_rs_cur_part, g_snand_k_spare_per_sector, g_snand_rs_ecc_bit);

    if (mtk_snand_rs_if_require_split())
    {
        u4SecNum--;
    }

    retry = 0;

mtk_nand_exec_read_page_retry:

    bRet = 1;

    #ifdef CFG_SNAND_DRV_RW_BREAKDOWN
    do_gettimeofday(&g_etimer[0]);
    #endif

    if (0 != g_snand_rs_ecc_bit)
    {
        if (mtk_snand_ready_for_read_custom(nand, u4RowAddr, 0, u4SecNum, buf, 1, 1, 1))
        {

            #ifdef CFG_SNAND_DRV_RW_BREAKDOWN
            do_gettimeofday(&g_etimer[1]);
            #endif

            if (!mtk_snand_read_page_data(mtd, buf, u4SecNum, 1))
            {
                bRet = 0;
            }

            #ifdef CFG_SNAND_DRV_RW_BREAKDOWN
            do_gettimeofday(&g_etimer[2]);
            #endif

            if (g_bHwEcc)
            {
                if (!mtk_snand_check_dececc_done(u4SecNum))
                {
                    bRet = 0;
                }
            }

            #ifdef CFG_SNAND_DRV_RW_BREAKDOWN
            do_gettimeofday(&g_etimer[3]);
            #endif

            mtk_snand_read_fdm_data(pFDMBuf, u4SecNum);

            if (g_bHwEcc)
            {
                if (!mtk_snand_check_bch_error(mtd, buf, pFDMBuf,u4SecNum - 1, u4RowAddr))
                {
                    bRet = 0;
                }
            }

            mtk_snand_stop_read_custom(1);

            #ifdef CFG_SNAND_DRV_RW_BREAKDOWN
            do_gettimeofday(&g_etimer[4]);
            #endif
        }
        else
        {
            bRet = 0;

            mtk_snand_stop_read_custom(0);
        }
    }   // use device ECC
    else
    {
        // use internal buffer for read with device ECC on, then transform the format of data and spare

        if (mtk_snand_ready_for_read_custom(nand, u4RowAddr, 0, u4SecNum, g_snand_k_temp, 0, 0, 1))   // MTK ECC off, AUTO FMT off, AHB on
        {
            if (!mtk_snand_read_page_data(mtd, g_snand_k_temp, u4SecNum, 1))
            {
                bRet = 0;
            }

            // copy data
            memcpy((void *)buf, (void *)g_snand_k_temp, NAND_SECTOR_SIZE * u4SecNum);

            // copy spare
            if (!mtk_snand_read_fdm_data_devecc(pFDMBuf, g_snand_k_temp, 0, u4SecNum))
            {
                bRet = 0;
            }

            mtk_snand_stop_read_custom(0);
        }
        else
        {
            mtk_snand_stop_read_custom(0);

            bRet = 0;
        }
    }

    if (0 == bRet)
    {
        if (retry <= 3)
        {
            retry++;
            printk(KERN_ERR "[%s] read retrying ... (the %d time)\n", __FUNCTION__, retry);
            mtk_snand_reset_dev();

            goto mtk_nand_exec_read_page_retry;
        }
    }

    if (mtk_snand_rs_if_require_split())
    {
        // read part II

        #ifdef CFG_SNAND_DRV_RW_BREAKDOWN
        do_gettimeofday(&g_etimer[5]);
        #endif

        //u4SecNum++;
        u4SecNum = u4PageSize >> 9;

        // note: use local temp buffer to read part 2
        if (!mtk_snand_read_page_part2(mtd, u4RowAddr, u4SecNum, g_snand_k_temp))
        {
            bRet = 0;
        }

        #ifdef CFG_SNAND_DRV_RW_BREAKDOWN
        do_gettimeofday(&g_etimer[6]);
        #endif

        mb();

        // copy data

        p_buf_local = buf + NAND_SECTOR_SIZE * (u4SecNum - 1);

        memcpy(p_buf_local, g_snand_k_temp, NAND_SECTOR_SIZE);

        mb();

        // copy FDM data

        p_buf_local = pFDMBuf + OOB_AVAI_PER_SECTOR * (u4SecNum - 1);

        memcpy(p_buf_local, (g_snand_k_temp + NAND_SECTOR_SIZE), OOB_AVAI_PER_SECTOR);

        #ifdef CFG_SNAND_DRV_RW_BREAKDOWN
        do_gettimeofday(&g_etimer[7]);
        #endif

    }

    mtk_snand_dev_enable_spiq(0);

    if (buf == local_buffer_cache_size_align)
    {
        memcpy(pPageBuf, buf, u4PageSize);
    }

    #ifdef CFG_SNAND_DRV_RW_BREAKDOWN
    do_gettimeofday(&g_etimer[8]);

    printk("perf-rp, r:%d (b:%d) 0:%d, 1:%d, 2:%d, 3:%d, 4:%d, 5:%d, 6:%d, 7:%d, F:%d, CPU:%d, BUS:%d, EMI:%d, GF3:0x%X, STA3:0x%X\n"
            ,u4RowAddr
            ,u4RowAddr / 64
            ,Cal_timediff(&g_etimer[0],&g_stimer[0])
            ,Cal_timediff(&g_etimer[1],&g_stimer[0])
            ,Cal_timediff(&g_etimer[2],&g_stimer[0])
            ,Cal_timediff(&g_etimer[3],&g_stimer[0])
            ,Cal_timediff(&g_etimer[4],&g_stimer[0])
            ,Cal_timediff(&g_etimer[5],&g_stimer[0])
            ,Cal_timediff(&g_etimer[6],&g_stimer[0])
            ,Cal_timediff(&g_etimer[7],&g_stimer[0])
            ,Cal_timediff(&g_etimer[8],&g_stimer[0])
            ,mt_get_cpu_freq()
            ,mt_get_bus_freq()
            ,mt_get_emi_freq()
            ,*(RW_SNAND_GF_CTL3)
            ,*(RW_SNAND_STA_CTL3)
            );
    #endif

    PFM_END_R(pfm_time_read, u4PageSize + 32);

    #ifdef _SNAND_DEBUG
    if (0 == bRet)
    {
        printk("[K-SNAND][%s] ERROR!\n", __FUNCTION__);
        //mtk_snand_dump_reg();
        //dump_stack();
        //while (1);
    }
    #endif

    return bRet;
}

/******************************************************************************
 * mtk_nand_exec_write_page
 *
 * DESCRIPTION:
 *   Write a page data !
 *
 * PARAMETERS:
 *   struct mtd_info *mtd, u32 u4RowAddr, u32 u4PageSize,
 *   u8* pPageBuf, u8* pFDMBuf
 *
 * RETURNS:
 *   None
 *
 * NOTES:
 *   None
 *
 ******************************************************************************/

static bool mtk_snand_dev_program_execute(u32 page)
{
    u32 cmd;
    bool bRet = 1;

    // 3. Program Execute

    g_snand_dev_status = SNAND_DEV_PROG_EXECUTING;
    g_snand_cmd_status = 0; // busy

    cmd = mtk_snand_gen_c1a3(SNAND_CMD_PROGRAM_EXECUTE, page);

    mtk_snand_dev_command(cmd, 4);

    return bRet;
}

int mtk_nand_exec_write_page(struct mtd_info *mtd, u32 u4RowAddr, u32 u4PageSize, u8 * pPageBuf, u8 * pFDMBuf)
{
    struct nand_chip *chip = mtd->priv;
    u32 u4SecNum = u4PageSize >> 9;
    u8 *buf;
    u8 status = 0;
    u8 wait_status = 0;

#ifdef _MTK_NAND_DUMMY_DRIVER_
    if (dummy_driver_debug)
    {
        unsigned long long time = sched_clock();
        if (!((time * 123 + 59) % 32768))
        {
            printk(KERN_INFO "[NAND_DUMMY_DRIVER] Simulate write error at page: 0x%x\n", u4RowAddr);
            return -EIO;
        }
    }
#endif

#ifdef NAND_PFM
    struct timeval pfm_time_write;
#endif

    PFM_BEGIN(pfm_time_write);

    if (((u32) pPageBuf % _SNAND_CACHE_LINE_SIZE) && local_buffer_cache_size_align)
    {
        memcpy(local_buffer_cache_size_align, pPageBuf, mtd->writesize);
        buf = local_buffer_cache_size_align;
    }
    else
    {
        buf = pPageBuf;
    }

    mtk_snand_rs_reconfig_nfiecc(u4RowAddr);

    if (0 != g_snand_rs_ecc_bit)
    {
        if (mtk_snand_ready_for_write(chip, u4RowAddr, 0, buf, 1, 1, 1))
        {
            mtk_snand_write_fdm_data(chip, pFDMBuf, u4SecNum);

            if (!mtk_snand_write_page_data(mtd, buf, u4PageSize, 1))
            {
                status |= NAND_STATUS_FAIL;
            }

            if (!mtk_snand_check_RW_count(u4PageSize))
            {
                status |= NAND_STATUS_FAIL;
            }

            mtk_snand_stop_write(1);

            {
                struct timeval stimer,etimer;
                do_gettimeofday(&stimer);

                mtk_snand_dev_program_execute(u4RowAddr);

                do_gettimeofday(&etimer);
                g_NandPerfLog.WriteBusyTotalTime+= Cal_timediff(&etimer,&stimer);
                g_NandPerfLog.WriteBusyCount++;
            }
        }
        else
        {
            mtk_snand_stop_write(0);

            status |= NAND_STATUS_FAIL;
        }
    }
    else    // mtk_ecc off, use device ECC
    {
        // use internal buffer for program with device ECC on

        // prepare page data
        memcpy((void *)g_snand_k_temp, (void *)pPageBuf, devinfo.pagesize);

        // prepare spare data
        mtk_snand_write_fdm_data_devecc(&(g_snand_k_temp[devinfo.pagesize]), pFDMBuf, u4SecNum);

        if (mtk_snand_ready_for_write(chip, u4RowAddr, 0, buf, 0, 0, 1))
        {
            if (!mtk_snand_write_page_data(mtd, buf, u4PageSize, 1))
            {
                status |= NAND_STATUS_FAIL;
            }

            if (!mtk_snand_check_RW_count(u4PageSize))
            {
                status |= NAND_STATUS_FAIL;
            }

            mtk_snand_stop_write(0);

            mtk_snand_dev_program_execute(u4RowAddr);
        }
        else
        {
            mtk_snand_stop_write(0);

            status |= NAND_STATUS_FAIL;
        }
    }

    PFM_END_W(pfm_time_write, u4PageSize + 32);

    wait_status = chip->waitfunc(mtd, chip); // return NAND_STATUS_READY or return NAND_STATUS_FAIL

    if ((status & NAND_STATUS_FAIL) || (wait_status & NAND_STATUS_FAIL))
    {
        return -EIO;
    }
    else
    {
        return 0;
    }
}

/******************************************************************************
 *
 * Write a page to a logical address
 *
 * Return values
 *  0: No error
 *  1: Error
 *
 *****************************************************************************/
static int mtk_snand_write_page(struct mtd_info *mtd, struct nand_chip *chip, const u8 * buf, int page, int cached, int raw)
{
    int page_per_block = 1 << (chip->phys_erase_shift - chip->page_shift);
    int block = page / page_per_block;
    u16 page_in_block = page % page_per_block;
    int mapped_block = get_mapping_block_index(block);
    struct timeval stimer,etimer;
    do_gettimeofday(&stimer);

    if (1 == mtk_snand_is_vendor_reserved_blocks(page_in_block + mapped_block * page_per_block))  // return write error for reserved blocks
    {
        return 1;
    }

    // write bad index into oob
    if (mapped_block != block)
    {
        set_bad_index_to_oob(chip->oob_poi, block);
    } else
    {
        set_bad_index_to_oob(chip->oob_poi, FAKE_INDEX);
    }

    if (mtk_nand_exec_write_page(mtd, page_in_block + mapped_block * page_per_block, mtd->writesize, (u8 *) buf, chip->oob_poi))
    {
        MSG(INIT, "write fail at block: 0x%x, page: 0x%x\n", mapped_block, page_in_block);
        if (update_bmt((page_in_block + mapped_block * page_per_block) << chip->page_shift, UPDATE_WRITE_FAIL, (u8 *) buf, chip->oob_poi))
        {
            MSG(INIT, "Update BMT success\n");
            return 0;
        } else
        {
            MSG(INIT, "Update BMT fail\n");
            return -EIO;
        }
    }

    do_gettimeofday(&etimer);
    g_NandPerfLog.WritePageTotalTime+= Cal_timediff(&etimer,&stimer);
    g_NandPerfLog.WritePageCount++;

    #ifdef CFG_SNAND_ACCESS_PATTERN_LOGGER
    if (g_snand_dbg_ap_on == 1)
    {
        mtk_snand_pm_add_drv_record(_SNAND_PM_OP_PROGRAM, page_in_block + mapped_block * page_per_block, 0, Cal_timediff(&etimer,&stimer));
    }
    #endif

    return 0;
}

//-------------------------------------------------------------------------------
/*
static void mtk_nand_command_sp(
	struct mtd_info *mtd, unsigned int command, int column, int page_addr)
{
	g_u4ColAddr	= column;
	g_u4RowAddr	= page_addr;

	switch(command)
	{
	case NAND_CMD_STATUS:
		break;

	case NAND_CMD_READID:
		break;

	case NAND_CMD_RESET:
		break;

	case NAND_CMD_RNDOUT:
	case NAND_CMD_RNDOUTSTART:
	case NAND_CMD_RNDIN:
	case NAND_CMD_CACHEDPROG:
	case NAND_CMD_STATUS_MULTI:
	default:
		break;
	}

}
*/

static void mtk_snand_dev_read_id(u8 id[])
{
    u8 cmd = SNAND_CMD_READ_ID;

    mtk_snand_dev_command_ext(SPI, &cmd, id, 1, SNAND_MAX_ID + 1);
}

/******************************************************************************
 * mtk_snand_command_bp
 *
 * DESCRIPTION:
 *   Handle the commands from MTD !
 *
 * PARAMETERS:
 *   struct mtd_info *mtd, unsigned int command, int column, int page_addr
 *
 * RETURNS:
 *   None
 *
 * NOTES:
 *   None
 *
 ******************************************************************************/
static void mtk_snand_command_bp(struct mtd_info *mtd, unsigned int command, int column, int page_addr)
{
    struct nand_chip *nand = mtd->priv;
#ifdef NAND_PFM
    struct timeval pfm_time_erase;
#endif
    switch (command)
    {
      case NAND_CMD_SEQIN:
          memset(g_kCMD.au1OOB, 0xFF, sizeof(g_kCMD.au1OOB));
          g_kCMD.pDataBuf = NULL;
          //}
          g_kCMD.u4RowAddr = page_addr;
          g_kCMD.u4ColAddr = column;
          break;

      case NAND_CMD_PAGEPROG:
          if (g_kCMD.pDataBuf || (0xFF != g_kCMD.au1OOB[0]))
          {
              u8 *pDataBuf = g_kCMD.pDataBuf ? g_kCMD.pDataBuf : nand->buffers->databuf;
              mtk_nand_exec_write_page(mtd, g_kCMD.u4RowAddr, mtd->writesize, pDataBuf, g_kCMD.au1OOB);
              g_kCMD.u4RowAddr = (u32) - 1;
              g_kCMD.u4OOBRowAddr = (u32) - 1;
          }
          break;

      case NAND_CMD_READOOB:
          g_kCMD.u4RowAddr = page_addr;
          g_kCMD.u4ColAddr = column + mtd->writesize;
#ifdef NAND_PFM
          g_kCMD.pureReadOOB = 1;
          g_kCMD.pureReadOOBNum += 1;
#endif
          break;

      case NAND_CMD_READ0:
          g_kCMD.u4RowAddr = page_addr;
          g_kCMD.u4ColAddr = column;
#ifdef NAND_PFM
          g_kCMD.pureReadOOB = 0;
#endif
          break;

      case NAND_CMD_ERASE1:

      	  #if 0
          PFM_BEGIN(pfm_time_erase);
          (void)mtk_snand_reset_con();

          mtk_snand_set_mode(CNFG_OP_ERASE);
          (void)mtk_nand_set_command(NAND_CMD_ERASE1);
          (void)mtk_nand_set_address(0, page_addr, 0, devinfo.addr_cycle - 2);
          #else
          printk(KERN_ERR "[SNAND-ERROR] Not allowed NAND_CMD_ERASE1!\n");
          #endif

          break;

      case NAND_CMD_ERASE2:

      	  #if 0
          (void)mtk_nand_set_command(NAND_CMD_ERASE2);
          while (*(NFI_STA_REG32) & STA_NAND_BUSY) ;
          PFM_END_E(pfm_time_erase);
          #else
          printk(KERN_ERR "[SNAND-ERROR] Not allowed NAND_CMD_ERASE2!\n"	);
          #endif

          break;

      case NAND_CMD_STATUS:

			if ((SNAND_DEV_ERASE_DONE == g_snand_dev_status) ||
				(SNAND_DEV_PROG_DONE == g_snand_dev_status))
			{
				g_snand_dev_status = SNAND_IDLE;	// reset command status
			}

            g_snand_read_byte_mode = SNAND_RB_CMD_STATUS;

          break;

      case NAND_CMD_RESET:

            (void)mtk_snand_reset_con();

            g_snand_read_byte_mode = SNAND_RB_DEFAULT;

            break;

      case NAND_CMD_READID:

            mtk_snand_reset_dev();
            mtk_snand_reset_con();

            mb();

            mtk_snand_dev_read_id(g_snand_id_data);

            g_snand_id_data_idx = 1;

            g_snand_read_byte_mode = SNAND_RB_READ_ID;

            break;

      default:
          BUG();
          break;
    }
}

/******************************************************************************
 * mtk_snand_select_chip
 *
 * DESCRIPTION:
 *   Select a chip !
 *
 * PARAMETERS:
 *   struct mtd_info *mtd, int chip
 *
 * RETURNS:
 *   None
 *
 * NOTES:
 *   None
 *
 ******************************************************************************/
static void mtk_snand_select_chip(struct mtd_info *mtd, int chip)
{
    if (chip == -1 && 0 == g_bInitDone)
    {
        struct nand_chip *nand = mtd->priv;

    	struct mtk_nand_host *host = nand->priv;
    	struct mtk_nand_host_hw *hw = host->hw;
    	u32 spare_per_sector = mtd->oobsize/( mtd->writesize/512);
    	u32 ecc_bit = 4;
    	u32 spare_bit = PAGEFMT_SPARE_16;

        if(spare_per_sector>=32){
    		spare_bit = PAGEFMT_SPARE_32;
    		ecc_bit = 12;
    		spare_per_sector = 32;
      	}else if(spare_per_sector>=28){
    		spare_bit = PAGEFMT_SPARE_28;
    		ecc_bit = 8;
    		spare_per_sector = 28;
      	}else if(spare_per_sector>=27){
      		spare_bit = PAGEFMT_SPARE_27;
        		ecc_bit = 8;
     		spare_per_sector = 27;
      	}else if(spare_per_sector>=26){
      		spare_bit = PAGEFMT_SPARE_26;
        		ecc_bit = 8;
    		spare_per_sector = 26;
      	}else if(spare_per_sector>=16){
      		spare_bit = PAGEFMT_SPARE_16;
        		ecc_bit = 4;
    		spare_per_sector = 16;
      	}else{
      		MSG(INIT, "[NAND]: NFI not support oobsize: %x\n", spare_per_sector);
        		ASSERT(0);
      	}

      	 mtd->oobsize = spare_per_sector*(mtd->writesize/512);
      	 printk("[NAND]select ecc bit:%d, sparesize :%d\n",ecc_bit,mtd->oobsize);

        /* Setup PageFormat */
        if (4096 == mtd->writesize)
        {
            SNAND_REG_SET_BITS(NFI_PAGEFMT_REG16, (spare_bit << PAGEFMT_SPARE_SHIFT) | PAGEFMT_4K | PAGEFMT_SEC_SEL_512);
            nand->cmdfunc = mtk_snand_command_bp;
        } else if (2048 == mtd->writesize)
        {
            SNAND_REG_SET_BITS(NFI_PAGEFMT_REG16, (spare_bit << PAGEFMT_SPARE_SHIFT) | PAGEFMT_2K | PAGEFMT_SEC_SEL_512);
            nand->cmdfunc = mtk_snand_command_bp;
        }

    	mtk_snand_ecc_config(hw,ecc_bit);

        g_snand_rs_ecc_bit_second_part = ecc_bit;
        g_snand_rs_spare_per_sector_second_part_nfi = (spare_bit << PAGEFMT_SPARE_SHIFT);
        g_snand_k_spare_per_sector = spare_per_sector;

        g_bInitDone = 1;
    }
    switch (chip)
    {
      case -1:
          break;
      case 0:
#if defined(CONFIG_EARLY_LINUX_PORTING)		// FPGA NAND is placed at CS1 not CS0
			//SNAND_WriteReg(NFI_CSEL_REG16, 1);
			break;
#endif
      case 1:
          //SNAND_WriteReg(NFI_CSEL_REG16, chip);
          break;
    }
}

/******************************************************************************
 * mtk_snand_read_byte
 *
 * DESCRIPTION:
 *   Read a byte of data !
 *
 * PARAMETERS:
 *   struct mtd_info *mtd
 *
 * RETURNS:
 *   None
 *
 * NOTES:
 *   None
 *
 ******************************************************************************/
static uint8_t mtk_snand_read_byte(struct mtd_info *mtd)
{
    u8      reg8;
    u32     reg32;

    if (SNAND_RB_READ_ID == g_snand_read_byte_mode)
    {
        if (g_snand_id_data_idx > SNAND_MAX_ID)
        {
            return 0;
        }
        else
        {
            return g_snand_id_data[g_snand_id_data_idx++];
        }
    }
    else if (SNAND_RB_CMD_STATUS == g_snand_read_byte_mode) // get feature to see the status of program and erase (e.g., nand_wait)
    {
		if ((SNAND_DEV_ERASE_DONE == g_snand_dev_status) ||
			(SNAND_DEV_PROG_DONE == g_snand_dev_status))
		{
			return g_snand_cmd_status | NAND_STATUS_WP;	// report the latest device operation status (program OK, erase OK, program failed, erase failed ...etc)
		}
		else if (SNAND_DEV_PROG_EXECUTING == g_snand_dev_status)    // check ready again
        {
            SNAND_WriteReg(RW_SNAND_GPRAM_DATA, (SNAND_CMD_GET_FEATURES | (SNAND_CMD_FEATURES_STATUS << 8)));
            SNAND_WriteReg(RW_SNAND_MAC_OUTL, 2);
            SNAND_WriteReg(RW_SNAND_MAC_INL , 1);

            mtk_snand_dev_mac_op(SPI);

            reg8 = DRV_Reg8(((P_U8)RW_SNAND_GPRAM_DATA + 2));

            if (0 == (reg8 & SNAND_STATUS_OIP)) // ready
            {
            	g_snand_dev_status = SNAND_DEV_PROG_DONE;

                if (0 != (reg8 & SNAND_STATUS_PROGRAM_FAIL)) // ready but having fail report from device
                {
                	MSG(INIT, "[snand] Prog failed!\n");	// Stanley Chu (add more infomation like page address)

    				g_snand_cmd_status = NAND_STATUS_READY | NAND_STATUS_FAIL | NAND_STATUS_WP;
                }
                else
                {
    				g_snand_cmd_status = NAND_STATUS_READY | NAND_STATUS_WP;
                }

                return g_snand_cmd_status;
            }
            else
            {
                return NAND_STATUS_WP;	// busy
            }
        }
        else if (SNAND_NFI_AUTO_ERASING == g_snand_dev_status)  // check ready again
        {
            // wait for auto erase finish

            reg32 = *(RW_SNAND_STA_CTL1);

            if ((reg32 & SNAND_AUTO_BLKER) == 0)
            {
                return NAND_STATUS_WP;	// busy
            }
            else
            {
            	g_snand_dev_status = SNAND_DEV_ERASE_DONE;

                reg8 = (u8)(*(RW_SNAND_GF_CTL1) & SNAND_GF_STATUS_MASK);

                if (0 != (reg8 & SNAND_STATUS_ERASE_FAIL)) // ready but having fail report from device
                {
                	MSG(INIT, "[snand] Erase failed!\n");  // Stanley Chu (add more infomation like page address)

    				g_snand_cmd_status = NAND_STATUS_READY | NAND_STATUS_FAIL | NAND_STATUS_WP;
                }
                else
                {
    				g_snand_cmd_status = NAND_STATUS_READY | NAND_STATUS_WP;
                }

                return g_snand_cmd_status;    // return non-0
            }
        }
        else if ((SNAND_NFI_CUST_READING == g_snand_dev_status))
		{
		    g_snand_cmd_status = NAND_STATUS_WP; // busy

			return g_snand_cmd_status;
		}
        else    // idle
        {
            g_snand_dev_status = SNAND_IDLE;

            g_snand_cmd_status = (NAND_STATUS_READY | NAND_STATUS_WP);

            return g_snand_cmd_status;    // return NAND_STATUS_WP to indicate SPI-NAND does NOT WP
        }
    }
    else
    {
        return 0;
    }
}

/******************************************************************************
 * mtk_snand_read_buf
 *
 * DESCRIPTION:
 *   Read NAND data !
 *
 * PARAMETERS:
 *   struct mtd_info *mtd, uint8_t *buf, int len
 *
 * RETURNS:
 *   None
 *
 * NOTES:
 *   None
 *
 ******************************************************************************/
static void mtk_snand_read_buf(struct mtd_info *mtd, uint8_t * buf, int len)
{
    struct nand_chip *nand = (struct nand_chip *)mtd->priv;
    struct NAND_CMD *pkCMD = &g_kCMD;
    u32 u4ColAddr = pkCMD->u4ColAddr;
    u32 u4PageSize = mtd->writesize;

    if (u4ColAddr < u4PageSize)
    {
        if ((u4ColAddr == 0) && (len >= u4PageSize))
        {
            mtk_nand_exec_read_page(mtd, pkCMD->u4RowAddr, u4PageSize, buf, pkCMD->au1OOB);
            if (len > u4PageSize)
            {
                u32 u4Size = min(len - u4PageSize, sizeof(pkCMD->au1OOB));
                memcpy(buf + u4PageSize, pkCMD->au1OOB, u4Size);
            }
        } else
        {
            mtk_nand_exec_read_page(mtd, pkCMD->u4RowAddr, u4PageSize, nand->buffers->databuf, pkCMD->au1OOB);
            memcpy(buf, nand->buffers->databuf + u4ColAddr, len);
        }
        pkCMD->u4OOBRowAddr = pkCMD->u4RowAddr;
    } else
    {
        u32 u4Offset = u4ColAddr - u4PageSize;
        u32 u4Size = min(len - u4Offset, sizeof(pkCMD->au1OOB));
        if (pkCMD->u4OOBRowAddr != pkCMD->u4RowAddr)
        {
            mtk_nand_exec_read_page(mtd, pkCMD->u4RowAddr, u4PageSize, nand->buffers->databuf, pkCMD->au1OOB);
            pkCMD->u4OOBRowAddr = pkCMD->u4RowAddr;
        }
        memcpy(buf, pkCMD->au1OOB + u4Offset, u4Size);
    }
    pkCMD->u4ColAddr += len;
}

/******************************************************************************
 * mtk_snand_write_buf
 *
 * DESCRIPTION:
 *   Write NAND data !
 *
 * PARAMETERS:
 *   struct mtd_info *mtd, const uint8_t *buf, int len
 *
 * RETURNS:
 *   None
 *
 * NOTES:
 *   None
 *
 ******************************************************************************/
static void mtk_snand_write_buf(struct mtd_info *mtd, const uint8_t * buf, int len)
{
    struct NAND_CMD *pkCMD = &g_kCMD;
    u32 u4ColAddr = pkCMD->u4ColAddr;
    u32 u4PageSize = mtd->writesize;
    int i4Size, i;

    if (u4ColAddr >= u4PageSize)
    {
        u32 u4Offset = u4ColAddr - u4PageSize;
        u8 *pOOB = pkCMD->au1OOB + u4Offset;
        i4Size = min(len, (int)(sizeof(pkCMD->au1OOB) - u4Offset));

        for (i = 0; i < i4Size; i++)
        {
            pOOB[i] &= buf[i];
        }
    } else
    {
        pkCMD->pDataBuf = (u8 *) buf;
    }

    pkCMD->u4ColAddr += len;
}

/******************************************************************************
 * mtk_snand_write_page_hwecc
 *
 * DESCRIPTION:
 *   Write NAND data with hardware ecc !
 *
 * PARAMETERS:
 *   struct mtd_info *mtd, struct nand_chip *chip, const uint8_t *buf
 *
 * RETURNS:
 *   None
 *
 * NOTES:
 *   None
 *
 ******************************************************************************/
static void mtk_snand_write_page_hwecc(struct mtd_info *mtd, struct nand_chip *chip, const uint8_t * buf)
{
    mtk_snand_write_buf(mtd, buf, mtd->writesize);
    mtk_snand_write_buf(mtd, chip->oob_poi, mtd->oobsize);
}

/******************************************************************************
 * mtk_snand_read_page_hwecc
 *
 * DESCRIPTION:
 *   Read NAND data with hardware ecc !
 *
 * PARAMETERS:
 *   struct mtd_info *mtd, struct nand_chip *chip, uint8_t *buf
 *
 * RETURNS:
 *   None
 *
 * NOTES:
 *   None
 *
 ******************************************************************************/
static int mtk_snand_read_page_hwecc(struct mtd_info *mtd, struct nand_chip *chip, uint8_t * buf, int page)
{
#if 0
    mtk_snand_read_buf(mtd, buf, mtd->writesize);
    mtk_snand_read_buf(mtd, chip->oob_poi, mtd->oobsize);
#else
    struct NAND_CMD *pkCMD = &g_kCMD;
    u32 u4ColAddr = pkCMD->u4ColAddr;
    u32 u4PageSize = mtd->writesize;

    if (u4ColAddr == 0)
    {
        mtk_nand_exec_read_page(mtd, pkCMD->u4RowAddr, u4PageSize, buf, chip->oob_poi);
        pkCMD->u4ColAddr += u4PageSize + mtd->oobsize;
    }
#endif
    return 0;
}

static int mtk_snand_read_subpage(struct mtd_info *mtd, struct nand_chip *chip, u8 * buf, int page, int subpage_begin, int subpage_cnt)
{
    int page_per_block = 1 << (chip->phys_erase_shift - chip->page_shift);
    int block = page / page_per_block;
    u16 page_in_block = page % page_per_block;
    int mapped_block = get_mapping_block_index(block);
    struct timeval stimer,etimer;
    do_gettimeofday(&stimer);

    if (mtk_nand_exec_read_subpage(mtd, page_in_block + mapped_block * page_per_block, subpage_begin, subpage_cnt, buf, chip->oob_poi))
    {
        do_gettimeofday(&etimer);
        g_NandPerfLog.ReadSectorTotalTime+= Cal_timediff(&etimer,&stimer);
        g_NandPerfLog.ReadSectorCount++;

        #ifdef CFG_SNAND_ACCESS_PATTERN_LOGGER
        if (g_snand_dbg_ap_on == 1)
        {
            mtk_snand_pm_add_drv_record(_SNAND_PM_OP_READ_SEC, page_in_block + mapped_block * page_per_block, subpage_begin, Cal_timediff(&etimer,&stimer));
        }
        #endif

        return 0;
    }
    /* else
       return -EIO; */
    return 0;
}

/******************************************************************************
 *
 * Read a page to a logical address
 *
 *****************************************************************************/
static int mtk_snand_read_page(struct mtd_info *mtd, struct nand_chip *chip, u8 * buf, int page)
{
    int page_per_block = 1 << (chip->phys_erase_shift - chip->page_shift);
    int block = page / page_per_block;
    u16 page_in_block = page % page_per_block;
    int mapped_block = get_mapping_block_index(block);
    struct timeval stimer,etimer;
    do_gettimeofday(&stimer);

    if (mtk_nand_exec_read_page(mtd, page_in_block + mapped_block * page_per_block, mtd->writesize, buf, chip->oob_poi))
    {
        do_gettimeofday(&etimer);
        g_NandPerfLog.ReadPageTotalTime+= Cal_timediff(&etimer,&stimer);
        g_NandPerfLog.ReadPageCount++;

        #ifdef CFG_SNAND_ACCESS_PATTERN_LOGGER
        if (g_snand_dbg_ap_on == 1)
        {
            mtk_snand_pm_add_drv_record(_SNAND_PM_OP_READ_PAGE, page_in_block + mapped_block * page_per_block, 0, Cal_timediff(&etimer,&stimer));
        }
        #endif

        return 0;
    }
    /* else
       return -EIO; */
    return 0;
}

/******************************************************************************
 *
 * Erase a block at a logical address
 *
 *****************************************************************************/

static void mtk_snand_dev_stop_erase(void)
{
    u32 reg;

    // set 1 then set 0 to clear done flag
    reg = *(RW_SNAND_STA_CTL1);

    SNAND_WriteReg(RW_SNAND_STA_CTL1, reg);
    reg = reg & ~SNAND_AUTO_BLKER;
    SNAND_WriteReg(RW_SNAND_STA_CTL1, reg);

    // clear trigger bit
    reg = *(RW_SNAND_ER_CTL);
    reg &= ~SNAND_AUTO_ERASE_TRIGGER;
    SNAND_WriteReg(RW_SNAND_ER_CTL, reg);

    g_snand_dev_status = SNAND_IDLE;
}

static void mtk_snand_dev_erase(u32 row_addr)   // auto erase
{
    u32  reg;

    #ifdef _SNAND_DEBUG
    //if (row_addr == (9217 / 64) || row_addr == (9219 / 64))
    {
        printk("[K-SNAND][%s] row:%d, blk:%d\n", __FUNCTION__, row_addr, row_addr / 64);
        //dump_stack();
    }
    #endif

    mtk_snand_reset_con();

    // erase address
    SNAND_WriteReg(RW_SNAND_ER_CTL2, row_addr);

    // set loop limit and polling cycles
    reg = (SNAND_LOOP_LIMIT_NO_LIMIT << SNAND_LOOP_LIMIT_OFFSET) | 0x20;
    SNAND_WriteReg(RW_SNAND_GF_CTL3, reg);

    // set latch latency & CS de-select latency (ignored)

    // set erase command
    reg = SNAND_CMD_BLOCK_ERASE << SNAND_ER_CMD_OFFSET;
    SNAND_WriteReg(RW_SNAND_ER_CTL, reg);

    // trigger interrupt waiting
    reg = *(NFI_INTR_EN_REG16);
    reg = INTR_AUTO_BLKER_INTR_EN;
    SNAND_WriteReg(NFI_INTR_EN_REG16, reg);

    // trigger auto erase
    reg = *(RW_SNAND_ER_CTL);
    reg |= SNAND_AUTO_ERASE_TRIGGER;
    SNAND_WriteReg(RW_SNAND_ER_CTL, reg);

    g_snand_dev_status = SNAND_NFI_AUTO_ERASING;
    g_snand_cmd_status = 0; // busy
}

int mtk_nand_erase_hw(struct mtd_info *mtd, int page)
{
    struct nand_chip *chip = (struct nand_chip *)mtd->priv;
    int ret;

#ifdef _MTK_NAND_DUMMY_DRIVER_
    if (dummy_driver_debug)
    {
        unsigned long long time = sched_clock();
        if (!((time * 123 + 59) % 1024))
        {
            printk(KERN_INFO "[NAND_DUMMY_DRIVER] Simulate erase error at page: 0x%x\n", page);
            return NAND_STATUS_FAIL;
        }
    }
#endif

    if (1 == mtk_snand_is_vendor_reserved_blocks(page))  // return erase failed for reserved blocks
    {
        return NAND_STATUS_FAIL;
    }

    mtk_snand_dev_erase(page);

    ret = chip->waitfunc(mtd, chip);

    // FIXME: debug
    if (ret & NAND_STATUS_FAIL)
    {
        printk("[K-SNAND][%s] Erase blk %d failed!\n", __FUNCTION__, page / 64);
    }

    mtk_snand_dev_stop_erase();

    return ret;
}

static int mtk_nand_erase(struct mtd_info *mtd, int page)
{
    // get mapping
    struct nand_chip *chip = mtd->priv;
    int page_per_block = 1 << (chip->phys_erase_shift - chip->page_shift);
    int page_in_block = page % page_per_block;
    int block = page / page_per_block;
	struct timeval stimer,etimer;
    int mapped_block;
    int status;

    mapped_block = get_mapping_block_index(block);

    do_gettimeofday(&stimer);

    status = mtk_nand_erase_hw(mtd, page_in_block + page_per_block * mapped_block);

    if (status & NAND_STATUS_FAIL)
    {
        if (update_bmt((page_in_block + mapped_block * page_per_block) << chip->page_shift, UPDATE_ERASE_FAIL, NULL, NULL))
        {
            MSG(INIT, "Erase fail at block: 0x%x, update BMT success\n", mapped_block);
            return 0;
        } else
        {
            MSG(INIT, "Erase fail at block: 0x%x, update BMT fail\n", mapped_block);
            return NAND_STATUS_FAIL;
        }
    }
    do_gettimeofday(&etimer);
    g_NandPerfLog.EraseBlockTotalTime+= Cal_timediff(&etimer,&stimer);
    g_NandPerfLog.EraseBlockCount++;

    #ifdef CFG_SNAND_ACCESS_PATTERN_LOGGER
    if (g_snand_dbg_ap_on == 1)
    {
        mtk_snand_pm_add_drv_record(_SNAND_PM_OP_ERASE, page_in_block + page_per_block * mapped_block, 0, Cal_timediff(&etimer,&stimer));
    }
    #endif

    return 0;
}

/******************************************************************************
 * mtk_nand_read_multi_page_cache
 *
 * description:
 *   read multi page data using cache read
 *
 * parameters:
 *   struct mtd_info *mtd, struct nand_chip *chip, int page, struct mtd_oob_ops *ops
 *
 * returns:
 *   none
 *
 * notes:
 *   only available for nand flash support cache read.
 *   read main data only.
 *
 *****************************************************************************/
#define _GPIO_MODE9         ((volatile P_U32)(GPIO_BASE + 0x390))

void mtk_snand_dump_reg(void)
{
    printk("~~~~Dump NFI/SNF/GPIO Register in Kernel~~~~\n");
    printk("NFI_CNFG_REG16: 0x%x\n", *(NFI_CNFG_REG16));
    printk("NFI_PAGEFMT_REG16: 0x%x\n", *(NFI_PAGEFMT_REG16));
    printk("NFI_CON_REG16: 0x%x\n", *(NFI_CON_REG16));
    printk("NFI_INTR_EN_REG16: 0x%x\n", *(NFI_INTR_EN_REG16));
    printk("NFI_INTR_REG16: 0x%x\n", *(NFI_INTR_REG16));
    printk("NFI_CMD_REG16: 0x%x\n", *(NFI_CMD_REG16));
    printk("NFI_ROWADDR_REG32: 0x%x\n", *(NFI_ROWADDR_REG32));
    printk("NFI_STRDATA_REG16: 0x%x\n", *(NFI_STRDATA_REG16));
    printk("NFI_PIO_DIRDY_REG16: 0x%x\n", *(NFI_PIO_DIRDY_REG16));
    printk("NFI_STA_REG32: 0x%x\n", *(NFI_STA_REG32));
    printk("NFI_FIFOSTA_REG16: 0x%x\n", *(NFI_FIFOSTA_REG16));
    printk("NFI_ADDRCNTR_REG16: 0x%x\n", *(NFI_ADDRCNTR_REG16));
    printk("NFI_STRADDR_REG32: 0x%x\n", *(NFI_STRADDR_REG32));
    printk("NFI_BYTELEN_REG16: 0x%x\n", *(NFI_BYTELEN_REG16));
    printk("NFI_CSEL_REG16: 0x%x\n", *(NFI_CSEL_REG16));
    printk("NFI_FDM0L_REG32: 0x%x\n", *(NFI_FDM0L_REG32));
    printk("NFI_FDM0M_REG32: 0x%x\n", *(NFI_FDM0M_REG32));
    printk("NFI_FIFODATA0_REG32: 0x%x\n", *(NFI_FIFODATA0_REG32));
    printk("NFI_FIFODATA1_REG32: 0x%x\n", *(NFI_FIFODATA1_REG32));
    printk("NFI_FIFODATA2_REG32: 0x%x\n", *(NFI_FIFODATA2_REG32));
    printk("NFI_FIFODATA3_REG32: 0x%x\n", *(NFI_FIFODATA3_REG32));
    printk("NFI_MASTERSTA_REG16: 0x%x\n", *(NFI_MASTERSTA_REG16));

    printk("NFI_DEBUG_CON1_REG16: 0x%x\n", *(NFI_DEBUG_CON1_REG16));

    printk("ECC_DECCNFG_REG32: 0x%x\n", *(ECC_DECCNFG_REG32));
    printk("ECC_DECENUM0_REG32: 0x%x\n", *(ECC_DECENUM0_REG32));
    printk("ECC_DECENUM1_REG32: 0x%x\n", *(ECC_DECENUM1_REG32));
    printk("ECC_DECDONE_REG16: 0x%x\n", *(ECC_DECDONE_REG16));
    printk("ECC_ENCCNFG_REG32: 0x%x\n", *(ECC_ENCCNFG_REG32));

	printk("RW_SNAND_MAC_CTL: 0x%x\n", *(RW_SNAND_MAC_CTL));
	printk("RW_SNAND_MAC_OUTL: 0x%x\n", *(RW_SNAND_MAC_OUTL));
	printk("RW_SNAND_MAC_INL: 0x%x\n", *(RW_SNAND_MAC_INL));

	printk("RW_SNAND_RD_CTL1: 0x%x\n", *(RW_SNAND_RD_CTL1));
	printk("RW_SNAND_RD_CTL2: 0x%x\n", *(RW_SNAND_RD_CTL2));
	printk("RW_SNAND_RD_CTL3: 0x%x\n", *(RW_SNAND_RD_CTL3));

	printk("RW_SNAND_GF_CTL1: 0x%x\n", *(RW_SNAND_GF_CTL1));
	printk("RW_SNAND_GF_CTL3: 0x%x\n", *(RW_SNAND_GF_CTL3));

	printk("RW_SNAND_PG_CTL1: 0x%x\n", *(RW_SNAND_PG_CTL1));
	printk("RW_SNAND_PG_CTL2: 0x%x\n", *(RW_SNAND_PG_CTL2));
	printk("RW_SNAND_PG_CTL3: 0x%x\n", *(RW_SNAND_PG_CTL3));

	printk("RW_SNAND_ER_CTL: 0x%x\n", *(RW_SNAND_ER_CTL));
	printk("RW_SNAND_ER_CTL2: 0x%x\n", *(RW_SNAND_ER_CTL2));

	printk("RW_SNAND_MISC_CTL: 0x%x\n", *(RW_SNAND_MISC_CTL));
	printk("RW_SNAND_MISC_CTL2: 0x%x\n", *(RW_SNAND_MISC_CTL2));

	printk("RW_SNAND_DLY_CTL1: 0x%x\n", *(RW_SNAND_DLY_CTL1));
	printk("RW_SNAND_DLY_CTL2: 0x%x\n", *(RW_SNAND_DLY_CTL2));
	printk("RW_SNAND_DLY_CTL3: 0x%x\n", *(RW_SNAND_DLY_CTL3));
	printk("RW_SNAND_DLY_CTL4: 0x%x\n", *(RW_SNAND_DLY_CTL4));

	printk("RW_SNAND_STA_CTL1: 0x%x\n", *(RW_SNAND_STA_CTL1));

	printk("RW_SNAND_CNFG: 0x%x\n", *(RW_SNAND_CNFG));

	printk("RW_DRV_CFG: 0x%x\n", *(RW_DRV_CFG));

	printk("GPIO_MODE9: 0x%x\n", *(_GPIO_MODE9));

	dump_clk_info_by_id(MT_CG_NFI_SW_CG);
	dump_clk_info_by_id(MT_CG_SPINFI_SW_CG);
	dump_clk_info_by_id(MT_CG_NFI_HCLK_SW_CG);
}

void mtk_snand_dump_mem(u32 * buf, u32 size)
{
    u32 i;

    for (i = 0; i < (size / sizeof(u32)); i++)
    {
        printk("%08X ", buf[i]);

        if ((i % 8) == 7)
        {
            printk("\n");
        }
    }
}

/******************************************************************************
 * mtk_snand_read_oob_raw
 *
 * DESCRIPTION:
 *   Read oob data
 *
 * PARAMETERS:
 *   struct mtd_info *mtd, const uint8_t *buf, int addr, int len
 *
 * RETURNS:
 *   1: OK
 *   0: Error
 *
 * NOTES:
 *   this function read raw oob data out of flash, so need to re-organise
 *   data format before using.
 *   len should be times of 8, call this after nand_get_device.
 *   Should notice, this function read data without ECC protection.
 *
 *****************************************************************************/
static int mtk_snand_read_oob_raw(struct mtd_info *mtd, uint8_t * buf, int page_addr, int len)
{
    struct nand_chip *chip = (struct nand_chip *)mtd->priv;
    int bRet = 1;
    int num_sec, num_sec_original;
    u32 i;

    if (len > NAND_MAX_OOBSIZE || len % OOB_AVAI_PER_SECTOR || !buf)
    {
        printk(KERN_WARNING "[%s] invalid parameter, len: %d, buf: %p\n", __FUNCTION__, len, buf);
        return -EINVAL;
    }

    num_sec_original = num_sec = len / OOB_AVAI_PER_SECTOR;

    mtk_snand_rs_reconfig_nfiecc(page_addr);

    //#ifdef _SNAND_DEBUG
    //printk("[K-SNAND][%s] p:%d, b:%d, s:%d, part:%d, spare:%d, ecc:%d\n", __FUNCTION__, page_addr, page_addr / 64, num_sec, g_snand_rs_cur_part, g_snand_k_spare_per_sector, g_snand_rs_ecc_bit);
    //#endif

    if (((num_sec_original * NAND_SECTOR_SIZE) == mtd->writesize) && mtk_snand_rs_if_require_split())
    {
        num_sec--;
    }

    // read the 1st sector (including its spare area) with MTK ECC enabled

    if (0 != g_snand_rs_ecc_bit)
    {
        if (mtk_snand_ready_for_read_custom(chip, page_addr, 0, num_sec, g_snand_k_temp, 1, 1, 1))
        {
            if (!mtk_snand_read_page_data(mtd, g_snand_k_temp, num_sec, 1))  // only read 1 sector
            {
                bRet = 0;
            }

            if (!mtk_snand_check_dececc_done(num_sec))
            {
                bRet = 0;
            }

            mtk_snand_read_fdm_data(g_snand_k_spare, num_sec);

            /*
            if (g_snand_k_spare[0] != 0xFF)
            {
                printk("[K-SNAND][%s] g_snand_k_spare[0]=%x\n", __FUNCTION__, g_snand_k_spare[0]);
                mtk_snand_dump_reg();
                printk("[K-SNAND]Data:\n");
                mtk_snand_dump_mem((u32 *)g_snand_k_temp, num_sec * NAND_SECTOR_SIZE);
                printk("[K-SNAND]OOB:\n");
                mtk_snand_dump_mem((u32 *)g_snand_k_spare, num_sec * OOB_AVAI_PER_SECTOR);
            }
            */

            mtk_snand_stop_read_custom(1);
        }
        else
        {
            mtk_snand_stop_read_custom(0);

            bRet = 0;
        }
    }
    else
    {
        // use internal buffer for read with device ECC on, then transform the format of data and spare

        if (mtk_snand_ready_for_read_custom(chip, page_addr, 0, devinfo.pagesize / NAND_SECTOR_SIZE, g_snand_k_temp, 0, 0, 1))   // MTK ECC off, AUTO FMT off, AHB on
        {
            if (!mtk_snand_read_page_data(mtd, g_snand_k_temp, devinfo.pagesize / NAND_SECTOR_SIZE, 1))
            {
                bRet = 0;
            }

            // copy spare
            if (!mtk_snand_read_fdm_data_devecc(g_snand_k_spare, g_snand_k_temp, 0, num_sec))
            {
                bRet = 0;
            }

            mtk_snand_stop_read_custom(0);
        }
        else
        {
            mtk_snand_stop_read_custom(0);

            bRet = 0;
        }
    }

    if (((num_sec_original * NAND_SECTOR_SIZE) == mtd->writesize) && mtk_snand_rs_if_require_split())
    {
        // read part II

        num_sec++;

        // note: use local temp buffer to read part 2
        mtk_snand_read_page_part2(mtd, page_addr, num_sec, g_snand_k_temp + ((num_sec - 1) * NAND_SECTOR_SIZE));

        // copy spare data
        for (i = 0; i < OOB_AVAI_PER_SECTOR; i++)
        {
            g_snand_k_spare[(num_sec - 1) * OOB_AVAI_PER_SECTOR + i] = g_snand_k_temp[((num_sec - 1) * NAND_SECTOR_SIZE) + NAND_SECTOR_SIZE + i];
        }
    }

    mtk_snand_dev_enable_spiq(0);

    num_sec = num_sec * OOB_AVAI_PER_SECTOR;

    for (i = 0; i < num_sec; i++)
    {
        buf[i] = g_snand_k_spare[i];
    }

    #ifdef _SNAND_DEBUG
    if (0 == bRet)
    {
        printk("[K-SNAND][%s] ERROR!\n", __FUNCTION__);
        //mtk_snand_dump_reg();
        //dump_stack();
        //while (1);
    }
    #endif

    return bRet;
}

static int mtk_snand_write_oob_raw(struct mtd_info *mtd, const uint8_t * buf, int page_addr, int len)
{
    int status = 0;
    u32 i;

    if (len >  NAND_MAX_OOBSIZE || len % OOB_AVAI_PER_SECTOR || !buf)
    {
        printk(KERN_WARNING "[%s] invalid parameter, len: %d, buf: %p\n", __FUNCTION__, len, buf);
        return -EINVAL;
    }

    memset((void *)g_snand_k_temp, 0xFF, mtd->writesize);
    memset((void *)g_snand_k_spare, 0xFF, mtd->oobsize);

    for (i = 0; i < (u32)len; i++)
    {
        ((u8 *)g_snand_k_spare)[i] = buf[i];
    }

    printk("[K-SNAND] g_snand_k_spare:\n");
    mtk_snand_dump_mem((u32 *)g_snand_k_spare, mtd->oobsize);

    status = mtk_nand_exec_write_page(mtd, page_addr, mtd->writesize, (u8 *)g_snand_k_temp, (u8 *)g_snand_k_spare);

    printk("[K-SNAND][%s] page_addr=%d (blk=%d), status=%d\n", __FUNCTION__, page_addr, page_addr / 64, status);

    return status;
}

static int mtk_snand_write_oob_hw(struct mtd_info *mtd, struct nand_chip *chip, int page)
{
    // u8 *buf = chip->oob_poi;
    int i, iter;

    int sec_num = 1<<(chip->page_shift-9);
    int spare_per_sector = mtd->oobsize/sec_num;

    memcpy(local_oob_buf, chip->oob_poi, mtd->oobsize);

    // copy ecc data
    for (i = 0; i < chip->ecc.layout->eccbytes; i++)
    {
        iter = (i / OOB_AVAI_PER_SECTOR) * spare_per_sector + OOB_AVAI_PER_SECTOR + i % OOB_AVAI_PER_SECTOR;
        local_oob_buf[iter] = chip->oob_poi[chip->ecc.layout->eccpos[i]];
        // chip->oob_poi[chip->ecc.layout->eccpos[i]] = local_oob_buf[iter];
    }

    // copy FDM data
    for (i = 0; i < sec_num; i++)
    {
        memcpy(&local_oob_buf[i * spare_per_sector], &chip->oob_poi[i * OOB_AVAI_PER_SECTOR], OOB_AVAI_PER_SECTOR);
    }

    return mtk_snand_write_oob_raw(mtd, local_oob_buf, page, mtd->oobsize);
}

static int mtk_snand_write_oob(struct mtd_info *mtd, struct nand_chip *chip, int page)
{
    int page_per_block = 1 << (chip->phys_erase_shift - chip->page_shift);
    int block = page / page_per_block;
    u16 page_in_block = page % page_per_block;
    int mapped_block = get_mapping_block_index(block);

    // write bad index into oob
    if (mapped_block != block)
    {
        set_bad_index_to_oob(chip->oob_poi, block);
    } else
    {
        set_bad_index_to_oob(chip->oob_poi, FAKE_INDEX);
    }

    if (mtk_snand_write_oob_hw(mtd, chip, page_in_block + mapped_block * page_per_block /* page */ ))
    {
        MSG(INIT, "write oob fail at block: 0x%x, page: 0x%x\n", mapped_block, page_in_block);
        if (update_bmt((page_in_block + mapped_block * page_per_block) << chip->page_shift, UPDATE_WRITE_FAIL, NULL, chip->oob_poi))
        {
            MSG(INIT, "Update BMT success\n");
            return 0;
        } else
        {
            MSG(INIT, "Update BMT fail\n");
            return -EIO;
        }
    }

    return 0;
}

int mtk_nand_block_markbad_hw(struct mtd_info *mtd, loff_t offset)
{
    struct nand_chip *chip = mtd->priv;
    int block = (int)offset >> chip->phys_erase_shift;
    int page = block * (1 << (chip->phys_erase_shift - chip->page_shift));
    int ret;
    u32 buf[2]; // 8 bytes

    // FIXME
    mtk_nand_erase_hw(mtd, page);  // erase before marking bad

    memset((u8 *)buf, 0xFF, 8);

    ((u8 *)buf)[0] = 0;

    ret = mtk_snand_write_oob_raw(mtd, (const uint8_t *)buf, page, 8);

    return ret;
}

static int mtk_snand_block_markbad(struct mtd_info *mtd, loff_t offset)
{
    struct nand_chip *chip = mtd->priv;
    int block = (int)offset >> chip->phys_erase_shift;
    int mapped_block;
    int ret;

    nand_get_device(chip, mtd, FL_WRITING);

    mapped_block = get_mapping_block_index(block);
    ret = mtk_nand_block_markbad_hw(mtd, mapped_block << chip->phys_erase_shift);

    nand_release_device(mtd);

    return ret;
}

int mtk_snand_read_oob_hw(struct mtd_info *mtd, struct nand_chip *chip, int page)
{
    int i;
    u8 iter = 0;

    int sec_num = 1<<(chip->page_shift-9);
    int spare_per_sector = mtd->oobsize/sec_num;
#ifdef TESTTIME
    unsigned long long time1, time2;

    time1 = sched_clock();
#endif

    if (0 == mtk_snand_read_oob_raw(mtd, chip->oob_poi, page, mtd->oobsize))
    {
        // printk(KERN_ERR "[%s]mtk_snand_read_oob_raw return failed\n", __FUNCTION__);
        return -EIO;
    }
#ifdef TESTTIME
    time2 = sched_clock() - time1;
    if (!readoobflag)
    {
        readoobflag = 1;
        printk(KERN_ERR "[%s] time is %llu", __FUNCTION__, time2);
    }
#endif

    // adjust to ecc physical layout to memory layout
    /*********************************************************/
    /* FDM0 | ECC0 | FDM1 | ECC1 | FDM2 | ECC2 | FDM3 | ECC3 */
    /*  8B  |  8B  |  8B  |  8B  |  8B  |  8B  |  8B  |  8B  */
    /*********************************************************/

    memcpy(local_oob_buf, chip->oob_poi, mtd->oobsize);

    // copy ecc data
    for (i = 0; i < chip->ecc.layout->eccbytes; i++)
    {
        iter = (i / OOB_AVAI_PER_SECTOR) *  spare_per_sector + OOB_AVAI_PER_SECTOR + i % OOB_AVAI_PER_SECTOR;
        chip->oob_poi[chip->ecc.layout->eccpos[i]] = local_oob_buf[iter];
    }

    // copy FDM data
    for (i = 0; i < sec_num; i++)
    {
        memcpy(&chip->oob_poi[i * OOB_AVAI_PER_SECTOR], &local_oob_buf[i *  spare_per_sector], OOB_AVAI_PER_SECTOR);
    }

    return 0;
}

static int mtk_snand_read_oob(struct mtd_info *mtd, struct nand_chip *chip, int page, int sndcmd)
{
    int page_per_block = 1 << (chip->phys_erase_shift - chip->page_shift);
    int block = page / page_per_block;
    u16 page_in_block = page % page_per_block;
    int mapped_block = get_mapping_block_index(block);

    mtk_snand_read_oob_hw(mtd, chip, page_in_block + mapped_block * page_per_block);

    return 0;                   // the return value is sndcmd
}

int mtk_nand_block_bad_hw(struct mtd_info *mtd, loff_t ofs)
{
    struct nand_chip *chip = (struct nand_chip *)mtd->priv;
    int page_addr = (int)(ofs >> chip->page_shift);
    unsigned int page_per_block = 1 << (chip->phys_erase_shift - chip->page_shift);
    unsigned char oob_buf[8];
    page_addr &= ~(page_per_block - 1);

    if (1 == mtk_snand_is_vendor_reserved_blocks(page_addr)) // return bad block if it is reserved block
    {
        return 1;
    }

    if (0 == mtk_snand_read_oob_raw(mtd, oob_buf, page_addr, sizeof(oob_buf)))   // only read 8 bytes
    {
        printk(KERN_WARNING "mtk_snand_read_oob_raw return error\n");
        return 1;
    }

    if (oob_buf[0] != 0xff)
    {
        printk(KERN_WARNING "Bad block detected at 0x%x (blk:%d), oob_buf[0] is 0x%x\n", page_addr, page_addr / page_per_block, oob_buf[0]);

        #ifdef _SNAND_DEBUG
        printk("[K-SNAND] bad block found! blk=%d, oob_buf[0]=%x\n", page_addr / 64, oob_buf[0]);
        #endif

        return 1;
    }

    return 0;                   // everything is OK, good block
}

static int mtk_snand_block_bad(struct mtd_info *mtd, loff_t ofs, int getchip)
{
    int chipnr = 0;

    struct nand_chip *chip = (struct nand_chip *)mtd->priv;
    int block = (int)ofs >> chip->phys_erase_shift;
    int mapped_block;
    int ret;

    if (getchip)
    {
        chipnr = (int)(ofs >> chip->chip_shift);
        nand_get_device(chip, mtd, FL_READING);
        /* Select the NAND device */
        chip->select_chip(mtd, chipnr);
    }

    mapped_block = get_mapping_block_index(block);

    ret = mtk_nand_block_bad_hw(mtd, mapped_block << chip->phys_erase_shift);

    if (ret)
    {
        MSG(INIT, "Unmapped bad block: 0x%x\n", mapped_block);

        if (update_bmt(mapped_block << chip->phys_erase_shift, UPDATE_UNMAPPED_BLOCK, NULL, NULL))
        {
            MSG(INIT, "Update BMT success\n");
            ret = 0;
        } else
        {
            MSG(INIT, "Update BMT fail\n");
            ret = 1;
        }
    }

    if (getchip)
    {
        nand_release_device(mtd);
    }

    return ret;
}
/******************************************************************************
 * mtk_nand_init_size
 *
 * DESCRIPTION:
 *   initialize the pagesize, oobsize, blocksize
 *
 * PARAMETERS:
 *   struct mtd_info *mtd, struct nand_chip *this, u8 *id_data
 *
 * RETURNS:
 *   Buswidth
 *
 * NOTES:
 *   None
 *
 ******************************************************************************/

static int mtk_nand_init_size(struct mtd_info *mtd, struct nand_chip *this, u8 *id_data)
{
    /* Get page size */
    mtd->writesize = devinfo.pagesize ;

    /* Get oobsize */
    mtd->oobsize = devinfo.sparesize;

    /* Get blocksize. */
    mtd->erasesize = devinfo.blocksize*1024;

    return 0;
}

/******************************************************************************
 * mtk_snand_verify_buf
 *
 * DESCRIPTION:
 *   Verify the NAND write data is correct or not !
 *
 * PARAMETERS:
 *   struct mtd_info *mtd, const uint8_t *buf, int len
 *
 * RETURNS:
 *   None
 *
 * NOTES:
 *   None
 *
 ******************************************************************************/
#ifdef CONFIG_MTD_NAND_VERIFY_WRITE

char gacBuf[4096 + 128];

static int mtk_snand_verify_buf(struct mtd_info *mtd, const uint8_t * buf, int len)
{
#if 1
    struct nand_chip *chip = (struct nand_chip *)mtd->priv;
    struct NAND_CMD *pkCMD = &g_kCMD;
    u32 u4PageSize = mtd->writesize;
    u32 *pSrc, *pDst;
    int i;

    mtk_nand_exec_read_page(mtd, pkCMD->u4RowAddr, u4PageSize, gacBuf, gacBuf + u4PageSize);

    pSrc = (u32 *) buf;
    pDst = (u32 *) gacBuf;
    len = len / sizeof(u32);
    for (i = 0; i < len; ++i)
    {
        if (*pSrc != *pDst)
        {
            MSG(VERIFY, "mtk_snand_verify_buf page fail at page %d\n", pkCMD->u4RowAddr);
            return -1;
        }
        pSrc++;
        pDst++;
    }

    pSrc = (u32 *) chip->oob_poi;
    pDst = (u32 *) (gacBuf + u4PageSize);

    if ((pSrc[0] != pDst[0]) || (pSrc[1] != pDst[1]) || (pSrc[2] != pDst[2]) || (pSrc[3] != pDst[3]) || (pSrc[4] != pDst[4]) || (pSrc[5] != pDst[5]))
        // TODO: Ask Designer Why?
        //(pSrc[6] != pDst[6]) || (pSrc[7] != pDst[7]))
    {
        MSG(VERIFY, "mtk_snand_verify_buf oob fail at page %d\n", pkCMD->u4RowAddr);
        MSG(VERIFY, "0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n", pSrc[0], pSrc[1], pSrc[2], pSrc[3], pSrc[4], pSrc[5], pSrc[6], pSrc[7]);
        MSG(VERIFY, "0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n", pDst[0], pDst[1], pDst[2], pDst[3], pDst[4], pDst[5], pDst[6], pDst[7]);
        return -1;
    }
    /*
       for (i = 0; i < len; ++i) {
       if (*pSrc != *pDst) {
       printk(KERN_ERR"mtk_snand_verify_buf oob fail at page %d\n", g_kCMD.u4RowAddr);
       return -1;
       }
       pSrc++;
       pDst++;
       }
     */
    //printk(KERN_INFO"mtk_snand_verify_buf OK at page %d\n", g_kCMD.u4RowAddr);

    return 0;
#else
    return 0;
#endif
}
#endif

/******************************************************************************
 * mtk_nand_init_hw
 *
 * DESCRIPTION:
 *   Initial NAND device hardware component !
 *
 * PARAMETERS:
 *   struct mtk_nand_host *host (Initial setting data)
 *
 * RETURNS:
 *   None
 *
 * NOTES:
 *   None
 *
 ******************************************************************************/
static void mtk_nand_init_hw(struct mtk_nand_host *host)
{
    struct mtk_nand_host_hw *hw = host->hw;

    g_bInitDone = 0;
    g_kCMD.u4OOBRowAddr = (u32) - 1;

    /* Set default NFI access timing control */
    SNAND_WriteReg(NFI_ACCCON_REG32, hw->nfi_access_timing);
    SNAND_WriteReg(NFI_CNFG_REG16, 0);
    SNAND_WriteReg(NFI_PAGEFMT_REG16, 0);

    /* Reset the state machine and data FIFO, because flushing FIFO */
    (void)mtk_snand_reset_con();

    /* Set the ECC engine */
    if (hw->nand_ecc_mode == NAND_ECC_HW)
    {
        MSG(INIT, "%s : Use HW ECC\n", MODULE_NAME);
        if (g_bHwEcc)
        {
            SNAND_REG_SET_BITS(NFI_CNFG_REG16, CNFG_HW_ECC_EN);
        }

        mtk_snand_ecc_config(host->hw,4);
        mtk_snand_configure_fdm(8);
    }

    /* Initilize interrupt. Clear interrupt, read clear. */
    *(NFI_INTR_REG16);

    /* Interrupt arise when read data or program data to/from AHB is done. */
    SNAND_WriteReg(NFI_INTR_EN_REG16, 0);

    // Enable automatic disable ECC clock when NFI is busy state

    // REMARKS! HWDCM_SWCON_ON should not be enabled for SPI-NAND! Otherwise NFI will behave abnormally!
    SNAND_WriteReg(NFI_DEBUG_CON1_REG16, (WBUF_EN));

    #ifdef CONFIG_PM
    host->saved_para.suspend_flag = 0;
    #endif
    // Reset
}

//-------------------------------------------------------------------------------
static int mtk_snand_dev_ready(struct mtd_info *mtd)
{
    u8  reg8;
    u32 reg32;

    if (SNAND_DEV_PROG_EXECUTING == g_snand_dev_status)
    {
        SNAND_WriteReg(RW_SNAND_GPRAM_DATA, (SNAND_CMD_GET_FEATURES | (SNAND_CMD_FEATURES_STATUS << 8)));
        SNAND_WriteReg(RW_SNAND_MAC_OUTL, 2);
        SNAND_WriteReg(RW_SNAND_MAC_INL , 1);

        mtk_snand_dev_mac_op(SPI);

        reg8 = DRV_Reg8(((P_U8)RW_SNAND_GPRAM_DATA + 2));

        if (0 == (reg8 & SNAND_STATUS_OIP)) // ready
        {
        	g_snand_dev_status = SNAND_DEV_PROG_DONE;

            if (0 != (reg8 & SNAND_STATUS_PROGRAM_FAIL)) // ready but having fail report from device
            {
				g_snand_cmd_status = NAND_STATUS_READY | NAND_STATUS_FAIL;
            }
            else
            {
				g_snand_cmd_status = NAND_STATUS_READY;
            }
        }
        else    // busy
        {
            g_snand_cmd_status = 0;
        }
    }
    else if (SNAND_NFI_AUTO_ERASING == g_snand_dev_status)
    {
        // wait for auto erase finish

        reg32 = *(RW_SNAND_STA_CTL1);

        if ((reg32 & SNAND_AUTO_BLKER) == 0)
        {
            g_snand_cmd_status = 0; // busy
        }
        else
        {
        	g_snand_dev_status = SNAND_DEV_ERASE_DONE;

            reg8 = (u8)(*(RW_SNAND_GF_CTL1) & SNAND_GF_STATUS_MASK);

            if (0 != (reg8 & SNAND_STATUS_ERASE_FAIL)) // ready but having fail report from device
            {
				g_snand_cmd_status = NAND_STATUS_READY | NAND_STATUS_FAIL;
            }
            else
            {
				g_snand_cmd_status = NAND_STATUS_READY;
            }
        }
    }
    else if (SNAND_NFI_CUST_READING == g_snand_dev_status)
    {
        g_snand_cmd_status = 0; // busy
    }
    else
    {
		g_snand_cmd_status = NAND_STATUS_READY; // idle
    }

    return g_snand_cmd_status;
}

/******************************************************************************
 * mtk_snand_proc_read
 *
 * DESCRIPTION:
 *   Read the proc file to get the interrupt scheme setting !
 *
 * PARAMETERS:
 *   char *page, char **start, off_t off, int count, int *eof, void *data
 *
 * RETURNS:
 *   None
 *
 * NOTES:
 *   None
 *
 ******************************************************************************/
static int mtk_snand_proc_read(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	char 	*p = page;
	int 	len = 0;
    int i;
    p += sprintf(p, "ID:");
    for(i=0;i<devinfo.id_length;i++){
        p += sprintf(p, " 0x%x", devinfo.id[i]);
    }
    p += sprintf(p, "\n");
    p += sprintf(p, "total size: %dMiB; part number: %s\n", devinfo.totalsize,devinfo.devicename);
    p += sprintf(p, "Current working in %s mode\n", g_i4Interrupt ? "interrupt" : "polling");
    p += sprintf(p, "SNF_DLY_CTL1: 0x%x\n", (*RW_SNAND_DLY_CTL1));
    p += sprintf(p, "SNF_DLY_CTL2: 0x%x\n", (*RW_SNAND_DLY_CTL2));
    p += sprintf(p, "SNF_DLY_CTL3: 0x%x\n", (*RW_SNAND_DLY_CTL3));
    p += sprintf(p, "SNF_DLY_CTL4: 0x%x\n", (*RW_SNAND_DLY_CTL4));
    p += sprintf(p, "SNF_MISC_CTL: 0x%x\n", (*RW_SNAND_MISC_CTL));
    //p += sprintf(p, "Driving: 0x%x\n", (*RW_GPIO_DRV0 & (0x0007)));
    if(g_NandPerfLog.ReadPageCount!=0)
        p += sprintf(p, "Read Page Count:%d, Read Page totalTime:%lu, Avg. RPage:%lu\r\n",
                     g_NandPerfLog.ReadPageCount,g_NandPerfLog.ReadPageTotalTime,
                     g_NandPerfLog.ReadPageTotalTime/g_NandPerfLog.ReadPageCount);

    if(g_NandPerfLog.ReadSectorCount!=0)
        p += sprintf(p, "Read Sub-Page Count:%d, Read Sub-Page totalTime:%lu, Avg. RPage:%lu\r\n",
                     g_NandPerfLog.ReadSectorCount,g_NandPerfLog.ReadSectorTotalTime,
                     g_NandPerfLog.ReadSectorTotalTime/g_NandPerfLog.ReadSectorCount);

    if(g_NandPerfLog.ReadBusyCount!=0)
        p += sprintf(p, "Read Busy Count:%d, Read Busy totalTime:%lu, Avg. R Busy:%lu\r\n",
                     g_NandPerfLog.ReadBusyCount,g_NandPerfLog.ReadBusyTotalTime,
                     g_NandPerfLog.ReadBusyTotalTime/g_NandPerfLog.ReadBusyCount);
    if(g_NandPerfLog.ReadDMACount!=0)
        p += sprintf(p, "Read DMA Count:%d, Read DMA totalTime:%lu, Avg. R DMA:%lu\r\n",
                     g_NandPerfLog.ReadDMACount,g_NandPerfLog.ReadDMATotalTime,
                     g_NandPerfLog.ReadDMATotalTime/g_NandPerfLog.ReadDMACount);

    if(g_NandPerfLog.WritePageCount!=0)
        p += sprintf(p, "Write Page Count:%d, Write Page totalTime:%lu, Avg. WPage:%lu\r\n",
                     g_NandPerfLog.WritePageCount,g_NandPerfLog.WritePageTotalTime,
                     g_NandPerfLog.WritePageTotalTime/g_NandPerfLog.WritePageCount);
    if(g_NandPerfLog.WriteBusyCount!=0)
        p += sprintf(p, "Write Busy Count:%d, Write Busy totalTime:%lu, Avg. W Busy:%lu\r\n",
                     g_NandPerfLog.WriteBusyCount,g_NandPerfLog.WriteBusyTotalTime,
                     g_NandPerfLog.WriteBusyTotalTime/g_NandPerfLog.WriteBusyCount);
    if(g_NandPerfLog.WriteDMACount!=0)
        p += sprintf(p, "Write DMA Count:%d, Write DMA totalTime:%lu, Avg. W DMA:%lu\r\n",
                     g_NandPerfLog.WriteDMACount,g_NandPerfLog.WriteDMATotalTime,
                     g_NandPerfLog.WriteDMATotalTime/g_NandPerfLog.WriteDMACount);
    if(g_NandPerfLog.EraseBlockCount!=0)
        p += sprintf(p, "EraseBlock Count:%d, EraseBlock totalTime:%lu, Avg. Erase:%lu\r\n",
                     g_NandPerfLog.EraseBlockCount,g_NandPerfLog.EraseBlockTotalTime,
                     g_NandPerfLog.EraseBlockTotalTime/g_NandPerfLog.EraseBlockCount);


    *start = page + off;
    len = p - page;

    if (len > off)
        len -= off;
    else
        len = 0;

    return len < count ? len : count;
}

/******************************************************************************
 * mtk_snand_proc_write
 *
 * DESCRIPTION:
 *   Write the proc file to set the interrupt scheme !
 *
 * PARAMETERS:
 *   struct file* file, const char* buffer,	unsigned long count, void *data
 *
 * RETURNS:
 *   None
 *
 * NOTES:
 *   None
 *
 ******************************************************************************/
static int mtk_snand_proc_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
    struct mtd_info *mtd = &host->mtd;
    char buf[16];
    char cmd;
    int len = count;
    u32 value;
    #ifdef CFG_SNAND_SLEEP_WHILE_BUSY_PROC_TEST
    u32 s1, s2;
    struct timeval stimer, etimer;
    #endif

    if (len >= sizeof(buf))
    {
        len = sizeof(buf) - 1;
    }

    if (copy_from_user(buf, buffer, len))
    {
        return -EFAULT;
    }

    sscanf(buf, "%c %x",&cmd, &value);

    switch(cmd)
    {
        case 'S':
        {
            #ifdef CFG_SNAND_SLEEP_WHILE_BUSY_PROC_TEST

            u32 value2, value3, value4, min_ori, max_ori;

            printk("[SNAND][proc] ====== SPI-NAND usleep test (begin) ======\n");

            min_ori = g_snand_sleep_us_page_read_min;
            max_ori = g_snand_sleep_us_page_read_max;

            cmd = 0;

            for (s1 = 0; s1 <= 100; s1 += 10)
            {
                for (s2 = s1; s2 <= 100; s2 += 10)
                {
                    g_snand_sleep_us_page_read_min = s1;
                    g_snand_sleep_us_page_read_max = s2;

                    printk("[SNAND][proc]   -- min: %d, max: %d\n", g_snand_sleep_us_page_read_min, g_snand_sleep_us_page_read_max);

                    value = value2 = value3 = value4 = 0;

                    for (len = 0; len <= 10; len++)
                    {
                        nand_get_device((struct nand_chip *)mtd->priv, mtd, FL_READING);

                        do_gettimeofday(&stimer);

                        if (0 == mtk_nand_exec_read_page(mtd, 512 + s1 + s2 + len, mtd->writesize, g_snand_k_temp, g_snand_k_temp + mtd->writesize))
                        {
                            printk("[SNAND][proc] read error! test halt!\n");

                            cmd = 1;

                            break;
                        }

                        do_gettimeofday(&etimer);

                        nand_release_device(mtd);

                        if (Cal_timediff(&etimer,&stimer) < 250)
                        {
                            value += Cal_timediff(&etimer,&stimer);
                            value3 += g_snand_sleep_temp_val1;
                            value4 += g_snand_sleep_temp_val2;

                            value2++;
                        }

                        printk("[SNAND][proc]     + read time: %d us, dev: %d us (sleep: %d us)\n", Cal_timediff(&etimer,&stimer), g_snand_sleep_temp_val2, g_snand_sleep_temp_val1);
                    }

                    printk("[SNAND][proc]     + AVG: read time: %d us, dev: %d us (sleep: %d us)\n", value / value2, value4 / value2, value3 / value2);

                    if (1 == cmd) break;
                }

                if (1 == cmd) break;
            }

            printk("[SNAND][proc] ====== SPI-NAND usleep test (end) ======\n");

            g_snand_sleep_us_page_read_min = min_ori;
            g_snand_sleep_us_page_read_max = max_ori;

            #endif

            break;
        }
        case '1':
            g_NandPerfLog.ReadPageCount = 0;
            g_NandPerfLog.ReadPageTotalTime = 0;
            g_NandPerfLog.ReadSectorCount = 0;
            g_NandPerfLog.ReadSectorTotalTime = 0;
            g_NandPerfLog.ReadBusyCount = 0;
            g_NandPerfLog.ReadBusyTotalTime = 0;
            g_NandPerfLog.ReadDMACount = 0;
            g_NandPerfLog.ReadDMATotalTime = 0;

            g_NandPerfLog.WritePageCount = 0;
            g_NandPerfLog.WritePageTotalTime = 0;
            g_NandPerfLog.WriteBusyCount = 0;
            g_NandPerfLog.WriteBusyTotalTime = 0;
            g_NandPerfLog.WriteDMACount = 0;
            g_NandPerfLog.WriteDMATotalTime = 0;

            g_NandPerfLog.EraseBlockCount = 0;
            g_NandPerfLog.EraseBlockTotalTime = 0;

            #ifdef CFG_SNAND_ACCESS_PATTERN_LOGGER
            g_snand_dbg_ap_on = 1;
            g_snand_pm_cnt = 0;
            g_snand_pm_wrapped = 0;
            #endif

            break;

        case '2':

            #ifdef CFG_SNAND_ACCESS_PATTERN_LOGGER
            nand_get_device((struct nand_chip *)mtd->priv, mtd, FL_READING);

            g_snand_dbg_ap_on = 0;

            mtk_snand_pm_dump_record();

            nand_release_device(mtd);
            #endif

            if(g_NandPerfLog.ReadPageCount!=0)
            printk("Read Page Count:%d, Read Page totalTime:%lu, Avg. RPage:%lu\r\n",
                     g_NandPerfLog.ReadPageCount,g_NandPerfLog.ReadPageTotalTime,
                     g_NandPerfLog.ReadPageTotalTime/g_NandPerfLog.ReadPageCount);

            if(g_NandPerfLog.ReadBusyCount!=0)
            printk("Read Busy Count:%d, Read Busy totalTime:%lu, Avg. R Busy:%lu\r\n",
                         g_NandPerfLog.ReadBusyCount,g_NandPerfLog.ReadBusyTotalTime,
                         g_NandPerfLog.ReadBusyTotalTime/g_NandPerfLog.ReadBusyCount);
            if(g_NandPerfLog.ReadDMACount!=0)
            printk("Read DMA Count:%d, Read DMA totalTime:%lu, Avg. R DMA:%lu\r\n",
                         g_NandPerfLog.ReadDMACount,g_NandPerfLog.ReadDMATotalTime,
                         g_NandPerfLog.ReadDMATotalTime/g_NandPerfLog.ReadDMACount);

            if(g_NandPerfLog.WritePageCount!=0)
            printk("Write Page Count:%d, Write Page totalTime:%lu, Avg. WPage:%lu\r\n",
                         g_NandPerfLog.WritePageCount,g_NandPerfLog.WritePageTotalTime,
                         g_NandPerfLog.WritePageTotalTime/g_NandPerfLog.WritePageCount);
            if(g_NandPerfLog.WriteBusyCount!=0)
            printk("Write Busy Count:%d, Write Busy totalTime:%lu, Avg. W Busy:%lu\r\n",
                         g_NandPerfLog.WriteBusyCount,g_NandPerfLog.WriteBusyTotalTime,
                         g_NandPerfLog.WriteBusyTotalTime/g_NandPerfLog.WriteBusyCount);
            if(g_NandPerfLog.WriteDMACount!=0)
            printk("Write DMA Count:%d, Write DMA totalTime:%lu, Avg. W DMA:%lu\r\n",
                         g_NandPerfLog.WriteDMACount,g_NandPerfLog.WriteDMATotalTime,
                         g_NandPerfLog.WriteDMATotalTime/g_NandPerfLog.WriteDMACount);

            if(g_NandPerfLog.EraseBlockCount!=0)
            printk("EraseBlock Count:%d, EraseBlock totalTime:%lu, Avg. Erase:%lu\r\n",
                         g_NandPerfLog.EraseBlockCount,g_NandPerfLog.EraseBlockTotalTime,
                         g_NandPerfLog.EraseBlockTotalTime/g_NandPerfLog.EraseBlockCount);

            if(g_NandPerfLog.ReadSectorCount!=0)
            printk("Read Sector Count:%d, Read Sector totalTime:%lu, Avg. RSector:%lu\r\n",
                     g_NandPerfLog.ReadSectorCount,g_NandPerfLog.ReadSectorTotalTime,
                     g_NandPerfLog.ReadSectorTotalTime/g_NandPerfLog.ReadSectorCount);

            break;
        case 'V':  // NFIA driving setting
            //*RW_GPIO_DRV0 = *RW_GPIO_DRV0 & ~(0x0007);          // Clear Driving
            //*RW_GPIO_DRV0 = *RW_GPIO_DRV0 | value;
            break;
        case 'B': // NFIB driving setting
            *((volatile u32 *)(IO_CFG_BOTTOM_BASE+0x0060)) = value;
            break;
        case 'D':
   #ifdef _MTK_NAND_DUMMY_DRIVER_
               printk(KERN_INFO "Enable dummy driver\n");
               dummy_driver_debug = 1;
   #endif
            break;
        case 'I':   // Interrupt control
            break;  // stanley chu, SPI-NAND disable traditional interrupt feature
            if ((value > 0 && !g_i4Interrupt) || (value== 0 && g_i4Interrupt))
             {
                 nand_get_device((struct nand_chip *)mtd->priv, mtd, FL_READING);

                 g_i4Interrupt = value;

                 if (g_i4Interrupt)
                 {
                     *(NFI_INTR_REG16);
                     enable_irq(MT_NFI_IRQ_ID);
                 } else
                     disable_irq(MT_NFI_IRQ_ID);

                 nand_release_device(mtd);
             }
            break;
         case 'P': // Reset Performance monitor counter
             #ifdef NAND_PFM
                 /* Reset values */
                 g_PFM_R = 0;
                 g_PFM_W = 0;
                 g_PFM_E = 0;
                 g_PFM_RD = 0;
                 g_PFM_WD = 0;
                 g_kCMD.pureReadOOBNum = 0;
            #endif
            break;
        case 'R': // Reset NFI performance log
               g_NandPerfLog.ReadPageCount = 0;
               g_NandPerfLog.ReadPageTotalTime = 0;
			   g_NandPerfLog.ReadBusyCount = 0;
			   g_NandPerfLog.ReadBusyTotalTime = 0;
               g_NandPerfLog.ReadDMACount = 0;
               g_NandPerfLog.ReadDMATotalTime = 0;

               g_NandPerfLog.WritePageCount = 0;
               g_NandPerfLog.WritePageTotalTime = 0;
               g_NandPerfLog.WriteBusyCount = 0;
               g_NandPerfLog.WriteBusyTotalTime = 0;
               g_NandPerfLog.WriteDMACount = 0;
               g_NandPerfLog.WriteDMATotalTime = 0;

               g_NandPerfLog.EraseBlockCount = 0;
               g_NandPerfLog.EraseBlockTotalTime = 0;

           break;
         case 'T':  // ACCCON Setting
             SNAND_WriteReg(NFI_ACCCON_REG32,value);
            break;
         default:
            break;
    }

    return len;
}

/******************************************************************************
 * mtk_snand_probe
 *
 * DESCRIPTION:
 *   register the nand device file operations !
 *
 * PARAMETERS:
 *   struct platform_device *pdev : device structure
 *
 * RETURNS:
 *   0 : Success
 *
 * NOTES:
 *   None
 *
 ******************************************************************************/
static int mtk_snand_probe(struct platform_device *pdev)
{
    struct mtk_nand_host_hw *hw;
    struct mtd_info *mtd;
    struct nand_chip *nand_chip;
    struct resource *res = pdev->resource;
    int err = 0;
    u8 id[SNAND_MAX_ID];
    int i;

    hw = (struct mtk_nand_host_hw *)pdev->dev.platform_data;
    BUG_ON(!hw);

    if (pdev->num_resources != 4 || res[0].flags != IORESOURCE_MEM || res[1].flags != IORESOURCE_MEM || res[2].flags != IORESOURCE_IRQ || res[3].flags != IORESOURCE_IRQ)
    {
        MSG(INIT, "%s: invalid resource type\n", __FUNCTION__);
        return -ENODEV;
    }

    /* Request IO memory */
    if (!request_mem_region(res[0].start, res[0].end - res[0].start + 1, pdev->name))
    {
        return -EBUSY;
    }
    if (!request_mem_region(res[1].start, res[1].end - res[1].start + 1, pdev->name))
    {
        return -EBUSY;
    }

    /* Allocate memory for the device structure (and zero it) */
    host = kzalloc(sizeof(struct mtk_nand_host), GFP_KERNEL);
    if (!host)
    {
        MSG(INIT, "mtk_nand: failed to allocate device structure.\n");
        return -ENOMEM;
    }

    /* Allocate memory for 16 byte aligned buffer */
    local_buffer_cache_size_align = local_buffer + _SNAND_CACHE_LINE_SIZE - ((u32) local_buffer % _SNAND_CACHE_LINE_SIZE);
    printk(KERN_INFO "Allocate cache-line-size aligned buffer: %p\n", local_buffer_cache_size_align);

    host->hw = hw;

    /* init mtd data structure */
    nand_chip = &host->nand_chip;
    nand_chip->priv = host;     /* link the private data structures */

    mtd = &host->mtd;
    mtd->priv = nand_chip;
    mtd->owner = THIS_MODULE;
    mtd->name = "MTK-SNand";

    hw->nand_ecc_mode = NAND_ECC_HW;

    /* Set address of NAND IO lines */
    //nand_chip->IO_ADDR_R = (void __iomem *)NFI_DATAR_REG32;
    //nand_chip->IO_ADDR_W = (void __iomem *)NFI_DATAW_REG32;
    nand_chip->IO_ADDR_R = NULL;
    nand_chip->IO_ADDR_W = NULL;
    nand_chip->chip_delay = 20; /* 20us command delay time */
    nand_chip->ecc.mode = hw->nand_ecc_mode;    /* enable ECC */

    nand_chip->read_byte = mtk_snand_read_byte;
    nand_chip->read_buf = mtk_snand_read_buf;
    nand_chip->write_buf = mtk_snand_write_buf;
#ifdef CONFIG_MTD_NAND_VERIFY_WRITE
    nand_chip->verify_buf = mtk_snand_verify_buf;
#endif
    nand_chip->select_chip = mtk_snand_select_chip;
    nand_chip->dev_ready = mtk_snand_dev_ready;
    nand_chip->cmdfunc = mtk_snand_command_bp;
    nand_chip->ecc.read_page = mtk_snand_read_page_hwecc;
    nand_chip->ecc.write_page = mtk_snand_write_page_hwecc;

    nand_chip->ecc.layout = &nand_oob_64;
    nand_chip->ecc.size = hw->nand_ecc_size;    //2048
    nand_chip->ecc.bytes = hw->nand_ecc_bytes;  //32

    nand_chip->options = NAND_SKIP_BBTSCAN;

    // For BMT, we need to revise driver architecture
    nand_chip->write_page = mtk_snand_write_page;
    nand_chip->read_page = mtk_snand_read_page;
#if 1
    nand_chip->read_subpage = mtk_snand_read_subpage;   // assigned this API to enable read subpage feature
    nand_chip->subpage_size = NAND_SECTOR_SIZE;
#else
    nand_chip->read_subpage = NULL;
#endif
    nand_chip->ecc.write_oob = mtk_snand_write_oob;
    nand_chip->ecc.read_oob = mtk_snand_read_oob;
    nand_chip->block_markbad = mtk_snand_block_markbad;   // need to add nand_get_device()/nand_release_device().
    nand_chip->erase = mtk_nand_erase;
    nand_chip->block_bad = mtk_snand_block_bad;
    nand_chip->init_size = mtk_nand_init_size;

    MSG(INIT, "Enable NFI, NFIECC and SPINFI Clock\n");

    if (clock_is_on(MT_CG_NFI_SW_CG) == PWR_DOWN)
    {
        enable_clock(MT_CG_NFI_SW_CG, "NAND");
    }
    if (clock_is_on(MT_CG_NFI_HCLK_SW_CG) == PWR_DOWN)
    {
        enable_clock(MT_CG_NFI_HCLK_SW_CG, "NAND");
    }
    if (clock_is_on(MT_CG_SPINFI_SW_CG) == PWR_DOWN)
    {
        enable_clock(MT_CG_SPINFI_SW_CG, "NAND");
    }

    //clkmux_sel(MT_CLKMUX_SPINFI_MUX_SEL, MT_CG_UPLL_D12, "NAND"); // SNF is raised to 104 MHz in preloader

    mtk_nand_init_hw(host);

    /* Select the device */
    nand_chip->select_chip(mtd, NFI_DEFAULT_CS);

    /*
     * Reset the chip, required by some chips (e.g. Micron MT29FxGxxxxx)
     * after power-up
     */
    nand_chip->cmdfunc(mtd, NAND_CMD_RESET, -1, -1);

    /* Send the command for reading device ID */
    nand_chip->cmdfunc(mtd, NAND_CMD_READID, 0x00, -1);

    for(i=0;i<SNAND_MAX_ID;i++)
    {
        id[i]=nand_chip->read_byte(mtd);
    }

    manu_id = id[0];
    dev_id = id[1];

    if (!mtk_snand_get_device_info(id, &devinfo))
    {
        MSG(INIT, "Not Support this Device! \r\n");
    }

    if (devinfo.pagesize == 4096)
    {
        nand_chip->ecc.layout = &nand_oob_128;
        hw->nand_ecc_size = 4096;
    } else if (devinfo.pagesize == 2048)
    {
        nand_chip->ecc.layout = &nand_oob_64;
        hw->nand_ecc_size = 2048;
    } else if (devinfo.pagesize == 512)
    {
        nand_chip->ecc.layout = &nand_oob_16;
        hw->nand_ecc_size = 512;
    }

    nand_chip->ecc.layout->eccbytes = devinfo.sparesize-OOB_AVAI_PER_SECTOR*(devinfo.pagesize/NAND_SECTOR_SIZE);
    hw->nand_ecc_bytes = nand_chip->ecc.layout->eccbytes;

    // Modify to fit device character
    nand_chip->ecc.size = hw->nand_ecc_size;
    nand_chip->ecc.bytes = hw->nand_ecc_bytes;

    for(i=0;i<nand_chip->ecc.layout->eccbytes;i++)
    {
		nand_chip->ecc.layout->eccpos[i]=OOB_AVAI_PER_SECTOR*(devinfo.pagesize/NAND_SECTOR_SIZE)+i;
    }

    MSG(INIT, "[NAND] pagesz:%d , oobsz: %d,eccbytes: %d\n",
       devinfo.pagesize,  sizeof(g_kCMD.au1OOB),nand_chip->ecc.layout->eccbytes);

    //MSG(INIT, "Support this Device in MTK table! %x \r\n", id);
    hw->nfi_bus_width = 4;
    //SNAND_WriteReg(NFI_ACCCON_REG32, NFI_DEFAULT_ACCESS_TIMING);

    mt_irq_set_sens(MT_NFI_IRQ_ID, MT65xx_LEVEL_SENSITIVE);
    mt_irq_set_polarity(MT_NFI_IRQ_ID, MT65xx_POLARITY_LOW);
    err = request_irq(MT_NFI_IRQ_ID, mtk_nand_irq_handler, IRQF_DISABLED, "mtk-nand", NULL);

	if (0 != err)
    {
        MSG(INIT, "%s : Request IRQ fail: err = %d\n", MODULE_NAME, err);
        goto out;
    }

    if (g_i4Interrupt)
        enable_irq(MT_NFI_IRQ_ID);
    else
        disable_irq(MT_NFI_IRQ_ID);

	mtd->oobsize = devinfo.sparesize;

    /* Scan to find existance of the device */
    if (nand_scan(mtd, hw->nfi_cs_num))
    {
        MSG(INIT, "%s : nand_scan fail.\n", MODULE_NAME);
        err = -ENXIO;
        goto out;
    }

    g_page_size = mtd->writesize;

    platform_set_drvdata(pdev, host);

    nand_chip->select_chip(mtd, 0);
    #if defined(MTK_COMBO_NAND_SUPPORT)
    	nand_chip->chipsize -= (PART_SIZE_BMTPOOL);
    #else
	    nand_chip->chipsize -= (BMT_POOL_SIZE) << nand_chip->phys_erase_shift;
    #endif
    mtd->size = nand_chip->chipsize;

    // config read empty threshold for MTK ECC (MT6571 only)
    *NFI_EMPTY_THRESHOLD = 1;

    if (!g_bmt)
    {
    		#if defined(MTK_COMBO_NAND_SUPPORT)
    		if (!(g_bmt = init_bmt(nand_chip, ((PART_SIZE_BMTPOOL) >> nand_chip->phys_erase_shift))))
    		#else
        if (!(g_bmt = init_bmt(nand_chip, BMT_POOL_SIZE)))
        #endif
        {
            MSG(INIT, "Error: init bmt failed\n");
            return 0;
        }
    }

    nand_chip->chipsize -= (PMT_POOL_SIZE) << nand_chip->phys_erase_shift;
    mtd->size = nand_chip->chipsize;


#ifdef PMT
    part_init_pmt(mtd, (u8 *) & g_exist_Partition[0]);
    err = mtd_device_register(mtd, g_exist_Partition, part_num);
#else
    err = mtd_device_register(mtd, g_pasStatic_Partition, part_num);
#endif


#ifdef _MTK_NAND_DUMMY_DRIVER_
    dummy_driver_debug = 0;
#endif

    /* Successfully!! */
    if (!err)
    {
        MSG(INIT, "[mtk_snand] probe successfully!\n");
        nand_disable_clock();

        return err;
    }

    /* Fail!! */
  out:
    MSG(INIT, "[mtk_snand] mtk_snand_probe fail, err = %d!\n", err);
    nand_release(mtd);
    platform_set_drvdata(pdev, NULL);
    kfree(host);
    nand_disable_clock();
    return err;
}
/******************************************************************************
 * mtk_snand_suspend
 *
 * DESCRIPTION:
 *   Suspend the nand device!
 *
 * PARAMETERS:
 *   struct platform_device *pdev : device structure
 *
 * RETURNS:
 *   0 : Success
 *
 * NOTES:
 *   None
 *
 ******************************************************************************/
static int mtk_snand_suspend(struct platform_device *pdev, pm_message_t state)
{
      struct mtk_nand_host *host = platform_get_drvdata(pdev);

      // backup register
      #ifdef CONFIG_PM

      if(host->saved_para.suspend_flag==0)
      {
          nand_enable_clock();

          // Save NFI register
          host->saved_para.sNFI_CNFG_REG16 = *(NFI_CNFG_REG16);
          host->saved_para.sNFI_PAGEFMT_REG16 = *(NFI_PAGEFMT_REG16);
          host->saved_para.sNFI_CON_REG16 = *(NFI_CON_REG16);
          host->saved_para.sNFI_ACCCON_REG32 = *(NFI_ACCCON_REG32);
          host->saved_para.sNFI_INTR_EN_REG16 = *(NFI_INTR_EN_REG16);
          host->saved_para.sNFI_IOCON_REG16 = *(NFI_IOCON_REG16);
          host->saved_para.sNFI_CSEL_REG16 = *(NFI_CSEL_REG16);
          host->saved_para.sNFI_DEBUG_CON1_REG16 = *(NFI_DEBUG_CON1_REG16);

          // save ECC register
          host->saved_para.sECC_ENCCNFG_REG32 = *(ECC_ENCCNFG_REG32);
          //host->saved_para.sECC_FDMADDR_REG32 = *(ECC_FDMADDR_REG32);
          host->saved_para.sECC_DECCNFG_REG32 = *(ECC_DECCNFG_REG32);

          // save SNF
          host->saved_para.sSNAND_MISC_CTL = *(RW_SNAND_MISC_CTL);
          host->saved_para.sSNAND_MISC_CTL2 = *(RW_SNAND_MISC_CTL2);
          host->saved_para.sSNAND_DLY_CTL1 = *(RW_SNAND_DLY_CTL1);
          host->saved_para.sSNAND_DLY_CTL2 = *(RW_SNAND_DLY_CTL2);
          host->saved_para.sSNAND_DLY_CTL3 = *(RW_SNAND_DLY_CTL3);
          host->saved_para.sSNAND_DLY_CTL4 = *(RW_SNAND_DLY_CTL4);
          host->saved_para.sSNAND_CNFG = *(RW_SNAND_CNFG);

          nand_disable_clock();
          host->saved_para.suspend_flag=1;
      }
      else
      {
          MSG(POWERCTL, "[NFI] Suspend twice !\n");
      }
      #endif

      MSG(POWERCTL, "[NFI] Suspend !\n");
      return 0;
}

/******************************************************************************
 * mtk_snand_resume
 *
 * DESCRIPTION:
 *   Resume the nand device!
 *
 * PARAMETERS:
 *   struct platform_device *pdev : device structure
 *
 * RETURNS:
 *   0 : Success
 *
 * NOTES:
 *   None
 *
 ******************************************************************************/
static int mtk_snand_resume(struct platform_device *pdev)
{
    struct mtk_nand_host *host = platform_get_drvdata(pdev);

#ifdef CONFIG_PM

        if(host->saved_para.suspend_flag==1)
        {
            nand_enable_clock();
            // restore NFI register
            SNAND_WriteReg(NFI_CNFG_REG16 ,host->saved_para.sNFI_CNFG_REG16);
            SNAND_WriteReg(NFI_PAGEFMT_REG16 ,host->saved_para.sNFI_PAGEFMT_REG16);
            SNAND_WriteReg(NFI_CON_REG16 ,host->saved_para.sNFI_CON_REG16);
            SNAND_WriteReg(NFI_ACCCON_REG32 ,host->saved_para.sNFI_ACCCON_REG32);
            SNAND_WriteReg(NFI_IOCON_REG16 ,host->saved_para.sNFI_IOCON_REG16);
            SNAND_WriteReg(NFI_CSEL_REG16 ,host->saved_para.sNFI_CSEL_REG16);
            SNAND_WriteReg(NFI_DEBUG_CON1_REG16 ,host->saved_para.sNFI_DEBUG_CON1_REG16);

            // restore ECC register
            SNAND_WriteReg(ECC_ENCCNFG_REG32 ,host->saved_para.sECC_ENCCNFG_REG32);
            //SNAND_WriteReg(ECC_FDMADDR_REG32 ,host->saved_para.sECC_FDMADDR_REG32);
            SNAND_WriteReg(ECC_DECCNFG_REG32 ,host->saved_para.sECC_DECCNFG_REG32);

            // restore SNAND register
            SNAND_WriteReg(RW_SNAND_MISC_CTL ,host->saved_para.sSNAND_MISC_CTL);
            SNAND_WriteReg(RW_SNAND_MISC_CTL2 ,host->saved_para.sSNAND_MISC_CTL2);
            SNAND_WriteReg(RW_SNAND_DLY_CTL1 ,host->saved_para.sSNAND_DLY_CTL1);
            SNAND_WriteReg(RW_SNAND_DLY_CTL2 ,host->saved_para.sSNAND_DLY_CTL2);
            SNAND_WriteReg(RW_SNAND_DLY_CTL3 ,host->saved_para.sSNAND_DLY_CTL3);
            SNAND_WriteReg(RW_SNAND_DLY_CTL4 ,host->saved_para.sSNAND_DLY_CTL4);
            SNAND_WriteReg(RW_SNAND_CNFG ,host->saved_para.sSNAND_CNFG);

            // Reset NFI and ECC state machine
            /* Reset the state machine and data FIFO, because flushing FIFO */
            (void)mtk_snand_reset_con();
            // Reset ECC
            SNAND_WriteReg(ECC_DECCON_REG16, DEC_DE);
            while (!*(ECC_DECIDLE_REG16));

            SNAND_WriteReg(ECC_ENCCON_REG16, ENC_DE);
            while (!*(ECC_ENCIDLE_REG32));


            /* Initilize interrupt. Clear interrupt, read clear. */
            *(NFI_INTR_REG16);

            SNAND_WriteReg(NFI_INTR_EN_REG16 ,host->saved_para.sNFI_INTR_EN_REG16);

            nand_disable_clock();
            host->saved_para.suspend_flag = 0;
        }
        else
        {
            MSG(POWERCTL, "[NFI] Resume twice !\n");
        }
#endif
    MSG(POWERCTL, "[NFI] Resume !\n");
    return 0;
}

/******************************************************************************
 * mtk_snand_remove
 *
 * DESCRIPTION:
 *   unregister the nand device file operations !
 *
 * PARAMETERS:
 *   struct platform_device *pdev : device structure
 *
 * RETURNS:
 *   0 : Success
 *
 * NOTES:
 *   None
 *
 ******************************************************************************/

static int __devexit mtk_snand_remove(struct platform_device *pdev)
{
    struct mtk_nand_host *host = platform_get_drvdata(pdev);
    struct mtd_info *mtd = &host->mtd;

    nand_release(mtd);

    kfree(host);

    nand_disable_clock();

    return 0;
}

#if 0
int mtk_snand_dbg_read_page_raw(struct mtd_info *mtd, u32 page, u8 * dat)
{
    bool    bRet = 1;
    u32     reg;
    SNAND_Mode mode = SPIQ;
    u32     col_part2, i, len;
    u32     spare_per_sector;
    P_U8    buf_part2;
    u32     timeout = 0xFFFF;
    u32     old_dec_mode = 0;
    u32 cmd;
    struct scatterlist sg;
    enum dma_data_direction dir = DMA_FROM_DEVICE;

    sg_init_one(&sg, dat, 2048 + 128);
    dma_map_sg(&(mtd->dev), &sg, 1, dir);

    mtk_snand_reset_con();

    // 1. Read page to cache

    cmd = mtk_snand_gen_c1a3(SNAND_CMD_PAGE_READ, page); // PAGE_READ command + 3-byte address

    SNAND_WriteReg(RW_SNAND_GPRAM_DATA, cmd);
    SNAND_WriteReg(RW_SNAND_MAC_OUTL, 1 + 3);
    SNAND_WriteReg(RW_SNAND_MAC_INL , 0);

    mtk_snand_dev_mac_op(SPI);

    // 2. Get features (status polling)

    cmd = SNAND_CMD_GET_FEATURES | (SNAND_CMD_FEATURES_STATUS << 8);

    SNAND_WriteReg(RW_SNAND_GPRAM_DATA, cmd);
    SNAND_WriteReg(RW_SNAND_MAC_OUTL, 2);
    SNAND_WriteReg(RW_SNAND_MAC_INL , 1);

    for (;;)
    {
        mtk_snand_dev_mac_op(SPI);

        cmd = DRV_Reg8(((P_U8)RW_SNAND_GPRAM_DATA + 2));

        if ((cmd & SNAND_STATUS_OIP) == 0)
        {
            break;
        }
    }

    spare_per_sector = devinfo.sparesize / (devinfo.pagesize / NAND_SECTOR_SIZE);

    for (i = 0; i < 2 ; i++)
    {
        if (0 == i)
        {
            col_part2 = 0;

            buf_part2 = dat;

            len = 2112;
        }
        else
        {
            col_part2 = 2112;

            buf_part2 += len;   // append to first round

            len = 64;
        }

        //------ SNF Part ------

        // set DATA READ address
        SNAND_WriteReg(RW_SNAND_RD_CTL3, (col_part2 & SNAND_DATA_READ_ADDRESS_MASK));

        // set RW_SNAND_MISC_CTL
        reg = *(RW_SNAND_MISC_CTL);

        reg |= SNAND_DATARD_CUSTOM_EN;

        reg &= ~SNAND_DATA_READ_MODE_MASK;
        reg |= ((SNAND_DATA_READ_MODE_X4 << SNAND_DATA_READ_MODE_OFFSET) & SNAND_DATA_READ_MODE_MASK);

        SNAND_WriteReg(RW_SNAND_MISC_CTL, reg);

        // set SNF data length
        reg = len | (len << SNAND_PROGRAM_LOAD_BYTE_LEN_OFFSET);

        SNAND_WriteReg(RW_SNAND_MISC_CTL2, reg);

        //------ NFI Part ------

        mtk_snand_set_mode(CNFG_OP_CUST);
        SNAND_REG_SET_BITS(NFI_CNFG_REG16, CNFG_READ_EN);
        SNAND_REG_SET_BITS(NFI_CNFG_REG16, CNFG_AHB);
        SNAND_REG_CLN_BITS(NFI_CNFG_REG16, CNFG_HW_ECC_EN);
        mtk_snand_set_autoformat(0);

        SNAND_WriteReg(NFI_CON_REG16, 1 << CON_NFI_SEC_SHIFT);  // fixed to sector number 1

        SNAND_WriteReg(NFI_STRADDR_REG32, buf_part2);

        SNAND_WriteReg(NFI_SPIDMA_REG32, SPIDMA_SEC_EN | (len & SPIDMA_SEC_SIZE_MASK));


        SNAND_REG_CLN_BITS(NFI_CNFG_REG16, CNFG_BYTE_RW);

        // set dummy command to trigger NFI enter custom mode
        SNAND_WriteReg(NFI_CMD_REG16, NAND_CMD_DUMMYREAD);

        *(NFI_INTR_REG16);  // read clear
        SNAND_WriteReg(NFI_INTR_EN_REG16, INTR_AHB_DONE_EN);

        SNAND_REG_SET_BITS(NFI_CON_REG16, CON_NFI_BRD);

        timeout = 0xFFFF;

        while (!(*(NFI_INTR_REG16) & INTR_AHB_DONE))    // for custom read, wait NFI's INTR_AHB_DONE done to ensure all data are transferred to buffer
        {
            timeout--;

            if (0 == timeout)
            {
                return 0;
            }
        }

        timeout = 0xFFFF;

        while (((*(NFI_BYTELEN_REG16) & 0x1f000) >> 12) != 1)
        {
            timeout--;

            if (0 == timeout)
            {
                return 0;
            }
        }

        //------ NFI Part

        SNAND_REG_CLN_BITS(NFI_CON_REG16, CON_NFI_BRD);

        //------ SNF Part

        // set 1 then set 0 to clear done flag
        SNAND_WriteReg(RW_SNAND_STA_CTL1, SNAND_CUSTOM_READ);
        SNAND_WriteReg(RW_SNAND_STA_CTL1, 0);

        // clear essential SNF setting
        SNAND_REG_CLN_BITS(RW_SNAND_MISC_CTL, SNAND_DATARD_CUSTOM_EN);
    }

    dma_unmap_sg(&(mtd->dev), &sg, 1, dir);

    // restore essential NFI and ECC registers
    SNAND_WriteReg(NFI_SPIDMA_REG32, 0);

  cleanup:
    return bRet;
}
#endif

/******************************************************************************
Device driver structure
******************************************************************************/
static struct platform_driver mtk_nand_driver = {
    .probe = mtk_snand_probe,
    .remove = mtk_snand_remove,
    .suspend = mtk_snand_suspend,
    .resume = mtk_snand_resume,
    .driver = {
               .name = "mtk-nand",
               .owner = THIS_MODULE,
               },
};

/******************************************************************************
 * mtk_nand_init
 *
 * DESCRIPTION:
 *   Init the device driver !
 *
 * PARAMETERS:
 *   None
 *
 * RETURNS:
 *   None
 *
 * NOTES:
 *   None
 *
 ******************************************************************************/
static int __init mtk_nand_init(void)
{
    struct proc_dir_entry *entry;

    g_i4Interrupt = 0;

    entry = create_proc_entry(PROCNAME, 0664, NULL);    // stanley chu

    if (entry == NULL)
    {
        MSG(INIT, "MTK SNand : unable to create /proc entry\n");
        return -ENOMEM;
    }

    entry->read_proc = mtk_snand_proc_read;
    entry->write_proc = mtk_snand_proc_write;

    printk("MediaTek SNand driver init, version %s\n", VERSION);

    return platform_driver_register(&mtk_nand_driver);
}

/******************************************************************************
 * mtk_nand_exit
 *
 * DESCRIPTION:
 *   Free the device driver !
 *
 * PARAMETERS:
 *   None
 *
 * RETURNS:
 *   None
 *
 * NOTES:
 *   None
 *
 ******************************************************************************/
static void __exit mtk_nand_exit(void)
{
    MSG(INIT, "MediaTek SNand driver exit, version %s\n", VERSION);

    platform_driver_unregister(&mtk_nand_driver);
    remove_proc_entry(PROCNAME, NULL);
}

module_init(mtk_nand_init);
module_exit(mtk_nand_exit);
MODULE_LICENSE("GPL");


