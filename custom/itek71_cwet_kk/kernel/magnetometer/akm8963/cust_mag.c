#include <linux/types.h>
#include <mach/mt_pm_ldo.h>
#include <cust_mag.h>

#ifndef PMIC_APP_MAGNETOMETER_SENSOR_VDD
#define PMIC_APP_MAGNETOMETER_SENSOR_VDD MT65XX_POWER_NONE
#endif

#ifndef PMIC_APP_MAGNETOMETER_SENSOR_VIO
#define PMIC_APP_MAGNETOMETER_SENSOR_VIO MT65XX_POWER_NONE
#endif

static struct mag_hw cust_mag_hw = {
    .i2c_num = 1,
    .direction = 8,
    .power_id = PMIC_APP_MAGNETOMETER_SENSOR_VDD,  /*!< LDO is not used */
    .power_vol= VOL_DEFAULT,        /*!< LDO is not used */
    .power_vio_id = PMIC_APP_MAGNETOMETER_SENSOR_VIO,  /*!< LDO is not used */
    .power_vio_vol= VOL_DEFAULT,        /*!< LDO is not used */
};
struct mag_hw* get_cust_mag_hw(void) 
{
    return &cust_mag_hw;
}
