#include <utils/Log.h>
#include <fcntl.h>
#include <math.h>

//#include "msdk_nvram_camera_exp.h"
#include "camera_custom_nvram.h"
//#include "msdk_sensor_exp.h"
#include "camera_custom_sensor.h"
#include "kd_imgsensor.h"
#include "kd_imgsensor_define.h"
//#include "image_sensor.h"
//TODO:remove once build system ready
//#include "camera_custom_cfg.h"

#define SENSOR_ID   S5K4ECGX_SENSOR_ID


typedef NSFeature::YUVSensorInfo<SENSOR_ID> SensorInfoSingleton_T;
namespace NSFeature {
template <>
UINT32
SensorInfoSingleton_T::
impGetDefaultData(CAMERA_DATA_TYPE_ENUM const CameraDataType, VOID*const pDataBuf, UINT32 const size) const
{
    return  NULL;
}};  //  NSFeature






//PFUNC_GETCAMERADEFAULT pNXSC301HS5K4EC_YUV_getDefaultData = NXSC301HS5K4EC_YUV_getDefaultData;