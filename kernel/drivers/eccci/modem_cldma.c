/*
 * this is a CLDMA modem driver for 6595.
 *
 * V0.1: Xiao Wang <xiao.wang@mediatek.com>
 */

#include <linux/list.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/kdev_t.h>
#include <linux/slab.h>
#include <linux/skbuff.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/timer.h>
#include <linux/fs.h>
#include <linux/netdevice.h>
#include <linux/random.h>
#include <linux/platform_device.h>
#ifndef MT6290
#include <mach/mt_boot.h>
#endif
#include "ccci_core.h"
#include "ccci_bm.h"
#include "ccci_dfo.h"
#include "ccci_sysfs.h"
#include "ccci_platform.h"
#include "modem_cldma.h"
#include "cldma_platform.h"
#include "cldma_reg.h"

// always keep this in mind: what if there are more than 1 modems using CLDMA...

extern void md_ex_monitor_func(unsigned long data);
extern void md_bootup_timeout_func(unsigned long data);
extern unsigned int ccci_get_md_debug_mode(struct ccci_modem *md);

// Port mapping
extern struct ccci_port_ops char_port_ops;
extern struct ccci_port_ops net_port_ops;
extern struct ccci_port_ops kernel_port_ops;
extern struct ccci_port_ops ipc_port_ack_ops;
static struct ccci_port md_cd_ports_normal[] = {
#ifdef MT6290
{CCCI_MONITOR_CH,	CCCI_MONITOR_CH,	0xFF,	0xFF,	0xFF,	0xFF,	4,	&char_port_ops, 	MD_CD_MAJOR,	0,	"eemcs_monitor",	},
{CCCI_PCM_TX,		CCCI_PCM_RX,		0,		0,		0xFF,	0xFF,	4,	&char_port_ops, 	MD_CD_MAJOR,	1,	"eemcs_aud",	},
{CCCI_CONTROL_TX,	CCCI_CONTROL_RX,	0,		0,		0,		0,		0,	&kernel_port_ops,	0,				0,	"eemcs_ctrl",	},
{CCCI_SYSTEM_TX,	CCCI_SYSTEM_RX, 	0,		0,		0xFF,	0xFF,	0,	&kernel_port_ops,	0,				0,	"eemcs_sys",	},

{CCCI_UART1_TX, 	CCCI_UART1_RX,		1,		1,		3,		3,		0,	&char_port_ops, 	MD_CD_MAJOR,	2,	"eemcs_md_log_ctrl",	},
{CCCI_UART2_TX, 	CCCI_UART2_RX,		1,		1,		0xFF,	0xFF,	0,	&char_port_ops, 	MD_CD_MAJOR,	3,	"eemcs_mux",	},
{CCCI_FS_TX,		CCCI_FS_RX, 		1,		1,		1,		1,		4,	&char_port_ops, 	MD_CD_MAJOR,	4,	"eemcs_fs",	},
{CCCI_IPC_UART_TX,	CCCI_IPC_UART_RX,	1,		1,		0xFF,	0xFF,	0,	&char_port_ops, 	MD_CD_MAJOR,	5,	"eemcs_ipc_uart",	},
{CCCI_ICUSB_TX, 	CCCI_ICUSB_RX,		1,		1,		0xFF,	0xFF,	0,	&char_port_ops, 	MD_CD_MAJOR,	6,	"eemcs_ICUSB",	},
{CCCI_RPC_TX,		CCCI_RPC_RX,		1,		1,		0xFF,	0xFF,	0,	&kernel_port_ops,	0,				0,	"eemcs_rpc",	},

{CCCI_IPC_TX,		CCCI_IPC_RX,		1,		1,		0xFF,	0xFF,	0,	&char_port_ops, 	MD_CD_MAJOR,	0,	"eemcs_ipc_0",	},
{CCCI_IPC_RX_ACK,	CCCI_IPC_TX_ACK,	1,		1,		0xFF,	0xFF,	0,	&ipc_port_ack_ops,	0,				0,	"eemcs_ipc_0_ack",	},
{CCCI_IPC_TX,		CCCI_IPC_RX,		1,		1,		0xFF,	0xFF,	0,	&char_port_ops, 	MD_CD_MAJOR,	2,	"eemcs_ipc_2",	},
{CCCI_IPC_RX_ACK,	CCCI_IPC_TX_ACK,	1,		1,		0xFF,	0xFF,	0,	&ipc_port_ack_ops,	0,				0,	"eemcs_ipc_2_ack",	},

{CCCI_MD_LOG_TX,	CCCI_MD_LOG_RX, 	2,		2,		2,		2,		0,	&char_port_ops, 	MD_CD_MAJOR,	7,	"eemcs_md_log",	},

{CCCI_CCMNI1_TX,	CCCI_CCMNI1_RX, 	3,		3,		0xFF,	0xFF,	8,	&net_port_ops,		0,				0,	"ccemni0",	},
{CCCI_CCMNI2_TX,	CCCI_CCMNI2_RX, 	4,		4,		0xFF,	0xFF,	8,	&net_port_ops,		0,				0,	"ccemni1",	},
{CCCI_CCMNI3_TX,	CCCI_CCMNI3_RX, 	5,		5,		0xFF,	0xFF,	8,	&net_port_ops,		0,				0,	"ccemni2",	},

{CCCI_IMSV_UL,		CCCI_IMSV_DL,		6,		6,		0xFF,	0xFF,	0,	&char_port_ops, 	MD_CD_MAJOR,	8,	"eemcs_imsv",	},
{CCCI_IMSC_UL,		CCCI_IMSC_DL,		6,		6,		0xFF,	0xFF,	0,	&char_port_ops, 	MD_CD_MAJOR,	9,	"eemcs_imsc",	},
{CCCI_IMSA_UL,		CCCI_IMSA_DL,		6,		6,		0xFF,	0xFF,	0,	&char_port_ops, 	MD_CD_MAJOR,	10, "eemcs_imsa",	},
{CCCI_IMSDC_UL, 	CCCI_IMSDC_DL,		6,		6,		0xFF,	0xFF,	0,	&char_port_ops, 	MD_CD_MAJOR,	11, "eemcs_imsdc",	},

{CCCI_DUMMY_CH, 	CCCI_DUMMY_CH,		0xFF,	0xFF,	0xFF,	0xFF,	0,	&char_port_ops, 	MD_CD_MAJOR,	12, "eemcs_muxrp",	},
{CCCI_DUMMY_CH, 	CCCI_DUMMY_CH,		0xFF,	0xFF,	0xFF,	0xFF,	0,	&char_port_ops, 	MD_CD_MAJOR,	13, "eemcs_ril",	},

{CCCI_IT_TX,		CCCI_IT_RX, 		0,		0,		0xFF,	0xFF,	4,	&char_port_ops, 	MD_CD_MAJOR,	17, "eemcs_it",	},
{CCCI_LB_IT_TX, 	CCCI_LB_IT_RX,		0,		0,		0xFF,	0xFF,	4,	&char_port_ops, 	MD_CD_MAJOR,	18, "eemcs_lb_it",	},
#else
{CCCI_MONITOR_CH,	CCCI_MONITOR_CH,	0xFF,	0xFF,	0xFF,	0xFF,	4,	&char_port_ops, 	MD_CD_MAJOR,	0,	"ccci_monitor",	},
{CCCI_PCM_TX,		CCCI_PCM_RX,		0,		0,		0xFF,	0xFF,	4,	&char_port_ops, 	MD_CD_MAJOR,	1,	"ccci_aud",	},
{CCCI_CONTROL_TX,	CCCI_CONTROL_RX,	0,		0,		0,		0,		0,	&kernel_port_ops,	0,				0,	"ccci_ctrl",	},
{CCCI_SYSTEM_TX,	CCCI_SYSTEM_RX, 	0,		0,		0xFF,	0xFF,	0,	&kernel_port_ops,	0,				0,	"ccci_sys",	},

{CCCI_UART1_TX, 	CCCI_UART1_RX,		1,		1,		3,		3,		0,	&char_port_ops, 	MD_CD_MAJOR,	2,	"ccci_md_log_ctrl",	},
{CCCI_UART2_TX, 	CCCI_UART2_RX,		1,		1,		0xFF,	0xFF,	0,	&char_port_ops, 	MD_CD_MAJOR,	3,	"ttyC0",	},
{CCCI_FS_TX,		CCCI_FS_RX, 		1,		1,		1,		1,		4,	&char_port_ops, 	MD_CD_MAJOR,	4,	"ccci_fs",	},
{CCCI_IPC_UART_TX,	CCCI_IPC_UART_RX,	1,		1,		0xFF,	0xFF,	0,	&char_port_ops, 	MD_CD_MAJOR,	5,	"ttyC2",	},
{CCCI_ICUSB_TX, 	CCCI_ICUSB_RX,		1,		1,		0xFF,	0xFF,	0,	&char_port_ops, 	MD_CD_MAJOR,	6,	"ttyC3",	},
{CCCI_RPC_TX,		CCCI_RPC_RX,		1,		1,		0xFF,	0xFF,	0,	&kernel_port_ops,	0,				0,	"ccci_rpc",	},

{CCCI_IPC_TX,		CCCI_IPC_RX,		1,		1,		0xFF,	0xFF,	0,	&char_port_ops, 	MD_CD_MAJOR,	0,	"ccci_ipc_1220_0",	},
{CCCI_IPC_RX_ACK,	CCCI_IPC_TX_ACK,	1,		1,		0xFF,	0xFF,	0,	&ipc_port_ack_ops,	0,				0,	"ccci_ipc_1220_0_ack",	},
{CCCI_IPC_TX,		CCCI_IPC_RX,		1,		1,		0xFF,	0xFF,	0,	&char_port_ops, 	MD_CD_MAJOR,	2,	"ccci_ipc_2",	},
{CCCI_IPC_RX_ACK,	CCCI_IPC_TX_ACK,	1,		1,		0xFF,	0xFF,	0,	&ipc_port_ack_ops,	0,				0,	"ccci_ipc_2_ack",	},

{CCCI_MD_LOG_TX,	CCCI_MD_LOG_RX, 	2,		2,		2,		2,		0,	&char_port_ops, 	MD_CD_MAJOR,	7,	"ttyC1",	},

{CCCI_CCMNI1_TX,	CCCI_CCMNI1_RX, 	3,		3,		0xFF,	0xFF,	8,	&net_port_ops,		0,				0,	"ccmni0",	},
{CCCI_CCMNI2_TX,	CCCI_CCMNI2_RX, 	4,		4,		0xFF,	0xFF,	8,	&net_port_ops,		0,				0,	"ccmni1",	},
{CCCI_CCMNI3_TX,	CCCI_CCMNI3_RX, 	5,		5,		0xFF,	0xFF,	8,	&net_port_ops,		0,				0,	"ccmni2",	},

{CCCI_IMSV_UL,		CCCI_IMSV_DL,		6,		6,		0xFF,	0xFF,	0,	&char_port_ops, 	MD_CD_MAJOR,	8,	"ccci_imsv",	},
{CCCI_IMSC_UL,		CCCI_IMSC_DL,		6,		6,		0xFF,	0xFF,	0,	&char_port_ops, 	MD_CD_MAJOR,	9,	"ccci_imsc",	},
{CCCI_IMSA_UL,		CCCI_IMSA_DL,		6,		6,		0xFF,	0xFF,	0,	&char_port_ops, 	MD_CD_MAJOR,	10, "ccci_imsa",	},
{CCCI_IMSDC_UL, 	CCCI_IMSDC_DL,		6,		6,		0xFF,	0xFF,	0,	&char_port_ops, 	MD_CD_MAJOR,	11, "ccci_imsdc",	},

{CCCI_DUMMY_CH, 	CCCI_DUMMY_CH,		0xFF,	0xFF,	0xFF,	0xFF,	0,	&char_port_ops, 	MD_CD_MAJOR,	12, "ccci_ioctl0",	},
{CCCI_DUMMY_CH, 	CCCI_DUMMY_CH,		0xFF,	0xFF,	0xFF,	0xFF,	0,	&char_port_ops, 	MD_CD_MAJOR,	13, "ccci_ioctl1",	},
{CCCI_DUMMY_CH, 	CCCI_DUMMY_CH,		0xFF,	0xFF,	0xFF,	0xFF,	0,	&char_port_ops, 	MD_CD_MAJOR,	14, "ccci_ioctl2",	},
{CCCI_DUMMY_CH, 	CCCI_DUMMY_CH,		0xFF,	0xFF,	0xFF,	0xFF,	0,	&char_port_ops, 	MD_CD_MAJOR,	15, "ccci_ioctl3",	},
{CCCI_DUMMY_CH, 	CCCI_DUMMY_CH,		0xFF,	0xFF,	0xFF,	0xFF,	0,	&char_port_ops, 	MD_CD_MAJOR,	16, "ccci_ioctl4",	},

{CCCI_IT_TX,		CCCI_IT_RX, 		0,		0,		0xFF,	0xFF,	4,	&char_port_ops, 	MD_CD_MAJOR,	17, "eemcs_it",	},
{CCCI_LB_IT_TX, 	CCCI_LB_IT_RX,		0,		0,		0xFF,	0xFF,	0,	&char_port_ops, 	MD_CD_MAJOR,	18, "eemcs_lb_it",	},
#endif
};

// CLDMA setting
// we use this as rgpd->data_allow_len, so skb length must be >= this size, check ccci_bm.c's skb pool design
static int rx_queue_buffer_size[8] = {SKB_4K, SKB_4K, SKB_4K, SKB_1_5K, SKB_1_5K, SKB_1_5K, SKB_4K, SKB_4K};
static const unsigned char high_priority_queue_mask =  0x00;
#ifdef CCCI_USE_NAPI
#define NAPI_QUEUE_MASK 0x38 // only Rx-exclusive port can enable NAPI
#else
#define NAPI_QUEUE_MASK 0x00
#endif
#define NONSTOP_QUEUE_MASK 0xF0 // for convenience, queue 0,1,2,3 are non-stop
#define NONSTOP_QUEUE_MASK_32 0xF0F0F0F0

#define CLDMA_CG_POLL 6
#define BOOT_TIMER_ON 10
#define BOOT_TIMER_HS1 10

#define TAG "mcd"

/*
 * do NOT add any static data, data should be in modem's instance
 */
 
static void cldma_gpd_ring_dump(void *start)
{
	// assume TGPD and RGPD's "next" pointers use the same offset
	struct cldma_tgpd *curr = (struct cldma_tgpd *)start;
	int i, *tmp;
	printk("[CLDMA] gpd starts from %p\n", start);
	/*
	 * virtual address get from dma_pool_alloc is not equal to phys_to_virt. 
	 * e.g. dma_pool_alloca returns 0xFFDFF00, and phys_to_virt will return 0xDF364000 for the same
	 * DMA address. therefore we can't compare gpd address with @start to exit loop.
	 */
	for(i=0; i<RING_BUFF_SIZE; i++) {
		tmp = (int *) curr;
		printk("[CLDMA] %p: %X %X %X %X\n", curr, *tmp, *(tmp+1), *(tmp+2), *(tmp+3));
		curr = (struct cldma_tgpd *)phys_to_virt(curr->next_gpd_ptr);
	}
	printk("[CLDMA] end of ring %p\n", start);
}

static void cldma_dump_all_gpd(struct md_cd_ctrl *md_ctrl)
{
	int i;
	struct ccci_request *req = NULL;
	for(i=0; i<QUEUE_LEN(md_ctrl->txq); i++) {
		// use GPD's pointer to traverse
		req = list_entry(md_ctrl->txq[i].tr_ring, struct ccci_request, entry);
		cldma_gpd_ring_dump(req->gpd);
		// use request's link head to traverse
		printk("[CLDMA] dump txq %d request\n", i);
		list_for_each_entry(req, md_ctrl->txq[i].tr_ring, entry) { // due to we do NOT have an extra head, this will miss the first request		
			printk("[CLDMA] %p (%x->%x)\n", req->gpd, 
				req->gpd_addr, ((struct cldma_tgpd *)req->gpd)->next_gpd_ptr);
		}
	}
	for(i=0; i<QUEUE_LEN(md_ctrl->rxq); i++) {
		// use GPD's pointer to traverse
		req = list_entry(md_ctrl->rxq[i].tr_ring, struct ccci_request, entry);
		cldma_gpd_ring_dump(req->gpd);
		// use request's link head to traverse
		printk("[CLDMA] dump rxq %d request\n", i);
		list_for_each_entry(req, md_ctrl->rxq[i].tr_ring, entry) {
			printk("[CLDMA] %p/%p (%x->%x)\n", req->gpd, req->skb,
				req->gpd_addr, ((struct cldma_rgpd *)req->gpd)->next_gpd_ptr);
		}
	}
}

#if CHECKSUM_SIZE
static inline void caculate_checksum(char *address, char first_byte)
{
	int i;
	char sum = first_byte;
	for (i = 2 ; i < CHECKSUM_SIZE; i++)
		sum += *(address + i);
	*(address + 1) = 0xFF - sum; 
}
#else
#define caculate_checksum(address, first_byte)	 
#endif

// this function may be called from both workqueue and softirq (NAPI)
static int cldma_rx_collect(struct md_cd_queue *queue, int budget)
{
	struct ccci_modem *md = queue->modem;
	
	struct ccci_request *req = queue->tr_done;
	struct cldma_rgpd *rgpd = (struct cldma_rgpd *)req->gpd;
	struct ccci_request *new_req = NULL;
	int ret=0, count=0;
	int result = 0;
	struct ccci_header *ccci_h = NULL;

	// find all done RGPD
	while((rgpd->gpd_flags&0x1) == 0) { // not hardware own
		dma_unmap_single(NULL, rgpd->data_buff_bd_ptr, skb_size(req->skb), DMA_FROM_DEVICE);
		if(unlikely(!req->skb)) {
			CCCI_ERR_MSG(md->index, TAG, "find a hole on q%d, try refill and move forward\n", queue->index);
			goto fill_and_move;
		}
		if(req->skb->len!=0) {
			// should never happen
			CCCI_ERR_MSG(md->index, TAG, "reuse skb %p with len %d\n", req->skb, req->skb->len);
			break;
		}
		// update skb
		skb_put(req->skb, rgpd->data_buff_len);
		// allocate a new wrapper
		new_req = ccci_alloc_req(IN, -1, 1, 0);
		new_req->skb = req->skb;
		INIT_LIST_HEAD(&new_req->entry); // as port will run list_del
		CCCI_DBG_MSG(md->index, TAG, "handle Rxq%d skb=%p len=%d\n", queue->index, new_req->skb, rgpd->data_buff_len);
		ccci_h = (struct ccci_header *)new_req->skb->data;
		if(atomic_cmpxchg(&md->wakeup_src, 1, 0) == 1)
			CCCI_INF_MSG(md->index, TAG, "CLDMA_MD wakeup source:(%d/%d)\n", queue->index, ccci_h->channel);
		CCCI_DBG_MSG(md->index, TAG, "Rx msg %x %x %x %x\n", ccci_h->data[0], ccci_h->data[1], ccci_h->channel, ccci_h->reserved);
		ret = ccci_port_recv_request(md, new_req);
		CCCI_DBG_MSG(md->index, TAG, "Rx port recv req ret=%d\n", ret);
		if(ret>=0 || ret==-CCCI_ERR_DROP_PACKET) {
fill_and_move:
			CCCI_DBG_MSG(md->index, TAG, "harvest RGPD %p for q%d\n", rgpd, queue->index);
			// allocate a new skb and change skb pointer
			req->skb = ccci_alloc_skb(rx_queue_buffer_size[queue->index]);
			if(likely(req->skb)) {
				rgpd->data_buff_bd_ptr = dma_map_single(NULL, req->skb->data, skb_size(req->skb), DMA_FROM_DEVICE);
				// checksum of GPD
				caculate_checksum((char *)rgpd, 0x81);
				// update GPD
#ifndef MT6290
				cldma_write8(&rgpd->gpd_flags, 0, 0x81);
#else
				wmb();
				rgpd->gpd_flags = 0x81;
#endif
				// step forward
				req = list_entry(req->entry.next, struct ccci_request, entry);
				rgpd = (struct cldma_rgpd *)req->gpd;
			} else {
				// low memory, this queue will not be functional
				CCCI_ERR_MSG(md->index, TAG, "alloc skb fail on q%d\n", queue->index);
				result = 3;
				break;
			}
		} else {
			// undo skb, as it remains in buffer and will be handled later
			new_req->skb->len = 0;
			skb_reset_tail_pointer(new_req->skb);
			// free the wrapper
			list_del(&new_req->entry);
			new_req->policy = NOOP;
			ccci_free_req(new_req);
			result = 1;
			break;
		}
		// check budget
		if(count++>=budget) {
			result = 2;
			break;
		}
	}
	queue->tr_done = req;
	/*
	 * do not use if(count == RING_BUFFER_SIZE) to resume Rx queue.
	 * resume Rx queue every time. we may not handle all RX ring buffer at one time due to
	 * user can refuse to receive patckets. so when a queue is stopped after it consumes all
	 * GPD, there is a chance that "count" never reaches ring buffer size and the queue is stopped 
	 * permanentely.
	 *
	 * resume after all RGPD handled also makes budget useless when it is less than ring buffer length.
	 */
	// if result == 0, that means all skb have been handled
	CCCI_DBG_MSG(md->index, TAG, "CLDMA Rxq%d collected, ret=%d\n", queue->index, result);
	return count;
}

static void cldma_rx_done(struct work_struct *work)
{
	struct md_cd_queue *queue = container_of(work, struct md_cd_queue, cldma_work);
	struct ccci_modem *md = queue->modem;
	struct md_cd_ctrl *md_ctrl = (struct md_cd_ctrl *)md->private_data;
	
	cldma_rx_collect(queue, queue->budget);
	md_cd_lock_cldma_clock_src(1);
	// resume Rx queue
	cldma_write32(md_ctrl->cldma_ap_base, CLDMA_AP_SO_RESUME_CMD, CLDMA_BM_ALL_QUEUE&(1<<queue->index));
	// enable RX_DONE interrupt
	cldma_write32(md_ctrl->cldma_ap_base, CLDMA_AP_L2RIMCR0, CLDMA_BM_ALL_QUEUE&(1<<queue->index));
	md_cd_lock_cldma_clock_src(0);
}

static void cldma_tx_done(struct work_struct *work)
{
	struct md_cd_queue *queue = container_of(work, struct md_cd_queue, cldma_work);
	struct ccci_modem *md = queue->modem;
	struct md_cd_ctrl *md_ctrl = (struct md_cd_ctrl *)md->private_data;
	unsigned long flags;
	
	struct ccci_request *req;
	struct cldma_tgpd *tgpd;
	int count = 0;
	struct sk_buff *skb_free;
	DATA_POLICY skb_free_p;
	
	while(1) {
		spin_lock_irqsave(&queue->ring_lock, flags);
		req = queue->tr_done;
		tgpd = (struct cldma_tgpd *)req->gpd;
		if(!((tgpd->gpd_flags&0x1) == 0 && req->skb)) {
			spin_unlock_irqrestore(&queue->ring_lock, flags);
			break;
		}
		// update counter
		queue->free_slot++;
		CCCI_DBG_MSG(md->index, TAG, "harvest TGPD %p for q%d\n", tgpd, queue->index);
		dma_unmap_single(NULL, tgpd->data_buff_bd_ptr, req->skb->len, DMA_TO_DEVICE);
		// free skb
		skb_free = req->skb;
		skb_free_p = req->policy;
		req->skb = NULL;
		count++;
		// step forward
		req = list_entry(req->entry.next, struct ccci_request, entry);
		tgpd = (struct cldma_tgpd *)req->gpd;
		queue->tr_done = req;
		spin_unlock_irqrestore(&queue->ring_lock, flags);
		/* 
		 * After enabled NAPI, when free skb, cosume_skb() will eventually called nf_nat_cleanup_conntrack(),
		 * which will call spin_unlock_bh() to let softirq to run. so there is a chance a Rx softirq is triggered (cldma_rx_collect)	  
		 * and if it's a TCP packet, it will send ACK -- another Tx is scheduled which will require queue->ring_lock,
		 * cause a deadlock!
		 */
		ccci_free_skb(skb_free, skb_free_p);
	}
	if(count)
		wake_up_nr(&queue->req_wq, count);
	CCCI_DBG_MSG(md->index, TAG, "%d TGPD done on q%d\n", count, queue->index);
	// enable TX_DONE interrupt
	md_cd_lock_cldma_clock_src(1);
	cldma_write32(md_ctrl->cldma_ap_base, CLDMA_AP_L2TIMCR0, CLDMA_BM_ALL_QUEUE&(1<<queue->index));
	md_cd_lock_cldma_clock_src(0);
}

static void cldma_rx_queue_init(struct md_cd_queue *queue)
{
	int i;
	struct ccci_request *req;
	struct cldma_rgpd *gpd=NULL, *prev_gpd=NULL;
	struct ccci_modem *md = queue->modem;
	struct md_cd_ctrl *md_ctrl = (struct md_cd_ctrl *)md->private_data;
	
	for(i=0; i<RING_BUFF_SIZE; i++) {
		req = ccci_alloc_req(IN, rx_queue_buffer_size[queue->index], 1, 0);
		req->gpd = dma_pool_alloc(md_ctrl->rgpd_dmapool, GFP_KERNEL, &req->gpd_addr);
		gpd = (struct cldma_rgpd *)req->gpd;
		memset(gpd, 0, sizeof(struct cldma_rgpd));
		gpd->data_buff_bd_ptr = dma_map_single(NULL, req->skb->data, skb_size(req->skb), DMA_FROM_DEVICE);
		gpd->data_allow_len = rx_queue_buffer_size[queue->index];
		gpd->gpd_flags = 0x81; // IOC|HWO
		if(i==0) {
			queue->tr_done = req;
			queue->tr_ring = &req->entry;
			INIT_LIST_HEAD(queue->tr_ring); // check ccci_request_struct_init for why we init here
		} else {
			prev_gpd->next_gpd_ptr = req->gpd_addr;
			caculate_checksum((char *)prev_gpd, 0x81);
		}
		prev_gpd = gpd;
		list_add_tail(&req->entry, queue->tr_ring);
	}
	gpd->next_gpd_ptr = queue->tr_done->gpd_addr;
	caculate_checksum((char *)gpd, 0x81);

	/*
	 * we hope work item of different CLDMA queue can work concurrently, but work items of the same
	 * CLDMA queue must be work sequentially as wo didn't implement any lock in rx_done or tx_done.
	 */
	queue->worker = alloc_workqueue("rx%d_worker", WQ_UNBOUND | WQ_MEM_RECLAIM | WQ_HIGHPRI, 1, queue->index);
	INIT_WORK(&queue->cldma_work, cldma_rx_done);
}

static void cldma_tx_queue_init(struct md_cd_queue *queue)
{
	int i;
	struct ccci_request *req;
	struct cldma_tgpd *gpd, *prev_gpd=NULL;
	struct ccci_modem *md = queue->modem;
	struct md_cd_ctrl *md_ctrl = (struct md_cd_ctrl *)md->private_data;
	
	for(i=0; i<RING_BUFF_SIZE; i++) {
		req = ccci_alloc_req(OUT, -1, 1, 0);
		req->gpd = dma_pool_alloc(md_ctrl->tgpd_dmapool, GFP_KERNEL, &req->gpd_addr);
		gpd = (struct cldma_tgpd *)req->gpd;
		memset(gpd, 0, sizeof(struct cldma_tgpd));
		// gpd->gpd_flags = 0x80; // IOC, FIXME
		if((i & 0xF) == 0xF)
			gpd->gpd_flags = 0x80;
		if(i==0) {
			queue->tr_done = req;
			queue->tx_xmit = req;
			queue->tr_ring = &req->entry;
			INIT_LIST_HEAD(queue->tr_ring);
		} else {
			prev_gpd->next_gpd_ptr = req->gpd_addr;
		}
		prev_gpd = gpd;
		list_add_tail(&req->entry, queue->tr_ring);
	}
	gpd->next_gpd_ptr = queue->tr_done->gpd_addr;

	queue->worker = alloc_workqueue("tx%d_worker", WQ_UNBOUND | WQ_MEM_RECLAIM | WQ_HIGHPRI, 1, queue->index);
	INIT_WORK(&queue->cldma_work, cldma_tx_done);
}

static irqreturn_t cldma_isr(int irq, void *data)
{
	struct ccci_modem *md = (struct ccci_modem *)data;
	struct md_cd_ctrl *md_ctrl = (struct md_cd_ctrl *)md->private_data;

	CCCI_DBG_MSG(md->index, TAG, "CLDMA IRQ!\n");
	disable_irq_nosync(CLDMA_AP_IRQ);
	tasklet_hi_schedule(&md_ctrl->cldma_irq_task);
	return IRQ_HANDLED;
}

void cldma_irq_task(unsigned long data)
{
	struct ccci_modem *md = (struct ccci_modem *)data;
	struct md_cd_ctrl *md_ctrl = (struct md_cd_ctrl *)md->private_data;
	int i;
	unsigned int L2TIMR0, L2RIMR0, L2TISAR0, L2RISAR0;
	unsigned int L3TIMR0, L3RIMR0, L3TISAR0, L3RISAR0;

	md_cd_lock_cldma_clock_src(1);
	// get L2 interrupt status
	L2TISAR0 = cldma_read32(md_ctrl->cldma_ap_base, CLDMA_AP_L2TISAR0);
	L2RISAR0 = cldma_read32(md_ctrl->cldma_ap_base, CLDMA_AP_L2RISAR0);
	L2TIMR0 = cldma_read32(md_ctrl->cldma_ap_base, CLDMA_AP_L2TIMR0);
	L2RIMR0 = cldma_read32(md_ctrl->cldma_ap_base, CLDMA_AP_L2RIMR0);
	// get L3 interrupt status
	L3TISAR0 = cldma_read32(md_ctrl->cldma_ap_base, CLDMA_AP_L3TISAR0);
	L3RISAR0 = cldma_read32(md_ctrl->cldma_ap_base, CLDMA_AP_L3RISAR0);
	L3TIMR0 = cldma_read32(md_ctrl->cldma_ap_base, CLDMA_AP_L3TIMR0);
	L3RIMR0 = cldma_read32(md_ctrl->cldma_ap_base, CLDMA_AP_L3RIMR0);
	
	CCCI_DBG_MSG(md->index, TAG, "CLDMA irq L2(%x/%x) L3(%x/%x)!\n", L2TISAR0, L2RISAR0, L3TISAR0, L3RISAR0);

	L2TISAR0 &= (~L2TIMR0);
	L2RISAR0 &= (~L2RIMR0);

	L3TISAR0 &= (~L3TIMR0);
	L3RISAR0 &= (~L3RIMR0);

	if(L2TISAR0 & CLDMA_BM_INT_ERROR) {
		CCCI_DBG_MSG(md->index, TAG, "L2TISAR0=%X, L3TISAR0=%X\n", L2TISAR0, L3TISAR0);
		// TODO:
	}
	if(L2RISAR0 & CLDMA_BM_INT_ERROR) {
		CCCI_DBG_MSG(md->index, TAG, "L2RISAR0=%X, L3RISAR0=%X\n", L2RISAR0, L3RISAR0);
		// TODO:
	}
	// ack Tx interrupt
	cldma_write32(md_ctrl->cldma_ap_base, CLDMA_AP_L2TISAR0, L2TISAR0);
	for(i=0; i<QUEUE_LEN(md_ctrl->txq); i++) {
		if(L2TISAR0 & CLDMA_BM_INT_DONE & (1<<i)) {
			// disable TX_DONE interrupt
			cldma_write32(md_ctrl->cldma_ap_base, CLDMA_AP_L2TIMSR0, CLDMA_BM_ALL_QUEUE&(1<<i));
			queue_work(md_ctrl->txq[i].worker, &md_ctrl->txq[i].cldma_work);
		}
	}
	// ack Rx interrupt
	cldma_write32(md_ctrl->cldma_ap_base, CLDMA_AP_L2RISAR0, L2RISAR0);
	// clear MD2AP_PEER_WAKEUP when get RX_DONE
#ifdef MD_PEER_WAKEUP
	if(L2RISAR0 & CLDMA_BM_INT_DONE)
		cldma_write32(md_ctrl->md_peer_wakeup, 0, cldma_read32(md_ctrl->md_peer_wakeup, 0) & ~0x01);
#endif
	for(i=0; i<QUEUE_LEN(md_ctrl->rxq); i++) {
		if(L2RISAR0 & CLDMA_BM_INT_DONE & (1<<i)) {
			// disable RX_DONE interrupt
			cldma_write32(md_ctrl->cldma_ap_base, CLDMA_AP_L2RIMSR0, CLDMA_BM_ALL_QUEUE&(1<<i));
			if(md->md_state!=EXCEPTION && ((1<<i)&NAPI_QUEUE_MASK))
				md_ctrl->rxq[i].napi_port->ops->md_state_notice(md_ctrl->rxq[i].napi_port, RX_IRQ);
			else
				queue_work(md_ctrl->rxq[i].worker, &md_ctrl->rxq[i].cldma_work);
		}
	}
	md_cd_lock_cldma_clock_src(0);
	enable_irq(CLDMA_AP_IRQ);
}

static inline void cldma_stop(struct ccci_modem *md)
{
	struct md_cd_ctrl *md_ctrl = (struct md_cd_ctrl *)md->private_data;
	// stop all Tx and Rx queues
	cldma_write32(md_ctrl->cldma_ap_base, CLDMA_AP_UL_STOP_CMD, CLDMA_BM_ALL_QUEUE);
	cldma_write32(md_ctrl->cldma_ap_base, CLDMA_AP_SO_STOP_CMD, CLDMA_BM_ALL_QUEUE);
	// clear all L2 and L3 interrupts
	cldma_write32(md_ctrl->cldma_ap_base, CLDMA_AP_L2TISAR0, CLDMA_BM_INT_ALL);
	cldma_write32(md_ctrl->cldma_ap_base, CLDMA_AP_L2TISAR1, CLDMA_BM_INT_ALL);
	cldma_write32(md_ctrl->cldma_ap_base, CLDMA_AP_L2RISAR0, CLDMA_BM_INT_ALL);
	cldma_write32(md_ctrl->cldma_ap_base, CLDMA_AP_L2RISAR1, CLDMA_BM_INT_ALL);
	cldma_write32(md_ctrl->cldma_ap_base, CLDMA_AP_L3TISAR0, CLDMA_BM_INT_ALL);
	cldma_write32(md_ctrl->cldma_ap_base, CLDMA_AP_L3TISAR1, CLDMA_BM_INT_ALL);
	cldma_write32(md_ctrl->cldma_ap_base, CLDMA_AP_L3RISAR0, CLDMA_BM_INT_ALL);
	cldma_write32(md_ctrl->cldma_ap_base, CLDMA_AP_L3RISAR1, CLDMA_BM_INT_ALL);
	// disable all L2 and L3 interrupts
	cldma_write32(md_ctrl->cldma_ap_base, CLDMA_AP_L2TIMSR0, CLDMA_BM_INT_ALL);
	cldma_write32(md_ctrl->cldma_ap_base, CLDMA_AP_L2TIMSR1, CLDMA_BM_INT_ALL);
	cldma_write32(md_ctrl->cldma_ap_base, CLDMA_AP_L2RIMSR0, CLDMA_BM_INT_ALL);
	cldma_write32(md_ctrl->cldma_ap_base, CLDMA_AP_L2RIMSR1, CLDMA_BM_INT_ALL);
	cldma_write32(md_ctrl->cldma_ap_base, CLDMA_AP_L3TIMSR0, CLDMA_BM_INT_ALL);
	cldma_write32(md_ctrl->cldma_ap_base, CLDMA_AP_L3TIMSR1, CLDMA_BM_INT_ALL);
	cldma_write32(md_ctrl->cldma_ap_base, CLDMA_AP_L3RIMSR0, CLDMA_BM_INT_ALL);
	cldma_write32(md_ctrl->cldma_ap_base, CLDMA_AP_L3RIMSR1, CLDMA_BM_INT_ALL);
}

static inline void cldma_stop_for_ee(struct ccci_modem *md)
{
	struct md_cd_ctrl *md_ctrl = (struct md_cd_ctrl *)md->private_data;
	// stop all Tx and Rx queues, but non-stop Rx ones
	cldma_write32(md_ctrl->cldma_ap_base, CLDMA_AP_UL_STOP_CMD, CLDMA_BM_ALL_QUEUE);
	cldma_write32(md_ctrl->cldma_ap_base, CLDMA_AP_SO_STOP_CMD, CLDMA_BM_ALL_QUEUE&NONSTOP_QUEUE_MASK);
	// clear all L2 and L3 interrupts, but non-stop Rx ones
	cldma_write32(md_ctrl->cldma_ap_base, CLDMA_AP_L2TISAR0, CLDMA_BM_INT_ALL);
	cldma_write32(md_ctrl->cldma_ap_base, CLDMA_AP_L2TISAR1, CLDMA_BM_INT_ALL);
	cldma_write32(md_ctrl->cldma_ap_base, CLDMA_AP_L2RISAR0, CLDMA_BM_INT_ALL&NONSTOP_QUEUE_MASK_32);
	cldma_write32(md_ctrl->cldma_ap_base, CLDMA_AP_L2RISAR1, CLDMA_BM_INT_ALL&NONSTOP_QUEUE_MASK_32);
	cldma_write32(md_ctrl->cldma_ap_base, CLDMA_AP_L3TISAR0, CLDMA_BM_INT_ALL);
	cldma_write32(md_ctrl->cldma_ap_base, CLDMA_AP_L3TISAR1, CLDMA_BM_INT_ALL);
	cldma_write32(md_ctrl->cldma_ap_base, CLDMA_AP_L3RISAR0, CLDMA_BM_INT_ALL&NONSTOP_QUEUE_MASK_32);
	cldma_write32(md_ctrl->cldma_ap_base, CLDMA_AP_L3RISAR1, CLDMA_BM_INT_ALL&NONSTOP_QUEUE_MASK_32);
	// disable all L2 and L3 interrupts, but non-stop Rx ones
	cldma_write32(md_ctrl->cldma_ap_base, CLDMA_AP_L2TIMSR0, CLDMA_BM_INT_ALL);
	cldma_write32(md_ctrl->cldma_ap_base, CLDMA_AP_L2TIMSR1, CLDMA_BM_INT_ALL);
	cldma_write32(md_ctrl->cldma_ap_base, CLDMA_AP_L2RIMSR0, CLDMA_BM_INT_ALL&NONSTOP_QUEUE_MASK_32);
	cldma_write32(md_ctrl->cldma_ap_base, CLDMA_AP_L2RIMSR1, CLDMA_BM_INT_ALL&NONSTOP_QUEUE_MASK_32);
	cldma_write32(md_ctrl->cldma_ap_base, CLDMA_AP_L3TIMSR0, CLDMA_BM_INT_ALL);
	cldma_write32(md_ctrl->cldma_ap_base, CLDMA_AP_L3TIMSR1, CLDMA_BM_INT_ALL);
	cldma_write32(md_ctrl->cldma_ap_base, CLDMA_AP_L3RIMSR0, CLDMA_BM_INT_ALL&NONSTOP_QUEUE_MASK_32);
	cldma_write32(md_ctrl->cldma_ap_base, CLDMA_AP_L3RIMSR1, CLDMA_BM_INT_ALL&NONSTOP_QUEUE_MASK_32);
}

static inline void cldma_reset(struct ccci_modem *md)
{
	struct md_cd_ctrl *md_ctrl = (struct md_cd_ctrl *)md->private_data;
	cldma_stop(md);
	// enable OUT DMA
	cldma_write32(md_ctrl->cldma_ap_base, CLDMA_AP_SO_CFG, cldma_read32(md_ctrl->cldma_ap_base, CLDMA_AP_SO_CFG)|0x01);
	// enable SPLIT_EN
	cldma_write32(md_ctrl->cldma_ap_base, CLDMA_AP_BUS_CFG, cldma_read32(md_ctrl->cldma_ap_base, CLDMA_AP_BUS_CFG)|0x02);
	// set high priority queue
	cldma_write32(md_ctrl->cldma_ap_base, CLDMA_AP_HPQR, high_priority_queue_mask);
	// TODO: traffic control value
	// set checksum
	switch (CHECKSUM_SIZE) {
	case 0:
		cldma_write32(md_ctrl->cldma_ap_base, CLDMA_AP_UL_CHECKSUM_CHANNEL_ENABLE, 0);
		cldma_write32(md_ctrl->cldma_ap_base, CLDMA_AP_SO_CHECKSUM_CHANNEL_ENABLE, 0);
		break;
	case 12:
		cldma_write32(md_ctrl->cldma_ap_base, CLDMA_AP_UL_CHECKSUM_CHANNEL_ENABLE, CLDMA_BM_ALL_QUEUE);
		cldma_write32(md_ctrl->cldma_ap_base, CLDMA_AP_SO_CHECKSUM_CHANNEL_ENABLE, CLDMA_BM_ALL_QUEUE);		
		cldma_write32(md_ctrl->cldma_ap_base, CLDMA_AP_UL_CFG, cldma_read32(md_ctrl->cldma_ap_base, CLDMA_AP_UL_CFG)&~0x10);
		cldma_write32(md_ctrl->cldma_ap_base, CLDMA_AP_SO_CFG, cldma_read32(md_ctrl->cldma_ap_base, CLDMA_AP_SO_CFG)&~0x10);
		break;
	case 16:
		cldma_write32(md_ctrl->cldma_ap_base, CLDMA_AP_UL_CHECKSUM_CHANNEL_ENABLE, CLDMA_BM_ALL_QUEUE);
		cldma_write32(md_ctrl->cldma_ap_base, CLDMA_AP_SO_CHECKSUM_CHANNEL_ENABLE, CLDMA_BM_ALL_QUEUE);	
		cldma_write32(md_ctrl->cldma_ap_base, CLDMA_AP_UL_CFG, cldma_read32(md_ctrl->cldma_ap_base, CLDMA_AP_UL_CFG)|0x10);
		cldma_write32(md_ctrl->cldma_ap_base, CLDMA_AP_SO_CFG, cldma_read32(md_ctrl->cldma_ap_base, CLDMA_AP_SO_CFG)|0x10);
		break;
	}
	// TODO: need to select CLDMA mode in CFG?
	// TODO: enable debug ID?
}

static inline void cldma_start(struct ccci_modem *md)
{
	struct md_cd_ctrl *md_ctrl = (struct md_cd_ctrl *)md->private_data;
	int i;
	// set start address
	for(i=0; i<QUEUE_LEN(md_ctrl->txq); i++) {
		cldma_write32(md_ctrl->cldma_ap_base, CLDMA_AP_TQSAR(md_ctrl->txq[i].index), md_ctrl->txq[i].tr_done->gpd_addr);
	}
	for(i=0; i<QUEUE_LEN(md_ctrl->rxq); i++) {
		cldma_write32(md_ctrl->cldma_ap_base, CLDMA_AP_RQSAR(md_ctrl->rxq[i].index), md_ctrl->rxq[i].tr_done->gpd_addr);
	}
	wmb();
	// start all Tx and Rx queues
	cldma_write32(md_ctrl->cldma_ap_base, CLDMA_AP_UL_START_CMD, CLDMA_BM_ALL_QUEUE);
	cldma_write32(md_ctrl->cldma_ap_base, CLDMA_AP_SO_START_CMD, CLDMA_BM_ALL_QUEUE);
	// enable L2 DONE and ERROR interrupts
	cldma_write32(md_ctrl->cldma_ap_base, CLDMA_AP_L2TIMCR0, CLDMA_BM_INT_DONE|CLDMA_BM_INT_ERROR);
	cldma_write32(md_ctrl->cldma_ap_base, CLDMA_AP_L2RIMCR0, CLDMA_BM_INT_DONE|CLDMA_BM_INT_ERROR);
	// enable all L3 interrupts
	cldma_write32(md_ctrl->cldma_ap_base, CLDMA_AP_L3TIMCR0, CLDMA_BM_INT_ALL);
	cldma_write32(md_ctrl->cldma_ap_base, CLDMA_AP_L3TIMCR1, CLDMA_BM_INT_ALL);
	cldma_write32(md_ctrl->cldma_ap_base, CLDMA_AP_L3RIMCR0, CLDMA_BM_INT_ALL);
	cldma_write32(md_ctrl->cldma_ap_base, CLDMA_AP_L3RIMCR1, CLDMA_BM_INT_ALL);
}

// only allowed when cldma is stopped
static void md_cd_clear_queue(struct ccci_modem *md, DIRECTION dir)
{
	struct md_cd_ctrl *md_ctrl = (struct md_cd_ctrl *)md->private_data;
	int i, j, k;
	struct ccci_request *req = NULL;
	struct cldma_tgpd *tgpd;
	unsigned long flags;
	struct sk_buff* skb_free[RING_BUFF_SIZE];
	DATA_POLICY skb_free_p[RING_BUFF_SIZE];

	if(dir == OUT) {	
		for(i=0; i<QUEUE_LEN(md_ctrl->txq); i++) {
			spin_lock_irqsave(&md_ctrl->txq[i].ring_lock, flags);
			j=0;
			req = list_entry(md_ctrl->txq[i].tr_ring, struct ccci_request, entry);
			md_ctrl->txq[i].tr_done = req;
			md_ctrl->txq[i].tx_xmit = req;
			md_ctrl->txq[i].free_slot = RING_BUFF_SIZE;
			// clear the 1st request
			tgpd = (struct cldma_tgpd *)req->gpd;
#ifndef MT6290
			cldma_write8(&tgpd->gpd_flags, 0, cldma_read8(&tgpd->gpd_flags, 0) & ~0x1);
#else
			wmb();
			tgpd->gpd_flags &= ~0x1;
#endif
			if(req->skb) {
				skb_free[j] = req->skb;
				skb_free_p[j++] = req->policy;
				req->skb = NULL;
			}
			// clear the reset
			list_for_each_entry(req, md_ctrl->txq[i].tr_ring, entry) {
				tgpd = (struct cldma_tgpd *)req->gpd;
#ifndef MT6290
				cldma_write8(&tgpd->gpd_flags, 0, cldma_read8(&tgpd->gpd_flags, 0) & ~0x1);
#else
				wmb();
				tgpd->gpd_flags &= ~0x1;
#endif
				if(req->skb) {
					skb_free[j] = req->skb;
					skb_free_p[j++] = req->policy;
					req->skb = NULL;
				}
			}
			spin_unlock_irqrestore(&md_ctrl->txq[i].ring_lock, flags);
			for(k=0; k<j; k++)
				ccci_free_skb(skb_free[k], skb_free_p[k]);
		}
	} else if(dir == IN) {
		/*
		 * Rx do NOT need clear, as skb is always there.
		 * this design should be sync with port->recv_request() and char_dev->release(),
		 * we do not need to flugh Rx because we have greedy ports which will always consume
		 * (deliver or drop) as many packets as they could.
		 */
	}
}

static irqreturn_t md_cd_wdt_isr(int irq, void *data)
{
	struct ccci_modem *md = (struct ccci_modem *)data;
	struct md_cd_ctrl *md_ctrl = (struct md_cd_ctrl *)md->private_data;
	int ret = 0;
	
	CCCI_INF_MSG(md->index, TAG, "MD WDT IRQ\n");
#if 0
	md->boot_stage = MD_BOOT_STAGE_EXCEPTION;
	return IRQ_HANDLED;
#endif
	// 1. disable MD WDT
	del_timer(&md_ctrl->bus_timeout_timer);
#ifdef ENABLE_MD_WDT_DBG
	unsigned int state;
	state = cldma_read32(md_ctrl->md_rgu_base, WDT_MD_STA);
	cldma_write32(md_ctrl->md_rgu_base, WDT_MD_MODE, WDT_MD_MODE_KEY);
	CCCI_INF_MSG(md->index, TAG, "WDT IRQ disabled for debug, state=%X\n", state);
#endif
	// 2. reset
	ret = md->ops->reset(md);
	if(ret<0) {
		CCCI_ERR_MSG(md->index, TAG, "MD reset fail after WDT %d\n", ret);
		return IRQ_HANDLED;
	}
	// 4. wakelock
	wake_lock_timeout(&md_ctrl->trm_wake_lock, 10*HZ);
	// 5. send message
	ccci_send_virtual_md_msg(md, CCCI_MONITOR_CH, CCCI_MD_MSG_RESET, 0);
	return IRQ_HANDLED;
}

void md_cd_ap2md_bus_timeout_timer_func(unsigned long data)
{
	struct ccci_modem *md = (struct ccci_modem *)data;
	struct md_cd_ctrl *md_ctrl = (struct md_cd_ctrl *)md->private_data;
	int ret = 0;
	
	CCCI_INF_MSG(md->index, TAG, "MD bus timeout but no WDT IRQ\n");
	// same as WDT ISR
	ret = md->ops->reset(md);
	if(ret<0)
		return;
	wake_lock_timeout(&md_ctrl->trm_wake_lock, 10*HZ);
	ccci_send_virtual_md_msg(md, CCCI_MONITOR_CH, CCCI_MD_MSG_RESET, 0);
}

static irqreturn_t md_cd_ap2md_bus_timeout_isr(int irq, void *data)
{
	struct ccci_modem *md = (struct ccci_modem *)data;
	struct md_cd_ctrl *md_ctrl = (struct md_cd_ctrl *)md->private_data;

	CCCI_INF_MSG(md->index, TAG, "MD bus timeout IRQ\n");
	mod_timer(&md_ctrl->bus_timeout_timer, jiffies+5*HZ);
	return IRQ_HANDLED;
}

static int md_cd_ccif_send(struct ccci_modem *md, int channel_id)
{
#ifndef MT6290
	int busy = 0;
	busy = cldma_read32(AP_CCIF0_BASE, APCCIF_BUSY);
	if(busy & (1<<channel_id))
		CCCI_ERR_MSG(md->index, TAG, "CCIF channel %d busy\n", channel_id);
	cldma_write32(AP_CCIF0_BASE, APCCIF_BUSY, 1<<channel_id);
	cldma_write32(AP_CCIF0_BASE, APCCIF_TCHNUM, channel_id);
	CCCI_INF_MSG(md->index, TAG, "CCIF start=0x%x\n", cldma_read32(AP_CCIF0_BASE, APCCIF_START));
#endif
	return 0;
}

static void md_cd_exception(struct ccci_modem *md, HIF_EX_STAGE stage)
{
	struct md_cd_ctrl *md_ctrl = (struct md_cd_ctrl *)md->private_data;
	
	CCCI_INF_MSG(md->index, TAG, "MD exception HIF %d\n", stage);
	switch(stage) {
	case HIF_EX_INIT:
		wake_lock_timeout(&md_ctrl->trm_wake_lock, 10*HZ);
		ccci_md_exception_notify(md, EX_INIT);
		// disable CLDMA
		cldma_stop_for_ee(md);
		// purge Tx queue	
		md_cd_clear_queue(md, OUT);
		// Rx dispatch does NOT depend on queue index in port structure, so it still can find right port.
		md_cd_ccif_send(md, H2D_EXCEPTION_ACK);
		break;
	case HIF_EX_INIT_DONE:
		ccci_md_exception_notify(md, EX_DHL_DL_RDY);
		break;
	case HIF_EX_CLEARQ_DONE:
		md_cd_ccif_send(md, H2D_EXCEPTION_CLEARQ_ACK);
		break;
	case HIF_EX_ALLQ_RESET:
		// start CLDMA
		cldma_reset(md);
		cldma_start(md);
		ccci_md_exception_notify(md, EX_INIT_DONE);
		break;
	default:
		break;
	};
}

static void md_cd_ccif_work(struct work_struct *work)
{
	struct md_cd_ctrl *md_ctrl = container_of(work, struct md_cd_ctrl, ccif_work);
	struct ccci_modem *md = md_ctrl->txq[0].modem;

	// seems sometime MD send D2H_EXCEPTION_INIT_DONE and D2H_EXCEPTION_CLEARQ_DONE together
	if(md_ctrl->channel_id & (1<<D2H_EXCEPTION_INIT))
		md_cd_exception(md, HIF_EX_INIT);
	if(md_ctrl->channel_id & (1<<D2H_EXCEPTION_INIT_DONE))
		md_cd_exception(md, HIF_EX_INIT_DONE);
	if(md_ctrl->channel_id & (1<<D2H_EXCEPTION_CLEARQ_DONE))
		md_cd_exception(md, HIF_EX_CLEARQ_DONE);
	if(md_ctrl->channel_id & (1<<D2H_EXCEPTION_ALLQ_RESET))
		md_cd_exception(md, HIF_EX_ALLQ_RESET);
}

static irqreturn_t md_cd_ccif_isr(int irq, void *data)
{
	struct ccci_modem *md = (struct ccci_modem *)data;
	struct md_cd_ctrl *md_ctrl = (struct md_cd_ctrl *)md->private_data;
	
#ifndef MT6290
	// must ack first, otherwise IRQ will rush in
	md_ctrl->channel_id = cldma_read32(AP_CCIF0_BASE, APCCIF_RCHNUM);
	CCCI_INF_MSG(md->index, TAG, "MD CCIF IRQ 0x%X\n", md_ctrl->channel_id);
	cldma_write32(AP_CCIF0_BASE, APCCIF_ACK, md_ctrl->channel_id);
#endif
#if 1
	//schedule_work(&md_ctrl->ccif_work); // workqueue is too slow
	md_cd_ccif_work(&md_ctrl->ccif_work);
#else
	md->boot_stage = MD_BOOT_STAGE_EXCEPTION;
#endif
	return IRQ_HANDLED;
}

static inline int cldma_sw_init(struct ccci_modem *md)
{
	struct md_cd_ctrl *md_ctrl = (struct md_cd_ctrl *)md->private_data;
	int ret;
	// do NOT touch CLDMA HW after power on MD
	// ioremap CLDMA register region
	md_ctrl->cldma_ap_base = ioremap_nocache(CLDMA_AP_BASE, CLDMA_AP_LENGTH);
	md_ctrl->cldma_md_base = ioremap_nocache(CLDMA_MD_BASE, CLDMA_MD_LENGTH);
	CCCI_INF_MSG(md->index, TAG, "CLDMA AP base = %X/%X\n", (unsigned int)md_ctrl->cldma_ap_base, CLDMA_AP_BASE);
#ifndef MT6290
	md_ctrl->md_boot_slave_Vector = ioremap_nocache(MD_BOOT_VECTOR, 0x4);
	md_ctrl->md_boot_slave_Key = ioremap_nocache(MD_BOOT_VECTOR_KEY, 0x4);
	md_ctrl->md_boot_slave_En = ioremap_nocache(MD_BOOT_VECTOR_EN, 0x4);
	md_ctrl->md_rgu_base = ioremap_nocache(MD_RGU_BASE, 0x40);
	md_ctrl->md_global_con0 = ioremap_nocache(MD_GLOBAL_CON0, 0x4);
#endif
#ifdef MD_PEER_WAKEUP
	md_ctrl->md_peer_wakeup = ioremap_nocache(MD_PEER_WAKEUP, 0x4);
#endif
	// request IRQ
	ret = request_irq(CLDMA_AP_IRQ, cldma_isr, IRQF_TRIGGER_HIGH, "CLDMA_AP", md);
	if(ret) {
		CCCI_ERR_MSG(md->index, TAG, "request CLDMA_AP IRQ(%d) error %d\n", CLDMA_AP_IRQ, ret);
		return ret;
	}
#ifndef MT6290
	ret = request_irq(MD_WDT_IRQ, md_cd_wdt_isr, IRQF_TRIGGER_FALLING, "MD_WDT", md);
	if(ret) {
		CCCI_ERR_MSG(md->index, TAG, "request MD_WDT IRQ(%d) error %d\n", MD_WDT_IRQ, ret);
		return ret;
	}
	disable_irq_nosync(MD_WDT_IRQ); // to balance the first start
	ret = request_irq(AP2MD_BUS_TIMEOUT_IRQ, md_cd_ap2md_bus_timeout_isr, IRQF_TRIGGER_FALLING, "AP2MD_BUS_TIMEOUT", md);
	if(ret) {
		CCCI_ERR_MSG(md->index, TAG, "request AP2MD_BUS_TIMEOUT IRQ(%d) error %d\n", AP2MD_BUS_TIMEOUT_IRQ, ret);
		return ret;
	}
	ret = request_irq(CCIF0_AP_IRQ, md_cd_ccif_isr, IRQF_TRIGGER_LOW, "CCIF0_AP", md);
	if(ret) {
		CCCI_ERR_MSG(md->index, TAG, "request CCIF0_AP IRQ(%d) error %d\n", CCIF0_AP_IRQ, ret);
		return ret;
	}
#endif
	return 0;
}

static int md_cd_broadcast_state(struct ccci_modem *md, MD_STATE state)
{
	int i;
	struct ccci_port *port;

	if(md->md_state == state) // must have, due to we broadcast EXCEPTION both in MD_EX and EX_INIT
		return 1;
	
	md->md_state = state;
	for(i=0;i<md->port_number;i++) {
		port = md->ports + i;
		if(port->ops->md_state_notice)
			port->ops->md_state_notice(port, state);
	}
	return 0;
}

static int md_cd_init(struct ccci_modem *md)
{
	int i;
	struct md_cd_ctrl *md_ctrl = (struct md_cd_ctrl *)md->private_data;
	struct ccci_port *port = NULL;

	CCCI_INF_MSG(md->index, TAG, "CLDMA modem is initializing\n");
	// init CLMDA, must before queue init as we set start address there
	cldma_sw_init(md);
	// init queue
	for(i=0; i<QUEUE_LEN(md_ctrl->txq); i++) {
		md_cd_queue_struct_init(&md_ctrl->txq[i], md, OUT, i, QUEUE_BUDGET);
		cldma_tx_queue_init(&md_ctrl->txq[i]);
	}
	for(i=0; i<QUEUE_LEN(md_ctrl->rxq); i++) {
		md_cd_queue_struct_init(&md_ctrl->rxq[i], md, IN, i, QUEUE_BUDGET);
		cldma_rx_queue_init(&md_ctrl->rxq[i]);
	}
	// init port
	for(i=0; i<md->port_number; i++) {
		port = md->ports + i;
		ccci_port_struct_init(port, md);
		port->ops->init(port);
		if(port->flags & PORT_F_RX_EXCLUSIVE) {
			md_ctrl->rxq[port->rxq_index].napi_port = port;
			CCCI_DBG_MSG(md->index, TAG, "queue%d add NAPI port %s\n", port->rxq_index, port->name);
		}
	}
	ccci_setup_channel_mapping(md);
	// update state
	md->md_state = GATED;
	return 0;
}

static int md_cd_start(struct ccci_modem *md)
{
	struct md_cd_ctrl *md_ctrl = (struct md_cd_ctrl *)md->private_data;
	char img_err_str[IMG_ERR_STR_LEN];
	int ret=0, retry, cldma_on=0;
	
#ifdef DUMP_GPD_LAYOUT
	cldma_dump_all_gpd(md_ctrl);
#endif
	// 0. init security, as security depends on dummy_char, which is ready very late.
	ccci_init_security(md);
	
	CCCI_INF_MSG(md->index, TAG, "CLDMA modem is starting\n");
	// 1. load modem image
	if(md->config.setting&MD_SETTING_FIRST_BOOT || md->config.setting&MD_SETTING_RELOAD) {
		ret = ccci_load_firmware(md, IMG_MD, img_err_str);
		if(ret<0) {
			CCCI_ERR_MSG(md->index, TAG, "load firmware fail, %s\n", img_err_str);
#ifndef MT6290		
			goto out;
#endif
		}
		ret = 0; // load_std_firmware returns MD image size
		md->config.setting &= ~MD_SETTING_RELOAD;
	}
	// 2. enable MPU
	ccci_set_mem_access_protection(md);
	// 3. power on modem, do NOT touch MD register before this
	ret = md_cd_power_on(md);
	if(ret) {
		CCCI_ERR_MSG(md->index, TAG, "power on MD fail %d\n", ret);
		goto out;
	}
	// 4. update mutex
	atomic_set(&md_ctrl->reset_on_going, 0);
	// 5. start timer
	mod_timer(&md->bootup_timer, jiffies+BOOT_TIMER_ON*HZ);
	// 6. let modem go
	md_cd_let_md_go(md);
#ifndef MT6290	
	enable_irq(MD_WDT_IRQ);
#endif
	// 7. start CLDMA
	retry = CLDMA_CG_POLL;
#ifndef MT6290
	while(retry-->0) {
		if(!(cldma_read32(md_ctrl->md_global_con0, 0) & (1<<MD_GLOBAL_CON0_CLDMA_BIT))) {
			CCCI_INF_MSG(md->index, TAG, "CLDMA clock is on, retry=%d\n", retry);
			cldma_on = 1;
			break;
		} else {
			CCCI_INF_MSG(md->index, TAG, "CLDMA clock is still off, retry=%d\n", retry);
			mdelay(1000);
		}
	}
	if(!cldma_on) {
		ret = -CCCI_ERR_HIF_NOT_POWER_ON;
		CCCI_ERR_MSG(md->index, TAG, "CLDMA clock is off, retry=%d\n", retry);
		goto out;
	}
#endif
	cldma_reset(md);
	cldma_start(md);
#ifdef MT6290
	md->boot_stage = MD_BOOT_STAGE_2;
	md->ops->broadcast_state(md, READY);
#endif
out:
	CCCI_INF_MSG(md->index, TAG, "CLDMA modem started %d\n", ret);
	return ret;
}

static int md_cd_stop(struct ccci_modem *md, unsigned int timeout)
{
	int ret = 0;
	CCCI_INF_MSG(md->index, TAG, "CLDMA modem is power off, timeout=%d\n", timeout);
	ret = md_cd_power_off(md, timeout);
	CCCI_INF_MSG(md->index, TAG, "CLDMA modem is power off done, %d\n", ret);
	md->ops->broadcast_state(md, GATED);
	return 0;
}

static int md_cd_reset(struct ccci_modem *md)
{
	struct md_cd_ctrl *md_ctrl = (struct md_cd_ctrl *)md->private_data;
	
	// 1. mutex check
	if(atomic_inc_and_test(&md_ctrl->reset_on_going) > 1){
		CCCI_INF_MSG(md->index, TAG, "One reset flow is on-going\n");
		return -CCCI_ERR_MD_IN_RESET;
	}
	CCCI_INF_MSG(md->index, TAG, "CLDMA modem is resetting\n");
	// 2. disable IRQ (use nosync)
#ifndef MT6290
	disable_irq_nosync(MD_WDT_IRQ);
#endif
	md->ops->broadcast_state(md, RESET); // to block char's write operation
	cldma_stop(md);
	// 3. hold reset pin
	// 4. update state
	del_timer(&md->bootup_timer);
	md_cd_clear_queue(md, OUT);
	md->boot_stage = MD_BOOT_STAGE_0;
	return 0;
}

static int md_cd_write_room(struct ccci_modem *md, unsigned char qno)
{
	struct md_cd_ctrl *md_ctrl = (struct md_cd_ctrl *)md->private_data;
	if(qno == 0xFF)
		return -CCCI_ERR_INVALID_QUEUE_INDEX;
	return md_ctrl->txq[qno].free_slot;
}

static int md_cd_send_request(struct ccci_modem *md, unsigned char qno, struct ccci_request* req)
{
	struct md_cd_ctrl *md_ctrl = (struct md_cd_ctrl *)md->private_data;
	struct md_cd_queue *queue = &md_ctrl->txq[qno];
	struct ccci_request *tx_req;
	struct cldma_tgpd *tgpd;
	int ret;
	unsigned long flags;
	
	CCCI_DBG_MSG(md->index, TAG, "get a Tx req on q%d free=%d\n", qno, queue->free_slot);
	if(qno == 0xFF)
		return -CCCI_ERR_INVALID_QUEUE_INDEX;
retry:
	spin_lock_irqsave(&queue->ring_lock, flags); // we use irqsave as network require a lock in softirq, cause a potential deadlock
	if(queue->free_slot > 0) {
		queue->free_slot--;
		// step forward
		tx_req = queue->tx_xmit;
		tgpd = tx_req->gpd;
		queue->tx_xmit = list_entry(tx_req->entry.next, struct ccci_request, entry);
		CCCI_DBG_MSG(md->index, TAG, "sending TGPD=%p\n", tgpd);
		// copy skb pointer
		tx_req->skb = req->skb;
		tx_req->policy = req->policy;
		// free old request as wrapper, do NOT reference this request after this, use tx_req instead
		req->policy = NOOP;
		ccci_free_req(req);
		// update GPD
		tgpd->data_buff_bd_ptr = dma_map_single(NULL, tx_req->skb->data, tx_req->skb->len, DMA_TO_DEVICE);
		tgpd->data_buff_len = tx_req->skb->len;
		tgpd->debug_id = queue->debug_id++;
		// checksum of GPD
		caculate_checksum((char *)tgpd, tgpd->gpd_flags | 0x1);
		// resume Tx queue
#ifndef MT6290
		cldma_write8(&tgpd->gpd_flags, 0, cldma_read8(&tgpd->gpd_flags, 0) | 0x1);
#else
		wmb();
		tgpd->gpd_flags |= 0x1;
#endif
		/*
		 * resume queue inside spinlock, otherwise there is race conditon between ports over the same queue.
		 * one port is just setting TGPD, another port may have resumed the queue.
		 */
		md_cd_lock_cldma_clock_src(1);
		cldma_write32(md_ctrl->cldma_ap_base, CLDMA_AP_UL_RESUME_CMD, CLDMA_BM_ALL_QUEUE&(1<<qno)); 
		md_cd_lock_cldma_clock_src(0);
		spin_unlock_irqrestore(&queue->ring_lock, flags); // TX_DONE will check this flag to recycle TGPD
	} else {
		spin_unlock_irqrestore(&queue->ring_lock, flags);
		CCCI_DBG_MSG(md->index, TAG, "busy on q%d\n", qno);
		if(req->blocking) {
			ret = wait_event_interruptible_exclusive(queue->req_wq, (queue->free_slot>0));
			if(ret == -ERESTARTSYS) {
				return -EINTR;
			}
			goto retry;
		} else {
			return -EBUSY;
		}
	}
	return 0;
}

static int md_cd_give_more(struct ccci_modem *md, unsigned char qno)
{
	struct md_cd_ctrl *md_ctrl = (struct md_cd_ctrl *)md->private_data;
	if(qno == 0xFF)
		return -CCCI_ERR_INVALID_QUEUE_INDEX;
	queue_work(md_ctrl->rxq[qno].worker, &md_ctrl->rxq[qno].cldma_work);
	return 0;
}

static int md_cd_napi_poll(struct ccci_modem *md, unsigned char qno, struct napi_struct *napi ,int budget)
{
	struct md_cd_ctrl *md_ctrl = (struct md_cd_ctrl *)md->private_data;
	int ret;
	
	if(qno == 0xFF)
		return -CCCI_ERR_INVALID_QUEUE_INDEX;
	
	budget = budget<md_ctrl->rxq[qno].budget?budget:md_ctrl->rxq[qno].budget;
	ret = cldma_rx_collect(&md_ctrl->rxq[qno], budget);
	md_cd_lock_cldma_clock_src(1);
	// resume Rx queue
	cldma_write32(md_ctrl->cldma_ap_base, CLDMA_AP_SO_RESUME_CMD, CLDMA_BM_ALL_QUEUE&(1<<md_ctrl->rxq[qno].index));
	if(ret == 0) {
		napi_complete(napi);
		// enable RX_DONE interrupt
		cldma_write32(md_ctrl->cldma_ap_base, CLDMA_AP_L2RIMCR0, CLDMA_BM_ALL_QUEUE&(1<<qno));
	}
	md_cd_lock_cldma_clock_src(0);
	return ret;
}

static struct ccci_port* md_cd_get_port_by_minor(struct ccci_modem *md, int minor)
{
	int i;
	struct ccci_port *port;
	
	for(i=0; i<md->port_number; i++) {
		port = md->ports + i;
		if(port->minor == minor)
			return port;
	}
	return NULL;
}

static struct ccci_port* md_cd_get_port_by_channel(struct ccci_modem *md, CCCI_CH ch)
{
	int i;
	struct ccci_port *port;
	
	for(i=0; i<md->port_number; i++) {
		port = md->ports + i;
		if(port->rx_ch == ch || port->tx_ch == ch)
			return port;
	}
	return NULL;
}

static void dump_runtime_data(struct ccci_modem *md, struct modem_runtime *runtime)
{
	char	ctmp[12];
	int		*p;

	p = (int*)ctmp;
	*p = runtime->Prefix;
	p++;
	*p = runtime->Platform_L;
	p++;
	*p = runtime->Platform_H;

	CCCI_INF_MSG(md->index, TAG, "**********************************************\n");
	CCCI_INF_MSG(md->index, TAG, "Prefix                      %c%c%c%c\n", ctmp[0], ctmp[1], ctmp[2], ctmp[3]);
	CCCI_INF_MSG(md->index, TAG, "Platform_L                  %c%c%c%c\n", ctmp[4], ctmp[5], ctmp[6], ctmp[7]);
	CCCI_INF_MSG(md->index, TAG, "Platform_H                  %c%c%c%c\n", ctmp[8], ctmp[9], ctmp[10], ctmp[11]);
	CCCI_INF_MSG(md->index, TAG, "DriverVersion               0x%x\n", runtime->DriverVersion);
	CCCI_INF_MSG(md->index, TAG, "BootChannel                 %d\n", runtime->BootChannel);
	CCCI_INF_MSG(md->index, TAG, "BootingStartID(Mode)        0x%x\n", runtime->BootingStartID);
	CCCI_INF_MSG(md->index, TAG, "BootAttributes              %d\n", runtime->BootAttributes);
	CCCI_INF_MSG(md->index, TAG, "BootReadyID                 %d\n", runtime->BootReadyID);
	
	CCCI_INF_MSG(md->index, TAG, "ExceShareMemBase            0x%x\n", runtime->ExceShareMemBase);
	CCCI_INF_MSG(md->index, TAG, "ExceShareMemSize            0x%x\n", runtime->ExceShareMemSize);

	CCCI_INF_MSG(md->index, TAG, "CheckSum                    %d\n", runtime->CheckSum);

	p = (int*)ctmp;
	*p = runtime->Postfix;
	CCCI_INF_MSG(md->index, TAG, "Postfix                     %c%c%c%c\n", ctmp[0], ctmp[1], ctmp[2], ctmp[3]);
	CCCI_INF_MSG(md->index, TAG, "**********************************************\n");
	
	p = (int*)ctmp;
	*p = runtime->misc_prefix;
	CCCI_INF_MSG(md->index, TAG, "Prefix                      %c%c%c%c\n", ctmp[0], ctmp[1], ctmp[2], ctmp[3]);
	CCCI_INF_MSG(md->index, TAG, "SupportMask                 0x%x\n", runtime->support_mask);
	CCCI_INF_MSG(md->index, TAG, "Index                       0x%x\n", runtime->index);
	CCCI_INF_MSG(md->index, TAG, "Next                        0x%x\n", runtime->next);
	CCCI_INF_MSG(md->index, TAG, "Feature2                    0x%x\n", runtime->feature_2_val[0]);
	
	p = (int*)ctmp;
	*p = runtime->misc_postfix;
	CCCI_INF_MSG(md->index, TAG, "Postfix                     %c%c%c%c\n", ctmp[0], ctmp[1], ctmp[2], ctmp[3]);

	CCCI_INF_MSG(md->index, TAG, "----------------------------------------------\n");
}

static int md_cd_send_runtime_data(struct ccci_modem *md)
{
	int packet_size = sizeof(struct modem_runtime)+sizeof(struct ccci_header);
	struct ccci_request *req = NULL;
	struct ccci_header *ccci_h;
	struct modem_runtime *runtime;
    struct file *filp = NULL;
	LOGGING_MODE mdlog_flag = MODE_IDLE;
	int ret;
    char str[16];
	unsigned int random_seed = 0;
	snprintf(str, sizeof(str), "%s", CCCI_PLATFORM);

	req = ccci_alloc_req(OUT, packet_size, 1, 1);
	if(!req) {
		return -CCCI_ERR_ALLOCATE_MEMORY_FAIL;
	}
	ccci_h = (struct ccci_header *)req->skb->data;
	runtime = (struct modem_runtime *)(req->skb->data + sizeof(struct ccci_header));

	ccci_set_ap_region_protection(md);
	// header
	ccci_h->data[0]=0x00;
	ccci_h->data[1]= packet_size;
	ccci_h->reserved = MD_INIT_CHK_ID;
	ccci_h->channel = CCCI_CONTROL_TX;
	memset(runtime, 0, sizeof(struct modem_runtime));
	// runtime data, little endian for string
    runtime->Prefix = 0x46494343; // "CCIF"
    runtime->Postfix = 0x46494343; // "CCIF"
	runtime->Platform_L = *((int*)str);
    runtime->Platform_H = *((int*)&str[4]);
    runtime->BootChannel = CCCI_CONTROL_RX;
    runtime->DriverVersion = CCCI_DRIVER_VER;
    filp = filp_open(MDLOGGER_FILE_PATH, O_RDONLY, 0777);
    if (!IS_ERR(filp)) {
        ret = kernel_read(filp, 0, (char*)&mdlog_flag, sizeof(int));	
        if (ret != sizeof(int)) 
            mdlog_flag = MODE_IDLE;
    } else {
        CCCI_ERR_MSG(md->index, TAG, "open %s fail", MDLOGGER_FILE_PATH);
        filp = NULL;
    }
    if (filp != NULL) {
        filp_close(filp, NULL);
    }
#ifndef MT6290
    if (is_meta_mode() || is_advanced_meta_mode())
        runtime->BootingStartID = ((char)mdlog_flag << 8 | META_BOOT_ID);
    else
        runtime->BootingStartID = ((char)mdlog_flag << 8 | NORMAL_BOOT_ID);
#endif
	// share memory layout
	runtime->ExceShareMemBase = md->smem_layout.ccci_exp_smem_base_phy - md->mem_layout.smem_offset_AP_to_MD;
	runtime->ExceShareMemSize = md->smem_layout.ccci_exp_smem_size;
	// misc region, little endian for string
	runtime->misc_prefix = 0x4353494D; // "MISC"
	runtime->misc_postfix = 0x4353494D; // "MISC"
	runtime->index = 0;
	runtime->next = 0;
	// random seed
	get_random_bytes(&random_seed, sizeof(int));
	runtime->feature_2_val[0] = random_seed;
	CCCI_INF_MSG(md->index, TAG, "random_seed=%x,misc_info.feature_2_val[0]=%x\n", random_seed, runtime->feature_2_val[0]);
	runtime->support_mask |= (FEATURE_SUPPORT<<(MISC_RAND_SEED*2));
	
	dump_runtime_data(md, runtime);
	skb_put(req->skb, packet_size);
	ret =  md->ops->send_request(md, 0, req); // hardcode to queue 0
	if(ret==0 && (ccci_get_md_debug_mode(md)&(DBG_FLAG_JTAG|DBG_FLAG_DEBUG))==0) {
		mod_timer(&md->bootup_timer, jiffies+BOOT_TIMER_HS1*HZ);
	}
	return ret;
}

static int md_cd_force_assert(struct ccci_modem *md)
{
	struct ccci_request *req = NULL;
	struct ccci_header *ccci_h;

	req = ccci_alloc_req(OUT, sizeof(struct ccci_header), 1, 0);
	if(req) {
		req->policy = RECYCLE;
		ccci_h = (struct ccci_header *)skb_put(req->skb, sizeof(struct ccci_header));
		ccci_h->data[0] = 0xFFFFFFFF;
		ccci_h->data[1] = 0x5A5A5A5A;
		ccci_h->channel = CCCI_FORCE_ASSERT_CH;
		ccci_h->reserved = 0xA5A5A5A5;
		return md->ops->send_request(md, 0, req);
	}
	return -CCCI_ERR_ALLOCATE_MEMORY_FAIL;
}

static struct ccci_modem_ops md_cd_ops = {
	.init = &md_cd_init,
	.start = &md_cd_start,
	.stop = &md_cd_stop,
	.reset = &md_cd_reset,
	.send_request = &md_cd_send_request,
	.give_more = &md_cd_give_more,
	.napi_poll = &md_cd_napi_poll,
	.send_runtime_data = &md_cd_send_runtime_data,
	.broadcast_state = &md_cd_broadcast_state,
	.force_assert = &md_cd_force_assert,
	.write_room = &md_cd_write_room,
	.get_port_by_minor = &md_cd_get_port_by_minor,
	.get_port_by_channel = &md_cd_get_port_by_channel,
};

static ssize_t md_cd_dump_reg_show(struct ccci_modem *md, char *buf)
{	
	struct md_cd_ctrl *md_ctrl = (struct md_cd_ctrl *)md->private_data;
	int count = 0;
	int i, length;
	static  int type = 0;
	static int side = 0;
	void __iomem * cldma_base;
	if(side==0)
		cldma_base = md_ctrl->cldma_ap_base;
	else
		cldma_base = md_ctrl->cldma_md_base;
	md_cd_lock_cldma_clock_src(1);
	switch(type) {
	case 0:
		length = (CLDMA_AP_UL_CHECKSUM_CHANNEL_ENABLE-CLDMA_AP_UL_START_ADDR_0)/sizeof(int);
		count += snprintf(buf, 128, "[CLDMA%d IN reg dump(%d):]\n", side, length);
		for(i=0; i<length; i++) {
			count += snprintf(buf+count, 128, "%X: %X\n", 
				(unsigned int)(cldma_base+CLDMA_AP_UL_START_ADDR_0+i*sizeof(int)), 
				*((int *)(cldma_base+CLDMA_AP_UL_START_ADDR_0)+i));
		}
		type++;
		break;
	case 1:
		length = (CLDMA_AP_DEBUG_ID_EN-CLDMA_AP_SO_ERROR)/sizeof(int);
		count += snprintf(buf, 128, "[CLDMA%d OUT reg dump(%d):]\n", side, length);
		for(i=0; i<length; i++) {
			count += snprintf(buf+count, 128, "%X: %X\n", 
				(unsigned int)(cldma_base+CLDMA_AP_SO_ERROR+i*sizeof(int)), 
				*((int *)(cldma_base+CLDMA_AP_SO_ERROR)+i));
		}
		type++;
		break;
	case 2:
		length = (CLDMA_AP_CHNL_IDLE-CLDMA_AP_L2TISAR0)/sizeof(int);
		count += snprintf(buf, 128, "[CLDMA%d MISC reg dump(%d):]\n", side, length);
		for(i=0; i<length; i++) {
			count += snprintf(buf+count, 128, "%X: %X\n", 
				(unsigned int)(cldma_base+CLDMA_AP_L2TISAR0+i*sizeof(int)), 
				*((int *)(cldma_base+CLDMA_AP_L2TISAR0)+i));
		}
		type = 0;
		side = side==0?1:0;
		break;
	};
	md_cd_lock_cldma_clock_src(0);
	return count;
}

static ssize_t md_cd_dump_reg_store(struct ccci_modem *md, const char *buf, size_t count)
{
	cldma_reset(md);
	md_cd_clear_queue(md, OUT);
	cldma_start(md);
	return count;
}

static ssize_t md_cd_dump_gpd_show(struct ccci_modem *md, char *buf)
{	
	struct md_cd_ctrl *md_ctrl = (struct md_cd_ctrl *)md->private_data;
	int count = 0;
	count = snprintf(buf, 128, "check kernel log for result\n");
	cldma_dump_all_gpd(md_ctrl);
	return count;
}

static ssize_t md_cd_dump_gpd_store(struct ccci_modem *md, const char *buf, size_t count)
{
	return count;
}


static ssize_t md_cd_parameter_show(struct ccci_modem *md, char *buf)
{	
	int count = 0;
	
	count += snprintf(buf, 128, "QUEUE_BUDGET=%d\n", QUEUE_BUDGET);
	count += snprintf(buf+count, 128, "RING_BUFF_SIZE=%d\n", RING_BUFF_SIZE);
	count += snprintf(buf+count, 128, "CHECKSUM_SIZE=%d\n", CHECKSUM_SIZE);
	return count;
}

static ssize_t md_cd_parameter_store(struct ccci_modem *md, const char *buf, size_t count)
{
	return count;
}

static ssize_t md_cd_ccif_show(struct ccci_modem *md, char *buf)
{	
	int count = 0;
	count = snprintf(buf, 128, "check kernel log for result\n");
#ifndef MT6290	
	CCCI_INF_MSG(md->index, TAG, "AP_CON(%x)=%d\n", AP_CCIF0_BASE+APCCIF_CON, cldma_read32(AP_CCIF0_BASE, APCCIF_CON));
	CCCI_INF_MSG(md->index, TAG, "AP_BUSY(%x)=%d\n", AP_CCIF0_BASE+APCCIF_BUSY, cldma_read32(AP_CCIF0_BASE, APCCIF_BUSY));
	CCCI_INF_MSG(md->index, TAG, "AP_START(%x)=%d\n", AP_CCIF0_BASE+APCCIF_START, cldma_read32(AP_CCIF0_BASE, APCCIF_START));
	CCCI_INF_MSG(md->index, TAG, "AP_TCHNUM(%x)=%d\n", AP_CCIF0_BASE+APCCIF_TCHNUM, cldma_read32(AP_CCIF0_BASE, APCCIF_TCHNUM));
	CCCI_INF_MSG(md->index, TAG, "AP_RCHNUM(%x)=%d\n", AP_CCIF0_BASE+APCCIF_RCHNUM, cldma_read32(AP_CCIF0_BASE, APCCIF_RCHNUM));
	CCCI_INF_MSG(md->index, TAG, "AP_ACK(%x)=%d\n", AP_CCIF0_BASE+APCCIF_ACK, cldma_read32(AP_CCIF0_BASE, APCCIF_ACK));
	#define _MD_CCIF0_BASE (AP_CCIF0_BASE+0x1000)
	CCCI_INF_MSG(md->index, TAG, "MD_CON(%x)=%d\n", _MD_CCIF0_BASE+APCCIF_CON, cldma_read32(_MD_CCIF0_BASE, APCCIF_CON));
	CCCI_INF_MSG(md->index, TAG, "MD_BUSY(%x)=%d\n", _MD_CCIF0_BASE+APCCIF_BUSY, cldma_read32(_MD_CCIF0_BASE, APCCIF_BUSY));
	CCCI_INF_MSG(md->index, TAG, "MD_START(%x)=%d\n", _MD_CCIF0_BASE+APCCIF_START, cldma_read32(_MD_CCIF0_BASE, APCCIF_START));
	CCCI_INF_MSG(md->index, TAG, "MD_TCHNUM(%x)=%d\n", _MD_CCIF0_BASE+APCCIF_TCHNUM, cldma_read32(_MD_CCIF0_BASE, APCCIF_TCHNUM));
	CCCI_INF_MSG(md->index, TAG, "MD_RCHNUM(%x)=%d\n", _MD_CCIF0_BASE+APCCIF_RCHNUM, cldma_read32(_MD_CCIF0_BASE, APCCIF_RCHNUM));
	CCCI_INF_MSG(md->index, TAG, "MD_ACK(%x)=%d\n", _MD_CCIF0_BASE+APCCIF_ACK, cldma_read32(_MD_CCIF0_BASE, APCCIF_ACK));
#endif
	return count;
}

static ssize_t md_cd_ccif_store(struct ccci_modem *md, const char *buf, size_t count)
{
	return count;
}


CCCI_MD_ATTR(NULL, dump_reg, 0660, md_cd_dump_reg_show, md_cd_dump_reg_store);
CCCI_MD_ATTR(NULL, dump_gpd, 0660, md_cd_dump_gpd_show, md_cd_dump_gpd_store);
CCCI_MD_ATTR(NULL, parameter, 0660, md_cd_parameter_show, md_cd_parameter_store);
CCCI_MD_ATTR(NULL, ccif, 0660, md_cd_ccif_show, md_cd_ccif_store);

static void md_cd_sysfs_init(struct ccci_modem *md)
{
	int ret;
	ccci_md_attr_dump_reg.modem = md;
	ret = sysfs_create_file(&md->kobj, &ccci_md_attr_dump_reg.attr);
	if(ret)
		CCCI_ERR_MSG(md->index, TAG, "fail to add sysfs node %s %d\n", 
		ccci_md_attr_dump_reg.attr.name, ret);

	ccci_md_attr_dump_gpd.modem = md;
	ret = sysfs_create_file(&md->kobj, &ccci_md_attr_dump_gpd.attr);
	if(ret)
		CCCI_ERR_MSG(md->index, TAG, "fail to add sysfs node %s %d\n", 
		ccci_md_attr_dump_gpd.attr.name, ret);

	ccci_md_attr_parameter.modem = md;
	ret = sysfs_create_file(&md->kobj, &ccci_md_attr_parameter.attr);
	if(ret)
		CCCI_ERR_MSG(md->index, TAG, "fail to add sysfs node %s %d\n", 
		ccci_md_attr_parameter.attr.name, ret);

	ccci_md_attr_ccif.modem = md;
	ret = sysfs_create_file(&md->kobj, &ccci_md_attr_ccif.attr);
	if(ret)
		CCCI_ERR_MSG(md->index, TAG, "fail to add sysfs node %s %d\n", 
		ccci_md_attr_ccif.attr.name, ret);
}

static void md_cd_modem_setup(struct ccci_modem *md)
{
	struct md_cd_ctrl *md_ctrl;
	static char wakelock_name[16];	
	// init modem structure
	md->index = MD_SYS1;
	md->major = MD_CD_MAJOR;
	md->ops = &md_cd_ops;
	md->ports = md_cd_ports_normal;
	md->port_number = ARRAY_SIZE(md_cd_ports_normal);
	md->bootup_timer.function = md_bootup_timeout_func;
	md->bootup_timer.data = (unsigned long)md;
	md->ex_monitor.function = md_ex_monitor_func;
	md->ex_monitor.data = (unsigned long)md;
	// init modem private data
	md_ctrl = (struct md_cd_ctrl *)md->private_data;
	init_waitqueue_head(&md_ctrl->sched_thread_wq);
	md_ctrl->sched_thread_kick = 0;
	snprintf(wakelock_name, sizeof(wakelock_name), "ccci%d_trm", md->index);
	wake_lock_init(&md_ctrl->trm_wake_lock, WAKE_LOCK_SUSPEND, wakelock_name);
	md_ctrl->tgpd_dmapool = dma_pool_create("CLDMA_TGPD_DMA",
						NULL, 
						sizeof(struct cldma_tgpd), 
						16, 
						0);
	md_ctrl->rgpd_dmapool = dma_pool_create("CLDMA_RGPD_DMA", 
						NULL, 
						sizeof(struct cldma_rgpd), 
						16, 
						0);
	INIT_WORK(&md_ctrl->ccif_work, md_cd_ccif_work);
	init_timer(&md_ctrl->bus_timeout_timer);
	md_ctrl->bus_timeout_timer.function = md_cd_ap2md_bus_timeout_timer_func;
	md_ctrl->bus_timeout_timer.data = (unsigned long)md;
	tasklet_init(&md_ctrl->cldma_irq_task, cldma_irq_task, (unsigned long)md);
	md_ctrl->channel_id = 0;
}

static int ccci_modem_probe(struct platform_device *dev)
{
	struct ccci_modem *md_cd;
	int i;

	CCCI_INF_MSG(-1, TAG, "modem CLDMA module init\n");
	md_cd = ccci_allocate_modem(sizeof(struct md_cd_ctrl), md_cd_modem_setup);
	// register modem
	ccci_register_modem(md_cd);
	// add sysfs entries
	md_cd_sysfs_init(md_cd);
	// hoop up to device
	dev->dev.platform_data = md_cd;
	// init CCIF
#ifndef MT6290
	cldma_write32(AP_CCIF0_BASE, APCCIF_CON, 0x01); // arbitration
	cldma_write32(AP_CCIF0_BASE, APCCIF_ACK, 0xFFFF);
	for(i=0; i<APCCIF_SRAM_SIZE/sizeof(u32); i++) {
		cldma_write32(AP_CCIF0_BASE, APCCIF_CHDATA+i*sizeof(u32), 0);
	}
#endif

	return 0;
}

static struct dev_pm_ops ccci_modem_pm_ops = {
    .suspend = ccci_modem_pm_suspend,
    .resume = ccci_modem_pm_resume,
    .freeze = ccci_modem_pm_suspend,
    .thaw = ccci_modem_pm_resume,
    .poweroff = ccci_modem_pm_suspend,
    .restore = ccci_modem_pm_resume,
    .restore_noirq = ccci_modem_pm_restore_noirq,
};

static struct platform_driver ccci_modem_driver =
{
	.driver = {
		.name = "cldma_modem",
#ifdef CONFIG_PM
		.pm = &ccci_modem_pm_ops,
#endif
	},
	.probe = ccci_modem_probe,
	.remove = ccci_modem_remove,
	.shutdown = ccci_modem_shutdown,
	.suspend = ccci_modem_suspend,
	.resume = ccci_modem_resume,
};

#ifdef MT6290
static struct platform_device ccci_cldma_device = {
	.name = "cldma_modem",
	.id = -1,
};
#endif

static int __init modem_cd_init(void)
{
	int ret;
	ret = platform_driver_register(&ccci_modem_driver);
	if (ret) {
		CCCI_ERR_MSG(-1, TAG, "clmda modem platform driver register fail(%d)\n", ret);
		return ret;
	}
#ifdef MT6290
	ret = platform_device_register(&ccci_cldma_device);
	if (ret) {
		CCCI_ERR_MSG(-1, TAG, "cldma modem platform device register fail(%d)\n", ret);
		return ret;
	}
#endif
	return 0;
}

module_init(modem_cd_init);

MODULE_AUTHOR("Xiao Wang <xiao.wang@mediatek.com>");
MODULE_DESCRIPTION("CLDMA modem driver v0.1");
MODULE_LICENSE("GPL");
