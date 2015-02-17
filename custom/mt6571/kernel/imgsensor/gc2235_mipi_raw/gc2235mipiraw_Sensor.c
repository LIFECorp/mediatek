/*******************************************************************************************/


/*******************************************************************************************/

#include <linux/videodev2.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <asm/atomic.h>
#include <linux/xlog.h>
#include <asm/system.h>

#include <linux/proc_fs.h> 


#include <linux/dma-mapping.h>

#include "kd_camera_hw.h"
#include "kd_imgsensor.h"
#include "kd_imgsensor_define.h"
#include "kd_imgsensor_errcode.h"

#include "gc2235mipiraw_Sensor.h"
#include "gc2235mipiraw_Camera_Sensor_para.h"
#include "gc2235mipiraw_CameraCustomized.h"
static DEFINE_SPINLOCK(gc2235mipiraw_drv_lock);

#define GC2235_DEBUG
//#define GC2235_DEBUG_SOFIA
#define GC2235_TEST_PATTERN_CHECKSUM (0x3f8c7160)//(0x5593c632)

#ifdef GC2235_DEBUG
	#define GC2235DB(fmt, arg...) xlog_printk(ANDROID_LOG_DEBUG, "[GC2235Raw] ",  fmt, ##arg)
#else
	#define GC2235DB(fmt, arg...)
#endif

#ifdef GC2235_DEBUG_SOFIA
	#define GC2235DBSOFIA(fmt, arg...) xlog_printk(ANDROID_LOG_DEBUG, "[GC2235Raw] ",  fmt, ##arg)
#else
	#define GC2235DBSOFIA(fmt, arg...)
#endif

#define mDELAY(ms)  mdelay(ms)

kal_uint32 GC2235_FeatureControl_PERIOD_PixelNum=GC2235_PV_PERIOD_PIXEL_NUMS;
kal_uint32 GC2235_FeatureControl_PERIOD_LineNum=GC2235_PV_PERIOD_LINE_NUMS;
UINT16  gc2235VIDEO_MODE_TARGET_FPS = 30;
MSDK_SENSOR_CONFIG_STRUCT GC2235SensorConfigData;
MSDK_SCENARIO_ID_ENUM GC2235CurrentScenarioId = MSDK_SCENARIO_ID_CAMERA_PREVIEW;

/* FIXME: old factors and DIDNOT use now. s*/
SENSOR_REG_STRUCT GC2235SensorCCT[]=CAMERA_SENSOR_CCT_DEFAULT_VALUE;
SENSOR_REG_STRUCT GC2235SensorReg[ENGINEER_END]=CAMERA_SENSOR_REG_DEFAULT_VALUE;
/* FIXME: old factors and DIDNOT use now. e*/

static GC2235_PARA_STRUCT gc2235;

#define Sleep(ms) mdelay(ms)
#define GC2235_ORIENTATION IMAGE_V_MIRROR //IMAGE_NORMAL//IMAGE_NORMAL

extern int iReadRegI2C(u8 *a_pSendData , u16 a_sizeSendData, u8 * a_pRecvData, u16 a_sizeRecvData, u16 i2cId);
extern int iWriteRegI2C(u8 *a_pSendData , u16 a_sizeSendData, u16 i2cId);

/*************************************************************************
* FUNCTION
*    GC2235MIPI_write_cmos_sensor
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
static void GC2235_write_cmos_sensor(kal_uint8 addr, kal_uint8 para)
{
kal_uint8 out_buff[2];

    out_buff[0] = addr;
    out_buff[1] = para;

    iWriteRegI2C((u8*)out_buff , 2, GC2235MIPI_WRITE_ID); 
}

/*************************************************************************
* FUNCTION
*    GC2035_read_cmos_sensor
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
static kal_uint8 GC2235_read_cmos_sensor(kal_uint8 addr)
{
  kal_uint8 in_buff[1] = {0xFF};
  kal_uint8 out_buff[1];
  
  out_buff[0] = addr;

    if (0 != iReadRegI2C((u8*)out_buff , 1, (u8*)in_buff, 1, GC2235MIPI_WRITE_ID)) {
        GC2235DB("ERROR: GC2235MIPI_read_cmos_sensor \n");
    }
  return in_buff[0];
}


void GC2235_SetGain(UINT16 iGain)
{
	    kal_uint16 iReg,temp;
		
#ifdef GC2235_DEBUG
		GC2235DB("GC2235_SetGain iGain = %d \n",iGain);
#endif
		iReg = iGain;
        //if(iGain==gc2235.Gain) return;
		gc2235.Gain = iGain;
		if(256> iReg)
		{
		//analogic gain
		GC2235_write_cmos_sensor(0xb0, 0x40); // global gain
		GC2235_write_cmos_sensor(0xb1, iReg);//only digital gain 12.13
		}
		else
		{
		//analogic gain
		temp = 64*iReg/256; 
		//temp = iReg - 256 + 64;
		if(temp > 0xc0) temp = 0xc0;
		GC2235_write_cmos_sensor(0xb0, temp); // global gain
		GC2235_write_cmos_sensor(0xb1, 0xff);//only digital gain 12.13
		}	
		//GC2235DB("GC2235_SetGain reg 0xb0=%x,0xb1=%x\n",GC2235_read_cmos_sensor(0xb0),GC2235_read_cmos_sensor(0xb1));		
		return;
}   /*  GC2235_SetGain_SetGain  */


void GC2235_camera_para_to_sensor(void)
{}

void GC2235_sensor_to_camera_para(void)
{}

kal_int32  GC2235_get_sensor_group_count(void)
{
    return GROUP_TOTAL_NUMS;
}

void GC2235_get_sensor_group_info(kal_uint16 group_idx, kal_int8* group_name_ptr, kal_int32* item_count_ptr)
{}
void GC2235_get_sensor_item_info(kal_uint16 group_idx,kal_uint16 item_idx, MSDK_SENSOR_ITEM_INFO_STRUCT* info_ptr)
{}
kal_bool GC2235_set_sensor_item_info(kal_uint16 group_idx, kal_uint16 item_idx, kal_int32 ItemValue)
{    return KAL_TRUE;}

static void GC2235_SetDummy( const kal_uint32 iPixels, const kal_uint32 iLines )
{
 	kal_uint32 hb = 0;
	kal_uint32 vb = 0;

	if ( SENSOR_MODE_PREVIEW == gc2235.sensorMode )	//SXGA size output
	{
		hb = GC2235_DEFAULT_DUMMY_PIXEL_NUMS + iPixels;
		vb = GC2235_DEFAULT_DUMMY_LINE_NUMS + iLines;
	}
	else if( SENSOR_MODE_VIDEO== gc2235.sensorMode )
	{
		hb = GC2235_DEFAULT_DUMMY_PIXEL_NUMS + iPixels;
		vb = GC2235_DEFAULT_DUMMY_LINE_NUMS + iLines;
	}
	else//QSXGA size output
	{
		hb = GC2235_DEFAULT_DUMMY_PIXEL_NUMS + iPixels;
		vb = GC2235_DEFAULT_DUMMY_LINE_NUMS + iLines;
	}

	//if(gc2235.maxExposureLines > frame_length -4 )
	//	return;

	//ASSERT(line_length < GC2235_MAX_LINE_LENGTH);		//0xCCCC
	//ASSERT(frame_length < GC2235_MAX_FRAME_LENGTH);	//0xFFFF

	//Set HB
	GC2235_write_cmos_sensor(0x05, (hb >> 8)& 0xFF);
	GC2235_write_cmos_sensor(0x06, hb & 0xFF);

	//Set VB
	GC2235_write_cmos_sensor(0x07, (vb >> 8) & 0xFF);
	GC2235_write_cmos_sensor(0x08, vb & 0xFF);
	GC2235DB("GC2235_SetDummy framelength=%d\n",vb+GC2235_VALID_LINE_NUMS);

}   /*  GC2235_SetDummy */

static void GC2235_Sensor_Init(void)
{
	/////////////////////////////////////////////////////
	//////////////////////	 SYS   //////////////////////
	/////////////////////////////////////////////////////
	GC2235_write_cmos_sensor(0xfe, 0x80);
	GC2235_write_cmos_sensor(0xfe, 0x80);
	GC2235_write_cmos_sensor(0xfe, 0x80);
	GC2235_write_cmos_sensor(0xf2, 0x00);
	GC2235_write_cmos_sensor(0xf6, 0x00);
	GC2235_write_cmos_sensor(0xfc, 0x06);


	GC2235_write_cmos_sensor(0xf7, 0x15); //pll enable //0x15->0x17 30fps setting
	#ifdef GC2235_MIPI_30FPS
	GC2235_write_cmos_sensor(0xf8, 0x86); //Pll mode 2 //0x84->0x8c
	#else //20fps
	GC2235_write_cmos_sensor(0xf8, 0x84); //Pll mode 2 //0x84
	#endif
	GC2235_write_cmos_sensor(0xfa, 0x00); //div

	GC2235_write_cmos_sensor(0xf9, 0xfe); //[0] pll enable
	GC2235_write_cmos_sensor(0xfe, 0x00);
	
	/////////////////////////////////////////////////////
	////////////////   ANALOG & CISCTL	 ////////////////
	/////////////////////////////////////////////////////

    
	#ifdef GC2235_MIPI_30FPS
	GC2235_write_cmos_sensor(0x03, 0x04);  //30fps setting
	GC2235_write_cmos_sensor(0x04, 0x68);

	#else //20fps
    GC2235_write_cmos_sensor(0x03, 0x04);  //30fps setting
	GC2235_write_cmos_sensor(0x04, 0x4b);
	#endif
	GC2235_write_cmos_sensor(0x05, (GC2235_DEFAULT_DUMMY_PIXEL_NUMS>>8) & 0xFF);
	GC2235_write_cmos_sensor(0x06, GC2235_DEFAULT_DUMMY_PIXEL_NUMS & 0xFF);//hb
	GC2235_write_cmos_sensor(0x07, (GC2235_DEFAULT_DUMMY_LINE_NUMS>>8) & 0xFF);
	GC2235_write_cmos_sensor(0x08, GC2235_DEFAULT_DUMMY_LINE_NUMS & 0xFF);//vb

	//GC2235_write_cmos_sensor(0x03, 0x05);
	//GC2235_write_cmos_sensor(0x04, 0x4b);
	//GC2235_write_cmos_sensor(0x05, 0x01);
	//GC2235_write_cmos_sensor(0x06, 0x1d);
	//GC2235_write_cmos_sensor(0x07, 0x00);
	//GC2235_write_cmos_sensor(0x08, 0x9b);

	GC2235_write_cmos_sensor(0x0a, 0x02);
	GC2235_write_cmos_sensor(0x0c, 0x04);//00->04
	GC2235_write_cmos_sensor(0x0d, 0x04);
	GC2235_write_cmos_sensor(0x0e, 0xd0);
	GC2235_write_cmos_sensor(0x0f, 0x06); 
	
    if(GC2235_ORIENTATION == IMAGE_V_MIRROR)
		GC2235_write_cmos_sensor(0x10, 0x60);//50->60
	else
		GC2235_write_cmos_sensor(0x10, 0x50);//50->60->50 avoid black line issue
	
	GC2235_write_cmos_sensor(0x17, 0x14);//14 //[0]mirror [1]flip->15->14
	GC2235_write_cmos_sensor(0x18, 0x12); //  0x1e->0x12 
	GC2235_write_cmos_sensor(0x19, 0x06);
	GC2235_write_cmos_sensor(0x1a, 0x01);
	GC2235_write_cmos_sensor(0x1b, 0x4d);//0x48->0x4d->0x8d->0x4d video
	GC2235_write_cmos_sensor(0x1e, 0x88); //88->a8 noise
	GC2235_write_cmos_sensor(0x1f, 0x48); 
	GC2235_write_cmos_sensor(0x20, 0x83);//03->0x83 08/14
	GC2235_write_cmos_sensor(0x21, 0x7f); //0x6f->0x7f
	GC2235_write_cmos_sensor(0x22, 0x80); 
	GC2235_write_cmos_sensor(0x23, 0x42);//c1->42 08/14
	GC2235_write_cmos_sensor(0x24, 0x2f);//PAD_drv
	GC2235_write_cmos_sensor(0x26, 0x01);
	GC2235_write_cmos_sensor(0x27, 0x30);
	GC2235_write_cmos_sensor(0x3f, 0x00);
	
	/////////////////////////////////////////////////////
	//////////////////////	 ISP   //////////////////////
	/////////////////////////////////////////////////////
	GC2235_write_cmos_sensor(0x8b, 0xa0);
	GC2235_write_cmos_sensor(0x8c, 0x02);
	GC2235_write_cmos_sensor(0x90, 0x01);
	GC2235_write_cmos_sensor(0x92, 0x03);
	GC2235_write_cmos_sensor(0x94, 0x0b);//06->0b
	GC2235_write_cmos_sensor(0x95, 0x04);
	GC2235_write_cmos_sensor(0x96, 0xb0);
	GC2235_write_cmos_sensor(0x97, 0x06);
	GC2235_write_cmos_sensor(0x98, 0x40);
	
	/////////////////////////////////////////////////////
	//////////////////////	 BLK   //////////////////////
	/////////////////////////////////////////////////////

	GC2235_write_cmos_sensor(0x40, 0x42); //smooth speed  //0x72->0x7a->0x72->>0x42
	GC2235_write_cmos_sensor(0x41, 0x04);
	GC2235_write_cmos_sensor(0x5e, 0x00);
	GC2235_write_cmos_sensor(0x5f, 0x00);
	GC2235_write_cmos_sensor(0x60, 0x00);
	GC2235_write_cmos_sensor(0x61, 0x00); 
	GC2235_write_cmos_sensor(0x62, 0x00);
	GC2235_write_cmos_sensor(0x63, 0x00); 
	GC2235_write_cmos_sensor(0x64, 0x00);
	GC2235_write_cmos_sensor(0x65, 0x00);
	GC2235_write_cmos_sensor(0x66, 0x20);
	GC2235_write_cmos_sensor(0x67, 0x20); 
	GC2235_write_cmos_sensor(0x68, 0x20);
	GC2235_write_cmos_sensor(0x69, 0x20);
	mdelay(100);

	
	/////////////////////////////////////////////////////
	//////////////////////	 GAIN	/////////////////////
	/////////////////////////////////////////////////////
	GC2235_write_cmos_sensor(0xb2, 0x00);
	GC2235_write_cmos_sensor(0xb3, 0x40);
	GC2235_write_cmos_sensor(0xb4, 0x40);
	GC2235_write_cmos_sensor(0xb5, 0x40);
	
	/////////////////////////////////////////////////////
	////////////////////   DARK SUN   ///////////////////
	/////////////////////////////////////////////////////
	GC2235_write_cmos_sensor(0xb8, 0x0f);
	GC2235_write_cmos_sensor(0xb9, 0x23);
	GC2235_write_cmos_sensor(0xba, 0xff);
	GC2235_write_cmos_sensor(0xbc, 0x00);
	GC2235_write_cmos_sensor(0xbd, 0x00);
	GC2235_write_cmos_sensor(0xbe, 0xff);
	GC2235_write_cmos_sensor(0xbf, 0x09);

	/////////////////////////////////////////////////////
	//////////////////////	 OUTPUT	/////////////////////
	/////////////////////////////////////////////////////
	GC2235_write_cmos_sensor(0xfe, 0x03);
	GC2235_write_cmos_sensor(0x01, 0x07);
	GC2235_write_cmos_sensor(0x02, 0x33);//mipi drv 0x11->0x33
	GC2235_write_cmos_sensor(0x03, 0x13);//mipi drv 0x11->0x13
	GC2235_write_cmos_sensor(0x06, 0x80);
	GC2235_write_cmos_sensor(0x11, 0x2b);
	GC2235_write_cmos_sensor(0x12, 0xd0);
	GC2235_write_cmos_sensor(0x13, 0x07);
	GC2235_write_cmos_sensor(0x15, 0x12);
	GC2235_write_cmos_sensor(0x04, 0x20);
	GC2235_write_cmos_sensor(0x05, 0x00);
	GC2235_write_cmos_sensor(0x17, 0x01);

	GC2235_write_cmos_sensor(0x21, 0x01);
	GC2235_write_cmos_sensor(0x22, 0x02);
	GC2235_write_cmos_sensor(0x23, 0x01);
	GC2235_write_cmos_sensor(0x29, 0x02);
	GC2235_write_cmos_sensor(0x2a, 0x01);

	GC2235_write_cmos_sensor(0x10, 0x91);  // 93 line_sync_mode 
	GC2235_write_cmos_sensor(0xfe, 0x00);
	GC2235_write_cmos_sensor(0xf2, 0x00);
}   /*  GC2235_Sensor_Init  */

UINT32 GC2235Open(void)
{
	volatile signed int i;
	kal_uint16 sensor_id = 0;
	GC2235DB("GC2235Open enter :\n ");
	sensor_id=((GC2235_read_cmos_sensor(0xf0) << 8) | GC2235_read_cmos_sensor(0xf1)); 
	GC2235DB("GC2235MIPIGetSensorID:%x \n",sensor_id);
	spin_lock(&gc2235mipiraw_drv_lock);
	gc2235.sensorMode = SENSOR_MODE_INIT;
	gc2235.GC2235AutoFlickerMode = KAL_FALSE;
	gc2235.GC2235VideoMode = KAL_FALSE;
	gc2235.shutter = 0xff;
	gc2235.Gain = 64;
	spin_unlock(&gc2235mipiraw_drv_lock);
	GC2235_Sensor_Init();

	GC2235DB("GC2235Open exit :\n ");

    return ERROR_NONE;
}

UINT32 GC2235GetSensorID(UINT32 *sensorID)
{
    int  retry = 1;

	GC2235DB("GC2235GetSensorID enter :\n ");

    // check if sensor ID correct
    *sensorID =((GC2235_read_cmos_sensor(0xf0) << 8) | GC2235_read_cmos_sensor(0xf1)); 
	GC2235DB("GC2235MIPIGetSensorID:%x \n",*sensorID);

    if (*sensorID != GC2235_SENSOR_ID) {
        *sensorID = 0xFFFFFFFF;
        return ERROR_SENSOR_CONNECT_FAIL;
    }
    return ERROR_NONE;
}


void GC2235_SetShutter(kal_uint32 iShutter)
{
    //kal_uint8 i=0;
	if(MSDK_SCENARIO_ID_CAMERA_ZSD == GC2235CurrentScenarioId )
	{
		//GC2235DB("always UPDATE SHUTTER when gc2235.sensorMode == SENSOR_MODE_CAPTURE\n");
	}
	else{
		if(gc2235.sensorMode == SENSOR_MODE_CAPTURE)
		{
			//GC2235DB("capture!!DONT UPDATE SHUTTER!!\n");
			//return;
		}
	}
	if(gc2235.shutter == iShutter);
		//return;
	
   spin_lock(&gc2235mipiraw_drv_lock);
   gc2235.shutter= iShutter;
  
   spin_unlock(&gc2235mipiraw_drv_lock);
   if(gc2235.shutter > 8191) gc2235.shutter = 8191;
   if(gc2235.shutter < 1) gc2235.shutter = 1;
   GC2235_write_cmos_sensor(0x03, (gc2235.shutter>>8) & 0x1F);
   GC2235_write_cmos_sensor(0x04, gc2235.shutter & 0xFF);
   	
   
   GC2235DB("GC2235_SetShutter:%x \n",iShutter); 
   	
 //  GC2235DB("GC2235_SetShutter 0x03=%x, 0x04=%x \n",GC2235_read_cmos_sensor(0x03),GC2235_read_cmos_sensor(0x04));
   return;
}   /*  GC2235_SetShutter   */

UINT32 GC2235_read_shutter(void)
{

	kal_uint16 temp_reg1, temp_reg2;
	UINT32 shutter =0;
	temp_reg1 = GC2235_read_cmos_sensor(0x03);    // AEC[b19~b16]
	temp_reg2 = GC2235_read_cmos_sensor(0x04);    // AEC[b15~b8]
	shutter  = ((temp_reg1 << 8)| (temp_reg2));

	return shutter;
}

void GC2235_NightMode(kal_bool bEnable)
{}


UINT32 GC2235Close(void)
{    return ERROR_NONE;}

void GC2235SetFlipMirror(kal_int32 imgMirror)
{
	kal_int16 mirror=0,flip=0;

    switch (imgMirror)
    {
        case IMAGE_NORMAL://IMAGE_NORMAL:
			GC2235_write_cmos_sensor(0x17,0x14);//bit[1][0]
			GC2235_write_cmos_sensor(0x92,0x03);
			GC2235_write_cmos_sensor(0x94,0x0b);
            break;
        case IMAGE_H_MIRROR://IMAGE_H_MIRROR:
            GC2235_write_cmos_sensor(0x17,0x15);
			GC2235_write_cmos_sensor(0x92,0x03);
			GC2235_write_cmos_sensor(0x94,0x0b);
            break;
        case IMAGE_V_MIRROR://IMAGE_V_MIRROR:
            GC2235_write_cmos_sensor(0x17,0x16);
			GC2235_write_cmos_sensor(0x92,0x02);
			GC2235_write_cmos_sensor(0x94,0x0b);
            break;
        case IMAGE_HV_MIRROR://IMAGE_HV_MIRROR:
			GC2235_write_cmos_sensor(0x17,0x17);
			GC2235_write_cmos_sensor(0x92,0x02);
			GC2235_write_cmos_sensor(0x94,0x0b);
            break;
    }
}

UINT32 GC2235Preview(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
                                                MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{

	GC2235DB("GC2235Preview enter:");

	
	spin_lock(&gc2235mipiraw_drv_lock);
	gc2235.sensorMode = SENSOR_MODE_PREVIEW; // Need set preview setting after capture mode
	gc2235.DummyPixels = 0;//define dummy pixels and lines
	gc2235.DummyLines = 0 ;
	GC2235_FeatureControl_PERIOD_PixelNum=GC2235_PV_PERIOD_PIXEL_NUMS+ gc2235.DummyPixels;
	GC2235_FeatureControl_PERIOD_LineNum=GC2235_PV_PERIOD_LINE_NUMS+gc2235.DummyLines;
	spin_unlock(&gc2235mipiraw_drv_lock);

	//GC2235_write_shutter(gc2235.shutter);
	//write_GC2235_gain(gc2235.pvGain);
	//set mirror & flip
	//GC2235DB("[GC2235Preview] mirror&flip: %d \n",sensor_config_data->SensorImageMirror);
	spin_lock(&gc2235mipiraw_drv_lock);
	gc2235.imgMirror = sensor_config_data->SensorImageMirror;
	spin_unlock(&gc2235mipiraw_drv_lock);
	//if(SENSOR_MODE_PREVIEW==gc2235.sensorMode ) return ERROR_NONE;
	
	GC2235DB("GC2235Preview mirror:%d\n",sensor_config_data->SensorImageMirror);
	GC2235SetFlipMirror(GC2235_ORIENTATION);
	GC2235_SetDummy(gc2235.DummyPixels,gc2235.DummyLines);
	GC2235DB("GC2235Preview exit: \n");
	mdelay(100);
    return ERROR_NONE;
}	/* GC2235Preview() */


UINT32 GC2235Video(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
                                                MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	GC2235DB("GC2235Video enter:");
	if(gc2235.sensorMode == SENSOR_MODE_VIDEO)
	{
		// do nothing
	}
	else
		//GC2235VideoSetting();
	
	spin_lock(&gc2235mipiraw_drv_lock);
	gc2235.sensorMode = SENSOR_MODE_VIDEO;
	GC2235_FeatureControl_PERIOD_PixelNum=GC2235_VIDEO_PERIOD_PIXEL_NUMS+ gc2235.DummyPixels;
	GC2235_FeatureControl_PERIOD_LineNum=GC2235_VIDEO_PERIOD_LINE_NUMS+gc2235.DummyLines;
	spin_unlock(&gc2235mipiraw_drv_lock);

	//GC2235_write_shutter(gc2235.shutter);
	//write_GC2235_gain(gc2235.pvGain);

	spin_lock(&gc2235mipiraw_drv_lock);
	gc2235.imgMirror = sensor_config_data->SensorImageMirror;
	spin_unlock(&gc2235mipiraw_drv_lock);
	GC2235SetFlipMirror(GC2235_ORIENTATION);
	GC2235DB("GC2235Video mirror:%d\n",sensor_config_data->SensorImageMirror);

	//GC2235DBSOFIA("[GC2235Video]frame_len=%x\n", ((GC2235_read_cmos_sensor(0x380e)<<8)+GC2235_read_cmos_sensor(0x380f)));

	GC2235DB("GC2235Video exit:\n");
    return ERROR_NONE;
}


UINT32 GC2235Capture(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
                                                MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
 	kal_uint32 shutter = gc2235.shutter;
	kal_uint32 temp_data;
	if( SENSOR_MODE_CAPTURE== gc2235.sensorMode)
	{
		GC2235DB("GC2235Capture BusrtShot!!!\n");
	}else{
		GC2235DB("GC2235Capture enter:\n");
		
		spin_lock(&gc2235mipiraw_drv_lock);
		gc2235.pvShutter =shutter;
		gc2235.sensorGlobalGain = temp_data;
		gc2235.pvGain =gc2235.sensorGlobalGain;
		spin_unlock(&gc2235mipiraw_drv_lock);

		GC2235DB("[GC2235Capture]gc2235.shutter=%d, read_pv_shutter=%d, read_pv_gain = 0x%x\n",gc2235.shutter, shutter,gc2235.sensorGlobalGain);

		// Full size setting
		//.GC2235CaptureSetting();

		spin_lock(&gc2235mipiraw_drv_lock);
		gc2235.sensorMode = SENSOR_MODE_CAPTURE;
		gc2235.imgMirror = sensor_config_data->SensorImageMirror;
		gc2235.DummyPixels = 0;//define dummy pixels and lines                                                                                                         
		gc2235.DummyLines = 0 ;    
		GC2235_FeatureControl_PERIOD_PixelNum = GC2235_FULL_PERIOD_PIXEL_NUMS + gc2235.DummyPixels;
		GC2235_FeatureControl_PERIOD_LineNum = GC2235_FULL_PERIOD_LINE_NUMS + gc2235.DummyLines;
		spin_unlock(&gc2235mipiraw_drv_lock);

		//GC2235DB("[GC2235Capture] mirror&flip: %d\n",sensor_config_data->SensorImageMirror);
		
		GC2235DB("GC2235capture mirror:%d\n",sensor_config_data->SensorImageMirror);
		GC2235SetFlipMirror(GC2235_ORIENTATION);
		GC2235_SetDummy(gc2235.DummyPixels,gc2235.DummyLines);

	    if(GC2235CurrentScenarioId==MSDK_SCENARIO_ID_CAMERA_ZSD)
	    {
			GC2235DB("GC2235Capture exit ZSD!!\n");
			return ERROR_NONE;
	    }
		GC2235DB("GC2235Capture exit:\n");
	}

    return ERROR_NONE;
}	/* GC2235Capture() */

UINT32 GC2235GetResolution(MSDK_SENSOR_RESOLUTION_INFO_STRUCT *pSensorResolution)
{

    GC2235DB("GC2235GetResolution!!\n");
	pSensorResolution->SensorPreviewWidth	= GC2235_IMAGE_SENSOR_PV_WIDTH;
    pSensorResolution->SensorPreviewHeight	= GC2235_IMAGE_SENSOR_PV_HEIGHT;
    pSensorResolution->SensorFullWidth		= GC2235_IMAGE_SENSOR_FULL_WIDTH;
    pSensorResolution->SensorFullHeight		= GC2235_IMAGE_SENSOR_FULL_HEIGHT;
    pSensorResolution->SensorVideoWidth		= GC2235_IMAGE_SENSOR_VIDEO_WIDTH;
    pSensorResolution->SensorVideoHeight    = GC2235_IMAGE_SENSOR_VIDEO_HEIGHT;
    return ERROR_NONE;
}   /* GC2235GetResolution() */

UINT32 GC2235GetInfo(MSDK_SCENARIO_ID_ENUM ScenarioId,
                                                MSDK_SENSOR_INFO_STRUCT *pSensorInfo,
                                                MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData)
{

	pSensorInfo->SensorPreviewResolutionX= GC2235_IMAGE_SENSOR_PV_WIDTH;
	pSensorInfo->SensorPreviewResolutionY= GC2235_IMAGE_SENSOR_PV_HEIGHT;
	pSensorInfo->SensorFullResolutionX= GC2235_IMAGE_SENSOR_FULL_WIDTH;
    pSensorInfo->SensorFullResolutionY= GC2235_IMAGE_SENSOR_FULL_HEIGHT;

	spin_lock(&gc2235mipiraw_drv_lock);
	gc2235.imgMirror = pSensorConfigData->SensorImageMirror ;
	spin_unlock(&gc2235mipiraw_drv_lock);
    if(GC2235_ORIENTATION == IMAGE_V_MIRROR)
   		pSensorInfo->SensorOutputDataFormat= SENSOR_OUTPUT_FORMAT_RAW_B;
	else
		pSensorInfo->SensorOutputDataFormat= SENSOR_OUTPUT_FORMAT_RAW_Gb;
    pSensorInfo->SensorClockPolarity =SENSOR_CLOCK_POLARITY_LOW;
    pSensorInfo->SensorClockFallingPolarity=SENSOR_CLOCK_POLARITY_LOW;
    pSensorInfo->SensorHsyncPolarity = SENSOR_CLOCK_POLARITY_LOW;
    pSensorInfo->SensorVsyncPolarity = SENSOR_CLOCK_POLARITY_LOW;

    pSensorInfo->SensroInterfaceType=SENSOR_INTERFACE_TYPE_MIPI;
	pSensorInfo->MIPIsensorType = MIPI_OPHY_CSI2;

    pSensorInfo->CaptureDelayFrame = 4;
    pSensorInfo->PreviewDelayFrame = 3;
    pSensorInfo->VideoDelayFrame = 3;

    pSensorInfo->SensorDrivingCurrent = ISP_DRIVING_8MA;
    pSensorInfo->AEShutDelayFrame = 0;		    /* The frame of setting shutter default 0 for TG int */
    pSensorInfo->AESensorGainDelayFrame = 1;     /* The frame of setting sensor gain */
    pSensorInfo->AEISPGainDelayFrame = 3;

    switch (ScenarioId)
    {
        case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
            pSensorInfo->SensorClockFreq=24;
            pSensorInfo->SensorClockRisingCount= 0;

            pSensorInfo->SensorGrabStartX = GC2235_PV_X_START;
            pSensorInfo->SensorGrabStartY = GC2235_PV_Y_START;

            pSensorInfo->SensorMIPILaneNumber = SENSOR_MIPI_2_LANE;
            pSensorInfo->MIPIDataLowPwr2HighSpeedTermDelayCount = 0;
	     	pSensorInfo->MIPIDataLowPwr2HighSpeedSettleDelayCount = 14;
	    	pSensorInfo->MIPICLKLowPwr2HighSpeedTermDelayCount = 0;
            pSensorInfo->SensorPacketECCOrder = 1;
            break;
        case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
            pSensorInfo->SensorClockFreq=24;
            pSensorInfo->SensorClockRisingCount= 0;

            pSensorInfo->SensorGrabStartX = GC2235_VIDEO_X_START;
            pSensorInfo->SensorGrabStartY = GC2235_VIDEO_Y_START;

            pSensorInfo->SensorMIPILaneNumber = SENSOR_MIPI_2_LANE;
            pSensorInfo->MIPIDataLowPwr2HighSpeedTermDelayCount = 0;
	     	pSensorInfo->MIPIDataLowPwr2HighSpeedSettleDelayCount = 14;
	    	pSensorInfo->MIPICLKLowPwr2HighSpeedTermDelayCount = 0;
            pSensorInfo->SensorPacketECCOrder = 1;
            break;
        case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
		case MSDK_SCENARIO_ID_CAMERA_ZSD:
            pSensorInfo->SensorClockFreq=24;
            pSensorInfo->SensorClockRisingCount= 0;

            pSensorInfo->SensorGrabStartX = GC2235_FULL_X_START;	//2*GC2235_IMAGE_SENSOR_PV_STARTX;
            pSensorInfo->SensorGrabStartY = GC2235_FULL_Y_START;	//2*GC2235_IMAGE_SENSOR_PV_STARTY;

            pSensorInfo->SensorMIPILaneNumber = SENSOR_MIPI_2_LANE;
            pSensorInfo->MIPIDataLowPwr2HighSpeedTermDelayCount = 0;
            pSensorInfo->MIPIDataLowPwr2HighSpeedSettleDelayCount = 14;
            pSensorInfo->MIPICLKLowPwr2HighSpeedTermDelayCount = 0;
            pSensorInfo->SensorPacketECCOrder = 1;
            break;
        default:
			pSensorInfo->SensorClockFreq=24;
            pSensorInfo->SensorClockRisingCount= 0;

            pSensorInfo->SensorGrabStartX = GC2235_PV_X_START;
            pSensorInfo->SensorGrabStartY = GC2235_PV_Y_START;

            pSensorInfo->SensorMIPILaneNumber = SENSOR_MIPI_2_LANE;
            pSensorInfo->MIPIDataLowPwr2HighSpeedTermDelayCount = 0;
	     	pSensorInfo->MIPIDataLowPwr2HighSpeedSettleDelayCount = 14;
	    	pSensorInfo->MIPICLKLowPwr2HighSpeedTermDelayCount = 0;
            pSensorInfo->SensorPacketECCOrder = 1;
            break;
    }

    memcpy(pSensorConfigData, &GC2235SensorConfigData, sizeof(MSDK_SENSOR_CONFIG_STRUCT));

    return ERROR_NONE;
}   /* GC2235GetInfo() */


UINT32 GC2235Control(MSDK_SCENARIO_ID_ENUM ScenarioId, MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *pImageWindow,
                                                MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData)
{
		spin_lock(&gc2235mipiraw_drv_lock);
		GC2235CurrentScenarioId = ScenarioId;
		spin_unlock(&gc2235mipiraw_drv_lock);
		GC2235DB("GC2235CurrentScenarioId=%d\n",GC2235CurrentScenarioId);
    switch (ScenarioId)
    {
        case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
            GC2235Preview(pImageWindow, pSensorConfigData);
            break;
        case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			GC2235Video(pImageWindow, pSensorConfigData);
			break;
        case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
		case MSDK_SCENARIO_ID_CAMERA_ZSD:
            GC2235Capture(pImageWindow, pSensorConfigData);
            break;
        default:
            return ERROR_INVALID_SCENARIO_ID;
    }
    return ERROR_NONE;
} /* GC2235Control() */


UINT32 GC2235SetVideoMode(UINT16 u2FrameRate)
{
    kal_uint32 MIN_Frame_length =0,extralines=0;
    GC2235DB("[GC2235SetVideoMode] frame rate = %d\n", u2FrameRate);

	spin_lock(&gc2235mipiraw_drv_lock);
	 gc2235VIDEO_MODE_TARGET_FPS=u2FrameRate;
	spin_unlock(&gc2235mipiraw_drv_lock);

	if(u2FrameRate==0)
	{   
		GC2235DB("Disable Video Mode or dynimac fps\n");	
		spin_lock(&gc2235mipiraw_drv_lock);
		gc2235.DummyPixels = 0;//define dummy pixels and lines
		gc2235.DummyLines = extralines ;
		spin_unlock(&gc2235mipiraw_drv_lock);
		//GC2235_SetDummy(gc2235.DummyPixels ,gc2235.DummyLines);
		return KAL_TRUE;
	}
	
	if(u2FrameRate >30 || u2FrameRate <5)
	    GC2235DB("error frame rate seting %fps\n",u2FrameRate);

    if(gc2235.sensorMode == SENSOR_MODE_VIDEO)//video ScenarioId recording
    {
    	if(u2FrameRate==30)
    		{
		spin_lock(&gc2235mipiraw_drv_lock);
		gc2235.DummyPixels = 0;//define dummy pixels and lines
		gc2235.DummyLines = 0 ;
		spin_unlock(&gc2235mipiraw_drv_lock);
		
		GC2235_SetDummy(gc2235.DummyPixels,gc2235.DummyLines);
    		}
		else
		{
		spin_lock(&gc2235mipiraw_drv_lock);
		gc2235.DummyPixels = 0;//define dummy pixels and lines
		MIN_Frame_length = (GC2235MIPI_VIDEO_CLK)/(GC2235_VIDEO_PERIOD_PIXEL_NUMS + gc2235.DummyPixels)/u2FrameRate;
		gc2235.DummyLines = MIN_Frame_length - GC2235_VALID_LINE_NUMS-GC2235_DEFAULT_DUMMY_LINE_NUMS;
		spin_unlock(&gc2235mipiraw_drv_lock);
		GC2235DB("GC2235SetVideoMode MIN_Frame_length %d\n",MIN_Frame_length);
		
		GC2235_SetDummy(gc2235.DummyPixels,gc2235.DummyLines);
    		}	
    }
	else if(gc2235.sensorMode == SENSOR_MODE_CAPTURE)
	{
		GC2235DB("-------[GC2235SetVideoMode]ZSD???---------\n");
		if(u2FrameRate==30)
    		{
		spin_lock(&gc2235mipiraw_drv_lock);
		gc2235.DummyPixels = 0;//define dummy pixels and lines
		gc2235.DummyLines = 0 ;
		spin_unlock(&gc2235mipiraw_drv_lock);
		
		GC2235_SetDummy(gc2235.DummyPixels,gc2235.DummyLines);
    		}
		else
			{
		spin_lock(&gc2235mipiraw_drv_lock);
		gc2235.DummyPixels = 0;//define dummy pixels and lines
		MIN_Frame_length = (GC2235MIPI_VIDEO_CLK)/(GC2235_VIDEO_PERIOD_PIXEL_NUMS + gc2235.DummyPixels)/u2FrameRate;
		gc2235.DummyLines = MIN_Frame_length - GC2235_VALID_LINE_NUMS-GC2235_DEFAULT_DUMMY_LINE_NUMS;
		spin_unlock(&gc2235mipiraw_drv_lock);
		
		GC2235_SetDummy(gc2235.DummyPixels,gc2235.DummyLines);
    		}	
	}

    return KAL_TRUE;
}

UINT32 GC2235SetAutoFlickerMode(kal_bool bEnable, UINT16 u2FrameRate)
{
	//return ERROR_NONE;
    //GC2235DB("[GC2235SetAutoFlickerMode] frame rate(10base) = %d %d\n", bEnable, u2FrameRate);
	if(bEnable) {   // enable auto flicker
		spin_lock(&gc2235mipiraw_drv_lock);
		gc2235.GC2235AutoFlickerMode = KAL_TRUE;
		spin_unlock(&gc2235mipiraw_drv_lock);
    } else {
    	spin_lock(&gc2235mipiraw_drv_lock);
        gc2235.GC2235AutoFlickerMode = KAL_FALSE;
		spin_unlock(&gc2235mipiraw_drv_lock);
        GC2235DB("Disable Auto flicker\n");
    }

    return ERROR_NONE;
}

UINT32 GC2235SetTestPatternMode(kal_bool bEnable)
{
    GC2235DB("[GC2235SetTestPatternMode] Test pattern enable:%d\n", bEnable);
    if(bEnable)
    {
        GC2235_write_cmos_sensor(0x8b,0xb0); //bit[4]: 1 enable test pattern, 0 disable test pattern
    }
	else
	{
	    GC2235_write_cmos_sensor(0x8b,0xa0);//bit[4]: 1 enable test pattern, 0 disable test pattern
	}
    return ERROR_NONE;
}


UINT32 GC2235MIPISetMaxFramerateByScenario(MSDK_SCENARIO_ID_ENUM scenarioId, MUINT32 frameRate) {
	kal_uint32 pclk;
	kal_int16 dummyLine;
	kal_uint16 lineLength,frameHeight;
		
	GC2235DB("GC2235MIPISetMaxFramerateByScenario: scenarioId = %d, frame rate = %d\n",scenarioId,frameRate);
	switch (scenarioId) {
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
			pclk = GC2235MIPI_PREVIEW_CLK;
			lineLength = GC2235_PV_PERIOD_PIXEL_NUMS;
			frameHeight = (10 * pclk)/frameRate/lineLength;
			dummyLine = frameHeight - GC2235_VALID_LINE_NUMS;
			gc2235.sensorMode = SENSOR_MODE_PREVIEW;
			GC2235_SetDummy(0, dummyLine);			
			break;			
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			pclk = GC2235MIPI_VIDEO_CLK; 
			lineLength = GC2235_VIDEO_PERIOD_PIXEL_NUMS;
			frameHeight = (10 * pclk)/frameRate/lineLength;
			dummyLine = frameHeight - GC2235_VALID_LINE_NUMS;
			gc2235.sensorMode = SENSOR_MODE_VIDEO;
			GC2235_SetDummy(0, dummyLine);			
			break;			
			 break;
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
		case MSDK_SCENARIO_ID_CAMERA_ZSD:			
			pclk = GC2235MIPI_CAPTURE_CLK;
			lineLength = GC2235_FULL_PERIOD_PIXEL_NUMS;
			frameHeight = (10 * pclk)/frameRate/lineLength;
			dummyLine = frameHeight - GC2235_VALID_LINE_NUMS;
			gc2235.sensorMode = SENSOR_MODE_CAPTURE;
			GC2235_SetDummy(0, dummyLine);			
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
	return ERROR_NONE;
}


UINT32 GC2235MIPIGetDefaultFramerateByScenario(MSDK_SCENARIO_ID_ENUM scenarioId, MUINT32 *pframeRate) 
{

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

	return ERROR_NONE;
}


UINT32 GC2235FeatureControl(MSDK_SENSOR_FEATURE_ENUM FeatureId,
                                                                UINT8 *pFeaturePara,UINT32 *pFeatureParaLen)
{
    UINT16 *pFeatureReturnPara16=(UINT16 *) pFeaturePara;
    UINT16 *pFeatureData16=(UINT16 *) pFeaturePara;
    UINT32 *pFeatureReturnPara32=(UINT32 *) pFeaturePara;
    UINT32 *pFeatureData32=(UINT32 *) pFeaturePara;
    UINT32 SensorRegNumber;
    UINT32 i;
    PNVRAM_SENSOR_DATA_STRUCT pSensorDefaultData=(PNVRAM_SENSOR_DATA_STRUCT) pFeaturePara;
    MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData=(MSDK_SENSOR_CONFIG_STRUCT *) pFeaturePara;
    MSDK_SENSOR_REG_INFO_STRUCT *pSensorRegData=(MSDK_SENSOR_REG_INFO_STRUCT *) pFeaturePara;
    MSDK_SENSOR_GROUP_INFO_STRUCT *pSensorGroupInfo=(MSDK_SENSOR_GROUP_INFO_STRUCT *) pFeaturePara;
    MSDK_SENSOR_ITEM_INFO_STRUCT *pSensorItemInfo=(MSDK_SENSOR_ITEM_INFO_STRUCT *) pFeaturePara;
    MSDK_SENSOR_ENG_INFO_STRUCT	*pSensorEngInfo=(MSDK_SENSOR_ENG_INFO_STRUCT *) pFeaturePara;

    switch (FeatureId)
    {
        case SENSOR_FEATURE_GET_RESOLUTION:
            *pFeatureReturnPara16++= GC2235_IMAGE_SENSOR_FULL_WIDTH;
            *pFeatureReturnPara16= GC2235_IMAGE_SENSOR_FULL_HEIGHT;
            *pFeatureParaLen=4;
            break;
        case SENSOR_FEATURE_GET_PERIOD:
				*pFeatureReturnPara16++= GC2235_FeatureControl_PERIOD_PixelNum;
				*pFeatureReturnPara16= GC2235_FeatureControl_PERIOD_LineNum;
				*pFeatureParaLen=4;
				break;
        case SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ:
			switch(GC2235CurrentScenarioId)
			{
				case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
					*pFeatureReturnPara32 = GC2235MIPI_PREVIEW_CLK;
					*pFeatureParaLen=4;
					break;
				case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
					*pFeatureReturnPara32 = GC2235MIPI_VIDEO_CLK;
					*pFeatureParaLen=4;
					break;
				case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
				case MSDK_SCENARIO_ID_CAMERA_ZSD:
					*pFeatureReturnPara32 = GC2235MIPI_CAPTURE_CLK;
					*pFeatureParaLen=4;
					break;
				default:
					*pFeatureReturnPara32 = GC2235MIPI_CAPTURE_CLK;
					*pFeatureParaLen=4;
					break;
			}
		      break;

        case SENSOR_FEATURE_SET_ESHUTTER:
            GC2235_SetShutter(*pFeatureData16);
            break;
        case SENSOR_FEATURE_SET_NIGHTMODE:
            GC2235_NightMode((BOOL) *pFeatureData16);
            break;
        case SENSOR_FEATURE_SET_GAIN:
            GC2235_SetGain((UINT16) *pFeatureData16);
            break;
        case SENSOR_FEATURE_SET_FLASHLIGHT:
            break;
        case SENSOR_FEATURE_SET_ISP_MASTER_CLOCK_FREQ:
            //GC2235_isp_master_clock=*pFeatureData32;
            break;
        case SENSOR_FEATURE_SET_REGISTER:
            GC2235_write_cmos_sensor(pSensorRegData->RegAddr, pSensorRegData->RegData);
            break;
        case SENSOR_FEATURE_GET_REGISTER:
            pSensorRegData->RegData = GC2235_read_cmos_sensor(pSensorRegData->RegAddr);
            break;
        case SENSOR_FEATURE_SET_CCT_REGISTER:
            SensorRegNumber=FACTORY_END_ADDR;
            for (i=0;i<SensorRegNumber;i++)
            {
            	spin_lock(&gc2235mipiraw_drv_lock);
                GC2235SensorCCT[i].Addr=*pFeatureData32++;
                GC2235SensorCCT[i].Para=*pFeatureData32++;
				spin_unlock(&gc2235mipiraw_drv_lock);
            }
            break;
        case SENSOR_FEATURE_GET_CCT_REGISTER:
            SensorRegNumber=FACTORY_END_ADDR;
            if (*pFeatureParaLen<(SensorRegNumber*sizeof(SENSOR_REG_STRUCT)+4))
                return FALSE;
            *pFeatureData32++=SensorRegNumber;
            for (i=0;i<SensorRegNumber;i++)
            {
                *pFeatureData32++=GC2235SensorCCT[i].Addr;
                *pFeatureData32++=GC2235SensorCCT[i].Para;
            }
            break;
        case SENSOR_FEATURE_SET_ENG_REGISTER:
            SensorRegNumber=ENGINEER_END;
            for (i=0;i<SensorRegNumber;i++)
            {
            	spin_lock(&gc2235mipiraw_drv_lock);
                GC2235SensorReg[i].Addr=*pFeatureData32++;
                GC2235SensorReg[i].Para=*pFeatureData32++;
				spin_unlock(&gc2235mipiraw_drv_lock);
            }
            break;
        case SENSOR_FEATURE_GET_ENG_REGISTER:
            SensorRegNumber=ENGINEER_END;
            if (*pFeatureParaLen<(SensorRegNumber*sizeof(SENSOR_REG_STRUCT)+4))
                return FALSE;
            *pFeatureData32++=SensorRegNumber;
            for (i=0;i<SensorRegNumber;i++)
            {
                *pFeatureData32++=GC2235SensorReg[i].Addr;
                *pFeatureData32++=GC2235SensorReg[i].Para;
            }
            break;
        case SENSOR_FEATURE_GET_REGISTER_DEFAULT:
            if (*pFeatureParaLen>=sizeof(NVRAM_SENSOR_DATA_STRUCT))
            {
                pSensorDefaultData->Version=NVRAM_CAMERA_SENSOR_FILE_VERSION;
                pSensorDefaultData->SensorId=GC2235_SENSOR_ID;
                memcpy(pSensorDefaultData->SensorEngReg, GC2235SensorReg, sizeof(SENSOR_REG_STRUCT)*ENGINEER_END);
                memcpy(pSensorDefaultData->SensorCCTReg, GC2235SensorCCT, sizeof(SENSOR_REG_STRUCT)*FACTORY_END_ADDR);
            }
            else
                return FALSE;
            *pFeatureParaLen=sizeof(NVRAM_SENSOR_DATA_STRUCT);
            break;
        case SENSOR_FEATURE_GET_CONFIG_PARA:
            memcpy(pSensorConfigData, &GC2235SensorConfigData, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
            *pFeatureParaLen=sizeof(MSDK_SENSOR_CONFIG_STRUCT);
            break;
        case SENSOR_FEATURE_CAMERA_PARA_TO_SENSOR:
            GC2235_camera_para_to_sensor();
            break;

        case SENSOR_FEATURE_SENSOR_TO_CAMERA_PARA:
            GC2235_sensor_to_camera_para();
            break;
        case SENSOR_FEATURE_GET_GROUP_COUNT:
            *pFeatureReturnPara32++=GC2235_get_sensor_group_count();
            *pFeatureParaLen=4;
            break;
        case SENSOR_FEATURE_GET_GROUP_INFO:
            GC2235_get_sensor_group_info(pSensorGroupInfo->GroupIdx, pSensorGroupInfo->GroupNamePtr, &pSensorGroupInfo->ItemCount);
            *pFeatureParaLen=sizeof(MSDK_SENSOR_GROUP_INFO_STRUCT);
            break;
        case SENSOR_FEATURE_GET_ITEM_INFO:
            GC2235_get_sensor_item_info(pSensorItemInfo->GroupIdx,pSensorItemInfo->ItemIdx, pSensorItemInfo);
            *pFeatureParaLen=sizeof(MSDK_SENSOR_ITEM_INFO_STRUCT);
            break;

        case SENSOR_FEATURE_SET_ITEM_INFO:
            GC2235_set_sensor_item_info(pSensorItemInfo->GroupIdx, pSensorItemInfo->ItemIdx, pSensorItemInfo->ItemValue);
            *pFeatureParaLen=sizeof(MSDK_SENSOR_ITEM_INFO_STRUCT);
            break;

        case SENSOR_FEATURE_GET_ENG_INFO:
            pSensorEngInfo->SensorId = 129;
            pSensorEngInfo->SensorType = CMOS_SENSOR;
            pSensorEngInfo->SensorOutputDataFormat=SENSOR_OUTPUT_FORMAT_RAW_B;
            *pFeatureParaLen=sizeof(MSDK_SENSOR_ENG_INFO_STRUCT);
            break;
        case SENSOR_FEATURE_GET_LENS_DRIVER_ID:
            // get the lens driver ID from EEPROM or just return LENS_DRIVER_ID_DO_NOT_CARE
            // if EEPROM does not exist in camera module.
            *pFeatureReturnPara32=LENS_DRIVER_ID_DO_NOT_CARE;
            *pFeatureParaLen=4;
            break;

        case SENSOR_FEATURE_INITIALIZE_AF:
            break;
        case SENSOR_FEATURE_CONSTANT_AF:
            break;
        case SENSOR_FEATURE_MOVE_FOCUS_LENS:
            break;
        case SENSOR_FEATURE_SET_VIDEO_MODE:
            GC2235SetVideoMode(*pFeatureData16);
            break;
        case SENSOR_FEATURE_CHECK_SENSOR_ID:
            GC2235GetSensorID(pFeatureReturnPara32);
            break;
        case SENSOR_FEATURE_SET_AUTO_FLICKER_MODE:
            GC2235SetAutoFlickerMode((BOOL)*pFeatureData16, *(pFeatureData16+1));
	        break;
        case SENSOR_FEATURE_SET_TEST_PATTERN:
            GC2235SetTestPatternMode((BOOL)*pFeatureData16);
            break;
	    case SENSOR_FEATURE_GET_TEST_PATTERN_CHECKSUM_VALUE:
			*pFeatureReturnPara32=GC2235_TEST_PATTERN_CHECKSUM;
			*pFeatureParaLen=4;
			break;
		case SENSOR_FEATURE_SET_MAX_FRAME_RATE_BY_SCENARIO:
			GC2235MIPISetMaxFramerateByScenario((MSDK_SCENARIO_ID_ENUM)*pFeatureData32, *(pFeatureData32+1));
			break;
		case SENSOR_FEATURE_GET_DEFAULT_FRAME_RATE_BY_SCENARIO:
			GC2235MIPIGetDefaultFramerateByScenario((MSDK_SCENARIO_ID_ENUM)*pFeatureData32, (MUINT32 *)(*(pFeatureData32+1)));
			break;
        default:
            break;
    }
    return ERROR_NONE;
}	/* GC2235FeatureControl() */


SENSOR_FUNCTION_STRUCT	SensorFuncGC2235=
{
    GC2235Open,
    GC2235GetInfo,
    GC2235GetResolution,
    GC2235FeatureControl,
    GC2235Control,
    GC2235Close
};

UINT32 GC2235_MIPI_RAW_SensorInit(PSENSOR_FUNCTION_STRUCT *pfFunc)
{
    /* To Do : Check Sensor status here */
    if (pfFunc!=NULL)
        *pfFunc=&SensorFuncGC2235;

    return ERROR_NONE;
}   /* SensorInit() */
