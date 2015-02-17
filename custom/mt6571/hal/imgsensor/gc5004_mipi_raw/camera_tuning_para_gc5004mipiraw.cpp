#include <utils/Log.h>
#include <fcntl.h>
#include <math.h>

#include "camera_custom_nvram.h"
#include "camera_custom_sensor.h"
#include "image_sensor.h"
#include "kd_imgsensor_define.h"
#include "camera_AE_PLineTable_gc5004mipiraw.h"
#include "camera_info_gc5004mipiraw.h"
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
            1632,    // u4MinGain, 1024 base = 1x
            6144,    // u4MaxGain, 16x
            52,    // u4MiniISOGain, ISOxx  
            128,    // u4GainStepUnit, 1x/8 
            16,    // u4PreExpUnit 
            32,    // u4PreMaxFrameRate
            17,    // u4VideoExpUnit  
            32,    // u4VideoMaxFrameRate 
            1024,    // u4Video2PreRatio, 1024 base = 1x 
            41,    // u4CapExpUnit 
            12,    // u4CapMaxFrameRate
            1024,    // u4Cap2PreRatio, 1024 base = 1x
            28,    // u4LensFno, Fno = 2.8
            350    // u4FocusLength_100x
        },
        // rHistConfig
        {
            2,    // u4HistHighThres
            40,    // u4HistLowThres
            2,    // u4MostBrightRatio
            1,    // u4MostDarkRatio
            160,    // u4CentralHighBound
            20,    // u4CentralLowBound
            {240, 230, 220, 210, 200},    // u4OverExpThres[AE_CCT_STRENGTH_NUM] 
            {86, 108, 128, 148, 170},    // u4HistStretchThres[AE_CCT_STRENGTH_NUM] 
            {18, 22, 26, 30, 34}    // u4BlackLightThres[AE_CCT_STRENGTH_NUM] 
        },
        // rCCTConfig
        {
            TRUE,    // bEnableBlackLight
            TRUE,    // bEnableHistStretch
            FALSE,    // bEnableAntiOverExposure
            TRUE,    // bEnableTimeLPF
            FALSE,    // bEnableCaptureThres
            FALSE,    // bEnableVideoThres
            FALSE,    // bEnableStrobeThres
            50,    // u4AETarget
            0,    // u4StrobeAETarget
            50,    // u4InitIndex
            4,    // u4BackLightWeight
            32,    // u4HistStretchWeight
            4,    // u4AntiOverExpWeight
            2,    // u4BlackLightStrengthIndex
            2,    // u4HistStretchStrengthIndex
            2,    // u4AntiOverExpStrengthIndex
            2,    // u4TimeLPFStrengthIndex
            {1, 3, 5, 7, 8},    // u4LPFConvergeTable[AE_CCT_STRENGTH_NUM] 
            90,    // u4InDoorEV = 9.0, 10 base 
            -3,    // i4BVOffset delta BV = value/10 
            64,    // u4PreviewFlareOffset
            64,    // u4CaptureFlareOffset
            5,    // u4CaptureFlareThres
            64,    // u4VideoFlareOffset
            5,    // u4VideoFlareThres
            64,    // u4StrobeFlareOffset
            5,    // u4StrobeFlareThres
            50,    // u4PrvMaxFlareThres
            0,    // u4PrvMinFlareThres
            50,    // u4VideoMaxFlareThres
            0,    // u4VideoMinFlareThres
            18,    // u4FlatnessThres    // 10 base for flatness condition.
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
                778,    // i4R
                512,    // i4G
                541    // i4B
            }
        },
        // Original XY coordinate of AWB light source
        {
           // Strobe
            {
                134,    // i4X
                -175    // i4Y
            },
            // Horizon
            {
                -426,    // i4X
                -241    // i4Y
            },
            // A
            {
                -314,    // i4X
                -244    // i4Y
            },
            // TL84
            {
                -154,    // i4X
                -269    // i4Y
            },
            // CWF
            {
                -109,    // i4X
                -373    // i4Y
            },
            // DNP
            {
                -31,    // i4X
                -215    // i4Y
            },
            // D65
            {
                134,    // i4X
                -175    // i4Y
            },
            // DF
            {
                92,    // i4X
                -310    // i4Y
            }
        },
        // Rotated XY coordinate of AWB light source
        {
            // Strobe
            {
                105,    // i4X
                -194    // i4Y
            },
            // Horizon
            {
                -459,    // i4X
                -172    // i4Y
            },
            // A
            {
                -348,    // i4X
                -192    // i4Y
            },
            // TL84
            {
                -194,    // i4X
                -242    // i4Y
            },
            // CWF
            {
                -166,    // i4X
                -352    // i4Y
            },
            // DNP
            {
                -64,    // i4X
                -208    // i4Y
            },
            // D65
            {
                105,    // i4X
                -194    // i4Y
            },
            // DF
            {
                42,    // i4X
                -321    // i4Y
            }
        },
        // AWB gain of AWB light source
        {
            // Strobe 
            {
                778,    // i4R
                512,    // i4G
                541    // i4B
            },
            // Horizon 
            {
                512,    // i4R
                658,    // i4G
                1624    // i4B
            },
            // A 
            {
                512,    // i4R
                562,    // i4G
                1197    // i4B
            },
            // TL84 
            {
                598,    // i4R
                512,    // i4G
                909    // i4B
            },
            // CWF 
            {
                731,    // i4R
                512,    // i4G
                983    // i4B
            },
            // DNP 
            {
                657,    // i4R
                512,    // i4G
                715    // i4B
            },
            // D65 
            {
                778,    // i4R
                512,    // i4G
                541    // i4B
            },
            // DF 
            {
                883,    // i4R
                512,    // i4G
                688    // i4B
            }
        },
        // Rotation matrix parameter
        {
            9,    // i4RotationAngle
            253,    // i4Cos
            40    // i4Sin
        },
        // Daylight locus parameter
        {
            -175,    // i4SlopeNumerator
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
            -244,    // i4RightBound
            -894,    // i4LeftBound
            -132,    // i4UpperBound
            -232    // i4LowerBound
            },
            // Warm fluorescent
            {
            -244,    // i4RightBound
            -894,    // i4LeftBound
            -232,    // i4UpperBound
            -352    // i4LowerBound
            },
            // Fluorescent
            {
            -114,    // i4RightBound
            -244,    // i4LeftBound
            -123,    // i4UpperBound
            -297    // i4LowerBound
            },
            // CWF
            {
            -114,    // i4RightBound
            -244,    // i4LeftBound
            -297,    // i4UpperBound
            -402    // i4LowerBound
            },
            // Daylight
            {
            130,    // i4RightBound
            -114,    // i4LeftBound
            -114,    // i4UpperBound
            -274    // i4LowerBound
            },
            // Shade
            {
            490,    // i4RightBound
            130,    // i4LeftBound
            -114,    // i4UpperBound
            -274    // i4LowerBound
            },
            // Daylight Fluorescent
            {
            0,    // i4RightBound
            0,    // i4LeftBound
            0,    // i4UpperBound
            0    // i4LowerBound
            }
        },
        // PWB light area
        {
            // Reference area
            {
            490,    // i4RightBound
            -894,    // i4LeftBound
            -89,    // i4UpperBound
            -402    // i4LowerBound
            },
            // Daylight
            {
            155,    // i4RightBound
            -114,    // i4LeftBound
            -114,    // i4UpperBound
            -274    // i4LowerBound
            },
            // Cloudy daylight
            {
            255,    // i4RightBound
            80,    // i4LeftBound
            -114,    // i4UpperBound
            -274    // i4LowerBound
            },
            // Shade
            {
            355,    // i4RightBound
            80,    // i4LeftBound
            -114,    // i4UpperBound
            -274    // i4LowerBound
            },
            // Twilight
            {
            -114,    // i4RightBound
            -274,    // i4LeftBound
            -114,    // i4UpperBound
            -274    // i4LowerBound
            },
            // Fluorescent
            {
            155,    // i4RightBound
            -294,    // i4LeftBound
            -144,    // i4UpperBound
            -402    // i4LowerBound
            },
            // Warm fluorescent
            {
            -248,    // i4RightBound
            -448,    // i4LeftBound
            -144,    // i4UpperBound
            -402    // i4LowerBound
            },
            // Incandescent
            {
            -248,    // i4RightBound
            -448,    // i4LeftBound
            -114,    // i4UpperBound
            -274    // i4LowerBound
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
            707,    // i4R
            512,    // i4G
            617    // i4B
            },
            // Cloudy daylight
            {
            835,    // i4R
            512,    // i4G
            491    // i4B
            },
            // Shade
            {
            883,    // i4R
            512,    // i4G
            455    // i4B
            },
            // Twilight
            {
            556,    // i4R
            512,    // i4G
            860    // i4B
            },
            // Fluorescent
            {
            722,    // i4R
            512,    // i4G
            775    // i4B
            },
            // Warm fluorescent
            {
            528,    // i4R
            512,    // i4G
            1193    // i4B
            },
            // Incandescent
            {
            467,    // i4R
            512,    // i4G
            1091    // i4B
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
            7205    // i4OffsetThr
            },
            // Warm fluorescent	
            {
            0,    // i4SliderValue
            5814    // i4OffsetThr
            },
            // Shade
            {
            0,    // i4SliderValue
            1345    // i4OffsetThr
            },
            // Daylight WB gain
            {
            643,    // i4R
            512,    // i4G
            703    // i4B
            },
            // Preference gain: strobe
            {
            512,    // i4R
            512,    // i4G
            512    // i4B
            },
            // Preference gain: tungsten
            {
            500,    // i4R
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
                -564,    // i4RotatedXCoordinate[0]
                -453,    // i4RotatedXCoordinate[1]
                -299,    // i4RotatedXCoordinate[2]
                -169,    // i4RotatedXCoordinate[3]
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


