
#ifdef USE_SSUSB_QMU

#include <linux/mu3d/hal/mu3d_hal_osal.h>
#define _MTK_QMU_DRV_EXT_
#include <linux/mu3d/hal/mu3d_hal_qmu_drv.h>
#undef _MTK_QMU_DRV_EXT_
#include <linux/mu3d/hal/mu3d_hal_usb_drv.h>
#include <linux/mu3d/hal/mu3d_hal_hw.h>

/**
 * get_bd - get a null gpd
 * @args - arg1: dir, arg2: ep number
 */
PGPD get_gpd(USB_DIR dir,DEV_UINT32 num)
{
	PGPD ptr;

	if( dir == USB_RX) {
		ptr = Rx_gpd_List[num].pNext;

		//qmu_printk(K_DEBUG, "[RX]""GPD List[%d]->Next=%p\n", num, Rx_gpd_List[num].pNext);

		Rx_gpd_List[num].pNext = Rx_gpd_List[num].pNext + (AT_GPD_EXT_LEN/sizeof(TGPD)+1);

		//qmu_printk(K_DEBUG, "[Rx]""GPD List[%d]->Start=%p, Next=%p, End=%p\n",
		//	num, Rx_gpd_List[num].pStart, Rx_gpd_List[num].pNext, Rx_gpd_List[num].pEnd);

		if ( Rx_gpd_List[num].pNext >= Rx_gpd_List[num].pEnd ) {
			Rx_gpd_List[num].pNext = Rx_gpd_List[num].pStart;
		}
	} else {
		ptr = Tx_gpd_List[num].pNext;

		//qmu_printk(K_DEBUG, "[TX]""GPD List[%d]->Next=%p\n", num, Tx_gpd_List[num].pNext);

		/*
		 * Here is really tricky.
		 * The size of a GPD is 16 bytes. But the cache line size is 64B. If all GPDs are allocated continiously.
		 * When doing invalidating the cache. The size of 64B from the specified address would flush to
		 * the physical memory. This action may cause that other GPDs corrupted, like HWO=1 when receiving
		 * QMU Done interrupt. Current workaround is that let a GPD as 64 bytes. So the next GPD is behind 64bytes.
		 */
		Tx_gpd_List[num].pNext = Tx_gpd_List[num].pNext + (AT_GPD_EXT_LEN/sizeof(TGPD)+1);

		//qmu_printk(K_DEBUG, "[TX]""GPD List[%d]->Start=%p, pNext=%p, pEnd=%p\n",
		//	num, Tx_gpd_List[num].pStart, Tx_gpd_List[num].pNext, Tx_gpd_List[num].pEnd);

		if ( Tx_gpd_List[num].pNext >= Tx_gpd_List[num].pEnd ) {
			Tx_gpd_List[num].pNext = Tx_gpd_List[num].pStart;
		}
	}
	return ptr;
}

/**
 * get_bd - align gpd ptr to target ptr
 * @args - arg1: dir, arg2: ep number, arg3: target ptr
 */
void gpd_ptr_align(USB_DIR dir,DEV_UINT32 num,PGPD ptr)
{
 	DEV_UINT32 run_next;
	run_next =true;

	//qmu_printk(K_DEBUG,"%s %d, EP%d, ptr=%p\n", __func__, dir, num, ptr);

	while(run_next)
	{
	 	if(ptr==get_gpd(dir,num)){
			run_next=false;
	 	}
	}
}

/**
 * init_gpd_list - initialize gpd management list
 * @args - arg1: dir, arg2: ep number, arg3: gpd virtual addr, arg4: gpd ioremap addr, arg5: gpd number
 */
void init_gpd_list(USB_DIR dir,int num,PGPD ptr,PGPD io_ptr,DEV_UINT32 size)
{
	if (dir == USB_RX) {
		Rx_gpd_List[num].pStart = ptr;
		Rx_gpd_List[num].pEnd = (PGPD)((DEV_UINT8*)(ptr + size) + AT_GPD_EXT_LEN*size);
		Rx_gpd_Offset[num]=(DEV_UINT32)ptr - (DEV_UINT32)io_ptr;
		ptr++;
		Rx_gpd_List[num].pNext = (PGPD)((DEV_UINT8*)ptr + AT_GPD_EXT_LEN);
		qmu_printk(K_INFO, "Rx_gpd_List[%d].pStart=%p, pNext=%p, pEnd=%p\n", \
			num, Rx_gpd_List[num].pStart, Rx_gpd_List[num].pNext, Rx_gpd_List[num].pEnd);
		qmu_printk(K_INFO, "Rx_gpd_Offset[%d]=0x%08X\n", num, Rx_gpd_Offset[num]);
		qmu_printk(K_INFO, "virtual start=%p, end=%p\n", ptr, ptr+size);
		qmu_printk(K_INFO, "dma addr start=%p, end=%p\n", io_ptr, io_ptr+size);
		qmu_printk(K_INFO, "dma addr start=%p, end=%p\n", (void *)virt_to_phys(ptr), (void *)virt_to_phys(ptr+size));
	} else {
		Tx_gpd_List[num].pStart = ptr;
	 	Tx_gpd_List[num].pEnd = (PGPD)((DEV_UINT8*)(ptr + size) + AT_GPD_EXT_LEN*size);
		Tx_gpd_Offset[num] = (DEV_UINT32)ptr - (DEV_UINT32)io_ptr;
		ptr++;
	 	Tx_gpd_List[num].pNext = (PGPD)((DEV_UINT8*)ptr + AT_GPD_EXT_LEN);
		qmu_printk(K_INFO, "Tx_gpd_List[%d].pStart=%p, pNext=%p, pEnd=%p\n",
			num, Tx_gpd_List[num].pStart, Tx_gpd_List[num].pNext, Tx_gpd_List[num].pEnd);
		qmu_printk(K_INFO, "Tx_gpd_Offset[%d]=0x%08X\n", num, Tx_gpd_Offset[num]);
		qmu_printk(K_INFO, "virtual start=%p, end=%p\n", ptr, ptr+size);
		qmu_printk(K_INFO, "dma addr start=%p, end=%p\n", io_ptr, io_ptr+size);
		qmu_printk(K_INFO, "dma addr start=%p, end=%p\n", (void *)virt_to_phys(ptr), (void *)virt_to_phys(ptr+size));
	}
}

/**
 * free_gpd - free gpd management list
 * @args - arg1: dir, arg2: ep number
 */
void free_gpd(USB_DIR dir,int num)
{
	if (dir == USB_RX) {
		memset(Rx_gpd_List[num].pStart, 0, MAX_GPD_NUM * (sizeof(TGPD)+AT_GPD_EXT_LEN));
	} else {
		memset(Tx_gpd_List[num].pStart, 0, MAX_GPD_NUM * (sizeof(TGPD)+AT_GPD_EXT_LEN));
	}
}

/**
 * mu3d_hal_alloc_qmu_mem - allocate gpd and bd memory for all ep
 *
 */
void mu3d_hal_alloc_qmu_mem(void){
	DEV_UINT32 i, size;
	TGPD *ptr,*io_ptr;

	for ( i=1; i<=MAX_QMU_EP; i++) {
		/* Allocate Rx GPD */
		size = (sizeof(TGPD) + AT_GPD_EXT_LEN) * MAX_GPD_NUM;
		ptr = (TGPD*)kmalloc(size, GFP_KERNEL);
		memset(ptr, 0 , size);

		io_ptr = (TGPD *)dma_map_single(NULL, ptr, size, DMA_BIDIRECTIONAL);
		init_gpd_list( USB_RX, i, ptr, io_ptr, MAX_GPD_NUM);
		Rx_gpd_end[i]= ptr;
		qmu_printk(K_INFO, "ALLOC RX GPD End [%d] Virtual Mem=%p, DMA addr=%p\n", i, Rx_gpd_end[i], io_ptr);
		TGPD_CLR_FLAGS_HWO(Rx_gpd_end[i]);
		Rx_gpd_head[i]=Rx_gpd_last[i]=Rx_gpd_end[i];
		qmu_printk(K_INFO, "RQSAR[%d]=%p\n", i, (void *)virt_to_phys(Rx_gpd_end[i]));

		/* Allocate Tx GPD */
		size = (sizeof(TGPD) + AT_GPD_EXT_LEN) * MAX_GPD_NUM;
		ptr = (TGPD *)kmalloc(size, GFP_KERNEL);
		memset(ptr, 0 , size);

		io_ptr = (TGPD *)dma_map_single(NULL, ptr, size, DMA_BIDIRECTIONAL);
		init_gpd_list( USB_TX, i, ptr, io_ptr, MAX_GPD_NUM);
		Tx_gpd_end[i]= ptr;
		qmu_printk(K_INFO, "ALLOC TX GPD End [%d] Virtual Mem=%p, DMA addr=%p\n", i, Tx_gpd_end[i], io_ptr);
		TGPD_CLR_FLAGS_HWO(Tx_gpd_end[i]);
		Tx_gpd_head[i]=Tx_gpd_last[i]=Tx_gpd_end[i];
		qmu_printk(K_INFO, "TQSAR[%d]=%p\n", i, (void *)virt_to_phys(Tx_gpd_end[i]));
    }
}

/**
 * mu3d_hal_init_qmu - initialize qmu
 *
 */
void mu3d_hal_init_qmu(void)
{
	DEV_UINT32 i;
	DEV_UINT32 QCR = 0;

	/* Initialize QMU Tx/Rx start address. */
	for(i=1; i<=MAX_QMU_EP; i++){
		qmu_printk(K_INFO, "==EP[%d]==Start addr RXQ=0x%08x, TXQ=0x%08x\n", i, \
			virt_to_phys(Rx_gpd_head[i]), virt_to_phys(Tx_gpd_head[i]));
		QCR|=QMU_RX_EN(i);
		QCR|=QMU_TX_EN(i);
		os_writel(USB_QMU_RQSAR(i), virt_to_phys(Rx_gpd_head[i]));
		os_writel(USB_QMU_TQSAR(i), virt_to_phys(Tx_gpd_head[i]));
		Tx_gpd_end[i] = Tx_gpd_last[i] = Tx_gpd_head[i];
		Rx_gpd_end[i] = Rx_gpd_last[i] = Rx_gpd_head[i];
		gpd_ptr_align(USB_TX,i,Tx_gpd_end[i]);
		gpd_ptr_align(USB_RX,i,Rx_gpd_end[i]);
	}

	/* Enable QMU interrupt. */
	os_writel(U3D_QIESR1, TXQ_EMPTY_IESR | TXQ_CSERR_IESR | TXQ_LENERR_IESR | \
						RXQ_EMPTY_IESR | RXQ_CSERR_IESR | RXQ_LENERR_IESR | \
						RXQ_ZLPERR_IESR);
	os_writel(U3D_EPIESR, EP0ISR);
}

/**
 * mu3d_hal_cal_checksum - calculate check sum
 * @args - arg1: data buffer, arg2: data length
 */
DEV_UINT8 mu3d_hal_cal_checksum(DEV_UINT8 *data, DEV_INT32 len)
{
	DEV_UINT8 *uDataPtr, ckSum;
	DEV_INT32 i;

	*(data + 1) = 0x0;
	uDataPtr = data;
	ckSum = 0;
	for (i = 0; i < len; i++){
		ckSum += *(uDataPtr + i);
	}
  	return 0xFF - ckSum;
}

/**
 * mu3d_hal_resume_qmu - resume qmu function
 * @args - arg1: ep number, arg2: dir
 */
void mu3d_hal_resume_qmu(DEV_INT32 q_num,  USB_DIR dir)
{
	if (dir == USB_TX) {
		//qmu_printk(K_DEBUG, "%s EP%d CSR=%x, CPR=%x\n", __func__, q_num, os_readl(USB_QMU_TQCSR(q_num)), os_readl(USB_QMU_TQCPR(q_num)));
		os_writel(USB_QMU_TQCSR(q_num), QMU_Q_RESUME);
		if(!os_readl( USB_QMU_TQCSR(q_num))) {
			qmu_printk(K_WARNIN, "[ERROR]""%s TQCSR[%d]=%x\n", __func__, q_num, os_readl( USB_QMU_TQCSR(q_num)));
			os_writel( USB_QMU_TQCSR(q_num), QMU_Q_RESUME);
			qmu_printk(K_WARNIN, "[ERROR]""%s TQCSR[%d]=%x\n", __func__, q_num, os_readl( USB_QMU_TQCSR(q_num)));
		}
	} else if(dir == USB_RX) {
		os_writel(USB_QMU_RQCSR(q_num), QMU_Q_RESUME);
		if(!os_readl( USB_QMU_RQCSR(q_num))) {
			qmu_printk(K_WARNIN, "[ERROR]""%s RQCSR[%d]=%x\n", __func__, q_num, os_readl( USB_QMU_RQCSR(q_num)));
			os_writel( USB_QMU_RQCSR(q_num), QMU_Q_RESUME);
			qmu_printk(K_WARNIN, "[ERROR]""%s RQCSR[%d]=%x\n", __func__, q_num, os_readl( USB_QMU_RQCSR(q_num)));
		}
	} else {
		qmu_printk(K_ERR, "%s wrong direction!!!\n", __func__);
		BUG_ON(1);
	}
}

/**
 * mu3d_hal_prepare_tx_gpd - prepare tx gpd/bd
 * @args - arg1: gpd address, arg2: data buffer address, arg3: data length, arg4: ep number, arg5: with bd or not, arg6: write hwo bit or not,  arg7: write ioc bit or not
 */
TGPD* mu3d_hal_prepare_tx_gpd(TGPD* gpd, dma_addr_t pBuf, DEV_UINT32 data_len,
					DEV_UINT8 ep_num, DEV_UINT8 _is_bdp, DEV_UINT8 isHWO,
					DEV_UINT8 ioc, DEV_UINT8 bps,DEV_UINT8 zlp)
{
	qmu_printk(K_DEBUG, "[TX]""%s gpd=%p, epnum=%d, len=%d, _is_bdp=%d size(TGPD)=%d\n", __func__, \
		gpd, ep_num, data_len, _is_bdp, sizeof(TGPD));

	/*Set actual data point to "DATA Buffer"*/
	TGPD_SET_DATA(gpd, pBuf);
	/*Clear "BDP(Buffer Descriptor Present)" flag*/
	TGPD_CLR_FORMAT_BDP(gpd);
	/*
	 * "Data Buffer Length" =
	 * 0        (If data length > GPD buffer length, use BDs),
	 * data_len (If data length < GPD buffer length, only use GPD)
	 */
	TGPD_SET_BUF_LEN(gpd, data_len);

	/*"GPD extension length" = 0. Does not use GPD EXT!!*/
	TGPD_SET_EXT_LEN(gpd, 0);

	/*Default: zlp=false, except type=ISOC*/
	TGPD_CLR_FORMAT_ZLP(gpd);

	/*Default: bps=false*/
	TGPD_CLR_FORMAT_BPS(gpd);

	/*Default: ioc=true*/
	TGPD_SET_FORMAT_IOC(gpd);

	/*Get the next GPD*/
	Tx_gpd_end[ep_num] = get_gpd(USB_TX ,ep_num);
	qmu_printk(K_DEBUG, "[TX]""Tx_gpd_end[%d]=%p\n", ep_num, Tx_gpd_end[ep_num]);

	/*Initialize the new GPD*/
	memset(Tx_gpd_end[ep_num], 0 , sizeof(TGPD) + AT_GPD_EXT_LEN);

	/*Clear "HWO(Hardware Own)" flag*/
	TGPD_CLR_FLAGS_HWO(Tx_gpd_end[ep_num]);

	/*Set "Next GDP pointer" as the next GPD*/
	TGPD_SET_NEXT(gpd, virt_to_phys(Tx_gpd_end[ep_num]));

	/*Default: isHWO=true*/
	TGPD_SET_CHKSUM(gpd, CHECKSUM_LENGTH); /*Set GPD Checksum*/
	TGPD_SET_FLAGS_HWO(gpd); /*Set HWO flag*/

	/*Flush the data of GPD stuct to device*/
	dma_sync_single_for_device(NULL, virt_to_phys(gpd), sizeof(TGPD) + AT_GPD_EXT_LEN, DMA_TO_DEVICE);

	return gpd;
}

static inline int check_next_gpd(TGPD* gpd, TGPD* next_gpd)
{
	if( ((u32)next_gpd - (u32)gpd) == 0x40)
		return 1;
	else if( ((u32)gpd - (u32)next_gpd) == 0x7c0)
		return 1;
	else {
		qmu_printk(K_ERR, "[RX]""%p <-> %p\n", gpd, next_gpd);
		return 0;
	}
}

/**
 * mu3d_hal_prepare_rx_gpd - prepare rx gpd/bd
 * @args - arg1: gpd address, arg2: data buffer address, arg3: data length, arg4: ep number, arg5: with bd or not, arg6: write hwo bit or not,  arg7: write ioc bit or not
 */
TGPD* mu3d_hal_prepare_rx_gpd(TGPD*gpd, dma_addr_t pBuf, DEV_UINT32 data_len,
				DEV_UINT8 ep_num, DEV_UINT8 _is_bdp, DEV_UINT8 isHWO,
				DEV_UINT8 ioc, DEV_UINT8 bps, DEV_UINT32 cMaxPacketSize)
{
	qmu_printk(K_DEBUG, "[RX]""%s gpd=%p, epnum=%d, len=%d\n", __func__, \
		gpd, ep_num, data_len);

	/*Set actual data point to "DATA Buffer"*/
	TGPD_SET_DATA(gpd, pBuf);
	/*Clear "BDP(Buffer Descriptor Present)" flag*/
	TGPD_CLR_FORMAT_BDP(gpd);
	/*
	 * Set "Allow Data Buffer Length" =
	 * 0        (If data length > GPD buffer length, use BDs),
	 * data_len (If data length < GPD buffer length, only use GPD)
	 */
	TGPD_SET_DataBUF_LEN(gpd, data_len);

	/*Set "Transferred Data Length" = 0*/
	TGPD_SET_BUF_LEN(gpd, 0);

	/*Default: bps=false*/
	TGPD_CLR_FORMAT_BPS(gpd);

	/*Default: ioc=true*/
	TGPD_SET_FORMAT_IOC(gpd);

	/*Get the next GPD*/
	Rx_gpd_end[ep_num] = get_gpd(USB_RX ,ep_num);
	qmu_printk(K_DEBUG, "[RX]""Rx_gpd_end[%d]=%p gpd=%p\n", ep_num, Rx_gpd_end[ep_num], gpd);

	//BUG_ON(!check_next_gpd(gpd, Rx_gpd_end[ep_num]));

	/*Initialize the new GPD*/
	memset(Rx_gpd_end[ep_num], 0 , sizeof(TGPD) + AT_GPD_EXT_LEN);

	/*Clear "HWO(Hardware Own)" flag*/
	TGPD_CLR_FLAGS_HWO(Rx_gpd_end[ep_num]);

	/*Set Next GDP pointer to the next GPD*/
	TGPD_SET_NEXT(gpd, virt_to_phys(Rx_gpd_end[ep_num]));

	/*Default: isHWO=true*/
	TGPD_SET_CHKSUM(gpd, CHECKSUM_LENGTH); /*Set GPD Checksum*/
	TGPD_SET_FLAGS_HWO(gpd); /*Set HWO flag*/

	//os_printk(K_DEBUG,"Rx gpd info { HWO %d, Next_GPD %x ,DataBufferLength %d, DataBuffer %x, Recived Len %d, Endpoint %d, TGL %d, ZLP %d}\n",
	//				(DEV_UINT32)TGPD_GET_FLAG(gpd), (DEV_UINT32)TGPD_GET_NEXT(gpd),
	//				(DEV_UINT32)TGPD_GET_DataBUF_LEN(gpd), (DEV_UINT32)TGPD_GET_DATA(gpd),
	//				(DEV_UINT32)TGPD_GET_BUF_LEN(gpd), (DEV_UINT32)TGPD_GET_EPaddr(gpd),
	//				(DEV_UINT32)TGPD_GET_TGL(gpd), (DEV_UINT32)TGPD_GET_ZLP(gpd));

	/*Flush the data of GPD stuct to device*/
	dma_sync_single_for_device(NULL, virt_to_phys(gpd), sizeof(TGPD) + AT_GPD_EXT_LEN, DMA_TO_DEVICE);

	return gpd;
}

/**
 * mu3d_hal_insert_transfer_gpd - insert new gpd/bd
 * @args - arg1: ep number, arg2: dir, arg3: data buffer, arg4: data length,  arg5: write hwo bit or not,  arg6: write ioc bit or not
 */
void mu3d_hal_insert_transfer_gpd(DEV_INT32 ep_num,USB_DIR dir, dma_addr_t buf,
					DEV_UINT32 count, DEV_UINT8 isHWO, DEV_UINT8 ioc,
					DEV_UINT8 bps, DEV_UINT8 zlp, DEV_UINT32 maxp)
{
 	TGPD* gpd;

 	if(dir == USB_TX) {
		gpd = Tx_gpd_end[ep_num];
		mu3d_hal_prepare_tx_gpd(gpd, buf, count, ep_num, IS_BDP, isHWO, ioc, bps, zlp);
	} else if(dir == USB_RX) {
		gpd = Rx_gpd_end[ep_num];
	 	mu3d_hal_prepare_rx_gpd(gpd, buf, count, ep_num, IS_BDP, isHWO, ioc, bps, maxp);
	}
}

/**
 * mu3d_hal_start_qmu - start qmu function (QMU flow : mu3d_hal_init_qmu ->mu3d_hal_start_qmu -> mu3d_hal_insert_transfer_gpd -> mu3d_hal_resume_qmu)
 * @args - arg1: ep number, arg2: dir
 */
void mu3d_hal_start_qmu(DEV_INT32 Q_num,  USB_DIR dir)
{
    DEV_UINT32 QCR;
    DEV_UINT32 txcsr;

	if (dir == USB_TX) {
		txcsr = USB_ReadCsr32(U3D_TX1CSR0, Q_num) & 0xFFFEFFFF;
		USB_WriteCsr32(U3D_TX1CSR0, Q_num, txcsr | TX_DMAREQEN);
		QCR = os_readl(U3D_QCR0);
		os_writel(U3D_QCR0, QCR|QMU_TX_CS_EN(Q_num));
#if (TXZLP==HW_MODE)
		QCR = os_readl(U3D_QCR1);
		os_writel(U3D_QCR1, QCR &~ QMU_TX_ZLP(Q_num));
		QCR = os_readl(U3D_QCR2);
		os_writel(U3D_QCR2, QCR|QMU_TX_ZLP(Q_num));
#elif (TXZLP==GPD_MODE)
		QCR = os_readl(U3D_QCR1);
		os_writel(U3D_QCR1, QCR|QMU_TX_ZLP(Q_num));
#endif
		os_writel(U3D_QEMIESR, os_readl(U3D_QEMIESR) | QMU_TX_EMPTY(Q_num));
		os_writel(U3D_TQERRIESR0, QMU_TX_LEN_ERR(Q_num)|QMU_TX_CS_ERR(Q_num));

		qmu_printk(K_INFO, "USB_QMU_TQCSR:0x%08X\n", os_readl(USB_QMU_TQCSR(Q_num)));

		if(os_readl(USB_QMU_TQCSR(Q_num))&QMU_Q_ACTIVE){
			qmu_printk(K_INFO, "Tx %d Active Now!\n", Q_num);
			return;
		}

		os_writel(USB_QMU_TQCSR(Q_num), QMU_Q_START);

		qmu_printk(K_INFO, "USB_QMU_TQCSR:0x%08X\n", os_readl(USB_QMU_TQCSR(Q_num)));
	} else if(dir == USB_RX) {
		USB_WriteCsr32(U3D_RX1CSR0, Q_num, USB_ReadCsr32(U3D_RX1CSR0, Q_num) |(RX_DMAREQEN));
		QCR = os_readl(U3D_QCR0);
		os_writel(U3D_QCR0, QCR|QMU_RX_CS_EN(Q_num));

#ifdef CFG_RX_ZLP_EN
		QCR = os_readl(U3D_QCR3);
		os_writel(U3D_QCR3, QCR|QMU_RX_ZLP(Q_num));
#else
		QCR = os_readl(U3D_QCR3);
		os_writel(U3D_QCR3, QCR&~(QMU_RX_ZLP(Q_num)));
#endif

#ifdef CFG_RX_COZ_EN
		QCR = os_readl(U3D_QCR3);
		os_writel(U3D_QCR3, QCR|QMU_RX_COZ(Q_num));
#else
		QCR = os_readl(U3D_QCR3);
		os_writel(U3D_QCR3, QCR&~(QMU_RX_COZ(Q_num)));
#endif

		os_writel(U3D_QEMIESR, os_readl(U3D_QEMIESR) | QMU_RX_EMPTY(Q_num));
		os_writel(U3D_RQERRIESR0, QMU_RX_LEN_ERR(Q_num)|QMU_RX_CS_ERR(Q_num));
		os_writel(U3D_RQERRIESR1, QMU_RX_EP_ERR(Q_num)|QMU_RX_ZLP_ERR(Q_num));

		qmu_printk(K_INFO, "USB_QMU_RQCSR:0x%08X\n", os_readl(USB_QMU_RQCSR(Q_num)));

		if(os_readl(USB_QMU_RQCSR(Q_num))&QMU_Q_ACTIVE){
			qmu_printk(K_INFO, "Rx %d Active Now!\n", Q_num);
			return;
		}

		os_writel(USB_QMU_RQCSR(Q_num), QMU_Q_START);

		qmu_printk(K_INFO, "USB_QMU_RQCSR:0x%08X\n", os_readl(USB_QMU_RQCSR(Q_num)));
    }

#if (CHECKSUM_TYPE==CS_16B)
	os_writel(U3D_QCR0, os_readl(U3D_QCR0)|CS16B_EN);
#else
	os_writel(U3D_QCR0, os_readl(U3D_QCR0)&~CS16B_EN);
#endif
}

/**
 * mu3d_hal_stop_qmu - stop qmu function (after qmu stop, fifo should be flushed)
 * @args - arg1: ep number, arg2: dir
 */
void mu3d_hal_stop_qmu(DEV_INT32 q_num,  USB_DIR dir)
{
	if (dir == USB_TX) {
		if(!(os_readl(USB_QMU_TQCSR(q_num)) & (QMU_Q_ACTIVE))) {
			qmu_printk(K_DEBUG, "Tx%d inActive Now!\n", q_num);
			return;
		}
		os_writel(USB_QMU_TQCSR(q_num), QMU_Q_STOP);
		while((os_readl(USB_QMU_TQCSR(q_num)) & (QMU_Q_ACTIVE)));
		qmu_printk(K_CRIT, "Tx%d stop Now!\n", q_num);
	} else if(dir == USB_RX) {
		if(!(os_readl(USB_QMU_RQCSR(q_num)) & QMU_Q_ACTIVE)) {
			qmu_printk(K_DEBUG, "Rx%d inActive Now!\n", q_num);
			return;
		}
		os_writel(USB_QMU_RQCSR(q_num), QMU_Q_STOP);
		while((os_readl(USB_QMU_RQCSR(q_num)) & (QMU_Q_ACTIVE)));
		qmu_printk(K_CRIT, "Rx%d stop now!\n", q_num);
	}
}

/**
 * mu3d_hal_send_stall - send stall
 * @args - arg1: ep number, arg2: dir
 */
void mu3d_hal_send_stall(DEV_INT32 q_num,  USB_DIR dir)
{
	if (dir == USB_TX) {
		USB_WriteCsr32(U3D_TX1CSR0, q_num, USB_ReadCsr32(U3D_TX1CSR0, q_num) | TX_SENDSTALL);
		while(!(USB_ReadCsr32(U3D_TX1CSR0, q_num) & TX_SENTSTALL));
		USB_WriteCsr32(U3D_TX1CSR0, q_num, USB_ReadCsr32(U3D_TX1CSR0, q_num) | TX_SENTSTALL);
		USB_WriteCsr32(U3D_TX1CSR0, q_num, USB_ReadCsr32(U3D_TX1CSR0, q_num) &~ TX_SENDSTALL);
	} else if(dir == USB_RX) {
		USB_WriteCsr32(U3D_RX1CSR0, q_num, USB_ReadCsr32(U3D_RX1CSR0, q_num) | RX_SENDSTALL);
		while(!(USB_ReadCsr32(U3D_RX1CSR0, q_num) & RX_SENTSTALL));
		USB_WriteCsr32(U3D_RX1CSR0, q_num, USB_ReadCsr32(U3D_RX1CSR0, q_num) | RX_SENTSTALL);
		USB_WriteCsr32(U3D_RX1CSR0, q_num, USB_ReadCsr32(U3D_RX1CSR0, q_num) &~ RX_SENDSTALL);
	}

	os_printk(K_CRIT,"%s %s-EP[%d] sent stall\n", __func__, ((dir==USB_TX)?"TX":"RX"), q_num);
}

/**
 * mu3d_hal_restart_qmu - clear toggle(or sequence) number and start qmu
 * @args - arg1: ep number, arg2: dir
 */
void mu3d_hal_restart_qmu(DEV_INT32 q_num,  USB_DIR dir)
{
	DEV_UINT32 ep_rst;

	qmu_printk(K_CRIT, "%s : Rest %s-EP[%d]\n", __func__, ((dir==USB_TX)?"TX":"RX"), q_num);

	if (dir == USB_TX) {
		ep_rst = BIT16<<q_num;
		os_writel(U3D_EP_RST, ep_rst);
		mdelay(1);
		os_writel(U3D_EP_RST, 0);
	} else {
		ep_rst = 1<<q_num;
		os_writel(U3D_EP_RST, ep_rst);
		mdelay(1);
		os_writel(U3D_EP_RST, 0);
	}
	mu3d_hal_start_qmu(q_num, dir);
}

/**
 * flush_qmu - stop qmu and align qmu start ptr t0 current ptr
 * @args - arg1: ep number, arg2: dir
 */
void mu3d_hal_flush_qmu(DEV_INT32 Q_num,  USB_DIR dir)
{
	TGPD* gpd_current;

	qmu_printk(K_CRIT, "%s flush QMU %s\n", __func__, ((dir==USB_TX)?"TX":"RX"));

	if (dir == USB_TX) {
		/*Stop QMU*/
		mu3d_hal_stop_qmu(Q_num, USB_TX);

		/*Get TX Queue Current Pointer Register*/
		gpd_current = (TGPD*)(os_readl(USB_QMU_TQCPR(Q_num)));

		/*If gpd_current = 0, it means QMU has not yet to execute GPD in QMU.*/
		if(!gpd_current){
			/*Get TX Queue Starting Address Register*/
			gpd_current = (TGPD*)(os_readl(USB_QMU_TQSAR(Q_num)));
		}

		/*Switch physical to virtual address*/
		qmu_printk(K_CRIT, "gpd_current(P) %p\n", gpd_current);
		gpd_current = phys_to_virt((unsigned long)gpd_current);
		qmu_printk(K_CRIT, "gpd_current(V) %p\n", (void *)gpd_current);

		/*Reset the TX GPD list state*/
		Tx_gpd_end[Q_num] = Tx_gpd_last[Q_num] = gpd_current;
		gpd_ptr_align(dir,Q_num,Tx_gpd_end[Q_num]);
		free_gpd(dir,Q_num);

		/*FIXME: Do not know why...*/
		os_writel(USB_QMU_TQSAR(Q_num), virt_to_phys(Tx_gpd_last[Q_num]));
		qmu_printk(K_ERR, "USB_QMU_TQSAR %x\n", os_readl(USB_QMU_TQSAR(Q_num)));
	} else if(dir == USB_RX) {
		/*Stop QMU*/
		mu3d_hal_stop_qmu(Q_num, USB_RX);

		/*Get RX Queue Current Pointer Register*/
		gpd_current = (TGPD*)(os_readl(USB_QMU_RQCPR(Q_num)));
		if(!gpd_current){
			/*Get RX Queue Starting Address Register*/
			gpd_current = (TGPD*)(os_readl(USB_QMU_RQSAR(Q_num)));
		}

		/*Switch physical to virtual address*/
		qmu_printk(K_CRIT, "gpd_current(P) %p\n", gpd_current);
		gpd_current = phys_to_virt((unsigned long)gpd_current);
		qmu_printk(K_CRIT, "gpd_current(V) %p\n", (void *)gpd_current);

		/*Reset the RX GPD list state*/
		Rx_gpd_end[Q_num] = Rx_gpd_last[Q_num] = gpd_current;
		gpd_ptr_align(dir,Q_num,Rx_gpd_end[Q_num]);
		free_gpd(dir,Q_num);

		/*FIXME: Do not know why...*/
		os_writel(USB_QMU_RQSAR(Q_num), virt_to_phys(Rx_gpd_end[Q_num]));
		qmu_printk(K_ERR, "USB_QMU_RQSAR %x\n", os_readl(USB_QMU_RQSAR(Q_num)));
	}
}

#endif
