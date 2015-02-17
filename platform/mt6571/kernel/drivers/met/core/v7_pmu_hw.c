#include <asm/system.h>
#include <linux/smp.h>
#include "pmu.h"
#include "v7_pmu_hw.h"

enum ARM_TYPE armv7_get_ic(void)
{
	unsigned int value;
	/* Read Main ID Register */
	asm volatile ("mrc p15, 0, %0, c0, c0, 0":"=r" (value));

	value = (value & 0xffff) >> 4;	/* primary part number */
	return value;
}

static inline void armv7_pmu_counter_select(unsigned int idx)
{
	asm volatile ("mcr p15, 0, %0, c9, c12, 5" :  : "r" (idx));
	isb();
}

static inline void armv7_pmu_type_select(unsigned int idx, unsigned int type)
{
	armv7_pmu_counter_select(idx);
	asm volatile ("mcr p15, 0, %0, c9, c13, 1" :  : "r" (type));
}

static inline unsigned int armv7_pmu_read_count(unsigned int idx)
{
	unsigned int value;

	if (idx == 31) {
		asm volatile ("mrc p15, 0, %0, c9, c13, 0":"=r" (value));
	} else {
		armv7_pmu_counter_select(idx);
		asm volatile ("mrc p15, 0, %0, c9, c13, 2":"=r" (value));
	}
	return value;
}

static inline void armv7_pmu_write_count(int idx, u32 value)
{
	if (idx == 31) {
		asm volatile ("mcr p15, 0, %0, c9, c13, 0" :  : "r" (value));
	} else {
		armv7_pmu_counter_select(idx);
		asm volatile ("mcr p15, 0, %0, c9, c13, 2" :  : "r" (value));
	}
}

static inline void armv7_pmu_enable_count(unsigned int idx)
{
	asm volatile ("mcr p15, 0, %0, c9, c12, 1" :  : "r" (1 << idx));
}

static inline void armv7_pmu_disable_count(unsigned int idx)
{
	asm volatile ("mcr p15, 0, %0, c9, c12, 2" :  : "r" (1 << idx));
}

static inline void armv7_pmu_enable_intr(unsigned int idx)
{
	asm volatile ("mcr p15, 0, %0, c9, c14, 1" :  : "r" (1 << idx));
}

static inline void armv7_pmu_disable_intr(unsigned int idx)
{
	asm volatile ("mcr p15, 0, %0, c9, c14, 2" :  : "r" (1 << idx));
}

static inline unsigned int armv7_pmu_overflow(void)
{
	unsigned int val;
	asm volatile ("mrc p15, 0, %0, c9, c12, 3":"=r" (val));	/* read */
	asm volatile ("mcr p15, 0, %0, c9, c12, 3" :  : "r" (val));
	return val;
}

static inline unsigned int armv7_pmu_control_read(void)
{
	u32 val;
	asm volatile ("mrc p15, 0, %0, c9, c12, 0":"=r" (val));
	return val;
}

static inline void armv7_pmu_control_write(unsigned int val)
{
	val &= ARMV7_PMCR_MASK;
	isb();
	asm volatile ("mcr p15, 0, %0, c9, c12, 0" :  : "r" (val));
}

int armv7_pmu_hw_get_counters(void)
{
	int count = armv7_pmu_control_read();
	/* N, bits[15:11] */
	count = ((count >> 11) & 0x1f);
	return count;
}

static void armv7_pmu_hw_reset_all(int generic_counters)
{
	int i;
	armv7_pmu_control_write(ARMV7_PMCR_C | ARMV7_PMCR_P);
	/* generic counter */
	for (i = 0; i < generic_counters; i++) {
		armv7_pmu_disable_intr(i);
		armv7_pmu_disable_count(i);
	}
	/* cycle counter */
	armv7_pmu_disable_intr(31);
	armv7_pmu_disable_count(31);
	armv7_pmu_overflow();	/* clear overflow */
}

void armv7_pmu_hw_start(struct met_pmu *pmu, int count)
{
	int i;
	int generic = count - 1;

	armv7_pmu_hw_reset_all(generic);
	for (i = 0; i < generic; i++) {
		if (pmu[i].mode == MODE_POLLING) {
			armv7_pmu_type_select(i, pmu[i].event);
			armv7_pmu_enable_count(i);
		}
	}
	if (pmu[count - 1].mode == MODE_POLLING) {	/* cycle counter */
		armv7_pmu_enable_count(31);
	}
	armv7_pmu_control_write(ARMV7_PMCR_E);
}

void armv7_pmu_hw_stop(int count)
{
	int generic = count - 1;
	armv7_pmu_hw_reset_all(generic);
}

unsigned int armv7_pmu_hw_polling(struct met_pmu *pmu, int count, unsigned int *pmu_value)
{
	int i, cnt = 0;
	int generic = count - 1;

	for (i = 0; i < generic; i++) {
		if (pmu[i].mode == MODE_POLLING) {
			pmu_value[cnt] = armv7_pmu_read_count(i);
			cnt++;
		}
	}
	if (pmu[count - 1].mode == MODE_POLLING) {
		pmu_value[cnt] = armv7_pmu_read_count(31);
		cnt++;
	}
	armv7_pmu_control_write(ARMV7_PMCR_C | ARMV7_PMCR_P | ARMV7_PMCR_E);

	return cnt;
}
