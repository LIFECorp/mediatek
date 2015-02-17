#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/module.h>
#include "ccci_core.h"
#include "ccci_bm.h"

/*
 * if modem layer can disable IRQ and perform poll, define this can save some effort from calling netif_rx.
 * the difference between this method and the real NAPI is that, modem layer's poll is runing in its
 * work queue or something else it chooses, but NAPI's poll runs in NET RX SOFTIRQ.
 */
#ifndef CCCI_USE_NAPI
//#define DUM_NAPI
#endif

struct netdev_entity {
#ifdef CCCI_USE_NAPI
	struct napi_struct napi;
#endif
	struct net_device *ndev;
};

static int ccmni_open(struct net_device *dev)
{
	struct ccci_port *port = *((struct ccci_port **)netdev_priv(dev));

	atomic_inc(&port->usage_cnt);
	netif_start_queue(dev);
#ifdef CCCI_USE_NAPI
	napi_enable(&((struct netdev_entity *)port->private_data)->napi);
#endif
	return 0;
}

static int ccmni_close(struct net_device *dev)
{
	struct ccci_port *port = *((struct ccci_port **)netdev_priv(dev));

	atomic_dec(&port->usage_cnt);
	netif_stop_queue(dev);
#ifdef CCCI_USE_NAPI
	napi_disable(&((struct netdev_entity *)port->private_data)->napi);
#endif
	return 0;
}

static int ccmni_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct ccci_port *port = *((struct ccci_port **)netdev_priv(dev));
	struct ccci_request *req = NULL;
	struct ccci_header *ccci_h;
	int ret;
	static unsigned int ut_seq_num = 0;
	int skb_len = skb->len;

	CCCI_DBG_MSG(port->modem->index, NET, "tx skb %p on CH%d, len=%d/%d\n", skb, port->tx_ch, skb_headroom(skb), skb->len);
	if (skb->len > CCMNI_MTU) {
		CCCI_ERR_MSG(port->modem->index, NET, "exceeds MTU(%d) with %d/%d\n", CCMNI_MTU, dev->mtu, skb->len);
		dev_kfree_skb(skb);
		dev->stats.tx_dropped++;
		return NETDEV_TX_OK;
    }
    if (skb_headroom(skb) < sizeof(struct ccci_header)) {
		CCCI_ERR_MSG(port->modem->index, NET, "not enough header room on CH%d, len=%d header=%d hard_header=%d\n",
			port->tx_ch, skb->len, skb_headroom(skb), dev->hard_header_len);
		dev_kfree_skb(skb);
		dev->stats.tx_dropped++;
        return NETDEV_TX_OK;
    }

	req = ccci_alloc_req(OUT, -1, 1, 0);
	if(req) {
		req->skb = skb;
		req->policy = FREE;
		ccci_h = (struct ccci_header*)skb_push(skb, sizeof(struct ccci_header));
		ccci_h->channel = port->tx_ch;
		ccci_h->data[0] = 0;
		ccci_h->data[1] = skb->len; // as skb->len already included ccci_header after skb_push
		ccci_h->reserved = ut_seq_num++;
		ret = ccci_port_send_request(port, req);
		if(ret) {
			skb_pull(skb, sizeof(struct ccci_header)); // undo header, in next retry, we'll reserver header again
			req->policy = NOOP; // if you return busy, do NOT free skb as network may still use it
			ccci_free_req(req);
			return NETDEV_TX_BUSY;
		}
		dev->stats.tx_packets++;
		dev->stats.tx_bytes += skb_len;
	} else {
		CCCI_ERR_MSG(port->modem->index, NET, "fail to alloc request\n");
		return NETDEV_TX_BUSY;
	}
	return NETDEV_TX_OK;
}

static void ccmni_tx_timeout(struct net_device *dev)
{
	dev->stats.tx_errors++;
	netif_wake_queue(dev);
}

static const struct net_device_ops ccmni_netdev_ops = 
{
	.ndo_open		= ccmni_open,
	.ndo_stop		= ccmni_close,
	.ndo_start_xmit	= ccmni_start_xmit,
	.ndo_tx_timeout	= ccmni_tx_timeout,
};

int port_net_poll(struct napi_struct *napi ,int budget)
{
	struct ccci_port *port = *((struct ccci_port **)netdev_priv(napi->dev));

	return port->modem->ops->napi_poll(port->modem, PORT_RXQ_INDEX(port), napi, budget);
}

static int port_net_init(struct ccci_port *port)
{
	struct ccci_port **temp;
	struct net_device *dev = NULL;
	struct netdev_entity *nent = NULL;
	
	CCCI_DBG_MSG(port->modem->index, NET, "network port is initializing\n");
	dev = alloc_etherdev(sizeof(struct ccci_port *));
	dev->header_ops = NULL;
	dev->mtu = CCMNI_MTU;
	dev->tx_queue_len = 1000;
	dev->watchdog_timeo = 1*HZ;
	dev->flags = IFF_NOARP & /* ccmni is a pure IP device */ 
				(~IFF_BROADCAST & ~IFF_MULTICAST);	/* ccmni is P2P */
	dev->features = NETIF_F_VLAN_CHALLENGED; /* not support VLAN */
	dev->addr_len = ETH_ALEN; /* ethernet header size */
	dev->destructor = free_netdev;
	dev->hard_header_len += sizeof(struct ccci_header); /* reserve Tx CCCI header room */
	dev->netdev_ops = &ccmni_netdev_ops;
	
	temp = netdev_priv(dev);
	*temp = port;
	sprintf(dev->name, "%s", port->name);
	
	random_ether_addr((u8 *) dev->dev_addr);

	nent = kzalloc(sizeof(struct netdev_entity), GFP_KERNEL);
	nent->ndev = dev;
#ifdef CCCI_USE_NAPI
	netif_napi_add(dev, &nent->napi, port_net_poll, 64); // hardcode
#endif
	port->private_data = nent;
	register_netdev(dev);
	CCCI_INF_MSG(port->modem->index, NET, "network device %s hard_header_len=%d\n", dev->name, dev->hard_header_len);
	return 0;
}

static void ccmni_make_etherframe(void *_eth_hdr, unsigned char *mac_addr, unsigned int packet_type)
{
	struct ethhdr *eth_hdr = _eth_hdr;

	memcpy(eth_hdr->h_dest,   mac_addr, sizeof(eth_hdr->h_dest));
	memset(eth_hdr->h_source, 0, sizeof(eth_hdr->h_source));
	if(packet_type == 0x60){
		eth_hdr->h_proto = __constant_cpu_to_be16(ETH_P_IPV6);
	}else{
		eth_hdr->h_proto = __constant_cpu_to_be16(ETH_P_IP);
	}
}

static int port_net_recv_req(struct ccci_port *port, struct ccci_request* req)
{
	struct sk_buff *skb = req->skb;
	struct net_device *dev = ((struct netdev_entity *)port->private_data)->ndev;
	unsigned int packet_type;
	int skb_len = req->skb->len;
	
	CCCI_DBG_MSG(port->modem->index, NET, "incomming on CH%d\n", port->rx_ch);
	list_del(&req->entry); // dequeue from queue's list
	skb_pull(skb, sizeof(struct ccci_header));
	packet_type = skb->data[0] & 0xF0;
	ccmni_make_etherframe(skb->data-ETH_HLEN, dev->dev_addr, packet_type);
    skb_set_mac_header(skb, -ETH_HLEN);
	skb->dev = dev;
	if(packet_type == 0x60) {            
		skb->protocol  = htons(ETH_P_IPV6);
	}
	else {
		skb->protocol  = htons(ETH_P_IP);
	}
	skb->ip_summed = CHECKSUM_NONE;
#if defined(CCCI_USE_NAPI) || defined(DUM_NAPI)
	netif_receive_skb(skb);
#else
	netif_rx(skb);
#endif
	dev->stats.rx_packets++;
	dev->stats.rx_bytes += skb_len;
	req->policy = NOOP;
	ccci_free_req(req);
	wake_lock_timeout(&port->rx_wakelock, HZ);
	return 0;
}

static void port_net_md_state_notice(struct ccci_port *port, MD_STATE state)
{
	switch(state) {
	case RX_IRQ:
#ifdef CCCI_USE_NAPI
		napi_schedule(&((struct netdev_entity *)port->private_data)->napi);
		wake_lock_timeout(&port->rx_wakelock, HZ);
#endif
		break;
	default:
		break;
	};
}

struct ccci_port_ops net_port_ops = {
	.init = &port_net_init,
	.recv_request = &port_net_recv_req,
	.md_state_notice = &port_net_md_state_notice,
};
