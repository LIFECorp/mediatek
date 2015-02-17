/*BEGIN PN:DTS2013053100362, Added by y00213338 , 2013-05-31*/

/*BEGIN PN:DTS2013062906537, Modified by w00167383 , 2013-06-29*/
#include <utils/Log.h>
#include <fcntl.h>
#include <math.h>

#include "camera_custom_nvram.h"
#include "camera_custom_sensor.h"
#include "image_sensor.h"
#include "kd_imgsensor_define.h"
#include "camera_AE_PLineTable_s5k4e1gamipiraw.h"
#include "camera_info_s5k4e1gamipiraw.h"
#include "camera_custom_AEPlinetable.h"
#include "camera_custom_tsf_tbl.h"
//#include "camera_custom_flicker_table.h"
//#include "camera_flicker_table_s5k4e1gamipiraw.h"

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
        66350, // i4R_AVG
        11296, // i4R_STD
        89925, // i4B_AVG
        20671, // i4B_STD
        { // i4P00[9]
            4612500, -1827500, -225000, -1330000, 4042500, -152500,  -455000, -1727500, 4742500 
            //4402500, -1615000, -227500, -1275000, 3977500, -142500,  -455000, -1600000, 4615000 
        },
        { // i4P10[9]
            293933,  -317309, 23375, 192429,  -450480,  258050, -202378,   25466, 176912
            //340567,  -356792, 16225, 187789,  -483299,  295510, -253801,   6736, 247066
        },
        { // i4P01[9]
            -229996,  366877, -136880, -197981, 1690,  196290, -442919,  -2199,  445117
            //204236,  -65512, -138724, -299681, 81303,  218378, -495673,  -253056,  748729
        },
        { // i4P20[9]
            0,  0,   0,  0,   0,  0, 0,  0,  0
        },
        { // i4P11[9]
            0,  0,   0,  0,   0,  0, 0,  0,  0
        },
        { // i4P02[9]
            0,  0,   0,  0,   0,  0, 0,  0,  0
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
            16384,  // u4MaxGain, 16x
            51,     // u4MiniISOGain, ISOxx
            256,    // u4GainStepUnit, 1x/8
            33555,     // u4PreExpUnit
            30,     // u4PreMaxFrameRate
            33555,     // u4VideoExpUnit
            30,     // u4VideoMaxFrameRate
            1024,   // u4Video2PreRatio, 1024 base = 1x
            33555,     // u4CapExpUnit
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
            FALSE,            // bEnableCaptureThres  //fix flare
            FALSE,            // bEnableVideoThres   //lin yang modify fix flare
            FALSE,            // bEnableStrobeThres  // lin yang modify fix flare
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
            0,               // i4BVOffset delta BV = -2.3 
            64,                 // u4PreviewFlareOffset   //12bit
            64,                 // u4CaptureFlareOffset  //12bit
            5,                 // u4CaptureFlareThres
            64,                 // u4VideoFlareOffset   //12bit
            5,                 // u4VideoFlareThres
            32,                 // u4StrobeFlareOffset  //12bit
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
                793,    // i4R  
	           512,	// i4G
                615    // i4B   
    		}
    	},
    	// Original XY coordinate of AWB light source
    	{
		    // Strobe
		    {
                -31,    // i4X
                -258    // i4Y
		    },
    		// Horizon
    		{
                -343,    // i4X
                -291    // i4Y
    		},
    		// A
    		{
                -267,    // i4X
                -299    // i4Y
    		},
    		// TL84
    		{
                -142,    // i4X
                -275    // i4Y
    		},
    		// CWF
    		{
                -118,    // i4X
                -363    // i4Y
    		},
    		// DNP
    		{
                -34,    // i4X
                -261    // i4Y
    		},
    		// D65
    		{
                94,    // i4X
                -229    // i4Y
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
                -75,    // i4X
                -249    // i4Y
		    },
    		// Horizon
    		{
                -388,    // i4X
                -227    // i4Y
    		},
    		// A
    		{
                -314,    // i4X
                -248    // i4Y
    		},
    		// TL84
    		{
                -187,    // i4X
                -246    // i4Y
    		},
    		// CWF
    		{
                -179,    // i4X
                -337    // i4Y
    		},
    		// DNP
    		{
                -78,    // i4X
                -251    // i4Y
    		},
    		// D65
    		{
                53,    // i4X
                -242    // i4Y
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
                696,    // i4R
			512,	// i4G
                758    // i4B
		},
		// Horizon
		{
			512,	// i4R
                549,    // i4G
                1296    // i4B
		},
		// A
		{
                535,    // i4R
                512,    // i4G
                1101    // i4B
		},
		// TL84
		{
                613,    // i4R
			512,	// i4G
                900    // i4B
		},
		// CWF
		{
                713,    // i4R
			512,	// i4G
                981    // i4B
		},
		// DNP
		{
                696,    // i4R
			512,	// i4G
                764    // i4B
		},
		// D65
		{
                793,    // i4R
			512,	// i4G
                615    // i4B
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
            10,    // i4RotationAngle
            252,    // i4Cos
            44    // i4Sin
    	},
    	// Daylight locus parameter
    	{
            -179,    // i4SlopeNumerator
    		128	// i4SlopeDenominator
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
            //modify for red foam awb error 
    		{
            -237,    // i4RightBound
            -887,    // i4LeftBound
            -180,//-50,    // i4UpperBound //-47-->0   //f00208919 20130228  //0-->-20>-50  //0306
            -317 //-287    // i4LowerBound
    		},
    		// Warm fluorescent
    		{
            -237,    // i4RightBound
            -887,    // i4LeftBound
            -317, //-287,    // i4UpperBound
            -427 //-407    // i4LowerBound
    		},
    		// Fluorescent
    		{
            -128,    // i4RightBound
            -237,    // i4LeftBound
            -196,//-156,    // i4UpperBound  //-67--->156
            -291    // i4LowerBound
    		},
    		// CWF
    		{
            -128,    // i4RightBound
            -237,    // i4LeftBound
            -291,    // i4UpperBound
            -370 //-387    // i4LowerBound
    		},
    		// Daylight
    		{
            78,    // i4RightBound
            -128,    // i4LeftBound
            -152,    // i4UpperBound
            -332    // i4LowerBound
    		},
    		// Shade
    		{
            438,    // i4RightBound
            78,    // i4LeftBound
            -152,    // i4UpperBound
            -332    // i4LowerBound
		},
		// Daylight Fluorescent
		{
            40,    // i4RightBound
            -128,    // i4LeftBound
            -332,    // i4UpperBound
            -410    // i4LowerBound
    		}
    	},
    	// PWB light area
    	{
    		// Reference area
    		{
            438,    // i4RightBound
            -887,    // i4LeftBound
            0,    // i4UpperBound
            -410    // i4LowerBound
    		},
    		// Daylight
    		{
            103,    // i4RightBound
            -128,    // i4LeftBound
            -152,    // i4UpperBound
            -332    // i4LowerBound
    		},
    		// Cloudy daylight
    		{
            228,    // i4RightBound
            28,    // i4LeftBound
            -152,    // i4UpperBound
            -332    // i4LowerBound
    		},
    		// Shade
    		{
            378,    // i4RightBound
            178,    // i4LeftBound
            -152,    // i4UpperBound
            -332    // i4LowerBound
    		},
    		// Twilight
    		{
            -128,    // i4RightBound
            -288,    // i4LeftBound
            -152,    // i4UpperBound
            -332    // i4LowerBound
    		},
    		// Fluorescent
    		{
            103,    // i4RightBound
            -287,    // i4LeftBound
            -192,    // i4UpperBound
            -387    // i4LowerBound
    		},
    		// Warm fluorescent
    		{
            -287,    // i4RightBound
            -414,    // i4LeftBound
            -192,    // i4UpperBound
            -387    // i4LowerBound
    		},
    		// Incandescent
    		{
            -287,    // i4RightBound
            -414,    // i4LeftBound
            -152,    // i4UpperBound
            -332    // i4LowerBound
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
            738,    // i4R
    		512,	// i4G
            682    // i4B
    		},
    		// Cloudy daylight
    		{
            862,    // i4R
    		512,	// i4G
            547    // i4B
    		},
    		// Shade
    		{
            1016,    // i4R
    			512,	// i4G
            432    // i4B
    		},
    		// Twilight
    		{
            595,    // i4R
    			512,	// i4G
            926    // i4B
    		},
    		// Fluorescent
    		{
            728,    // i4R
    			512,	// i4G
            814    // i4B
    		},
    		// Warm fluorescent
    		{
            548,    // i4R
    			512,	// i4G
            1220    // i4B
    		},
    		// Incandescent
    		{
            509,    // i4R
    			512,	// i4G
            1158    // i4B
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
            5779    // i4OffsetThr
    		},
    		// Warm fluorescent
    		{
            0,    // i4SliderValue
            5006    // i4OffsetThr
    		},
    		// Shade
    		{
            10,    // i4SliderValue
            1145    // i4OffsetThr
    		},
    		// Daylight WB gain
    		{
            687,    // i4R
    			512,	// i4G
            755    // i4B
		},
		// Preference gain: strobe
		{
			512,	// i4R
			512,	// i4G
			512	// i4B
		},
		// Preference gain: tungsten
		{
            500,//450,    // i4R  //490 //501 //502  //506 //f00208919 20130228
			512,	// i4G
            512,//506    // i4B   //510//512 //518 //f00208919 20130228
		},
		// Preference gain: warm fluorescent
		{
            506,//460,    // i4R  //496 //497 //498  //502  //f00208919 20130228
			512,	// i4G
            512 //506    // i4B  //514 //516 //518  //f00208919 20130228
		},
		// Preference gain: fluorescent
		{
			506,	// i4R
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
            506,    // i4R  //512--502--500--499--498  //f00208919 20130228
            512,    // i4G
            512    // i4B  //512-->514--512  //f00208919 20130228
		},
		// Preference gain: shade
		{
            504,    // i4R  //512--->504--502--501--500  //f00208919 20130228
			512,	// i4G
            512    // i4B  //512---?514--512  //f00208919 20130228
		},
		// Preference gain: daylight fluorescent
		{
            506,    // i4R //512-->506
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
                -441,    // i4RotatedXCoordinate[0]
                -367,    // i4RotatedXCoordinate[1]
                -240,    // i4RotatedXCoordinate[2]
                -131,    // i4RotatedXCoordinate[3]
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

/*END PN:DTS2013053100362, Added by y00213338 , 2013-05-31*/
/*END PN:DTS2013062906537, Modified by w00167383 , 2013-06-29*/
