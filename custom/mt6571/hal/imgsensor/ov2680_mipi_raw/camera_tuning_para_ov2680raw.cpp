#include <utils/Log.h>
#include <fcntl.h>
#include <math.h>

#include "camera_custom_nvram.h"
#include "camera_custom_sensor.h"
#include "image_sensor.h"
#include "kd_imgsensor_define.h"
#include "camera_AE_PLineTable_ov2680raw.h"
#include "camera_info_ov2680raw.h"
#include "camera_custom_AEPlinetable.h"
#include "flash_awb_param.h"
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
        69125,    // i4R_AVG
        11374,    // i4R_STD
        93350,    // i4B_AVG
        13030,    // i4B_STD
        {  // i4P00[9]
            5210000, -2315000, -340000, -920000, 4062500, -582500, -797500, -3665000, 7027500
        },
        {  // i4P10[9]
            1389042, -2438270, 1025172, 17929, -967762, 949833, 470136, -142222, -324411
        },
        {  // i4P01[9]
            810263, -1246465, 421005, -298422, -434791, 733213, -145430, -613874, 766428
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
            1168,    // u4MinGain, 1024 base = 1x
            10240,    // u4MaxGain, 16x
            73,    // u4MiniISOGain, ISOxx  
            128,    // u4GainStepUnit, 1x/8 
            26,    // u4PreExpUnit 
            30,    // u4PreMaxFrameRate
            26,    // u4VideoExpUnit  
            30,    // u4VideoMaxFrameRate 
            1024,    // u4Video2PreRatio, 1024 base = 1x 
            26,    // u4CapExpUnit 
            30,    // u4CapMaxFrameRate
            1024,    // u4Cap2PreRatio, 1024 base = 1x
            24,    // u4LensFno, Fno = 2.8
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
            TRUE,    // bEnableCaptureThres
            TRUE,    // bEnableVideoThres
            TRUE,    // bEnableStrobeThres
            47,    // u4AETarget
            47,    // u4StrobeAETarget
            20,    // u4InitIndex
            4,    // u4BackLightWeight
            32,    // u4HistStretchWeight
            4,    // u4AntiOverExpWeight
            2,    // u4BlackLightStrengthIndex
            2,    // u4HistStretchStrengthIndex
            2,    // u4AntiOverExpStrengthIndex
            2,    // u4TimeLPFStrengthIndex
            {1, 3, 5, 7, 8},    // u4LPFConvergeTable[AE_CCT_STRENGTH_NUM] 
            90,    // u4InDoorEV = 9.0, 10 base 
            0,    // i4BVOffset delta BV = value/10 
            64,    // u4PreviewFlareOffset
            64,    // u4CaptureFlareOffset
            5,    // u4CaptureFlareThres
            64,    // u4VideoFlareOffset
            5,    // u4VideoFlareThres
            32,    // u4StrobeFlareOffset
            2,    // u4StrobeFlareThres
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
                847,    // i4R
                512,    // i4G
                750    // i4B
            }
        },
        // Original XY coordinate of AWB light source
        {
           // Strobe
            {
                11,    // i4X
                -387    // i4Y
            },
            // Horizon
            {
                -310,    // i4X
                -249    // i4Y
            },
            // A
            {
                -241,    // i4X
                -288    // i4Y
            },
            // TL84
            {
                -165,    // i4X
                -321    // i4Y
            },
            // CWF
            {
                -134,    // i4X
                -388    // i4Y
            },
            // DNP
            {
                -53,    // i4X
                -330    // i4Y
            },
            // D65
            {
                45,    // i4X
                -327    // i4Y
            },
            // DF
            {
                11,    // i4X
                -387    // i4Y
            }
        },
        // Rotated XY coordinate of AWB light source
        {
            // Strobe
            {
                11,    // i4X
                -387    // i4Y
            },
            // Horizon
            {
                -310,    // i4X
                -249    // i4Y
            },
            // A
            {
                -241,    // i4X
                -288    // i4Y
            },
            // TL84
            {
                -165,    // i4X
                -321    // i4Y
            },
            // CWF
            {
                -134,    // i4X
                -388    // i4Y
            },
            // DNP
            {
                -53,    // i4X
                -330    // i4Y
            },
            // D65
            {
                45,    // i4X
                -327    // i4Y
            },
            // DF
            {
                11,    // i4X
                -387    // i4Y
            }
        },
        // AWB gain of AWB light source
        {
            // Strobe 
            {
                877,    // i4R
                512,    // i4G
                852    // i4B
            },
            // Horizon 
            {
                512,    // i4R
                556,    // i4G
                1185    // i4B
            },
            // A 
            {
                546,    // i4R
                512,    // i4G
                1047    // i4B
            },
            // TL84 
            {
                632,    // i4R
                512,    // i4G
                989    // i4B
            },
            // CWF 
            {
                723,    // i4R
                512,    // i4G
                1038    // i4B
            },
            // DNP 
            {
                745,    // i4R
                512,    // i4G
                860    // i4B
            },
            // D65 
            {
                847,    // i4R
                512,    // i4G
                750    // i4B
            },
            // DF 
            {
                877,    // i4R
                512,    // i4G
                852    // i4B
            }
        },
        // Rotation matrix parameter
        {
            0,    // i4RotationAngle
            256,    // i4Cos
            0    // i4Sin
        },
        // Daylight locus parameter
        {
            -92,    // i4SlopeNumerator
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
            -203,    // i4RightBound
            -853,    // i4LeftBound
            -218,    // i4UpperBound
            -318    // i4LowerBound
            },
            // Warm fluorescent
            {
            -203,    // i4RightBound
            -853,    // i4LeftBound
            -318,    // i4UpperBound
            -438    // i4LowerBound
            },
            // Fluorescent
            {
            -103,    // i4RightBound
            -203,    // i4LeftBound
            -232,    // i4UpperBound
            -354    // i4LowerBound
            },
            // CWF
            {
            -103,    // i4RightBound
            -203,    // i4LeftBound
            -354,    // i4UpperBound
            -438    // i4LowerBound
            },
            // Daylight
            {
            70,    // i4RightBound
            -103,    // i4LeftBound
            -247,    // i4UpperBound
            -407    // i4LowerBound
            },
            // Shade
            {
            430,    // i4RightBound
            70,    // i4LeftBound
            -247,    // i4UpperBound
            -407    // i4LowerBound
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
            430,    // i4RightBound
            -853,    // i4LeftBound
            -193,    // i4UpperBound
            -438    // i4LowerBound
            },
            // Daylight
            {
            95,    // i4RightBound
            -103,    // i4LeftBound
            -247,    // i4UpperBound
            -407    // i4LowerBound
            },
            // Cloudy daylight
            {
            195,    // i4RightBound
            20,    // i4LeftBound
            -247,    // i4UpperBound
            -407    // i4LowerBound
            },
            // Shade
            {
            295,    // i4RightBound
            20,    // i4LeftBound
            -247,    // i4UpperBound
            -407    // i4LowerBound
            },
            // Twilight
            {
            -103,    // i4RightBound
            -263,    // i4LeftBound
            -247,    // i4UpperBound
            -407    // i4LowerBound
            },
            // Fluorescent
            {
            95,    // i4RightBound
            -265,    // i4LeftBound
            -271,    // i4UpperBound
            -438    // i4LowerBound
            },
            // Warm fluorescent
            {
            -141,    // i4RightBound
            -341,    // i4LeftBound
            -271,    // i4UpperBound
            -438    // i4LowerBound
            },
            // Incandescent
            {
            -141,    // i4RightBound
            -341,    // i4LeftBound
            -247,    // i4UpperBound
            -407    // i4LowerBound
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
            793,    // i4R
            512,    // i4G
            801    // i4B
            },
            // Cloudy daylight
            {
            922,    // i4R
            512,    // i4G
            689    // i4B
            },
            // Shade
            {
            987,    // i4R
            512,    // i4G
            644    // i4B
            },
            // Twilight
            {
            622,    // i4R
            512,    // i4G
            1021    // i4B
            },
            // Fluorescent
            {
            737,    // i4R
            512,    // i4G
            928    // i4B
            },
            // Warm fluorescent
            {
            597,    // i4R
            512,    // i4G
            1147    // i4B
            },
            // Incandescent
            {
            575,    // i4R
            512,    // i4G
            1105    // i4B
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
            4295    // i4OffsetThr
            },
            // Warm fluorescent	
            {
            0,    // i4SliderValue
            4421    // i4OffsetThr
            },
            // Shade
            {
            0,    // i4SliderValue
            1345    // i4OffsetThr
            },
            // Daylight WB gain
            {
            742,    // i4R
            512,    // i4G
            856    // i4B
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
                -355,    // i4RotatedXCoordinate[0]
                -286,    // i4RotatedXCoordinate[1]
                -210,    // i4RotatedXCoordinate[2]
                -98,    // i4RotatedXCoordinate[3]
                0    // i4RotatedXCoordinate[4]
            }
        }
    },
    {0}
};

#include INCLUDE_FILENAME_ISP_LSC_PARAM
//};  //  namespace

const CAMERA_TSF_TBL_STRUCT CAMERA_TSF_DEFAULT_VALUE =
{
    #include INCLUDE_FILENAME_TSF_PARA
    #include INCLUDE_FILENAME_TSF_DATA
};


const FLASH_AWB_CALIBRATION_DATA_STRUCT CAMERA_FLASH_AWB_CALIBRATION_DATA =
{
    #include INCLUDE_FILENAME_FLASH_AWB_CALIBRATION_DATA
};


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
                                             sizeof(AE_PLINETABLE_T),
                                             0,
                                             sizeof(CAMERA_TSF_TBL_STRUCT),
                                             sizeof(FLASH_AWB_CALIBRATION_DATA_STRUCT)};

    if (CameraDataType > CAMERA_DATA_FLASH_AWB_CALIBRATION_DATA || NULL == pDataBuf || (size < dataSize[CameraDataType]))
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
        case CAMERA_DATA_TSF_TABLE:
            memcpy(pDataBuf,&CAMERA_TSF_DEFAULT_VALUE,sizeof(CAMERA_TSF_TBL_STRUCT));
            break;
        case CAMERA_DATA_FLASH_AWB_CALIBRATION_DATA:
            memcpy(pDataBuf,&CAMERA_FLASH_AWB_CALIBRATION_DATA,sizeof(FLASH_AWB_CALIBRATION_DATA_STRUCT));
            break;
        default:
            return 1;
    }
    return 0;
}}; // NSFeature


