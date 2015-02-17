/*****************************************************************************
 *
 * Filename:
 * ---------
 *   A2030mipi_Sensor.h
 *
 * Project:
 * --------
 *   YUSU
 *
 * Description:
 * ------------
 *   Header file of Sensor driver
 *
 *
 * Author:
 * -------
 *   Guangye Yang (mtk70662)
 *
 *============================================================================
 *             HISTORY
 * Below this line, this part is controlled by CC/CQ. DO NOT MODIFY!!
 *------------------------------------------------------------------------------
 * $Revision:$
 * $Modtime:$
 * $Log:$
 *
 *
 *
 *------------------------------------------------------------------------------
 * Upper this line, this part is controlled by CC/CQ. DO NOT MODIFY!!
 *============================================================================
 ****************************************************************************/

#ifndef _A2030MIPI_SENSOR_H
#define _A2030MIPI_SENSOR_H

typedef enum group_enum {
    PRE_GAIN=0,
    CMMCLK_CURRENT,
    FRAME_RATE_LIMITATION,
    REGISTER_EDITOR,
    GROUP_TOTAL_NUMS
} FACTORY_GROUP_ENUM;


#define ENGINEER_START_ADDR 10
#define FACTORY_START_ADDR 0


typedef enum register_index
{
    SENSOR_BASEGAIN=FACTORY_START_ADDR,
    PRE_GAIN_R_INDEX,
    PRE_GAIN_Gr_INDEX,
    PRE_GAIN_Gb_INDEX,
    PRE_GAIN_B_INDEX,
    FACTORY_END_ADDR
} FACTORY_REGISTER_INDEX;


typedef enum engineer_index
{
    CMMCLK_CURRENT_INDEX=ENGINEER_START_ADDR,
    ENGINEER_END
} FACTORY_ENGINEER_INDEX;


typedef struct
{
    SENSOR_REG_STRUCT   Reg[ENGINEER_END];
    SENSOR_REG_STRUCT   CCT[FACTORY_END_ADDR];
} SENSOR_DATA_STRUCT, *PSENSOR_DATA_STRUCT;


//if define RAW10, MIPI_INTERFACE must be defined
//if MIPI_INTERFACE is marked, RAW10 must be marked too
#define MIPI_INTERFACE
#define RAW10

#define A2030MIPI_DATA_FORMAT SENSOR_OUTPUT_FORMAT_RAW_Gr


#define A2030MIPI_WRITE_ID_1    0x6C
#define A2030MIPI_READ_ID_1     0x6D
#define A2030MIPI_WRITE_ID_2    0x20
#define A2030MIPI_READ_ID_2     0x21


#define A2030MIPI_IMAGE_SENSOR_FULL_HACTIVE     (1600)//(1600-16)
#define A2030MIPI_IMAGE_SENSOR_FULL_VACTIVE     (1200)//(1200-12)
#define A2030MIPI_IMAGE_SENSOR_PV_HACTIVE       (1600)//(1600-16)
#define A2030MIPI_IMAGE_SENSOR_PV_VACTIVE       (1200)//(1200-12)


#define A2030MIPI_FULL_START_X                  (0)
#define A2030MIPI_FULL_START_Y                  (0)
#define A2030MIPI_IMAGE_SENSOR_FULL_WIDTH       (A2030MIPI_IMAGE_SENSOR_FULL_HACTIVE )  
#define A2030MIPI_IMAGE_SENSOR_FULL_HEIGHT      (A2030MIPI_IMAGE_SENSOR_FULL_VACTIVE )  

#define A2030MIPI_PV_START_X                    (0)
#define A2030MIPI_PV_START_Y                    (0)
#define A2030MIPI_IMAGE_SENSOR_PV_WIDTH         (A2030MIPI_IMAGE_SENSOR_PV_HACTIVE )    
#define A2030MIPI_IMAGE_SENSOR_PV_HEIGHT        (A2030MIPI_IMAGE_SENSOR_PV_VACTIVE )    


#define A2030MIPI_IMAGE_SENSOR_FULL_HBLANKING   752//752+16
#define A2030MIPI_IMAGE_SENSOR_FULL_VBLANKING       147//147+12


#define A2030MIPI_IMAGE_SENSOR_PV_HBLANKING     752//752+16
#define A2030MIPI_IMAGE_SENSOR_PV_VBLANKING         147//147+12


#define A2030MIPI_FULL_PERIOD_PIXEL_NUMS            (A2030MIPI_IMAGE_SENSOR_FULL_HACTIVE + A2030MIPI_IMAGE_SENSOR_FULL_HBLANKING)  //1600+752= 2352
#define A2030MIPI_FULL_PERIOD_LINE_NUMS             (A2030MIPI_IMAGE_SENSOR_FULL_VACTIVE + A2030MIPI_IMAGE_SENSOR_FULL_VBLANKING)  //1200+147 = 1347
#define A2030MIPI_PV_PERIOD_PIXEL_NUMS              (A2030MIPI_IMAGE_SENSOR_PV_HACTIVE + A2030MIPI_IMAGE_SENSOR_PV_HBLANKING)      //1600+752= 2352
#define A2030MIPI_PV_PERIOD_LINE_NUMS               (A2030MIPI_IMAGE_SENSOR_PV_VACTIVE + A2030MIPI_IMAGE_SENSOR_PV_VBLANKING)      //1200+147 = 1347


/* SENSOR PRIVATE STRUCT */
struct A2030MIPI_SENSOR_STRUCT
{
    kal_uint8 i2c_write_id;
    kal_uint8 i2c_read_id;
    kal_uint16 preview_vt_clk;
    kal_uint16 capture_vt_clk;	
	kal_bool video_mode;
	kal_bool NightMode;
	kal_uint16 FixedFps;	
	kal_uint16 frame_height;
	kal_uint16 line_length;  
};

#endif /* _A2030MIPI_SENSOR_H */

