#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/delay.h>

#include "core/met_drv.h"
#include "core/trace.h"

#include "mt_typedefs.h"
//#include "mt_reg_base.h"
#include "mt_emi_bm.h"
#include "sync_write.h"
#include "plf_trace.h"

#define MET_EMI_SUCCESS 	 0
#define MET_EMI_FAIL		-1
#define EMI_NMASTER 		 6
#define EMI_NCOUNTER 		 5

#define DEF_BM_RW_TYPE      (BM_BOTH_READ_WRITE)
extern struct metdevice met_emi;
static struct kobject *kobj_emi = NULL;
static volatile int rwtype = BM_BOTH_READ_WRITE;
static int latency_master = 0;

static ssize_t rwtype_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static ssize_t rwtype_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t n);
static ssize_t latency_master_show(struct kobject *kobj,
					struct kobj_attribute *attr,
					char *buf);
static ssize_t emi_clock_rate_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static struct kobj_attribute rwtype_attr = __ATTR(rwtype, 0644, rwtype_show, rwtype_store);
static struct kobj_attribute latency_master_attr =
		__ATTR(latency_master, 0644, latency_master_show, NULL);
static struct kobj_attribute emi_clock_rate_attr = __ATTR_RO(emi_clock_rate);

extern unsigned int mt_get_emi_freq(void);
static ssize_t emi_clock_rate_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	int i;
	i = snprintf(buf, PAGE_SIZE, "%d\n", mt_get_emi_freq());
	return i;
}

static ssize_t rwtype_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	int i;
	i = snprintf(buf, PAGE_SIZE, "%d\n", rwtype);
	return i;
}

static ssize_t rwtype_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t n)
{
	int value;

	if ((n == 0) || (buf == NULL)) {
		return -EINVAL;
	}
	if (sscanf(buf, "%d", &value) != 1) {
		return -EINVAL;
	}
	if (value < 0 && value > BM_BOTH_READ_WRITE) {
		return -EINVAL;
	}
	rwtype = value;
	return n;
}

static ssize_t latency_master_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	int i;
	i = snprintf(buf, PAGE_SIZE, "%d\n", latency_master);
	return i;
}

static void emi_init(void)
{
	/* Init. EMI bus monitor */
	MET_BM_Init();

	/* Set R/W Type */
	MET_BM_SetReadWriteType(rwtype);

	/* Set Bandwidth Monitor */
	MET_BM_SetMonitorCounter(1,
			BM_MASTER_AP_MCU,
			BM_TRANS_TYPE_4BEAT |
			BM_TRANS_TYPE_8Byte |
			BM_TRANS_TYPE_BURST_WRAP);
	MET_BM_SetMonitorCounter(2,
			BM_MASTER_CONN_SYS,
			BM_TRANS_TYPE_4BEAT |
			BM_TRANS_TYPE_8Byte |
			BM_TRANS_TYPE_BURST_WRAP);
	MET_BM_SetMonitorCounter(3,
			BM_MASTER_MMSYS,
			BM_TRANS_TYPE_4BEAT |
			BM_TRANS_TYPE_8Byte |
			BM_TRANS_TYPE_BURST_WRAP);
	MET_BM_SetMonitorCounter(4,
			BM_MASTER_MD_MCU,
			BM_TRANS_TYPE_4BEAT |
			BM_TRANS_TYPE_8Byte |
			BM_TRANS_TYPE_BURST_WRAP);
	MET_BM_SetMonitorCounter(5,
			BM_MASTER_MD_HW,
			BM_TRANS_TYPE_4BEAT |
			BM_TRANS_TYPE_8Byte |
			BM_TRANS_TYPE_BURST_WRAP);

	/* Set Latency Monitor */
	MET_LM_SetMaster(latency_master);
}

static void emi_start(void)
{
	/* Enable Bandwidth Monitor */
	MET_BM_Enable(1);
	/* Enable Latency Monitor */
	MET_LM_Enable(1);
}

static void emi_stop(void)
{
	/* Disable Bandwidth Monitor */
	MET_BM_Enable(0);
	/* Disable Latency Monitor */
	MET_LM_Enable(0);
}

static int do_emi(void)
{
	return met_emi.mode;
}

static unsigned int emi_polling(unsigned int *emi_value)
{
	int i;
	int j = -1;

	MET_BM_Pause();

	// Get Bus Cycle Count
	emi_value[++j] = MET_BM_GetBusCycCount();

	/* To derive the bandwidth % usage (BSCT / BACT) */
	// Get Word Count
	for (i=0; i<EMI_NCOUNTER; i++) {
		emi_value[++j] = MET_BM_GetWordCount(i+1);
	}

	// Get Word Count for all masters
	emi_value[++j] = MET_BM_GetWordAllCount();

	// Get Bus Busy Count
	for (i=0; i<EMI_NCOUNTER; i++) {
		emi_value[++j] = MET_BM_GetBusBusyCount(i+1);
	}
	// Get Bus Busy Count for all masters
	emi_value[++j] = MET_BM_GetBusBusyAllCount();

	// Get Transaction Count
	emi_value[++j] = MET_BM_GetEMIClockCount();

	// Get Latency and Transaction
	emi_value[++j] = MET_LM_GetWTransCount();
	emi_value[++j] = MET_LM_GetWLatCount();
	emi_value[++j] = MET_LM_GetRTransCount();
	emi_value[++j] = MET_LM_GetRLatCount();

	// Disable
	MET_BM_Enable(0);
	MET_LM_Enable(0);
	// Enable
	MET_BM_Enable(1);
	MET_LM_Enable(1);

	return j+1;
}

static void emi_uninit(void)
{
	MET_BM_DeInit();
}

//static struct kobject *emi_parent;
static int met_emi_create(struct kobject *parent)
{
	int ret = 0;

	kobj_emi = parent;

	ret = sysfs_create_file(kobj_emi, &rwtype_attr.attr);
	if (ret != 0) {
		pr_err("Failed to create rwtype in sysfs\n");
		return ret;
	}
	ret = sysfs_create_file(kobj_emi, &latency_master_attr.attr);
	if (ret != 0) {
		pr_err("Failed to create latency_master in sysfs\n");
		return ret;
	}
	ret = sysfs_create_file(kobj_emi, &emi_clock_rate_attr.attr);
	if (ret != 0) {
		pr_err("Failed to create emi_clock_rate in sysfs\n");
		return ret;
	}
    return ret;
}


static void met_emi_delete(void)
{
	sysfs_remove_file(kobj_emi, &rwtype_attr.attr);
	sysfs_remove_file(kobj_emi, &latency_master_attr.attr);
	sysfs_remove_file(kobj_emi, &emi_clock_rate_attr.attr);
	kobj_emi = NULL;
}

static void met_emi_start(void)
{
	if (do_emi()) {
		emi_init();
		emi_stop();
		emi_start();
	}
}

static void met_emi_stop(void)
{
	if (do_emi()) {
		emi_stop();
		emi_uninit();
	}

}

static void met_emi_polling(unsigned long long stamp, int cpu)
{

	unsigned char count;
	unsigned int emi_value[18];

	if (do_emi()) {
		count = emi_polling(emi_value);
		if (count) {
			ms_emi(stamp, count, emi_value);
		}
	}
}

static char help[] =
"  --emi"
"                                 monitor EMI banwidth\n"
"  --emi=latency:master"
"                  monitor EMI latency\n";
static char header[] =
"met-info [000] 0.0: met_emi_clockrate: %d000\n"
"met-info [000] 0.0: met_emi_header: "
		"BUS_CYCLE,"
		"APMCU_WSCT,"
		"CONNSYS_WSCT,"
		"MMSYS_WSCT,"
		"MDMCU_WSCT,"
		"MDHW_WSCT,"
		"ALL_WACT,"
		"APMCU_BSCT,"
		"CONNSYS_BSCT,"
		"MMSYS_BSCT,"
		"MDMCU_BSCT,"
		"MDHW_BSCT,"
		"ALL_BACT,"
		"EMI_TSCT,"
		"W_TRANS,"
		"W_LATENCY,"
		"R_TRANS,"
		"R_LATENCY\n";

static int emi_print_help(char *buf, int len)
{
	return snprintf(buf, PAGE_SIZE, help);
}

static int emi_print_header(char *buf, int len)
{
	return snprintf(buf, PAGE_SIZE, header, mt_get_emi_freq());
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

static int check_master_valid(int master)
{
	if ((master < 0) ||
		(master >= EMI_NMASTER))
		return MET_EMI_FAIL;
	return MET_EMI_SUCCESS;
}

/*
 * There are serveral cases as follows:
 *
 * 1. "met-cmd --start --emi"
 *
 * 2. "met-cmd --start --emi=latency:master"
 *
 */

static int emi_process_argument(const char *arg, int len)
{
	int ret;

	if (len < 7)
		return -1;

	if ((strncmp(arg, "latency", 7) == 0) &&
		(arg[7] == ':') &&
		(len > 8)) {
		ret = get_num(&(arg[8]), &latency_master);
		if (ret == 0)
			return -1;
		if (check_master_valid(latency_master) != MET_EMI_SUCCESS)
			return -1;
		met_emi.mode = 2;
	} else {
		return -1;
	}

	return 0;
}

struct metdevice met_emi = {
	.name = "emi",
	.owner = THIS_MODULE,
	.type = MET_TYPE_BUS,
	.create_subfs = met_emi_create,
	.delete_subfs = met_emi_delete,
	.cpu_related = 0,
	.start = met_emi_start,
	.stop = met_emi_stop,
	.timed_polling = met_emi_polling,
	.print_help = emi_print_help,
	.print_header = emi_print_header,
	.process_argument = emi_process_argument,
};

