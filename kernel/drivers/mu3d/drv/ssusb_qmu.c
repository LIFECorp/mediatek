#ifdef USE_SSUSB_QMU
#include <linux/spinlock.h>
#include <linux/dma-mapping.h>

#include "musb_core.h"
#include <linux/mu3d/hal/mu3d_hal_osal.h>
#include <linux/mu3d/hal/mu3d_hal_qmu_drv.h>
#include <linux/mu3d/hal/mu3d_hal_hw.h>
#include "ssusb_qmu.h"

//Sanity CR check in
/*
    1. Find the last gpd HW has executed and update Tx_gpd_last[]
    2. Set the flag for txstate to know that TX has been completed

    ported from proc_qmu_tx() from test driver.

    caller:qmu_interrupt after getting QMU done interrupt and TX is raised

*/
void qmu_done_tx(struct musb *musb, u8 ep_num, unsigned long flags)
{
	TGPD* gpd = Tx_gpd_last[ep_num];
	TGPD* gpd_current = (TGPD*)(os_readl(USB_QMU_TQCPR(ep_num)));
	struct musb_ep		*musb_ep = &musb->endpoints[ep_num].ep_in;
	struct usb_request	*request = NULL;
	struct musb_request	*req;

	/* trying to give_back the request to gadget driver. */
	req = next_request(musb_ep);
	if (!req) {
		qmu_printk(K_INFO, "[TXD]""%s Cannot get next request of %d, "
			"but QMU has done.\n", __func__, ep_num);
		return;
	} else {
		request = &req->request;
	}

	/*Transfer PHY addr got from QMU register to VIR addr*/
	gpd_current = phys_to_virt((unsigned long)gpd_current);

	/*
                      gpd or Last       gdp_current
                           |                  |
            |->  GPD1 --> GPD2 --> GPD3 --> GPD4 --> GPD5 -|
            |----------------------------------------------|
	*/

	qmu_printk(K_DEBUG, "[TXD]""%s EP%d, Last=%p, Current=%p, End=%p\n",
		__func__, ep_num, gpd, gpd_current, Tx_gpd_end[ep_num]);

	/*gpd_current should at least point to the next GPD to the previous last one.*/
	if (gpd == gpd_current) {
		qmu_printk(K_ERR, "[TXD][ERROR]""%s gpd(%p) == gpd_current(%p)\n", __func__, gpd, \
			gpd_current);
		return;
	}

	spin_unlock_irqrestore(&musb->lock, flags);

	//flush data from device to CPU
	dma_sync_single_for_cpu(musb->controller, virt_to_phys(gpd), \
				sizeof(TGPD), DMA_FROM_DEVICE);

	spin_lock_irqsave(&musb->lock, flags);

	if(TGPD_IS_FLAGS_HWO(gpd)) {
		qmu_printk(K_DEBUG, "[TXD]""%s HWO=1, CPR=%x\n", __func__, os_readl(USB_QMU_TQCPR(ep_num)));
		BUG_ON(1);
	}

	while (gpd!=gpd_current && !TGPD_IS_FLAGS_HWO(gpd)) {

		qmu_printk(K_DEBUG, "[TXD]""gpd=%p ->HWO=%d, BPD=%d, Next_GPD=%x, DataBuffer=%x, "
			"BufferLen=%d request=%p\n", gpd, (u32)TGPD_GET_FLAG(gpd), (u32)TGPD_GET_FORMAT(gpd), \
			(u32)TGPD_GET_NEXT(gpd), (u32)TGPD_GET_DATA(gpd), (u32)TGPD_GET_BUF_LEN(gpd), req);

		if(!TGPD_GET_NEXT(gpd)) {
			qmu_printk(K_ERR, "[TXD][ERROR]""Next GPD is null!!\n");
			//BUG_ON(1);
			break;
		}

		gpd = TGPD_GET_NEXT(gpd);

		spin_unlock_irqrestore(&musb->lock, flags);

		/*flush data from device to CPU*/
		dma_sync_single_for_cpu(musb->controller, \
				(dma_addr_t)gpd, sizeof(TGPD), DMA_FROM_DEVICE);

		spin_lock_irqsave(&musb->lock, flags);

		gpd = phys_to_virt((unsigned long)gpd);

		Tx_gpd_last[ep_num] = gpd;
		musb_g_giveback(musb_ep, request, 0);
		req = next_request(musb_ep);
		request = &req->request;
	}

	if(gpd!=gpd_current && TGPD_IS_FLAGS_HWO(gpd)) {
		qmu_printk(K_ERR, "[TXD][ERROR]""EP%d TQCSR=%x, TQSAR=%x, TQCPR=%x\n",
			ep_num, os_readl(USB_QMU_TQCSR(ep_num)), os_readl(USB_QMU_TQSAR(ep_num)),
				os_readl(USB_QMU_TQCPR(ep_num)));

		qmu_printk(K_ERR, "[TXD][ERROR]""QCR0=%x, QCR1=%x, QCR2=%x, QCR3=%x, "
			"QGCSR=%x\n", os_readl(U3D_QCR0), os_readl(U3D_QCR1), os_readl(U3D_QCR2), \
			os_readl(U3D_QCR3), os_readl(U3D_QGCSR));

		qmu_printk(K_ERR, "[TXD][ERROR]""HWO=%d, BPD=%d, Next_GPD=%x, DataBuffer=%x, "
			"BufferLen=%d, Endpoint=%d\n", (DEV_UINT32)TGPD_GET_FLAG(gpd), \
			(DEV_UINT32)TGPD_GET_FORMAT(gpd), (DEV_UINT32)TGPD_GET_NEXT(gpd), \
			(DEV_UINT32)TGPD_GET_DATA(gpd), (DEV_UINT32)TGPD_GET_BUF_LEN(gpd), \
			(DEV_UINT32)TGPD_GET_EPaddr(gpd));
	}

	qmu_printk(K_DEBUG, "[TXD]""%s EP%d, Last=%p, End=%p, complete\n", __func__,
		ep_num, Tx_gpd_last[ep_num], Tx_gpd_end[ep_num]);

	if (req != NULL) {
		if (request->length == 0) {
			qmu_printk(K_DEBUG, "[TXD]""==Send ZLP== %p\n", req);

			while(!(USB_ReadCsr32(U3D_TX1CSR0, req->epnum) & TX_FIFOFULL)) {
				USB_WriteCsr32(U3D_TX1CSR0, req->epnum, USB_ReadCsr32(U3D_TX1CSR0, req->epnum) | TX_TXPKTRDY);
				break;
			}

			qmu_printk(K_DEBUG, "[TXD]""Giveback ZLP of EP%d, actual:%d, length:%d %p\n",
				req->epnum, request->actual, request->length, request);

			musb_g_giveback(musb_ep, request, 0);
		}
	}
}

/*
   When receiving RXQ done interrupt, qmu_interrupt calls this function.

   1. Traverse GPD/BD data structures to count actual transferred length.
   2. Set the done flag to notify rxstate_qmu() to report status to upper gadget driver.

    ported from proc_qmu_rx() from test driver.

    caller:qmu_interrupt after getting QMU done interrupt and TX is raised

*/
void qmu_done_rx(struct musb *musb, u8 ep_num, unsigned long flags)
{
	TGPD* gpd = Rx_gpd_last[ep_num];
	TGPD* gpd_current = (TGPD*)(os_readl(USB_QMU_RQCPR(ep_num)));
	struct musb_ep		*musb_ep = &musb->endpoints[ep_num].ep_out;
	struct usb_request	*request = NULL;
	struct musb_request	*req;

	//trying to give_back the request to gadget driver.
	req = next_request(musb_ep);
	if (!req) {
		qmu_printk(K_ERR, "[RXD]""%s Cannot get next request of %d, "
			"but QMU has done.\n", __func__, ep_num);
		return;
	} else {
		request = &req->request;
	}

	/*Transfer PHY addr got from QMU register to VIR addr*/
	gpd_current = phys_to_virt((unsigned long)gpd_current);

	qmu_printk(K_DEBUG, "[RXD]""%s EP%d, Last=%p, Current=%p, End=%p\n",
		__func__, ep_num, gpd, gpd_current, Rx_gpd_end[ep_num]);

	/*gpd_current should at least point to the next GPD to the previous last one.*/
	if (gpd == gpd_current) {
		qmu_printk(K_ERR, "[RXD][ERROR]""%s gpd(%p) == gpd_current(%p)\n", __func__, gpd, \
			gpd_current);

		qmu_printk(K_ERR, "[RXD][ERROR]""EP%d RQCSR=%x, RQSAR=%x, RQCPR=%x, RQLDPR=%x\n",
				ep_num, os_readl(USB_QMU_RQCSR(ep_num)), os_readl(USB_QMU_RQSAR(ep_num)),
				os_readl(USB_QMU_RQCPR(ep_num)), os_readl(USB_QMU_RQLDPR(ep_num)));

		qmu_printk(K_ERR, "[RXD][ERROR]""QCR0=%x, QCR1=%x, QCR2=%x, QCR3=%x, "
			"QGCSR=%x\n", os_readl(U3D_QCR0), os_readl(U3D_QCR1), os_readl(U3D_QCR2), \
			os_readl(U3D_QCR3), os_readl(U3D_QGCSR));

		qmu_printk(K_INFO,"[RXD][ERROR]""HWO=%d, Next_GPD=%x ,DataBufLen=%d, "
			"DataBuf=%x, RecvLen=%d, Endpoint=%d\n",
			(DEV_UINT32)TGPD_GET_FLAG(gpd), (DEV_UINT32)TGPD_GET_NEXT(gpd),
			(DEV_UINT32)TGPD_GET_DataBUF_LEN(gpd), (DEV_UINT32)TGPD_GET_DATA(gpd),
			(DEV_UINT32)TGPD_GET_BUF_LEN(gpd), (DEV_UINT32)TGPD_GET_EPaddr(gpd));

		return;
	}

	spin_unlock_irqrestore(&musb->lock, flags);

	//invalidate GPD data in CPU
	dma_sync_single_for_cpu(musb->controller, virt_to_phys(gpd), \
				sizeof(TGPD), DMA_FROM_DEVICE);

	spin_lock_irqsave(&musb->lock, flags);

	if(!gpd || !gpd_current) {
		qmu_printk(K_ERR, "[RXD][ERROR]""%s EP%d, gpd=%p, gpd_current=%p, ishwo=%d, \
			rx_gpd_last=%p, RQCPR=0x%x\n", __func__,
			ep_num, gpd, gpd_current, ((gpd==NULL) ? 999 : TGPD_IS_FLAGS_HWO(gpd)),
			Rx_gpd_last[ep_num], os_readl(USB_QMU_RQCPR(ep_num)));
		return;
	}

	if(TGPD_IS_FLAGS_HWO(gpd)) {
		qmu_printk(K_ERR, "[RXD][ERROR]""HWO=1!!\n");
		BUG_ON(1);
	}

	while(gpd != gpd_current && !TGPD_IS_FLAGS_HWO(gpd)) {
		DEV_UINT32 rcv_len = (DEV_UINT32)TGPD_GET_BUF_LEN(gpd);
		DEV_UINT32 buf_len  = (DEV_UINT32)TGPD_GET_DataBUF_LEN(gpd);
		if(rcv_len > buf_len)
			qmu_printk(K_ERR, "[RXD][ERROR]""%s rcv(%d) > buf(%d) AUK!?\n", __func__, rcv_len, buf_len);

		qmu_printk(K_DEBUG, "[RXD]""gpd=%p ->HWO=%d, Next_GPD=%p, RcvLen=%d, BufLen=%d, pBuf=%p\n",
				gpd, TGPD_GET_FLAG(gpd), TGPD_GET_NEXT(gpd), rcv_len, buf_len, TGPD_GET_DATA(gpd));

		request->actual += rcv_len;

		if (!TGPD_GET_NEXT(gpd) || !TGPD_GET_DATA(gpd)) {
			qmu_printk(K_ERR, "[RXD][ERROR]""%s EP%d ,gpd=%p\n", __func__, ep_num, gpd);
			BUG_ON(1);
		}

		gpd = TGPD_GET_NEXT(gpd);

		spin_unlock_irqrestore(&musb->lock, flags);

		//invalidate next GPD data in CPU
		dma_sync_single_for_cpu(musb->controller, (dma_addr_t)gpd, sizeof(TGPD), DMA_FROM_DEVICE);

		spin_lock_irqsave(&musb->lock, flags);

		gpd = phys_to_virt((unsigned long)gpd);

		if(!gpd) {
			qmu_printk(K_ERR, "[RXD][ERROR]""%s EP%d ,gpd=%p\n", __func__, ep_num, gpd);
			BUG_ON(1);
		}

		Rx_gpd_last[ep_num]=gpd;
		musb_g_giveback(musb_ep, request, 0);
		req = next_request(musb_ep);
		request = &req->request;
	}

	if(gpd!=gpd_current && TGPD_IS_FLAGS_HWO(gpd)) {
		qmu_printk(K_ERR, "[RXD][ERROR]""gpd=%p\n", gpd);

		qmu_printk(K_ERR, "[RXD][ERROR]""EP%d RQCSR=%x, RQSAR=%x, RQCPR=%x, RQLDPR=%x\n",
				ep_num, os_readl(USB_QMU_RQCSR(ep_num)), os_readl(USB_QMU_RQSAR(ep_num)),
				os_readl(USB_QMU_RQCPR(ep_num)), os_readl(USB_QMU_RQLDPR(ep_num)));

		qmu_printk(K_ERR, "[RXD][ERROR]""QCR0=%x, QCR1=%x, QCR2=%x, QCR3=%x, "
			"QGCSR=%x\n", os_readl(U3D_QCR0), os_readl(U3D_QCR1), os_readl(U3D_QCR2), \
			os_readl(U3D_QCR3), os_readl(U3D_QGCSR));

		qmu_printk(K_INFO,"[RXD][ERROR]""HWO=%d, Next_GPD=%x ,DataBufLen=%d, "
			"DataBuf=%x, RecvLen=%d, Endpoint=%d\n",
			(DEV_UINT32)TGPD_GET_FLAG(gpd), (DEV_UINT32)TGPD_GET_NEXT(gpd),
			(DEV_UINT32)TGPD_GET_DataBUF_LEN(gpd), (DEV_UINT32)TGPD_GET_DATA(gpd),
			(DEV_UINT32)TGPD_GET_BUF_LEN(gpd), (DEV_UINT32)TGPD_GET_EPaddr(gpd));
	}

	qmu_printk(K_DEBUG, "[RXD]""%s EP%d, Last=%p, End=%p, complete\n", __func__,
		ep_num, Rx_gpd_last[ep_num], Rx_gpd_end[ep_num]);
}

void qmu_done_tasklet(unsigned long data)
{
	unsigned int qmu_val;
	unsigned int i;
	unsigned long flags;
	struct musb *musb = (struct musb *)data;

	spin_lock_irqsave(&musb->lock, flags);

	qmu_val = musb->qmu_done_intr;

	musb->qmu_done_intr = 0;

	for(i=1; i<=MAX_QMU_EP; i++) {
		if(qmu_val & QMU_RX_DONE(i)) {
			qmu_done_rx(musb, i, flags);
		}
		if(qmu_val & QMU_TX_DONE(i)) {
			qmu_done_tx(musb, i, flags);
		}
	}
	spin_unlock_irqrestore(&musb->lock, flags);
}

void qmu_exception_interrupt(struct musb *musb, DEV_UINT32 wQmuVal)
{
	u32 wErrVal;
	int i = (int)wQmuVal;

	if(wQmuVal & RXQ_CSERR_INT)
		qmu_printk(K_ERR, "==Rx %d checksum error==\n", i);

	if(wQmuVal & RXQ_LENERR_INT)
		qmu_printk(K_ERR, "==Rx %d length error==\n", i);

	if(wQmuVal & TXQ_CSERR_INT)
		qmu_printk(K_ERR, "==Tx %d checksum error==\n", i);

	if(wQmuVal & TXQ_LENERR_INT)
		qmu_printk(K_ERR, "==Tx %d length error==\n", i);

	if((wQmuVal & RXQ_CSERR_INT)||(wQmuVal & RXQ_LENERR_INT)) {
		wErrVal=os_readl(U3D_RQERRIR0);
		qmu_printk(K_DEBUG, "Rx Queue error in QMU mode![0x%x]\r\n", (unsigned int)wErrVal);
		for(i=1; i<=MAX_QMU_EP; i++) {
			if(wErrVal &QMU_RX_CS_ERR(i))
				qmu_printk(K_ERR, "Rx %d length error!\r\n", i);

			if(wErrVal &QMU_RX_LEN_ERR(i))
				qmu_printk(K_ERR, "RX EP%d Recv Length error\n", i);
		}
		os_writel(U3D_RQERRIR0, wErrVal);
	}

	if(wQmuVal & RXQ_ZLPERR_INT) {
		wErrVal=os_readl(U3D_RQERRIR1);
		qmu_printk(K_DEBUG, "Rx Queue error in QMU mode![0x%x]\r\n", (unsigned int)wErrVal);
		for(i=1; i<=MAX_QMU_EP; i++) {
			if(wErrVal &QMU_RX_ZLP_ERR(i)) {
				/*FIXME: should _NOT_ got this error. But now just accept.*/
				qmu_printk(K_INFO, "RX EP%d Recv ZLP\n", i);
			}
		}
		os_writel(U3D_RQERRIR1, wErrVal);
	}

	if((wQmuVal & TXQ_CSERR_INT)||(wQmuVal & TXQ_LENERR_INT)) {
		wErrVal=os_readl(U3D_TQERRIR0);
		qmu_printk(K_DEBUG, "Tx Queue error in QMU mode![0x%x]\r\n", (unsigned int)wErrVal);
		for(i=1; i<=MAX_QMU_EP; i++) {
			if(wErrVal &QMU_TX_CS_ERR(i))
				qmu_printk(K_ERR, "Tx %d checksum error!\r\n", i);

			if(wErrVal &QMU_TX_LEN_ERR(i))
				qmu_printk(K_ERR, "Tx %d buffer length error!\r\n", i);
		}
		os_writel(U3D_TQERRIR0, wErrVal);
	}

	if((wQmuVal & RXQ_EMPTY_INT)||(wQmuVal & TXQ_EMPTY_INT)) {
		DEV_UINT32 wEmptyVal = os_readl(U3D_QEMIR);
		qmu_printk(K_DEBUG, "%s Empty in QMU mode![0x%x]\r\n",
					(wQmuVal & TXQ_EMPTY_INT)?"TX":"RX", wEmptyVal);
		os_writel(U3D_QEMIR, wEmptyVal);
	}
}

#endif
