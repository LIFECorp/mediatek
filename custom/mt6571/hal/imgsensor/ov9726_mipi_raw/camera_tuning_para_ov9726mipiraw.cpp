#include <utils/Log.h>
#include <fcntl.h>
#include <math.h>

#include "camera_custom_nvram.h"
#include "camera_custom_sensor.h"
#include "image_sensor.h"
#include "kd_imgsensor_define.h"
#include "camera_AE_PLineTable_ov9726mipiraw.h"
#include "camera_info_ov9726mipiraw.h"
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
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    	}
    },
    ISPPca: {
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
        64358, // i4R_AVG
        14818, // i4R_STD
        95496, // i4B_AVG
        21107, // i4B_STD
        { // i4P00[9]
            5447500,  -2095000, -792500,   -840000,   4350000,   -950000,   -280000,  -2835000,  5675000 
        },
        { // i4P10[9]
            1754115,  -1753070,     -1045,   -192502,   -524999,    717500,    -87065,   -576843,   663905
        },
        { // i4P01[9]
            991728,  -1363091,    371362,   -252965,   -634940,    887905,   -177573,   -902745,  1080316
        },
        { // i4P20[9]
            0,0,0,0,0,0,0,0,0
        },
        { // i4P11[9]
            0,0,0,0,0,0,0,0,0
        },
        { // i4P02[9]
            0,0,0,0,0,0,0,0,0
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
            1152,    // u4MinGain, 1024 base = 1x
            15872,    // u4MaxGain, 16x
            82,    // u4MiniISOGain, ISOxx  
            64,    // u4GainStepUnit, 1x/8 
            39620,    // u4PreExpUnit 
            30,    // u4PreMaxFrameRate
            39620,    // u4VideoExpUnit  
            30,    // u4VideoMaxFrameRate 
            1024,    // u4Video2PreRatio, 1024 base = 1x 
            39620,    // u4CapExpUnit 
            30,    // u4CapMaxFrameRate
            1024,    // u4Cap2PreRatio, 1024 base = 1x
            24,    // u4LensFno, Fno = 2.8
            350    // u4FocusLength_100x
         // rHistConfig
	},
         // rHistConfig
        {
            4, // 2,   // u4HistHighThres
            40,  // u4HistLowThres
            2,   // u4MostBrightRatio
            1,   // u4MostDarkRatio
            160, // u4CentralHighBound
            20,  // u4CentralLowBound
            {240, 230, 220, 210, 200}, // u4OverExpThres[AE_CCT_STRENGTH_NUM]
            {82, 108, 128, 148, 170},  // u4HistStretchThres[AE_CCT_STRENGTH_NUM]
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

            50,                // u4InitIndex
            4,                 // u4BackLightWeight
            32,                // u4HistStretchWeight
            4,                 // u4AntiOverExpWeight
            2,                 // u4BlackLightStrengthIndex
            0, // 2,                 // u4HistStretchStrengthIndex
            2,                 // u4AntiOverExpStrengthIndex
            2,                 // u4TimeLPFStrengthIndex
            {1, 3, 5, 7, 8}, // u4LPFConvergeTable[AE_CCT_STRENGTH_NUM]
            90,                // u4InDoorEV = 9.0, 10 base
            -12,               // i4BVOffset delta BV = -2.3
            64,                 // u4PreviewFlareOffset
            64,                 // u4CaptureFlareOffset
            1,                 // u4CaptureFlareThres
            64,                 // u4VideoFlareOffset
            1,                 // u4VideoFlareThres
            32,                 // u4StrobeFlareOffset
            1,                 // u4StrobeFlareThres
            160,                 // u4PrvMaxFlareThres
            0,                 // u4PrvMinFlareThres
            160,                 // u4VideoMaxFlareThres
            0,                 // u4VideoMinFlareThres            
            18,                // u4FlatnessThres              // 10 base for flatness condition.
            75                 // u4FlatnessStrength
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
                871,    // i4R
                512,    // i4G
                610    // i4B
            }
        },
        // Original XY coordinate of AWB light source
        {
           // Strobe
            {
                132,    // i4X
                -261    // i4Y
            },
            // Horizon
            {
                -439,    // i4X
                -268    // i4Y
            },
            // A
            {
                -322,    // i4X
                -283    // i4Y
            },
            // TL84
            {
                -137,    // i4X
                -281    // i4Y
            },
            // CWF
            {
                -95,    // i4X
                -383    // i4Y
            },
            // DNP
            {
                -19,    // i4X
                -296    // i4Y
            },
            // D65
            {
                132,    // i4X
                -261    // i4Y
            },
            // DF
            {
                0,    // i4X
                0    // i4Y
            }
        },
        // Rotated XY coordinate of AWB light source
        {
            // Strobe
            {
                123,    // i4X
                -266    // i4Y
            },
            // Horizon
            {
                -448,    // i4X
                -253    // i4Y
            },
            // A
            {
                -332,    // i4X
                -272    // i4Y
            },
            // TL84
            {
                -147,    // i4X
                -276    // i4Y
            },
            // CWF
            {
                -108,    // i4X
                -380    // i4Y
            },
            // DNP
            {
                -29,    // i4X
                -295    // i4Y
            },
            // D65
            {
                123,    // i4X
                -266    // i4Y
            },
            // DF
            {
                0,    // i4X
                0    // i4Y
            }
        },
        // AWB gain of AWB light source
        {
            // Strobe 
            {
                871,    // i4R
                512,    // i4G
                610    // i4B
            },
            // Horizon 
            {
                512,    // i4R
                646,    // i4G
                1682    // i4B
            },
            // A 
            {
                512,    // i4R
                539,    // i4G
                1224    // i4B
            },
            // TL84 
            {
                622,    // i4R
                512,    // i4G
                901    // i4B
            },
            // CWF 
            {
                755,    // i4R
                512,    // i4G
                978    // i4B
            },
            // DNP 
            {
                745,    // i4R
                512,    // i4G
                784    // i4B
            },
            // D65 
            {
                871,    // i4R
                512,    // i4G
                610    // i4B
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
            2,    // i4RotationAngle
            256,    // i4Cos
            9    // i4Sin
        },
        // Daylight locus parameter
        {
            -136,    // i4SlopeNumerator
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
            -197,    // i4RightBound
            -847,    // i4LeftBound
            -212,    // i4UpperBound
            -312    // i4LowerBound
            },
            // Warm fluorescent
            {
            -197,    // i4RightBound
            -847,    // i4LeftBound
            -312,    // i4UpperBound
            -432    // i4LowerBound
            },
            // Fluorescent
            {
            -79,    // i4RightBound
            -197,    // i4LeftBound
            -199,    // i4UpperBound
            -328    // i4LowerBound
            },
            // CWF
            {
            -79,    // i4RightBound
            -197,    // i4LeftBound
            -328,    // i4UpperBound
            -430    // i4LowerBound
            },
            // Daylight
            {
            148,    // i4RightBound
            -79,    // i4LeftBound
            -186,    // i4UpperBound
            -346    // i4LowerBound
            },
            // Shade
            {
            508,    // i4RightBound
            148,    // i4LeftBound
            -186,    // i4UpperBound
            -346    // i4LowerBound
            },
            // Daylight Fluorescent
            {
            148,    // i4RightBound
            -79,    // i4LeftBound
            -346,    // i4UpperBound
            -440    // i4LowerBound
            }
        },
        // PWB light area
        {
            // Reference area
            {
            508,    // i4RightBound
            -847,    // i4LeftBound
            -161,    // i4UpperBound
            -432    // i4LowerBound
            },
            // Daylight
            {
            173,    // i4RightBound
            -79,    // i4LeftBound
            -186,    // i4UpperBound
            -346    // i4LowerBound
            },
            // Cloudy daylight
            {
            298,    // i4RightBound
            98,    // i4LeftBound
            -186,    // i4UpperBound
            -346    // i4LowerBound
            },
            // Shade
            {
            448,    // i4RightBound
            248,    // i4LeftBound
            -186,    // i4UpperBound
            -346    // i4LowerBound
            },
            // Twilight
            {
            -79,    // i4RightBound
            -239,    // i4LeftBound
            -186,    // i4UpperBound
            -346    // i4LowerBound
            },
            // Fluorescent
            {
            173,    // i4RightBound
            -247,    // i4LeftBound
            -216,    // i4UpperBound
            -430    // i4LowerBound
            },
            // Warm fluorescent
            {
            -247,    // i4RightBound
            -432,    // i4LeftBound
            -216,    // i4UpperBound
            -430    // i4LowerBound
            },
            // Incandescent
            {
            -247,    // i4RightBound
            -432,    // i4LeftBound
            -186,    // i4UpperBound
            -346    // i4LowerBound
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
            790,    // i4R
            512,    // i4G
            678    // i4B
            },
            // Cloudy daylight
            {
            962,    // i4R
            512,    // i4G
            549    // i4B
            },
            // Shade
            {
            1170,    // i4R
            512,    // i4G
            445    // i4B
            },
            // Twilight
            {
            604,    // i4R
            512,    // i4G
            905    // i4B
            },
            // Fluorescent
            {
            767,    // i4R
            512,    // i4G
            822    // i4B
            },
            // Warm fluorescent
            {
            517,    // i4R
            512,    // i4G
            1255    // i4B
            },
            // Incandescent
            {
            477,    // i4R
            512,    // i4G
            1165    // i4B
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
            50,    // i4SliderValue
            4362    // i4OffsetThr
            },
            // Warm fluorescent	
            {
            50,    // i4SliderValue
            4362    // i4OffsetThr
            },
            // Shade
            {
            50,    // i4SliderValue
            841    // i4OffsetThr
            },
            // Daylight WB gain
            {
            715,    // i4R
            512,    // i4G
            754    // i4B
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
                -571,    // i4RotatedXCoordinate[0]
                -455,    // i4RotatedXCoordinate[1]
                -270,    // i4RotatedXCoordinate[2]
                -152,    // i4RotatedXCoordinate[3]
                0    // i4RotatedXCoordinate[4]
            }
        }
    },
    {0}
};

#include INCLUDE_FILENAME_ISP_LSC_PARAM
//};  //  namespace

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
        case CAMERA_DATA_FLASH_AWB_CALIBRATION_DATA:
            memcpy(pDataBuf,&CAMERA_FLASH_AWB_CALIBRATION_DATA,sizeof(FLASH_AWB_CALIBRATION_DATA_STRUCT));
            break;            
        default:
            return 1;
    }
    return 0;
}};  //  NSFeature


