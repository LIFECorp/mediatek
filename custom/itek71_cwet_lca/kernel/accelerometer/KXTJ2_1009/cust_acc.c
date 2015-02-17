#include <linux/types.h>
#include <cust_acc.h>
#include <mach/mt_pm_ldo.h>

#ifndef PMIC_APP_ACCELEROMETER_SENSOR_VDD
#define PMIC_APP_ACCELEROMETER_SENSOR_VDD MT65XX_POWER_NONE
#endif

#ifndef PMIC_APP_ACCELEROMETER_SENSOR_VIO
#define PMIC_APP_ACCELEROMETER_SENSOR_VIO MT65XX_POWER_NONE
#endif

/*---------------------------------------------------------------------------*/
int cust_acc_power(struct acc_hw *hw, unsigned int on, char* devname)
{
    if (hw->power_id == MT65XX_POWER_NONE)
        return 0;
    if (on)
        return hwPowerOn(hw->power_id, hw->power_vol, devname);
    else
        return hwPowerDown(hw->power_id, devname); 
}
/*---------------------------------------------------------------------------*/
static struct acc_hw cust_acc_hw = {
    .i2c_num = 1,
    .direction = 6,
    .power_id = PMIC_APP_ACCELEROMETER_SENSOR_VDD,  /*!< LDO is not used */
    .power_vol= VOL_DEFAULT,        /*!< LDO is not used */
    .firlen = 16,                   /*!< don't enable low pass fileter */
    .power = cust_acc_power,        
    .power_vio_id = PMIC_APP_ACCELEROMETER_SENSOR_VIO,  /*!< LDO is not used */
    .power_vio_vol= VOL_DEFAULT,        /*!< LDO is not used */
};
/*---------------------------------------------------------------------------*/
struct acc_hw* get_cust_acc_hw(void) 
{
    return &cust_acc_hw;
}
