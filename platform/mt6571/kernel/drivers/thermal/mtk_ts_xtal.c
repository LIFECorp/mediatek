#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/dmi.h>
#include <linux/acpi.h>
#include <linux/thermal.h>
#include <linux/platform_device.h>
#include <linux/aee.h>
#include <linux/xlog.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/proc_fs.h>

#include <mach/system.h>
#include "mach/mtk_thermal_monitor.h"
#include "mach/mt_typedefs.h"
#include "mach/mt_thermal.h"

#include <mach/upmu_common_sw.h>
#include <mach/upmu_hw.h>
#include <mach/mt_pmic_wrap.h>
#include <mach/pmic_mt6323_sw.h>
#include "mach/mtk_mdm_monitor.h"

#include <mach/mtk_ccci_helper.h>


#define MTK_THERMAL_XTAL_SOL

#define MTK_TZ_COOLER_MAX 10
#define PMIC_XTAL_TIMEOUT 300
#define MIN_TSX_TEMP (-40)
#define MAX_TSX_TEMP (+85)

#define ERROR_TSX_TIMEOUT  -2
#define ERROR_TSX_OVERTIME -1

struct mtkts_xtal_data {
    int temperature;
    s64 prev_time;
    s64 post_time;
};


/* Zone */
static struct thermal_zone_device *thz_dev;
static unsigned int interval = 0;/* seconds, 0 : no auto polling */
static unsigned int trip_temp[MTK_TZ_COOLER_MAX] = {120000,110000,100000,90000,80000,70000,65000,60000,55000,50000};
static int g_THERMAL_TRIP[MTK_TZ_COOLER_MAX] = {0,0,0,0,0,0,0,0,0,0};
static int num_trip=0;
static char g_bind[MTK_TZ_COOLER_MAX][THERMAL_NAME_LENGTH] = {{0},{0},{0},{0},{0},{0},{0},{0},{0},{0}};
static int kernelmode = 0;

#define mtktsxtal_TEMP_CRIT 150000 /* 150.000 degree Celsius */

/* Cooler */
//static unsigned int cl_dev_sysrst_state = 0;
//static struct thermal_cooling_device *cl_dev_sysrst;

static int mtktsxtal_pretemp = 0;
static bool mtktsxtal_resume_first = true;


#ifdef MTK_THERMAL_XTAL_SOL
static int ap_on_threshold = 210;
static int ap_off_diff = 10000;
static int mtktsxtal_ccci_count = 0;
static int mtktsxtal_ccci_trigger = 0;
static int mtktsxtal_slew = 0;
static s64  mtktsxtal_prev_time,mtktsxtal_measure_time;
#endif
//static int mtktsxtal_Tjtemp = 0;
//static s64 mtktsxtal_tj_time = 0;

/* Logging */
static int mtktsxtal_debug_log = 1;
#define mtktsxtal_dprintk(fmt, args...)   \
do {									\
	if (mtktsxtal_debug_log) {				\
		xlog_printk(ANDROID_LOG_INFO, "Power/XTAL_Thermal", fmt, ##args); \
	}								   \
} while(0)



static int mtktsxtal_register_thermal(void);
static void mtktsxtal_unregister_thermal(void);
static int mtkts_match(struct thermal_cooling_device *cdev, char bind[MTK_TZ_COOLER_MAX][THERMAL_NAME_LENGTH]);
extern int read_tbat_value(void);
extern void thermal_suspend_register(enum thermal_suspend_id id, thermal_suspend_cbk callbk);
extern int get_immediate_temp(void);
extern int read_tbat_value(void);

UINT32 u_table[126]={
9777,
9761,
9744,
9726,
9706,
9686,
9664,
9641,
9617,
9591,
9564,
9536,
9505,
9474,
9440,
9405,
9368,
9329,
9288,
9245,
9200,
9153,
9104,
9053,
8999,
8943,
8885,
8824,
8761,
8696,
8628,
8557,
8484,
8409,
8331,
8250,
8167,
8082,
7994,
7904,
7811,
7716,
7619,
7520,
7418,
7315,
7210,
7102,
6993,
6883,
6771,
6657,
6543,
6427,
6310,
6192,
6074,
5955,
5835,
5716,
5596,
5476,
5356,
5237,
5118,
5000,
4882,
4765,
4649,
4534,
4420,
4307,
4196,
4086,
3978,
3871,
3766,
3662,
3561,
3461,
3363,
3267,
3173,
3081,
2990,
2902,
2816,
2732,
2650,
2570,
2491,
2416,
2341,
2270,
2199,
2131,
2065,
2000,
1938,
1877,
1818,
1760,
1705,
1651,
1599,
1548,
1499,
1452,
1405,
1361,
1317,
1276,
1235,
1196,
1158,
1122,
1086,
1052,
1018,
986,
955,
925,
896,
868,
840,
814
};
	
static s64 _get_current_time_ms(void)
{
	s64 msecs;
	ktime_t now;

	now = ktime_get_boottime();
	msecs = ktime_to_ms(now);
	return msecs;
}

int binarysearch(int data[], int search, int n)
{
	int top = 0, bottom = n - 1;
	int count=0;
	while ((top <= bottom) && (count < 127))
	{
		int mid = (top + bottom) / 2;

		if (data[mid] > search)
		{
			top = mid + 1;
			if(data[top] <= search){
				return top-1;
			}
		}
		else if (data[mid] < search)
		{
			bottom = mid - 1;
			if(data[bottom] >= search){
				return bottom;
			}
		}
		else{
			return mid;
		}
		count++;
	}

	return -1;
}

static int TSX_U2T(int u, int *retval)
{
	int i;
	INT32 ret = 0;
	INT32 u_upper;
	INT32 u_low;
	INT32 t_upper;
	INT32 t_low;

	u = (u *10000)/65536;

	i = binarysearch(u_table, u, sizeof(u_table)/sizeof(UINT32));

	if(i == -1) return -1;
	
	u_upper =  u_table[i+1];
	u_low	=  u_table[i];

	t_upper =	MIN_TSX_TEMP + (i+1);
	t_low	=	MIN_TSX_TEMP + i;


	if((u_upper-u_low) == 0 )
	{
		*retval = (INT32)MIN_TSX_TEMP * 1000;
	}
	else
	{
		*retval = 1000*t_low + (1000*(t_upper-t_low)*(u_low - u))/(u_low - u_upper);
	}
	
	return ret;
}

static DEFINE_MUTEX(mtktsxtal);

#if 0
void mtkts_xtal_TJ_update(int temperature,s64 time)
{
	mtktsxtal_Tjtemp = temperature;
	mtktsxtal_tj_time = time;
	return;
}
#endif

void upmu_set_rg_clksq_en_aux_md_set(kal_uint32 val)
{
  kal_uint32 ret=0;

  ret=pmic_config_interface( (kal_uint32)(TOP_CKPDN1_SET),
							 (kal_uint32)(val),
							 (kal_uint32)(PMIC_RG_CLKSQ_EN_AUX_MD_MASK),
							 (kal_uint32)(PMIC_RG_CLKSQ_EN_AUX_MD_SHIFT)
							 );
}
void upmu_set_rg_clksq_en_aux_md_clr(kal_uint32 val)
{
  kal_uint32 ret=0;

  ret=pmic_config_interface( (kal_uint32)(TOP_CKPDN1_CLR),
							 (kal_uint32)(val),
							 (kal_uint32)(PMIC_RG_CLKSQ_EN_AUX_MD_MASK),
							 (kal_uint32)(PMIC_RG_CLKSQ_EN_AUX_MD_SHIFT)
							 );
}

static int mtktsxtal_get_now_temp(struct mtkts_xtal_data *t)
{
	INT32 t_ret=0, val1=0;
	INT32 val2 = -1;
	INT32 count = 0;
//	INT32 retry_count = 0;
	s64 prev_time,curr_time;

	mutex_lock(&mtktsxtal);
	upmu_set_rg_clksq_en_aux_md_set(1);
	upmu_set_rg_vref18_enb_md(0);

	prev_time = _get_current_time_ms();
	upmu_set_rg_md_rqst(0);
	upmu_set_rg_md_rqst(1);
	val1 = upmu_get_rg_adc_rdy_md();
	while(val1 == 0)
	{
		udelay(30);
		if((count++) > PMIC_XTAL_TIMEOUT)
		{
			printk("[mtktsxtal] xtal timeout\n");
			break;
		}
		val1 = upmu_get_rg_adc_rdy_md();
	}
	
	curr_time = _get_current_time_ms();
	
	if(val1 != 0)
	{
		val2 = TSX_U2T(upmu_get_rg_adc_out_md(), &t_ret);
	}


	
	upmu_set_rg_md_rqst(0);
	upmu_set_rg_clksq_en_aux_md_clr(1);
	upmu_set_rg_vref18_enb_md(1);
	mutex_unlock(&mtktsxtal);
	


	if(val2 == -1)
	{
		t->temperature = mtktsxtal_pretemp;
	}
	else
	{
		t->temperature = t_ret;	
	}

	t->prev_time = 	prev_time;	
	t->post_time = curr_time;
//	mtktsxtal_dprintk("[mtktsxtal_measure] temp,time1,time2 = %d %lld %lld\n",t->temperature,t->prev_time,t->post_time);

	if(val2 == -1)
	{
		return ERROR_TSX_TIMEOUT;
	}	
	
	if(curr_time - prev_time > 100)
	{
		return ERROR_TSX_OVERTIME;
	}
	
	return 0;
}

#ifdef MTK_THERMAL_XTAL_SOL	
static int mtktsxtal_get_hw_temp(void)
{
	int ret;
	struct mtkts_xtal_data val,final;
	s64 diff,min_diff = 0;
	int count = 0;
	int curr_temp;

	final.prev_time = mtktsxtal_prev_time;
	final.post_time = mtktsxtal_prev_time;
	final.temperature = mtktsxtal_pretemp;
	
	while(count < 10)
	{
		ret = mtktsxtal_get_now_temp(&val);		
		
		if(ret == 0)
		{
			final.prev_time = val.prev_time;
			final.post_time = val.post_time;
			final.temperature = val.temperature;
			break;
		}
		else if(ret == ERROR_TSX_OVERTIME)
		{		
			mtktsxtal_dprintk("[mtktsxtal] fail time = %d %lld %lld\n",val.temperature,val.prev_time,val.post_time);
		}
		else if(ret == ERROR_TSX_TIMEOUT)
		{
			mtktsxtal_dprintk("[mtktsxtal] fail temp = %d %lld %lld\n",val.temperature,val.prev_time,val.post_time);		
		}		
		
		diff = val.post_time-val.prev_time;		
		if((count == 0) || (diff < min_diff))
		{
			final.prev_time = val.prev_time;
			final.post_time = val.post_time;
			final.temperature = val.temperature;
			min_diff = diff;
		}
		count++;		
	}

	mtktsxtal_measure_time = ((final.post_time-final.prev_time) / 2) + final.prev_time;	
	curr_temp = get_immediate_temp();

	mtktsxtal_dprintk("[mtktsxtal_get_hw_temp] TXTAL,T1,T2,first = %d %lld %lld %d\n", 
		final.temperature,
		final.prev_time,
		final.post_time,
		mtktsxtal_resume_first);
	#if 0	
	mtktsxtal_dprintk("[mtktsxtal_get_hw_temp] TXTAL,TBAT,T1,T2,TABB,Tabbtime,first = %d %d %lld %lld %d %lld %d\n", 
		final.temperature
		,read_tbat_value()*1000
		,final.prev_time
		,final.post_time
		,mtktsxtal_Tjtemp,
		mtktsxtal_tj_time,
		mtktsxtal_resume_first);
	#endif
	return final.temperature;
}
#else
static int mtktsxtal_get_hw_temp(void)
{
	int ret;
	struct mtkts_xtal_data val;
	
	ret = mtktsxtal_get_now_temp(&val);				
	mtktsxtal_dprintk("[mtktsxtal_get_hw_temp] TXTAL= %d \n", 
		val.temperature);
	
	return val.temperature;
}

#endif

static int mtktsxtal_get_temp(struct thermal_zone_device *thermal,
				   unsigned long *t)
{
#ifdef MTK_THERMAL_XTAL_SOL	
	int ret_temp;
	int temp_slew = 0; 
	int diff_time = 0;
	unsigned int retbuf;
	
	ret_temp = mtktsxtal_get_hw_temp();

	if(mtktsxtal_resume_first == false)
	{	
		diff_time = (mtktsxtal_measure_time - mtktsxtal_prev_time);	
		temp_slew = (abs(ret_temp - mtktsxtal_pretemp) * 1000) / (diff_time);
		if(temp_slew > 0xFFFF) temp_slew =0xFFFF;
		retbuf = 2000;		 // short duration
		if(mtktsxtal_ccci_trigger > 0)
		{
			mtktsxtal_ccci_count++;		
			if(mtktsxtal_ccci_count >= mtktsxtal_ccci_trigger)
			{
				exec_ccci_kern_func_by_md_id(0, ID_SEND_TSX_TEMP, (char *)&retbuf, sizeof(retbuf));
				mtktsxtal_dprintk("mtktsxtal_slew_trigger = %d %d\n", temp_slew, (diff_time));
				mtktsxtal_ccci_count = 0;
			}
		}
		else
		{
			// check threshold 	 				
			if(temp_slew > ap_on_threshold)
			{
				mtktsxtal_dprintk("mtktsxtal_slew_trigger = %d %d\n", temp_slew, (diff_time));
				exec_ccci_kern_func_by_md_id(0, ID_SEND_TSX_TEMP, (char *)&retbuf, sizeof(retbuf));			
			}
		}
	}
	else
	{
		mtktsxtal_resume_first = false;
	}	
	
	mtktsxtal_prev_time = mtktsxtal_measure_time;
	mtktsxtal_pretemp = ret_temp;

	mtktsxtal_slew = temp_slew;
#else
	int ret_temp;
	ret_temp = mtktsxtal_get_hw_temp();
#endif
	*t = ret_temp ;	
	return 0;
}

static void mtktsxtal_suspend(void)
{
#ifdef MTK_THERMAL_XTAL_SOL	
	int temp_diff;
	unsigned int retbuf;

	if(mtktsxtal_resume_first == true) return;
	
	temp_diff = abs(mtktsxtal_pretemp - (read_tbat_value() * 1000));
	if(temp_diff > ap_off_diff)
	{
		if(temp_diff > 0xFFFF) temp_diff =0xFFFF;	
		
		retbuf = 20000;	// long duration
		mtktsxtal_dprintk("mtktsxtal_diff = %d\n", temp_diff);
		exec_ccci_kern_func_by_md_id(0, ID_SEND_TSX_TEMP, (char *)&retbuf, sizeof(retbuf));
	}
	mtktsxtal_resume_first = true;
#endif	
}

static int mtkts_match(struct thermal_cooling_device *cdev, char bind[MTK_TZ_COOLER_MAX][THERMAL_NAME_LENGTH])
{
	int i;
	
	for(i=0;i<MTK_TZ_COOLER_MAX;i++)
	{
		if(!strcmp(cdev->type, bind[i]))
		{
			return i;
		}
	}	
	return i;
}


static int mtktsxtal_bind(struct thermal_zone_device *thermal,
			struct thermal_cooling_device *cdev)
{
	int table_val=0;
	table_val = mtkts_match(cdev,g_bind);
	if(table_val >= MTK_TZ_COOLER_MAX ) 
	{
		return 0;
	}
	else
	{
		mtktsxtal_dprintk("[mtktsxtal_bind] %s\n", cdev->type);	
		if (mtk_thermal_zone_bind_cooling_device(thermal, table_val, cdev)) {
			mtktsxtal_dprintk("[mtktsxtal_bind] error binding cooling dev\n");
			return -EINVAL;
		} else {
			mtktsxtal_dprintk("[mtktsxtal_bind] binding OK, %d\n", table_val);
		}	
	}
	return 0;  
}

static int mtktsxtal_unbind(struct thermal_zone_device *thermal,
			  struct thermal_cooling_device *cdev)
{
	int table_val=0;
	table_val = mtkts_match(cdev,g_bind);
	if(table_val >= MTK_TZ_COOLER_MAX ) 
	{
		return 0;
	}
	else
	{
		mtktsxtal_dprintk("[mtktsxtal_unbind] %s\n", cdev->type); 
		if (thermal_zone_unbind_cooling_device(thermal, table_val, cdev)) {
			mtktsxtal_dprintk("[mtktsxtal_unbind] error unbinding cooling dev\n");
			return -EINVAL;
		} else {
			mtktsxtal_dprintk("[mtktsxtal_unbind] unbinding OK, %d\n", table_val);
		}	
	}
	return 0;  
}

static int mtktsxtal_get_mode(struct thermal_zone_device *thermal,
				enum thermal_device_mode *mode)
{
	*mode = (kernelmode) ? THERMAL_DEVICE_ENABLED
				 : THERMAL_DEVICE_DISABLED;
	return 0;
}

static int mtktsxtal_set_mode(struct thermal_zone_device *thermal,
				enum thermal_device_mode mode)
{
	kernelmode = mode;
	return 0;
}

static int mtktsxtal_get_trip_type(struct thermal_zone_device *thermal, int trip,
				 enum thermal_trip_type *type)
{
	*type = g_THERMAL_TRIP[trip];
	return 0;
}

static int mtktsxtal_get_trip_temp(struct thermal_zone_device *thermal, int trip,
				 unsigned long *temp)
{
	*temp = trip_temp[trip]; 
	return 0;
}

static int mtktsxtal_get_crit_temp(struct thermal_zone_device *thermal,
				 unsigned long *temperature)
{
	*temperature = mtktsxtal_TEMP_CRIT;
	return 0;
}

/* bind callback functions to thermalzone */
static struct thermal_zone_device_ops mtktsxtal_dev_ops = {
	.bind = mtktsxtal_bind,
	.unbind = mtktsxtal_unbind,
	.get_temp = mtktsxtal_get_temp,
	.get_mode = mtktsxtal_get_mode,
	.set_mode = mtktsxtal_set_mode,
	.get_trip_type = mtktsxtal_get_trip_type,
	.get_trip_temp = mtktsxtal_get_trip_temp,
	.get_crit_temp = mtktsxtal_get_crit_temp,
};
#if 0
static int sysrst_get_max_state(struct thermal_cooling_device *cdev,
				 unsigned long *state)
{		
	*state = 1;	
	return 0;
}
static int sysrst_get_cur_state(struct thermal_cooling_device *cdev,
				 unsigned long *state)
{		
	*state = cl_dev_sysrst_state;
	return 0;
}
static int sysrst_set_cur_state(struct thermal_cooling_device *cdev,
				 unsigned long state)
{
	cl_dev_sysrst_state = state;
	if(cl_dev_sysrst_state == 1)
	{
		printk("Power/PMIC_Thermal: reset, reset, reset!!!");
//		printk("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
//		printk("*****************************************");
//		printk("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
		BUG();
		//arch_reset(0,NULL);   
	}	
	return 0;
}

static struct thermal_cooling_device_ops mtktsxtal_cooling_sysrst_ops = {
	.get_max_state = sysrst_get_max_state,
	.get_cur_state = sysrst_get_cur_state,
	.set_cur_state = sysrst_set_cur_state,
};
#endif


static int mtktsxtal_read(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	int len = 0;
	char *p = buf;
	
	p+= sprintf(p, "[mtktsxtal_read]\n\
[trip_temp] = %d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n\
[trip_type] = %d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n\
[cool_bind] = %s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n\
time_ms=%d\n",
	trip_temp[0],trip_temp[1],trip_temp[2],trip_temp[3],trip_temp[4],
	trip_temp[5],trip_temp[6],trip_temp[7],trip_temp[8],trip_temp[9],
	g_THERMAL_TRIP[0],g_THERMAL_TRIP[1],g_THERMAL_TRIP[2],g_THERMAL_TRIP[3],g_THERMAL_TRIP[4],
	g_THERMAL_TRIP[5],g_THERMAL_TRIP[6],g_THERMAL_TRIP[7],g_THERMAL_TRIP[8],g_THERMAL_TRIP[9],
	g_bind[0],g_bind[1],g_bind[2],g_bind[3],g_bind[4],g_bind[5],g_bind[6],g_bind[7],g_bind[8],g_bind[9],
								interval*1000);
		
	*start = buf + off;
	
	len = p - buf;
	if (len > off)
		len -= off;
	else
		len = 0;
	
	return len < count ? len  : count;
}


static ssize_t mtktsxtal_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
	int len=0,time_msec=0;
	int trip[10]={0};
	int t_type[MTK_TZ_COOLER_MAX]={0};
	int i;
	char bind[MTK_TZ_COOLER_MAX][THERMAL_NAME_LENGTH];	
	char desc[512];

	len = (count < (sizeof(desc) - 1)) ? count : (sizeof(desc) - 1);
	if (copy_from_user(desc, buffer, len))
	{
		return 0;
	}
	desc[len] = '\0';
	
	if (sscanf(desc, "%d %d %d %s %d %d %s %d %d %s %d %d %s %d %d %s %d %d %s %d %d %s %d %d %s %d %d %s %d %d %s %d",
				&num_trip, 
				&trip[0],&t_type[0],bind[0], &trip[1],&t_type[1],bind[1],
				&trip[2],&t_type[2],bind[2], &trip[3],&t_type[3],bind[3],
				&trip[4],&t_type[4],bind[4], &trip[5],&t_type[5],bind[5],
				&trip[6],&t_type[6],bind[6], &trip[7],&t_type[7],bind[7],
				&trip[8],&t_type[8],bind[8], &trip[9],&t_type[9],bind[9],
				&time_msec) == 32)

	{
		mtktsxtal_dprintk("[mtktsxtal_write] unregister_thermal\n");
		mtktsxtal_unregister_thermal();
	
		for(i=0; i<MTK_TZ_COOLER_MAX; i++)
		{
			g_THERMAL_TRIP[i] = t_type[i];	
			memcpy(g_bind[i], bind[i], THERMAL_NAME_LENGTH); 		
			trip_temp[i]=trip[i];			
		}		
		interval=time_msec / 1000;
		interval = 1;

		mtktsxtal_dprintk("[mtktsxtal_write] [trip_type]=%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
				g_THERMAL_TRIP[0],g_THERMAL_TRIP[1],g_THERMAL_TRIP[2],g_THERMAL_TRIP[3],g_THERMAL_TRIP[4],
				g_THERMAL_TRIP[5],g_THERMAL_TRIP[6],g_THERMAL_TRIP[7],g_THERMAL_TRIP[8],g_THERMAL_TRIP[9]);
		
		mtktsxtal_dprintk("[mtktsxtal_write] [cool_bind]=%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n",
				g_bind[0],g_bind[1],g_bind[2],g_bind[3],g_bind[4],g_bind[5],g_bind[6],g_bind[7],g_bind[8],g_bind[9]);

		mtktsxtal_dprintk("[mtktsxtal_write] [trip_temp]==%d,%d,%d,%d,%d,%d,%d,%d,%d,%d, [time_ms]=%d\n", 
				trip_temp[0],trip_temp[1],trip_temp[2],trip_temp[3],trip_temp[4],
				trip_temp[5],trip_temp[6],trip_temp[7],trip_temp[8],trip_temp[9],interval*1000);
													
		mtktsxtal_dprintk("[mtktsxtal_write] register_thermal\n");
		mtktsxtal_resume_first = true;		
		mtktsxtal_register_thermal();
				
		return count;
	}
	else
	{
		mtktsxtal_dprintk("[mtktsxtal_write] bad argument\n");
	}
		
	return -EINVAL;
}

static int mtktsxtal_read_log(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	int len = 0;
	char *p = buf;

	p += sprintf(p, "[ mtktsxtal_read_log] log = %d\n",mtktsxtal_debug_log);

	*start = buf + off;

	len = p - buf;
	if (len > off)
		len -= off;
	else
		len = 0;

	return len < count ? len  : count;
}

static ssize_t mtktsxtal_write_log(struct file *file, const char *buffer, unsigned long count, void *data)
{
	char desc[32];
	int log_switch;
	int len = 0;

	len = (count < (sizeof(desc) - 1)) ? count : (sizeof(desc) - 1);
	if (copy_from_user(desc, buffer, len))
	{
		return 0;
	}
	desc[len] = '\0';

	if (sscanf(desc, "%d", &log_switch) == 1)
	{
		mtktsxtal_debug_log = log_switch;
		return count;
	}
	return -EINVAL;
}


#ifdef MTK_THERMAL_XTAL_SOL
static int mtktsxtal_read_trigger(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	int len = 0;
	char *p = buf;

	p += sprintf(p, "[ccci_trigger] = %d\n",mtktsxtal_ccci_trigger);

	*start = buf + off;

	len = p - buf;
	if (len > off)
		len -= off;
	else
		len = 0;

	return len < count ? len  : count;
}

static ssize_t mtktsxtal_write_trigger(struct file *file, const char *buffer, unsigned long count, void *data)
{
	char desc[32];
	int log_switch;
	int len = 0;

	len = (count < (sizeof(desc) - 1)) ? count : (sizeof(desc) - 1);
	if (copy_from_user(desc, buffer, len))
	{
		return 0;
	}
	desc[len] = '\0';

	if (sscanf(desc, "%d", &log_switch) == 1)
	{
		mtktsxtal_ccci_trigger = log_switch;
		mtktsxtal_ccci_count = 0;
		return count;
	}
	return -EINVAL;
}


static int mtktsxtal_read_threshold(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	int len = 0;
	char *p = buf;

	p += sprintf(p, "[ap_on_threshold] = %d\n",ap_on_threshold);

	*start = buf + off;

	len = p - buf;
	if (len > off)
		len -= off;
	else
		len = 0;

	return len < count ? len  : count;
}

static ssize_t mtktsxtal_write_threshold(struct file *file, const char *buffer, unsigned long count, void *data)
{
	char desc[32];
	int log_switch;
	int len = 0;

	len = (count < (sizeof(desc) - 1)) ? count : (sizeof(desc) - 1);
	if (copy_from_user(desc, buffer, len))
	{
		return 0;
	}
	desc[len] = '\0';

	if (sscanf(desc, "%d", &log_switch) == 1)
	{
		ap_on_threshold = log_switch;
		return count;
	}
	return -EINVAL;
}

static int mtktsxtal_read_diff(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	int len = 0;
	char *p = buf;

	p += sprintf(p, "[ap_off_diff] = %d\n",ap_off_diff);

	*start = buf + off;

	len = p - buf;
	if (len > off)
		len -= off;
	else
		len = 0;

	return len < count ? len  : count;
}

static ssize_t mtktsxtal_write_diff(struct file *file, const char *buffer, unsigned long count, void *data)
{
	char desc[32];
	int log_switch;
	int len = 0;

	len = (count < (sizeof(desc) - 1)) ? count : (sizeof(desc) - 1);
	if (copy_from_user(desc, buffer, len))
	{
		return 0;
	}
	desc[len] = '\0';

	if (sscanf(desc, "%d", &log_switch) == 1)
	{
		ap_off_diff = log_switch;
		return count;
	}
	return -EINVAL;
}


static int mtktsxtal_read_slew(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	int len = 0;
	char *p = buf;

	p += sprintf(p, "[mtktsxtal_slew] = %d\n",mtktsxtal_slew);

	*start = buf + off;

	len = p - buf;
	if (len > off)
		len -= off;
	else
		len = 0;

	return len < count ? len  : count;
}
#endif

static int mtktsxtal_register_cooler(void)
{
//	cl_dev_sysrst = mtk_thermal_cooling_device_register("mtktsxtal-sysrst", NULL,
//					   &mtktsxtal_cooling_sysrst_ops);		
   	return 0;
}

static int mtktsxtal_register_thermal(void)
{
	mtktsxtal_dprintk("[mtktsxtal_register_thermal] \n");

	if(interval !=0) {
	thz_dev = mtk_thermal_zone_device_register("mtktsxtal", num_trip, NULL,
					  &mtktsxtal_dev_ops, 0, 0, 0, interval*1000);
	}

	return 0;
}

static void mtktsxtal_unregister_cooler(void)
{
//	if (cl_dev_sysrst) {
//		mtk_thermal_cooling_device_unregister(cl_dev_sysrst);
//		cl_dev_sysrst = NULL;
//	}
}

static void mtktsxtal_unregister_thermal(void)
{
	mtktsxtal_dprintk("[mtktsxtal_unregister_thermal] \n");
	
	if (thz_dev) {
		mtk_thermal_zone_device_unregister(thz_dev);
		thz_dev = NULL;
	}
}

static int __init mtktsxtal_init(void)
{
	int err = 0;
	struct proc_dir_entry *entry = NULL;
	struct proc_dir_entry *mtktsxtal_dir = NULL;

	mtktsxtal_dprintk("[mtktsxtal_init] \n");

	err = mtktsxtal_register_cooler();
	if(err)
		return err;
	err = mtktsxtal_register_thermal();
	if (err)
		goto err_unreg;

	mtktsxtal_dir = proc_mkdir("mtktsxtal", NULL);
	if (!mtktsxtal_dir)
	{
		mtktsxtal_dprintk("[mtktsxtal_init]: mkdir /proc/mtktsxtal failed\n");
	}
	else
	{
		entry = create_proc_entry("mtktsxtal", S_IRUGO | S_IWUSR, mtktsxtal_dir);
		if (entry)
		{
			entry->read_proc = mtktsxtal_read;
			entry->write_proc = mtktsxtal_write;
		}

		entry = create_proc_entry("mtktsxtal_log", S_IRUGO | S_IWUSR, mtktsxtal_dir);
		if (entry)
		{
			entry->read_proc = mtktsxtal_read_log;
			entry->write_proc = mtktsxtal_write_log;
		}
#ifdef MTK_THERMAL_XTAL_SOL		
		entry = create_proc_entry("mtktsxtal_threshold", S_IRUGO | S_IWUSR, mtktsxtal_dir);
		if (entry)
		{
			entry->read_proc = mtktsxtal_read_threshold;
			entry->write_proc = mtktsxtal_write_threshold;
		}		
		entry = create_proc_entry("mtktsxtal_diff", S_IRUGO | S_IWUSR, mtktsxtal_dir);
		if (entry)
		{
			entry->read_proc = mtktsxtal_read_diff;
			entry->write_proc = mtktsxtal_write_diff;
		}		
		entry = create_proc_entry("mtktsxtal_trigger", S_IRUGO | S_IWUSR, mtktsxtal_dir);
		if (entry)
		{
			entry->read_proc = mtktsxtal_read_trigger;
			entry->write_proc = mtktsxtal_write_trigger;
		}			
		entry = create_proc_entry("mtktsxtal_slew", S_IRUGO, mtktsxtal_dir);
		if (entry)
		{
			entry->read_proc = mtktsxtal_read_slew;
		}			
#endif		
//		entry = create_proc_entry("mtktsxtal_enable", S_IRUGO | S_IWUSR, mtktsxtal_dir);
//		if (entry)
//		{
//			entry->read_proc = mtktsxtal_read_enable;
//			entry->write_proc = mtktsxtal_write_enable;
//		}			
	}
	thermal_suspend_register(THERMAL_XTAL, mtktsxtal_suspend);

	return 0;

err_unreg:
		mtktsxtal_unregister_cooler();
		return err;
}

static void __exit mtktsxtal_exit(void)
{
	mtktsxtal_dprintk("[mtktsxtal_exit] \n");
	mtktsxtal_unregister_thermal();
	mtktsxtal_unregister_cooler();
}

module_init(mtktsxtal_init);
module_exit(mtktsxtal_exit);

