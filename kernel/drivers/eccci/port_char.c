#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/wait.h>
#include <linux/module.h>
#include <linux/poll.h>
#ifdef FEATURE_GET_MD_BAT_VOL
#include <mach/battery_common.h>
#else
#define BAT_Get_Battery_Voltage(polling_mode)	({ 0; })
#endif
#include "ccci_core.h"
#include "ccci_ioctl.h"
#include "ccci_bm.h"
#include "ccci_dfo.h"
#include "port_ipc.h"

unsigned int md_img_exist[MAX_IMG_NUM]; // for META

#ifdef CUSTOM_KERNEL_SSW
extern int switch_sim_mode(int id, char *buf, unsigned int len);
extern unsigned int get_sim_switch_type(void);
#else
int switch_sim_mode(int id, char *buf, unsigned int len){return 0;}
unsigned int get_sim_switch_type(void){return 0;}
#endif

#define MAX_QUEUE_LENGTH 32

static int dev_char_open(struct inode *inode, struct file *file)
{
	int major = imajor(inode);
	int minor = iminor(inode);
	struct ccci_port *port;
	
	port = ccci_get_port_for_node(major, minor);
	if(atomic_read(&port->usage_cnt))
		return -EBUSY;
	CCCI_INF_MSG(port->modem->index, CHAR, "port %s open with flag %X by %s\n", port->name, file->f_flags, current->comm);
	atomic_inc(&port->usage_cnt);
	file->private_data = port;
	nonseekable_open(inode,file);
	return 0;
}

static int dev_char_close_check(struct ccci_port *port)
{
	if(port->rx_ch==CCCI_FS_RX && atomic_read(&port->usage_cnt))
		return 1;
	if(port->rx_ch==CCCI_UART2_RX && atomic_read(&port->usage_cnt))
		return 2;
	if(port->rx_ch==CCCI_MD_LOG_RX && atomic_read(&port->usage_cnt))
		return 3;
	return 0;
}

static int dev_char_close(struct inode *inode, struct file *file)
{
	struct ccci_port *port = file->private_data;
	struct ccci_request *req = NULL;
	struct ccci_request *reqn;
	unsigned long flags;

	// 0. decrease usage count, so when we ask more, the packet can be dropped in recv_request
	atomic_dec(&port->usage_cnt);
	// 1. purge Rx request list
	spin_lock_irqsave(&port->rx_req_lock, flags);
	list_for_each_entry_safe(req, reqn, &port->rx_req_list, entry) {
		// 1.1. remove from list
		list_del(&req->entry);
		port->rx_length--;
		// 1.2. free it
		req->policy = RECYCLE;
		ccci_free_req(req);
	}
	// 1.3 flush Rx
	ccci_port_ask_more_request(port);
	spin_unlock_irqrestore(&port->rx_req_lock, flags);
	CCCI_INF_MSG(port->modem->index, CHAR, "close port %s rx_len=%d empty=%d\n", port->name, 
		port->rx_length, list_empty(&port->rx_req_list));
	// 2. check critical nodes for reset
	if(port->modem->md_state==GATED && dev_char_close_check(port)==0)
		ccci_send_virtual_md_msg(port->modem, CCCI_MONITOR_CH, CCCI_MD_MSG_READY_TO_RESET, 0);
	return 0;
}

static ssize_t dev_char_read(struct file *file, char *buf, size_t count, loff_t *ppos)
{
	struct ccci_port *port = file->private_data;
	struct ccci_request *req;
	struct ccci_header *ccci_h;
	int ret, read_len, full_req_done=0;
	unsigned long flags;

	// 1. get incomming request
	if(list_empty(&port->rx_req_list)) {
		if(!(file->f_flags & O_NONBLOCK)) {
			ret = wait_event_interruptible(port->rx_wq, !list_empty(&port->rx_req_list));	
			if(ret == -ERESTARTSYS) {
				ret = -EINTR;
				goto exit;
			}
		} else {
			ret = -EAGAIN;
			goto exit;
		}
	}
	CCCI_DBG_MSG(port->modem->index, CHAR, "read on CH%u for %d\n", port->rx_ch, count);
	spin_lock_irqsave(&port->rx_req_lock, flags);
	req = list_first_entry(&port->rx_req_list, struct ccci_request, entry);
	// 2. caculate available data
	if(req->state != PARTIAL_READ) {
		ccci_h = (struct ccci_header *)req->skb->data;
		if(port->flags & PORT_F_USER_HEADER) { // header provide by user
			// CCCI_MON_CH should fall in here, as header must be send to md_init
			if(ccci_h->data[0] == CCCI_MAGIC_NUM) {
				read_len = sizeof(struct ccci_header);
				if(ccci_h->channel == CCCI_MONITOR_CH)
					ccci_h->channel = CCCI_MONITOR_CH_ID;
			} else {
				read_len = req->skb->len;
			}
		} else {
			// ATTENTION, if user does not provide header, it should NOT send empty packet.
			read_len = req->skb->len - sizeof(struct ccci_header);
			// remove CCCI header
			skb_pull(req->skb, sizeof(struct ccci_header));
		}
	} else {
		read_len = req->skb->len;
	}
	if(count>=read_len) {
		full_req_done = 1;
		list_del(&req->entry);
		/*
		 * here we only ask for more request when rx list is empty. no need to be too gready, because
		 * for most of the case, queue will not stop sending request to port.
		 * actually we just need to ask by ourselves when we rejected requests before. these
		 * rejected requests will staty in queue's buffer and may never get a chance to be handled again.
		 */
		if(--(port->rx_length) == 0)
			ccci_port_ask_more_request(port);
		BUG_ON(port->rx_length<0);
	} else {
		req->state = PARTIAL_READ;
		read_len = count;
	}
	spin_unlock_irqrestore(&port->rx_req_lock, flags);
	// 3. copy to user
	ret = copy_to_user(buf, req->skb->data, read_len);
	skb_pull(req->skb, read_len);
	CCCI_DBG_MSG(port->modem->index, CHAR, "copy done l=%d r=%d pr=%d\n", read_len, ret, (req->state==PARTIAL_READ));
	// 4. free request
	if(full_req_done) {
		req->policy = RECYCLE; // Rx flow doesn't know the free policy until it reaches port (network and char are different)
#ifdef CCCI_STATISTIC
		ccci_update_request_stamp(req);
		ccci_update_request_statistic(req);
#endif
#if 0
		if(port->rx_ch==CCCI_IPC_RX)
			port_ipc_rx_ack(port);
#endif
		ccci_free_req(req);
	}
exit:
	return ret?ret:read_len;
}

static ssize_t dev_char_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
	struct ccci_port *port = file->private_data;
	unsigned char blocking = !(file->f_flags&O_NONBLOCK);
	struct ccci_request *req = NULL;
	struct ccci_header *ccci_h = NULL;
	size_t actual_count = 0;
	int ret;

	if(port->tx_ch == CCCI_MONITOR_CH)
		return -EPERM;

	if(port->modem->md_state==BOOTING && port->tx_ch!=CCCI_FS_TX)
		return -ENODEV;
	if(port->modem->md_state==EXCEPTION && port->tx_ch!=CCCI_MD_LOG_TX && port->tx_ch!=CCCI_UART1_TX)
		return -ETXTBSY;
	if(port->modem->md_state==GATED || port->modem->md_state==RESET)
		return -ENODEV;
		
	if(port->flags & PORT_F_USER_HEADER) {
		if(count>CCCI_MTU+sizeof(struct ccci_header)) {
			CCCI_ERR_MSG(port->modem->index, CHAR, "reject packet(size=%d ), lager than MTU on CH%u\n", count, port->tx_ch);
			return -ENOMEM;
		}
	}

	if(port->flags & PORT_F_USER_HEADER)
		actual_count = count>CCCI_MTU+sizeof(struct ccci_header)?CCCI_MTU+sizeof(struct ccci_header):count;
	else
		actual_count = count>CCCI_MTU?CCCI_MTU:count;
	CCCI_DBG_MSG(port->modem->index, CHAR, "write %dof%d on CH%u\n", actual_count, count, port->tx_ch);
	
	req = ccci_alloc_req(OUT, actual_count, blocking, blocking);
	if(req) {
		// 1. for Tx packet, who issued it should know whether recycle it  or not
		req->policy = RECYCLE;
		// 2. prepare CCCI header, every member of header should be re-write as request may be re-used
		if(!(port->flags & PORT_F_USER_HEADER)) { 
			ccci_h = (struct ccci_header *)skb_put(req->skb, sizeof(struct ccci_header));
			ccci_h->data[0] = 0;
			ccci_h->data[1] = actual_count + sizeof(struct ccci_header);
			ccci_h->channel = port->tx_ch;
			ccci_h->reserved = 0;
		} else {
			ccci_h = (struct ccci_header *)req->skb->data;
		}
		// 3. get user data
		ret = copy_from_user(skb_put(req->skb, actual_count), buf, actual_count);
		if(ret)
			goto err_out;
		if(port->flags & PORT_F_USER_HEADER) { // header provided by user, valid after copy_from_user
			if(actual_count == sizeof(struct ccci_header))
				ccci_h->data[0] = CCCI_MAGIC_NUM;
			else
				ccci_h->data[1] = actual_count;
			ccci_h->channel = port->tx_ch; // as EEMCS VA will not fill this filed
			CCCI_DBG_MSG(port->modem->index, CHAR, "Tx header %x, %x, %x, %x\n", 
				ccci_h->data[0], ccci_h->data[1], ccci_h->channel, ccci_h->reserved);
		}
		if(port->rx_ch == CCCI_IPC_RX) {
			if((ret=port_ipc_write_check_id(port, req)) < 0)
				goto err_out;
			else
				ccci_h->reserved = ret; // Unity ID
		}
		// 4. send out
		ret = ccci_port_send_request(port, req); // do NOT reference request after called this, modem may have freed it, unless you get -EBUSY
		CCCI_DBG_MSG(port->modem->index, CHAR, "write done l=%d r=%d\n", actual_count, ret);
		if(ret) {
			if(ret == -EBUSY && !req->blocking) {
				ret = -EAGAIN;
			}
			goto err_out;
		} else {
#if 0
			if(port->rx_ch==CCCI_IPC_RX)
				port_ipc_tx_wait(port);
#endif
		}
		return ret<0?ret:actual_count;
		
err_out:
		ccci_free_req(req);
		return ret;
	} else {
		// consider this case as non-blocking
		return -EBUSY;
	}
}

static long dev_char_ioctl( struct file *file, unsigned int cmd, unsigned long arg)
{
	int state, ret = 0;
	struct ccci_port *port = file->private_data;
	struct ccci_modem *md = port->modem;
	int ch = port->rx_ch; // use Rx channel number to distinguish a port
	unsigned int sim_mode, sim_switch_type, enable_sim_type, sim_id;
	unsigned int traffic_control = 0;
#ifdef MT6290
	CCCI_INF_MSG(md->index, CHAR, "command %X for 6290 IT\n", cmd);
#define CCCI_IOC_SET_HEADER             _IO(CCCI_IOC_MAGIC,  21)
#define CCCI_IOC_CLR_HEADER             _IO(CCCI_IOC_MAGIC,  22)
	switch (cmd) {
	case CCCI_IOC_SET_HEADER:
		port->flags |= PORT_F_USER_HEADER;
		break;
	case CCCI_IOC_CLR_HEADER:
		port->flags &= ~PORT_F_USER_HEADER;
		break;
	}
	return ret;
#endif
	switch (cmd) {
	case CCCI_IOC_GET_MD_STATE:
		state = md->boot_stage;
		if(state >= 0) { 
			//CCCI_DBG_MSG(md->index, CHAR, "MD state %d\n", state);
			//state+='0'; // convert number to charactor
			ret = put_user((unsigned int)state, (unsigned int __user *)arg);
		} else {
			CCCI_ERR_MSG(md->index, CHAR, "Get MD state fail: %d\n", state);
			ret = state;
		}
		break;
	case CCCI_IOC_PCM_BASE_ADDR:
	case CCCI_IOC_PCM_LEN:
	case CCCI_IOC_ALLOC_MD_LOG_MEM:
		// deprecated, share memory operation
		break;
	case CCCI_IOC_MD_RESET:
		CCCI_INF_MSG(md->index, CHAR, "MD reset ioctl(%d) called by %s\n", ch, current->comm);
		ret = md->ops->reset(md);
		if(ret==0)
			ccci_send_virtual_md_msg(md, CCCI_MONITOR_CH, CCCI_MD_MSG_RESET, 0);
		break;
	case CCCI_IOC_FORCE_MD_ASSERT:
		CCCI_INF_MSG(md->index, CHAR, "Force MD assert ioctl(%d) called by %s\n", ch, current->comm);
		ret = md->ops->force_assert(md);
		break;
	case CCCI_IOC_SEND_RUN_TIME_DATA:
		if(ch == CCCI_MONITOR_CH) {
			ret = md->ops->send_runtime_data(md);
		} else {
			CCCI_INF_MSG(md->index, CHAR, "Set runtime by invalid user(%u) called by %s\n", ch, current->comm);
			ret = -1;
		}
		break;
	case CCCI_IOC_GET_MD_INFO:
		state = md->img_info[IMG_MD].img_info.version;
		ret = put_user((unsigned int)state, (unsigned int __user *)arg);
		break;
	case CCCI_IOC_GET_MD_EX_TYPE:
		ret = put_user((unsigned int)md->ex_type, (unsigned int __user *)arg);
		CCCI_INF_MSG(md->index, CHAR, "get modem exception type=%d ret=%d\n", md->ex_type, ret);
		break;
	case CCCI_IOC_SEND_STOP_MD_REQUEST:
		CCCI_INF_MSG(md->index, CHAR, "stop MD request ioctl called by %s\n", current->comm);
		ret = md->ops->reset(md);
		if(ret == 0) {
			md->ops->stop(md, 0);
			ret = ccci_send_virtual_md_msg(md, CCCI_MONITOR_CH, CCCI_MD_MSG_STOP_MD_REQUEST, 0);
		}
		break;
	case CCCI_IOC_SEND_START_MD_REQUEST:
		CCCI_INF_MSG(md->index, CHAR, "start MD request ioctl called by %s\n", current->comm);
		ret = ccci_send_virtual_md_msg(md, CCCI_MONITOR_CH, CCCI_MD_MSG_START_MD_REQUEST, 0);
		break;
	case CCCI_IOC_DO_START_MD:
		CCCI_INF_MSG(md->index, CHAR, "start MD ioctl called by %s\n", current->comm);
		ret = md->ops->start(md);
		break;
	case CCCI_IOC_DO_STOP_MD:
		CCCI_INF_MSG(md->index, CHAR, "stop MD ioctl called by %s\n", current->comm);
		ret = md->ops->stop(md, 0);
		break;
	case CCCI_IOC_ENTER_DEEP_FLIGHT:
		CCCI_INF_MSG(md->index, CHAR, "enter MD flight mode ioctl called by %s\n", current->comm);
		ret = md->ops->reset(md);
		if(ret==0) {
			md->ops->stop(md, 1000);
			ret = ccci_send_virtual_md_msg(md, CCCI_MONITOR_CH, CCCI_MD_MSG_ENTER_FLIGHT_MODE, 0);
		}
		break;
	case CCCI_IOC_LEAVE_DEEP_FLIGHT:
		CCCI_INF_MSG(md->index, CHAR, "leave MD flight mode ioctl called by %s\n", current->comm);
		ret = ccci_send_virtual_md_msg(md, CCCI_MONITOR_CH, CCCI_MD_MSG_LEAVE_FLIGHT_MODE, 0);
		break;

	case CCCI_IOC_POWER_ON_MD_REQUEST:
		CCCI_INF_MSG(md->index, CHAR, "Power on MD request ioctl called by %s\n", current->comm);
		ret = ccci_send_virtual_md_msg(md, CCCI_MONITOR_CH, CCCI_MD_MSG_POWER_ON_REQUEST, 0);
		break;

	case CCCI_IOC_POWER_OFF_MD_REQUEST:
		CCCI_INF_MSG(md->index, CHAR, "Power off MD request ioctl called by %s\n", current->comm);
		ret = ccci_send_virtual_md_msg(md, CCCI_MONITOR_CH, CCCI_MD_MSG_POWER_OFF_REQUEST, 0);
		break;
	case CCCI_IOC_POWER_ON_MD:
	case CCCI_IOC_POWER_OFF_MD:
		// abandoned
		CCCI_INF_MSG(md->index, CHAR, "Power on/off MD by user(%d) called by %s\n", ch, current->comm);
		ret = -1;
		break;
	case CCCI_IOC_SIM_SWITCH:
		if(copy_from_user(&sim_mode, (void __user *)arg, sizeof(unsigned int))) {
			CCCI_INF_MSG(md->index, CHAR, "IOC_SIM_SWITCH: copy_from_user fail!\n");
			ret = -EFAULT;
		} else {
			switch_sim_mode(md->index, (char*)&sim_mode, sizeof(sim_mode));
			CCCI_INF_MSG(md->index, CHAR, "IOC_SIM_SWITCH(%x): %d\n", sim_mode, ret); 	
		}
		break;
	case CCCI_IOC_SIM_SWITCH_TYPE:
		sim_switch_type = get_sim_switch_type();
		ret = put_user(sim_switch_type, (unsigned int __user *)arg);
		break;
	case CCCI_IOC_GET_SIM_TYPE:
		if (md->sim_type == 0xEEEEEEEE)
			CCCI_ERR_MSG(md->index, KERN, "md has not send sim type yet(%d)", md->sim_type);
		ret = put_user(md->sim_type, (unsigned int __user *)arg);
		break;
	case CCCI_IOC_ENABLE_GET_SIM_TYPE:
		if(copy_from_user(&enable_sim_type, (void __user *)arg, sizeof(unsigned int))) {
			CCCI_INF_MSG(md->index, CHAR, "CCCI_IOC_ENABLE_GET_SIM_TYPE: copy_from_user fail!\n");
			ret = -EFAULT;
		} else {
			ret = ccci_send_msg_to_md(md, CCCI_SYSTEM_TX, MD_SIM_TYPE, enable_sim_type);
		}
		break;
	case CCCI_IOC_SEND_BATTERY_INFO:
		ret = ccci_send_msg_to_md(md, CCCI_SYSTEM_TX, MD_GET_BATTERY_INFO, (u32)BAT_Get_Battery_Voltage(0));
		break;
	case CCCI_IOC_RELOAD_MD_TYPE:
		if(copy_from_user(&state, (void __user *)arg, sizeof(unsigned int))) {
			CCCI_INF_MSG(md->index, CHAR, "IOC_RELOAD_MD_TYPE: copy_from_user fail!\n");
			ret = -EFAULT;
		} else {
			CCCI_INF_MSG(md->index, CHAR, "IOC_RELOAD_MD_TYPE: storing md type(%d)!\n", state);
			ccci_reload_md_type(md, state);
		}
		break;
	case CCCI_IOC_SET_MD_IMG_EXIST:
		if(copy_from_user(&md_img_exist, (void __user *)arg, sizeof(md_img_exist))) {
			CCCI_INF_MSG(md->index, CHAR, "CCCI_IOC_SET_MD_IMG_EXIST: copy_from_user fail!\n");
			ret = -EFAULT;
		}
		break;
	case CCCI_IOC_GET_MD_IMG_EXIST:
		if (copy_to_user((void __user *)arg, &md_img_exist, sizeof(md_img_exist))) {
			CCCI_INF_MSG(md->index, CHAR, "CCCI_IOC_GET_MD_IMG_EXIST: copy_to_user fail\n");
			ret= -EFAULT;
		}
		break;
	case CCCI_IOC_GET_MD_TYPE:
		state = md->config.load_type;
		ret = put_user((unsigned int)state, (unsigned int __user *)arg);
		break;
	case CCCI_IOC_STORE_MD_TYPE:
		CCCI_DBG_MSG(md->index, CHAR, "store md type ioctl called by %s!\n",  current->comm);
		if(copy_from_user(&md->config.load_type_saving, (void __user *)arg, sizeof(unsigned int))) {
			CCCI_INF_MSG(md->index, CHAR, "store md type fail: copy_from_user fail!\n");
			ret = -EFAULT;
		}
		else {
			CCCI_INF_MSG(md->index, CHAR, "storing md type(%d) in kernel space!\n", md->config.load_type_saving);
			if (md->config.load_type_saving>=1 && md->config.load_type_saving<=MAX_IMG_NUM){
				if (md->config.load_type_saving != md->config.load_type)
					CCCI_INF_MSG(md->index, CHAR, "Maybe Wrong: md type storing not equal with current setting!(%d %d)\n", md->config.load_type_saving, md->config.load_type);
				//Notify md_init daemon to store md type in nvram
				ccci_send_virtual_md_msg(md, CCCI_MONITOR_CH, CCCI_MD_MSG_STORE_NVRAM_MD_TYPE, 0);
			}
			else {
				CCCI_INF_MSG(md->index, CHAR, "store md type fail: invalid md type(0x%x)\n", md->config.load_type_saving);
			}
		}
		break;
	case CCCI_IOC_GET_MD_TYPE_SAVING:
		ret = put_user(md->config.load_type_saving, (unsigned int __user *)arg);
		break;
	case CCCI_IPC_RESET_RECV:
	case CCCI_IPC_RESET_SEND:
	case CCCI_IPC_WAIT_MD_READY:
		ret = port_ipc_ioctl(port, cmd, arg);
		break;
	case CCCI_IOC_GET_EXT_MD_POST_FIX:
		if(copy_to_user((void __user *)arg, md->post_fix, IMG_POSTFIX_LEN)) {
			CCCI_INF_MSG(md->index, CHAR, "CCCI_IOC_GET_EXT_MD_POST_FIX: copy_to_user fail\n");
			ret= -EFAULT;
		}
		break;
	case CCCI_IOC_SEND_ICUSB_NOTIFY:
		if(copy_from_user(&sim_id, (void __user *)arg, sizeof(unsigned int))) {
			CCCI_INF_MSG(md->index, CHAR, "CCCI_IOC_SEND_ICUSB_NOTIFY: copy_from_user fail!\n");
			ret = -EFAULT;
		} else {
			ret = ccci_send_msg_to_md(md, CCCI_SYSTEM_TX, MD_ICUSB_NOTIFY, sim_id);
		}
		break;
	case CCCI_IOC_DL_TRAFFIC_CONTROL:
		if(copy_from_user(&traffic_control, (void __user *)arg, sizeof(unsigned int))) {
			CCCI_INF_MSG(md->index, CHAR, "CCCI_IOC_DL_TRAFFIC_CONTROL: copy_from_user fail\n");
		}
		if(traffic_control == 1) {
			// turn off downlink queue
		} else if(traffic_control == 0) {
			// turn on donwlink queue
		} else {
		}
		ret = 0;
		break;
		
	case CCCI_IOC_STORE_SIM_MODE:
	case CCCI_IOC_GET_SIM_MODE:
		// TODO:
		break;
		
	default:
		ret = -ENOTTY;
		break;
	}
	return ret;
}

unsigned int dev_char_poll(struct file *fp, struct poll_table_struct *poll)
{
	struct ccci_port *port = fp->private_data;
	unsigned int mask = 0;
	
	CCCI_DBG_MSG(port->modem->index, CHAR, "polling on CH%u\n", port->rx_ch);
	if(port->rx_ch == CCCI_IPC_RX) {
		mask = port_ipc_poll(fp, poll);
	} else {
		poll_wait(fp, &port->rx_wq, poll);
		// TODO: lack of poll wait for Tx
		if(!list_empty(&port->rx_req_list))
			mask |= POLLIN|POLLRDNORM;
		if(port->modem->ops->write_room(port->modem, PORT_TXQ_INDEX(port)) > 0)
			mask |= POLLOUT|POLLWRNORM;
	}

	return mask;
}

static struct file_operations char_dev_fops = {
	.owner = THIS_MODULE,
	.open = &dev_char_open,
	.read = &dev_char_read,
	.write = &dev_char_write,
	.release = &dev_char_close,
	.unlocked_ioctl = &dev_char_ioctl,
	.poll = &dev_char_poll,
};

static int port_char_init(struct ccci_port *port)
{
	struct cdev *dev;
	int ret = 0;

	CCCI_DBG_MSG(port->modem->index, CHAR, "char port %s is initializing\n", port->name);
	dev = kmalloc(sizeof(struct cdev), GFP_KERNEL);
	cdev_init(dev, &char_dev_fops);
	dev->owner = THIS_MODULE;
	if(port->rx_ch==CCCI_IPC_RX)
		port_ipc_init(port); // this will change port->minor, call it before register device
	else	
		port->private_data = dev; // not using
	ret = cdev_add(dev, MKDEV(port->major,port->minor), 1);
	ret = ccci_register_dev_node(port->name, port->major, port->minor);
	port->rx_length_th = MAX_QUEUE_LENGTH;
	return ret;
}

static int port_char_recv_req(struct ccci_port *port, struct ccci_request *req)
{
	unsigned long flags; // as we can not tell the context, use spin_lock_irqsafe for safe
	
	CCCI_DBG_MSG(port->modem->index, CHAR, "incomming on CH%u\n", port->rx_ch);

	if(!atomic_read(&port->usage_cnt)) {
		if(port->modem->md_state==EXCEPTION) {
			if(port->rx_ch!=CCCI_MD_LOG_RX && port->rx_ch!=CCCI_UART1_RX) // to make sure mdlogger can get MD dump
				goto drop;
		} else {
			if(port->rx_ch!=CCCI_UART2_RX) // to make sure muxd can get +EIND128 message
				goto drop;
		}
	}
	
	spin_lock_irqsave(&port->rx_req_lock, flags);
	if(port->rx_length < port->rx_length_th) {
		port->flags &= ~PORT_F_RX_FULLED;
		port->rx_length++;
		list_del(&req->entry); // dequeue from queue's list
		list_add_tail(&req->entry, &port->rx_req_list);
		spin_unlock_irqrestore(&port->rx_req_lock, flags);
		CCCI_DBG_MSG(port->modem->index, CHAR, "CH%u Rx length %d\n", port->rx_ch, port->rx_length);
#ifdef CCCI_STATISTIC
		ccci_update_request_stamp(req);
#endif
		wake_lock_timeout(&port->rx_wakelock, HZ);
		wake_up_all(&port->rx_wq);
		return 0;
	} else {
		port->flags |= PORT_F_RX_FULLED;
		spin_unlock_irqrestore(&port->rx_req_lock, flags);
		if(port->flags & PORT_F_ALLOW_DROP)
			goto drop;
		else
			return -CCCI_ERR_PORT_RX_FULL;
	}

drop:
	// drop this packet
	CCCI_DBG_MSG(port->modem->index, CHAR, "dropping on CH%u length %d\n", port->rx_ch, port->rx_length);
	list_del(&req->entry); 
	req->policy = RECYCLE;
	ccci_free_req(req);
	return -CCCI_ERR_DROP_PACKET;
}

static int port_char_req_match(struct ccci_port *port, struct ccci_request *req)
{
	struct ccci_header *ccci_h = (struct ccci_header *)req->skb->data;
	if(ccci_h->channel == port->rx_ch) {
		if(unlikely(port->rx_ch == CCCI_IPC_RX)) {
			return port_ipc_req_match(port, req);
		}
		return 1;
	}
	return 0;
}

static void port_char_md_state_notice(struct ccci_port *port, MD_STATE state)
{
	if(unlikely(port->rx_ch == CCCI_IPC_RX))
		port_ipc_md_state_notice(port, state);
}

struct ccci_port_ops char_port_ops = {
	.init = &port_char_init,
	.recv_request = &port_char_recv_req,
	.req_match = &port_char_req_match,
	.md_state_notice = &port_char_md_state_notice,
};

int ccci_subsys_char_init(struct ccci_modem *md)
{
	register_chrdev_region(MKDEV(md->major,0), 120, CCCI_DEV_NAME); // as IPC minor starts from 100
	return 0;
}
