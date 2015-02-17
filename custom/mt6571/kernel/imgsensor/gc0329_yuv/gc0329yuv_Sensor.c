
#include <linux/videodev2.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <asm/atomic.h>
#include <asm/io.h>

#include "kd_camera_hw.h"
#include "kd_imgsensor.h"
#include "kd_imgsensor_define.h"
#include "kd_imgsensor_errcode.h"
#include "kd_camera_feature.h"

#include "gc0329yuv_Sensor.h"
#include "gc0329yuv_Camera_Sensor_para.h"
#include "gc0329yuv_CameraCustomized.h"

#define GC0329YUV_DEBUG
#ifdef GC0329YUV_DEBUG
#define SENSORDB printk
#else
#define SENSORDB(x,...)
#endif
struct GC0329_Sensor_Struct
{
	//kal_uint8   Banding;
//	kal_bool	  NightMode;
//	kal_bool	  VideoMode;
//	kal_uint16  Fps;
//	kal_uint16  ShutterStep;
//	kal_uint8   IsPVmode;
//	kal_uint32  PreviewDummyPixels;
//	kal_uint32  PreviewDummyLines;
//	kal_uint32  CaptureDummyPixels;
//	kal_uint32  CaptureDummyLines;
//	kal_uint32  PreviewPclk;
//	kal_uint32  CapturePclk;
//	kal_uint32  ZsdturePclk;
//	kal_uint32  PreviewShutter;
//	kal_uint32  PreviewExtraShutter;
//	kal_uint32  SensorGain;
//	kal_bool    	manualAEStart;
	kal_bool    	userAskAeLock;
    kal_bool    	userAskAwbLock;
//	kal_uint32      currentExposureTime;
//    kal_uint32      currentShutter;
//	kal_uint32      currentextshutter;
//    kal_uint32      currentAxDGain;
	kal_uint32  	sceneMode;
    unsigned char isoSpeed;
	kal_bool    	AE_ENABLE;
	
//	unsigned char zsd_flag;
//	GC0329MIPI_SENSOR_MODE SensorMode;
	kal_uint16 wb;

} ;

static struct GC0329_Sensor_Struct GC0329_SensorDriver;
static DEFINE_SPINLOCK(GC0329_drv_lock);
static MSDK_SCENARIO_ID_ENUM GC0329_CurrentScenarioId = MSDK_SCENARIO_ID_CAMERA_PREVIEW;
#define GC0329_TEST_PATTERN_CHECKSUM (0xa2054b84)//0x12345678

static kal_uint32 GC0329_zoom_factor = 0; 


///////////////////////////////////
extern int iReadRegI2C(u8 *a_pSendData , u16 a_sizeSendData, u8 * a_pRecvData, u16 a_sizeRecvData, u16 i2cId);
extern int iWriteRegI2C(u8 *a_pSendData , u16 a_sizeSendData, u16 i2cId);

/*************************************************************************
* FUNCTION
*    GC0329_write_cmos_sensor
*
* DESCRIPTION
*    This function wirte data to CMOS sensor through I2C
*
* PARAMETERS
*    addr: the 16bit address of register
*    para: the 8bit value of register
*
* RETURNS
*    None
*
* LOCAL AFFECTED
*
*************************************************************************/
static void GC0329_write_cmos_sensor(kal_uint8 addr, kal_uint8 para)
{
kal_uint8 out_buff[2];

    out_buff[0] = addr;
    out_buff[1] = para;

    iWriteRegI2C((u8*)out_buff , (u16)sizeof(out_buff), GC0329_WRITE_ID); 

#if (defined(__GC0329_DEBUG_TRACE__))
  if (sizeof(out_buff) != rt) printk("I2C write %x, %x error\n", addr, para);
#endif
}

/*************************************************************************
* FUNCTION
*    GC0329_read_cmos_sensor
*
* DESCRIPTION
*    This function read data from CMOS sensor through I2C.
*
* PARAMETERS
*    addr: the 16bit address of register
*
* RETURNS
*    8bit data read through I2C
*
* LOCAL AFFECTED
*
*************************************************************************/
static kal_uint8 GC0329_read_cmos_sensor(kal_uint8 addr)
{
  kal_uint8 in_buff[1] = {0xFF};
  kal_uint8 out_buff[1];
  
  out_buff[0] = addr;
   
    if (0 != iReadRegI2C((u8*)out_buff , (u16) sizeof(out_buff), (u8*)in_buff, (u16) sizeof(in_buff), GC0329_WRITE_ID)) {
        SENSORDB("ERROR: GC0329_read_cmos_sensor \n");
    }

#if (defined(__GC0329_DEBUG_TRACE__))
  if (size != rt) printk("I2C read %x error\n", addr);
#endif

  return in_buff[0];
}

/*******************************************************************************
 * // Adapter for Winmo typedef
 ********************************************************************************/
#define WINMO_USE 0

#define Sleep(ms) mdelay(ms)
#define RETAILMSG(x,...)
#define TEXT

kal_bool   GC0329_MPEG4_encode_mode = KAL_FALSE;
kal_uint16 GC0329_dummy_pixels = 0, GC0329_dummy_lines = 0;
kal_bool   GC0329_MODE_CAPTURE = KAL_FALSE;
kal_bool   GC0329_CAM_BANDING_50HZ = KAL_FALSE;

kal_uint32 GC0329_isp_master_clock;
static kal_uint32 GC0329_g_fPV_PCLK = 26;

kal_uint8 GC0329_sensor_write_I2C_address = GC0329_WRITE_ID;
kal_uint8 GC0329_sensor_read_I2C_address = GC0329_READ_ID;

UINT8 GC0329PixelClockDivider=0;

MSDK_SENSOR_CONFIG_STRUCT GC0329SensorConfigData;

#define GC0329_SET_PAGE0 	GC0329_write_cmos_sensor(0xfe, 0x00)
#define GC0329_SET_PAGE1 	GC0329_write_cmos_sensor(0xfe, 0x01)

kal_bool GC0329_night_mode_enable = KAL_FALSE;

/*************************************************************************
 * FUNCTION
 *	GC0329_SetShutter
 *
 * DESCRIPTION
 *	This function set e-shutter of GC0329 to change exposure time.
 *
 * PARAMETERS
 *   iShutter : exposured lines
 *
 * RETURNS
 *	None
 *
 * GLOBALS AFFECTED
 *
 *************************************************************************/
void GC0329_Set_Shutter(kal_uint16 iShutter)
{
} /* Set_GC0329_Shutter */


/*************************************************************************
 * FUNCTION
 *	GC0329_read_Shutter
 *
 * DESCRIPTION
 *	This function read e-shutter of GC0329 .
 *
 * PARAMETERS
 *  None
 *
 * RETURNS
 *	None
 *
 * GLOBALS AFFECTED
 *
 *************************************************************************/
kal_uint16 GC0329_Read_Shutter(void)
{
    	kal_uint8 temp_reg1, temp_reg2;
	kal_uint16 shutter;

	temp_reg1 = GC0329_read_cmos_sensor(0x04);
	temp_reg2 = GC0329_read_cmos_sensor(0x03);

	shutter = (temp_reg1 & 0xFF) | (temp_reg2 << 8);

	return shutter;
} /* GC0329_read_shutter */


/*************************************************************************
 * FUNCTION
 *	GC0329_write_reg
 *
 * DESCRIPTION
 *	This function set the register of GC0329.
 *
 * PARAMETERS
 *	addr : the register index of GC0329
 *  para : setting parameter of the specified register of GC0329
 *
 * RETURNS
 *	None
 *
 * GLOBALS AFFECTED
 *
 *************************************************************************/
void GC0329_write_reg(kal_uint32 addr, kal_uint32 para)
{
	GC0329_write_cmos_sensor(addr, para);
} /* GC0329_write_reg() */


/*************************************************************************
 * FUNCTION
 *	GC0329_read_cmos_sensor
 *
 * DESCRIPTION
 *	This function read parameter of specified register from GC0329.
 *
 * PARAMETERS
 *	addr : the register index of GC0329
 *
 * RETURNS
 *	the data that read from GC0329
 *
 * GLOBALS AFFECTED
 *
 *************************************************************************/
kal_uint32 GC0329_read_reg(kal_uint32 addr)
{
	return GC0329_read_cmos_sensor(addr);
} /* OV7670_read_reg() */


/*************************************************************************
* FUNCTION
*	GC0329_awb_enable
*
* DESCRIPTION
*	This function enable or disable the awb (Auto White Balance).
*
* PARAMETERS
*	1. kal_bool : KAL_TRUE - enable awb, KAL_FALSE - disable awb.
*
* RETURNS
*	kal_bool : It means set awb right or not.
*
*************************************************************************/
static void GC0329_awb_enable(kal_bool enalbe)
{	 
	kal_uint16 temp_AWB_reg = 0;

	//temp_AWB_reg = GC0329_read_cmos_sensor(0x42);
	spin_lock(&GC0329_drv_lock);
	temp_AWB_reg = GC0329_SensorDriver.wb;
	spin_unlock(&GC0329_drv_lock);
	
	SENSORDB("[GC0329]GC0329_awb_enable reg 0x42=%x:\n ", temp_AWB_reg);
	if (enalbe)
	{
		temp_AWB_reg = (temp_AWB_reg |0x02);
		GC0329_write_cmos_sensor(0x42, temp_AWB_reg);
	}
	else
	{
		temp_AWB_reg = (temp_AWB_reg & (~0x02));
		GC0329_write_cmos_sensor(0x42, temp_AWB_reg);
	}

	spin_lock(&GC0329_drv_lock);
	GC0329_SensorDriver.wb = temp_AWB_reg;
	spin_unlock(&GC0329_drv_lock);

}
static void GC0329_set_AE_mode(kal_bool AE_enable)
{
    kal_uint8 AeTemp;
	SENSORDB("[GC0329]enter GC0329_set_AE_mode function:\n ");

    if (AE_enable == KAL_TRUE)
    {
        // turn on AEC/AGC
			
			GC0329_write_cmos_sensor(0xfe, 0x00);
			GC0329_write_cmos_sensor(0x4f, 0x01);

		

    }
    else
    {
        // turn off AEC/AGC
			GC0329_write_cmos_sensor(0xfe, 0x00);
			GC0329_write_cmos_sensor(0x4f, 0x00);
    }
	SENSORDB("[GC0329]exit GC0329_set_AE_mode function:\n ");
}

/*************************************************************************
 * FUNCTION
 *	GC0329_config_window
 *
 * DESCRIPTION
 *	This function config the hardware window of GC0329 for getting specified
 *  data of that window.
 *
 * PARAMETERS
 *	start_x : start column of the interested window
 *  start_y : start row of the interested window
 *  width  : column widht of the itnerested window
 *  height : row depth of the itnerested window
 *
 * RETURNS
 *	the data that read from GC0329
 *
 * GLOBALS AFFECTED
 *
 *************************************************************************/
void GC0329_config_window(kal_uint16 startx, kal_uint16 starty, kal_uint16 width, kal_uint16 height)
{
} /* GC0329_config_window */


/*************************************************************************
 * FUNCTION
 *	GC0329_SetGain
 *
 * DESCRIPTION
 *	This function is to set global gain to sensor.
 *
 * PARAMETERS
 *   iGain : sensor global gain(base: 0x40)
 *
 * RETURNS
 *	the actually gain set to sensor.
 *
 * GLOBALS AFFECTED
 *
 *************************************************************************/
kal_uint16 GC0329_SetGain(kal_uint16 iGain)
{
	return iGain;
}
kal_uint16 GC0329_ReadGain(void)
{
    	kal_uint8 temp_reg0, temp_reg1, temp_reg2;
	kal_uint16 gain;

	temp_reg1 = GC0329_read_cmos_sensor(0x71);
	temp_reg2 = GC0329_read_cmos_sensor(0x72);

	gain = (temp_reg1 & 0xFF) * (temp_reg2 & 0xFF);

	return gain;
}

/*************************************************************************
 * FUNCTION
 *	GC0329_GAMMA_Select
 *
 * DESCRIPTION
 *	This function select gamma of GC0329.
 *
 * PARAMETERS
 *	bEnable: KAL_TRUE -> enable night mode, otherwise, disable night mode
 *
 * RETURNS
 *	None
 *
 * GLOBALS AFFECTED
 *
 *************************************************************************/
void GC0329_GAMMA_Select(kal_uint32 GammaLvl)
{
	switch(GammaLvl)
	{
		case GC0329_RGB_Gamma_m1:											  //smallest gamma curve
			GC0329_write_cmos_sensor(0xfe, 0x00);
			GC0329_write_cmos_sensor(0xbf, 0x06);
			GC0329_write_cmos_sensor(0xc0, 0x12);
			GC0329_write_cmos_sensor(0xc1, 0x22);
			GC0329_write_cmos_sensor(0xc2, 0x35);
			GC0329_write_cmos_sensor(0xc3, 0x4b);
			GC0329_write_cmos_sensor(0xc4, 0x5f);
			GC0329_write_cmos_sensor(0xc5, 0x72);
			GC0329_write_cmos_sensor(0xc6, 0x8d);
			GC0329_write_cmos_sensor(0xc7, 0xa4);
			GC0329_write_cmos_sensor(0xc8, 0xb8);
			GC0329_write_cmos_sensor(0xc9, 0xc8);
			GC0329_write_cmos_sensor(0xca, 0xd4);
			GC0329_write_cmos_sensor(0xcb, 0xde);
			GC0329_write_cmos_sensor(0xcc, 0xe6);
			GC0329_write_cmos_sensor(0xcd, 0xf1);
			GC0329_write_cmos_sensor(0xce, 0xf8);
			GC0329_write_cmos_sensor(0xcf, 0xfd);
			break;
		case GC0329_RGB_Gamma_m2:
			GC0329_write_cmos_sensor(0xBF, 0x08);
			GC0329_write_cmos_sensor(0xc0, 0x0F);
			GC0329_write_cmos_sensor(0xc1, 0x21);
			GC0329_write_cmos_sensor(0xc2, 0x32);
			GC0329_write_cmos_sensor(0xc3, 0x43);
			GC0329_write_cmos_sensor(0xc4, 0x50);
			GC0329_write_cmos_sensor(0xc5, 0x5E);
			GC0329_write_cmos_sensor(0xc6, 0x78);
			GC0329_write_cmos_sensor(0xc7, 0x90);
			GC0329_write_cmos_sensor(0xc8, 0xA6);
			GC0329_write_cmos_sensor(0xc9, 0xB9);
			GC0329_write_cmos_sensor(0xcA, 0xC9);
			GC0329_write_cmos_sensor(0xcB, 0xD6);
			GC0329_write_cmos_sensor(0xcC, 0xE0);
			GC0329_write_cmos_sensor(0xcD, 0xEE);
			GC0329_write_cmos_sensor(0xcE, 0xF8);
			GC0329_write_cmos_sensor(0xcF, 0xFF);
			break;
			
		case GC0329_RGB_Gamma_m3:			
			GC0329_write_cmos_sensor(0xBF, 0x0B);
			GC0329_write_cmos_sensor(0xc0, 0x16);
			GC0329_write_cmos_sensor(0xc1, 0x29);
			GC0329_write_cmos_sensor(0xc2, 0x3C);
			GC0329_write_cmos_sensor(0xc3, 0x4F);
			GC0329_write_cmos_sensor(0xc4, 0x5F);
			GC0329_write_cmos_sensor(0xc5, 0x6F);
			GC0329_write_cmos_sensor(0xc6, 0x8A);
			GC0329_write_cmos_sensor(0xc7, 0x9F);
			GC0329_write_cmos_sensor(0xc8, 0xB4);
			GC0329_write_cmos_sensor(0xc9, 0xC6);
			GC0329_write_cmos_sensor(0xcA, 0xD3);
			GC0329_write_cmos_sensor(0xcB, 0xDD);
			GC0329_write_cmos_sensor(0xcC, 0xE5);
			GC0329_write_cmos_sensor(0xcD, 0xF1);
			GC0329_write_cmos_sensor(0xcE, 0xFA);
			GC0329_write_cmos_sensor(0xcF, 0xFF);
			break;
			
		case GC0329_RGB_Gamma_m4:
			GC0329_write_cmos_sensor(0xBF, 0x0E);
			GC0329_write_cmos_sensor(0xc0, 0x1C);
			GC0329_write_cmos_sensor(0xc1, 0x34);
			GC0329_write_cmos_sensor(0xc2, 0x48);
			GC0329_write_cmos_sensor(0xc3, 0x5A);
			GC0329_write_cmos_sensor(0xc4, 0x6B);
			GC0329_write_cmos_sensor(0xc5, 0x7B);
			GC0329_write_cmos_sensor(0xc6, 0x95);
			GC0329_write_cmos_sensor(0xc7, 0xAB);
			GC0329_write_cmos_sensor(0xc8, 0xBF);
			GC0329_write_cmos_sensor(0xc9, 0xCE);
			GC0329_write_cmos_sensor(0xcA, 0xD9);
			GC0329_write_cmos_sensor(0xcB, 0xE4);
			GC0329_write_cmos_sensor(0xcC, 0xEC);
			GC0329_write_cmos_sensor(0xcD, 0xF7);
			GC0329_write_cmos_sensor(0xcE, 0xFD);
			GC0329_write_cmos_sensor(0xcF, 0xFF);
			break;
			
		case GC0329_RGB_Gamma_m5:
			GC0329_write_cmos_sensor(0xBF, 0x10);
			GC0329_write_cmos_sensor(0xc0, 0x20);
			GC0329_write_cmos_sensor(0xc1, 0x38);
			GC0329_write_cmos_sensor(0xc2, 0x4E);
			GC0329_write_cmos_sensor(0xc3, 0x63);
			GC0329_write_cmos_sensor(0xc4, 0x76);
			GC0329_write_cmos_sensor(0xc5, 0x87);
			GC0329_write_cmos_sensor(0xc6, 0xA2);
			GC0329_write_cmos_sensor(0xc7, 0xB8);
			GC0329_write_cmos_sensor(0xc8, 0xCA);
			GC0329_write_cmos_sensor(0xc9, 0xD8);
			GC0329_write_cmos_sensor(0xcA, 0xE3);
			GC0329_write_cmos_sensor(0xcB, 0xEB);
			GC0329_write_cmos_sensor(0xcC, 0xF0);
			GC0329_write_cmos_sensor(0xcD, 0xF8);
			GC0329_write_cmos_sensor(0xcE, 0xFD);
			GC0329_write_cmos_sensor(0xcF, 0xFF);
			break;
			
		case GC0329_RGB_Gamma_m6:
			GC0329_write_cmos_sensor(0xBF, 0x14);
			GC0329_write_cmos_sensor(0xc0, 0x28);
			GC0329_write_cmos_sensor(0xc1, 0x44);
			GC0329_write_cmos_sensor(0xc2, 0x5D);
			GC0329_write_cmos_sensor(0xc3, 0x72);
			GC0329_write_cmos_sensor(0xc4, 0x86);
			GC0329_write_cmos_sensor(0xc5, 0x95);
			GC0329_write_cmos_sensor(0xc6, 0xB1);
			GC0329_write_cmos_sensor(0xc7, 0xC6);
			GC0329_write_cmos_sensor(0xc8, 0xD5);
			GC0329_write_cmos_sensor(0xc9, 0xE1);
			GC0329_write_cmos_sensor(0xcA, 0xEA);
			GC0329_write_cmos_sensor(0xcB, 0xF1);
			GC0329_write_cmos_sensor(0xcC, 0xF5);
			GC0329_write_cmos_sensor(0xcD, 0xFB);
			GC0329_write_cmos_sensor(0xcE, 0xFE);
			GC0329_write_cmos_sensor(0xcF, 0xFF);							// largest gamma curve
			break;
		case GC0329_RGB_Gamma_night:									//Gamma for night mode
			GC0329_write_cmos_sensor(0xBF, 0x0B);
			GC0329_write_cmos_sensor(0xc0, 0x16);
			GC0329_write_cmos_sensor(0xc1, 0x29);
			GC0329_write_cmos_sensor(0xc2, 0x3C);
			GC0329_write_cmos_sensor(0xc3, 0x4F);
			GC0329_write_cmos_sensor(0xc4, 0x5F);
			GC0329_write_cmos_sensor(0xc5, 0x6F);
			GC0329_write_cmos_sensor(0xc6, 0x8A);
			GC0329_write_cmos_sensor(0xc7, 0x9F);
			GC0329_write_cmos_sensor(0xc8, 0xB4);
			GC0329_write_cmos_sensor(0xc9, 0xC6);
			GC0329_write_cmos_sensor(0xcA, 0xD3);
			GC0329_write_cmos_sensor(0xcB, 0xDD);
			GC0329_write_cmos_sensor(0xcC, 0xE5);
			GC0329_write_cmos_sensor(0xcD, 0xF1);
			GC0329_write_cmos_sensor(0xcE, 0xFA);
			GC0329_write_cmos_sensor(0xcF, 0xFF);
			break;
		default:
			break;
	}
}

/*************************************************************************
 * FUNCTION
 *	GC0329_NightMode
 *
 * DESCRIPTION
 *	This function night mode of GC0329.
 *
 * PARAMETERS
 *	bEnable: KAL_TRUE -> enable night mode, otherwise, disable night mode
 *
 * RETURNS
 *	None
 *
 * GLOBALS AFFECTED
 *
 *************************************************************************/
void GC0329_night_mode(kal_bool bEnable)
{
	if (bEnable)
	{
		GC0329_SET_PAGE1;
		GC0329_write_cmos_sensor(0xfe, 0x01);
		GC0329_write_cmos_sensor(0x11, 0xa1);
		//GC0329_write_cmos_sensor(0x13, 0x68);//ae target
		if(GC0329_MPEG4_encode_mode == KAL_TRUE) 
			{
			GC0329_SET_PAGE0;
			GC0329_write_cmos_sensor(0xfa, 0x10);
			GC0329_SET_PAGE1;
			GC0329_write_cmos_sensor(0x33, 0x00);
		SENSORDB("GC0329_night_mode mode VIDEO = %d \n", bEnable);
			}
		else
			{
			GC0329_SET_PAGE0;
			GC0329_write_cmos_sensor(0xfa, 0x00);
			GC0329_SET_PAGE1;
			GC0329_write_cmos_sensor(0x33, 0x30);
		SENSORDB("GC0329_night_mode mode preview = %d \n", bEnable);
			
			}
		GC0329_write_cmos_sensor(0x21, 0xb0);
		GC0329_write_cmos_sensor(0x22, 0x60);
		GC0329_SET_PAGE0;
		GC0329_write_cmos_sensor(0x40, 0xef);
		GC0329_write_cmos_sensor(0x41, 0x74);
		//GC0329_write_cmos_sensor(0x42, 0x7e);
		//GC0329_write_cmos_sensor(0xd1, 0x40);//saturation Cb
		//GC0329_write_cmos_sensor(0xd2, 0x40);//saturation Cr
		//GC0329_write_cmos_sensor(0xd3, 0x4b);//contrast
		//GC0329_write_cmos_sensor(0xd5, 0x2b);//luma offset ,modify in bright and EV
		GC0329_write_cmos_sensor(0xde, 0x30);
	
		GC0329_GAMMA_Select(GC0329_RGB_Gamma_night);
	}
	else 
	{
		GC0329_SET_PAGE1;
		GC0329_write_cmos_sensor(0x11, 0xa1);
		//GC0329_write_cmos_sensor(0x13, 0x40);//ae target
		if(GC0329_MPEG4_encode_mode == KAL_TRUE)
			{
			GC0329_SET_PAGE0;
			GC0329_write_cmos_sensor(0xfa, 0x00);
			GC0329_SET_PAGE1;
			GC0329_write_cmos_sensor(0x33, 0x00);
		SENSORDB("GC0329_night_mode mode VIDEO = %d\n", bEnable);
			}
		else
			{
			GC0329_SET_PAGE0;
			GC0329_write_cmos_sensor(0xfa, 0x00);
			GC0329_SET_PAGE1;
			GC0329_write_cmos_sensor(0x33, 0x20);
		SENSORDB("GC0329_night_mode mode preview = %d\n", bEnable);
			
			}
		
		GC0329_write_cmos_sensor(0x21, 0xb0);
		GC0329_write_cmos_sensor(0x22, 0x48);
		GC0329_SET_PAGE0;
		GC0329_write_cmos_sensor(0x40, 0xff);
		GC0329_write_cmos_sensor(0x41, 0x04);
		//GC0329_write_cmos_sensor(0x42, 0x7e);
		//GC0329_write_cmos_sensor(0xd1, 0x30);//saturation Cb
		//GC0329_write_cmos_sensor(0xd2, 0x30);//saturation Cr
		//GC0329_write_cmos_sensor(0xd3, 0x40);//contrast
		//GC0329_write_cmos_sensor(0xd5, 0x00);//luma offset ,modify in bright and EV
		//GC0329_write_cmos_sensor(0xde, 0x34);//2013.01.29
		GC0329_GAMMA_Select(GC0329_RGB_Gamma_m1);		
	}
	
	spin_lock(&GC0329_drv_lock);
	//GC0329_SensorDriver.SensorMode= SENSOR_MODE_PREVIEW;
	GC0329_night_mode_enable = bEnable;
	spin_unlock(&GC0329_drv_lock);
	SENSORDB("[GC0329]CONTROLFLOW GC0329_night_mode mode = %d", bEnable);
} /* GC0329_NightMode */


/*************************************************************************
* FUNCTION
*	GC0329_Sensor_Init
*
* DESCRIPTION
*	This function apply all of the initial setting to sensor.
*
* PARAMETERS
*	None
*
* RETURNS
*	None
*
*************************************************************************/
void GC0329_Sensor_Init(void)
{
	GC0329_write_cmos_sensor(0xfe, 0x80);
	GC0329_write_cmos_sensor(0xfc, 0x16); 
	GC0329_write_cmos_sensor(0xfc, 0x16); 
	GC0329_write_cmos_sensor(0xfe, 0x00);
        GC0329_write_cmos_sensor(0x4f, 0x00); //AEC off 
	GC0329_write_cmos_sensor(0x73, 0x90); 
	GC0329_write_cmos_sensor(0x74, 0x80); 
	GC0329_write_cmos_sensor(0x75, 0x80); 
	GC0329_write_cmos_sensor(0x76, 0x94);

	GC0329_write_cmos_sensor(0x09, 0x00); 
	GC0329_write_cmos_sensor(0x0a, 0x02); 
	GC0329_write_cmos_sensor(0x0b, 0x00); 
	GC0329_write_cmos_sensor(0x0c, 0x02);
	GC0329_write_cmos_sensor(0x03, 0x02); 
	GC0329_write_cmos_sensor(0x04, 0x98);//3
	
			
	GC0329_write_cmos_sensor(0x17, 0x14); 
	GC0329_write_cmos_sensor(0x19, 0x05); 
	GC0329_write_cmos_sensor(0x1b, 0x24); 
	GC0329_write_cmos_sensor(0x1c, 0x04); 
	GC0329_write_cmos_sensor(0x1e, 0x08); 
	GC0329_write_cmos_sensor(0x1f, 0xc0);
	GC0329_write_cmos_sensor(0x20, 0x00); 
	GC0329_write_cmos_sensor(0x21, 0x48);
	GC0329_write_cmos_sensor(0x22, 0xba);
	GC0329_write_cmos_sensor(0x23, 0x22); 
	//GC0329_write_cmos_sensor(0x24, 0x16); 
	//GC0329_write_cmos_sensor(0x24, 0x17); 
	GC0329_write_cmos_sensor(0x24, 0x16); 
	//GC0329_write_cmos_sensor(0x24, 0x1e); 

	////////////////////blk////////////////////
	GC0329_write_cmos_sensor(0x26, 0xf7);//blk speed    97
	GC0329_write_cmos_sensor(0x32, 0x04);
	GC0329_write_cmos_sensor(0x33, 0x20);
	GC0329_write_cmos_sensor(0x34, 0x20);
	GC0329_write_cmos_sensor(0x35, 0x20);
	GC0329_write_cmos_sensor(0x36, 0x20);
	////////////////////ISP BLOCK ENABLE////////////////////
	GC0329_write_cmos_sensor(0x40, 0xff); 
	GC0329_write_cmos_sensor(0x41, 0x04);
	GC0329_write_cmos_sensor(0x42, 0x7e); 
	GC0329_write_cmos_sensor(0x46, 0x02); 
	//GC0329_write_cmos_sensor(0x46, 0x06); 
	GC0329_write_cmos_sensor(0x4b, 0xcb);
	GC0329_write_cmos_sensor(0x4d, 0x01); 
	GC0329_write_cmos_sensor(0x4f, 0x00);
	GC0329_write_cmos_sensor(0x70, 0x48);
	
	////////////////////DNDD////////////////////
	GC0329_write_cmos_sensor(0x80, 0xe7); 
	GC0329_write_cmos_sensor(0x82, 0x55); 
	GC0329_write_cmos_sensor(0x87, 0x4a); 
	////////////////////ASDE////////////////////
	GC0329_write_cmos_sensor(0xfe, 0x01);
	GC0329_write_cmos_sensor(0x18, 0x22); 
	GC0329_write_cmos_sensor(0xfe, 0x00);
	GC0329_write_cmos_sensor(0x9c, 0x0a); 
	GC0329_write_cmos_sensor(0xa4, 0x60); 
	GC0329_write_cmos_sensor(0xa5, 0x21); 
	GC0329_write_cmos_sensor(0xa7, 0x35); 
	GC0329_write_cmos_sensor(0xdd, 0x54); 
	GC0329_write_cmos_sensor(0x95, 0x35); 
	////////////////////RGB gamma////////////////////
	GC0329_write_cmos_sensor(0xfe, 0x00);
	GC0329_write_cmos_sensor(0xbf, 0x06);
	GC0329_write_cmos_sensor(0xc0, 0x12);
	GC0329_write_cmos_sensor(0xc1, 0x22);
	GC0329_write_cmos_sensor(0xc2, 0x35);
	GC0329_write_cmos_sensor(0xc3, 0x4b);
	GC0329_write_cmos_sensor(0xc4, 0x5f);
	GC0329_write_cmos_sensor(0xc5, 0x72);
	GC0329_write_cmos_sensor(0xc6, 0x8d);
	GC0329_write_cmos_sensor(0xc7, 0xa4);
	GC0329_write_cmos_sensor(0xc8, 0xb8);
	GC0329_write_cmos_sensor(0xc9, 0xc8);
	GC0329_write_cmos_sensor(0xca, 0xd4);
	GC0329_write_cmos_sensor(0xcb, 0xde);
	GC0329_write_cmos_sensor(0xcc, 0xe6);
	GC0329_write_cmos_sensor(0xcd, 0xf1);
	GC0329_write_cmos_sensor(0xce, 0xf8);
	GC0329_write_cmos_sensor(0xcf, 0xfd);
	//////////////////CC///////////////////
	GC0329_write_cmos_sensor(0xfe, 0x00);
	GC0329_write_cmos_sensor(0xb3, 0x44);
	GC0329_write_cmos_sensor(0xb4, 0xfd);
	GC0329_write_cmos_sensor(0xb5, 0x02);
	GC0329_write_cmos_sensor(0xb6, 0xfa);
	GC0329_write_cmos_sensor(0xb7, 0x48);
	GC0329_write_cmos_sensor(0xb8, 0xf0);
	//skin
	//GC0329_write_cmos_sensor(0xb3, 0x3c);
	//GC0329_write_cmos_sensor(0xb4, 0xFF);
	//GC0329_write_cmos_sensor(0xb5, 0x03);
	//GC0329_write_cmos_sensor(0xb6, 0x01);
	//GC0329_write_cmos_sensor(0xb7, 0x3f);
	//GC0329_write_cmos_sensor(0xb8, 0xF3);
	// crop 
	GC0329_write_cmos_sensor(0x50, 0x01);
	GC0329_write_cmos_sensor(0x19, 0x05);
	GC0329_write_cmos_sensor(0x20, 0x01);
	GC0329_write_cmos_sensor(0x22, 0xba);
	GC0329_write_cmos_sensor(0x21, 0x48);
	////////////////////YCP////////////////////
	GC0329_write_cmos_sensor(0xfe, 0x00);
	GC0329_write_cmos_sensor(0xd1, 0x30); 
	GC0329_write_cmos_sensor(0xd2, 0x30);	
	GC0329_write_cmos_sensor(0xde, 0x34);//2013.01.29
	////////////////////AEC////////////////////
	GC0329_write_cmos_sensor(0xfe, 0x01);
	GC0329_write_cmos_sensor(0x10, 0x40);
	GC0329_write_cmos_sensor(0x11, 0xa1);
	GC0329_write_cmos_sensor(0x12, 0x03); //06
	GC0329_write_cmos_sensor(0x13, 0x60); //50 40
	GC0329_write_cmos_sensor(0x17, 0x88);
	GC0329_write_cmos_sensor(0x1a, 0x21);
	GC0329_write_cmos_sensor(0x21, 0xb0);
	GC0329_write_cmos_sensor(0x22, 0x48);
	GC0329_write_cmos_sensor(0x3c, 0x30);//95
	GC0329_write_cmos_sensor(0x3d, 0x70);//50
	GC0329_write_cmos_sensor(0x3e, 0x30);//90
	////////////////////AWB////////////////////
	GC0329_write_cmos_sensor(0xfe, 0x01);
	GC0329_write_cmos_sensor(0x06, 0x16);
	GC0329_write_cmos_sensor(0x07, 0x06);
	GC0329_write_cmos_sensor(0x08, 0x98);
	GC0329_write_cmos_sensor(0x09, 0xee);
	GC0329_write_cmos_sensor(0x50, 0xfc);
	GC0329_write_cmos_sensor(0x51, 0x28);
	GC0329_write_cmos_sensor(0x52, 0x10); //0b
	GC0329_write_cmos_sensor(0x53, 0x10);
	GC0329_write_cmos_sensor(0x54, 0x10);
	GC0329_write_cmos_sensor(0x55, 0x10); //10
	GC0329_write_cmos_sensor(0x56, 0x20); //20	//GC0329_write_cmos_sensor(0x57, 0x40); 
	GC0329_write_cmos_sensor(0x58, 0x60);
	GC0329_write_cmos_sensor(0x59, 0x08);  // 28
	GC0329_write_cmos_sensor(0x5a, 0x02);
	GC0329_write_cmos_sensor(0x5b, 0x63);
	GC0329_write_cmos_sensor(0x5c, 0x35);//3
	GC0329_write_cmos_sensor(0x5d, 0x73);
	GC0329_write_cmos_sensor(0x5e, 0x11);
	GC0329_write_cmos_sensor(0x5f, 0x40);
	GC0329_write_cmos_sensor(0x60, 0x40);
	GC0329_write_cmos_sensor(0x61, 0xc8);
	GC0329_write_cmos_sensor(0x62, 0xa0);
	GC0329_write_cmos_sensor(0x63, 0x40);
	GC0329_write_cmos_sensor(0x64, 0x50);
	GC0329_write_cmos_sensor(0x65, 0x98);
	GC0329_write_cmos_sensor(0x66, 0xfa);
	GC0329_write_cmos_sensor(0x67, 0x70);
	GC0329_write_cmos_sensor(0x68, 0x58);
	GC0329_write_cmos_sensor(0x69, 0x85);
	GC0329_write_cmos_sensor(0x6a, 0x40);
	GC0329_write_cmos_sensor(0x6b, 0x39);
	GC0329_write_cmos_sensor(0x6c, 0x20);//20
	GC0329_write_cmos_sensor(0x6d, 0x40);//30
	GC0329_write_cmos_sensor(0x6e, 0x00);
	GC0329_write_cmos_sensor(0x70, 0x02);
	GC0329_write_cmos_sensor(0x71, 0x00);
	GC0329_write_cmos_sensor(0x72, 0x10);
	GC0329_write_cmos_sensor(0x73, 0x40);
	
	GC0329_write_cmos_sensor(0x80, 0x58);
	GC0329_write_cmos_sensor(0x81, 0x50);
	GC0329_write_cmos_sensor(0x82, 0x44);
	GC0329_write_cmos_sensor(0x83, 0x40);
	GC0329_write_cmos_sensor(0x84, 0x40);
	GC0329_write_cmos_sensor(0x85, 0x40);
	
	GC0329_write_cmos_sensor(0x74, 0x40);
	GC0329_write_cmos_sensor(0x75, 0x58);
	GC0329_write_cmos_sensor(0x76, 0x24);
	GC0329_write_cmos_sensor(0x77, 0x40);
	GC0329_write_cmos_sensor(0x78, 0x20);
	GC0329_write_cmos_sensor(0x79, 0x60);
	GC0329_write_cmos_sensor(0x7a, 0x58);
	GC0329_write_cmos_sensor(0x7b, 0x20);
	GC0329_write_cmos_sensor(0x7c, 0x30);
	GC0329_write_cmos_sensor(0x7d, 0x35);
	GC0329_write_cmos_sensor(0x7e, 0x10);
	GC0329_write_cmos_sensor(0x7f, 0x08);
	///////////////////ABS///////////////////////
	GC0329_write_cmos_sensor(0x9c, 0x02);
	GC0329_write_cmos_sensor(0x9d, 0x30);
	////////////////////CC-AWB////////////////////
	GC0329_write_cmos_sensor(0xd0, 0x00);
	GC0329_write_cmos_sensor(0xd2, 0x2c); 
	GC0329_write_cmos_sensor(0xd3, 0x80);
	///////////////////LSC //////////////////////
	//// for xuye062d lens setting
	GC0329_write_cmos_sensor(0xfe, 0x01);
	GC0329_write_cmos_sensor(0xa0, 0x00);
	GC0329_write_cmos_sensor(0xa1, 0x3c);
	GC0329_write_cmos_sensor(0xa2, 0x50);
	GC0329_write_cmos_sensor(0xa3, 0x00);
	GC0329_write_cmos_sensor(0xa8, 0x0f);
	GC0329_write_cmos_sensor(0xa9, 0x08);
	GC0329_write_cmos_sensor(0xaa, 0x00);
	GC0329_write_cmos_sensor(0xab, 0x04);
	GC0329_write_cmos_sensor(0xac, 0x00);
	GC0329_write_cmos_sensor(0xad, 0x07);
	GC0329_write_cmos_sensor(0xae, 0x0e);
	GC0329_write_cmos_sensor(0xaf, 0x00);
	GC0329_write_cmos_sensor(0xb0, 0x00);
	GC0329_write_cmos_sensor(0xb1, 0x09);
	GC0329_write_cmos_sensor(0xb2, 0x00);
	GC0329_write_cmos_sensor(0xb3, 0x00);
	GC0329_write_cmos_sensor(0xb4, 0x35);
	GC0329_write_cmos_sensor(0xb5, 0x28);
	GC0329_write_cmos_sensor(0xb6, 0x24);
	GC0329_write_cmos_sensor(0xba, 0x3c);
	GC0329_write_cmos_sensor(0xbb, 0x2f);
	GC0329_write_cmos_sensor(0xbc, 0x2c);
	GC0329_write_cmos_sensor(0xc0, 0x1b);
	GC0329_write_cmos_sensor(0xc1, 0x16);
	GC0329_write_cmos_sensor(0xc2, 0x15);
	GC0329_write_cmos_sensor(0xc6, 0x21);
	GC0329_write_cmos_sensor(0xc7, 0x1c);
	GC0329_write_cmos_sensor(0xc8, 0x1b);
	GC0329_write_cmos_sensor(0xb7, 0x00);
	GC0329_write_cmos_sensor(0xb8, 0x00);
	GC0329_write_cmos_sensor(0xb9, 0x00);
	GC0329_write_cmos_sensor(0xbd, 0x00);
	GC0329_write_cmos_sensor(0xbe, 0x00);
	GC0329_write_cmos_sensor(0xbf, 0x00);
	GC0329_write_cmos_sensor(0xc3, 0x00);
	GC0329_write_cmos_sensor(0xc4, 0x00);
	GC0329_write_cmos_sensor(0xc5, 0x00);
	GC0329_write_cmos_sensor(0xc9, 0x00);
	GC0329_write_cmos_sensor(0xca, 0x00);
	GC0329_write_cmos_sensor(0xcb, 0x00);
	GC0329_write_cmos_sensor(0xa4, 0x00);
	GC0329_write_cmos_sensor(0xa5, 0x00);
	GC0329_write_cmos_sensor(0xa6, 0x00);
	GC0329_write_cmos_sensor(0xa7, 0x00);
	GC0329_write_cmos_sensor(0xfe, 0x00);
	////////////////////asde ///////////////////
	GC0329_write_cmos_sensor(0xa0, 0xaf);
	GC0329_write_cmos_sensor(0xa2, 0xff);
	GC0329_write_cmos_sensor(0xa4, 0x60);//2013.1.29
	GC0329_write_cmos_sensor(0x44, 0xa0);
	//GC0329_write_cmos_sensor(0x44, 0xa2);
	GC0329_write_cmos_sensor(0x4f, 0x01);//add
	
	Sleep(300);
	//GC0329_write_cmos_sensor(0x4f, 0x01);//add
	GC0329_write_cmos_sensor(0xf0, 0x07); 
	GC0329_write_cmos_sensor(0xf1, 0x01); 

}


/*************************************************************************
* FUNCTION
*	GC329_Lens_Select
*
* DESCRIPTION
*	This function is served for FAE to select the appropriate lens parameter.
*
* PARAMETERS
*	None
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/
void GC0329_Lens_Select(kal_uint8 Lens_Tag)
{
	switch(Lens_Tag)
	{
		case CHT_806C_2:
			GC0329_write_cmos_sensor(0xfe, 0x01);
			GC0329_write_cmos_sensor(0xa0, 0x00);
			GC0329_write_cmos_sensor(0xa1, 0x3c);
			GC0329_write_cmos_sensor(0xa2, 0x50);
			GC0329_write_cmos_sensor(0xa3, 0x00);
			GC0329_write_cmos_sensor(0xa4, 0x00);
			GC0329_write_cmos_sensor(0xa5, 0x00);
			GC0329_write_cmos_sensor(0xa6, 0x00);
			GC0329_write_cmos_sensor(0xa7, 0x04);
			
			GC0329_write_cmos_sensor(0xa8, 0x0f);
			GC0329_write_cmos_sensor(0xa9, 0x08);
			GC0329_write_cmos_sensor(0xaa, 0x00);
			GC0329_write_cmos_sensor(0xab, 0x04);
			GC0329_write_cmos_sensor(0xac, 0x00);
			GC0329_write_cmos_sensor(0xad, 0x07);
			GC0329_write_cmos_sensor(0xae, 0x0e);
			GC0329_write_cmos_sensor(0xaf, 0x00);
			GC0329_write_cmos_sensor(0xb0, 0x00);
			GC0329_write_cmos_sensor(0xb1, 0x09);
			GC0329_write_cmos_sensor(0xb2, 0x00);
			GC0329_write_cmos_sensor(0xb3, 0x00);

			GC0329_write_cmos_sensor(0xb4, 0x30);
			GC0329_write_cmos_sensor(0xb5, 0x19);
			GC0329_write_cmos_sensor(0xb6, 0x21);
			GC0329_write_cmos_sensor(0xba, 0x3e);
			GC0329_write_cmos_sensor(0xbb, 0x26);
			GC0329_write_cmos_sensor(0xbc, 0x2f);
			GC0329_write_cmos_sensor(0xc0, 0x15);
			GC0329_write_cmos_sensor(0xc1, 0x11);
			GC0329_write_cmos_sensor(0xc2, 0x15);
			GC0329_write_cmos_sensor(0xc6, 0x1f);
			GC0329_write_cmos_sensor(0xc7, 0x16);
			GC0329_write_cmos_sensor(0xc8, 0x16);

			GC0329_write_cmos_sensor(0xb7, 0x00);
			GC0329_write_cmos_sensor(0xb8, 0x00);
			GC0329_write_cmos_sensor(0xb9, 0x00);
			GC0329_write_cmos_sensor(0xbd, 0x00);
			GC0329_write_cmos_sensor(0xbe, 0x00);
			GC0329_write_cmos_sensor(0xbf, 0x00);
			GC0329_write_cmos_sensor(0xc3, 0x00);
			GC0329_write_cmos_sensor(0xc4, 0x00);
			GC0329_write_cmos_sensor(0xc5, 0x00);
			GC0329_write_cmos_sensor(0xc9, 0x0d);
			GC0329_write_cmos_sensor(0xca, 0x00);
			GC0329_write_cmos_sensor(0xcb, 0x00);
			
			GC0329_write_cmos_sensor(0xfe, 0x00);
			break;

		case CHT_808C_2:
			GC0329_write_cmos_sensor(0xfe, 0x01);
			GC0329_write_cmos_sensor(0xa0, 0x00);
			GC0329_write_cmos_sensor(0xa1, 0x3c);
			GC0329_write_cmos_sensor(0xa2, 0x50);
			GC0329_write_cmos_sensor(0xa3, 0x00);
			GC0329_write_cmos_sensor(0xa4, 0x00);
			GC0329_write_cmos_sensor(0xa5, 0x02);
			GC0329_write_cmos_sensor(0xa6, 0x00);
			GC0329_write_cmos_sensor(0xa7, 0x00);

			GC0329_write_cmos_sensor(0xa8, 0x0c);
			GC0329_write_cmos_sensor(0xa9, 0x03);
			GC0329_write_cmos_sensor(0xaa, 0x00);
			GC0329_write_cmos_sensor(0xab, 0x05);
			GC0329_write_cmos_sensor(0xac, 0x01);
			GC0329_write_cmos_sensor(0xad, 0x07);
			GC0329_write_cmos_sensor(0xae, 0x0e);
			GC0329_write_cmos_sensor(0xaf, 0x00);
			GC0329_write_cmos_sensor(0xb0, 0x00);
			GC0329_write_cmos_sensor(0xb1, 0x08);
			GC0329_write_cmos_sensor(0xb2, 0x02);
			GC0329_write_cmos_sensor(0xb3, 0x00);

			GC0329_write_cmos_sensor(0xb4, 0x30);
			GC0329_write_cmos_sensor(0xb5, 0x0f);
			GC0329_write_cmos_sensor(0xb6, 0x16);
			GC0329_write_cmos_sensor(0xba, 0x44);
			GC0329_write_cmos_sensor(0xbb, 0x24);
			GC0329_write_cmos_sensor(0xbc, 0x2a);
			GC0329_write_cmos_sensor(0xc0, 0x13);
			GC0329_write_cmos_sensor(0xc1, 0x0e);
			GC0329_write_cmos_sensor(0xc2, 0x11);
			GC0329_write_cmos_sensor(0xc6, 0x28);
			GC0329_write_cmos_sensor(0xc7, 0x21);
			GC0329_write_cmos_sensor(0xc8, 0x20);

			GC0329_write_cmos_sensor(0xb7, 0x00);
			GC0329_write_cmos_sensor(0xb8, 0x00);
			GC0329_write_cmos_sensor(0xb9, 0x01);
			GC0329_write_cmos_sensor(0xbd, 0x00);
			GC0329_write_cmos_sensor(0xbe, 0x00);
			GC0329_write_cmos_sensor(0xbf, 0x00);
			GC0329_write_cmos_sensor(0xc3, 0x00);
			GC0329_write_cmos_sensor(0xc4, 0x00);
			GC0329_write_cmos_sensor(0xc5, 0x00);
			GC0329_write_cmos_sensor(0xc9, 0x00);


			GC0329_write_cmos_sensor(0xca, 0x00);
			GC0329_write_cmos_sensor(0xcb, 0x00);

			GC0329_write_cmos_sensor(0xfe, 0x00);
			break;
			
		case LY_982A_H114:
			GC0329_write_cmos_sensor(0xfe, 0x01);
			GC0329_write_cmos_sensor(0xa0, 0x00);
			GC0329_write_cmos_sensor(0xa1, 0x3c);
			GC0329_write_cmos_sensor(0xa2, 0x50);
			GC0329_write_cmos_sensor(0xa3, 0x00);
			GC0329_write_cmos_sensor(0xa4, 0x00);
			GC0329_write_cmos_sensor(0xa5, 0x00);
			GC0329_write_cmos_sensor(0xa6, 0x00);
			GC0329_write_cmos_sensor(0xa7, 0x00);

			GC0329_write_cmos_sensor(0xa8, 0x0c);
			GC0329_write_cmos_sensor(0xa9, 0x06);
			GC0329_write_cmos_sensor(0xaa, 0x02);
			GC0329_write_cmos_sensor(0xab, 0x13);
			GC0329_write_cmos_sensor(0xac, 0x06);
			GC0329_write_cmos_sensor(0xad, 0x05);
			GC0329_write_cmos_sensor(0xae, 0x0b);
			GC0329_write_cmos_sensor(0xaf, 0x03);
			GC0329_write_cmos_sensor(0xb0, 0x00);
			GC0329_write_cmos_sensor(0xb1, 0x08);
			GC0329_write_cmos_sensor(0xb2, 0x01);
			GC0329_write_cmos_sensor(0xb3, 0x00);

			GC0329_write_cmos_sensor(0xb4, 0x34);
			GC0329_write_cmos_sensor(0xb5, 0x29);
			GC0329_write_cmos_sensor(0xb6, 0x2e);
			GC0329_write_cmos_sensor(0xba, 0x30);
			GC0329_write_cmos_sensor(0xbb, 0x24);
			GC0329_write_cmos_sensor(0xbc, 0x28);
			GC0329_write_cmos_sensor(0xc0, 0x1c);
			GC0329_write_cmos_sensor(0xc1, 0x19);
			GC0329_write_cmos_sensor(0xc2, 0x19);
			GC0329_write_cmos_sensor(0xc6, 0x1a);
			GC0329_write_cmos_sensor(0xc7, 0x19);
			GC0329_write_cmos_sensor(0xc8, 0x1b);

			GC0329_write_cmos_sensor(0xb7, 0x01);
			GC0329_write_cmos_sensor(0xb8, 0x01);
			GC0329_write_cmos_sensor(0xb9, 0x00);
			GC0329_write_cmos_sensor(0xbd, 0x00);
			GC0329_write_cmos_sensor(0xbe, 0x00);
			GC0329_write_cmos_sensor(0xbf, 0x00);
			GC0329_write_cmos_sensor(0xc3, 0x00);
			GC0329_write_cmos_sensor(0xc4, 0x00);
			GC0329_write_cmos_sensor(0xc5, 0x03);
			GC0329_write_cmos_sensor(0xc9, 0x00);
			GC0329_write_cmos_sensor(0xca, 0x00);
			GC0329_write_cmos_sensor(0xcb, 0x00);

			GC0329_write_cmos_sensor(0xfe, 0x00);
			break;

		case XY_046A:
			GC0329_write_cmos_sensor(0xfe, 0x01);
			GC0329_write_cmos_sensor(0xa0, 0x00);
			GC0329_write_cmos_sensor(0xa1, 0x3c);
			GC0329_write_cmos_sensor(0xa2, 0x50);
			GC0329_write_cmos_sensor(0xa3, 0x00);
			GC0329_write_cmos_sensor(0xa4, 0x00);
			GC0329_write_cmos_sensor(0xa5, 0x00);
			GC0329_write_cmos_sensor(0xa6, 0x10);
			GC0329_write_cmos_sensor(0xa7, 0x00);

			GC0329_write_cmos_sensor(0xa8, 0x11);
			GC0329_write_cmos_sensor(0xa9, 0x0a);
			GC0329_write_cmos_sensor(0xaa, 0x05);
			GC0329_write_cmos_sensor(0xab, 0x04);
			GC0329_write_cmos_sensor(0xac, 0x03);
			GC0329_write_cmos_sensor(0xad, 0x00);
			GC0329_write_cmos_sensor(0xae, 0x08);
			GC0329_write_cmos_sensor(0xaf, 0x01);
			GC0329_write_cmos_sensor(0xb0, 0x00);
			GC0329_write_cmos_sensor(0xb1, 0x09);
			GC0329_write_cmos_sensor(0xb2, 0x02);
			GC0329_write_cmos_sensor(0xb3, 0x03);

			GC0329_write_cmos_sensor(0xb4, 0x2e);
			GC0329_write_cmos_sensor(0xb5, 0x16);
			GC0329_write_cmos_sensor(0xb6, 0x24);
			GC0329_write_cmos_sensor(0xba, 0x3a);
			GC0329_write_cmos_sensor(0xbb, 0x1e);
			GC0329_write_cmos_sensor(0xbc, 0x24);
			GC0329_write_cmos_sensor(0xc0, 0x09);
			GC0329_write_cmos_sensor(0xc1, 0x02);
			GC0329_write_cmos_sensor(0xc2, 0x06);
			GC0329_write_cmos_sensor(0xc6, 0x25);
			GC0329_write_cmos_sensor(0xc7, 0x21);
			GC0329_write_cmos_sensor(0xc8, 0x23);

			GC0329_write_cmos_sensor(0xb7, 0x00);
			GC0329_write_cmos_sensor(0xb8, 0x00);
			GC0329_write_cmos_sensor(0xb9, 0x0f);
			GC0329_write_cmos_sensor(0xbd, 0x00);
			GC0329_write_cmos_sensor(0xbe, 0x00);
			GC0329_write_cmos_sensor(0xbf, 0x00);
			GC0329_write_cmos_sensor(0xc3, 0x00);
			GC0329_write_cmos_sensor(0xc4, 0x00);
			GC0329_write_cmos_sensor(0xc5, 0x00);
			GC0329_write_cmos_sensor(0xc9, 0x00);
			GC0329_write_cmos_sensor(0xca, 0x00);
			GC0329_write_cmos_sensor(0xcb, 0x00);

			GC0329_write_cmos_sensor(0xfe, 0x00);
			break;

		case XY_0620:
			GC0329_write_cmos_sensor(0xfe, 0x01);
			GC0329_write_cmos_sensor(0xa0, 0x00);
			GC0329_write_cmos_sensor(0xa1, 0x3c);
			GC0329_write_cmos_sensor(0xa2, 0x50);
			GC0329_write_cmos_sensor(0xa3, 0x00);
			GC0329_write_cmos_sensor(0xa4, 0x00);
			GC0329_write_cmos_sensor(0xa5, 0x00);
			GC0329_write_cmos_sensor(0xa6, 0x00);
			GC0329_write_cmos_sensor(0xa7, 0x00);

			GC0329_write_cmos_sensor(0xa8, 0x0f);
			GC0329_write_cmos_sensor(0xa9, 0x06);
			GC0329_write_cmos_sensor(0xaa, 0x00);
			GC0329_write_cmos_sensor(0xab, 0x07);
			GC0329_write_cmos_sensor(0xac, 0x05);
			GC0329_write_cmos_sensor(0xad, 0x08);
			GC0329_write_cmos_sensor(0xae, 0x13);
			GC0329_write_cmos_sensor(0xaf, 0x06);
			GC0329_write_cmos_sensor(0xb0, 0x00);
			GC0329_write_cmos_sensor(0xb1, 0x06);
			GC0329_write_cmos_sensor(0xb2, 0x01);
			GC0329_write_cmos_sensor(0xb3, 0x04);

			GC0329_write_cmos_sensor(0xb4, 0x2d);
			GC0329_write_cmos_sensor(0xb5, 0x18);
			GC0329_write_cmos_sensor(0xb6, 0x22);
			GC0329_write_cmos_sensor(0xba, 0x45);
			GC0329_write_cmos_sensor(0xbb, 0x2d);
			GC0329_write_cmos_sensor(0xbc, 0x34);
			GC0329_write_cmos_sensor(0xc0, 0x16);
			GC0329_write_cmos_sensor(0xc1, 0x13);
			GC0329_write_cmos_sensor(0xc2, 0x19);

			GC0329_write_cmos_sensor(0xc6, 0x21);
			GC0329_write_cmos_sensor(0xc7, 0x1c);
			GC0329_write_cmos_sensor(0xc8, 0x18);

			GC0329_write_cmos_sensor(0xb7, 0x00);
			GC0329_write_cmos_sensor(0xb8, 0x00);
			GC0329_write_cmos_sensor(0xb9, 0x00);
			GC0329_write_cmos_sensor(0xbd, 0x00);
			GC0329_write_cmos_sensor(0xbe, 0x00);
			GC0329_write_cmos_sensor(0xbf, 0x08);
			GC0329_write_cmos_sensor(0xc3, 0x00);
			GC0329_write_cmos_sensor(0xc4, 0x00);
			GC0329_write_cmos_sensor(0xc5, 0x01);
			GC0329_write_cmos_sensor(0xc9, 0x00);
			GC0329_write_cmos_sensor(0xca, 0x00);
			GC0329_write_cmos_sensor(0xcb, 0x10);

			GC0329_write_cmos_sensor(0xfe, 0x00);
			break;

		case XY_078V: 
			GC0329_write_cmos_sensor(0xfe, 0x01);
			GC0329_write_cmos_sensor(0xa0, 0x00);
			GC0329_write_cmos_sensor(0xa1, 0x3c);
			GC0329_write_cmos_sensor(0xa2, 0x50);
			GC0329_write_cmos_sensor(0xa3, 0x00);
			GC0329_write_cmos_sensor(0xa4, 0x00);
			GC0329_write_cmos_sensor(0xa5, 0x00);
			GC0329_write_cmos_sensor(0xa6, 0x00);
			GC0329_write_cmos_sensor(0xa7, 0x00);

			GC0329_write_cmos_sensor(0xa8, 0x14);
			GC0329_write_cmos_sensor(0xa9, 0x08);
			GC0329_write_cmos_sensor(0xaa, 0x0a);
			GC0329_write_cmos_sensor(0xab, 0x11);
			GC0329_write_cmos_sensor(0xac, 0x05);
			GC0329_write_cmos_sensor(0xad, 0x07);
			GC0329_write_cmos_sensor(0xae, 0x0b);
			GC0329_write_cmos_sensor(0xaf, 0x03);
			GC0329_write_cmos_sensor(0xb0, 0x00);
			GC0329_write_cmos_sensor(0xb1, 0x09);
			GC0329_write_cmos_sensor(0xb2, 0x04);
			GC0329_write_cmos_sensor(0xb3, 0x01);

			GC0329_write_cmos_sensor(0xb4, 0x2f);
			GC0329_write_cmos_sensor(0xb5, 0x2a);
			GC0329_write_cmos_sensor(0xb6, 0x2c);
			GC0329_write_cmos_sensor(0xba, 0x3a);
			GC0329_write_cmos_sensor(0xbb, 0x2b);
			GC0329_write_cmos_sensor(0xbc, 0x32);
			GC0329_write_cmos_sensor(0xc0, 0x1b);
			GC0329_write_cmos_sensor(0xc1, 0x18);
			GC0329_write_cmos_sensor(0xc2, 0x1a);
			GC0329_write_cmos_sensor(0xc6, 0x12);
			GC0329_write_cmos_sensor(0xc7, 0x10);
			GC0329_write_cmos_sensor(0xc8, 0x12);

			GC0329_write_cmos_sensor(0xb7, 0x0a);
			GC0329_write_cmos_sensor(0xb8, 0x00);
			GC0329_write_cmos_sensor(0xb9, 0x00);
			GC0329_write_cmos_sensor(0xbd, 0x00);
			GC0329_write_cmos_sensor(0xbe, 0x00);
			GC0329_write_cmos_sensor(0xbf, 0x00);
			GC0329_write_cmos_sensor(0xc3, 0x00);
			GC0329_write_cmos_sensor(0xc4, 0x00);
			GC0329_write_cmos_sensor(0xc5, 0x00);
			GC0329_write_cmos_sensor(0xc9, 0x0d);
			GC0329_write_cmos_sensor(0xca, 0x00);
			GC0329_write_cmos_sensor(0xcb, 0x00);

			GC0329_write_cmos_sensor(0xfe, 0x00);
			break;

		case YG1001A_F:
			GC0329_write_cmos_sensor(0xfe, 0x01);
			GC0329_write_cmos_sensor(0xa0, 0x00);
			GC0329_write_cmos_sensor(0xa1, 0x3c);
			GC0329_write_cmos_sensor(0xa2, 0x50);
			GC0329_write_cmos_sensor(0xa3, 0x00);
			GC0329_write_cmos_sensor(0xa4, 0x00);
			GC0329_write_cmos_sensor(0xa5, 0x00);
			GC0329_write_cmos_sensor(0xa6, 0x00);
			GC0329_write_cmos_sensor(0xa7, 0x00);

			GC0329_write_cmos_sensor(0xa8, 0x0e);
			GC0329_write_cmos_sensor(0xa9, 0x05);
			GC0329_write_cmos_sensor(0xaa, 0x01);
			GC0329_write_cmos_sensor(0xab, 0x07);
			GC0329_write_cmos_sensor(0xac, 0x00);
			GC0329_write_cmos_sensor(0xad, 0x07);
			GC0329_write_cmos_sensor(0xae, 0x0e);
			GC0329_write_cmos_sensor(0xaf, 0x02);
			GC0329_write_cmos_sensor(0xb0, 0x00);
			GC0329_write_cmos_sensor(0xb1, 0x0d);
			GC0329_write_cmos_sensor(0xb2, 0x00);
			GC0329_write_cmos_sensor(0xb3, 0x00);

			GC0329_write_cmos_sensor(0xb4, 0x2a);
			GC0329_write_cmos_sensor(0xb5, 0x0f);
			GC0329_write_cmos_sensor(0xb6, 0x14);
			GC0329_write_cmos_sensor(0xba, 0x40);
			GC0329_write_cmos_sensor(0xbb, 0x26);
			GC0329_write_cmos_sensor(0xbc, 0x2a);
			GC0329_write_cmos_sensor(0xc0, 0x0e);
			GC0329_write_cmos_sensor(0xc1, 0x0a);
			GC0329_write_cmos_sensor(0xc2, 0x0d);
			GC0329_write_cmos_sensor(0xc6, 0x27);
			GC0329_write_cmos_sensor(0xc7, 0x20);
			GC0329_write_cmos_sensor(0xc8, 0x1f);

			GC0329_write_cmos_sensor(0xb7, 0x00);
			GC0329_write_cmos_sensor(0xb8, 0x00);
			GC0329_write_cmos_sensor(0xb9, 0x00);
			GC0329_write_cmos_sensor(0xbd, 0x00);
			GC0329_write_cmos_sensor(0xbe, 0x00);
			GC0329_write_cmos_sensor(0xbf, 0x00);
			GC0329_write_cmos_sensor(0xc3, 0x00);
			GC0329_write_cmos_sensor(0xc4, 0x00);
			GC0329_write_cmos_sensor(0xc5, 0x00);
			GC0329_write_cmos_sensor(0xc9, 0x00);
			GC0329_write_cmos_sensor(0xca, 0x00);
			GC0329_write_cmos_sensor(0xcb, 0x00);

			GC0329_write_cmos_sensor(0xfe, 0x00);
			GC0329_write_cmos_sensor(0x4f, 0x01);//aec enable
			break;

		default:
			break;
	}
}


/*************************************************************************
* FUNCTION
*	GC0329_GAMMA_Select
*
* DESCRIPTION
*	This function is served for FAE to select the appropriate GAMMA curve.
*
* PARAMETERS
*	None
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/
UINT32 GC0329GetSensorID(UINT32 *sensorID)
{
    kal_uint16 sensor_id=0;
    int i;
	
	SENSORDB("[GC0329]CONTROLFLOW GC0329GetSensorID\n");

    GC0329_write_cmos_sensor(0xfc, 0x16);
    Sleep(20);

    do
    {
        	// check if sensor ID correct
        	for(i = 0; i < 3; i++)
		{
	            	sensor_id = GC0329_read_cmos_sensor(0x00);
	            	printk("GC0329 Sensor id = %x\n", sensor_id);
	            	if (sensor_id == GC0329_SENSOR_ID)
			{
	               	break;
	            	}
        	}
        	mdelay(50);
    }while(0);

    printk("[==CAMERA YCD==] sensor_id = 0x%x", sensor_id);
    if(sensor_id != GC0329_SENSOR_ID)
    {
        SENSORDB("GC0329 Sensor id read failed, ID = %x\n", sensor_id);
		//sensor_id = GC0329_SENSOR_ID;
		*sensorID = 0xffffffff;
        return ERROR_SENSOR_CONNECT_FAIL;
    }

    *sensorID = sensor_id;

    RETAILMSG(1, (TEXT("Sensor Read ID OK \n")));
	
    return ERROR_NONE;
}


/*************************************************************************
* FUNCTION
*	GC0329_Write_More_Registers
*
* DESCRIPTION
*	This function is served for FAE to modify the necessary Init Regs. Do not modify the regs
*     in init_GC0329() directly.
*
* PARAMETERS
*	None
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/
void GC0329_Write_More_Registers(void)
{
    //GC0329_GAMMA_Select(0);//0:use default
    //GC0329_Lens_Select(0);//0:use default
}


/*************************************************************************
 * FUNCTION
 *	GC0329Open
 *
 * DESCRIPTION
 *	This function initialize the registers of CMOS sensor
 *
 * PARAMETERS
 *	None
 *
 * RETURNS
 *	None
 *
 * GLOBALS AFFECTED
 *
 *************************************************************************/
UINT32 GC0329Open(void)
{
    kal_uint16 sensor_id=0;
    int i;

	SENSORDB("[GC0329]CONTROLFLOW GC0329Open\n");

    GC0329_write_cmos_sensor(0xfc, 0x16);
    Sleep(20);

    do
    {
        	// check if sensor ID correct
        	for(i = 0; i < 3; i++)
		{
	            	sensor_id = GC0329_read_cmos_sensor(0x00);
	            	printk("GC0329 Sensor id = %x\n", sensor_id);
	            	if (sensor_id == GC0329_SENSOR_ID)
			{
	               	break;
	            	}
        	}
        	mdelay(50);
    }while(0);

    if(sensor_id != GC0329_SENSOR_ID)
    {
        SENSORDB("GC0329 Sensor id read failed, ID = %x\n", sensor_id);
		//sensor_id = GC0329_SENSOR_ID;
        return ERROR_SENSOR_CONNECT_FAIL;
    }

    RETAILMSG(1, (TEXT("Sensor Read ID OK \n")));
    // initail sequence write in
    GC0329_Sensor_Init();
    GC0329_Write_More_Registers();
	
	spin_lock(&GC0329_drv_lock);
	//GC0329_SensorDriver.SensorMode= SENSOR_MODE_PREVIEW;
	GC0329_night_mode_enable = KAL_FALSE;
	GC0329_MPEG4_encode_mode = KAL_FALSE;
	GC0329_SensorDriver.wb = 0x7e;
	spin_unlock(&GC0329_drv_lock);
    return ERROR_NONE;
} /* GC0329Open */

/*************************************************************************
 * FUNCTION
 *	GC0329Close
 *
 * DESCRIPTION
 *	This function is to turn off sensor module power.
 *
 * PARAMETERS
 *	None
 *
 * RETURNS
 *	None
 *
 * GLOBALS AFFECTED
 *
 *************************************************************************/
UINT32 GC0329Close(void)
{

	SENSORDB("[GC0329]CONTROLFLOW GC0329Close\n");

    return ERROR_NONE;
} /* GC0329Close */


/*************************************************************************
 * FUNCTION
 * GC0329Preview
 *
 * DESCRIPTION
 *	This function start the sensor preview.
 *
 * PARAMETERS
 *	*image_window : address pointer of pixel numbers in one period of HSYNC
 *  *sensor_config_data : address pointer of line numbers in one period of VSYNC
 *
 * RETURNS
 *	None
 *
 * GLOBALS AFFECTED
 *
 *************************************************************************/
UINT32 GC0329Preview(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
        MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)

{
    kal_uint32 iTemp;
    kal_uint16 iStartX = 0, iStartY = 1;

    if(sensor_config_data->SensorOperationMode == MSDK_SENSOR_OPERATION_MODE_VIDEO)		// MPEG4 Encode Mode
    {
		SENSORDB("[GC0329]CONTROLFLOW GC0329Preview Video\n");
		    spin_lock(&GC0329_drv_lock);
        GC0329_MPEG4_encode_mode = KAL_TRUE;
    spin_unlock(&GC0329_drv_lock);
       
    }
    else
    {
		SENSORDB("[GC0329]CONTROLFLOW GC0329Preview camera\n");
		    spin_lock(&GC0329_drv_lock);
        GC0329_MPEG4_encode_mode = KAL_FALSE;
    spin_unlock(&GC0329_drv_lock);
    }

    image_window->GrabStartX= IMAGE_SENSOR_VGA_GRAB_PIXELS;
    image_window->GrabStartY= IMAGE_SENSOR_VGA_GRAB_LINES;
    image_window->ExposureWindowWidth = IMAGE_SENSOR_PV_WIDTH;
    image_window->ExposureWindowHeight =IMAGE_SENSOR_PV_HEIGHT;

    // copy sensor_config_data
    memcpy(&GC0329SensorConfigData, sensor_config_data, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
	GC0329_night_mode(GC0329_night_mode_enable);
    return ERROR_NONE;
} /* GC0329Preview */


/*************************************************************************
 * FUNCTION
 *	GC0329Capture
 *
 * DESCRIPTION
 *	This function setup the CMOS sensor in capture MY_OUTPUT mode
 *
 * PARAMETERS
 *
 * RETURNS
 *	None
 *
 * GLOBALS AFFECTED
 *
 *************************************************************************/
UINT32 GC0329Capture(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
        MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)

{

		    spin_lock(&GC0329_drv_lock);
    GC0329_MODE_CAPTURE=KAL_TRUE;
        GC0329_MPEG4_encode_mode = KAL_FALSE;
    spin_unlock(&GC0329_drv_lock);

	
	SENSORDB("[GC0329]CONTROLFLOW GC0329Capture\n");

    image_window->GrabStartX = IMAGE_SENSOR_VGA_GRAB_PIXELS;
    image_window->GrabStartY = IMAGE_SENSOR_VGA_GRAB_LINES;
    image_window->ExposureWindowWidth= IMAGE_SENSOR_FULL_WIDTH;
    image_window->ExposureWindowHeight = IMAGE_SENSOR_FULL_HEIGHT;


    // copy sensor_config_data
    memcpy(&GC0329SensorConfigData, sensor_config_data, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
	//GC0329_night_mode(GC0329_night_mode_enable);
    return ERROR_NONE;
} /* GC0329_Capture() */



UINT32 GC0329GetResolution(MSDK_SENSOR_RESOLUTION_INFO_STRUCT *pSensorResolution)
{
    pSensorResolution->SensorFullWidth=IMAGE_SENSOR_FULL_WIDTH;
    pSensorResolution->SensorFullHeight=IMAGE_SENSOR_FULL_HEIGHT;
    pSensorResolution->SensorPreviewWidth=IMAGE_SENSOR_PV_WIDTH;
    pSensorResolution->SensorPreviewHeight=IMAGE_SENSOR_PV_HEIGHT;
    pSensorResolution->SensorVideoWidth=IMAGE_SENSOR_PV_WIDTH;
    pSensorResolution->SensorVideoHeight=IMAGE_SENSOR_PV_HEIGHT;
    return ERROR_NONE;
} /* GC0329GetResolution() */


UINT32 GC0329GetInfo(MSDK_SCENARIO_ID_ENUM ScenarioId,
        MSDK_SENSOR_INFO_STRUCT *pSensorInfo,
        MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData)
{
    pSensorInfo->SensorPreviewResolutionX=IMAGE_SENSOR_PV_WIDTH;
    pSensorInfo->SensorPreviewResolutionY=IMAGE_SENSOR_PV_HEIGHT;
    pSensorInfo->SensorFullResolutionX=IMAGE_SENSOR_FULL_WIDTH;
    pSensorInfo->SensorFullResolutionY=IMAGE_SENSOR_FULL_WIDTH;

    pSensorInfo->SensorCameraPreviewFrameRate=30;
    pSensorInfo->SensorVideoFrameRate=30;
    pSensorInfo->SensorStillCaptureFrameRate=10;
    pSensorInfo->SensorWebCamCaptureFrameRate=15;
    pSensorInfo->SensorResetActiveHigh=FALSE;
    pSensorInfo->SensorResetDelayCount=1;
    pSensorInfo->SensorOutputDataFormat=SENSOR_OUTPUT_FORMAT_YUYV;
    pSensorInfo->SensorClockPolarity=SENSOR_CLOCK_POLARITY_LOW;
    pSensorInfo->SensorClockFallingPolarity=SENSOR_CLOCK_POLARITY_LOW;
    pSensorInfo->SensorHsyncPolarity = SENSOR_CLOCK_POLARITY_LOW;
    pSensorInfo->SensorVsyncPolarity = SENSOR_CLOCK_POLARITY_LOW;
    pSensorInfo->SensorInterruptDelayLines = 1;
    pSensorInfo->SensroInterfaceType=SENSOR_INTERFACE_TYPE_PARALLEL;

#if 0
    pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_100_MODE].MaxWidth=CAM_SIZE_5M_WIDTH;
    pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_100_MODE].MaxHeight=CAM_SIZE_5M_HEIGHT;
    pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_100_MODE].ISOSupported=TRUE;
    pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_100_MODE].BinningEnable=FALSE;

    pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_200_MODE].MaxWidth=CAM_SIZE_5M_WIDTH;
    pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_200_MODE].MaxHeight=CAM_SIZE_5M_HEIGHT;
    pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_200_MODE].ISOSupported=TRUE;
    pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_200_MODE].BinningEnable=FALSE;

    pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_400_MODE].MaxWidth=CAM_SIZE_5M_WIDTH;
    pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_400_MODE].MaxHeight=CAM_SIZE_5M_HEIGHT;
    pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_400_MODE].ISOSupported=TRUE;
    pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_400_MODE].BinningEnable=FALSE;

    pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_800_MODE].MaxWidth=CAM_SIZE_1M_WIDTH;
    pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_800_MODE].MaxHeight=CAM_SIZE_1M_HEIGHT;
    pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_800_MODE].ISOSupported=TRUE;
    pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_800_MODE].BinningEnable=FALSE;

    pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_1600_MODE].MaxWidth=CAM_SIZE_1M_WIDTH;
    pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_1600_MODE].MaxHeight=CAM_SIZE_1M_HEIGHT;
    pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_1600_MODE].ISOSupported=TRUE;
    pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_1600_MODE].BinningEnable=FALSE;
 #endif
    pSensorInfo->CaptureDelayFrame = 3;
    pSensorInfo->PreviewDelayFrame = 14; // 12
    pSensorInfo->VideoDelayFrame = 4;
    pSensorInfo->SensorMasterClockSwitch = 0;
    pSensorInfo->SensorDrivingCurrent = ISP_DRIVING_4MA;

    switch (ScenarioId)
    {
    case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
    case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
   // case MSDK_SCENARIO_ID_VIDEO_CAPTURE_MPEG4:
        pSensorInfo->SensorClockFreq=26;
        pSensorInfo->SensorClockDividCount=	3;  
        pSensorInfo->SensorClockRisingCount= 0;
        pSensorInfo->SensorClockFallingCount= 2;
        pSensorInfo->SensorPixelClockCount= 3;
        pSensorInfo->SensorDataLatchCount= 2;
        pSensorInfo->SensorGrabStartX = IMAGE_SENSOR_VGA_GRAB_PIXELS+3;
        pSensorInfo->SensorGrabStartY = IMAGE_SENSOR_VGA_GRAB_LINES;

        break;
    case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
    //case MSDK_SCENARIO_ID_CAMERA_CAPTURE_MEM:
        pSensorInfo->SensorClockFreq=26;
        pSensorInfo->SensorClockDividCount= 3;
        pSensorInfo->SensorClockRisingCount=0;
        pSensorInfo->SensorClockFallingCount=2;
        pSensorInfo->SensorPixelClockCount=3;
        pSensorInfo->SensorDataLatchCount=2;
        pSensorInfo->SensorGrabStartX = IMAGE_SENSOR_VGA_GRAB_PIXELS+3;
        pSensorInfo->SensorGrabStartY = IMAGE_SENSOR_VGA_GRAB_LINES;
        break;
    default:
        pSensorInfo->SensorClockFreq=26;
        pSensorInfo->SensorClockDividCount= 3;
        pSensorInfo->SensorClockRisingCount=0;
        pSensorInfo->SensorClockFallingCount=2;
        pSensorInfo->SensorPixelClockCount=3;
        pSensorInfo->SensorDataLatchCount=2;
        pSensorInfo->SensorGrabStartX = IMAGE_SENSOR_VGA_GRAB_PIXELS+3;
        pSensorInfo->SensorGrabStartY = IMAGE_SENSOR_VGA_GRAB_LINES;
        break;
    }
    GC0329PixelClockDivider=pSensorInfo->SensorPixelClockCount;
    memcpy(pSensorConfigData, &GC0329SensorConfigData, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
    return ERROR_NONE;
} /* GC0329GetInfo() */


UINT32 GC0329Control(MSDK_SCENARIO_ID_ENUM ScenarioId, MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *pImageWindow,
        MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData)
{

	SENSORDB("[GC0329]CONTROLFLOW GC0329Control ScenarioId = %d\n", ScenarioId); 

	spin_lock(&GC0329_drv_lock);
	GC0329_CurrentScenarioId = ScenarioId;
	spin_unlock(&GC0329_drv_lock);

    switch (ScenarioId)
    {
    case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
    case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
    //case MSDK_SCENARIO_ID_VIDEO_CAPTURE_MPEG4:
        GC0329Preview(pImageWindow, pSensorConfigData);
        break;
    case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
	case MSDK_SCENARIO_ID_CAMERA_ZSD:
   // case MSDK_SCENARIO_ID_CAMERA_CAPTURE_MEM:
        GC0329Capture(pImageWindow, pSensorConfigData);
        break;
    }


    return ERROR_NONE;
}	/* GC0329Control() */

BOOL GC0329_set_param_wb(UINT16 para)
{

	SENSORDB("[GC0329]CONTROLFLOW GC0329_set_param_wb para = %d\n", para); 

	switch (para)
	{
		case AWB_MODE_OFF:
		
			GC0329_awb_enable(KAL_FALSE);
			GC0329_write_cmos_sensor(0x77, 0x57);
			GC0329_write_cmos_sensor(0x78, 0x4d);
			GC0329_write_cmos_sensor(0x79, 0x45);
			
		break;
		
		case AWB_MODE_AUTO:
		default:
			GC0329_write_cmos_sensor(0x77, 0x57);
			GC0329_write_cmos_sensor(0x78, 0x4d);
			GC0329_write_cmos_sensor(0x79, 0x45);
			GC0329_awb_enable(KAL_TRUE);
		break;
		
		case AWB_MODE_CLOUDY_DAYLIGHT: //cloudy,D75
			GC0329_awb_enable(KAL_FALSE);
			GC0329_write_cmos_sensor(0x77, 0x7a); //8c  //WB_manual_gain 
			GC0329_write_cmos_sensor(0x78, 0x5a); //50
			GC0329_write_cmos_sensor(0x79, 0x40);
		break;
		
		case AWB_MODE_DAYLIGHT: //sunny,D65
			GC0329_awb_enable(KAL_FALSE);
			GC0329_write_cmos_sensor(0x77, 0x70); //55//74 6f
			GC0329_write_cmos_sensor(0x78, 0x58); //44//52
			GC0329_write_cmos_sensor(0x79, 0x40); //48			
		break;
		
		case AWB_MODE_INCANDESCENT: //office,TL84
			GC0329_awb_enable(KAL_FALSE);
			GC0329_write_cmos_sensor(0x77, 0x40);//48
			GC0329_write_cmos_sensor(0x78, 0x43);//40
			GC0329_write_cmos_sensor(0x79, 0x45);//5c
		break;
		
		case AWB_MODE_TUNGSTEN: //home,A
			GC0329_awb_enable(KAL_FALSE);
			GC0329_write_cmos_sensor(0x77, 0x40);//40
			GC0329_write_cmos_sensor(0x78, 0x51);//54
			GC0329_write_cmos_sensor(0x79, 0x65);//70
		break;
		
		case AWB_MODE_FLUORESCENT://CWF
			GC0329_awb_enable(KAL_FALSE);
			GC0329_write_cmos_sensor(0x77, 0x46);//40
			GC0329_write_cmos_sensor(0x78, 0x40);//42
			GC0329_write_cmos_sensor(0x79, 0x45);//50
		break;
		
		//default:
		//return FALSE;
	}

	return TRUE;
} /* GC0329_set_param_wb */


BOOL GC0329_set_param_effect(UINT16 para)
{
	kal_uint32  ret = KAL_TRUE;

	SENSORDB("[GC0329]CONTROLFLOW GC0329_set_param_effect para = %d\n", para); 

	switch (para)
	{
		case MEFFECT_OFF:
		default:
			GC0329_write_cmos_sensor(0x43 , 0x00);
		break;

		case MEFFECT_SEPIA:
			GC0329_write_cmos_sensor(0x43 , 0x02);
			GC0329_write_cmos_sensor(0xda , 0xd0);
			GC0329_write_cmos_sensor(0xdb , 0x28);
		break;
		
		case MEFFECT_NEGATIVE:
			GC0329_write_cmos_sensor(0x43 , 0x01);
		break;
		
		case MEFFECT_SEPIAGREEN:
			GC0329_write_cmos_sensor(0x43 , 0x02);
			GC0329_write_cmos_sensor(0xda , 0xc0);
			GC0329_write_cmos_sensor(0xdb , 0xc0);
		break;
		
		case MEFFECT_SEPIABLUE:
			GC0329_write_cmos_sensor(0x43 , 0x02);
			GC0329_write_cmos_sensor(0xda , 0x50);
			GC0329_write_cmos_sensor(0xdb , 0xe0);
		break;

		case MEFFECT_MONO:
			GC0329_write_cmos_sensor(0x43 , 0x02);
			GC0329_write_cmos_sensor(0xda , 0x00);
			GC0329_write_cmos_sensor(0xdb , 0x00);
		break;
		//default:
		//	ret = FALSE;
	}

	return ret;

} /* GC0329_set_param_effect */


BOOL GC0329_set_param_banding(UINT16 para)
{

	SENSORDB("[GC0329]CONTROLFLOW GC0329_set_param_banding para = %d\n", para); 

	switch (para)
	{
		case AE_FLICKER_MODE_50HZ:
			GC0329_write_cmos_sensor(0x05, 0x00); 	
			GC0329_write_cmos_sensor(0x06, 0x59);
			GC0329_write_cmos_sensor(0x07, 0x00);
			GC0329_write_cmos_sensor(0x08, 0x15);
			
			GC0329_SET_PAGE1;
			GC0329_write_cmos_sensor(0x29, 0x00);   //anti-flicker step [11:8]
			GC0329_write_cmos_sensor(0x2a, 0xa6);   //anti-flicker step [7:0]
				
			GC0329_write_cmos_sensor(0x2b, 0x01);   //exp level 0  30.00fps
			GC0329_write_cmos_sensor(0x2c, 0xf2); 
			GC0329_write_cmos_sensor(0x2d, 0x04);   //exp level 1  14.29fps
			GC0329_write_cmos_sensor(0x2e, 0x8a); 
			GC0329_write_cmos_sensor(0x2f, 0x06);   //exp level 2  10.00fps
			GC0329_write_cmos_sensor(0x30, 0x7c); 
			GC0329_write_cmos_sensor(0x31, 0x0a);   //exp level 3  5.26fps
			GC0329_write_cmos_sensor(0x32, 0x60); 
			GC0329_write_cmos_sensor(0xfe, 0x00);
			GC0329_SET_PAGE0;

			GC0329_CAM_BANDING_50HZ = KAL_TRUE;
			break;

		case AE_FLICKER_MODE_60HZ:
			GC0329_write_cmos_sensor(0x05, 0x00);
			GC0329_write_cmos_sensor(0x06, 0x5b);//30
			GC0329_write_cmos_sensor(0x07, 0x00);
			GC0329_write_cmos_sensor(0x08, 0x21);

			
			GC0329_SET_PAGE1;
			GC0329_write_cmos_sensor(0x29, 0x00);   //anti-flicker step [11:8]
			GC0329_write_cmos_sensor(0x2a, 0x8a);   //anti-flicker step [7:0]
				
			GC0329_write_cmos_sensor(0x2b, 0x01);   //exp level 0  30.00fps
			GC0329_write_cmos_sensor(0x2c, 0x9e); 
			GC0329_write_cmos_sensor(0x2d, 0x04);   //exp level 0  15.00fps
			GC0329_write_cmos_sensor(0x2e, 0x50); 
			GC0329_write_cmos_sensor(0x2f, 0x06);   //exp level 0  10.00fps
			GC0329_write_cmos_sensor(0x30, 0x78); 
			GC0329_write_cmos_sensor(0x31, 0x09);   //exp level 0  5.00fps
			GC0329_write_cmos_sensor(0x32, 0xb4); 
			GC0329_write_cmos_sensor(0xfe, 0x00);
			GC0329_SET_PAGE0;

			GC0329_CAM_BANDING_50HZ = KAL_FALSE;
		break;
		case AE_FLICKER_MODE_AUTO:
		case AE_FLICKER_MODE_OFF:
		default:
			GC0329_write_cmos_sensor(0x05, 0x00); 	
			GC0329_write_cmos_sensor(0x06, 0x59);
			GC0329_write_cmos_sensor(0x07, 0x00);
			GC0329_write_cmos_sensor(0x08, 0x15);
			
			GC0329_SET_PAGE1;
			GC0329_write_cmos_sensor(0x29, 0x00);   //anti-flicker step [11:8]
			GC0329_write_cmos_sensor(0x2a, 0xa6);   //anti-flicker step [7:0]
				
			GC0329_write_cmos_sensor(0x2b, 0x01);   //exp level 0  30.00fps
			GC0329_write_cmos_sensor(0x2c, 0xf2); 
			GC0329_write_cmos_sensor(0x2d, 0x04);   //exp level 1  14.29fps
			GC0329_write_cmos_sensor(0x2e, 0x8a); 
			GC0329_write_cmos_sensor(0x2f, 0x06);   //exp level 2  10.00fps
			GC0329_write_cmos_sensor(0x30, 0x7c); 
			GC0329_write_cmos_sensor(0x31, 0x0a);   //exp level 3  5.26fps
			GC0329_write_cmos_sensor(0x32, 0x60); 
			GC0329_write_cmos_sensor(0xfe, 0x00);
			GC0329_SET_PAGE0;
			GC0329_CAM_BANDING_50HZ = KAL_TRUE;
			break;
		//return FALSE;
	}
	
    //	Sleep(500);
	return TRUE;
} /* GC0329_set_param_banding */

BOOL GC0329_set_param_exposure_for_HDR(UINT16 para)
{
	
	SENSORDB("[GC0329]CONTROLFLOW GC0329_set_param_exposure_for_HDR\n ");

	return TRUE;
}

BOOL GC0329_set_param_exposure(UINT16 para)
{

	 SENSORDB("[GC0329]CONTROLFLOW enter GC0329_set_param_exposure function:\n ");
	 SENSORDB("[GC0329]para=%d:\n",para);
	 //spin_lock(&GC0329_drv_lock);
/*	if (SCENE_MODE_HDR == GC0329Sensor.sceneMode && 
	 SENSOR_MODE_CAPTURE == GC0329Sensor.SensorMode)
	{
		//spin_unlock(&GC0329_drv_lock);
		GC0329_set_param_exposure_for_HDR(para);
		return TRUE;
	}*/


	switch (para)
	{
		case AE_EV_COMP_n20:
			GC0329_write_cmos_sensor(0xd5, 0x90);
		//	GC0329_SET_PAGE1;
		//	GC0329_write_cmos_sensor(0x13, 0x30);
			GC0329_SET_PAGE0;
		break;
		case AE_EV_COMP_n13:
			GC0329_write_cmos_sensor(0xd5, 0xc0);
		//	GC0329_SET_PAGE1;
		//	GC0329_write_cmos_sensor(0x13, 0x30);
			GC0329_SET_PAGE0;
		break;
		
		case AE_EV_COMP_n10:
			GC0329_write_cmos_sensor(0xd5, 0xd0);
		//	GC0329_SET_PAGE1;
		//	GC0329_write_cmos_sensor(0x13, 0x38);
			GC0329_SET_PAGE0;
		break;
		
		case AE_EV_COMP_n07:
			GC0329_write_cmos_sensor(0xd5, 0xe0);
		//	GC0329_SET_PAGE1;
		//	GC0329_write_cmos_sensor(0x13, 0x40);
			GC0329_SET_PAGE0;
		break;
		
		case AE_EV_COMP_n03:
			GC0329_write_cmos_sensor(0xd5, 0xf0);
		//	GC0329_SET_PAGE1;
		//	GC0329_write_cmos_sensor(0x13, 0x48);
			GC0329_SET_PAGE0;
		break;
		
		case AE_EV_COMP_00:
		default:
			GC0329_write_cmos_sensor(0xd5, 0x00);
		//	GC0329_SET_PAGE1;
		//	GC0329_write_cmos_sensor(0x13, 0x60);
			GC0329_SET_PAGE0;
		break;

		case AE_EV_COMP_03:
			GC0329_write_cmos_sensor(0xd5, 0x10);
		//	GC0329_SET_PAGE1;
		//	GC0329_write_cmos_sensor(0x13, 0x60);
			GC0329_SET_PAGE0;
		break;
		
		case AE_EV_COMP_07:
			GC0329_write_cmos_sensor(0xd5, 0x20);
		//	GC0329_SET_PAGE1;
		//	GC0329_write_cmos_sensor(0x13, 0x70);
			GC0329_SET_PAGE0;
		break;
		
		case AE_EV_COMP_10:
			GC0329_write_cmos_sensor(0xd5, 0x30);
		//	GC0329_SET_PAGE1;
		//	GC0329_write_cmos_sensor(0x13, 0x80);
			GC0329_SET_PAGE0;
		break;
		
		case AE_EV_COMP_13:
			GC0329_write_cmos_sensor(0xd5, 0x40);
		//	GC0329_SET_PAGE1;
		//	GC0329_write_cmos_sensor(0x13, 0x90);
			GC0329_SET_PAGE0;
		break;
		case AE_EV_COMP_20:
			GC0329_write_cmos_sensor(0xd5, 0x70);
		//	GC0329_SET_PAGE1;
		//	GC0329_write_cmos_sensor(0x13, 0x90);
			GC0329_SET_PAGE0;
		break;
		//default:
		//return FALSE;
	}

	return TRUE;
} /* GC0329_set_param_exposure */
///add
void GC0329_set_contrast(UINT16 para)
{   
    SENSORDB("[GC0329]CONTROLFLOW enter GC0329_set_contrast function:\n ");
    switch (para)
    {
        case ISP_CONTRAST_LOW:			 
			GC0329_write_cmos_sensor(0xfe, 0x00);	 
			GC0329_write_cmos_sensor(0xd3, 0x30);
			GC0329_write_cmos_sensor(0xfe, 0x00); 
			break;
        case ISP_CONTRAST_HIGH:			 
			GC0329_write_cmos_sensor(0xfe, 0x00); 	
			GC0329_write_cmos_sensor(0xd3, 0x60);
			GC0329_write_cmos_sensor(0xfe, 0x00);
			break;
        case ISP_CONTRAST_MIDDLE: 
        default:
			GC0329_write_cmos_sensor(0xfe, 0x00);	 
			GC0329_write_cmos_sensor(0xd3, 0x40);
			GC0329_write_cmos_sensor(0xfe, 0x00);	 
			break;
        //default:
		//	break;
    }
    SENSORDB("[GC0329]exit GC0329_set_contrast function:\n ");
    return;
}

void GC0329_set_brightness(UINT16 para)
{
    SENSORDB("[GC0329]CONTROLFLOW enter GC0329_set_brightness function:\n ");
	//return;
    switch (para)
    {
        case ISP_BRIGHT_LOW:
		//case AE_EV_COMP_n13:
		//	GC0329_write_cmos_sensor(0xd5, 0xc0);
			GC0329_SET_PAGE1;
			GC0329_write_cmos_sensor(0x13, 0x30);
			GC0329_SET_PAGE0;
		break;
        case ISP_BRIGHT_HIGH:
		//case AE_EV_COMP_13:
		//	GC0329_write_cmos_sensor(0xd5, 0x40);
			GC0329_SET_PAGE1;
			GC0329_write_cmos_sensor(0x13, 0x90);
			GC0329_SET_PAGE0;
			break;
        case ISP_BRIGHT_MIDDLE:
        default:
		//case AE_EV_COMP_00:
		//	GC0329_write_cmos_sensor(0xd5, 0x00);
			GC0329_SET_PAGE1;
			GC0329_write_cmos_sensor(0x13, 0x60);
			GC0329_SET_PAGE0;
		break;
		//	return KAL_FALSE;
		//	break;
    }
    SENSORDB("[GC0329]exit GC0329_set_brightness function:\n ");
    return;
}
void GC0329_set_saturation(UINT16 para)
{
	SENSORDB("[GC0329]CONTROLFLOW enter GC0329_set_saturation function:\n ");
    switch (para)
    {
        case ISP_SAT_HIGH:
			GC0329_write_cmos_sensor(0xfe, 0x00); 	
			GC0329_write_cmos_sensor(0xd0, 0x60);
			GC0329_write_cmos_sensor(0xfe, 0x00);
			break;
        case ISP_SAT_LOW:
			GC0329_write_cmos_sensor(0xfe, 0x00); 	
			GC0329_write_cmos_sensor(0xd0, 0x30);
			GC0329_write_cmos_sensor(0xfe, 0x00);
			break;
        case ISP_SAT_MIDDLE:
        default:
			GC0329_write_cmos_sensor(0xfe, 0x00); 	
			GC0329_write_cmos_sensor(0xd0, 0x40);
			GC0329_write_cmos_sensor(0xfe, 0x00);
			break;
		//	return KAL_FALSE;
		//	break;
    }
	SENSORDB("[GC0329]exit GC0329_set_saturation function:\n ");
     return;
}
void GC0329_set_scene_mode(UINT16 para)
{
	SENSORDB("[GC0329]CONTROLFLOW enter GC0329_set_scene_mode function:\n ");
	SENSORDB("[GC0329] GC0329_set_scene_mode=%d",para);	
	spin_lock(&GC0329_drv_lock);
	GC0329_SensorDriver.sceneMode=para;
	spin_unlock(&GC0329_drv_lock);
    switch (para)
    { 

		case SCENE_MODE_NIGHTSCENE:
          	GC0329_night_mode(KAL_TRUE); 
			break;
        case SCENE_MODE_PORTRAIT:
			/*Night mode disable*/
          	GC0329_night_mode(KAL_FALSE); 
			GC0329_write_cmos_sensor(0xfe, 0x01); 	
			GC0329_write_cmos_sensor(0x67, 0x70);
			GC0329_write_cmos_sensor(0x68, 0x58);
			GC0329_write_cmos_sensor(0x69, 0x70);
			GC0329_write_cmos_sensor(0xfe, 0x00); 
            break;
        case SCENE_MODE_LANDSCAPE:
			/*Night mode disable*/
          	GC0329_night_mode(KAL_FALSE); 
			GC0329_write_cmos_sensor(0xfe, 0x00); 
			GC0329_write_cmos_sensor(0xd0, 0x48);
			GC0329_write_cmos_sensor(0xfe, 0x00);
	 
             break;
        case SCENE_MODE_SUNSET:
			/*Night mode disable*/
          	GC0329_night_mode(KAL_FALSE); 
			GC0329_write_cmos_sensor(0xfe, 0x00); 
			GC0329_write_cmos_sensor(0xd1, 0x50);
			GC0329_write_cmos_sensor(0xd2, 0x40);	
			GC0329_write_cmos_sensor(0xfe, 0x00); 
	 
            break;
        case SCENE_MODE_SPORTS:
             /*Night mode disable*/
          	GC0329_night_mode(KAL_FALSE); 
			GC0329_write_cmos_sensor(0xfe, 0x01); 	
			GC0329_write_cmos_sensor(0x33, 0x10);
			GC0329_write_cmos_sensor(0xfe, 0x00); 
	 
            break;
        case SCENE_MODE_BEACH:
        case SCENE_MODE_SNOW:
          	GC0329_night_mode(KAL_FALSE); 			
			GC0329_write_cmos_sensor(0xfe, 0x00); 
			GC0329_write_cmos_sensor(0xd1, 0x46);
			GC0329_write_cmos_sensor(0xd2, 0x40);	
			GC0329_write_cmos_sensor(0xfe, 0x00); 
            break;
        case SCENE_MODE_HDR:
        //    if (1 == GC0329_SensorDriver.manualAEStart)
            {
                GC0329_set_AE_mode(KAL_TRUE);//Manual AE disable
                spin_lock(&GC0329_drv_lock);
            	//GC0329_SensorDriver.manualAEStart = 0;
                //GC0329_SensorDriver.currentExposureTime = 0;
                //GC0329_SensorDriver.currentAxDGain = 0;
				spin_unlock(&GC0329_drv_lock);
            }
            break;
        case SCENE_MODE_OFF:
        default:
			 /*Night mode disable*/
	   /*	 GC0329_write_cmos_sensor(0xfe, 0x01);	 
			 GC0329_write_cmos_sensor(0x67, 0x70);
			 GC0329_write_cmos_sensor(0x68, 0x58);
			 GC0329_write_cmos_sensor(0x69, 0x85);
			 GC0329_write_cmos_sensor(0xfe, 0x00); 
			 */
/*GC0329_write_cmos_sensor(0xfe, 0x01); 	
GC0329_write_cmos_sensor(0x67, 0x70);
GC0329_write_cmos_sensor(0x68, 0x58);
GC0329_write_cmos_sensor(0x69, 0x85);
GC0329_write_cmos_sensor(0xfe, 0x00); */
          	GC0329_night_mode(KAL_FALSE); 
			break;
		//	return KAL_FALSE;
        //    break;
    }
	SENSORDB("[GC0329]exit GC0329_set_scene_mode function:\n ");
	//if( GC0329_SensorDriver.sceneMode != SCENE_MODE_NIGHTSCENE) && ( GC0329_SensorDriver.sceneMode != SCENE_MODE_NIGHTSCENE)
	return;
}
void GC0329_set_iso(UINT16 para)
{
    spin_lock(&GC0329_drv_lock);
    GC0329_SensorDriver.isoSpeed = para;
    spin_unlock(&GC0329_drv_lock);   

	SENSORDB("[GC0329]CONTROLFLOW GC0329_set_iso:\n ");

    switch (para)
	{
        case AE_ISO_100:
             //ISO 100
			GC0329_write_cmos_sensor(0xfe, 0x01); 	
			GC0329_write_cmos_sensor(0x21, 0x40);
			GC0329_write_cmos_sensor(0x22, 0x40);
			GC0329_write_cmos_sensor(0xfe, 0x00);
			break;
        case AE_ISO_200:
             //ISO 200
			GC0329_write_cmos_sensor(0xfe, 0x01); 	
			GC0329_write_cmos_sensor(0x21, 0x60);
			GC0329_write_cmos_sensor(0x22, 0x40);
			GC0329_write_cmos_sensor(0xfe, 0x00);
			break;
        case AE_ISO_400:
             //ISO 400
			GC0329_write_cmos_sensor(0xfe, 0x01); 	
			GC0329_write_cmos_sensor(0x21, 0xC0);
			GC0329_write_cmos_sensor(0x22, 0x40);
			GC0329_write_cmos_sensor(0xfe, 0x00);
			break;
		case AE_ISO_AUTO:
			default:
/*GC0329_write_cmos_sensor(0xfe, 0x01); 
		GC0329_write_cmos_sensor(0x21, 0xb0);
		GC0329_write_cmos_sensor(0x22, 0x60);
GC0329_write_cmos_sensor(0xfe, 0x00);*/
			if (GC0329_night_mode_enable)
			{
				GC0329_write_cmos_sensor(0xfe, 0x01);
				GC0329_write_cmos_sensor(0x21, 0xb0);
				GC0329_write_cmos_sensor(0x22, 0x60);
				GC0329_write_cmos_sensor(0xfe, 0x00);
			}
			else 
			{
				GC0329_write_cmos_sensor(0xfe, 0x01);		
				GC0329_write_cmos_sensor(0x21, 0xb0);
				GC0329_write_cmos_sensor(0x22, 0x48);
				GC0329_write_cmos_sensor(0xfe, 0x00);
		    }
			break;
	}
    return;
}

UINT32 GC0329YUVSetVideoMode(UINT16 u2FrameRate)    // lanking add
{
  
		    spin_lock(&GC0329_drv_lock);
        GC0329_MPEG4_encode_mode = KAL_TRUE;
    spin_unlock(&GC0329_drv_lock);
	SENSORDB("[GC0329]CONTROLFLOW GC0329 Frame Rate= %d\n", u2FrameRate); 
     if (u2FrameRate == 30)
   	{
   	    /*********video frame ************/
		
   	}
    else if (u2FrameRate == 15)       
    	{
    	
   	    /*********video frame ************/
		
    	}
    else
   	{
   	
            SENSORDB("GC0329 wrong Frame Rate= %d\n", u2FrameRate); 
			
   	}

	  GC0329_night_mode(GC0329_night_mode_enable);

      return TRUE;

}


UINT32 GC0329YUVSensorSetting(FEATURE_ID iCmd, UINT16 iPara)
{
 /*   switch (iCmd) {
    case FID_AWB_MODE:
        GC0329_set_param_wb(iPara);
        break;
    case FID_COLOR_EFFECT:
        GC0329_set_param_effect(iPara);
        break;
    case FID_AE_EV:
        GC0329_set_param_exposure(iPara);
        break;
    case FID_AE_FLICKER:
        GC0329_set_param_banding(iPara);
        break;
	case FID_SCENE_MODE:
		GC0329_night_mode(iPara);
		break;
		
    default:
        break;
    }*/
	
		switch (iCmd) {
			case FID_SCENE_MODE:
				GC0329_set_scene_mode(iPara);
				break;		
			case FID_AWB_MODE:
					GC0329_set_param_wb(iPara);
				  break;
			case FID_COLOR_EFFECT:				
					GC0329_set_param_effect(iPara);
				  break;
			case FID_AE_EV:   
					GC0329_set_param_exposure(iPara);
				break;
			case FID_AE_FLICKER:					
					GC0329_set_param_banding(iPara);
				  break;
			case FID_AE_SCENE_MODE: 
				if (iPara == AE_MODE_OFF) 
				{
					spin_lock(&GC0329_drv_lock);
					GC0329_SensorDriver.AE_ENABLE = KAL_FALSE; 
					spin_unlock(&GC0329_drv_lock);
				}
				else 
				{
					spin_lock(&GC0329_drv_lock);
					GC0329_SensorDriver.AE_ENABLE = KAL_TRUE; 
					spin_unlock(&GC0329_drv_lock);
				}
				GC0329_set_AE_mode(GC0329_SensorDriver.AE_ENABLE);
			break; 
			case FID_ISP_CONTRAST:
				GC0329_set_contrast(iPara);
				break;
			case FID_ISP_BRIGHT:
				GC0329_set_brightness(iPara);
				break;
			case FID_ISP_SAT:
				GC0329_set_saturation(iPara);
			break; 
		case FID_ZOOM_FACTOR:
				SENSORDB("FID_ZOOM_FACTOR:%d\n", iPara);		
				spin_lock(&GC0329_drv_lock);
				GC0329_zoom_factor = iPara; 
				spin_unlock(&GC0329_drv_lock);
				break; 
			case FID_AE_ISO:
				GC0329_set_iso(iPara);
				break;
#if 0 //afc
			case FID_AF_MODE:
				 GC0329_set_param_afmode(iPara);
						break;	   
#endif            
		  default:
					  break;
		}

    return TRUE;
} /* GC0329YUVSensorSetting */
void GC0329GetExifInfo(UINT32 exifAddr)
{
	SENSORDB("[GC0329]enter GC0329GetExifInfo function\n");
	SENSOR_EXIF_INFO_STRUCT* pExifInfo = (SENSOR_EXIF_INFO_STRUCT*)exifAddr;
	pExifInfo->FNumber = 20;
	pExifInfo->AEISOSpeed = GC0329_SensorDriver.isoSpeed;
	pExifInfo->FlashLightTimeus = 0;
	pExifInfo->RealISOValue = GC0329_SensorDriver.isoSpeed;
	SENSORDB("[GC0329]exit GC0329GetExifInfo function\n");
}

static void GC0329SetDummy(kal_uint32 dummy_pixels, kal_uint32 dummy_lines)
{
/*	SENSORDB("[GC0329]enter GC0329SetDummy function:\n ");
	if (GC0329_SensorDriver.IsPVmode)	
	{
		dummy_pixels = dummy_pixels+GC0329_PV_PERIOD_PIXEL_NUMS; 
		GC0329_write_cmos_sensor(0x380D,( dummy_pixels&0xFF));		   
		GC0329_write_cmos_sensor(0x380C,(( dummy_pixels&0xFF00)>>8)); 
  
		dummy_lines= dummy_lines+GC0329_PV_PERIOD_LINE_NUMS; 
		GC0329_write_cmos_sensor(0x380F,(dummy_lines&0xFF));	   
		GC0329_write_cmos_sensor(0x380E,((dummy_lines&0xFF00)>>8));  
	} 
	else
	{
		dummy_pixels = dummy_pixels+GC0329_FULL_PERIOD_PIXEL_NUMS; 
		GC0329_write_cmos_sensor(0x380D,( dummy_pixels&0xFF));		   
		GC0329_write_cmos_sensor(0x380C,(( dummy_pixels&0xFF00)>>8)); 
  
		dummy_lines= dummy_lines+GC0329_FULL_PERIOD_LINE_NUMS; 
		GC0329_write_cmos_sensor(0x380F,(dummy_lines&0xFF));	   
		GC0329_write_cmos_sensor(0x380E,((dummy_lines&0xFF00)>>8));  
	} 
	SENSORDB("[GC0329]exit GC0329SetDummy function:\n ");*/
}	 /* GC0329_set_dummy */


/**************************/
static void GC0329GetEvAwbRef(UINT32 pSensorAEAWBRefStruct)
{
	PSENSOR_AE_AWB_REF_STRUCT Ref = (PSENSOR_AE_AWB_REF_STRUCT)pSensorAEAWBRefStruct;
	Ref->SensorAERef.AeRefLV05Shutter=0x170c;
	Ref->SensorAERef.AeRefLV05Gain=0x30;
	Ref->SensorAERef.AeRefLV13Shutter=0x24e;
	Ref->SensorAERef.AeRefLV13Gain=0x10;
	Ref->SensorAwbGainRef.AwbRefD65Rgain=0x610;
	Ref->SensorAwbGainRef.AwbRefD65Bgain=0x448;
	Ref->SensorAwbGainRef.AwbRefCWFRgain=0x4e0;
	Ref->SensorAwbGainRef.AwbRefCWFBgain=0x5a0;	
}

static void GC0329GetCurAeAwbInfo(UINT32 pSensorAEAWBCurStruct)
{

	PSENSOR_AE_AWB_CUR_STRUCT Info = (PSENSOR_AE_AWB_CUR_STRUCT)pSensorAEAWBCurStruct;
	Info->SensorAECur.AeCurShutter=100;//GC0329ReadShutter();
	Info->SensorAECur.AeCurGain=10;//GC0329ReadSensorGain() ;
	Info->SensorAwbGainCur.AwbCurRgain=10;//((GC0329YUV_read_cmos_sensor(0x3401)&&0xff)+((GC0329YUV_read_cmos_sensor(0x3400)&&0xff)*256));
	Info->SensorAwbGainCur.AwbCurBgain=10;//((GC0329YUV_read_cmos_sensor(0x3405)&&0xff)+((GC0329YUV_read_cmos_sensor(0x3404)&&0xff)*256));

}
UINT32 GC0329MaxFramerateByScenario(MSDK_SCENARIO_ID_ENUM scenarioId, MUINT32 frameRate) 
	{
		kal_uint32 pclk;
		kal_int16 dummyLine;
		kal_uint16 lineLength,frameHeight;
		SENSORDB("GC0329MaxFramerateByScenario: scenarioId = %d, frame rate = %d\n",scenarioId,frameRate);
		SENSORDB("[GC0329]enter GC0329MaxFramerateByScenario function:\n ");
		switch (scenarioId) {
			case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
				pclk = 26000000;
				lineLength = VGA_PERIOD_PIXEL_NUMS;
				frameHeight = ( pclk)/frameRate/lineLength;
				dummyLine = frameHeight - VGA_PERIOD_LINE_NUMS;
				if(dummyLine<0)
					dummyLine = 0;
				
				spin_lock(&GC0329_drv_lock);
				//GC0329_SensorDriver.SensorMode= SENSOR_MODE_PREVIEW;
				GC0329_dummy_lines = dummyLine;
				spin_unlock(&GC0329_drv_lock);
				GC0329SetDummy(GC0329_dummy_lines, GC0329_dummy_pixels);			
				break;			
			case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
				pclk = 26000000;
				lineLength = VGA_PERIOD_PIXEL_NUMS;
				frameHeight = ( pclk)/frameRate/lineLength;
				dummyLine = frameHeight - VGA_PERIOD_LINE_NUMS;
				if(dummyLine<0)
					dummyLine = 0;
				
				spin_lock(&GC0329_drv_lock);
				//GC0329_SensorDriver.SensorMode= SENSOR_MODE_PREVIEW;
				GC0329_dummy_lines = dummyLine;
				spin_unlock(&GC0329_drv_lock);
				GC0329SetDummy(GC0329_dummy_lines, GC0329_dummy_pixels);		
				break;			
			case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
			case MSDK_SCENARIO_ID_CAMERA_ZSD:	
				pclk = 26000000;
				lineLength = VGA_PERIOD_PIXEL_NUMS;
				frameHeight = ( pclk)/frameRate/lineLength;
				dummyLine = frameHeight - VGA_PERIOD_LINE_NUMS;
				if(dummyLine<0)
					dummyLine = 0;
				
				spin_lock(&GC0329_drv_lock);
				//GC0329_SensorDriver.SensorMode= SENSOR_MODE_PREVIEW;
				GC0329_dummy_lines = dummyLine;
				spin_unlock(&GC0329_drv_lock);
				GC0329SetDummy(GC0329_dummy_lines, GC0329_dummy_pixels);				
				break;		
			case MSDK_SCENARIO_ID_CAMERA_3D_PREVIEW: //added
				break;
			case MSDK_SCENARIO_ID_CAMERA_3D_VIDEO:
				break;
			case MSDK_SCENARIO_ID_CAMERA_3D_CAPTURE: //added   
				break;		
			default:
				break;
		}	
		SENSORDB("[GC0329]exit GC0329MaxFramerateByScenario function:\n ");
		return ERROR_NONE;
	}
UINT32 GC0329GetDefaultFramerateByScenario(MSDK_SCENARIO_ID_ENUM scenarioId, MUINT32 *pframeRate) 
{
	SENSORDB("[GC0329]enter GC0329GetDefaultFramerateByScenario function:\n ");
	switch (scenarioId) {
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			 *pframeRate = 300;
			 break;
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
		case MSDK_SCENARIO_ID_CAMERA_ZSD:
			 *pframeRate = 300;
			break;		
        case MSDK_SCENARIO_ID_CAMERA_3D_PREVIEW: //added
        case MSDK_SCENARIO_ID_CAMERA_3D_VIDEO:
        case MSDK_SCENARIO_ID_CAMERA_3D_CAPTURE: //added   
			 *pframeRate = 300;
			break;		
		default:
			break;
	}
	SENSORDB("[GC0329]exit GC0329GetDefaultFramerateByScenario function:\n ");
	return ERROR_NONE;
}
void GC0329_get_AEAWB_lock(UINT32 *pAElockRet32, UINT32 *pAWBlockRet32)
{
	SENSORDB("[GC0329]enter GC0329_get_AEAWB_lock function:\n ");
	*pAElockRet32 =1;
	*pAWBlockRet32=1;
	SENSORDB("[GC0329]GC0329_get_AEAWB_lock,AE=%d,AWB=%d\n",*pAElockRet32,*pAWBlockRet32);
	SENSORDB("[GC0329]exit GC0329_get_AEAWB_lock function:\n ");
}
void GC0329_GetDelayInfo(UINT32 delayAddr)
{
	SENSORDB("[GC0329]enter GC0329_GetDelayInfo function:\n ");
	SENSOR_DELAY_INFO_STRUCT *pDelayInfo=(SENSOR_DELAY_INFO_STRUCT*)delayAddr;
	pDelayInfo->InitDelay=0;
	pDelayInfo->EffectDelay=3;
	pDelayInfo->AwbDelay=0;
	
	//pDelayInfo->FlickerDelayFrame=3;
	pDelayInfo->AFSwitchDelayFrame=50;
	SENSORDB("[GC0329]exit GC0329_GetDelayInfo function:\n ");
}
void GC0329_AutoTestCmd(UINT32 *cmd,UINT32 *para)
{
	SENSORDB("[GC0329]enter GC0329_AutoTestCmd function:\n ");
	switch(*cmd)
	{
		case YUV_AUTOTEST_SET_SHADDING:
			SENSORDB("YUV_AUTOTEST_SET_SHADDING:para=%d\n",*para);
			break;
		case YUV_AUTOTEST_SET_GAMMA:
			SENSORDB("YUV_AUTOTEST_SET_GAMMA:para=%d\n",*para);
			break;
		case YUV_AUTOTEST_SET_AE:
			SENSORDB("YUV_AUTOTEST_SET_AE:para=%d\n",*para);
			break;
		case YUV_AUTOTEST_SET_SHUTTER:
			SENSORDB("YUV_AUTOTEST_SET_SHUTTER:para=%d\n",*para);
			break;
		case YUV_AUTOTEST_SET_GAIN:
			SENSORDB("YUV_AUTOTEST_SET_GAIN:para=%d\n",*para);
			break;
		case YUV_AUTOTEST_GET_SHUTTER_RANGE:
			//*para=8228;
			break;
		default:
			SENSORDB("YUV AUTOTEST NOT SUPPORT CMD:%d\n",*cmd);
			break;	
	}
	SENSORDB("[GC0329]exit GC0329_AutoTestCmd function:\n ");
}
UINT32 GC0329SetTestPatternMode(kal_bool bEnable)
{
	SENSORDB("[GC0329_GC0329SetTestPatternMode]test pattern bEnable:=%d\n",bEnable);

	/*GC0329_set_AE_mode( KAL_FALSE);
		
	GC0329_write_cmos_sensor(0xfe, 0x00);	
	GC0329_write_cmos_sensor(0x4c, 0x01);
	GC0329_write_cmos_sensor(0xfe, 0x00);*/
#if 1
		GC0329_write_cmos_sensor(0xfe, 0x00);////
		GC0329_write_cmos_sensor(0x4f, 0x00);////
		GC0329_write_cmos_sensor(0x03, 0x00);//  
		GC0329_write_cmos_sensor(0x04, 0x01);//  
		//GC0329_write_cmos_sensor(0x  , 0x  );//
		GC0329_write_cmos_sensor(0x40, 0x08);////
		GC0329_write_cmos_sensor(0x41, 0x00);////
		GC0329_write_cmos_sensor(0x42, 0x00);////
		GC0329_write_cmos_sensor(0x73, 0x80);//  
		GC0329_write_cmos_sensor(0x74, 0x80);//  
		GC0329_write_cmos_sensor(0x75, 0x80);//  
		GC0329_write_cmos_sensor(0x76, 0x80);//  
		GC0329_write_cmos_sensor(0x77, 0x40);////
		GC0329_write_cmos_sensor(0x78, 0x40);////
		GC0329_write_cmos_sensor(0x79, 0x40);////
		//GC0329_write_cmos_sensor(0x  , 0x  );//
		//GC0329_write_cmos_sensor(0x  , 0x  );//
		GC0329_write_cmos_sensor(0x71, 0x40);////
		GC0329_write_cmos_sensor(0x72, 0x40);////
		GC0329_write_cmos_sensor(0xfe, 0x01);////
		GC0329_write_cmos_sensor(0x9e, 0xc0);////
		GC0329_write_cmos_sensor(0x9f, 0x40);////
		GC0329_write_cmos_sensor(0xfe, 0x00);////
		GC0329_write_cmos_sensor(0xd0, 0x40);////
		GC0329_write_cmos_sensor(0xd1, 0x20);////
		GC0329_write_cmos_sensor(0xd2, 0x20);////
		GC0329_write_cmos_sensor(0xd3, 0x40);////
		GC0329_write_cmos_sensor(0xd5, 0x00);////
		GC0329_write_cmos_sensor(0xd8, 0x00);////
		GC0329_write_cmos_sensor(0xdd, 0x00);////
		GC0329_write_cmos_sensor(0xde, 0x00);////
		GC0329_write_cmos_sensor(0xed, 0x00);////
		GC0329_write_cmos_sensor(0xfe, 0x00);////
		GC0329_write_cmos_sensor(0x4c, 0x01);////
#else
GC0329_write_cmos_sensor(0xfe, 0x00);
GC0329_write_cmos_sensor(0x40, 0x08);
GC0329_write_cmos_sensor(0x41, 0x00);
GC0329_write_cmos_sensor(0x42, 0x00);
GC0329_write_cmos_sensor(0x4f, 0x00);
GC0329_write_cmos_sensor(0x03, 0x00);
GC0329_write_cmos_sensor(0x04, 0x00);
GC0329_write_cmos_sensor(0x26, 0x80);
GC0329_write_cmos_sensor(0x18, 0x01);
GC0329_write_cmos_sensor(0xfe, 0x01);
GC0329_write_cmos_sensor(0x12, 0x04);
GC0329_write_cmos_sensor(0x9c, 0x00);
GC0329_write_cmos_sensor(0x9e, 0xc0);
GC0329_write_cmos_sensor(0x9f, 0x40);
GC0329_write_cmos_sensor(0xfe, 0x00);
GC0329_write_cmos_sensor(0x70, 0x40);
GC0329_write_cmos_sensor(0x71, 0x40);
GC0329_write_cmos_sensor(0x72, 0x40);
GC0329_write_cmos_sensor(0x73, 0x80);
GC0329_write_cmos_sensor(0x74, 0x80);
GC0329_write_cmos_sensor(0x75, 0x80);
GC0329_write_cmos_sensor(0x76, 0x80);
GC0329_write_cmos_sensor(0x7a, 0x80);
GC0329_write_cmos_sensor(0x7b, 0x80);
GC0329_write_cmos_sensor(0x7c, 0x80);
GC0329_write_cmos_sensor(0x29, 0x00);
GC0329_write_cmos_sensor(0x2a, 0x00);
GC0329_write_cmos_sensor(0x2b, 0x00);
GC0329_write_cmos_sensor(0x2c, 0x00);
GC0329_write_cmos_sensor(0x2d, 0x00);
GC0329_write_cmos_sensor(0x2e, 0x00);
GC0329_write_cmos_sensor(0x2f, 0x00);
GC0329_write_cmos_sensor(0x30, 0x00);
GC0329_write_cmos_sensor(0x31, 0x00);
GC0329_write_cmos_sensor(0x3b, 0x00);
GC0329_write_cmos_sensor(0x3c, 0x00);
GC0329_write_cmos_sensor(0x3d, 0x00);
GC0329_write_cmos_sensor(0x3e, 0x00);
GC0329_write_cmos_sensor(0x77, 0x40);
GC0329_write_cmos_sensor(0x78, 0x40);
GC0329_write_cmos_sensor(0x79, 0x40);
GC0329_write_cmos_sensor(0xd0, 0x40);
GC0329_write_cmos_sensor(0xd1, 0x30);
GC0329_write_cmos_sensor(0xd2, 0x20);
GC0329_write_cmos_sensor(0xd3, 0x22);
GC0329_write_cmos_sensor(0xd4, 0x80);
GC0329_write_cmos_sensor(0xd5, 0x00);
GC0329_write_cmos_sensor(0xfe, 0x00);
GC0329_write_cmos_sensor(0x4c, 0x01);
GC0329_write_cmos_sensor(0x4d, 0x88);

#endif
	if(bEnable)
	{
		//GC0329_write_cmos_sensor(0x503d,0x80);
	}
	else
	{
		//GC0329_write_cmos_sensor(0x503d,0x00);
	}
	
	return ERROR_NONE;
}

void GC0329_3ACtrl(ACDK_SENSOR_3A_LOCK_ENUM action)
{
	SENSORDB("[GC0329]enter ACDK_SENSOR_3A_LOCK_ENUM function:action=%d\n",action);
   switch (action)
   {
      case SENSOR_3A_AE_LOCK:
          spin_lock(&GC0329_drv_lock);
          GC0329_SensorDriver.userAskAeLock = TRUE;
          spin_unlock(&GC0329_drv_lock);
          GC0329_set_AE_mode(KAL_FALSE);
      break;
      case SENSOR_3A_AE_UNLOCK:
          spin_lock(&GC0329_drv_lock);
          GC0329_SensorDriver.userAskAeLock = FALSE;
          spin_unlock(&GC0329_drv_lock);
          GC0329_set_AE_mode(KAL_TRUE);
      break;

      case SENSOR_3A_AWB_LOCK:
          spin_lock(&GC0329_drv_lock);
          GC0329_SensorDriver.userAskAwbLock = TRUE;
          spin_unlock(&GC0329_drv_lock);
      //    GC0329_awb_enable(KAL_FALSE);
      break;

      case SENSOR_3A_AWB_UNLOCK:
          spin_lock(&GC0329_drv_lock);
          GC0329_SensorDriver.userAskAwbLock = FALSE;
          spin_unlock(&GC0329_drv_lock);
      //    GC0329_awb_enable(KAL_TRUE);
      break;
      default:
      	break;
   }
   SENSORDB("[GC0329]exit ACDK_SENSOR_3A_LOCK_ENUM function:action=%d\n",action);
   return;
}

UINT32 GC0329FeatureControl(MSDK_SENSOR_FEATURE_ENUM FeatureId,UINT8 *pFeaturePara,UINT32 *pFeatureParaLen)
{
	UINT16 *pFeatureReturnPara16=(UINT16 *) pFeaturePara;
	UINT16 *pFeatureData16=(UINT16 *) pFeaturePara;
	UINT32 *pFeatureReturnPara32=(UINT32 *) pFeaturePara;
	UINT32 *pFeatureData32=(UINT32 *) pFeaturePara;
	MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData=(MSDK_SENSOR_CONFIG_STRUCT *) pFeaturePara;
	MSDK_SENSOR_REG_INFO_STRUCT *pSensorRegData=(MSDK_SENSOR_REG_INFO_STRUCT *) pFeaturePara;
	UINT32 Tony_Temp1 = 0;
	UINT32 Tony_Temp2 = 0;
	Tony_Temp1 = pFeaturePara[0];
	Tony_Temp2 = pFeaturePara[1];
	//SENSORDB("[GC0329]CONTROLFLOW [GC0329FeatureControl]feature id=%d \n",FeatureId);
	SENSORDB("[GC0329] [GC0329FeatureControl]feature id=%d \n",FeatureId);
	switch (FeatureId)
	{
		case SENSOR_FEATURE_GET_RESOLUTION:
			*pFeatureReturnPara16++=VGA_PERIOD_PIXEL_NUMS;
			*pFeatureReturnPara16=VGA_PERIOD_LINE_NUMS;
			*pFeatureParaLen=4;
			break;
		case SENSOR_FEATURE_GET_PERIOD:
			switch(GC0329_CurrentScenarioId)
			{
				case MSDK_SCENARIO_ID_CAMERA_ZSD:
					*pFeatureReturnPara16++=VGA_PERIOD_PIXEL_NUMS + GC0329_dummy_pixels;
					*pFeatureReturnPara16=VGA_PERIOD_LINE_NUMS + GC0329_dummy_lines;
					*pFeatureParaLen=4;
					break;
				default:
					*pFeatureReturnPara16++=VGA_PERIOD_PIXEL_NUMS + GC0329_dummy_pixels;
					*pFeatureReturnPara16=VGA_PERIOD_LINE_NUMS + GC0329_dummy_lines;
					*pFeatureParaLen=4;
					break;
			}
			break;
		case SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ:
			switch(GC0329_CurrentScenarioId)
			{
				case MSDK_SCENARIO_ID_CAMERA_ZSD:
					*pFeatureReturnPara32 = GC0329_g_fPV_PCLK * 1000 *1000;  //unit: Hz 			
					*pFeatureParaLen=4;
					break;
				default:
					*pFeatureReturnPara32 = GC0329_g_fPV_PCLK * 1000 *1000;  //unit: Hz
					*pFeatureParaLen=4;
					break;
			}
			break;
		case SENSOR_FEATURE_SET_ESHUTTER:
			break;
		case SENSOR_FEATURE_GET_EXIF_INFO:
			GC0329GetExifInfo(*pFeatureData32);
			break;
		case SENSOR_FEATURE_SET_NIGHTMODE:
			GC0329_night_mode((BOOL) *pFeatureData16);
			break;
		case SENSOR_FEATURE_SET_GAIN:
			break;
		case SENSOR_FEATURE_SET_FLASHLIGHT:
			break;
		case SENSOR_FEATURE_SET_ISP_MASTER_CLOCK_FREQ:
			break;
		case SENSOR_FEATURE_SET_REGISTER:
			GC0329_write_cmos_sensor(pSensorRegData->RegAddr, pSensorRegData->RegData);
			break;
		case SENSOR_FEATURE_GET_REGISTER:
			pSensorRegData->RegData = GC0329_read_cmos_sensor(pSensorRegData->RegAddr);
			break;
		case SENSOR_FEATURE_GET_CONFIG_PARA:
			memcpy(pSensorConfigData, &GC0329SensorConfigData, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
			*pFeatureParaLen=sizeof(MSDK_SENSOR_CONFIG_STRUCT);
			break;
		case SENSOR_FEATURE_SET_CCT_REGISTER:
		case SENSOR_FEATURE_GET_CCT_REGISTER:
		case SENSOR_FEATURE_SET_ENG_REGISTER:
		case SENSOR_FEATURE_GET_ENG_REGISTER:
		case SENSOR_FEATURE_GET_REGISTER_DEFAULT:
		case SENSOR_FEATURE_CAMERA_PARA_TO_SENSOR:
		case SENSOR_FEATURE_SENSOR_TO_CAMERA_PARA:
		case SENSOR_FEATURE_GET_GROUP_INFO:
		case SENSOR_FEATURE_GET_ITEM_INFO:
		case SENSOR_FEATURE_SET_ITEM_INFO:
		case SENSOR_FEATURE_GET_ENG_INFO:
			break;
		case SENSOR_FEATURE_GET_GROUP_COUNT:
			*pFeatureReturnPara32++=0;
			*pFeatureParaLen=4;    
			break; 
		case SENSOR_FEATURE_GET_LENS_DRIVER_ID:
			*pFeatureReturnPara32=LENS_DRIVER_ID_DO_NOT_CARE;
			*pFeatureParaLen=4;
			break;
		case SENSOR_FEATURE_SET_TEST_PATTERN:			 
			GC0329SetTestPatternMode((BOOL)*pFeatureData16);			
			break;
		case SENSOR_FEATURE_CHECK_SENSOR_ID:
			GC0329GetSensorID(pFeatureData32);
			break;
		case SENSOR_FEATURE_GET_TEST_PATTERN_CHECKSUM_VALUE:
			*pFeatureReturnPara32=GC0329_TEST_PATTERN_CHECKSUM;
			*pFeatureParaLen=4;
			break;
		case SENSOR_FEATURE_SET_YUV_CMD:
			GC0329YUVSensorSetting((FEATURE_ID)*pFeatureData32, *(pFeatureData32+1));
			break;
		case SENSOR_FEATURE_SET_YUV_3A_CMD:
			GC0329_3ACtrl((ACDK_SENSOR_3A_LOCK_ENUM)*pFeatureData32);
			break;
		case SENSOR_FEATURE_SET_VIDEO_MODE:
			GC0329YUVSetVideoMode(*pFeatureData16);
			break; 
		case SENSOR_FEATURE_GET_EV_AWB_REF:
			GC0329GetEvAwbRef(*pFeatureData32);
			break;
		case SENSOR_FEATURE_GET_SHUTTER_GAIN_AWB_GAIN:
			GC0329GetCurAeAwbInfo(*pFeatureData32); 		
			break;
		case SENSOR_FEATURE_SET_MAX_FRAME_RATE_BY_SCENARIO:
			GC0329MaxFramerateByScenario((MSDK_SCENARIO_ID_ENUM)*pFeatureData32,*(pFeatureData32+1));
			break;
		case SENSOR_FEATURE_GET_DEFAULT_FRAME_RATE_BY_SCENARIO:
			GC0329GetDefaultFramerateByScenario((MSDK_SCENARIO_ID_ENUM)*pFeatureData32,(MUINT32 *)*(pFeatureData32+1));
			break;
		case SENSOR_FEATURE_GET_AE_AWB_LOCK_INFO:
			GC0329_get_AEAWB_lock(*pFeatureData32, *(pFeatureData32+1));
			break;
		case SENSOR_FEATURE_GET_DELAY_INFO:
			SENSORDB("SENSOR_FEATURE_GET_DELAY_INFO\n");
			GC0329_GetDelayInfo(*pFeatureData32);
			break;
		case SENSOR_FEATURE_AUTOTEST_CMD:
			SENSORDB("SENSOR_FEATURE_AUTOTEST_CMD\n");
			GC0329_AutoTestCmd(*pFeatureData32,*(pFeatureData32+1));
			break;

		//AF
		case SENSOR_FEATURE_INITIALIZE_AF:			 
			 break;
		case SENSOR_FEATURE_MOVE_FOCUS_LENS:
		//	  GC0329_FOCUS_Move_to(*pFeatureData16);
			break;
		case SENSOR_FEATURE_GET_AF_STATUS:
		//	  GC0329_FOCUS_OVT_AFC_Get_AF_Status(pFeatureReturnPara32); 		   
			*pFeatureParaLen=4;
			break;
		case SENSOR_FEATURE_GET_AF_INF:
		//	  GC0329_FOCUS_Get_AF_Inf(pFeatureReturnPara32);
			*pFeatureParaLen=4; 		   
			break;
		case SENSOR_FEATURE_GET_AF_MACRO:
		//	  GC0329_FOCUS_Get_AF_Macro(pFeatureReturnPara32);
			*pFeatureParaLen=4; 		   
			break;
		case SENSOR_FEATURE_CONSTANT_AF:
		//	GC0329_FOCUS_OVT_AFC_Constant_Focus();
			 break;
		case SENSOR_FEATURE_SET_AF_WINDOW:		 
			//GC0329_FOCUS_Set_AF_Window(*pFeatureData32);
			break;
		case SENSOR_FEATURE_SINGLE_FOCUS_MODE:
			//GC0329_FOCUS_OVT_AFC_Single_Focus();
			break;	
		case SENSOR_FEATURE_CANCEL_AF:
			//GC0329_FOCUS_OVT_AFC_Cancel_Focus();
			break;					
		case SENSOR_FEATURE_GET_AF_MAX_NUM_FOCUS_AREAS:
			//GC0329_FOCUS_Get_AF_Max_Num_Focus_Areas(pFeatureReturnPara32);			
			*pFeatureParaLen=4;
			break;		  
		case SENSOR_FEATURE_GET_AE_MAX_NUM_METERING_AREAS:
			//GC0329_FOCUS_Get_AE_Max_Num_Metering_Areas(pFeatureReturnPara32); 		   
			*pFeatureParaLen=4;
			break;		  
		case SENSOR_FEATURE_SET_AE_WINDOW:
			SENSORDB("AE zone addr = 0x%x\n",*pFeatureData32);			
			//GC0329_FOCUS_Set_AE_Window(*pFeatureData32);
			break; 
		default:
			SENSORDB("GC0329FeatureControl:default \n");
			break;			
	}
	SENSORDB("[GC0329]exit GC0329FeatureControl function:\n ");
	return ERROR_NONE;
}	/* GC0329MIPIFeatureControl() */

SENSOR_FUNCTION_STRUCT	SensorFuncGC0329YUV=
{
	GC0329Open,
	GC0329GetInfo,
	GC0329GetResolution,
	GC0329FeatureControl,
	GC0329Control,
	GC0329Close
};


UINT32 GC0329_YUV_SensorInit(PSENSOR_FUNCTION_STRUCT *pfFunc)
{
	/* To Do : Check Sensor status here */
	if (pfFunc!=NULL)
		*pfFunc=&SensorFuncGC0329YUV;
	return ERROR_NONE;
} /* SensorInit() */



