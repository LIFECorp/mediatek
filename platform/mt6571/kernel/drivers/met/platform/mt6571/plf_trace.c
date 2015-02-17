#include <linux/kernel.h>
#include <linux/module.h>

#include "core/trace.h"

#define MP_2P_FMT	"%5lu.%06lu"
#define MP_2P_VAL	(unsigned long)(timestamp), nano_rem/1000
void mp_2p(unsigned long long timestamp, unsigned char cnt, unsigned int *value)
{
	unsigned long nano_rem = do_div(timestamp, 1000000000);
	switch (cnt) {
	case 1: trace_printk(MP_2P_FMT FMT1, MP_2P_VAL VAL1); break;
	case 2: trace_printk(MP_2P_FMT FMT2, MP_2P_VAL VAL2); break;
	case 3: trace_printk(MP_2P_FMT FMT3, MP_2P_VAL VAL3); break;
	case 4: trace_printk(MP_2P_FMT FMT4, MP_2P_VAL VAL4); break;
	case 5: trace_printk(MP_2P_FMT FMT5, MP_2P_VAL VAL5); break;
	case 6: trace_printk(MP_2P_FMT FMT6, MP_2P_VAL VAL6); break;
	case 7: trace_printk(MP_2P_FMT FMT7, MP_2P_VAL VAL7); break;
	case 8: trace_printk(MP_2P_FMT FMT8, MP_2P_VAL VAL8); break;
	case 9: trace_printk(MP_2P_FMT FMT9, MP_2P_VAL VAL9); break;
	}
}

void mp_2pr(unsigned long long timestamp, unsigned char cnt, unsigned int *value)
{
	unsigned long nano_rem = do_div(timestamp, 1000000000);
	switch (cnt) {
	case 1: trace_printk(MP_2P_FMT FMT1, MP_2P_VAL VAL1); break;
	case 2: trace_printk(MP_2P_FMT FMT2, MP_2P_VAL VAL2); break;
	case 3: trace_printk(MP_2P_FMT FMT3, MP_2P_VAL VAL3); break;
	case 4: trace_printk(MP_2P_FMT FMT4, MP_2P_VAL VAL4); break;
	case 5: trace_printk(MP_2P_FMT FMT5, MP_2P_VAL VAL5); break;
	case 6: trace_printk(MP_2P_FMT FMT6, MP_2P_VAL VAL6); break;
	case 7: trace_printk(MP_2P_FMT FMT7, MP_2P_VAL VAL7); break;
	case 8: trace_printk(MP_2P_FMT FMT8, MP_2P_VAL VAL8); break;
	case 9: trace_printk(MP_2P_FMT FMT9, MP_2P_VAL VAL9); break;
	}
}

void mp_2pw(unsigned long long timestamp, unsigned char cnt, unsigned int *value)
{
	unsigned long nano_rem = do_div(timestamp, 1000000000);
	switch (cnt) {
	case 1: trace_printk(MP_2P_FMT FMT1, MP_2P_VAL VAL1); break;
	case 2: trace_printk(MP_2P_FMT FMT2, MP_2P_VAL VAL2); break;
	case 3: trace_printk(MP_2P_FMT FMT3, MP_2P_VAL VAL3); break;
	case 4: trace_printk(MP_2P_FMT FMT4, MP_2P_VAL VAL4); break;
	case 5: trace_printk(MP_2P_FMT FMT5, MP_2P_VAL VAL5); break;
	case 6: trace_printk(MP_2P_FMT FMT6, MP_2P_VAL VAL6); break;
	case 7: trace_printk(MP_2P_FMT FMT7, MP_2P_VAL VAL7); break;
	case 8: trace_printk(MP_2P_FMT FMT8, MP_2P_VAL VAL8); break;
	case 9: trace_printk(MP_2P_FMT FMT9, MP_2P_VAL VAL9); break;
	}
}

#define MS_EMI_FMT	"%5lu.%06lu"
#define MS_EMI_VAL	(unsigned long)(timestamp), nano_rem/1000
void ms_emi(unsigned long long timestamp, unsigned char cnt, unsigned int *value)
{
	unsigned long nano_rem = do_div(timestamp, 1000000000);
	switch (cnt) {
	case 18: trace_printk(MS_EMI_FMT FMT18, MS_EMI_VAL VAL18); break;
	}
}

#define MS_SMI_FMT	"%5lu.%06lu"
#define MS_SMI_VAL	(unsigned long)(timestamp), nano_rem/1000
void ms_smi(unsigned long long timestamp, unsigned char cnt, unsigned int *value)
{
	unsigned long nano_rem = do_div(timestamp, 1000000000);
	switch (cnt) {
	case 18: trace_printk(MS_SMI_FMT FMT18, MS_SMI_VAL VAL18); break;
	case 30: trace_printk(MS_SMI_FMT FMT30, MS_SMI_VAL VAL30); break;
	case 34: trace_printk(MS_SMI_FMT FMT34, MS_SMI_VAL VAL34); break;
	case 37: trace_printk(MS_SMI_FMT FMT37, MS_SMI_VAL VAL37); break;
	case 44: trace_printk(MS_SMI_FMT FMT44, MS_SMI_VAL VAL44); break;
	case 50: trace_printk(MS_SMI_FMT FMT50, MS_SMI_VAL VAL50); break;
	case 66: trace_printk(MS_SMI_FMT FMT66, MS_SMI_VAL VAL66); break;
	case 82: trace_printk(MS_SMI_FMT FMT82, MS_SMI_VAL VAL82); break;
	}
}

void ms_smit(unsigned long long timestamp, unsigned char cnt, unsigned int *value)
{
	unsigned long nano_rem = do_div(timestamp, 1000000000);
	switch (cnt) {
	case 10: trace_printk(MS_SMI_FMT FMT10, MS_SMI_VAL VAL10); break;
	case 19: trace_printk(MS_SMI_FMT FMT19, MS_SMI_VAL VAL19); break;
	case 14: trace_printk(MS_SMI_FMT FMT14, MS_SMI_VAL VAL14); break;
	}
}

#define MS_TH_FMT	"%5lu.%06lu"
#define MS_TH_VAL	(unsigned long)(timestamp), nano_rem/1000
#define MS_TH_UD_FMT7	",%d,%d,%d,%d,%d,%d,%d\n"
#define MS_TH_UD_VAL7	,value[0],value[1],value[2],value[3],value[4],value[5],value[6]

void ms_th(unsigned long long timestamp, unsigned char cnt, unsigned int *value)
{
	unsigned long nano_rem = do_div(timestamp, 1000000000);
	switch (cnt) {
	case 7: trace_printk(MS_TH_FMT MS_TH_UD_FMT7, MS_TH_VAL MS_TH_UD_VAL7); break;
	}
}
