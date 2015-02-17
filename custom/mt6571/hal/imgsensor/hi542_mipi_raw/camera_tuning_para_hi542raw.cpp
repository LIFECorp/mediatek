#include <utils/Log.h>
#include <fcntl.h>
#include <math.h>

#include "camera_custom_nvram.h"
#include "camera_custom_sensor.h"
#include "image_sensor.h"
#include "kd_imgsensor_define.h"
#include "camera_AE_PLineTable_hi542raw.h"
#include "camera_info_hi542raw.h"
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
        72033,    // i4R_AVG
        11423,    // i4R_STD
        92733,    // i4B_AVG
        18791,    // i4B_STD
        {  // i4P00[9]
            5220000, -3276667, 620000, -1360000, 4593333, -673333, -223333, -2293333, 5076667
        },
        {  // i4P10[9]
            2107614, -1606190, -513305, -229561, -204397, 411212, 222692, 434694, -657385
        },
        {  // i4P01[9]
            2139575, -1677669, -470093, -396529, -257243, 632556, 170073, -127295, -42778
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
            8192,  // u4MaxGain, 16x
            32,     // u4MiniISOGain, ISOxx
            128,    // u4GainStepUnit, 1x/8
            33,     // u4PreExpUnit
            30,     // u4PreMaxFrameRate
            33,     // u4VideoExpUnit
            30,     // u4VideoMaxFrameRate
            1024,   // u4Video2PreRatio, 1024 base = 1x
            58,     // u4CapExpUnit
            15,     // u4CapMaxFrameRate
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
            0,                 // u4HistStretchStrengthIndex
            2,                 // u4AntiOverExpStrengthIndex
            2,                 // u4TimeLPFStrengthIndex
            {1, 3, 5, 7, 8}, // u4LPFConvergeTable[AE_CCT_STRENGTH_NUM]
            90,                // u4InDoorEV = 9.0, 10 base
            -10,               // i4BVOffset delta BV = -2.3
            4,                 // u4PreviewFlareOffset
            4,                 // u4CaptureFlareOffset
            5,                 // u4CaptureFlareThres
            4,                 // u4VideoFlareOffset
            5,                 // u4VideoFlareThres
            2,                 // u4StrobeFlareOffset
            2,                 // u4StrobeFlareThres
            8,                 // u4PrvMaxFlareThres
            0,                 // u4PrvMinFlareThres
            8,                 // u4VideoMaxFlareThres
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
    			0,	// i4R
    			0,	// i4G
    			0	// i4B
    		},
    		// rGoldenGain (golden sample gain: 1.0 = 512)
    		{
	            0,	// i4R
	            0,	// i4G
	            0	// i4B
            },
    		// rTuningUnitGain (Tuning sample unit gain: 1.0 = 512)
    		{
	            0,	// i4R
	            0,	// i4G
	            0	// i4B
            },
            // rD65Gain (D65 WB gain: 1.0 = 512)
            {
                867,    // i4R
                512,    // i4G
                708    // i4B
    		}
    	},
    	// Original XY coordinate of AWB light source
    	{
		    // Strobe
		    {
                -64,    // i4X
                -347    // i4Y
		    },
    		// Horizon
    		{
                -395,    // i4X
                -379    // i4Y
            },
            // A
            {
                -284,    // i4X
                -374    // i4Y
            },
            // TL84
            {
                -164,    // i4X
                -342    // i4Y
            },
            // CWF
            {
                -126,    // i4X
                -381    // i4Y
            },
            // DNP
            {
                -64,    // i4X
                -347    // i4Y
            },
            // D65
            {
                75,    // i4X
                -314    // i4Y
    		},
		// DF
		{
			0,	// i4X
			0	// i4Y
    		}
    	},
    	// Rotated XY coordinate of AWB light source
    	{
		    // Strobe
		    {
                -112,    // i4X
                -335    // i4Y
		    },
    		// Horizon
            {
                -445,    // i4X
                -320    // i4Y
            },
            // A
            {
                -334,    // i4X
                -331    // i4Y
            },
            // TL84
            {
                -211,    // i4X
                -316    // i4Y
            },
            // CWF
            {
                -179,    // i4X
                -360    // i4Y
            },
            // DNP
            {
                -112,    // i4X
                -335    // i4Y
            },
            // D65
            {
                30,    // i4X
                -322    // i4Y
            },
		// DF
		{
			0,	// i4X
			0	// i4Y
    		}
    	},
	// AWB gain of AWB light source
	{
		// Strobe
		{
                751,    // i4R
			512,	// i4G
                893    // i4B
		},
		// Horizon
            {
                512,    // i4R
                523,    // i4G
                1490    // i4B
            },
            // A 
            {
                579,    // i4R
                512,    // i4G
                1248    // i4B
            },
            // TL84 
            {
                652,    // i4R
                512,    // i4G
                1015    // i4B
            },
            // CWF 
            {
                724,    // i4R
                512,    // i4G
                1017    // i4B
            },
            // DNP 
            {
                751,    // i4R
                512,    // i4G
                893    // i4B
            },
            // D65 
            {
                867,    // i4R
                512,    // i4G
                708    // i4B
            },
		// DF
		{
			512,	// i4R
			512,	// i4G
			512     // i4B
		}
	},
    	// Rotation matrix parameter
    	{
            8,    // i4RotationAngle
            254,    // i4Cos
            36    // i4Sin
        },
        // Daylight locus parameter
        {
            -171,    // i4SlopeNumerator
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
            -261,    // i4RightBound
            -911,    // i4LeftBound
            -215,    // i4UpperBound
            -375    // i4LowerBound
            },
            // Warm fluorescent
            {
            -261,    // i4RightBound
            -911,    // i4LeftBound
            -375,    // i4UpperBound
            -495    // i4LowerBound
            },
            // Fluorescent
            {
            -162,    // i4RightBound
            -261,    // i4LeftBound
            -251,    // i4UpperBound
            -338    // i4LowerBound
            },
            // CWF
            {
            -162,    // i4RightBound
            -261,    // i4LeftBound
            -338,    // i4UpperBound
            -410    // i4LowerBound
            },
            // Daylight
            {
            55,    // i4RightBound
            -162,    // i4LeftBound
            -242,    // i4UpperBound
            -402    // i4LowerBound
            },
            // Shade
            {
            415,    // i4RightBound
            55,    // i4LeftBound
            -242,    // i4UpperBound
            -402    // i4LowerBound
		},
		// Daylight Fluorescent
		{
            55,    // i4RightBound
            -162,    // i4LeftBound
            -402,    // i4UpperBound
            -550    // i4LowerBound
    		}
    	},
    	// PWB light area
    	{
    		// Reference area
    		{
            415,    // i4RightBound
            -911,    // i4LeftBound
            0,    // i4UpperBound
            -550    // i4LowerBound
            },
            // Daylight
            {
            80,    // i4RightBound
            -162,    // i4LeftBound
            -242,    // i4UpperBound
            -402    // i4LowerBound
            },
            // Cloudy daylight
            {
            180,    // i4RightBound
            5,    // i4LeftBound
            -242,    // i4UpperBound
            -402    // i4LowerBound
            },
            // Shade
            {
            280,    // i4RightBound
            5,    // i4LeftBound
            -242,    // i4UpperBound
            -402    // i4LowerBound
            },
            // Twilight
            {
            -162,    // i4RightBound
            -322,    // i4LeftBound
            -242,    // i4UpperBound
            -402    // i4LowerBound
            },
            // Fluorescent
            {
            80,    // i4RightBound
            -311,    // i4LeftBound
            -266,    // i4UpperBound
            -410    // i4LowerBound
            },
            // Warm fluorescent
            {
            -234,    // i4RightBound
            -434,    // i4LeftBound
            -266,    // i4UpperBound
            -410    // i4LowerBound
            },
            // Incandescent
            {
            -234,    // i4RightBound
            -434,    // i4LeftBound
            -242,    // i4UpperBound
            -402    // i4LowerBound
    		},
            {// Gray World
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
            799,    // i4R
            512,    // i4G
            789    // i4B
            },
            // Cloudy daylight
            {
            931,    // i4R
            512,    // i4G
            643    // i4B
            },
            // Shade
            {
            986,    // i4R
            512,    // i4G
            596    // i4B
            },
            // Twilight
            {
            634,    // i4R
            512,    // i4G
            1072    // i4B
            },
            // Fluorescent
            {
            751,    // i4R
            512,    // i4G
            900    // i4B
            },
            // Warm fluorescent
            {
            585,    // i4R
            512,    // i4G
            1257    // i4B
            },
            // Incandescent
            {
            571,    // i4R
            512,    // i4G
            1234    // i4B
		},
		// Gray World
		{
			512,	// i4R
			512,	// i4G
			512	// i4B
    		}
    	},
    	// AWB preference color
    	{
    		// Tungsten
    		{
            0,    // i4SliderValue
            5680    // i4OffsetThr
            },
            // Warm fluorescent	
            {
            0,    // i4SliderValue
            5005    // i4OffsetThr
            },
            // Shade
            {
            50,    // i4SliderValue
            344    // i4OffsetThr
            },
            // Daylight WB gain
            {
            736,    // i4R
            512,    // i4G
            879    // i4B
		},
		// Preference gain: strobe
		{
			512,	// i4R
			512,	// i4G
			512	// i4B
		},
		// Preference gain: tungsten
		{
			512,	// i4R
			512,	// i4G
			512	    // i4B
		},
		// Preference gain: warm fluorescent
		{
			512,	// i4R
			512,	// i4G
			512	    // i4B
		},
		// Preference gain: fluorescent
		{
			512,	// i4R
			512,	// i4G
			512	    // i4B
		},
		// Preference gain: CWF
		{
			512,	// i4R
			512,	// i4G
			512	    // i4B
		},
		// Preference gain: daylight
		{
			512,	// i4R
			512,	// i4G
			512	    // i4B
		},
		// Preference gain: shade
		{
			512,	// i4R
			512,	// i4G
			512	    // i4B
		},
		// Preference gain: daylight fluorescent
		{
			512,	// i4R
			512,	// i4G
			512	    // i4B
    		}
    	},
    	// CCT estimation
    	{
    		// CCT
    		{
			    2300,	// i4CCT[0]
    			2850,	// i4CCT[1]
    			4100,	// i4CCT[2]
    			5100,	// i4CCT[3]
    			6500	// i4CCT[4]
    		},
    		// Rotated X coordinate
    		{
                -475,    // i4RotatedXCoordinate[0]
                -364,    // i4RotatedXCoordinate[1]
                -241,    // i4RotatedXCoordinate[2]
                -142,    // i4RotatedXCoordinate[3]
    			0	// i4RotatedXCoordinate[4]
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
}};  //  NSFeature


