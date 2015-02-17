#include <linux/module.h>
#include <linux/string.h>

#include "core/met_drv.h"
#include "core/trace.h"

//#include "mt_reg_base.h"
#include "mt_smi.h"
#include "sync_write.h"
#include "mt_typedefs.h"
#include "smi.h"
#include "smi_name.h"
#include "plf_trace.h"

//#define DEBUG_MET_SMI_TEST_PATTERN
#define MET_SMI_SUCCESS 0
#define MET_SMI_FAIL -1
#define MET_SMI_BUF_SIZE 128

extern struct metdevice met_smi;
static int enable_master_cnt = 0;

static int count = SMI_LARB_NUMBER + SMI_COMM_NUMBER;
static int portnum = SMI_ALLPORT_COUNT;

static struct kobject *kobj_smi = NULL;
static struct kobject *kobj_smi_mon_con = NULL;
static struct met_smi smi_larb[SMI_LARB_NUMBER];
static struct met_smi smi_comm[SMI_COMM_NUMBER];

static int toggle_idx = 0;
static int toggle_cnt = 1000;
static int toggle_master = 0;
static int toggle_master_min = -1;
static int toggle_master_max = -1;
static char err_msg[MET_SMI_BUF_SIZE];

static struct smi_cfg allport[SMI_ALLPORT_COUNT*4];
static SMIBMCfg_Ext monitorctrl;

struct chip_smi {
	unsigned int master;
	struct smi_desc *desc;
	unsigned int count;
};

static struct chip_smi smi_map[] ={
	{ 0, larb0_desc, SMI_LARB0_DESC_COUNT }, //larb0
	{ 1, common_desc, SMI_COMMON_DESC_COUNT } //common
};

static ssize_t requesttype_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%d\n", (int)monitorctrl.bRequestSelection);
}

static ssize_t requesttype_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t n)
{
	unsigned long requesttype;
	if (sscanf(buf, "%lu", &(requesttype)) != 1) {
		return -EINVAL;
	}
	monitorctrl.bRequestSelection = requesttype;
	return n;
}

static ssize_t toggle_cnt_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%d\n", toggle_cnt);
}

static ssize_t toggle_cnt_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t n)
{
	if (sscanf(buf, "%d", &(toggle_cnt)) != 1) {
			return -EINVAL;
	}
	return n;
}

static ssize_t count_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%d\n", count);
}

static ssize_t count_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t n)
{
	return -EINVAL;
}

static ssize_t portnum_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%d\n", portnum);
}

static ssize_t portnum_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t n)
{
	return -EINVAL;
}

static ssize_t err_msg_show(struct kobject *kobj,
				struct kobj_attribute *attr,
				char *buf)
{
	if (MET_SMI_BUF_SIZE < PAGE_SIZE)
		return snprintf(buf, MET_SMI_BUF_SIZE, "%s", err_msg);
	else
		return snprintf(buf, PAGE_SIZE, "%s", err_msg);
}

static struct kobj_attribute requesttype_attr = __ATTR(requesttype, 0644, requesttype_show, requesttype_store);
static struct kobj_attribute toggle_cnt_attr = __ATTR(toggle_cnt, 0644, toggle_cnt_show, toggle_cnt_store);
static struct kobj_attribute count_attr = __ATTR(count, 0644, count_show, count_store);
static struct kobj_attribute portnum_attr = __ATTR(portnum, 0644, portnum_show, portnum_store);
static struct kobj_attribute err_msg_attr = __ATTR(err_msg, 0644, err_msg_show, NULL);

static int do_smi(void)
{
	return met_smi.mode;
}

static void toggle_port(int toggle_idx)
{
	int i;
	i = allport[toggle_idx].master;
	if (i < SMI_LARB_NUMBER) {
		smi_larb[i].port = allport[toggle_idx].port;
		smi_larb[i].rwtype = allport[toggle_idx].rwtype;
		smi_larb[i].desttype = allport[toggle_idx].desttype;
		smi_larb[i].bustype = allport[toggle_idx].bustype;
		SMI_Disable(i);
		SMI_Clear(i);
		SMI_SetSMIBMCfg(i, smi_larb[i].port, smi_larb[i].desttype, smi_larb[i].rwtype);
		//SMI_SetMonitorControl(0);  //SMIBMCfgEx default
		SMI_SetMonitorControl(&monitorctrl);
		SMI_Enable(i, smi_larb[i].bustype);
	} else {
		i = i - SMI_LARB_NUMBER;
		smi_comm[i].port = allport[toggle_idx].port;
		smi_comm[i].rwtype = allport[toggle_idx].rwtype;
		smi_comm[i].desttype = allport[toggle_idx].desttype;
		smi_comm[i].bustype = allport[toggle_idx].bustype;
		SMI_Comm_Disable(i);
		SMI_Comm_Clear(i);
		SMI_SetCommBMCfg(i, smi_comm[i].port, smi_comm[i].desttype, smi_comm[i].rwtype);
		SMI_Comm_Enable(i);
	};
}

static void smi_init_value(void)
{
	int i;
	int m1=0,/*m2=0,m3=0,m4=0,m5=0,*/ m6=0;

	printk("smi_init_value\n");

	monitorctrl.bIdleSelection = 1;
	monitorctrl.uIdleOutStandingThresh = 3;
	monitorctrl.bDPSelection = 1;
	monitorctrl.bMaxPhaseSelection = 1;
	monitorctrl.bRequestSelection = SMI_REQ_ALL;
	monitorctrl.bStarvationEn = 1;
	monitorctrl.uStarvationTime = 8;

	for (i=0; i<SMI_LARB_NUMBER; i++) {
		smi_larb[i].mode = 0;
		smi_larb[i].master = i;
		smi_larb[i].port = 0;
		smi_larb[i].desttype = SMI_DEST_EMI;
		smi_larb[i].rwtype = SMI_RW_ALL;
		smi_larb[i].bustype = SMI_BUS_GMC;
	}
	for (i=0; i<SMI_COMM_NUMBER; i++) {
		smi_comm[i].mode = 0;
		smi_comm[i].master = SMI_LARB_NUMBER + i;
		//smi_comm[i].port = 8;	//GPU
		smi_comm[i].port = 0;	//GPU
		smi_comm[i].desttype = SMI_DEST_EMI;	//EMI
		smi_comm[i].rwtype = SMI_READ_ONLY;
		smi_comm[i].bustype = SMI_BUS_NONE;
	}

	for (i=0;i<SMI_ALLPORT_COUNT*4;i++) {
		if(i<SMI_LARB0_DESC_COUNT*4) {
			allport[i].master = 0;
			allport[i].port = (m1/4);
			allport[i].bustype = 0;
			allport[i].desttype = ((m1%2) ? 3 : 1);
			allport[i].rwtype = ((m1&0x2)? 2: 1);
			m1++;
/*		} else if(i<(SMI_LARB0_DESC_COUNT+SMI_LARB1_DESC_COUNT)*4) {
			allport[i].master = 1;
			allport[i].port = (m2/4);
			allport[i].bustype = 0;
			allport[i].desttype = ((m2%2) ? 3 : 1);
			allport[i].rwtype = ((m2&0x2)? 2: 1);
			m2++;
		} else if(i<(SMI_LARB0_DESC_COUNT+SMI_LARB1_DESC_COUNT+SMI_LARB2_DESC_COUNT)*4) {
			allport[i].master = 2;
			allport[i].port = (m3/4);
			allport[i].bustype = 0;
			allport[i].desttype = ((m3%2) ? 3 : 1);
			allport[i].rwtype = ((m3&0x2)? 2: 1);
			m3++;
		} else if(i<(SMI_LARB0_DESC_COUNT+SMI_LARB1_DESC_COUNT+SMI_LARB2_DESC_COUNT+SMI_LARB3_DESC_COUNT)*4) {
			allport[i].master = 3;
			allport[i].port = (m4/4);
			allport[i].bustype = 0;
			allport[i].desttype = ((m4%2) ? 3 : 1);
			allport[i].rwtype = ((m4&0x2)? 2: 1);
			m4++;
		} else if(i<(SMI_LARB0_DESC_COUNT+SMI_LARB1_DESC_COUNT+SMI_LARB2_DESC_COUNT+SMI_LARB3_DESC_COUNT+SMI_LARB4_DESC_COUNT)*4) {
			allport[i].master = 4;
			allport[i].port = (m5/4);
			allport[i].bustype = 0;
			allport[i].desttype = ((m5%2) ? 3 : 1);
			allport[i].rwtype = ((m5&0x2)? 2: 1);
			m5++;
*/		} else if(i<SMI_ALLPORT_COUNT*4) {
			//allport[i].master = 5;
			allport[i].master = 1;
			allport[i].port = (m6/4);
			allport[i].bustype = 1;
			allport[i].desttype = ((m6%2) ? 3 : 1);
			allport[i].rwtype = ((m6&0x2)? 2: 1);
			m6++;
		} else {
			printk("Error: SMI Index overbound");
		}
	}
}

static void smi_init(void)
{
	int i;
	SMI_Init();
	//SMI_PowerOn();

	if (do_smi() == 1) {
		for (i=0; i< SMI_LARB_NUMBER; i++) {
			SMI_Disable(i);
			SMI_SetSMIBMCfg(i, smi_larb[i].port, smi_larb[i].desttype, smi_larb[i].rwtype);
			if (smi_larb[i].mode == 1) {
				enable_master_cnt += 1;
			}
		}
		//SMI_SetMonitorControl(0);  //SMIBMCfgEx default
		SMI_SetMonitorControl(&monitorctrl);

		for (i=0; i< SMI_COMM_NUMBER; i++) {
			SMI_Comm_Disable(i);
			SMI_SetCommBMCfg(i, smi_comm[i].port, smi_comm[i].desttype, smi_comm[i].rwtype);
			if (smi_comm[i].mode == 1) {
				enable_master_cnt += 1;
			}
		}
	} else if (do_smi() == 2) {
		toggle_idx = 0;
		toggle_port(toggle_idx);
	} else if (do_smi() == 3) {
		toggle_master_max = toggle_master_min = -1;
		for (i=0; i<SMI_ALLPORT_COUNT*4; i++) {
			if (allport[i].master == toggle_master) {
				if (toggle_master_min == -1) {
					toggle_master_max = i;
					toggle_master_min = i;
				}
				if (i > toggle_master_max) {
					toggle_master_max = i;
				}
				if (i < toggle_master_min) {
					toggle_master_min = i;
				}
			}
		}
		printk("smi toggle min=%d, max=%d\n",toggle_master_min,toggle_master_max);
		if (toggle_master_min >=0 ) {
			toggle_idx = toggle_master_min;
			toggle_port(toggle_idx);
		}
	} else if (do_smi() == 4) {

	}
}

static void smi_start(void)
{
    int i;
    for (i=0; i< SMI_LARB_NUMBER; i++) {
        SMI_Enable(i, smi_larb[i].bustype);
    }
    for (i=0; i< SMI_COMM_NUMBER; i++) {
        SMI_Comm_Enable(i);
    }

}

static void smi_start_master(int i)
{
    if (i < SMI_LARB_NUMBER) {
        SMI_Enable(i, smi_larb[i].bustype);
    } else {
        SMI_Comm_Enable(i-SMI_LARB_NUMBER);
    }
}

static void smi_stop(void)
{
    int i;
    for (i=0; i< SMI_LARB_NUMBER; i++) {
        SMI_Clear(i);
    }
    for (i=0; i< SMI_COMM_NUMBER; i++) {
        SMI_Comm_Clear(i);
    }
}

static void smi_stop_master(int i)
{
    if (i < SMI_LARB_NUMBER) {
        SMI_Clear(i);
    } else {
        SMI_Comm_Clear(i-SMI_LARB_NUMBER);
    }
}


static unsigned int smi_polling(unsigned int *smi_value)
{
	int i=0,j=-1;

	//return 0;
	for (i=0; i<SMI_LARB_NUMBER; i++) {
		SMI_Pause(i);
	}

	for (i=0; i<SMI_COMM_NUMBER; i++) {
		SMI_Comm_Disable(i);
	}
#if 0
	smi_value[++j] = SMI_LARB_NUMBER+SMI_COMM_NUMBER;
	smi_value[++j] = 7;
	// read counter
	for (i=0; i<SMI_LARB_NUMBER; i++) {
		if (smi_larb[i].mode == 1) {
			smi_value[++j] = smi_larb[i].master;   //master
			smi_value[++j] = smi_larb[i].port;   //portNo
			smi_value[++j] = SMI_GetActiveCnt(i);    //ActiveCnt
			smi_value[++j] = SMI_GetRequestCnt(i);    //RequestCnt
			smi_value[++j] = SMI_GetIdleCnt(i);      //IdleCnt
			smi_value[++j] = SMI_GetBeatCnt(i);    //BeatCnt
			smi_value[++j] = SMI_GetByteCnt(i);    //ByteCnt
		}
	}
	for (i=0; i<SMI_COMM_NUMBER; i++) {
		if (smi_comm[i].mode == 1) {
			smi_value[++j] = SMI_LARB_NUMBER+i;   //fake master
			smi_value[++j] = smi_comm[i].port;   //portNo
			smi_value[++j] = SMI_Comm_GetActiveCnt(i);    //ActiveCnt
			smi_value[++j] = SMI_Comm_GetRequestCnt(i);    //RequestCnt
			smi_value[++j] = SMI_Comm_GetIdleCnt(i);      //IdleCnt
			smi_value[++j] = SMI_Comm_GetBeatCnt(i);    //BeatCnt
			smi_value[++j] = SMI_Comm_GetByteCnt(i);    //ByteCnt
		}
	}
#else
	smi_value[++j] = enable_master_cnt;
	smi_value[++j] = 16;
	// read counter
	for (i=0; i<SMI_LARB_NUMBER; i++) {
		if (smi_larb[i].mode == 1) {
			smi_value[++j] = smi_larb[i].master;   //master
			smi_value[++j] = smi_larb[i].port;   //portNo
			smi_value[++j] = SMI_GetActiveCnt(i);    //ActiveCnt
			smi_value[++j] = SMI_GetRequestCnt(i);    //RequestCnt
			smi_value[++j] = SMI_GetIdleCnt(i);      //IdleCnt
			smi_value[++j] = SMI_GetBeatCnt(i);    //BeatCnt
			smi_value[++j] = SMI_GetByteCnt(i);    //ByteCnt

			smi_value[++j] = SMI_GetCPCnt(i);    //CPCnt
			smi_value[++j] = SMI_GetDPCnt(i);    //DPCnt
			smi_value[++j] = SMI_GetCDP_MAX(i);    //CDP_MAX
			smi_value[++j] = SMI_GetCOS_MAX(i);    //COS_MAX
			smi_value[++j] = SMI_GetBUS_REQ0(i);    //BUS_REQ0
			smi_value[++j] = SMI_GetBUS_REQ1(i);    //BUS_REQ1
			smi_value[++j] = SMI_GetWDTCnt(i);    //WDTCnt
			smi_value[++j] = SMI_GetRDTCnt(i);    //RDTCnt
			smi_value[++j] = SMI_GetOSTCnt(i);    //OSTCnt
		}
	}
	for (i=0; i<SMI_COMM_NUMBER; i++) {
		if (smi_comm[i].mode == 1) {
			smi_value[++j] = SMI_LARB_NUMBER+i;   //fake master
			smi_value[++j] = smi_comm[i].port;   //portNo
			smi_value[++j] = SMI_Comm_GetActiveCnt(i);    //ActiveCnt
			smi_value[++j] = SMI_Comm_GetRequestCnt(i);    //RequestCnt
			smi_value[++j] = SMI_Comm_GetIdleCnt(i);      //IdleCnt
			smi_value[++j] = SMI_Comm_GetBeatCnt(i);    //BeatCnt
			smi_value[++j] = SMI_Comm_GetByteCnt(i);    //ByteCnt

			smi_value[++j] = SMI_Comm_GetCPCnt(i);    //CPCnt
			smi_value[++j] = SMI_Comm_GetDPCnt(i);    //DPCnt
			smi_value[++j] = SMI_Comm_GetCDP_MAX(i);    //CDP_MAX
			smi_value[++j] = SMI_Comm_GetCOS_MAX(i);    //COS_MAX
			smi_value[++j] = 0;    //BUS_REQ0
			smi_value[++j] = 0;    //BUS_REQ1
			smi_value[++j] = 0;    //WDTCnt
			smi_value[++j] = 0;    //RDTCnt
			smi_value[++j] = 0;    //OSTCnt
		}
	}
#endif

	smi_stop();
	smi_start();

	return j+1;
}

/*
static unsigned int smi_toggle_polling(unsigned int *smi_value)
{
	int j = -1;
	int i ;

	i = allport[toggle_idx].master;
	//return 0;
	if (allport[toggle_idx].master < SMI_LARB_NUMBER) {
		SMI_Pause(i);
		smi_value[++j] = allport[toggle_idx].master;   //larb master
		smi_value[++j] = allport[toggle_idx].port;   //portNo
		smi_value[++j] = allport[toggle_idx].rwtype;
		smi_value[++j] = allport[toggle_idx].desttype;
		smi_value[++j] = allport[toggle_idx].bustype;

		smi_value[++j] = SMI_GetActiveCnt(i);    //ActiveCnt
		smi_value[++j] = SMI_GetRequestCnt(i);    //RequestCnt
		smi_value[++j] = SMI_GetIdleCnt(i);      //IdleCnt
		smi_value[++j] = SMI_GetBeatCnt(i);    //BeatCnt
		smi_value[++j] = SMI_GetByteCnt(i);    //ByteCnt
	} else {
		i = allport[toggle_idx].master - SMI_LARB_NUMBER;
		SMI_Comm_Disable(i);
		smi_value[++j] = allport[toggle_idx].master;   //fake master
		smi_value[++j] = allport[toggle_idx].port;   //portNo
		smi_value[++j] = allport[toggle_idx].rwtype;
		smi_value[++j] = allport[toggle_idx].desttype;
		smi_value[++j] = allport[toggle_idx].bustype;

		smi_value[++j] = SMI_Comm_GetActiveCnt(i);    //ActiveCnt
		smi_value[++j] = SMI_Comm_GetRequestCnt(i);    //RequestCnt
		smi_value[++j] = SMI_Comm_GetIdleCnt(i);      //IdleCnt
		smi_value[++j] = SMI_Comm_GetBeatCnt(i);    //BeatCnt
		smi_value[++j] = SMI_Comm_GetByteCnt(i);    //ByteCnt
	}
	smi_stop_master(allport[toggle_idx].master);
	smi_start_master(allport[toggle_idx].master);
	return j+1;
}
*/

static unsigned int smi_dump_polling(unsigned int *smi_value)
{
	int j = -1;
	int i ;

	i = allport[toggle_idx].master;
	//return 0;
	if (allport[toggle_idx].master < SMI_LARB_NUMBER) {
		SMI_Pause(i);
		smi_value[++j] = allport[toggle_idx].master;   //larb master
		smi_value[++j] = allport[toggle_idx].port;   //portNo
		smi_value[++j] = allport[toggle_idx].rwtype;
		smi_value[++j] = allport[toggle_idx].desttype;
		smi_value[++j] = allport[toggle_idx].bustype;

		smi_value[++j] = SMI_GetActiveCnt(i);    //ActiveCnt
		smi_value[++j] = SMI_GetRequestCnt(i);    //RequestCnt
		smi_value[++j] = SMI_GetIdleCnt(i);      //IdleCnt
		smi_value[++j] = SMI_GetBeatCnt(i);    //BeatCnt
		smi_value[++j] = SMI_GetByteCnt(i);    //ByteCnt
		smi_value[++j] = SMI_GetCPCnt(i);    //CPCnt
		smi_value[++j] = SMI_GetDPCnt(i);    //DPCnt
		smi_value[++j] = SMI_GetCDP_MAX(i);    //CDP_MAX
		smi_value[++j] = SMI_GetCOS_MAX(i);    //COS_MAX
		smi_value[++j] = SMI_GetBUS_REQ0(i);    //BUS_REQ0
		smi_value[++j] = SMI_GetBUS_REQ1(i);    //BUS_REQ1
		smi_value[++j] = SMI_GetWDTCnt(i);    //WDTCnt
		smi_value[++j] = SMI_GetRDTCnt(i);    //RDTCnt
		smi_value[++j] = SMI_GetOSTCnt(i);    //OSTCnt

	} else {
		i = allport[toggle_idx].master - SMI_LARB_NUMBER;
		SMI_Comm_Disable(i);
		smi_value[++j] = allport[toggle_idx].master;   //fake master
		smi_value[++j] = allport[toggle_idx].port;   //portNo
		smi_value[++j] = allport[toggle_idx].rwtype;
		smi_value[++j] = allport[toggle_idx].desttype;
		smi_value[++j] = allport[toggle_idx].bustype;

		smi_value[++j] = SMI_Comm_GetActiveCnt(i);    //ActiveCnt
		smi_value[++j] = SMI_Comm_GetRequestCnt(i);    //RequestCnt
		smi_value[++j] = SMI_Comm_GetIdleCnt(i);      //IdleCnt
		smi_value[++j] = SMI_Comm_GetBeatCnt(i);    //BeatCnt
		smi_value[++j] = SMI_Comm_GetByteCnt(i);    //ByteCnt
		smi_value[++j] = SMI_Comm_GetCPCnt(i);    //CPCnt
		smi_value[++j] = SMI_Comm_GetDPCnt(i);    //DPCnt
		smi_value[++j] = SMI_Comm_GetCDP_MAX(i);    //CDP_MAX
		smi_value[++j] = SMI_Comm_GetCOS_MAX(i);    //COS_MAX
	}
	smi_stop_master(allport[toggle_idx].master);
	smi_start_master(allport[toggle_idx].master);

	return j+1;
}

static void smi_uninit(void)
{
	//SMI_DeInit();
	//SMI_PowerOff();
}

static int met_smi_create(struct kobject *parent)
{
	int ret = 0;
	char buf[16];

	smi_init_value();

	kobj_smi = parent;

	ret = sysfs_create_file(kobj_smi, &toggle_cnt_attr.attr);
	if (ret != 0) {
		pr_err("Failed to create toggle_cnt in sysfs\n");
		return ret;
	}

	ret = sysfs_create_file(kobj_smi, &count_attr.attr);
	if (ret != 0) {
		pr_err("Failed to create count in sysfs\n");
		return ret;
	}

	ret = sysfs_create_file(kobj_smi, &portnum_attr.attr);
	if (ret != 0) {
		pr_err("Failed to create portnum in sysfs\n");
		return ret;
	}

	snprintf(buf, sizeof(buf), "monitorctrl");
	kobj_smi_mon_con = kobject_create_and_add(buf, kobj_smi);
	if (NULL == kobj_smi_mon_con) {
		pr_err("Failed to create monitorctrl in sysfs\n");
	}

	ret = sysfs_create_file(kobj_smi_mon_con, &requesttype_attr.attr);
	if (ret != 0) {
		pr_err("Failed to create requesttype in sysfs\n");
		return ret;
	}

	ret = sysfs_create_file(kobj_smi, &err_msg_attr.attr);
	if (ret != 0) {
		pr_err("Failed to create err_msg in sysfs\n");
		return ret;
	}

	return ret;
}

void met_smi_delete(void)
{
	if (kobj_smi != NULL) {

		sysfs_remove_file(kobj_smi_mon_con, &requesttype_attr.attr);
		kobject_del(kobj_smi_mon_con);

		sysfs_remove_file(kobj_smi, &err_msg_attr.attr);
		sysfs_remove_file(kobj_smi, &toggle_cnt_attr.attr);
		sysfs_remove_file(kobj_smi, &count_attr.attr);
		sysfs_remove_file(kobj_smi, &portnum_attr.attr);
		kobj_smi = NULL;
	}
}

static void met_smi_start(void)
{
#ifdef DEBUG_MET_SMI_TEST_PATTERN
	int i;

	for (i=0; i<SMI_LARB_NUMBER; i++) {
		printk("master = %d, "
			"port = %d, "
			"rwtype = %d, "
			"desttype = %d, "
			"bustype = %d, "
			"mode = %d\n",
			smi_larb[i].master,
			smi_larb[i].port,
			smi_larb[i].rwtype,
			smi_larb[i].desttype,
			smi_larb[i].bustype,
			smi_larb[i].mode);
	}
	for (i=0; i<SMI_COMM_NUMBER; i++) {
		printk("master = %d, "
			"port = %d, "
			"rwtype = %d, "
			"desttype = %d, "
			"bustype = %d, "
			"mode = %d\n",
			smi_comm[i].master,
			smi_comm[i].port,
			smi_comm[i].rwtype,
			smi_comm[i].desttype,
			smi_comm[i].bustype,
			smi_comm[i].mode);
	}
	printk("requesttype = %d, "
		"toggle_cnt = %d, "
		"count = %d, "
		"portnum = %d\n",
		monitorctrl.bRequestSelection,
		toggle_cnt,
		count,
		portnum);
#else
	//printk("do smi mode=%d, toggle_idx=%d, idx220_port=%lu\n", do_smi(), toggle_idx, allport[220].port);
	if (do_smi()) {
        //printk("do smi\n");
		smi_init();
		smi_stop();
		smi_start();
	}
#endif
}

static void met_smi_stop(void)
{
	if (do_smi()) {
		smi_stop();
		smi_uninit();
	}
}

static void met_smi_polling(unsigned long long stamp, int cpu)
{
	unsigned char count=0;
	unsigned int smi_value[100];  //TODO: need re-check
	static int times=0;
	static int toggle_stop=0;

	if (do_smi() == 1) { //single port polling
		count = smi_polling(smi_value);
		//printk("smi_polling result count=%d\n",count);
		if (count) {
			ms_smi(stamp, count, smi_value);
		}
	} else if (toggle_stop == 0) {
		if (do_smi() == 2) { //all-toggling
			//count = smi_toggle_polling(smi_value);
			count = smi_dump_polling(smi_value);
			if (count) {
				//printk("smi_polling result count=%d\n",count);
				ms_smit(stamp, count, smi_value);
			}
			if (times == toggle_cnt) {//switch port
				toggle_idx = (toggle_idx + 1) % (SMI_ALLPORT_COUNT*4);
				//toggle_idx = toggle_idx + 1;
				//if (toggle_idx < SMI_ALLPORT_COUNT*4) {
					toggle_port(toggle_idx);
				//}
				/* else {
					toggle_stop = 1;	//stop smi polling
					return;
				} */
				times = 0;
			} else {
				times++;
			}
		} else if (do_smi() == 3) { //per-master toggling
			//count = smi_toggle_polling(smi_value);
			count = smi_dump_polling(smi_value);
			if (count) {
				//printk("smi_polling result count=%d\n",count);
				ms_smit(stamp, count, smi_value);
			}
			if (times == toggle_cnt) {//switch port
				toggle_idx = toggle_idx + 1;
				if (toggle_idx > toggle_master_max) {
					toggle_idx = toggle_master_min;
				}
				toggle_port(toggle_idx);
				times = 0;
			} else {
				times++;
			}
		} else if (do_smi() == 4) { //toggle all and stop
			count = smi_dump_polling(smi_value);
			if (count) {
				ms_smit(stamp, count, smi_value);
			}
			if (times == toggle_cnt) {//switch port
				//toggle_idx = (toggle_idx + 1) % (SMI_ALLPORT_COUNT*4);
				toggle_idx = toggle_idx + 1;
				if (toggle_idx < SMI_ALLPORT_COUNT*4) {
					toggle_port(toggle_idx);
				} else {
					toggle_stop = 1;	//stop smi polling
					return;
				}
				times = 0;
			} else {
				times++;
			}
		}
	}
}

static char help[] =
"  --smi=toggle                          "
"monitor all SMI port banwidth\n"
"  --smi=toggle:master                   "
"monitor one master's SMI port banwidth\n"
"  --smi=master:port:rw:dest:bus         "
"monitor specified SMI banwidth\n";
static char header_master_port_mode[] =
"# ms_smi: timestamp,set_num,element_num,master,port,active,request,idle,"
"beat,byte,CP,DP,CDP,COS,BUS_REQ0,BUS_REQ1,WDT,RDT,OST\n";
static char header_toggle_mode[] =
"# ms_smit: timestamp,master,port,rwtype,desttype,bustype,active,request,idle,"
"beat,byte,CP,DP,CDP,COS,BUS_REQ0,BUS_REQ1,WDT,RDT,OST\n";

static int smi_print_help(char *buf, int len)
{
	return snprintf(buf, PAGE_SIZE, help);
}

static int smi_print_header(char *buf, int len)
{
	int ret;

	if (met_smi.mode == 1)
		ret = snprintf(buf, PAGE_SIZE, header_master_port_mode);
	else
		ret = snprintf(buf, PAGE_SIZE, header_toggle_mode);

	met_smi.mode = 0;
	return ret;
}

static int get_num(const char *dc, int *pValue)
{
	int i = 0;
	int value = 0;
	int digit;

	while (((*dc) >= '0') && ((*dc) <= '9')) {
		digit = *dc - '0';
		value = 10*value + digit;
		dc++;
		i++;
	}

	if (i == 0)
		return 0;

	*pValue = value;
	return i;
}

static int check_master_vaild(int master)
{
	if ((master < 0) || (master >= (SMI_LARB_NUMBER + SMI_COMM_NUMBER)))
		return MET_SMI_FAIL;
	return MET_SMI_SUCCESS;
}

static int check_port_vaild(int master, int port)
{
	if (port < smi_map[master].count)
		return MET_SMI_SUCCESS;
	else
		return MET_SMI_FAIL;
}

static int assign_port(int master, int port)
{
	if ((master >= 0) && (master < SMI_LARB_NUMBER)) {
		smi_larb[master].port = port;
		return MET_SMI_SUCCESS;
	} else if ((master >= SMI_LARB_NUMBER) &&
			(master < (SMI_LARB_NUMBER + SMI_COMM_NUMBER))) {
		smi_comm[master-SMI_LARB_NUMBER].port = port;
		return MET_SMI_SUCCESS;
	} else {
		return MET_SMI_FAIL;
	}
}

static int check_rwtype_vaild(int master, int port, int rwtype)
{
	if (SMI_RW_ALL == smi_map[master].desc[port].rwtype) {
		if ((SMI_RW_ALL == rwtype) ||
			(SMI_READ_ONLY == rwtype) ||
			(SMI_WRITE_ONLY == rwtype))
			return MET_SMI_SUCCESS;
		else
			return MET_SMI_FAIL;
	} else if (SMI_RW_RESPECTIVE == smi_map[master].desc[port].rwtype) {
		if ((SMI_READ_ONLY == rwtype) ||
			(SMI_WRITE_ONLY == rwtype))
			return MET_SMI_SUCCESS;
		else
			return MET_SMI_FAIL;
	} else {
		return MET_SMI_FAIL;
	}
}

static int assign_rwtype(int master, int rwtype)
{
	if ((master >= 0) && (master < SMI_LARB_NUMBER)) {
		smi_larb[master].rwtype = rwtype;
		return MET_SMI_SUCCESS;
	} else if ((master >= SMI_LARB_NUMBER) &&
			(master < (SMI_LARB_NUMBER + SMI_COMM_NUMBER))) {
		smi_comm[master-SMI_LARB_NUMBER].rwtype = rwtype;
		return MET_SMI_SUCCESS;
	} else {
		return MET_SMI_FAIL;
	}

}

static int check_desttype_valid(int master, int port, int desttype)
{
	if ((SMI_DEST_NONE == smi_map[master].desc[port].desttype) ||
		(desttype == smi_map[master].desc[port].desttype))
		return MET_SMI_SUCCESS;
	else if (SMI_DEST_ALL == smi_map[master].desc[port].desttype) {
		if ((SMI_DEST_ALL == desttype) ||
			(SMI_DEST_EMI == desttype) ||
			(SMI_DEST_INTERNAL == desttype))
			return MET_SMI_SUCCESS;
		else
			return MET_SMI_FAIL;
	} else {
		return MET_SMI_FAIL;
	}
}

static int assign_desttype(int master, int desttype)
{
	if ((master >= 0) && (master < SMI_LARB_NUMBER)) {
		smi_larb[master].desttype = desttype;
		return MET_SMI_SUCCESS;
	} else if ((master >= SMI_LARB_NUMBER) &&
			(master < (SMI_LARB_NUMBER + SMI_COMM_NUMBER))) {
		smi_comm[master-SMI_LARB_NUMBER].desttype = desttype;
		return MET_SMI_SUCCESS;
	} else {
		return MET_SMI_FAIL;
	}
}

static int check_bustype_valid(int master, int port, int bustype)
{
	if ((SMI_BUS_NONE == smi_map[master].desc[port].bustype) ||
		(bustype == smi_map[master].desc[port].bustype))
		return MET_SMI_SUCCESS;
	else
		return MET_SMI_FAIL;
}

static int assign_bustype(int master, int bustype)
{
	if ((master >= 0) && (master < SMI_LARB_NUMBER)) {
		smi_larb[master].bustype = bustype;
		return MET_SMI_SUCCESS;
	} else if ((master >= SMI_LARB_NUMBER) &&
			(master < (SMI_LARB_NUMBER + SMI_COMM_NUMBER))) {
		smi_comm[master-SMI_LARB_NUMBER].bustype = bustype;
		return MET_SMI_SUCCESS;
	} else {
		return MET_SMI_FAIL;
	}
}

static int assign_mode(int master, int mode)
{
	if ((master >= 0) && (master < SMI_LARB_NUMBER)) {
		smi_larb[master].mode = mode;
		return MET_SMI_SUCCESS;
	} else if ((master >= SMI_LARB_NUMBER) &&
			(master < (SMI_LARB_NUMBER + SMI_COMM_NUMBER))) {
		smi_comm[master-SMI_LARB_NUMBER].mode = mode;
		return MET_SMI_SUCCESS;
	} else {
		return MET_SMI_FAIL;
	}
}

/*
 * There are serveal cases as follows:
 *
 * 1. "met-cmd --start --smi=toggle"
 *
 * 2. "met-cmd --start --smi=toggle:master"
 *
 * 3. "met-cmd --start --smi=master:port:rwtype:desttype:bustype"
 *
 * 4. "met-cmd --start --smi=dump"
 *
 */
static int smi_process_argument(const char *arg, int len)
{
	int master, port;
	int rwtype, desttype, bustype;
	int ret;
	int idx;

	if (len < 6)
		return -1;

	memset(err_msg, 0, MET_SMI_BUF_SIZE);

	/* --smi=toggle */
	if ((strncmp(arg, "toggle", 6) == 0) && (len == 6)) {
		if (met_smi.mode != 0)
			return -1;
		/* Set mode */
		met_smi.mode = 2;
	/* --smi=toggle:master */
	} else if ((strncmp(arg, "toggle", 6) == 0) &&
		arg[6] == ':' &&
		len > 7) {
		if (met_smi.mode != 0)
			return -1;
		ret = get_num(&(arg[7]), &toggle_master);
		if (ret == 0) {
			snprintf(err_msg,
				MET_SMI_BUF_SIZE,
				"Toggle master: can't get number [%s]\n",
				arg);
			return -1;
		}
		if (check_master_vaild(toggle_master) != MET_SMI_SUCCESS) {
			snprintf(err_msg,
				MET_SMI_BUF_SIZE,
				"Toggle master: check master failed [%s]\n",
				arg);
			return -1;
		}
		/* Set mode */
		met_smi.mode = 3;
	/* --smi=master:port:rwtype:desttype:bustype */
	} else if (len >= 9) {
		if ((met_smi.mode != 0) &&
			(met_smi.mode != 1))
			return -1;
		/* Initial variables */
		master = 0;
		port = 0;
		rwtype = 0;
		desttype = 0;
		bustype = 0;
		/* Get master */
		idx = 0;
		ret = get_num(&(arg[idx]), &master);
		if (ret == 0) {
			snprintf(err_msg,
				MET_SMI_BUF_SIZE,
				"Normal: can't get number [%s]\n",
				arg);
			return -1;
		}
		// Check master
		if (check_master_vaild(master) != MET_SMI_SUCCESS) {
			snprintf(err_msg,
				MET_SMI_BUF_SIZE,
				"Normal: check master failed [%s]\n",
				arg);
			return -1;
		}
		/* Get port */
		idx += ret + 1;
		ret = get_num(&(arg[idx]), &port);
		if (ret == 0) {
			snprintf(err_msg,
				MET_SMI_BUF_SIZE,
				"Normal: can't get number [%s]\n",
				arg);
			return -1;
		}
		// check port
		if (check_port_vaild(master, port) != MET_SMI_SUCCESS) {
			snprintf(err_msg,
				MET_SMI_BUF_SIZE,
				"Normal: check port failed [%s]\n",
				arg);
			return -1;
		}
		// assign port
		if (assign_port(master, port) != MET_SMI_SUCCESS) {
			snprintf(err_msg,
				MET_SMI_BUF_SIZE,
				"Normal: assign port failed [%s]\n",
				arg);
			return -1;
		}
		/* Get rwtype */
		idx += ret + 1;
		ret = get_num(&(arg[idx]), &rwtype);
		if (ret == 0) {
			snprintf(err_msg,
				MET_SMI_BUF_SIZE,
				"Normal: can't get number [%s]\n",
				arg);
			return -1;
		}
		// check rwtype
		if (check_rwtype_vaild(master, port, rwtype) !=
					MET_SMI_SUCCESS) {
			snprintf(err_msg,
				MET_SMI_BUF_SIZE,
				"Normal: check rwtype failed [%s]\n",
				arg);
			return -1;
		}
		// assign rwtype
		if (assign_rwtype(master, rwtype) != MET_SMI_SUCCESS) {
			snprintf(err_msg,
				MET_SMI_BUF_SIZE,
				"Normal: assign rwtype failed [%s]\n",
				arg);
			return -1;
		}
		/* Get desttype */
		idx += ret + 1;
		ret = get_num(&(arg[idx]), &desttype);
		if (ret == 0) {
			snprintf(err_msg,
				MET_SMI_BUF_SIZE,
				"Normal: can't get number [%s]\n",
				arg);
			return -1;
		}
		// check desttype
		if (check_desttype_valid(master, port, desttype) !=
					MET_SMI_SUCCESS) {
			snprintf(err_msg,
				MET_SMI_BUF_SIZE,
				"Normal: check desttype failed [%s]\n",
				arg);
			return -1;
		}
		// assign desttype
		if (assign_desttype(master, desttype) != MET_SMI_SUCCESS) {
			snprintf(err_msg,
				MET_SMI_BUF_SIZE,
				"Normal: assign desttype failed [%s]\n",
				arg);
			return -1;
		}
		/* Get bustype */
		idx += ret + 1;
		ret = get_num(&(arg[idx]), &bustype);
		if (ret == 0) {
			snprintf(err_msg,
				MET_SMI_BUF_SIZE,
				"Normal: can't get number [%s]\n",
				arg);
			return -1;
		}
		// check bustype
		if (check_bustype_valid(master, port, bustype) !=
					MET_SMI_SUCCESS) {
			snprintf(err_msg,
				MET_SMI_BUF_SIZE,
				"Normal: check bustype failed [%s]\n",
				arg);
			return -1;
		}
		// assign bustype
		if (assign_bustype(master, bustype) != MET_SMI_SUCCESS) {
			snprintf(err_msg,
				MET_SMI_BUF_SIZE,
				"Normal: assign bustype failed [%s]\n",
				arg);
			return -1;
		}
		// assign mode for each master TODO: need to re-check
		if (assign_mode(master, 1) != MET_SMI_SUCCESS) {
			snprintf(err_msg,
				MET_SMI_BUF_SIZE,
				"Normal: assign mode failed [%s]\n",
				arg);
			return -1;
		}
		/* Set mode */
		met_smi.mode = 1;
	/* --smi=dump */
	} else if (strncmp(arg, "dump", 4) == 0) {
		if (met_smi.mode != 0)
			return -1;
		/* Set mode */
		met_smi.mode = 4;
	} else {
		return -1;
	}

	return 0;
}

struct metdevice met_smi = {
	.name = "smi",
	.owner = THIS_MODULE,
	.type = MET_TYPE_BUS,
	.create_subfs = met_smi_create,
	.delete_subfs = met_smi_delete,
	.cpu_related = 0,
	.start = met_smi_start,
	.stop = met_smi_stop,
	.polling_interval = 0,
	.timed_polling = met_smi_polling,
	.print_help = smi_print_help,
	.print_header = smi_print_header,
	.process_argument = smi_process_argument,
};

