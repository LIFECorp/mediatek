#include <utils/Log.h>
#include <fcntl.h>
#include <math.h>

#include "camera_custom_nvram.h"
#include "camera_custom_sensor.h"
#include "image_sensor.h"
#include "kd_imgsensor_define.h"
#include "camera_AE_PLineTable_a2030raw.h"
#include "camera_info_a2030raw.h"
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
        68200,    // i4R_AVG
        13198,    // i4R_STD
        120875,    // i4B_AVG
        35419,    // i4B_STD
        {  // i4P00[9]
            4570000, -1690000, -317500, -612500, 3032500, 137500, 157500, -1515000, 3910000
        },
        {  // i4P10[9]
            924320, -935191, 4517, -171563, 159053, 16292, -110079, 471296, -352080
        },
        {  // i4P01[9]
            531494, -592326, 53607, -204947, -15883, 221024, -101145, -76291, 183924
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
            1352,    // u4MinGain, 1024 base = 1x
            10240,    // u4MaxGain, 16x
            58,    // u4MiniISOGain, ISOxx  
            128,    // u4GainStepUnit, 1x/8
            24759,    // u4PreExpUnit 
            30,     // u4PreMaxFrameRate
            24759,    // u4VideoExpUnit  
            30,     // u4VideoMaxFrameRate
            1024,   // u4Video2PreRatio, 1024 base = 1x
            24759,    // u4CapExpUnit 
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
            -2,    // i4BVOffset delta BV = value/10 
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
                838,    // i4R
                512,    // i4G
                698    // i4B
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
                -508,    // i4X
                -445    // i4Y
            },
            // A
            {
                -374,    // i4X
                -427    // i4Y
            },
            // TL84
            {
                -257,    // i4X
                -398    // i4Y
            },
            // CWF
            {
                -191,    // i4X
                -469    // i4Y
            },
            // DNP
            {
                -104,    // i4X
                -363    // i4Y
            },
            // D65
            {
                67,    // i4X
                -297    // i4Y
            },
            // DF
            {
                7,    // i4X
                -402    // i4Y
            }
        },
        // Rotated XY coordinate of AWB light source
        {
            // Strobe
            {
                0,    // i4X
                0    // i4Y
            },
            // Horizon
            {
                -612,    // i4X
                -287    // i4Y
            },
            // A
            {
                -478,    // i4X
                -307    // i4Y
            },
            // TL84
            {
                -357,    // i4X
                -311    // i4Y
            },
            // CWF
            {
                -314,    // i4X
                -398    // i4Y
            },
            // DNP
            {
                -201,    // i4X
                -320    // i4Y
            },
            // D65
            {
                -18,    // i4X
                -304    // i4Y
            },
            // DF
            {
                -105,    // i4X
                -388    // i4Y
            }
        },
        // AWB gain of AWB light source
        {
            // Strobe 
            {
                512,    // i4R
                512,    // i4G
                512    // i4B
            },
            // Horizon 
            {
                512,    // i4R
                558,    // i4G
                2025    // i4B
            },
            // A 
            {
                550,    // i4R
                512,    // i4G
                1515    // i4B
            },
            // TL84 
            {
                619,    // i4R
                512,    // i4G
                1243    // i4B
            },
            // CWF 
            {
                745,    // i4R
                512,    // i4G
                1251    // i4B
            },
            // DNP 
            {
                727,    // i4R
                512,    // i4G
                964    // i4B
            },
            // D65 
            {
                838,    // i4R
                512,    // i4G
                698    // i4B
            },
            // DF 
            {
                890,    // i4R
                512,    // i4G
                874    // i4B
            }
        },
        // Rotation matrix parameter
        {
            16,    // i4RotationAngle
            246,    // i4Cos
            71    // i4Sin
        },
        // Daylight locus parameter
        {
            -226,    // i4SlopeNumerator
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
            -407,    // i4RightBound
            -1057,    // i4LeftBound
            -247,    // i4UpperBound
            -347    // i4LowerBound
            },
            // Warm fluorescent
            {
            -407,    // i4RightBound
            -1057,    // i4LeftBound
            -347,    // i4UpperBound
            -467    // i4LowerBound
            },
            // Fluorescent
            {
            -251,    // i4RightBound
            -407,    // i4LeftBound
            -235,    // i4UpperBound
            -354    // i4LowerBound
            },
            // CWF
            {
            -251,    // i4RightBound
            -407,    // i4LeftBound
            -354,    // i4UpperBound
            -448    // i4LowerBound
            },
            // Daylight
            {
            7,    // i4RightBound
            -251,    // i4LeftBound
            -224,    // i4UpperBound
            -384    // i4LowerBound
            },
            // Shade
            {
            327,    // i4RightBound
            7,    // i4LeftBound
            -224,    // i4UpperBound
            -384    // i4LowerBound
            },
            // Daylight Fluorescent
            {
            7,    // i4RightBound
            -251,    // i4LeftBound
            -384,    // i4UpperBound
            -514    // i4LowerBound
            }
        },
        // PWB light area
        {
            // Reference area
            {
            327,    // i4RightBound
            -1057,    // i4LeftBound
            0,    // i4UpperBound
            -514    // i4LowerBound
            },
            // Daylight
            {
            32,    // i4RightBound
            -251,    // i4LeftBound
            -224,    // i4UpperBound
            -384    // i4LowerBound
            },
            // Cloudy daylight
            {
            132,    // i4RightBound
            -43,    // i4LeftBound
            -224,    // i4UpperBound
            -384    // i4LowerBound
            },
            // Shade
            {
            232,    // i4RightBound
            -43,    // i4LeftBound
            -224,    // i4UpperBound
            -384    // i4LowerBound
            },
            // Twilight
            {
            -251,    // i4RightBound
            -411,    // i4LeftBound
            -224,    // i4UpperBound
            -384    // i4LowerBound
            },
            // Fluorescent
            {
            32,    // i4RightBound
            -457,    // i4LeftBound
            -254,    // i4UpperBound
            -448    // i4LowerBound
            },
            // Warm fluorescent
            {
            -378,    // i4RightBound
            -578,    // i4LeftBound
            -254,    // i4UpperBound
            -448    // i4LowerBound
            },
            // Incandescent
            {
            -378,    // i4RightBound
            -578,    // i4LeftBound
            -224,    // i4UpperBound
            -384    // i4LowerBound
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
            770,    // i4R
            512,    // i4G
            815    // i4B
            },
            // Cloudy daylight
            {
            888,    // i4R
            512,    // i4G
            630    // i4B
            },
            // Shade
            {
            930,    // i4R
            512,    // i4G
            579    // i4B
            },
            // Twilight
            {
            627,    // i4R
            512,    // i4G
            1181    // i4B
            },
            // Fluorescent
            {
            757,    // i4R
            512,    // i4G
            1011    // i4B
            },
            // Warm fluorescent
            {
            592,    // i4R
            512,    // i4G
            1578    // i4B
            },
            // Incandescent
            {
            548,    // i4R
            512,    // i4G
            1511    // i4B
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
            7174    // i4OffsetThr
            },
            // Warm fluorescent	
            {
            0,    // i4SliderValue
            6500    // i4OffsetThr
            },
            // Shade
            {
            50,    // i4SliderValue
            353    // i4OffsetThr
            },
            // Daylight WB gain
            {
            708,    // i4R
            512,    // i4G
            950    // i4B
            },
            // Preference gain: strobe
            {
            522,    // i4R
            512,    // i4G
            492    // i4B
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
            522,    // i4R
            512,    // i4G
            492    // i4B
            },
            // Preference gain: shade
            {
            522,    // i4R
            512,    // i4G
            492    // i4B
            },
            // Preference gain: daylight fluorescent
            {
            522,    // i4R
            512,    // i4G
            492    // i4B
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
                -594,    // i4RotatedXCoordinate[0]
                -460,    // i4RotatedXCoordinate[1]
                -339,    // i4RotatedXCoordinate[2]
                -183,    // i4RotatedXCoordinate[3]
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


