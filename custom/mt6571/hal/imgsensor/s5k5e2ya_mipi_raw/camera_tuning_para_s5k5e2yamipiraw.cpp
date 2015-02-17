#include <utils/Log.h>
#include <fcntl.h>
#include <math.h>

#include "camera_custom_nvram.h"
#include "camera_custom_sensor.h"
#include "image_sensor.h"
#include "kd_imgsensor_define.h"
#include "camera_AE_PLineTable_s5k5e2yamipiraw.h"
#include "camera_info_s5k5e2yamipiraw.h"
#include "camera_custom_AEPlinetable.h"
#include "camera_custom_tsf_tbl.h"
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
        75170,    // i4R_AVG
        13190,    // i4R_STD
        79140,    // i4B_AVG
        26270,    // i4B_STD
        {  // i4P00[9]
            4448648, -1494813, -393843, -604477, 3414513, -250036, 85095, -1385454, 3860283
        },
        {  // i4P10[9]
            933698, -628943, -304758, -247520, -22220, 269740, -73861, 196166, -122555
        },
        {  // i4P01[9]
            814367, -494023, -320352, -358410, -180556, 538966, -57406, -190454, 247689
        },
        {  // i4P20[9]
            394007, -491950, 98031, -21525, 59812, -38287, 140879, -521951, 381045
        },
        {  // i4P11[9]
            -35750, -344806, 380738, 121574, 59500, -181074, 143388, -309535, 166309
        },
        {  // i4P02[9]
            -315751, 65233, 250618, 151463, 34149, -185612, 21808, -8637, -12997
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
            10240,    // u4MaxGain, 16x
            50,    // u4MiniISOGain, ISOxx  
            128,    // u4GainStepUnit, 1x/8 
            17,    // u4PreExpUnit 
            30,    // u4PreMaxFrameRate
            17,    // u4VideoExpUnit  
            30,    // u4VideoMaxFrameRate 
            1024,    // u4Video2PreRatio, 1024 base = 1x 
            17,    // u4CapExpUnit 
            29,    // u4CapMaxFrameRate
            1024,    // u4Cap2PreRatio, 1024 base = 1x
            28,    // u4LensFno, Fno = 2.8
            350    // u4FocusLength_100x
        },
        // rHistConfig
        {
            4,    // u4HistHighThres
            40,    // u4HistLowThres
            2,    // u4MostBrightRatio
            1,    // u4MostDarkRatio
            160,    // u4CentralHighBound
            20,    // u4CentralLowBound
            {240, 230, 220, 210, 200},    // u4OverExpThres[AE_CCT_STRENGTH_NUM] 
            {62, 70, 82, 108, 141},    // u4HistStretchThres[AE_CCT_STRENGTH_NUM] 
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
            47,    // u4AETarget
            47,    // u4StrobeAETarget
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
            -2,    // i4BVOffset delta BV = value/10 
            0,    // u4PreviewFlareOffset
            0,    // u4CaptureFlareOffset
            3,    // u4CaptureFlareThres
            0,    // u4VideoFlareOffset
            3,    // u4VideoFlareThres
            0,    // u4StrobeFlareOffset
            3,    // u4StrobeFlareThres
            160,    // u4PrvMaxFlareThres
            0,    // u4PrvMinFlareThres
            160,    // u4VideoMaxFlareThres
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
                765,    // i4R
                512,    // i4G
                578    // i4B
            }
        },
        // Original XY coordinate of AWB light source
        {
           // Strobe
            {
                104,    // i4X
                -193    // i4Y
            },
            // Horizon
            {
                -422,    // i4X
                -265    // i4Y
            },
            // A
            {
                -321,    // i4X
                -282    // i4Y
            },
            // TL84
            {
                -204,    // i4X
                -280    // i4Y
            },
            // CWF
            {
                -151,    // i4X
                -352    // i4Y
            },
            // DNP
            {
                -80,    // i4X
                -267    // i4Y
            },
            // D65
            {
                104,    // i4X
                -193    // i4Y
            },
            // DF
            {
                45,    // i4X
                -309    // i4Y
            }
        },
        // Rotated XY coordinate of AWB light source
        {
            // Strobe
            {
                65,    // i4X
                -209    // i4Y
            },
            // Horizon
            {
                -464,    // i4X
                -179    // i4Y
            },
            // A
            {
                -369,    // i4X
                -215    // i4Y
            },
            // TL84
            {
                -254,    // i4X
                -235    // i4Y
            },
            // CWF
            {
                -215,    // i4X
                -316    // i4Y
            },
            // DNP
            {
                -130,    // i4X
                -246    // i4Y
            },
            // D65
            {
                65,    // i4X
                -209    // i4Y
            },
            // DF
            {
                -15,    // i4X
                -312    // i4Y
            }
        },
        // AWB gain of AWB light source
        {
            // Strobe 
            {
                765,    // i4R
                512,    // i4G
                578    // i4B
            },
            // Horizon 
            {
                512,    // i4R
                633,    // i4G
                1606    // i4B
            },
            // A 
            {
                512,    // i4R
                540,    // i4G
                1219    // i4B
            },
            // TL84 
            {
                568,    // i4R
                512,    // i4G
                986    // i4B
            },
            // CWF 
            {
                672,    // i4R
                512,    // i4G
                1012    // i4B
            },
            // DNP 
            {
                660,    // i4R
                512,    // i4G
                819    // i4B
            },
            // D65 
            {
                765,    // i4R
                512,    // i4G
                578    // i4B
            },
            // DF 
            {
                826,    // i4R
                512,    // i4G
                732    // i4B
            }
        },
        // Rotation matrix parameter
        {
            11,    // i4RotationAngle
            251,    // i4Cos
            49    // i4Sin
        },
        // Daylight locus parameter
        {
            -185,    // i4SlopeNumerator
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
            -304,    // i4RightBound
            -954,    // i4LeftBound
            -147,    // i4UpperBound
            -247    // i4LowerBound
            },
            // Warm fluorescent
            {
            -304,    // i4RightBound
            -954,    // i4LeftBound
            -247,    // i4UpperBound
            -367    // i4LowerBound
            },
            // Fluorescent
            {
            -180,    // i4RightBound
            -304,    // i4LeftBound
            -138,    // i4UpperBound
            -275    // i4LowerBound
            },
            // CWF
            {
            -180,    // i4RightBound
            -304,    // i4LeftBound
            -275,    // i4UpperBound
            -366    // i4LowerBound
            },
            // Daylight
            {
            90,    // i4RightBound
            -180,    // i4LeftBound
            -129,    // i4UpperBound
            -289    // i4LowerBound
            },
            // Shade
            {
            450,    // i4RightBound
            90,    // i4LeftBound
            -129,    // i4UpperBound
            -289    // i4LowerBound
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
            450,    // i4RightBound
            -954,    // i4LeftBound
            -104,    // i4UpperBound
            -367    // i4LowerBound
            },
            // Daylight
            {
            115,    // i4RightBound
            -180,    // i4LeftBound
            -129,    // i4UpperBound
            -289    // i4LowerBound
            },
            // Cloudy daylight
            {
            215,    // i4RightBound
            40,    // i4LeftBound
            -129,    // i4UpperBound
            -289    // i4LowerBound
            },
            // Shade
            {
            315,    // i4RightBound
            40,    // i4LeftBound
            -129,    // i4UpperBound
            -289    // i4LowerBound
            },
            // Twilight
            {
            -180,    // i4RightBound
            -340,    // i4LeftBound
            -129,    // i4UpperBound
            -289    // i4LowerBound
            },
            // Fluorescent
            {
            115,    // i4RightBound
            -354,    // i4LeftBound
            -159,    // i4UpperBound
            -366    // i4LowerBound
            },
            // Warm fluorescent
            {
            -269,    // i4RightBound
            -469,    // i4LeftBound
            -159,    // i4UpperBound
            -366    // i4LowerBound
            },
            // Incandescent
            {
            -269,    // i4RightBound
            -469,    // i4LeftBound
            -129,    // i4UpperBound
            -289    // i4LowerBound
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
            689,    // i4R
            512,    // i4G
            674    // i4B
            },
            // Cloudy daylight
            {
            818,    // i4R
            512,    // i4G
            523    // i4B
            },
            // Shade
            {
            863,    // i4R
            512,    // i4G
            483    // i4B
            },
            // Twilight
            {
            540,    // i4R
            512,    // i4G
            968    // i4B
            },
            // Fluorescent
            {
            684,    // i4R
            512,    // i4G
            820    // i4B
            },
            // Warm fluorescent
            {
            524,    // i4R
            512,    // i4G
            1219    // i4B
            },
            // Incandescent
            {
            481,    // i4R
            512,    // i4G
            1151    // i4B
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
            5113    // i4OffsetThr
            },
            // Warm fluorescent	
            {
            50,    // i4SliderValue
            5113    // i4OffsetThr
            },
            // Shade
            {
            50,    // i4SliderValue
            846    // i4OffsetThr
            },
            // Daylight WB gain
            {
            621,    // i4R
            512,    // i4G
            787    // i4B
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
                -529,    // i4RotatedXCoordinate[0]
                -434,    // i4RotatedXCoordinate[1]
                -319,    // i4RotatedXCoordinate[2]
                -195,    // i4RotatedXCoordinate[3]
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
                                             sizeof(CAMERA_TSF_TBL_STRUCT)};

    if (CameraDataType > CAMERA_DATA_TSF_TABLE || NULL == pDataBuf || (size < dataSize[CameraDataType]))
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
        default:
            break;
    }
    return 0;
}}; // NSFeature


