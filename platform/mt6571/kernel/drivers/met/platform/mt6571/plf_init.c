#include <linux/kernel.h>
#include <linux/module.h>

#include "core/met_drv.h"

static const char strTopology[] = "LITTLE:0,1";

extern struct metdevice met_emi;
extern struct metdevice met_smi;
//extern struct metdevice met_dcm;
extern struct metdevice met_thermal;
//extern struct metdevice met_pl310;
extern struct metdevice met_pmic;

#ifndef NO_MTK_LPM_HANDLER
#define NO_MTK_LPM_HANDLER 1
#endif
#if NO_MTK_LPM_HANDLER == 0
extern struct metdevice met_lpm_device;
#endif

#ifndef NO_MTK_GPU_HANDLER
#define NO_MTK_GPU_HANDLER 1
#endif
#if NO_MTK_GPU_HANDLER == 0
extern struct metdevice met_gpu;
#endif

#ifndef NO_MTK_MTCMOS_HANDLER
#define NO_MTK_MTCMOS_HANDLER 1
#endif
#if NO_MTK_MTCMOS_HANDLER == 0
extern struct metdevice met_mtcmos;
#endif


static int __init met_plf_init(void)
{
	met_register(&met_emi);
	met_register(&met_smi);
//	met_register(&met_dcm);
	met_register(&met_thermal);
	met_register(&met_pmic);
//	met_devlink_register_all();

#if NO_MTK_LPM_HANDLER == 0
	met_register(&met_lpm_device);
#endif

#if NO_MTK_GPU_HANDLER == 0
	met_register(&met_gpu);
#endif

#if NO_MTK_MTCMOS_HANDLER == 0
	met_register(&met_mtcmos);
#endif

	met_set_platform("mt6571", 1);
	met_set_topology(strTopology, 1);

	return 0;
}

static void __exit met_plf_exit(void)
{
//	met_devlink_deregister_all();
	met_deregister(&met_emi);
	met_deregister(&met_smi);
//	met_deregister(&met_dcm);
	met_deregister(&met_thermal);
	met_deregister(&met_pmic);

#if NO_MTK_LPM_HANDLER == 0
	met_deregister(&met_lpm_device);
#endif

#if NO_MTK_GPU_HANDLER == 0
	met_deregister(&met_gpu);
#endif

#if NO_MTK_MTCMOS_HANDLER == 0
	met_deregister(&met_mtcmos);
#endif

	met_set_platform(NULL, 0);
	met_set_topology(NULL, 0);
}

module_init(met_plf_init);
module_exit(met_plf_exit);
MODULE_AUTHOR("DT_DM5");
MODULE_DESCRIPTION("MET_MT6571");
MODULE_LICENSE("GPL");
