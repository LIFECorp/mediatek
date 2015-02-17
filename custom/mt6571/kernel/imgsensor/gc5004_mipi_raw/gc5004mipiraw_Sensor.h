/*****************************************************************************
 *
 * Filename:
 * ---------
 *   gc5004mipiraw_sensor.h
 *
 * Project:
 * --------
 *   YUSU
 *
 * Description:
 * ------------
 *   Header file of Sensor driver
 *
 *
 * Author:

 *============================================================================
 ****************************************************************************/
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


/* Real PV Size, i.e. the size after all ISP processing (so already -4/-6), before MDP. */

#define GC5004MIPI_REAL_PV_WIDTH				1280//(1600)
#define GC5004MIPI_REAL_PV_HEIGHT				960//(1200)
/* Real CAP Size, i.e. the size after all ISP processing (so already -4/-6), before MDP. */
#define GC5004MIPI_REAL_CAP_WIDTH				2560
#define GC5004MIPI_REAL_CAP_HEIGHT				1920
#if 0
#define GC5004MIPI_REAL_VIDEO_WIDTH				1920
#define GC5004MIPI_REAL_VIDEO_HEIGHT			1080
#else
#define GC5004MIPI_REAL_VIDEO_WIDTH				1280
#define GC5004MIPI_REAL_VIDEO_HEIGHT			960
#endif

/* X/Y Starting point */
#define GC5004MIPI_IMAGE_SENSOR_PV_STARTX       0
#define GC5004MIPI_IMAGE_SENSOR_PV_STARTY       0	// 
#define GC5004MIPI_IMAGE_SENSOR_CAP_STARTX		12
#define GC5004MIPI_IMAGE_SENSOR_CAP_STARTY		2	// 
#define GC5004MIPI_IMAGE_SENSOR_VIDEO_STARTX      0
#define GC5004MIPI_IMAGE_SENSOR_VIDEO_STARTY      0	// 

/* SENSOR 2M SIZE */
#define GC5004MIPI_IMAGE_SENSOR_PV_WIDTH		(GC5004MIPI_REAL_PV_WIDTH  + 2 * GC5004MIPI_IMAGE_SENSOR_PV_STARTX)	//
#define GC5004MIPI_IMAGE_SENSOR_PV_HEIGHT		(GC5004MIPI_REAL_PV_HEIGHT + 2 * GC5004MIPI_IMAGE_SENSOR_PV_STARTY)	//
/* SENSOR 8M SIZE */
#define GC5004MIPI_IMAGE_SENSOR_FULL_WIDTH		(GC5004MIPI_REAL_CAP_WIDTH  + 2 * GC5004MIPI_IMAGE_SENSOR_CAP_STARTX)	// 
#define GC5004MIPI_IMAGE_SENSOR_FULL_HEIGHT		(GC5004MIPI_REAL_CAP_HEIGHT + 2 * GC5004MIPI_IMAGE_SENSOR_CAP_STARTY)	//

#define GC5004MIPI_IMAGE_SENSOR_VIDEO_WIDTH		(GC5004MIPI_REAL_VIDEO_WIDTH  + 2 * GC5004MIPI_IMAGE_SENSOR_VIDEO_STARTX)	// 
#define GC5004MIPI_IMAGE_SENSOR_VIDEO_HEIGHT	(GC5004MIPI_REAL_VIDEO_HEIGHT + 2 * GC5004MIPI_IMAGE_SENSOR_VIDEO_STARTY)	// 

#define GC5004MIPI_PV_LINE_LENGTH_PIXELS 						(4500)
#define GC5004MIPI_PV_FRAME_LENGTH_LINES						(1988)	
#define GC5004MIPI_FULL_LINE_LENGTH_PIXELS 						(4800)
#define GC5004MIPI_FULL_FRAME_LENGTH_LINES			            (1988)
#if 0
#define GC5004MIPI_VIDEO_LINE_LENGTH_PIXELS 					(3252)
#define GC5004MIPI_VIDEO_FRAME_LENGTH_LINES						(1124)	
#else
#define GC5004MIPI_VIDEO_LINE_LENGTH_PIXELS 					(4500)
#define GC5004MIPI_VIDEO_FRAME_LENGTH_LINES						(1988)	
#endif



#define GC5004MIPI_WRITE_ID (0x6c)
#define GC5004MIPI_READ_ID	(0x6d)



/* SENSOR PRIVATE STRUCT */
struct GC5004_SENSOR_STRUCT
{
	kal_uint8 i2c_write_id;
	kal_uint8 i2c_read_id;

};

struct GC5004MIPI_sensor_STRUCT
	{	 
		  kal_uint16 i2c_write_id;
		  kal_bool pv_mode; 
		  kal_bool video_mode; 				
		  kal_bool capture_mode; 				//True: Preview Mode; False: Capture Mode
		  kal_bool night_mode;				//True: Night Mode; False: Auto Mode
		  kal_uint8 mirror_flip;
		  kal_uint32 pv_pclk;				//Preview Pclk
		  kal_uint32 video_pclk;				//video Pclk
		  kal_uint32 cp_pclk;				//Capture Pclk
		  kal_uint32 pv_shutter;		   
		  kal_uint32 video_shutter;		   
		  kal_uint32 cp_shutter;
		  kal_uint32 pv_gain;
		  kal_uint32 video_gain;
		  kal_uint32 cp_gain;
		  kal_uint32 pv_line_length;
		  kal_uint32 pv_frame_length;
		  kal_uint32 video_line_length;
		  kal_uint32 video_frame_length;
		  kal_uint32 cp_line_length;
		  kal_uint32 cp_frame_length;
		  kal_uint16 pv_HB;		   
		  kal_uint16 pv_VB;		   
		  kal_uint16 video_HB;		   
		  kal_uint16 video_VB;		   
		  kal_uint16 cp_HB;		   
		  kal_uint16 cp_VB;		  	
		   
		  kal_uint16 video_current_frame_rate;
	};

// SENSOR CHIP VERSION

#define GC5004MIPI_SENSOR_ID            GC5004_SENSOR_ID

#define GC5004MIPI_PAGE_SETTING_REG    (0xFF)

//s_add for porting
//s_add for porting
//s_add for porting

//export functions
UINT32 GC5004MIPIOpen(void);
UINT32 GC5004MIPIGetResolution(MSDK_SENSOR_RESOLUTION_INFO_STRUCT *pSensorResolution);
UINT32 GC5004MIPIGetInfo(MSDK_SCENARIO_ID_ENUM ScenarioId, MSDK_SENSOR_INFO_STRUCT *pSensorInfo, MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData);
UINT32 GC5004MIPIControl(MSDK_SCENARIO_ID_ENUM ScenarioId, MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *pImageWindow, MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData);
UINT32 GC5004MIPIFeatureControl(MSDK_SENSOR_FEATURE_ENUM FeatureId, UINT8 *pFeaturePara,UINT32 *pFeatureParaLen);
UINT32 GC5004MIPIClose(void);

//#define Sleep(ms) mdelay(ms)
//#define RETAILMSG(x,...)
//#define TEXT

//e_add for porting
//e_add for porting
//e_add for porting

#endif /* __SENSOR_H */

