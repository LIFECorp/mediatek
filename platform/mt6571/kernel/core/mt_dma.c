#include <asm/io.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/workqueue.h>
#include <linux/miscdevice.h>
#include <linux/dma-mapping.h>

#include "mach/mt_reg_base.h"
#include "mach/irqs.h"
#include "mach/dma.h"
#include "mach/sync_write.h"
#include "mach/mt_clkmgr.h"
#include "mach/emi_mpu.h"

#define DMA_DEBUG   0
#if(DMA_DEBUG == 1)
#define dbgmsg printk
#else
#define dbgmsg(...)
#endif

/*
 * DMA information
 */

#define NR_GDMA_CHANNEL           (2)
#define GDMA_START                (0)

/*
 * Register Definition
 */

#define DMA_BASE_CH(n)      (AP_DMA_BASE + 0x0080 * (n + 1))
#define DMA_GLOBAL_INT_FLAG (AP_DMA_BASE + 0x0000)

/*
 * General DMA channel register mapping:
 */
#define DMA_INT_FLAG(base)      (base + 0x0000)
#define DMA_INT_EN(base)        (base + 0x0004)
#define DMA_START(base)         (base + 0x0008)
#define DMA_RESET(base)         (base + 0x000C)
#define DMA_STOP(base)          (base + 0x0010)
#define DMA_FLUSH(base)         (base + 0x0014)
#define DMA_CON(base)           (base + 0x0018)
#define DMA_SRC(base)           (base + 0x001C)
#define DMA_DST(base)           (base + 0x0020)
#define DMA_LEN1(base)          (base + 0x0024)
#define DMA_LEN2(base)          (base + 0x0028)
#define DMA_JUMP_ADDR(base)     (base + 0x002C)
#define DMA_IBUFF_SIZE(base)    (base + 0x0030)
#define DMA_CONNECT(base)       (base + 0x0034)
#define DMA_FIXCON(base)        (base + 0x0038)
#define DMA_DBG_STAT(base)      (base + 0x0050)

/*
 * Register Setting
 */

#define DMA_GLBSTA_RUN(ch)      (0x00000001 << ((ch)))
#define DMA_GLBSTA_IT(ch)       (0x00000001 << ((ch)))
#define DMA_GDMA_LEN_MAX_MASK   (0x000FFFFF)

#define DMA_CON_DIR             (0x00000001)
#define DMA_CON_FPEN            (0x00000002)    /* Use fix pattern. */
#define DMA_CON_SLOW_EN         (0x00000004)
#define DMA_CON_DFIX            (0x00000008)
#define DMA_CON_SFIX            (0x00000010)
#define DMA_CON_WPEN            (0x00008000)
#define DMA_CON_WPSD            (0x00100000)
#define DMA_CON_WSIZE_1BYTE     (0x00000000)
#define DMA_CON_WSIZE_2BYTE     (0x01000000)
#define DMA_CON_WSIZE_4BYTE     (0x02000000)
#define DMA_CON_RSIZE_1BYTE     (0x00000000)
#define DMA_CON_RSIZE_2BYTE     (0x10000000)
#define DMA_CON_RSIZE_4BYTE     (0x20000000)
#define DMA_CON_BURST_MASK      (0x00070000)
#define DMA_CON_SLOW_OFFSET     (5)
#define DMA_CON_SLOW_MAX_MASK   (0x000003FF)

#define DMA_START_BIT           (0x00000001)
#define DMA_STOP_BIT            (0x00000000)
#define DMA_INT_FLAG_BIT        (0x00000001)
#define DMA_INT_FLAG_CLR_BIT    (0x00000000)
#define DMA_INT_EN_BIT          (0x00000001)
#define DMA_FLUSH_BIT           (0x00000001)
#define DMA_FLUSH_CLR_BIT       (0x00000000)
#define DMA_UART_RX_INT_EN_BIT  (0x00000003)
#define DMA_INT_EN_CLR_BIT      (0x00000000)
#define DMA_WARM_RST_BIT        (0x00000001)
#define DMA_HARD_RST_BIT        (0x00000002)
#define DMA_HARD_RST_CLR_BIT    (0x00000000)

#define DMA_CONN_REQ_ACK_NO_CONNECT (0x00000000)
#define DMA_CONN_REQ_ACK_SET_1      (0x00000001)
#define DMA_CONN_REQ_ACK_SET_2      (0x00000002)
#define DMA_CONN_REQ_ACK_SET_3      (0x00000003)
#define DMA_CONN_REQ_ACK_DIR_WRITE  (0x00000000)
#define DMA_CONN_REQ_ACK_DIR_READ   (0x00000004)
#define DMA_CONN_REQ_ACK_RATIO_1_2  (0x00000000)
#define DMA_CONN_REQ_ACK_RATIO_1_1  (0x00000008)

#define DMA_FIXCON_BURST_SINGLE     (0x00000000)
#define DMA_FIXCON_BURST_2BEAT      (0x00000001)
#define DMA_FIXCON_BURST_3BEAT      (0x00000002)
#define DMA_FIXCON_BURST_4BEAT      (0x00000003)
#define DMA_FIXCON_BURST_5BEAT      (0x00000004)
#define DMA_FIXCON_BURST_6BEAT      (0x00000005)
#define DMA_FIXCON_BURST_7BEAT      (0x00000006)
#define DMA_FIXCON_BURST_8BEAT      (0x00000007)
#define DMA_FIXCON_BURST_EN         (0x80000000)

/*
 * Register Limitation
 */

#define MAX_TRANSFER_LEN1   (0xFFFFF)
#define MAX_TRANSFER_LEN2   (0xFFFFF)
#define MAX_SLOW_DOWN_CNTER (0x3FF)

/*
 * channel information structures
 */

struct dma_ctrl
{
    int in_use;
    void (*isr_cb)(void *);
    void *data;
};

/*
 * global variables
 */

static struct dma_ctrl dma_ctrl[NR_GDMA_CHANNEL];
static DEFINE_SPINLOCK(dma_drv_lock);

#define PDN_APDMA_MODULE_NAME   ("APDMA")
#define GDMA_REG_BANK_SIZE      (0x50)
#define GDMA_CG_BASE_ADDR       (0xF0000024)
#define GDMA_WARM_RST_TIMEOUT   (20) /* ms */

/* 
 * mt_req_gdma: request a general DMA.
 * @chan: specify a channel or not
 * Return channel number for success; return negative errot code for failure.
 */
int mt_req_gdma(DMA_CHAN chan)
{
    unsigned long flags;
    int i;

    dbgmsg("GDMA - %s\n", __func__);

    spin_lock_irqsave(&dma_drv_lock, flags);

    if (chan == GDMA_ANY) {
        for (i = GDMA_START; i < NR_GDMA_CHANNEL; i++) {
            if (dma_ctrl[i].in_use) {
                continue;
            } else {
                dma_ctrl[i].in_use = 1;
                break;
            }
        }
    } else {
        if (dma_ctrl[chan].in_use) {
            i = NR_GDMA_CHANNEL;
        }
        else {
            i = chan;
            dma_ctrl[chan].in_use = 1;
        }
    }

    spin_unlock_irqrestore(&dma_drv_lock, flags);

    if (i < NR_GDMA_CHANNEL) {
        enable_clock(MT_CG_APDMA_SW_CG, PDN_APDMA_MODULE_NAME);

        mt_reset_gdma_conf(i);

        return i;
    } else {
        return -DMA_ERR_NO_FREE_CH;
    }
}

EXPORT_SYMBOL(mt_req_gdma);

/*
 * mt_start_gdma: start the DMA stransfer for the specified GDMA channel
 * @channel: GDMA channel to start
 * Return 0 for success; return negative errot code for failure.
 */
int mt_start_gdma(int channel)
{
    if ((channel < GDMA_START) || (channel >= (GDMA_START + NR_GDMA_CHANNEL))) {
        return -DMA_ERR_INVALID_CH;
    }else if (dma_ctrl[channel].in_use == 0) {
        return -DMA_ERR_CH_FREE;
    }

    dbgmsg("GDMA_%d - %s\n", channel+1, __func__);

    writel(DMA_INT_FLAG_CLR_BIT, DMA_INT_FLAG(DMA_BASE_CH(channel)));
    mt65xx_reg_sync_writel(DMA_START_BIT, DMA_START(DMA_BASE_CH(channel)));

    return 0;
}

EXPORT_SYMBOL(mt_start_gdma);

/*
 * mt_polling_gdma: wait the DMA to finish for the specified GDMA channel
 * @channel: GDMA channel to polling
 * @timeout: polling timeout in ms
 * Return 0 for success; return negative errot code for failure.
 */
int mt_polling_gdma(int channel, unsigned long timeout)
{
    if (channel < GDMA_START) {
        return -DMA_ERR_INVALID_CH;
    }

    if (channel >= (GDMA_START + NR_GDMA_CHANNEL)) {
        return -DMA_ERR_INVALID_CH;
    }

    if (dma_ctrl[channel].in_use == 0) {
        return -DMA_ERR_CH_FREE;
    }

    dbgmsg("GDMA_%d - %s\n", channel+1, __func__);

    timeout = jiffies + ((HZ * timeout) / 1000); 

    while (readl(DMA_START(DMA_BASE_CH(channel))))
    {
        if (time_after(jiffies, timeout)) {
            printk("GDMA_%d polling timeout !!\n", channel+1);
            return 1;
        }
    }

    return 0;
}

EXPORT_SYMBOL(mt_polling_gdma);

/*
 * mt_stop_gdma: stop the DMA stransfer for the specified GDMA channel
 * @channel: GDMA channel to stop
 * Return 0 for success; return negative errot code for failure.
 */
int mt_stop_gdma(int channel)
{
    if (channel < GDMA_START) {
        return -DMA_ERR_INVALID_CH;
    }

    if (channel >= (GDMA_START + NR_GDMA_CHANNEL)) {
        return -DMA_ERR_INVALID_CH;
    }

    if (dma_ctrl[channel].in_use == 0) {
        return -DMA_ERR_CH_FREE;
    }

    dbgmsg("GDMA_%d - %s\n", channel+1, __func__);

    writel(DMA_FLUSH_BIT, DMA_FLUSH(DMA_BASE_CH(channel)));
    while (readl(DMA_START(DMA_BASE_CH(channel))));
    writel(DMA_FLUSH_CLR_BIT, DMA_FLUSH(DMA_BASE_CH(channel)));
    mt65xx_reg_sync_writel(DMA_INT_FLAG_CLR_BIT, DMA_INT_FLAG(DMA_BASE_CH(channel)));

    return 0;
}

EXPORT_SYMBOL(mt_stop_gdma);

/*
 * mt_config_gdma: configure the given GDMA channel.
 * @channel: GDMA channel to configure
 * @config: pointer to the mt_gdma_conf structure in which the GDMA configurations store
 * @flag: ALL, SRC, DST, or SRC_AND_DST.
 * Return 0 for success; return negative errot code for failure.
 */
int mt_config_gdma(int channel, struct mt_gdma_conf *config, DMA_CONF_FLAG flag)
{
    unsigned int dma_con = 0x0, dma_conn = 0x0, dma_fixcon = 0x0, limiter = 0;

    if ((channel < GDMA_START) || (channel >= (GDMA_START + NR_GDMA_CHANNEL))) {
        return -DMA_ERR_INVALID_CH;
    }

    if (dma_ctrl[channel].in_use == 0) {
        return -DMA_ERR_CH_FREE;
    }

    if (!config) {
        return -DMA_ERR_INV_CONFIG;
    }

//    if (config->sfix && ((config->src) % 8)) {
//        printk("GDMA fixed address mode requires 8-bytes aligned address\n");
    if (config->sfix)
    {
        printk("GMDA fixed adress mode doesn't support\n");
        return -DMA_ERR_INV_CONFIG;
    }

//    if (config->dfix && ((config->dst) % 8)) {
//        printk("GDMA fixed address mode requires 8-bytes aligned address\n");
    if (config->dfix)
    {
        printk("GMDA fixed adress mode doesn't support\n");
        return -DMA_ERR_INV_CONFIG;
    }

    if (config->count > MAX_TRANSFER_LEN1)
    {
        printk("GDMA transfer length cannot exceeed 0x%x.\n", MAX_TRANSFER_LEN1);
        return -DMA_ERR_INV_CONFIG;
    }

    if (config->limiter > MAX_SLOW_DOWN_CNTER)
    {
        printk("GDMA slow down counter cannot exceeed 0x%x.\n", MAX_SLOW_DOWN_CNTER);
        return -DMA_ERR_INV_CONFIG;
    }

    if ((config->mode == DMA_WIFI) && (channel != GDMA_2)) 
    {
        printk("Only GMDA2 support WiFi application\n");
        return -DMA_ERR_INV_CONFIG;
    }

    dbgmsg("GDMA_%d - %s\n", channel+1, __func__);

    switch (flag) {
    case ALL:
        /* Control Register */
        writel(config->src, DMA_SRC(DMA_BASE_CH(channel)));
        writel(config->dst, DMA_DST(DMA_BASE_CH(channel)));
        writel((config->wplen) & DMA_GDMA_LEN_MAX_MASK, DMA_LEN2(DMA_BASE_CH(channel)));
        writel(config->wpto, DMA_JUMP_ADDR(DMA_BASE_CH(channel)));
        writel((config->count) & DMA_GDMA_LEN_MAX_MASK, DMA_LEN1(DMA_BASE_CH(channel)));

        if (config->wpen) {
            dma_con |= DMA_CON_WPEN;
        }

        if (config->wpsd) {
            dma_con |= DMA_CON_WPSD;
        }

        if (config->iten) {
            dma_ctrl[channel].isr_cb = config->isr_cb;
            dma_ctrl[channel].data = config->data;
            writel(DMA_INT_EN_BIT, DMA_INT_EN(DMA_BASE_CH(channel)));
        } else {
            dma_ctrl[channel].isr_cb = NULL;
            dma_ctrl[channel].data = NULL;
            writel(DMA_INT_EN_CLR_BIT, DMA_INT_EN(DMA_BASE_CH(channel)));
        }

        if (config->mode == DMA_WIFI) {
            if (config->dir == HIF_DIR_TX) {
                dma_con |= DMA_CON_DFIX;
                dma_con |= DMA_CON_WSIZE_4BYTE;
            } else {
                dma_con |= DMA_CON_SFIX;
                dma_con |= DMA_CON_RSIZE_4BYTE;
            }

            dma_con |= (DMA_CON_BURST_4BEAT & DMA_CON_BURST_MASK);
        } else {
            if (!(config->dfix) && !(config->sfix)) {
                dma_con |= (config->burst & DMA_CON_BURST_MASK);
            } else {
                if (config->dfix) {
                    dma_con |= DMA_CON_DFIX;
                    dma_con |= DMA_CON_WSIZE_1BYTE;
                }

                if (config->sfix) {
                    dma_con |= DMA_CON_SFIX;
                    dma_con |= DMA_CON_RSIZE_1BYTE;
                }

                // fixed src/dst mode only supports burst type SINGLE
                dma_con |= DMA_CON_BURST_SINGLE;
            }
        }

        if (config->limiter) {
            limiter = (config->limiter) & DMA_CON_SLOW_MAX_MASK;
            dma_con |= limiter << DMA_CON_SLOW_OFFSET;
            dma_con |= DMA_CON_SLOW_EN;
        }
     
        writel(dma_con, DMA_CON(DMA_BASE_CH(channel)));
        
        if (config->mode == DMA_WIFI) {
            /* Connect Register */
            dma_conn |= DMA_CONN_REQ_ACK_SET_1;
            dma_conn |= DMA_CONN_REQ_ACK_RATIO_1_1;

            if (config->dir == HIF_DIR_TX)
                dma_conn |= DMA_CONN_REQ_ACK_DIR_WRITE;
            else
                dma_conn |= DMA_CONN_REQ_ACK_DIR_READ;
            
            writel(dma_conn, DMA_CONNECT(DMA_BASE_CH(channel)));

            /* Fixed Address Control Register*/            
            dma_fixcon |= DMA_FIXCON_BURST_4BEAT;
            dma_fixcon |= DMA_FIXCON_BURST_EN;
            writel(dma_fixcon, DMA_FIXCON(DMA_BASE_CH(channel)));
        } else {
            if (channel == GDMA_2) {
                writel(dma_conn, DMA_CONNECT(DMA_BASE_CH(channel)));
                writel(dma_fixcon, DMA_FIXCON(DMA_BASE_CH(channel)));
            }
        }
        
        break;

    case SRC:
        writel(config->src, DMA_SRC(DMA_BASE_CH(channel)));

        break;

    case DST:
        writel(config->dst, DMA_DST(DMA_BASE_CH(channel)));
        break;

    case SRC_AND_DST:
        writel(config->src, DMA_SRC(DMA_BASE_CH(channel)));
        writel(config->dst, DMA_DST(DMA_BASE_CH(channel)));
        break;

    default:
        break;
    } 

    /* use the data synchronization barrier to ensure that all writes are completed */
    dsb();

    return 0;
}

EXPORT_SYMBOL(mt_config_gdma);

/*
 * mt_free_gdma: free a general DMA.
 * @channel: channel to free
 * Return 0 for success; return negative errot code for failure.
 */
int mt_free_gdma(int channel)
{
    unsigned long flags;
    
    if (channel < GDMA_START) {
        return -DMA_ERR_INVALID_CH;
    }

    if (channel >= (GDMA_START + NR_GDMA_CHANNEL)) {
        return -DMA_ERR_INVALID_CH;
    }

    if (dma_ctrl[channel].in_use == 0) {
        return -DMA_ERR_CH_FREE;
    }

    dbgmsg("GDMA_%d - %s\n", channel+1, __func__);

    mt_stop_gdma(channel);

    dma_ctrl[channel].isr_cb = NULL;
    dma_ctrl[channel].data = NULL;
    dma_ctrl[channel].in_use = 0;

    disable_clock(MT_CG_APDMA_SW_CG, PDN_APDMA_MODULE_NAME);
    
    return 0;
}

EXPORT_SYMBOL(mt_free_gdma);

/*
 * mt_dump_gdma: dump registers for the specified GDMA channel
 * @channel: GDMA channel to dump registers
 * Return 0 for success; return negative errot code for failure.
 */
int mt_dump_gdma(int channel)
{
    unsigned int i, reg;
    unsigned int j = 0;

    printk("GDMA> Register content 0x%X=\n\t", DMA_BASE_CH(channel));
    for (i = 0; i <= GDMA_REG_BANK_SIZE; i += 4)
    {
        reg = readl(DMA_BASE_CH(channel)+i);
        printk("0x%08X ", reg);

        if (j++ >= 3)
        {
            printk("\n");
            printk("GDMA> Register content 0x%X=\n\t", DMA_BASE_CH(channel)+i+4);
            j = 0;
        }
    }

    printk("\nGDMA> clock status = 0x%X\n\n", *(volatile unsigned int *)GDMA_CG_BASE_ADDR);

    return 0;
}

EXPORT_SYMBOL(mt_dump_gdma);

/*
 * mt_warm_reset_gdma: warm reset the specified GDMA channel
 * @channel: GDMA channel to warm reset
 * Return 0 for success; return negative errot code for failure.
 */
int mt_warm_reset_gdma(int channel)
{
    if (channel < GDMA_START) {
        return -DMA_ERR_INVALID_CH;
    }

    if (channel >= (GDMA_START + NR_GDMA_CHANNEL)) {
        return -DMA_ERR_INVALID_CH;
    }

    if (dma_ctrl[channel].in_use == 0) {
        return -DMA_ERR_CH_FREE;
    }

    printk("GDMA_%d Warm Reset !!\n", channel+1);

    mt65xx_reg_sync_writel(DMA_WARM_RST_BIT, DMA_RESET(DMA_BASE_CH(channel)));
    
    if (mt_polling_gdma(channel, GDMA_WARM_RST_TIMEOUT) != 0)
        return 1;
    else
        return 0;
}

EXPORT_SYMBOL(mt_warm_reset_gdma);

/*
 * mt_hard_reset_gdma: hard reset the specified GDMA channel
 * @channel: GDMA channel to hard reset
 * Return 0 for success; return negative errot code for failure.
 */
int mt_hard_reset_gdma(int channel)
{
    if (channel < GDMA_START) {
        return -DMA_ERR_INVALID_CH;
    }

    if (channel >= (GDMA_START + NR_GDMA_CHANNEL)) {
        return -DMA_ERR_INVALID_CH;
    }

    if (dma_ctrl[channel].in_use == 0) {
        return -DMA_ERR_CH_FREE;
    }

    printk(KERN_ERR "GDMA_%d Hard Reset !!\n", channel+1);

    mt65xx_reg_sync_writel(DMA_HARD_RST_BIT, DMA_RESET(DMA_BASE_CH(channel)));
    mt65xx_reg_sync_writel(DMA_HARD_RST_CLR_BIT, DMA_RESET(DMA_BASE_CH(channel)));
    
    return 0;
}

EXPORT_SYMBOL(mt_hard_reset_gdma);

/*
 * mt_reset_gdma: reset the specified GDMA channel
 * @channel: GDMA channel to reset
 * Return 0 for success; return negative errot code for failure.
 */
int mt_reset_gdma(int channel)
{
    if (channel < GDMA_START) {
        return -DMA_ERR_INVALID_CH;
    }

    if (channel >= (GDMA_START + NR_GDMA_CHANNEL)) {
        return -DMA_ERR_INVALID_CH;
    }

    if (dma_ctrl[channel].in_use == 0) {
        return -DMA_ERR_CH_FREE;
    }

    dbgmsg("GDMA_%d Reset !!\n", channel+1);

    if (mt_warm_reset_gdma(channel) != 0)
        mt_hard_reset_gdma(channel);

    return 0;
}

EXPORT_SYMBOL(mt_reset_gdma);

/*
 * gdma1_irq_handler: general DMA channel 1 interrupt service routine.
 * @irq: DMA IRQ number
 * @dev_id:
 * Return IRQ returned code.
 */
static irqreturn_t gdma1_irq_handler(int irq, void *dev_id)
{
    volatile glbsta = readl(DMA_GLOBAL_INT_FLAG);

    dbgmsg("DMA Module - %s ISR Start\n", __func__);
    dbgmsg("DMA Module - GLBSTA = 0x%x\n", glbsta);

    if (glbsta & DMA_GLBSTA_IT(G_DMA_1)){
        if (dma_ctrl[G_DMA_1].isr_cb) {
            dma_ctrl[G_DMA_1].isr_cb(dma_ctrl[G_DMA_1].data);
        }

        mt65xx_reg_sync_writel(DMA_INT_FLAG_CLR_BIT, DMA_INT_FLAG(DMA_BASE_CH(G_DMA_1)));
#if(DMA_DEBUG == 1)
        glbsta = readl(DMA_GLOBAL_INT_FLAG);
        printk("DMA Module - GLBSTA after ack = 0x%x\n", glbsta);
#endif
    }

    dbgmsg("DMA Module - %s ISR END\n", __func__);

    return IRQ_HANDLED;
}

/*
 * gdma2_irq_handler: general DMA channel 2 interrupt service routine.
 * @irq: DMA IRQ number
 * @dev_id:
 * Return IRQ returned code.
 */
static irqreturn_t gdma2_irq_handler(int irq, void *dev_id)
{
    volatile glbsta = readl(DMA_GLOBAL_INT_FLAG);

    dbgmsg("DMA Module - %s ISR Start\n", __func__);
    dbgmsg("DMA Module - GLBSTA = 0x%x\n", glbsta);

    if (glbsta & DMA_GLBSTA_IT(G_DMA_2)){
        if (dma_ctrl[G_DMA_2].isr_cb) {
            dma_ctrl[G_DMA_2].isr_cb(dma_ctrl[G_DMA_2].data);
        }

        mt65xx_reg_sync_writel(DMA_INT_FLAG_CLR_BIT, DMA_INT_FLAG(DMA_BASE_CH(G_DMA_2)));

#if(DMA_DEBUG == 1)
        glbsta = readl(DMA_GLOBAL_INT_FLAG);
        printk("DMA Module - GLBSTA after ack = 0x%x\n", glbsta);
#endif
    }

    dbgmsg("DMA Module - %s ISR END\n", __func__);
    
    return IRQ_HANDLED;
}

/*
 * mt_reset_gdma_conf: reset the config of the specified DMA channel
 * @iChannel: channel number of the DMA channel to reset
 */
static void mt_reset_gdma_conf(const unsigned int iChannel)
{
    struct mt_gdma_conf conf;

    dbgmsg("GDMA_%d - %s\n", iChannel+1, __func__);

    memset(&conf, 0, sizeof(struct mt_gdma_conf));

    if (mt_config_gdma(iChannel, &conf, ALL) != 0){
        return;
    }

    return;
}

//#define DMA_TEST 1
#if(DMA_TEST == 1)

unsigned int *dma_dst_array_v;
unsigned int *dma_src_array_v;
dma_addr_t dma_dst_array_p;
dma_addr_t dma_src_array_p;

#define TEST_LEN 4000
#define LEN (TEST_LEN / sizeof(int))

void irq_dma_handler(void * data)
{
    int channel = (int)data;
    printk("irq_dma_handler called\n");
    int i = 0;
    for(i = 0; i < LEN; i++) {
        if(dma_dst_array_v[i] != dma_src_array_v[i]) {
            printk("DMA failed, src = %d, dst = %d, i = %d\n", dma_src_array_v[i], dma_dst_array_v[i], i);
            break;
        }
    }

    if(i == LEN)
        printk("DMA verified ok\n");

    mt_free_gdma(channel);
}

void APDMA_test_transfer(int testcase)
{
    int i, channel;
    struct mt_gdma_conf conf;

    channel = mt_req_gdma(GDMA_ANY);

    printk("GDMA channel:%d\n",channel);
    if(channel < 0 ){
        printk("ERROR Register DMA\n");
        return;
    }

    dma_dst_array_v = dma_alloc_coherent(NULL, TEST_LEN, &dma_dst_array_p, GFP_KERNEL ); // 25 unsinged int
    dma_src_array_v = dma_alloc_coherent(NULL, TEST_LEN, &dma_src_array_p, GFP_KERNEL );
        
    memset(&conf, 0, sizeof(struct mt_gdma_conf));

    conf.count = TEST_LEN;
    conf.src = dma_src_array_p;
    conf.dst = dma_dst_array_p;
    conf.iten = (testcase == 2) ? DMA_FALSE : DMA_TRUE;
    conf.isr_cb = (testcase == 2) ? NULL : irq_dma_handler;
    conf.data = channel;
    conf.limiter = (testcase == 3 || testcase == 4) ? 0x3FF : 0;

    for(i = 0; i < LEN; i++) {
        dma_dst_array_v[i] = 0;
        dma_src_array_v[i] = i;
    }
    
    if ( mt_config_gdma(channel, &conf, ALL) != 0) {
        printk("ERROR set DMA\n");
        goto _exit;
        return;
    }
    
    /*
    unsigned int dma_src = readl(DMA_SRC(DMA_BASE_CH(channel)));
    unsigned int dma_dst = readl(DMA_DST(DMA_BASE_CH(channel)));
    unsigned int len = readl(DMA_LEN1(DMA_BASE_CH(channel)));
    printk("start dma channel %d src = 0x%x, dst = 0x%x, len = %d bytes\n", channel, dma_src, dma_dst, len);
    */
    
    printk("Start %d\n",mt_start_gdma(channel));
    switch(testcase)
    {
        case 2:
            if (mt_polling_gdma(channel, GDMA_WARM_RST_TIMEOUT) != 0)
                printk("Polling transfer failed\n");    
            else
                printk("Polling succeeded\n");
            mt_free_gdma(channel);
            break;
        case 3:
            mt_warm_reset_gdma(channel);
            
            for(i = 0; i < LEN; i++) {
                if(dma_dst_array_v[i] != dma_src_array_v[i]) {
                    printk("Warm reset succeeded\n");
                    break;
                }
                mt_free_gdma(channel);
            }

            if(i == LEN)
                printk("Warm reset failed\n");
            break;
            
        case 4:
            mt_hard_reset_gdma(channel);
            
            for(i = 0; i < LEN; i++) {
                if(dma_dst_array_v[i] != dma_src_array_v[i]) {
                    printk("Hard reset succeeded\n");
                    break;
                }
                mt_free_gdma(channel);
            }
            if(i == LEN)
                printk("Hard reset failed\n");
            break;
        
        default:
            break;

    }

_exit:
    if(dma_dst_array_v){
        dma_free_coherent(NULL, TEST_LEN, dma_dst_array_v, dma_dst_array_p);
        dma_dst_array_v = dma_dst_array_p = NULL;
    }

    if(dma_src_array_v){
        dma_free_coherent(NULL, TEST_LEN, dma_src_array_v, dma_src_array_p);
        dma_src_array_v = dma_src_array_p = NULL;
    }

    return;
}

static ssize_t test_show(struct device* dev, struct device_attribute* attr, char* buf)
{
   return snprintf(buf, PAGE_SIZE, "==APDMA test==\n"
                                   "1.APDMA transfer (interrupt mode)\n"
                                   "2.APDMA transfer (polling mode)\n"
                                   "3.APDMA warm reset\n"
                                   "4.APDMA hard reset\n"
   ); 
}

static ssize_t test_store(struct device* dev, struct device_attribute* attr, const char* buf, size_t count)
{
	char *p = (char *)buf;
	unsigned int num;

	num = simple_strtoul(p, &p, 10);
        switch(num){
            /* Test APDMA Normal Function */
            case 1:
                APDMA_test_transfer(1);
                break;
            case 2:
                APDMA_test_transfer(2);
                break;
            case 3:
                APDMA_test_transfer(3);
                break;
            case 4:
                APDMA_test_transfer(4);
                break;
            default:
                break;
        }

	return count;
}

DEVICE_ATTR(dma_config, 0666, test_show, test_store);

static struct miscdevice mt_dma_dev = {
    .minor      = MISC_DYNAMIC_MINOR,
    .name       = "mt_dma",
    //.fops       = &dev_fops,
    .mode       = S_IRUGO | S_IWUGO,
};
#endif

/*
 * mt_init_dma: initialize DMA.
 * Always return 0.
 */
static int __init mt_init_dma(void)
{
    int i, ret;

    for (i = 0; i < NR_GDMA_CHANNEL; i++) {
        mt_reset_gdma_conf(i);
    }

    if (request_irq(MT_GDMA1_IRQ_ID, gdma1_irq_handler, IRQF_TRIGGER_LOW, "GDMA1",  NULL)) {
        printk(KERN_ERR"GDMA1 IRQ LINE NOT AVAILABLE!!\n");
    }

    if (request_irq(MT_GDMA2_IRQ_ID, gdma2_irq_handler, IRQF_TRIGGER_LOW, "GDMA2",  NULL)) {
        printk(KERN_ERR"GDMA2 IRQ LINE NOT AVAILABLE!!\n");
    }
    
    #if(DMA_TEST == 1)
    ret = misc_register(&mt_dma_dev);
    
    if(ret < 0) {
        printk("DMA MISC Register fail, ret = %d\n", ret);
        return ret;
    }

    device_create_file(mt_dma_dev.this_device, &dev_attr_dma_config);
    #endif

    printk("[APDMA] Init APDMA OK\n");

    return 0;
}

arch_initcall(mt_init_dma);

