#include <utils/Log.h>
#include <fcntl.h>
#include <math.h>

#include "camera_custom_nvram.h"
#include "camera_custom_sensor.h"
#include "image_sensor.h"
#include "kd_imgsensor_define.h"
#include "camera_AE_PLineTable_ov5648mipiraw.h"
#include "camera_info_ov5648mipiraw.h"
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
        66900,    // i4R_AVG
        9404,    // i4R_STD
        76150,    // i4B_AVG
        14830,    // i4B_STD
        {  // i4P00[9]
            4325000, -890000, -870000, -1032500, 4030000, -430000, -162500, -2662500, 5392500
        },
        {  // i4P10[9]
            1089465, -1312576, 229777, 134170, -303888, 177702, 87663, 172592, -252891
        },
        {  // i4P01[9]
            514500, -697814, 185076, -102170, -235752, 335593, -70065, -345098, 425991
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
            1136,   // u4MinGain, 1024 base =  1x
            10240,  // u4MaxGain, 16x
            51,     // u4MiniISOGain, ISOxx
            64,    // u4GainStepUnit, 1x/8
            33524,     // u4PreExpUnit
            30,     // u4PreMaxFrameRate
            33524,     // u4VideoExpUnit
            30,     // u4VideoMaxFrameRate
            1024,   // u4Video2PreRatio, 1024 base = 1x
            33524,     // u4CapExpUnit
            15,     // u4CapMaxFrameRate
            1024,   // u4Cap2PreRatio, 1024 base = 1x
            24,      // u4LensFno, Fno = 2.8
            350     // u4FocusLength_100x
         },
         // rHistConfig
        {
            4,   // u4HistHighThres
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
            -3,               // i4BVOffset delta BV = -2.3
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
		// rCalGain (calibration gain: 1.0 = 512)						
		{						
			0,	// u4R				
			0,	// u4G				
			0	// u4B				
		},						
		// rDefGain (Default calibration gain: 1.0 = 512)						
		{						
			0,	// u4R				
			0,	// u4G				
			0	// u4B				
		},						
		// rDefGain (Default calibration gain: 1.0 = 512)						
		{						
			0,	// u4R				
			0,	// u4G				
			0	// u4B				
		},						
		// rD65Gain (D65 WB gain: 1.0 = 512)						
		{						
			768,	// u4R				
			512,	// u4G				
			576	// u4B				
		}						
	},							
	// Original XY coordinate of AWB light source							
	{							
		// Strobe						
		{						
			-39,	// i4X				
			-411	// i4Y				
		},						
		// Horizon						
		{						
			-376,	// i4X				
			-294	// i4Y				
		},						
		// A						
		{						
			-263,	// i4X				
			-286	// i4Y				
		},						
		// TL84						
		{						
			-160,	// i4X				
			-259	// i4Y				
		},						
		// CWF						
		{						
			-116,	// i4X				
			-357	// i4Y				
		},						
		// DNP						
		{						
			-17,	// i4X				
			-252	// i4Y				
		},						
		// D65						
		{						
			106,	// i4X				
			-193	// i4Y				
		},						
		// DF						
		{						
			0, 	// i4X				
			0	// i4Y				
		}						
	},							
	// Rotated XY coordinate of AWB light source							
	{							
		// Strobe						
		{						
			-131,	// i4X				
			-391	// i4Y				
		},						
		// Horizon						
		{						
			-432,	// i4X				
			-201	// i4Y				
		},						
		// A						
		{						
			-321,	// i4X				
			-219	// i4Y				
		},						
		// TL84						
		{						
			-214,	// i4X				
			-216	// i4Y				
		},						
		// CWF						
		{						
			-194,	// i4X				
			-321	// i4Y				
		},						
		// DNP						
		{						
			-74,	// i4X				
			-241	// i4Y				
		},						
		// D65						
		{						
			59,	// i4X				
			-212	// i4Y				
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
			848,	// u4R				
			512,	// u4G				
			941	// u4B				
		},						
		// Horizon						
		{						
			512,	// u4R				
			572,	// u4G				
			1417	// u4B				
		},						
		// A						
		{						
			528,	// u4R				
			512,	// u4G				
			1077	// u4B				
		},						
		// TL84						
		{						
			586,	// u4R				
			512,	// u4G				
			903	// u4B				
		},						
		// CWF						
		{						
			709,	// u4R				
			512,	// u4G				
			971	// u4B				
		},						
		// DNP						
		{						
			703,	// u4R				
			512,	// u4G				
			737	// u4B				
		},						
		// D65						
		{						
			768,	// u4R				
			512,	// u4G				
			576	// u4B				
		},						
		// DF						
		{						
			512,	// u4R				
			512,	// u4G				
			512	// u4B				
		}						
	},							
	// Rotation matrix parameter							
	{							
		13,	// i4RotationAngle					
		249,	// i4Cos					
		58,	// i4Sin					
	},							
	// Daylight locus parameter							
	{							
		-203,	// i4SlopeNumerator					
		128	// i4SlopeDenominator					
	},							
	// AWB light area							
	{							
		// Strobe						
		{						
			120,	// i4RightBound				
			-264,	// i4LeftBound				
			-322,	// i4UpperBound				
			-491	// i4LowerBound				
		},						
		// Tungsten						
		{						
			-314,	// i4RightBound				
			-964,	// i4LeftBound				
			-170,	// i4UpperBound				
			-260	// i4LowerBound				
		},						
		// Warm fluorescent						
		{						
			-314,	// i4RightBound				
			-964,	// i4LeftBound				
			-260,	// i4UpperBound				
			-380	// i4LowerBound				
		},						
		// Fluorescent						
		{						
			-124,	// i4RightBound				
			-314,	// i4LeftBound				
			-179,	// i4UpperBound				
			-269	// i4LowerBound				
		},						
		// CWF						
		{						
			-124,	// i4RightBound				
			-314,	// i4LeftBound				
			-269,	// i4UpperBound				
			-401	// i4LowerBound				
		},						
		// Daylight						
		{						
			84,	// i4RightBound				
			-124,	// i4LeftBound				
			-147,	// i4UpperBound				
			-322	// i4LowerBound				
		},						
		// Shade						
		{						
			444,	// i4RightBound				
			84,	// i4LeftBound				
			-147,	// i4UpperBound				
			-322	// i4LowerBound				
		},						
		// Daylight Fluorescent						
		{						
			0,	// i4RightBound				
			0,	// i4LeftBound				
			0,	// i4UpperBound				
			0	// i4LowerBound				
		}						
	},							
	// PWB light area							
	{							
		// Reference area						
		{						
			444,	// i4RightBound				
			-964,	// i4LeftBound				
			-122,	// i4UpperBound				
			-401	// i4LowerBound				
		},						
		// Daylight						
		{						
			109,	// i4RightBound				
			-124,	// i4LeftBound				
			-147,	// i4UpperBound				
			-322	// i4LowerBound				
		},						
		// Cloudy daylight						
		{						
			209,	// i4RightBound				
			34,	// i4LeftBound				
			-147,	// i4UpperBound				
			-322	// i4LowerBound				
		},						
		// Shade						
		{						
			309,	// i4RightBound				
			34,	// i4LeftBound				
			-147,	// i4UpperBound				
			-322	// i4LowerBound				
		},						
		// Twilight						
		{						
			-124,	// i4RightBound				
			-284,	// i4LeftBound				
			-147,	// i4UpperBound				
			-322	// i4LowerBound				
		},						
		// Fluorescent						
		{						
			109,	// i4RightBound				
			-314,	// i4LeftBound				
			-162,	// i4UpperBound				
			-401	// i4LowerBound				
		},						
		// Warm fluorescent						
		{						
			-221,	// i4RightBound				
			-421,	// i4LeftBound				
			-162,	// i4UpperBound				
			-371	// i4LowerBound				
		},						
		// Incandescent						
		{						
			-221,	// i4RightBound				
			-421,	// i4LeftBound				
			-147,	// i4UpperBound				
			-322	// i4LowerBound				
		},						
		// Gray World						
		{						
			5000,	// i4RightBound				
			-5000,	// i4LeftBound				
			5000,	// i4UpperBound				
			-5000	// i4LowerBound				
		}						
	},							
	// PWB default gain							
	{							
		// Daylight						
		{						
			744,	// u4R				
			512,	// u4G				
			657	// u4B				
		},						
		// Cloudy daylight						
		{						
			848,	// u4R				
			512,	// u4G				
			533	// u4B				
		},						
		// Shade						
		{						
			892,	// u4R				
			512,	// u4G				
			491	// u4B				
		},						
		// Twilight						
		{						
			610,	// u4R				
			512,	// u4G				
			905	// u4B				
		},						
		// Fluorescent						
		{						
			712,	// u4R				
			512,	// u4G				
			792	// u4B				
		},						
		// Warm fluorescent						
		{						
			571,	// u4R				
			512,	// u4G				
			1131	// u4B				
		},						
		// Incandescent						
		{						
			542,	// u4R				
			512,	// u4G				
			1095	// u4B				
		},						
		// Gray World						
		{						
			512,	// u4R				
			512,	// u4G				
			512	// u4B				
		}						
	},							
	// AWB preference color							
	{							
		// Tungsten						
		{						
			50,	// i4SliderValue				
			5213	// i4OffsetThr				
		},						
		// Warm fluorescent						
		{						
			50,	// i4SliderValue				
			5213	// i4OffsetThr				
		},						
		// Shade						
		{						
			50,	// i4SliderValue				
			849	// i4OffsetThr				
		},						
		// Daylight WB gain						
		{						
			671,	// u4R				
			512,	// u4G				
			716	// u4B				
		},						
		// Preference gain: strobe						
		{						
			512,	// u4R				
			512,	// u4G				
			512	// u4B				
		},						
		// Preference gain: tungsten						
		{						
			512,	// u4R				
			512,	// u4G				
			512	// u4B				
		},						
		// Preference gain: warm fluorescent						
		{						
			512,	// u4R				
			512,	// u4G				
			512	// u4B				
		},						
		// Preference gain: fluorescent						
		{						
			512,	// u4R				
			512,	// u4G				
			512	// u4B				
		},						
		// Preference gain: CWF						
		{						
			512,	// u4R				
			512,	// u4G				
			512	// u4B				
		},						
		// Preference gain: daylight						
		{						
			512,	// u4R				
			512,	// u4G				
			512	// u4B				
		},						
		// Preference gain: shade						
		{						
			512,	// u4R				
			512,	// u4G				
			512	// u4B				
		},						
		// Preference gain: daylight fluorescent						
		{						
			512,	// u4R				
			512,	// u4G				
			512	// u4B				
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
			6500 	// i4CCT[4]				
		},						
		// Rotated X coordinate						
		{						
			-491,	// i4RotatedXCoordinate[0]				
			-380,	// i4RotatedXCoordinate[1]				
			-273,	// i4RotatedXCoordinate[2]				
			-133,	// i4RotatedXCoordinate[3]				
			0 	// i4RotatedXCoordinate[4]				
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
}};  //  NSFeature


