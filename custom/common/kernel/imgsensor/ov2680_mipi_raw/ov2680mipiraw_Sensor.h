/*****************************************************************************
 *
 * Filename:
 * ---------
 *   sensor.h
 *
 * Project:
 * --------
 *   DUMA
 *
 * Description:
 * ------------
 *   CMOS sensor header file
 *
 ****************************************************************************/
#ifndef _OV2680_SENSOR_H
#define _OV2680_SENSOR_H

#define OV2680_DEBUG
#define OV2680_DRIVER_TRACE
//#define OV2680_TEST_PATTEM
#ifdef OV2680_DEBUG
//#define SENSORDB printk
#else
//#define SENSORDB(x,...)
#endif


#define OV2680_FACTORY_START_ADDR 0
#define OV2680_ENGINEER_START_ADDR 10

//#define MIPI_INTERFACE
// sensor ID 0x2680
 
typedef enum OV2680_group_enum
{
  OV2680_PRE_GAIN = 0,
  OV2680_CMMCLK_CURRENT,
  OV2680_FRAME_RATE_LIMITATION,
  OV2680_REGISTER_EDITOR,
  OV2680_GROUP_TOTAL_NUMS
} OV2680_FACTORY_GROUP_ENUM;

typedef enum OV2680_register_index
{
  OV2680_SENSOR_BASEGAIN = OV2680_FACTORY_START_ADDR,
  OV2680_PRE_GAIN_R_INDEX,
  OV2680_PRE_GAIN_Gr_INDEX,
  OV2680_PRE_GAIN_Gb_INDEX,
  OV2680_PRE_GAIN_B_INDEX,
  OV2680_FACTORY_END_ADDR
} OV2680_FACTORY_REGISTER_INDEX;

typedef enum OV2680_engineer_index
{
  OV2680_CMMCLK_CURRENT_INDEX = OV2680_ENGINEER_START_ADDR,
  OV2680_ENGINEER_END
} OV2680_FACTORY_ENGINEER_INDEX;

typedef struct _sensor_data_struct
{
  SENSOR_REG_STRUCT reg[OV2680_ENGINEER_END];
  SENSOR_REG_STRUCT cct[OV2680_FACTORY_END_ADDR];
} sensor_data_struct;


#define OV2680_COLOR_FORMAT                    SENSOR_OUTPUT_FORMAT_RAW_B

#define OV2680_MIN_ANALOG_GAIN  1   /* 1x */
#define OV2680_MAX_ANALOG_GAIN      32 /* 32x */

 
/* FRAME RATE UNIT */
#define OV2680_FPS(x)                          (10 * (x))

#define OV2680_MIPI_LANE_NUM	1

#define OV2680_PREVIEW_CLK   66000000
#define OV2680_CAPTURE_CLK   66000000
#define OV2680_VIDEO_CLK     66000000
#define OV2680_ZSD_PRE_CLK   66000000

/* SENSOR PIXEL/LINE NUMBERS IN ONE PERIOD */
#define OV2680_FULL_PERIOD_PIXEL_NUMS          (1700) // 0x06a4)  //  15fps
#define OV2680_FULL_PERIOD_LINE_NUMS           1294 //0x050e

#define OV2680_PV_PERIOD_PIXEL_NUMS            OV2680_FULL_PERIOD_PIXEL_NUMS //
#define OV2680_PV_PERIOD_LINE_NUMS             OV2680_FULL_PERIOD_LINE_NUMS //

#define OV2680_VIDEO_PERIOD_PIXEL_NUMS         OV2680_FULL_PERIOD_PIXEL_NUMS  //
#define OV2680_VIDEO_PERIOD_LINE_NUMS          OV2680_FULL_PERIOD_LINE_NUMS  //

#define OV2680_3D_FULL_PERIOD_PIXEL_NUMS       OV2680_FULL_PERIOD_PIXEL_NUMS /* 15 fps */
#define OV2680_3D_FULL_PERIOD_LINE_NUMS        OV2680_FULL_PERIOD_LINE_NUMS
#define OV2680_3D_PV_PERIOD_PIXEL_NUMS         OV2680_FULL_PERIOD_PIXEL_NUMS /* 30 fps */
#define OV2680_3D_PV_PERIOD_LINE_NUMS          OV2680_FULL_PERIOD_LINE_NUMS
#define OV2680_3D_VIDEO_PERIOD_PIXEL_NUMS      OV2680_FULL_PERIOD_PIXEL_NUMS /* 30 fps */
#define OV2680_3D_VIDEO_PERIOD_LINE_NUMS       OV2680_FULL_PERIOD_LINE_NUMS
/* SENSOR START/END POSITION */
#define OV2680_FULL_X_START                    0//10
#define OV2680_FULL_Y_START                    0//10
#define OV2680_IMAGE_SENSOR_FULL_WIDTH         1600
#define OV2680_IMAGE_SENSOR_FULL_HEIGHT        1200

#define OV2680_PV_X_START                      0 // 2
#define OV2680_PV_Y_START                      0 // 2
#define OV2680_IMAGE_SENSOR_PV_WIDTH           OV2680_IMAGE_SENSOR_FULL_WIDTH
#define OV2680_IMAGE_SENSOR_PV_HEIGHT          OV2680_IMAGE_SENSOR_FULL_HEIGHT

#define OV2680_VIDEO_X_START                   0 //9
#define OV2680_VIDEO_Y_START                   0 //11
#define OV2680_IMAGE_SENSOR_VIDEO_WIDTH        OV2680_IMAGE_SENSOR_FULL_WIDTH
#define OV2680_IMAGE_SENSOR_VIDEO_HEIGHT       OV2680_IMAGE_SENSOR_FULL_HEIGHT

#define OV2680_3D_FULL_X_START                 0
#define OV2680_3D_FULL_Y_START                 0
#define OV2680_IMAGE_SENSOR_3D_FULL_WIDTH      OV2680_IMAGE_SENSOR_FULL_WIDTH
#define OV2680_IMAGE_SENSOR_3D_FULL_HEIGHT     OV2680_IMAGE_SENSOR_FULL_HEIGHT
#define OV2680_3D_PV_X_START                   2
#define OV2680_3D_PV_Y_START                   2
#define OV2680_IMAGE_SENSOR_3D_PV_WIDTH        OV2680_IMAGE_SENSOR_FULL_WIDTH
#define OV2680_IMAGE_SENSOR_3D_PV_HEIGHT       OV2680_IMAGE_SENSOR_FULL_HEIGHT
#define OV2680_3D_VIDEO_X_START                2
#define OV2680_3D_VIDEO_Y_START                2
#define OV2680_IMAGE_SENSOR_3D_VIDEO_WIDTH     OV2680_IMAGE_SENSOR_FULL_WIDTH
#define OV2680_IMAGE_SENSOR_3D_VIDEO_HEIGHT    OV2680_IMAGE_SENSOR_FULL_HEIGHT



/* SENSOR READ/WRITE ID */

#define OV2680_SLAVE_WRITE_ID_1   (0x6c)
#define OV2680_SLAVE_WRITE_ID_2   (0x20)
/************OTP Feature*********************/
//#define OV2680_USE_OTP
//#define OV2680_USE_WB_OTP

#if defined(OV2680_USE_OTP)

#endif
/************OTP Feature*********************/

/* SENSOR PRIVATE STRUCT */
typedef struct OV2680_sensor_STRUCT
{
  MSDK_SENSOR_CONFIG_STRUCT cfg_data;
  sensor_data_struct eng; /* engineer mode */
  MSDK_SENSOR_ENG_INFO_STRUCT eng_info;
  kal_uint8 mirror;
  kal_bool pv_mode;
  kal_bool video_mode;  
  //kal_bool NightMode;
  kal_bool is_zsd;
  kal_bool is_zsd_cap;
  kal_bool is_autofliker;
  //kal_uint16 normal_fps; /* video normal mode max fps */
  //kal_uint16 night_fps; /* video night mode max fps */  
  kal_uint16 FixedFps;
  kal_uint16 shutter;
  kal_uint16 gain;
  kal_uint32 pv_pclk;
  kal_uint32 cap_pclk;
  kal_uint32 pclk;
  kal_uint16 frame_height;
  kal_uint16 line_length;  
  kal_uint16 write_id;
  kal_uint16 read_id;
  kal_uint16 dummy_pixels;
  kal_uint16 dummy_lines;
  kal_bool enter_preview;
} OV2680_sensor_struct;

//export functions
UINT32 OV2680Open(void);
UINT32 OV2680Control(MSDK_SCENARIO_ID_ENUM ScenarioId, MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *pImageWindow, MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData);
UINT32 OV2680FeatureControl(MSDK_SENSOR_FEATURE_ENUM FeatureId, UINT8 *pFeaturePara,UINT32 *pFeatureParaLen);
UINT32 OV2680GetInfo(MSDK_SCENARIO_ID_ENUM ScenarioId, MSDK_SENSOR_INFO_STRUCT *pSensorInfo, MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData);
UINT32 OV2680GetResolution(MSDK_SENSOR_RESOLUTION_INFO_STRUCT *pSensorResolution);
UINT32 OV2680Close(void);

#define Sleep(ms) mdelay(ms)

#endif 
