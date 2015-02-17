#ifndef _SMI_NAME_H_
#define _SMI_NAME_H_

enum SMI_DEST {SMI_DEST_ALL=0, SMI_DEST_EMI=1, SMI_DEST_INTERNAL=3, SMI_DEST_NONE=9};
enum SMI_RW {SMI_RW_ALL=0, SMI_READ_ONLY=1, SMI_WRITE_ONLY=2, SMI_RW_RESPECTIVE=3, SMI_RW_NONE=9};
enum SMI_BUS {SMI_BUS_GMC=0, SMI_BUS_AXI=1, SMI_BUS_NONE=9};
enum SMI_REQUEST {SMI_REQ_ALL=0, SMI_REQ_ULTRA=1, SMI_REQ_PREULTRA=2, SMI_NORMAL_ULTRA=3, SMI_REQ_NONE=9};

/*
typedef struct {
	unsigned long u4Master;   //SMI master 0~3
	unsigned long u4PortNo;
	unsigned long bBusType : 1;//0 for GMC, 1 for AXI
	unsigned long bDestType : 2;//0 for EMI+internal mem, 1 for EMI, 3 for internal mem
	unsigned long bRWType : 2;//0 for R+W, 1 for read, 2 for write
}SMIBMCfg;
*/

struct smi_desc {
	unsigned long port;
	char name[40];
	enum SMI_DEST desttype;
	enum SMI_RW rwtype;
	enum SMI_BUS bustype;
	//enum SMI_REQUEST requesttype;
};

/* mt6571 */
struct smi_desc larb0_desc[] = {
	{ 0,	"DISP_OVL",		SMI_DEST_ALL, SMI_RW_ALL, SMI_BUS_GMC },
	{ 1,	"DISP_RDMA",		SMI_DEST_ALL, SMI_RW_ALL, SMI_BUS_GMC },
	{ 2,	"DISP_WDMA",		SMI_DEST_ALL, SMI_RW_ALL, SMI_BUS_GMC },
	{ 3,	"CAM_IMGO",		SMI_DEST_ALL, SMI_RW_ALL, SMI_BUS_GMC },
	{ 4,	"CAM_IMG2O",		SMI_DEST_ALL, SMI_RW_ALL, SMI_BUS_GMC },
	{ 5,	"CAM_LSCI",		SMI_DEST_ALL, SMI_RW_ALL, SMI_BUS_GMC },
	{ 6,	"MM_CMDQ",		SMI_DEST_ALL, SMI_RW_ALL, SMI_BUS_GMC },
	{ 7,	"VENC_BSDMA_VDEC_POST0",SMI_DEST_ALL, SMI_RW_ALL, SMI_BUS_GMC },
	{ 8,	"MDP_RDMA",		SMI_DEST_ALL, SMI_RW_ALL, SMI_BUS_GMC },
	{ 9,	"MDP_ROTO",		SMI_DEST_ALL, SMI_RW_ALL, SMI_BUS_GMC },
	{ 10,	"CAM_IMGI",		SMI_DEST_ALL, SMI_RW_ALL, SMI_BUS_GMC },
	{ 11,	"CAM_ESFKO",		SMI_DEST_ALL, SMI_RW_ALL, SMI_BUS_GMC },
	{ 12,	"CAM_AAO",		SMI_DEST_ALL, SMI_RW_ALL, SMI_BUS_GMC },
	{ 13,	"VENC_MVQP",		SMI_DEST_ALL, SMI_RW_ALL, SMI_BUS_GMC },
	{ 14,	"VENC_MC",		SMI_DEST_ALL, SMI_RW_ALL, SMI_BUS_GMC },
	{ 15,	"VENC_CDMA_VDEC_CDMA",	SMI_DEST_ALL, SMI_RW_ALL, SMI_BUS_GMC },
	{ 16,	"VENC_REC_VDEC_WDMA",	SMI_DEST_ALL, SMI_RW_ALL, SMI_BUS_GMC }
};

struct smi_desc common_desc[] = {
	{ 0,	"AXI_SMI_1",	SMI_DEST_EMI, SMI_RW_RESPECTIVE, SMI_BUS_NONE },
	{ 1,	"AXI_LARB0",	SMI_DEST_EMI, SMI_RW_RESPECTIVE, SMI_BUS_NONE },
	{ 2,	"AXI_LARB1",	SMI_DEST_EMI, SMI_RW_RESPECTIVE, SMI_BUS_NONE },
};

#define SMI_LARB0_DESC_COUNT (sizeof(larb0_desc) / sizeof(struct smi_desc))
#define SMI_COMMON_DESC_COUNT (sizeof(common_desc) / sizeof(struct smi_desc))
#define SMI_ALLPORT_COUNT (SMI_LARB0_DESC_COUNT+SMI_COMMON_DESC_COUNT)

#endif // _SMI_NAME_H_
