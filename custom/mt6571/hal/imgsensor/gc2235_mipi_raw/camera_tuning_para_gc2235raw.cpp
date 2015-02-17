#include <utils/Log.h>
#include <fcntl.h>
#include <math.h>

#include "camera_custom_nvram.h"
#include "camera_custom_sensor.h"
#include "image_sensor.h"
#include "kd_imgsensor_define.h"
#include "camera_AE_PLineTable_gc2235raw.h"
#include "camera_info_gc2235raw.h"
#include "camera_custom_AEPlinetable.h"
const NVRAM_CAMERA_ISP_PARAM_STRUCT CAMERA_ISP_DEFAULT_VALUE =
{{
    //Version
    Version: NVRAM_CAMERA_PARA_FILE_VERSION,
    //SensorId
    SensorId: SENSOR_ID,
    ISPComm:{
        {
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
        }
    },
    ISPPca:{
        #include INCLUDE_FILENAME_ISP_PCA_PARAM
    },
    ISPRegs:{
        #include INCLUDE_FILENAME_ISP_REGS_PARAM
        },
    ISPMfbMixer:{{
        {//00: MFB mixer for ISO 100
            0x00000000, 0x00000000
        },
        {//01: MFB mixer for ISO 200
            0x00000000, 0x00000000
        },
        {//02: MFB mixer for ISO 400
            0x00000000, 0x00000000
        },
        {//03: MFB mixer for ISO 800
            0x00000000, 0x00000000
        },
        {//04: MFB mixer for ISO 1600
            0x00000000, 0x00000000
        },
        {//05: MFB mixer for ISO 2400
            0x00000000, 0x00000000
        },
        {//06: MFB mixer for ISO 3200
            0x00000000, 0x00000000
        }
    }},
    ISPCcmPoly22:{
        70300,    // i4R_AVG
        8906,    // i4R_STD
        92267,    // i4B_AVG
        24788,    // i4B_STD
        {  // i4P00[9]
            5383333, -2373333, -453333, -790000, 3470000, -120000, 120000, -1833333, 4270000
        },
        {  // i4P10[9]
            639683, -691536, 50691, 25718, -78211, 52494, 39217, -43419, 3040
        },
        {  // i4P01[9]
            534236, -502884, -26376, -173683, -14681, 188364, -40808, -131880, 177664
        },
        {  // i4P20[9]
            0, 0, 0, 0, 0, 0, 0, 0, 0
        },
        {  // i4P11[9]
            0, 0, 0, 0, 0, 0, 0, 0, 0
        },
        {  // i4P02[9]
            0, 0, 0, 0, 0, 0, 0, 0, 0
        }
    }
}};

const NVRAM_CAMERA_3A_STRUCT CAMERA_3A_NVRAM_DEFAULT_VALUE =
{
    NVRAM_CAMERA_3A_FILE_VERSION, // u4Version
    SENSOR_ID, // SensorId

    // AE NVRAM
    {
        // rDevicesInfo
        {
            1024,   // u4MinGain, 1024 base =  1x
            6144,  // u4MaxGain, 16x
            100,     // u4MiniISOGain, ISOxx
            128,    // u4GainStepUnit, 1x/8
            43000,     // u4PreExpUnit
            30,     // u4PreMaxFrameRate
            43000,     // u4VideoExpUnit
            30,     // u4VideoMaxFrameRate
            1024,   // u4Video2PreRatio, 1024 base = 1x
            43000,     // u4CapExpUnit
            30,     // u4CapMaxFrameRate
            1024,   // u4Cap2PreRatio, 1024 base = 1x
            28,      // u4LensFno, Fno = 2.8
            350     // u4FocusLength_100x
         },
         // rHistConfig
        {
            2,   // u4HistHighThres
            40,  // u4HistLowThres
            2,   // u4MostBrightRatio
            1,   // u4MostDarkRatio
            160, // u4CentralHighBound
            20,  // u4CentralLowBound
            {240, 230, 220, 210, 200}, // u4OverExpThres[AE_CCT_STRENGTH_NUM]
            {86, 108, 128, 148, 170},  // u4HistStretchThres[AE_CCT_STRENGTH_NUM]
            {18, 22, 26, 30, 34}       // u4BlackLightThres[AE_CCT_STRENGTH_NUM]
        },
        // rCCTConfig
        {
            TRUE,            // bEnableBlackLight
            TRUE,            // bEnableHistStretch
            FALSE,           // bEnableAntiOverExposure
            TRUE,            // bEnableTimeLPF
            TRUE,            // bEnableCaptureThres
            TRUE,            // bEnableVideoThres
            TRUE,            // bEnableStrobeThres
            47,                // u4AETarget
            47,                // u4StrobeAETarget

            20,                // u4InitIndex
            4,                 // u4BackLightWeight
            32,                // u4HistStretchWeight
            4,                 // u4AntiOverExpWeight
            2,                 // u4BlackLightStrengthIndex
            2,                 // u4HistStretchStrengthIndex
            2,                 // u4AntiOverExpStrengthIndex
            2,                 // u4TimeLPFStrengthIndex
            {1, 3, 5, 7, 8}, // u4LPFConvergeTable[AE_CCT_STRENGTH_NUM]
            90,                // u4InDoorEV = 9.0, 10 base
            -10,               // i4BVOffset delta BV = -2.3
            64,                 // u4PreviewFlareOffset
            64,                 // u4CaptureFlareOffset
            5,                 // u4CaptureFlareThres
            64,                 // u4VideoFlareOffset
            5,                 // u4VideoFlareThres
            32,                 // u4StrobeFlareOffset
            2,                 // u4StrobeFlareThres
            50,                 // u4PrvMaxFlareThres
            0,                 // u4PrvMinFlareThres
            50,                 // u4VideoMaxFlareThres
            0,                 // u4VideoMinFlareThres            
            18,                // u4FlatnessThres              // 10 base for flatness condition.
            75    // u4FlatnessStrength
        }
    },
    // AWB NVRAM
    {
        // AWB calibration data
        {
            // rUnitGain (unit gain: 1.0 = 512)
            {
                0,    // i4R
                0,    // i4G
                0    // i4B
            },
            // rGoldenGain (golden sample gain: 1.0 = 512)
            {
                0,    // i4R
                0,    // i4G
                0    // i4B
            },
            // rTuningUnitGain (Tuning sample unit gain: 1.0 = 512)
            {
                0,    // i4R
                0,    // i4G
                0    // i4B
            },
            // rD65Gain (D65 WB gain: 1.0 = 512)
            {
                731,    // i4R
                512,    // i4G
                550    // i4B
            }
        },
        // Original XY coordinate of AWB light source
        {
           // Strobe
            {
                0,    // i4X
                0    // i4Y
            },
            // Horizon
            {
                -515,    // i4X
                -263    // i4Y
            },
            // A
            {
                -373,    // i4X
                -254    // i4Y
            },
            // TL84
            {
                -176,    // i4X
                -277    // i4Y
            },
            // CWF
            {
                -121,    // i4X
                -376    // i4Y
            },
            // DNP
            {
                -41,    // i4X
                -210    // i4Y
            },
            // D65
            {
                105,    // i4X
                -158    // i4Y
            },
            // DF
            {
                -41,    // i4X
                -310    // i4Y
            }
        },
        // Rotated XY coordinate of AWB light source
        {
            // Strobe
            {
                -84,    // i4X
                -197    // i4Y
            },
            // Horizon
            {
                -557,    // i4X
                -150    // i4Y
            },
            // A
            {
                -417,    // i4X
                -171    // i4Y
            },
            // TL84
            {
                -229,    // i4X
                -234    // i4Y
            },
            // CWF
            {
                -196,    // i4X
                -342    // i4Y
            },
            // DNP
            {
                -84,    // i4X
                -197    // i4Y
            },
            // D65
            {
                70,    // i4X
                -176    // i4Y
            },
            // DF
            {
                70,    // i4X
                -276    // i4Y
            }
        },
        // AWB gain of AWB light source
        {
            // Strobe 
            {
                643,    // i4R
                512,    // i4G
                719    // i4B
            },
            // Horizon 
            {
                512,    // i4R
                720,    // i4G
                2063    // i4B
            },
            // A 
            {
                512,    // i4R
                602,    // i4G
                1406    // i4B
            },
            // TL84 
            {
                587,    // i4R
                512,    // i4G
                946    // i4B
            },
            // CWF 
            {
                723,    // i4R
                512,    // i4G
                1003    // i4B
            },
            // DNP 
            {
                643,    // i4R
                512,    // i4G
                719    // i4B
            },
            // D65 
            {
                731,    // i4R
                512,    // i4G
                550    // i4B
            },
            // DF 
            {
                512,    // i4R
                512,    // i4G
                512    // i4B
            }
        },
        // Rotation matrix parameter
        {
            12,    // i4RotationAngle
            250,    // i4Cos
            53    // i4Sin
        },
        // Daylight locus parameter
        {
            -192,    // i4SlopeNumerator
            128    // i4SlopeDenominator
        },
        // AWB light area
        {
            // Strobe:FIXME
            {
            0,    // i4RightBound
            0,    // i4LeftBound
            0,    // i4UpperBound
            0    // i4LowerBound
            },
            // Tungsten
            {
            -279,    // i4RightBound
            -929,    // i4LeftBound
            -110,    // i4UpperBound
            -210    // i4LowerBound
            },
            // Warm fluorescent
            {
            -279,    // i4RightBound
            -929,    // i4LeftBound
            -210,    // i4UpperBound
            -330    // i4LowerBound
            },
            // Fluorescent
            {
            -134,    // i4RightBound
            -279,    // i4LeftBound
            -103,    // i4UpperBound
            -288    // i4LowerBound
            },
            // CWF
            {
            -134,    // i4RightBound
            -279,    // i4LeftBound
            -288,    // i4UpperBound
            -392    // i4LowerBound
            },
            // Daylight
            {
            95,    // i4RightBound
            -134,    // i4LeftBound
            -96,    // i4UpperBound
            -256    // i4LowerBound
            },
            // Shade
            {
            455,    // i4RightBound
            95,    // i4LeftBound
            -96,    // i4UpperBound
            -256    // i4LowerBound
            },
            // Daylight Fluorescent
            {
            95,    // i4RightBound
            -134,    // i4LeftBound
            -256,    // i4UpperBound
            -456    // i4LowerBound
            }
        },
        // PWB light area
        {
            // Reference area
            {
            455,    // i4RightBound
            -929,    // i4LeftBound
            0,    // i4UpperBound
            -392    // i4LowerBound
            },
            // Daylight
            {
            120,    // i4RightBound
            -134,    // i4LeftBound
            -96,    // i4UpperBound
            -256    // i4LowerBound
            },
            // Cloudy daylight
            {
            220,    // i4RightBound
            45,    // i4LeftBound
            -96,    // i4UpperBound
            -256    // i4LowerBound
            },
            // Shade
            {
            320,    // i4RightBound
            45,    // i4LeftBound
            -96,    // i4UpperBound
            -256    // i4LowerBound
            },
            // Twilight
            {
            -134,    // i4RightBound
            -294,    // i4LeftBound
            -96,    // i4UpperBound
            -256    // i4LowerBound
            },
            // Fluorescent
            {
            120,    // i4RightBound
            -329,    // i4LeftBound
            -126,    // i4UpperBound
            -392    // i4LowerBound
            },
            // Warm fluorescent
            {
            -317,    // i4RightBound
            -517,    // i4LeftBound
            -126,    // i4UpperBound
            -392    // i4LowerBound
            },
            // Incandescent
            {
            -317,    // i4RightBound
            -517,    // i4LeftBound
            -96,    // i4UpperBound
            -256    // i4LowerBound
            },
            // Gray World
            {
            5000,    // i4RightBound
            -5000,    // i4LeftBound
            5000,    // i4UpperBound
            -5000    // i4LowerBound
            }
        },
        // PWB default gain	
        {
            // Daylight
            {
            675,    // i4R
            512,    // i4G
            622    // i4B
            },
            // Cloudy daylight
            {
            780,    // i4R
            512,    // i4G
            497    // i4B
            },
            // Shade
            {
            822,    // i4R
            512,    // i4G
            459    // i4B
            },
            // Twilight
            {
            543,    // i4R
            512,    // i4G
            868    // i4B
            },
            // Fluorescent
            {
            696,    // i4R
            512,    // i4G
            794    // i4B
            },
            // Warm fluorescent
            {
            502,    // i4R
            512,    // i4G
            1312    // i4B
            },
            // Incandescent
            {
            439,    // i4R
            512,    // i4G
            1203    // i4B
            },
            // Gray World
            {
            512,    // i4R
            512,    // i4G
            512    // i4B
            }
        },
        // AWB preference color	
        {
            // Tungsten
            {
            0,    // i4SliderValue
            8058    // i4OffsetThr
            },
            // Warm fluorescent	
            {
            0,    // i4SliderValue
            5857    // i4OffsetThr
            },
            // Shade
            {
            0,    // i4SliderValue
            1348    // i4OffsetThr
            },
            // Daylight WB gain
            {
            622,    // i4R
            512,    // i4G
            1081    // i4B
            },
            // Preference gain: strobe
            {
            512,    // i4R
            512,    // i4G
            512    // i4B
            },
            // Preference gain: tungsten
            {
            512,    // i4R
            512,    // i4G
            512    // i4B
            },
            // Preference gain: warm fluorescent
            {
            512,    // i4R
            512,    // i4G
            512    // i4B
            },
            // Preference gain: fluorescent
            {
            512,    // i4R
            512,    // i4G
            512    // i4B
            },
            // Preference gain: CWF
            {
            512,    // i4R
            512,    // i4G
            512    // i4B
            },
            // Preference gain: daylight
            {
            512,    // i4R
            512,    // i4G
            512    // i4B
            },
            // Preference gain: shade
            {
            512,    // i4R
            512,    // i4G
            512    // i4B
            },
            // Preference gain: daylight fluorescent
            {
            512,    // i4R
            512,    // i4G
            512    // i4B
            }
        },
        {// CCT estimation
            {// CCT
                2300,    // i4CCT[0]
                2850,    // i4CCT[1]
                4100,    // i4CCT[2]
                5100,    // i4CCT[3]
                6500    // i4CCT[4]
            },
            {// Rotated X coordinate
                -627,    // i4RotatedXCoordinate[0]
                -487,    // i4RotatedXCoordinate[1]
                -299,    // i4RotatedXCoordinate[2]
                -154,    // i4RotatedXCoordinate[3]
                0    // i4RotatedXCoordinate[4]
            }
        }
    },
    {0}
};

#include INCLUDE_FILENAME_ISP_LSC_PARAM
//};  //  namespace


typedef NSFeature::RAWSensorInfo<SENSOR_ID> SensorInfoSingleton_T;


namespace NSFeature {
template <>
UINT32
SensorInfoSingleton_T::
impGetDefaultData(CAMERA_DATA_TYPE_ENUM const CameraDataType, VOID*const pDataBuf, UINT32 const size) const
{
    UINT32 dataSize[CAMERA_DATA_TYPE_NUM] = {sizeof(NVRAM_CAMERA_ISP_PARAM_STRUCT),
                                             sizeof(NVRAM_CAMERA_3A_STRUCT),
                                             sizeof(NVRAM_CAMERA_SHADING_STRUCT),
                                             sizeof(NVRAM_LENS_PARA_STRUCT),
                                             sizeof(AE_PLINETABLE_T)};

    if (CameraDataType > CAMERA_DATA_AE_PLINETABLE || NULL == pDataBuf || (size < dataSize[CameraDataType]))
    {
        return 1;
    }

    switch(CameraDataType)
    {
        case CAMERA_NVRAM_DATA_ISP:
            memcpy(pDataBuf,&CAMERA_ISP_DEFAULT_VALUE,sizeof(NVRAM_CAMERA_ISP_PARAM_STRUCT));
            break;
        case CAMERA_NVRAM_DATA_3A:
            memcpy(pDataBuf,&CAMERA_3A_NVRAM_DEFAULT_VALUE,sizeof(NVRAM_CAMERA_3A_STRUCT));
            break;
        case CAMERA_NVRAM_DATA_SHADING:
            memcpy(pDataBuf,&CAMERA_SHADING_DEFAULT_VALUE,sizeof(NVRAM_CAMERA_SHADING_STRUCT));
            break;
        case CAMERA_DATA_AE_PLINETABLE:
            memcpy(pDataBuf,&g_PlineTableMapping,sizeof(AE_PLINETABLE_T));
            break;
        default:
            break;
    }
    return 0;
}}; // NSFeature


