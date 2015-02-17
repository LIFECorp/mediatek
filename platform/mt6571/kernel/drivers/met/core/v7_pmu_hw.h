#ifndef _V7_PMU_HW_H_
#define _V7_PMU_HW_H_

#define ARMV7_PMCR_E		(1 << 0)	/* enable all counters */
#define ARMV7_PMCR_P		(1 << 1)
#define ARMV7_PMCR_C		(1 << 2)
#define ARMV7_PMCR_D		(1 << 3)
#define ARMV7_PMCR_X		(1 << 4)
#define ARMV7_PMCR_DP		(1 << 5)
#define ARMV7_PMCR_MASK		0x3f	/* mask for writable bits */

enum ARM_TYPE {
	CORTEX_A7 = 0xC07,
	CORTEX_A9 = 0xC09,
	CORTEX_A12 = 0xC0D,
	CORTEX_A15 = 0xC0F,
	CHIP_UNKNOWN = 0xFFF
};

enum ARM_TYPE armv7_get_ic(void);
int armv7_pmu_hw_get_counters(void);
void armv7_pmu_hw_start(struct met_pmu *pmu, int count);
void armv7_pmu_hw_stop(int count);
unsigned int armv7_pmu_hw_polling(struct met_pmu *pmu, int count, unsigned int *pmu_value);

#endif				/* _V7_PMU_HW_H_ */
