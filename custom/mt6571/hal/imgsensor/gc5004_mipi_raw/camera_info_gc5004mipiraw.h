#ifndef _CAMERA_INFO_GC5004MIPIRAW_H
#define _CAMERA_INFO_GC5004MIPIRAW_H

/*******************************************************************************
*   Configuration
********************************************************************************/
#define SENSOR_ID                           GC5004_SENSOR_ID
#define SENSOR_DRVNAME                      SENSOR_DRVNAME_GC5004_MIPI_RAW
#define INCLUDE_FILENAME_ISP_REGS_PARAM     "camera_isp_regs_gc5004mipiraw.h"
#define INCLUDE_FILENAME_ISP_PCA_PARAM      "camera_isp_pca_gc5004mipiraw.h"
#define INCLUDE_FILENAME_ISP_LSC_PARAM      "camera_isp_lsc_gc5004mipiraw.h"
#define INCLUDE_FILENAME_FLASH_AWB_CALIBRATION_DATA "camera_flash_awb_calibration_data_gc5004mipiraw.h"

/*******************************************************************************
*
********************************************************************************/

#if defined(ISP_SUPPORT)

#define GC5004MIPIRAW_CAMERA_AUTO_DSC CAM_AUTO_DSC
#define GC5004MIPIRAW_CAMERA_PORTRAIT CAM_PORTRAIT
#define GC5004MIPIRAW_CAMERA_LANDSCAPE CAM_LANDSCAPE
#define GC5004MIPIRAW_CAMERA_SPORT CAM_SPORT
#define GC5004MIPIRAW_CAMERA_FLOWER CAM_FLOWER
#define GC5004MIPIRAW_CAMERA_NIGHTSCENE CAM_NIGHTSCENE
#define GC5004MIPIRAW_CAMERA_DOCUMENT CAM_DOCUMENT
#define GC5004MIPIRAW_CAMERA_ISO_ANTI_HAND_SHAKE CAM_ISO_ANTI_HAND_SHAKE
#define GC5004MIPIRAW_CAMERA_ISO100 CAM_ISO100
#define GC5004MIPIRAW_CAMERA_ISO200 CAM_ISO200
#define GC5004MIPIRAW_CAMERA_ISO400 CAM_ISO400
#define GC5004MIPIRAW_CAMERA_ISO800 CAM_ISO800
#define GC5004MIPIRAW_CAMERA_ISO1600 CAM_ISO1600
#define GC5004MIPIRAW_CAMERA_VIDEO_AUTO CAM_VIDEO_AUTO
#define GC5004MIPIRAW_CAMERA_VIDEO_NIGHT CAM_VIDEO_NIGHT
#define GC5004MIPIRAW_CAMERA_NO_OF_SCENE_MODE CAM_NO_OF_SCENE_MODE

#endif
#endif
