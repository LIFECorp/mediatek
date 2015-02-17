#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/random.h>
#include <linux/fs.h>

#include "core/met_drv.h"
extern struct metdevice met_dcm;
static int dcm_type = 0;
static struct delayed_work dwork;

#define FMT11	",%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n"
#define VAL11	,value[0],value[1],value[2],value[3],value[4],value[5],value[6],value[7],value[8],value[9],value[10]

#define SAMPLE_FMT	"%5lu.%06lu"
#define SAMPLE_VAL	(unsigned long)(timestamp), nano_rem/1000
void mt_dcm(unsigned long long timestamp, unsigned char cnt, unsigned int *value)
{
	unsigned long nano_rem = do_div(timestamp, 1000000000);

	switch (cnt) {
	case 11: trace_printk(SAMPLE_FMT FMT11, SAMPLE_VAL VAL11); break;
	}
}

void wq_get_sample(struct work_struct *work)
{
	int cpu;
	unsigned long long stamp;
	unsigned int value[11];
	//rnd_num[0]: Total High, rnd_num[1]: Total Low
	//rnd_num[2]: Good Duration, rnd_num[3]: Bad Duration
	unsigned char rnd_num[4];

	get_random_bytes(rnd_num, 4);

	cpu = smp_processor_id();
	stamp = cpu_clock(cpu);

	value[0] = rnd_num[0] + rnd_num[1];		//Total Time
	value[1] = rnd_num[0];					//Total High Time
	value[2] = rnd_num[0]/10;				//Longest High Time
	value[3] = rnd_num[2] + rnd_num[3];		//Count of Low to High
	value[4] = rnd_num[2];					//Count of Good Duration
	value[5] = 0;
	value[6] = 0;
	value[7] = 0;
	value[8] = 0;
	value[9] = (value[1]*100)/value[0];		//Total High Percentage
	value[10] = (value[2]*100)/value[0];	//Longest High Percentage
	mt_dcm(stamp, 11, value);
}

//It will be called back when run "met-cmd --start"
static void dcm_start(void)
{
	INIT_DELAYED_WORK(&dwork, wq_get_sample);
	return;
}

//It will be called back when run "met-cmd --stop"
static void dcm_stop(void)
{
	cancel_delayed_work_sync(&dwork);
	return;
}

//This callback function was called from timer. It cannot sleep or wait for
//others. Use wrokqueue to do the waiting
static void dcm_polling(unsigned long long stamp, int cpu)
{
	schedule_delayed_work(&dwork, 0);
}

static char header[] =
"#met-info [000] 0.0: mt_dcm:type=%d\n"
"met-info [000] 0.0: ms_ud_sys_header:mt_dcm,timestamp,"
"TT,TH,LH,L2H,GD, THO,LHO,L2HO,GDO, THP,LHP, d,d,d,d,d,d,d,d,d,d,d\n";
static char help[] = "  --dcm=type_id                         monitor dcm\n";


//It will be called back when run "met-cmd -h"
static int dcm_print_help(char *buf, int len)
{
	return snprintf(buf, PAGE_SIZE, help);
}

//It will be called back when run "met-cmd --extract" and mode is 1
static int dcm_print_header(char *buf, int len)
{
	met_dcm.mode = 0;
	return snprintf(buf, PAGE_SIZE, header, dcm_type);
}

//It will be called back when run "met-cmd --start --dcm aaa=1"
//and arg is "aaa=1"
static int dcm_process_argument(const char *arg, int len)
{
	int i, digit;
	printk("====DCM Argument(l=%d):%s\n", len, arg);

	dcm_type = 0;
	i = 0;
	while (((*arg)>='0')&&((*arg)<='9'))
	{
		digit = (*arg) - '0';
		dcm_type = dcm_type * 10 + digit;
		arg++;
		i++;
		if (i >= len)
			break;
	}
	if (i!=len) return -1;

	met_dcm.mode = 1;
	return 0;
}


struct metdevice met_dcm = {
	.name = "dcm",
	.owner = THIS_MODULE,
	.type = MET_TYPE_BUS,
	.cpu_related = 0,
	.start = dcm_start,
	.stop = dcm_stop,
	.polling_interval = 10,//ms
	.timed_polling = dcm_polling,
	.tagged_polling = dcm_polling,
	.print_help = dcm_print_help,
	.print_header = dcm_print_header,
	.process_argument = dcm_process_argument
};

EXPORT_SYMBOL(met_dcm);
