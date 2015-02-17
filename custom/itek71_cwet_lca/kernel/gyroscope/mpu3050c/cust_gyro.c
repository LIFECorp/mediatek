#include <linux/types.h>
#include <mach/mt_pm_ldo.h>
#include <cust_gyro.h>

#ifndef PMIC_APP_GYROSCOPE_VDD
#define PMIC_APP_GYROSCOPE_VDD MT65XX_POWER_NONE
#endif

#ifndef PMIC_APP_GYROSCOPE_VIO
#define PMIC_APP_GYROSCOPE_VIO MT65XX_POWER_NONE
#endif

/*---------------------------------------------------------------------------*/
/*
int cust_gyro_power(struct gyro_hw *hw, unsigned int on, char* devname)
{
    if (hw->power_id == MT65XX_POWER_NONE)
        return 0;
    if (on)
        return hwPowerOn(hw->power_id, hw->power_vol, devname);
    else
        return hwPowerDown(hw->power_id, devname); 
}
*/
/*---------------------------------------------------------------------------*/
static struct gyro_hw cust_gyro_hw = {
    .addr = 0xd0,
    .i2c_num = 1,
    .direction = 3,
    .power_id = PMIC_APP_GYROSCOPE_VDD,  /*!< LDO is not used */
    .power_vol= VOL_DEFAULT,        /*!< LDO is not used */
    .firlen = 16,                   /*!< don't enable low pass fileter */
    .power_vio_id = PMIC_APP_GYROSCOPE_VIO,  /*!< LDO is not used */
    .power_vio_vol= VOL_DEFAULT,        /*!< LDO is not used */
   // .power = cust_gyro_power,
};
/*---------------------------------------------------------------------------*/
struct gyro_hw* get_cust_gyro_hw(void) 
{
    return &cust_gyro_hw;
}
