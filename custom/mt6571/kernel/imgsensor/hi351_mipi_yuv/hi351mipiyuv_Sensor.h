/*****************************************************************************
 *
 * Filename:
 * ---------
 *   sensor.h
 *
 * Project:
 * --------
 *   ALPS
 *
 * Description:
 * ------------
 *   Header file of Sensor driver
 *
 *
 * Author:
 * -------
 *   Qihao Geng (mtk70548)
 *
 *============================================================================
 *             HISTORY
 * Below this line, this part is controlled by CC/CQ. DO NOT MODIFY!!
 *------------------------------------------------------------------------------
 * $Revision:$
 * $Modtime:$
 * $Log:$
 *
 * 07 08 2013 yan.xu
 * [ALPS00732488] HI351 mipi yuv sensor driver check in
 * .
 *
 * 07 01 2013 yan.xu
 * [ALPS00732488] HI351 mipi yuv sensor driver check in
 * .
 *
 * 06 14 2013 yan.xu
 * [ALPS00732488] HI351 mipi yuv sensor driver check in
 * .
 *
 * 05 27 2013 yan.xu
 * [ALPS00732488] HI351 mipi yuv sensor driver check in
 * .
 *
 * 07 30 2012 qihao.geng
 * NULL
 * 1. support burst i2c write to save entry time.
 * 2. solve the view angle difference issue.
 * 3. video fix frame rate is OK and ready to release.
 *
 * 07 25 2012 qihao.geng
 * NULL
 * Increase the ZSD and read/write shutter/gain function.
 *
 * 07 20 2012 qihao.geng
 * NULL
 * HI351 MIPI Sensor Driver Check In 1st Versoin, can preview/capture.
 *
 * 07 19 2012 qihao.geng
 * NULL
 * HI351 MIPI YUV Sensor Driver Add, 1st version.
 *
 *
 *------------------------------------------------------------------------------
 * Upper this line, this part is controlled by CC/CQ. DO NOT MODIFY!!
 *============================================================================
 ****************************************************************************/
#ifndef __HI351_MIPI_YUV_SENSOR_H
    #define __HI351_MIPI_YUV_SENSOR_H

    #define __HI351_DEBUG_TRACE__
#define HI351YUV_DEBUG
#ifdef HI351YUV_DEBUG
    #define HI351_TRACE printk
#else
    #define HI351_TRACE(x,...)
#endif


/* SENSOR READ/WRITE ID */
#define HI351_WRITE_ID                      0x40
#define HI351_READ_ID                       0x41

#define HI351_PV_PERIOD_PIXEL_NUMS      (1140+110)    /* Default preview line length */
#define HI351_PV_PERIOD_LINE_NUMS       (802+164)     /* Default preview frame length */
#define HI351_FULL_PERIOD_PIXEL_NUMS    (2180+110) //0x6E    /* Default full size line length */
#define HI351_FULL_PERIOD_LINE_NUMS     (1586+164) //0xA4   /* Default full size frame length */

/* SENSOR PREVIEW SIZE & START GRAB PIXEL OFFSET */
#define IMAGE_SENSOR_PV_GRAB_START_X        (0)//(0)
#define IMAGE_SENSOR_PV_GRAB_START_Y        (2)//(1)
#define HI351_IMAGE_SENSOR_PV_WIDTH         (1024-16)//-4
#define HI351_IMAGE_SENSOR_PV_HEIGHT        (768-2)//-1

#define HI351_IMAGE_SENSOR_VIDEO_WIDTH      (1024)
#define HI351_IMAGE_SENSOR_VIDEO_HEIGHT     (768)


/* SENSOR FULL SIZE & START GRAB PIXEL OFFSET */
#define IMAGE_SENSOR_FULL_GRAB_START_X      (0)
#define IMAGE_SENSOR_FULL_GRAB_START_Y      (0)//1
#define HI351_IMAGE_SENSOR_FULL_WIDTH       (2048-32)//-8
#define HI351_IMAGE_SENSOR_FULL_HEIGHT      (1536-30)//(1536 - 24)

#define BYTE_LEN                            (0x11)
#define BURST_TYPE_ED                       (0xEE)

#define BURST_TYPE                          (0x33)
//#define DELAY_TYPE                          (0xEE)

#define HI351_TEST_PATTERN_CHECKSUM			0x77404507

typedef struct
{
    UINT8 addr;
    UINT8 val;
    UINT8 type;     /* BYTE_LEN or DELAY_TYPE or BURST_TYPE */
} HI351_I2C_REG_STRUCT;

/* Export functions */
UINT32 HI351Open(void);
UINT32 HI351GetResolution(MSDK_SENSOR_RESOLUTION_INFO_STRUCT *pSensorResolution);
UINT32 HI351GetInfo(MSDK_SCENARIO_ID_ENUM ScenarioId, MSDK_SENSOR_INFO_STRUCT *pSensorInfo, MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData);
UINT32 HI351Control(MSDK_SCENARIO_ID_ENUM ScenarioId, MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *pImageWindow, MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData);
UINT32 HI351FeatureControl(MSDK_SENSOR_FEATURE_ENUM FeatureId, UINT8 *pFeaturePara,UINT32 *pFeatureParaLen);
UINT32 HI351Close(void);

#endif /* #ifndef __HI351_MIPI_YUV_SENSOR_H */

