/*******************************************************************************************/


/*******************************************************************************************/

/* SENSOR FULL SIZE */
#ifndef __SENSOR_H
#define __SENSOR_H



typedef enum group_enum {
    PRE_GAIN=0,
    CMMCLK_CURRENT,
    FRAME_RATE_LIMITATION,
    REGISTER_EDITOR,
    GROUP_TOTAL_NUMS
} FACTORY_GROUP_ENUM;


#define ENGINEER_START_ADDR 10
#define FACTORY_START_ADDR 0

typedef enum engineer_index
{
    CMMCLK_CURRENT_INDEX=ENGINEER_START_ADDR,
    ENGINEER_END
} FACTORY_ENGINEER_INDEX;

typedef enum register_index
{
	SENSOR_BASEGAIN=FACTORY_START_ADDR,
	PRE_GAIN_R_INDEX,
	PRE_GAIN_Gr_INDEX,
	PRE_GAIN_Gb_INDEX,
	PRE_GAIN_B_INDEX,
	FACTORY_END_ADDR
} FACTORY_REGISTER_INDEX;


typedef struct
{
    SENSOR_REG_STRUCT	Reg[ENGINEER_END];
    SENSOR_REG_STRUCT	CCT[FACTORY_END_ADDR];
} SENSOR_DATA_STRUCT, *PSENSOR_DATA_STRUCT;

typedef enum {
    SENSOR_MODE_INIT = 0,
    SENSOR_MODE_PREVIEW,
    SENSOR_MODE_VIDEO,
    SENSOR_MODE_CAPTURE
} GC2235_SENSOR_MODE;


typedef struct
{
	kal_uint32 DummyPixels;
	kal_uint32 DummyLines;
	
	kal_uint32 pvShutter;
	kal_uint32 pvGain;
	kal_uint32 Gain;
	
	kal_uint32 pvPclk;  // x10 480 for 48MHZ
	kal_uint32 videoPclk;
	kal_uint32 capPclk; // x10
	
	kal_uint32 shutter;
	kal_uint32 maxExposureLines;

	kal_uint16 sensorGlobalGain;//sensor gain read from 0x350a 0x350b;
	kal_uint16 ispBaseGain;//64
	kal_uint16 realGain;//ispBaseGain as 1x

	kal_int16 imgMirror;

	GC2235_SENSOR_MODE sensorMode;

	kal_bool GC2235AutoFlickerMode;
	kal_bool GC2235VideoMode;
	
}GC2235_PARA_STRUCT,*PGC2235_PARA_STRUCT;

	#define GC2235_MIPI_2_Lane
	#define GC2235_MIPI_30FPS

	#define GC2235_IMAGE_SENSOR_FULL_WIDTH					1600	
	#define GC2235_IMAGE_SENSOR_FULL_HEIGHT					1200

	/* SENSOR PV SIZE */
	#define GC2235_IMAGE_SENSOR_PV_WIDTH					1600
	#define GC2235_IMAGE_SENSOR_PV_HEIGHT					1200

	#define GC2235_IMAGE_SENSOR_VIDEO_WIDTH					(1600)
	#define GC2235_IMAGE_SENSOR_VIDEO_HEIGHT				(1200)
	

	/* SENSOR SCALER FACTOR */
	#define GC2235_PV_SCALER_FACTOR					    	3
	#define GC2235_FULL_SCALER_FACTOR					    1
	                                        	
	/* SENSOR START/EDE POSITION */         	
	#define GC2235_FULL_X_START						    		(0)
	#define GC2235_FULL_Y_START						    		(0)

	
	#define GC2235_PV_X_START						    		0
	#define GC2235_PV_Y_START						    		0

	
	#define GC2235_VIDEO_X_START								0
	#define GC2235_VIDEO_Y_START								0


	#define GC2235_MAX_ANALOG_GAIN					(6)
	#define GC2235_MIN_ANALOG_GAIN					(1)
	#define GC2235_ANALOG_GAIN_1X						(0x0020)
#ifdef GC2235_MIPI_30FPS
	#define GC2235MIPI_CAPTURE_CLK 	(42000000)
	#define GC2235MIPI_PREVIEW_CLK 	(42000000)
	#define GC2235MIPI_VIDEO_CLK	(42000000)
#else //20fps
	#define GC2235MIPI_CAPTURE_CLK	(30000000)
	#define GC2235MIPI_PREVIEW_CLK	(30000000)
	#define GC2235MIPI_VIDEO_CLK	(30000000)
#endif

	/* SENSOR PIXEL/LINE NUMBERS IN ONE PERIOD */


#define GC2235_VALID_IXEL_NUMS            822
#define GC2235_VALID_LINE_NUMS            1232
#ifdef GC2235_MIPI_30FPS
#define GC2235_DEFAULT_DUMMY_PIXEL_NUMS   295 //244 
#define GC2235_DEFAULT_DUMMY_LINE_NUMS    20 //64
#define GC2235_VIDEO_PERIOD_PIXEL_NUMS          (1117)
#define GC2235_VIDEO_PERIOD_LINE_NUMS           (1252)
							
#define GC2235_PV_PERIOD_PIXEL_NUMS            (1117)
#define GC2235_PV_PERIOD_LINE_NUMS             (1252)

#define GC2235_FULL_PERIOD_PIXEL_NUMS          (1117)
#define GC2235_FULL_PERIOD_LINE_NUMS           (1252)

#else //20fps
#define GC2235_DEFAULT_DUMMY_PIXEL_NUMS   285 //244 
#define GC2235_DEFAULT_DUMMY_LINE_NUMS    123 //64
#define GC2235_VIDEO_PERIOD_PIXEL_NUMS          (1107)
#define GC2235_VIDEO_PERIOD_LINE_NUMS           (1355)
							
#define GC2235_PV_PERIOD_PIXEL_NUMS             (1107)
#define GC2235_PV_PERIOD_LINE_NUMS             (1355)

#define GC2235_FULL_PERIOD_PIXEL_NUMS          (1107)
#define GC2235_FULL_PERIOD_LINE_NUMS           (1355)

#endif

	

#define GC2235_MIN_LINE_LENGTH						0x1100     
#define GC2235_MIN_FRAME_LENGTH						0xc00	  

#define GC2235_MAX_LINE_LENGTH						0xCCCC
#define GC2235_MAX_FRAME_LENGTH						0xFFFF

	/* DUMMY NEEDS TO BE INSERTED */
	/* SETUP TIME NEED TO BE INSERTED */

#define GC2235MIPI_WRITE_ID 	(0x78)
#define GC2235MIPI_READ_ID	(0x79)

// SENSOR CHIP VERSION

#define GC2235MIPI_SENSOR_ID            GC2235_SENSOR_ID

#define GC2235MIPI_PAGE_SETTING_REG    (0xFF)

//s_add for porting
//s_add for porting
//s_add for porting

//export functions
UINT32 GC2235MIPIOpen(void);
UINT32 GC2235MIPIGetResolution(MSDK_SENSOR_RESOLUTION_INFO_STRUCT *pSensorResolution);
UINT32 GC2235MIPIGetInfo(MSDK_SCENARIO_ID_ENUM ScenarioId, MSDK_SENSOR_INFO_STRUCT *pSensorInfo, MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData);
UINT32 GC2235MIPIControl(MSDK_SCENARIO_ID_ENUM ScenarioId, MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *pImageWindow, MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData);
UINT32 GC2235MIPIFeatureControl(MSDK_SENSOR_FEATURE_ENUM FeatureId, UINT8 *pFeaturePara,UINT32 *pFeatureParaLen);
UINT32 GC2235MIPIClose(void);

//#define Sleep(ms) mdelay(ms)
//#define RETAILMSG(x,...)
//#define TEXT

//e_add for porting
//e_add for porting
//e_add for porting

#endif /* __SENSOR_H */

