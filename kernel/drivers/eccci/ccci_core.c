/*
 * CCCI common service and routine. Consider it as a "logical" layer.
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
#include "ccci_core.h"
#ifdef CCCI_STATISTIC
#define CREATE_TRACE_POINTS
#include "ccci_events.h"
#endif

static LIST_HEAD(modem_list); // don't use array, due to MD index may not be continuous
static void *dev_class = NULL;
static LIST_HEAD(vir_msg_list); // dummy list for virtual message on char like CCCI_MONITOR_CH

extern void ccci_config_modem(struct ccci_modem *md);
extern struct ccci_request *ccci_alloc_req(DIRECTION dir, int size, char blk1, char blk2);
void ccci_free_req(struct ccci_request *req);

// common sub-system
extern int ccci_subsys_bm_init(void);
extern int ccci_subsys_sysfs_init(void);
extern int ccci_sysfs_add_modem(struct ccci_modem *md);
extern int ccci_subsys_dfo_init(void);
// per-modem sub-system
extern int ccci_subsys_char_init(struct ccci_modem *md);

ssize_t boot_md_show(char *buf)
{
	int curr = 0;
	struct ccci_modem *md;

	list_for_each_entry(md, &modem_list, entry) {
		if(md->config.setting & MD_SETTING_ENABLE)
			curr += snprintf(&buf[curr], 128, "md%d:%d\n", md->index+1, md->boot_stage);
	}
    return curr;
}

ssize_t boot_md_store(const char *buf, size_t count)
{
	unsigned int md_id;
	struct ccci_modem *md;

	md_id = buf[0] - '0';
	CCCI_INF_MSG(-1, CORE, "ccci core boot md%d\n", md_id);
	list_for_each_entry(md, &modem_list, entry) {
		if(md->index == md_id && md->md_state == GATED) {
			md->ops->start(md);
			md->config.setting &= ~MD_SETTING_FIRST_BOOT;
			return count;
		}
	}
	return count;
}

#ifdef CCCI_STATISTIC
struct ccci_statistic core_statistic_data;
struct timer_list core_statistic_timer;

void core_statistic_timer_func(unsigned long data)
{
	int i;
	unsigned int tx_trace[CCCI_REQUEST_TRACE_DEPTH];
	unsigned int rx_trace[CCCI_REQUEST_TRACE_DEPTH];
	memset(tx_trace, 0, sizeof(tx_trace));
	memset(rx_trace, 0, sizeof(rx_trace));
	
	if(core_statistic_data.tx_req_count>0) {
		for(i=0;i<CCCI_REQUEST_TRACE_DEPTH;i++) {
			// devision of long long is a lot of  trouble, check asm/div64.h for macros
			tx_trace[i]=core_statistic_data.tx_req_time_trace[i]>>10; // divided by 1024 to get uS from nS
			tx_trace[i]/=core_statistic_data.tx_req_count;
		}
		printk("[CCCI-R]tx: %u, %u, %u\n", tx_trace[0], tx_trace[1], tx_trace[2]); // hardcode
	}
	if(core_statistic_data.rx_req_count>0) {
		for(i=0;i<CCCI_REQUEST_TRACE_DEPTH;i++){
			rx_trace[i]=core_statistic_data.rx_req_time_trace[i]>>10;
			rx_trace[i]/=core_statistic_data.rx_req_count;
		}
		printk("[CCCI-R]rx: %u, %u, %u\n", rx_trace[0], rx_trace[1], rx_trace[2]); // hardcode
	}
	if(core_statistic_data.alloc_count>0)
		printk("[CCCI-B]BM: %u/%u\n", core_statistic_data.alloc_empty_count, core_statistic_data.alloc_count);
	trace_ccci_core(rx_trace, tx_trace,
		core_statistic_data.alloc_empty_count*100/core_statistic_data.alloc_count);
	memset(&core_statistic_data, 0, sizeof(core_statistic_data));
	mod_timer(&core_statistic_timer, jiffies+CCCI_STATISTIC_DUMP_INTERVAL*HZ);
}

void ccci_update_request_statistic(struct ccci_request *req)
{
	int i;
	if(req->dir==OUT) {
		core_statistic_data.tx_req_count++;
		for(i=0;i<req->time_step;i++)
			core_statistic_data.tx_req_time_trace[i]+=req->time_trace[i];
	} else {
		core_statistic_data.rx_req_count++;
		for(i=0;i<req->time_step;i++)
			core_statistic_data.rx_req_time_trace[i]+=req->time_trace[i];
	}
	//printk("[CCCI-R]%d %u %u %u\n", req->dir, req->time_trace[0], req->time_trace[1], req->time_trace[2]);
}
EXPORT_SYMBOL(ccci_update_request_statistic);
#endif

static int __init ccci_init(void)
{
	CCCI_INF_MSG(-1, CORE, "ccci core init\n");
	dev_class = class_create(THIS_MODULE, "ccci_node");
	// init common sub-system
	ccci_subsys_dfo_init();
	ccci_subsys_sysfs_init();
	ccci_subsys_bm_init();
#ifdef CCCI_STATISTIC
	init_timer(&core_statistic_timer);
	core_statistic_timer.function = core_statistic_timer_func;
	mod_timer(&core_statistic_timer, jiffies+CCCI_STATISTIC_DUMP_INTERVAL*HZ);
#endif
	return 0;
}

struct ccci_modem *ccci_allocate_modem(int private_size, void (*setup)(struct ccci_modem *md))
{
	struct ccci_modem* md = kzalloc(sizeof(struct ccci_modem), GFP_KERNEL);
	int i;
	md->private_data = kmalloc(private_size, GFP_KERNEL);
	md->sim_type = 0xEEEEEEEE; //sim_type(MCC/MNC) sent by MD wouldn't be 0xEEEEEEEE
	md->config.setting |= MD_SETTING_FIRST_BOOT;
	md->md_state = INVALID;
	md->boot_stage = MD_BOOT_STAGE_0;
	md->ex_stage = EX_NONE;
	atomic_set(&md->wakeup_src, 0);
	INIT_LIST_HEAD(&md->entry);
	init_timer(&md->bootup_timer);	
	init_timer(&md->ex_monitor);
	spin_lock_init(&md->ctrl_lock);
	for(i=0; i<ARRAY_SIZE(md->ch_ports); i++) {
		INIT_LIST_HEAD(&md->ch_ports[i]);
	}
	setup(md);
	return md;
}
EXPORT_SYMBOL(ccci_allocate_modem);

int ccci_register_modem(struct ccci_modem *modem)
{
	int ret;
	
	CCCI_INF_MSG(-1, CORE, "register modem %d\n", modem->major);
	// init modem
	// TODO: check modem->ops for all must-have functions
	ret = modem->ops->init(modem);
	if(ret<0)
		return ret;
	ccci_config_modem(modem);
	list_add_tail(&modem->entry, &modem_list);
	// init per-modem sub-system
	ccci_subsys_char_init(modem);
	ccci_sysfs_add_modem(modem);
	return 0;
}
EXPORT_SYMBOL(ccci_register_modem);

int ccci_get_modem_state(int md_id)
{
	struct ccci_modem *md = NULL;
	list_for_each_entry(md, &modem_list, entry) {
		if(md->index == md_id)
			return md->md_state;
	}
	return -CCCI_ERR_MD_INDEX_NOT_FOUND;
}

int exec_ccci_kern_func_by_md_id(int md_id, unsigned int id, char *buf, unsigned int len)
{
	struct ccci_modem *md = NULL;
	int ret = 0;
	
	list_for_each_entry(md, &modem_list, entry) {
		if(md->index == md_id)
			break;
	}
	if(!md)
		return -CCCI_ERR_MD_INDEX_NOT_FOUND;

	switch(id) {
	case ID_GET_MD_WAKEUP_SRC:		
		atomic_set(&md->wakeup_src, 1);
		break;
	case ID_GET_TXPOWER:
		if(buf[0] == 0) {
			ret = ccci_send_msg_to_md(md, CCCI_SYSTEM_TX, MD_TX_POWER, 0);
		} else if(buf[0] == 1){
			ret = ccci_send_msg_to_md(md, CCCI_SYSTEM_TX, MD_RF_TEMPERATURE, 0);
		} else if(buf[0] == 2){
			ret = ccci_send_msg_to_md(md, CCCI_SYSTEM_TX, MD_RF_TEMPERATURE_3G, 0);
		}
		CCCI_DBG_MSG(md->index, CORE, "get_txpower(%d): %d\n", buf[0], ret);
		break;
	default:
		ret = -CCCI_ERR_FUNC_ID_ERROR;
		break;
	};
	return ret;
}

struct ccci_port *ccci_get_port_for_node(int major, int minor)
{
	struct ccci_modem *md = NULL;
	struct ccci_port *port;
	
	list_for_each_entry(md, &modem_list, entry) {
		if(md->major == major)
			break;
	}
	port = md->ops->get_port_by_minor(md, minor);
	return port;
}

int ccci_register_dev_node(const char *name, int major_id, int minor)
{
	int ret = 0;
	dev_t dev_n;
	struct device *dev;

	dev_n = MKDEV(major_id, minor);
	dev = device_create(dev_class, NULL, dev_n, NULL, "%s", name);

	if(IS_ERR(dev)) {
		ret = PTR_ERR(dev);
	}
	
	return ret;
}

/*
 * kernel inject CCCI message to modem.
 */
int ccci_send_msg_to_md(struct ccci_modem *md, CCCI_CH ch, CCCI_MD_MSG msg, u32 resv)
{
	struct ccci_port *port = NULL;
	struct ccci_request *req = NULL;
	struct ccci_header *ccci_h;
	int ret;

	port = md->ops->get_port_by_channel(md, ch);
	if(port) {
		if(in_irq())
			req = ccci_alloc_req(OUT, sizeof(struct ccci_header), 0, 0); // users like thermal may send message in ISR, so we don't use blocking Tx
		else
			req = ccci_alloc_req(OUT, sizeof(struct ccci_header), 1, 1);
		if(req) {
			req->policy = RECYCLE;
			ccci_h = (struct ccci_header *)skb_put(req->skb, sizeof(struct ccci_header));
			ccci_h->data[0] = CCCI_MAGIC_NUM;
			ccci_h->data[1] = msg;
			ccci_h->channel = ch;
			ccci_h->reserved = resv;
			ret = ccci_port_send_request(port, req);
			if(ret)
				ccci_free_req(req);
			return ret;
		} else {
			return -CCCI_ERR_ALLOCATE_MEMORY_FAIL;
		}
	}
	return -CCCI_ERR_INVALID_LOGIC_CHANNEL_ID;
}

/*
 * kernel inject message to user space daemon
 */
int ccci_send_virtual_md_msg(struct ccci_modem *md, CCCI_CH ch, CCCI_MD_MSG msg, u32 resv)
{
	struct ccci_request *req = NULL;
	struct ccci_header *ccci_h;

	if(ch != CCCI_MONITOR_CH)
		return -CCCI_ERR_INVALID_LOGIC_CHANNEL_ID;

	req = ccci_alloc_req(IN, sizeof(struct ccci_header), 1, 0);
	// request will be recycled in char device's read function
	ccci_h = (struct ccci_header *)skb_put(req->skb, sizeof(struct ccci_header));
	ccci_h->data[0] = CCCI_MAGIC_NUM;
	ccci_h->data[1] = msg;
	ccci_h->channel = ch;
	ccci_h->reserved = resv;
	/*
	 * normal request is passed from queue's list, so for virtual message, we also provided
	 * a list to let port layer to operate on.
	 */
	list_add_tail(&req->entry, &vir_msg_list);
	return ccci_port_recv_request(md, req);
}

subsys_initcall(ccci_init);

MODULE_AUTHOR("Xiao Wang <xiao.wang@mediatek.com>");
MODULE_DESCRIPTION("Unified CCCI driver v0.1");
MODULE_LICENSE("GPL");
