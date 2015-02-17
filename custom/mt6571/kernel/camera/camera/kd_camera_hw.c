#include <linux/videodev2.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <asm/atomic.h>
#include <linux/xlog.h>
#include <linux/kernel.h>

#include "kd_camera_hw.h"

#include "kd_imgsensor.h"
#include "kd_imgsensor_define.h"
#include "kd_camera_feature.h"

/******************************************************************************
 * Debug configuration
******************************************************************************/
#define PFX "[kd_camera_hw]"
#define PK_DBG_NONE(fmt, arg...)    do {} while (0)
//#define PK_DBG_FUNC(fmt, arg...)    printk(KERN_INFO PFX "%s: " fmt, __FUNCTION__ ,##arg)
#define PK_DBG_FUNC printk

#define DEBUG_CAMERA_HW_K
#ifdef DEBUG_CAMERA_HW_K
#define PK_DBG PK_DBG_FUNC
#define PK_ERR(fmt, arg...)         printk(KERN_ERR PFX "%s: " fmt, __FUNCTION__ ,##arg)
#define PK_XLOG_INFO(fmt, args...) \
                do {    \
                    xlog_printk(ANDROID_LOG_INFO, "kd_camera_hw", fmt, ##args); \
                } while(0)
#else
#define PK_DBG(a,...)
#define PK_ERR(a,...)
#define PK_XLOG_INFO(fmt, args...)
#endif


int kdCISModulePowerOn(CAMERA_DUAL_CAMERA_SENSOR_ENUM SensorIdx, char *currSensorName, BOOL On, char* mode_name)
{
    u32 pinSetIdx = 0;  //default main sensor
    
#define IDX_PS_CMRST 0
#define IDX_PS_CMPDN 4

#define IDX_PS_MODE 1
#define IDX_PS_ON   2
#define IDX_PS_OFF  3  
  
    u32 pinSet[2][8] = {
                  //for main sensor
                  {GPIO_CAMERA_CMRST_PIN,
                      GPIO_CAMERA_CMRST_PIN_M_GPIO,   /* mode */
                      GPIO_OUT_ONE,                   /* ON state */
                      GPIO_OUT_ZERO,                  /* OFF state */
                   GPIO_CAMERA_CMPDN_PIN,
                      GPIO_CAMERA_CMPDN_PIN_M_GPIO,
                      GPIO_OUT_ONE,
                      GPIO_OUT_ZERO,
                  },
                  //for sub sensor
                  {GPIO_CAMERA_2_CMRST_PIN,
                      GPIO_CAMERA_2_CMRST_PIN_M_GPIO,
                      GPIO_OUT_ONE,
                      GPIO_OUT_ZERO,
                   GPIO_CAMERA_2_CMPDN_PIN,
                      GPIO_CAMERA_2_CMPDN_PIN_M_GPIO,
                      GPIO_OUT_ONE,
                      GPIO_OUT_ZERO,
                  }
                 };

    if (DUAL_CAMERA_MAIN_SENSOR == SensorIdx){
        pinSetIdx = 0;
    }
    else if (DUAL_CAMERA_SUB_SENSOR == SensorIdx) {
        pinSetIdx = 1;
    }

    //power ON
    if (On) {

        PK_DBG("kdCISModulePowerOn -on:currSensorName=%s\n",currSensorName);
        PK_DBG("kdCISModulePowerOn -on:pinSetIdx=%d\n",pinSetIdx);
       
        if (currSensorName && (0 == strcmp(SENSOR_DRVNAME_OV5648_MIPI_RAW, currSensorName)))
        {
            //OV5648 Power UP
            //First Power Pin low and Reset Pin Low
            if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST]) {
                //PDN pin
                if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN], pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}
                if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN], GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}
                if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN], pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_OFF])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");}
            
                //Reset pin
                if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMRST], pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! \n");}
                if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMRST], GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! \n");}
                if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST], pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");}
            }        
            
            if(TRUE != hwPowerOn(PMIC_APP_MAIN_CAMERA_POWER_IO, VOL_1800, mode_name))
            {
                PK_DBG("[CAMERA SENSOR] Fail to enable IO power\n");
                goto _kdCISModulePowerOn_exit_;
            }
            
            if(TRUE != hwPowerOn(PMIC_APP_MAIN_CAMERA_POWER_A, VOL_2800, mode_name))
            {
                PK_DBG("[CAMERA SENSOR] Fail to enable analog power\n");
                goto _kdCISModulePowerOn_exit_;
            }
            
            if(TRUE != hwPowerOn(PMIC_APP_MAIN_CAMERA_POWER_D, VOL_1500,mode_name))
            {
                 PK_DBG("[CAMERA SENSOR] Fail to enable digital power\n");
                 goto _kdCISModulePowerOn_exit_;
            } 
            
            if(TRUE != hwPowerOn(PMIC_APP_MAIN_CAMERA_POWER_AF, VOL_2800,mode_name))
            {
                PK_DBG("[CAMERA SENSOR] Fail to enable AF power\n");
                goto _kdCISModulePowerOn_exit_;
            }
            
            // wait power to be stable 
            mdelay(5);
            
            //enable active sensor
            if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST]) {
                //PDN pin
                if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN], pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}
                if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN], GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}
                if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN], pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_ON])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");}
            
                //RST pin
                if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMRST], pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! \n");}
                if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMRST], GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! \n");}
                if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST], pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_ON])){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");}
            }
            
            msleep(20);
        }
        else if (currSensorName && (0 == strcmp(SENSOR_DRVNAME_OV9726_MIPI_RAW,currSensorName)))
		{
			if(TRUE != hwPowerOn(PMIC_APP_SUB_CAMERA_POWER_D, VOL_1500, mode_name))
			{
				 PK_DBG("[CAMERA SENSOR] Fail to enable digital power\n");
				 goto _kdCISModulePowerOn_exit_;
			}
			
			if(TRUE != hwPowerOn(PMIC_APP_SUB_CAMERA_POWER_IO, VOL_1800,mode_name))
			{
				PK_DBG("[CAMERA SENSOR] Fail to enable IO power\n");
				goto _kdCISModulePowerOn_exit_;
			}

			if(TRUE != hwPowerOn(PMIC_APP_SUB_CAMERA_POWER_A, VOL_2800,mode_name))
			{
				PK_DBG("[CAMERA SENSOR] Fail to enable analog power\n");
				goto _kdCISModulePowerOn_exit_;
			}
			
			msleep(3);

			//PDN/STBY pin
			if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST])
			{
				if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! \n");}
				if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! \n");}
				if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");}
				mdelay(10);
				if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_ON])){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");}
				mdelay(1);
			
				if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}
				if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}
				if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_ON])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");}
				msleep(10);
				if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_OFF])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");}
				msleep(1);
			}
	    }
    }
    else {//power OFF
       
        PK_DBG("kdCISModulePowerOn -off:currSensorName=%s\n", currSensorName);
        
        if(currSensorName && (0 == strcmp(SENSOR_DRVNAME_OV5648_MIPI_RAW,currSensorName)))
        {
            //PK_DBG("[OFF]sensorIdx:%d \n",SensorIdx);
            if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST]) {
                if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! \n");}
                if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}
                if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! \n");}
                if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}
                if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");} //low == reset sensor
                if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_OFF])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");} //high == power down lens module
            }
            
            if(TRUE != hwPowerDown(PMIC_APP_MAIN_CAMERA_POWER_A,mode_name)) {
                PK_DBG("[CAMERA SENSOR] Fail to OFF analog power\n");
                goto _kdCISModulePowerOn_exit_;
            }
            
            if(TRUE != hwPowerDown(PMIC_APP_MAIN_CAMERA_POWER_AF,mode_name))
            {
                PK_DBG("[CAMERA SENSOR] Fail to OFF AF power\n");
                goto _kdCISModulePowerOn_exit_;
            }
            
            if(TRUE != hwPowerDown(PMIC_APP_MAIN_CAMERA_POWER_D, mode_name)) {
                PK_DBG("[CAMERA SENSOR] Fail to OFF digital power\n");
                goto _kdCISModulePowerOn_exit_;
            }
            
            if(TRUE != hwPowerDown(PMIC_APP_MAIN_CAMERA_POWER_IO,mode_name))
            {
                PK_DBG("[CAMERA SENSOR] Fail to OFF IO power\n");
                goto _kdCISModulePowerOn_exit_;
            }
        }
        else if(currSensorName && (0 == strcmp(SENSOR_DRVNAME_OV9726_MIPI_RAW,currSensorName)))
		{
            //reset pull down
			if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST]) {
				if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! \n");}
				if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! \n");}
				if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");} //low == reset sensor
			}

			if(TRUE != hwPowerDown(PMIC_APP_SUB_CAMERA_POWER_D, mode_name)) {
				PK_DBG("[CAMERA SENSOR] Fail to OFF digital power\n");
				goto _kdCISModulePowerOn_exit_;
			}

			if(TRUE != hwPowerDown(PMIC_APP_SUB_CAMERA_POWER_A, mode_name)) {
				PK_DBG("[CAMERA SENSOR] Fail to OFF analog power\n");
				goto _kdCISModulePowerOn_exit_;
			}

			if(TRUE != hwPowerDown(PMIC_APP_SUB_CAMERA_POWER_IO, mode_name))
			{
				PK_DBG("[CAMERA SENSOR] Fail to OFF IO power\n");
				goto _kdCISModulePowerOn_exit_;
			}

	        //PDN pull down
	        if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST]) {
	            if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}
	            if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}
	    	    if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_ON])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");} //high == power down lens module
	        }
		}
  }

  return 0;
  
_kdCISModulePowerOn_exit_:
    return -EIO;
}

EXPORT_SYMBOL(kdCISModulePowerOn);
