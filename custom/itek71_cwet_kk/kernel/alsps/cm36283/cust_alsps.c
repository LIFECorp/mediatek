#include <linux/types.h>
#include <mach/mt_pm_ldo.h>
#include <cust_alsps.h>

#ifndef PMIC_APP_AMBIENT_LIGHT_SENSOR_VDD
#define PMIC_APP_AMBIENT_LIGHT_SENSOR_VDD MT65XX_POWER_NONE
#endif

#ifndef PMIC_APP_AMBIENT_LIGHT_SENSOR_VIO
#define PMIC_APP_AMBIENT_LIGHT_SENSOR_VIO MT65XX_POWER_NONE
#endif

#ifndef PMIC_APP_PROXIMITY_SENSOR_VDD
#define PMIC_APP_PROXIMITY_SENSOR_VDD MT65XX_POWER_NONE
#endif

#ifndef PMIC_APP_PROXIMITY_SENSOR_VIO
#define PMIC_APP_PROXIMITY_SENSOR_VIO MT65XX_POWER_NONE
#endif

#define CM63283_SENSITIVITY_6553_LUX    (0x0)
#define CM63283_SENSITIVITY_3277_LUX    (0x1)
#define CM63283_SENSITIVITY_1638_LUX    (0x2)
#define CM63283_SENSITIVITY_819_LUX     (0x3)

static struct alsps_hw cust_alsps_hw = {
    .i2c_num    = 1,
    .polling_mode_ps =0,
    .polling_mode_als =1,
    .power_id   = PMIC_APP_AMBIENT_LIGHT_SENSOR_VDD,    /*LDO is not used*/
    .power_vol  = VOL_DEFAULT,          /*LDO is not used*/
    //.i2c_addr   = {0x0C, 0x48, 0x78, 0x00},
    .als_level  = { 0,  1,  1,   7,  15,  15,  100, 1000, 2000,  3000,  6000, 10000, 14000, 18000, 20000},
    .als_value  = {40, 40, 90,  90, 160, 160,  225,  320,  640,  1280,  1280,  2600,  2600, 2600,  10240, 10240},
    .ps_threshold_high = 53,
    .ps_threshold_low = 46,
    .als_power_vio_id   = PMIC_APP_AMBIENT_LIGHT_SENSOR_VIO,    /*LDO is not used*/
    .als_power_vio_vol  = VOL_DEFAULT,          /*LDO is not used*/
    .ps_power_vdd_id   = PMIC_APP_PROXIMITY_SENSOR_VDD,    /*LDO is not used*/
    .ps_power_vdd_vol  = VOL_DEFAULT,          /*LDO is not used*/
    .ps_power_vio_id   = PMIC_APP_PROXIMITY_SENSOR_VIO,    /*LDO is not used*/
    .ps_power_vio_vol  = VOL_DEFAULT,          /*LDO is not used*/
};
struct alsps_hw *get_cust_alsps_hw(void) {
    return &cust_alsps_hw;
}

unsigned char cm36283_sensitivity = CM63283_SENSITIVITY_1638_LUX;

