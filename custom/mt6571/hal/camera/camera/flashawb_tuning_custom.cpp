/*
**
** Copyright 2008, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#include "camera_custom_nvram.h"
#include "camera_custom_types.h"
#include "flash_awb_param.h"
#include "flash_awb_tuning_custom.h"

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
isFlashAWBv2Enabled()
{
    return 1;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
FLASH_AWB_INIT_T const&
getFlashAWBParam()
{
    static FLASH_AWB_INIT_T rFlashAWBParam =
    {
       //flash awb tuning parameter
       {
            9,   //foreground percentage, 0~50
            95, //background percentage, 50~100

//Rule: map1_th1 < map1_th2 < map1_th3 < map1_th4
//Rule: map1_th1_val <= map1_th2_val <= map1_th3_val <= map1_th4_val
            100, // map1_th1, 100~1000
            200, // map1_th2, 100~1000
            300, // map1_th3, 100~1000
            400, // map1_th4, 100~1000
            100, // map1_th1_val, 100~1000
            100, // map1_th2_val, 100~1000
            100, // map1_th3_val, 100~1000
            100, // map1_th4_val, 100~1000

//Rule: map2_th1 < map2_th1 < map2_th3 < map2_th4
//Rule: map2_th1_val <= map2_th2_val <= map2_th3_val <= map2_th4_val 
            150, // map2_th1, 100~1000
            250, //map2_th2, 100~1000
            350, // map2_th3, 100~1000
            400, // map2_th4, 100~1000
            100, // map2_th1_val, 100~1000
            100, // map2_th2_val, 100~1000
            100, // map2_th3_val, 100~1000
            100, // map2_th4_val, 100~1000

//Rule: location_map_th1 < location_map_th2 < location_map_th3 <location_map_th4
//Rule: location_map_val1 <= location_map_val2 <= location_map_val3 <= location_map_val4
			10,//location_map_th1;
			10,//location_map_th2;
			10,//location_map_th3; 
			10,//location_map_th4;
			100, //location_map_val1;    //500 for Lab MP Criteria
			100, //location_map_val2;    //500 for Lab MP Criteria
			100, //location_map_val3;    //500 for Lab MP Criteria
			100, //location_map_val4;    //500 for Lab MP Criteria

            25 //min_flash_y, 0~255
       }
    };

    return (rFlashAWBParam);
}


FLASH_AWB_CALIBRATION_DATA_STRUCT const&
getFlashAWBCalibrationData()
{
    static FLASH_AWB_CALIBRATION_DATA_STRUCT rFlashAWBCalibrationData =
    {
        {
       	
//{0, 512, 0}, //duty=-1, step=0   
{884, 512, 961}, //duty=0, step=0  
{876, 512, 852}, //duty=1, step=0  
{876, 512, 852}, //duty=1, step=0  
{876, 512, 852}, //duty=1, step=0  
{876, 512, 852}, //duty=1, step=0  
{876, 512, 852}, //duty=1, step=0  
{876, 512, 852}, //duty=1, step=0  
{876, 512, 852}, //duty=1, step=0  
{876, 512, 852}, //duty=1, step=0  
{876, 512, 852}, //duty=1, step=0  
{876, 512, 852}, //duty=1, step=0  
{876, 512, 852}, //duty=1, step=0  
{876, 512, 852}, //duty=1, step=0  
{876, 512, 852}, //duty=1, step=0  
{876, 512, 852}, //duty=1, step=0  
{876, 512, 852}, //duty=1, step=0  
{876, 512, 852}, //duty=1, step=0  
{876, 512, 852}, //duty=1, step=0  
{876, 512, 852}, //duty=1, step=0  
{876, 512, 852}, //duty=1, step=0  
{876, 512, 852}, //duty=1, step=0  
{876, 512, 852}, //duty=1, step=0  
{876, 512, 852}, //duty=1, step=0  
{876, 512, 852}, //duty=1, step=0  
{876, 512, 852}, //duty=1, step=0  
{876, 512, 852}, //duty=1, step=0  
{876, 512, 852}, //duty=1, step=0  
{876, 512, 852}, //duty=1, step=0  
{876, 512, 852}, //duty=1, step=0  
{876, 512, 852}, //duty=1, step=0  
{876, 512, 852}, //duty=1, step=0  
{876, 512, 852}, //duty=1, step=0  
{876, 512, 852}, //duty=1, step=0  
{876, 512, 852}, //duty=1, step=0  
{876, 512, 852}, //duty=1, step=0  
{876, 512, 852}, //duty=1, step=0  
{876, 512, 852}, //duty=1, step=0  
{876, 512, 852}, //duty=1, step=0  
{876, 512, 852}, //duty=1, step=0  
{876, 512, 852}, //duty=1, step=0  
{876, 512, 852}, //duty=1, step=0  
{876, 512, 852}, //duty=1, step=0  
{876, 512, 852}, //duty=1, step=0  
{876, 512, 852}, //duty=1, step=0  
{876, 512, 852}, //duty=1, step=0  
{876, 512, 852}, //duty=1, step=0  
{876, 512, 852}, //duty=1, step=0  
{876, 512, 852}, //duty=1, step=0  
{876, 512, 852}, //duty=1, step=0  
{876, 512, 852}, //duty=1, step=0  
{876, 512, 852}, //duty=1, step=0  
{876, 512, 852}, //duty=1, step=0  
{876, 512, 852}, //duty=1, step=0  
{876, 512, 852}, //duty=1, step=0  
{876, 512, 852}, //duty=1, step=0  
{876, 512, 852}, //duty=1, step=0  
{876, 512, 852}, //duty=1, step=0  
{876, 512, 852}, //duty=1, step=0  
{876, 512, 852}, //duty=1, step=0  
{876, 512, 852}, //duty=1, step=0  
{876, 512, 852}, //duty=1, step=0  
{876, 512, 852}, //duty=1, step=0  
{876, 512, 852}, //duty=1, step=0  
{876, 512, 852}, //duty=1, step=0  

        }
    };

    return (rFlashAWBCalibrationData);
}


