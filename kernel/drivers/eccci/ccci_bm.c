#include <linux/skbuff.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/module.h>
#include "ccci_bm.h"

struct ccci_request req_pool[BM_POOL_SIZE];
int req_pool_cnt = BM_POOL_SIZE;
spinlock_t req_pool_lock;
wait_queue_head_t req_pool_wq;

struct sk_buff_head skb_pool_4K;
struct sk_buff_head skb_pool_1_5K;
struct sk_buff_head skb_pool_16;

struct workqueue_struct *pool_reload_work_queue;
struct work_struct _4K_reload_work;
struct work_struct _1_5K_reload_work;
struct work_struct _16_reload_work;

// may return NULL, caller should check
struct sk_buff *ccci_alloc_skb(int size)
{
	struct sk_buff *skb = NULL;
	int count = 0;

retry:
	if(size>SKB_4K) {
		return NULL;
	} else if(size>SKB_1_5K) {
		skb = skb_dequeue(&skb_pool_4K);
		if(skb_pool_4K.qlen < SKB_POOL_SIZE_4K/RELOAD_TH)
			queue_work(pool_reload_work_queue, &_4K_reload_work);
	} else if(size>SKB_16) {
		skb = skb_dequeue(&skb_pool_1_5K);
		if(skb_pool_1_5K.qlen < SKB_POOL_SIZE_1_5K/RELOAD_TH)
			queue_work(pool_reload_work_queue, &_1_5K_reload_work);
	} else if(size>0) {
		skb = skb_dequeue(&skb_pool_16);
		if(skb_pool_16.qlen < SKB_POOL_SIZE_16/RELOAD_TH)
			queue_work(pool_reload_work_queue, &_16_reload_work);
	} else {
		return NULL;
	}
	if(unlikely(!skb)) {
		CCCI_INF_MSG(-1, BM, "skb pool is empty! size=%d (%d)\n", size, count);
		if(!in_interrupt()) {
			if(count++<20) {
				msleep(300);
				goto retry;
			}
		} else {
			skb = dev_alloc_skb(size);
			if(!skb && count++<20)
				goto retry;
		}
	}
	if(unlikely(!skb))
		CCCI_ERR_MSG(-1, BM, "%ps alloc skb fail, size=%d\n", __builtin_return_address(0), size);
	else
		CCCI_DBG_MSG(-1, BM, "%ps alloc skb %p, size=%d\n", __builtin_return_address(0), skb, size);
	return skb;
}
EXPORT_SYMBOL(ccci_alloc_skb);

void ccci_free_skb(struct sk_buff *skb, DATA_POLICY policy)
{
	CCCI_DBG_MSG(-1, BM, "free skb %p, policy=%d\n", skb, policy);
	switch(policy) {
		case RECYCLE:	
			// 1. reset sk_buff (take __alloc_skb as ref.)
			skb->data = skb->head;
			skb->len = 0;
			skb_reset_tail_pointer(skb);
			// 2. enqueue
			if(skb_size(skb)<SKB_1_5K)
				skb_queue_tail(&skb_pool_16, skb);
			else if (skb_size(skb)<SKB_4K)
				skb_queue_tail(&skb_pool_1_5K, skb);
			else
				skb_queue_tail(&skb_pool_4K, skb);
			// TODO: add a shore queue for skb dump
			break;
		case FREE:
			dev_kfree_skb(skb);
			break;
		case NOOP:
		default:
			break;
	};
}
EXPORT_SYMBOL(ccci_free_skb);

static void __4K_reload_work(struct work_struct *work)
{
	struct sk_buff *skb;

	CCCI_DBG_MSG(-1, BM, "refill 4KB skb pool\n");
	while(skb_pool_4K.qlen<SKB_POOL_SIZE_4K) {
		skb = dev_alloc_skb(SKB_4K);
		if(skb)
			skb_queue_tail(&skb_pool_4K, skb);
	}
}

static void __1_5K_reload_work(struct work_struct *work)
{
	struct sk_buff *skb;
	
	CCCI_DBG_MSG(-1, BM, "refill 1.5KB skb pool\n");
	while(skb_pool_1_5K.qlen<SKB_POOL_SIZE_1_5K) {
		skb = dev_alloc_skb(SKB_1_5K);
		if(skb)
			skb_queue_tail(&skb_pool_1_5K, skb);
	}
}

static void __16_reload_work(struct work_struct *work)
{
	struct sk_buff *skb;
	
	CCCI_DBG_MSG(-1, BM, "refill 16B skb pool\n");
	while(skb_pool_16.qlen<SKB_POOL_SIZE_16) {
		skb = dev_alloc_skb(SKB_16);
		if(skb)
			skb_queue_tail(&skb_pool_16, skb);
	}
}

#ifdef CCCI_STATISTIC
extern struct ccci_statistic core_statistic_data;
#endif

/*
 * a write operation may block at 3 stages:
 * 1. ccci_alloc_req
 * 2. wait until the queue has available slot (threshold check)
 * 3. wait until the SDIO transfer is complete --> abandoned, see the reason below.
 * the 1st one is decided by @blk1. and the 2nd and 3rd are decided by @blk2, wating on @wq.
 * NULL is returned if no available skb, even when you set blk1=1.
 *
 * we removed the wait_queue_head_t in ccci_request, so user can NOT wait for certain request to
 * be completed. this is because request will be recycled and its state will be reset, so if a request
 * is completed and then used again, the poor guy who is waiting for it may never see the state 
 * transition (FLYING->IDLE/COMPLETE->FLYING) and wait forever.
 */
struct ccci_request *ccci_alloc_req(DIRECTION dir, int size, char blk1, char blk2)
{
	int i;
	struct ccci_request *req = NULL;
	unsigned long flags;

#ifdef CCCI_STATISTIC
	core_statistic_data.alloc_count++;
#endif

retry:
	spin_lock_irqsave(&req_pool_lock, flags);
	for(i=0; i<BM_POOL_SIZE; i++) {
		if(req_pool[i].state == IDLE) {
			// important checking when reqeust is passed cross-layer, make sure this request is no longer in any list
			if(req_pool[i].entry.next == LIST_POISON1 && req_pool[i].entry.prev == LIST_POISON2) {
				req = &req_pool[i];
				CCCI_DBG_MSG(-1, BM, "%ps alloc req=%p, i=%d size=%d\n", __builtin_return_address(0), req, i, size);
				req->state = FLYING;
				break;
			} else {
				// should not happen
				CCCI_ERR_MSG(-1, BM, "idle but in list i=%d\n", i);
			}
		}
	}
	if(req) {
		req->dir = dir;
#ifdef CCCI_STATISTIC
		req->time_step = 0;
		req->time_stamp = ktime_get_real();
		memset(req->time_trace, 0, sizeof(req->time_trace));
#endif
		req_pool_cnt--;
		CCCI_DBG_MSG(-1, BM, "pool count-=%d\n", req_pool_cnt);
	}
	spin_unlock_irqrestore(&req_pool_lock, flags);
	if(req) {
		if(size>0) {
			req->skb = ccci_alloc_skb(size);
			if(req->skb)
				CCCI_DBG_MSG(-1, BM, "req=%p skb=%p, len=%d\n", req, req->skb, skb_size(req->skb));
		} else {
			req->skb = NULL;
		}
		req->blocking = blk2;
	} else {
#ifdef CCCI_STATISTIC
		core_statistic_data.alloc_empty_count++;
#endif
		if(blk1) {
			wait_event_interruptible(req_pool_wq, (req_pool_cnt>0));
			goto retry;
		}
		CCCI_INF_MSG(-1, BM, "fail to allock req for %ps, no retry\n", __builtin_return_address(0));
	}
	if(unlikely(size>0 && !req->skb)) {
		CCCI_ERR_MSG(-1, BM, "fail to allock skb for %ps, size=%d\n", __builtin_return_address(0), size);
		ccci_free_req(req);
		req = NULL;
	}
	return req;
}
EXPORT_SYMBOL(ccci_alloc_req);

void ccci_free_req(struct ccci_request *req)
{
	unsigned long flags;

	CCCI_DBG_MSG(-1, BM, "%ps free req=%p, policy=%d, skb=%p, len=%d\n", __builtin_return_address(0),
		req, req->policy, req->skb, skb_size(req->skb));
	ccci_free_skb(req->skb, req->policy);
	spin_lock_irqsave(&req_pool_lock, flags);
	// 1. reset the request
	req->state = IDLE;
	req->skb = NULL;
	/*
	 * do NOT reset req->entry here, always maitain it by list API (list_del).
	 * for Tx requests, they are never in any queue, so no extra effort when delete them.
	 * but for Rx request, we must make sure list_del is called once before we free them.
	 */
	if(req->entry.next != LIST_POISON1 || req->entry.prev != LIST_POISON2)
		CCCI_ERR_MSG(-1, BM, "req %p entry not deleted yet\n", req);
	// 2. wake up pending allocation
	req_pool_cnt++;
	CCCI_DBG_MSG(-1, BM, "pool count+=%d\n", req_pool_cnt);
	spin_unlock_irqrestore(&req_pool_lock, flags);
	wake_up_all(&req_pool_wq);

}
EXPORT_SYMBOL(ccci_free_req);

void ccci_mem_dump(void *start_addr, int len)
{
	unsigned int *curr_p = (unsigned int *)start_addr;
	unsigned char *curr_ch_p;
	int _16_fix_num = len/16;
	int tail_num = len%16;
	char buf[16];
	int i,j;

	if(NULL == curr_p) {
		printk("[CCCI-DUMP]NULL point to dump!\n");
		return;
	}
	if(0 == len){
		printk("[CCCI-DUMP]Not need to dump\n");
		return;
	}

	printk("[CCCI-DUMP]Base: %08x\n", (unsigned int)start_addr);
	// Fix section
	for(i=0; i<_16_fix_num; i++){
		printk("[CCCI-DUMP]%03X: %08X %08X %08X %08X\n", 
				i*16, *curr_p, *(curr_p+1), *(curr_p+2), *(curr_p+3) );
		curr_p+=4;
	}

	// Tail section
	if(tail_num > 0){
		curr_ch_p = (unsigned char*)curr_p;
		for(j=0; j<tail_num; j++){
			buf[j] = *curr_ch_p;
			curr_ch_p++;
		}
		for(; j<16; j++)
			buf[j] = 0;
		curr_p = (unsigned int*)buf;
		printk("[CCCI-DUMP]%03X: %08X %08X %08X %08X\n", 
				i*16, *curr_p, *(curr_p+1), *(curr_p+2), *(curr_p+3) );
	}
}
EXPORT_SYMBOL(ccci_mem_dump);

void ccci_dump_req(struct ccci_request *req)
{
	/*
	int i, len;
	if(req && req->skb) {
		len = req->skb->len;
		printk("[ccci dump(%d)]", len);
		for(i=0; i<len && len<32; i++) {
			printk("%02X ", *(req->skb->data+i));
		}
		printk("\n");
	}
	*/
	ccci_mem_dump(req->skb->data, req->skb->len>32?32:req->skb->len);
}
EXPORT_SYMBOL(ccci_dump_req);

int ccci_subsys_bm_init(void)
{
	int i;
	struct sk_buff *skb = NULL;

	CCCI_DBG_MSG(-1, BM, "req_pool=%p\n", req_pool);
	// init ccci_request
	for(i=0; i<BM_POOL_SIZE; i++) {
		ccci_request_struct_init(&req_pool[i]);
	}
	// init skb pool
	skb_queue_head_init(&skb_pool_4K);
	for(i=0; i<SKB_POOL_SIZE_4K; i++) {
		skb = dev_alloc_skb(SKB_4K);
		skb_queue_tail(&skb_pool_4K, skb);
	}
	skb_queue_head_init(&skb_pool_1_5K);
	skb = NULL;
	for(i=0; i<SKB_POOL_SIZE_1_5K; i++) {
		skb = dev_alloc_skb(SKB_1_5K);
		skb_queue_tail(&skb_pool_1_5K, skb);
	}
	skb_queue_head_init(&skb_pool_16);
	skb = NULL;
	for(i=0; i<SKB_POOL_SIZE_16; i++) {
		skb = dev_alloc_skb(SKB_16);
		skb_queue_tail(&skb_pool_16, skb);
	}
	// init pool reload work
	pool_reload_work_queue = create_singlethread_workqueue("pool_reload_work");
	INIT_WORK(&_4K_reload_work, __4K_reload_work);
	INIT_WORK(&_1_5K_reload_work, __1_5K_reload_work);
	INIT_WORK(&_16_reload_work, __16_reload_work);
	
	spin_lock_init(&req_pool_lock);
	init_waitqueue_head(&req_pool_wq);
	return 0;
}

