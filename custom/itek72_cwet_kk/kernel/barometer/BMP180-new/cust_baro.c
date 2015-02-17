#include <linux/types.h>
#include <cust_baro.h>
#include <mach/mt_pm_ldo.h>

/*---------------------------------------------------------------------------*/
int cust_baro_power(struct baro_hw *hw, unsigned int on, char* devname)
{
    if (hw->power_id == MT65XX_POWER_NONE)
        return 0;
    if (on)
        return hwPowerOn(hw->power_id, hw->power_vol, devname);
    else
        return hwPowerDown(hw->power_id, devname); 
}
/*---------------------------------------------------------------------------*/
static struct baro_hw cust_baro_hw = {
    .i2c_num = 1,
    .direction = 6,
    .power_id = MT65XX_POWER_NONE,  /*!< LDO is not used */
    .power_vol= VOL_DEFAULT,        /*!< LDO is not used */
    .firlen = 16,                   /*!< don't enable low pass fileter */
    .power = cust_baro_power,        
};
/*---------------------------------------------------------------------------*/
struct baro_hw* get_cust_baro_hw(void) 
{
    return &cust_baro_hw;
}
