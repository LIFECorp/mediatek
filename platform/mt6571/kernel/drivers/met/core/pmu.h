#ifndef _PMU_H_
#define _PMU_H_

#include <linux/device.h>

struct met_pmu {
	unsigned char mode;
	unsigned short event;
	unsigned long freq;
	struct kobject *kobj_cpu_pmu;
};

#define MODE_DISABLED	0
#define MODE_INTERRUPT	1
#define MODE_POLLING	2

#endif				/* _PMU_H_ */
