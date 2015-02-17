#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched.h>

void cpu_frequency(unsigned int frequency, unsigned int cpu_id)
{
	trace_printk("state=%d cpu_id=%d\n", frequency, cpu_id);
}
