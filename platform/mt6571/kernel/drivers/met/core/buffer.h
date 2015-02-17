#ifndef _BUFFER_H_
#define _BUFFER_H_

#include <asm/irq_regs.h>
#include <linux/types.h>

struct sample {
	struct list_head list;
	struct task_struct *task;
	unsigned long long stamp;
	unsigned long pc;
	int cpu;
	unsigned int pmu_value[7];
	unsigned char count;

	unsigned long cookie;
	off_t off;
};

void add_sample(struct pt_regs *regs, int cpu);
void sync_samples(int cpu);
void sync_all_samples(void);

/* #define MET_DEBUG_COOKIE */
#ifdef MET_DEBUG_COOKIE
#define dbg_cookie_tprintk(fmt, a...) \
		trace_printk("[DEBUG_COOKIE]%s, %d: "fmt, \
			__func__, __LINE__ , ##a)
#else				/* MET_DEBUG_COOKIE */
#define dbg_cookie_tprintk(fmt, a...) \
		no_printk(fmt, ##a)
#endif				/* MET_DEBUG_COOKIE */

#endif				/* _BUFFER_H_ */
