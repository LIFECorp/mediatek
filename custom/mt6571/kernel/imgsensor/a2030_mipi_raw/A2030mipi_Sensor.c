/*****************************************************************************
 *
 * Filename:
 * ---------
 *   sensor.c
 *
 * Project:
 * --------
 *   YUSU
 *
 * Description:
 * ------------
 *   Source code of Sensor driver
 *
 *
 * Author:
 * -------
 *   
 *
 *============================================================================
 *             HISTORY
 * Below this line, this part is controlled by CC/CQ. DO NOT MODIFY!!
 *------------------------------------------------------------------------------
 *
 
 *------------------------------------------------------------------------------
 * Upper this line, this part is controlled by CC/CQ. DO NOT MODIFY!!
 *============================================================================
 ****************************************************************************/
#include <linux/videodev2.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <asm/atomic.h>
#include <linux/slab.h>


#include "kd_camera_hw.h"
#include "kd_imgsensor.h"
#include "kd_imgsensor_define.h"
#include "kd_imgsensor_errcode.h"

#include "A2030mipi_Sensor.h"
#include "A2030mipi_Camera_Sensor_para.h"
#include "A2030mipi_CameraCustomized.h"

#define A2030MIPI_DEBUG
#ifdef A2030MIPI_DEBUG
#define SENSORDB(fmt, arg...) printk("%s: " fmt "\n", __FUNCTION__ ,##arg)
#else
#define SENSORDB(x,...)
#endif

static DEFINE_SPINLOCK(A2030mipiraw_drv_lock);

#define AUTO_FLICKER_NO 10
kal_uint16 A2030_Frame_Length_preview = 0;

struct A2030MIPI_SENSOR_STRUCT A2030MIPI_sensor= 
{
    .i2c_write_id = 0x6c,
    .i2c_read_id  = 0x6d,
#ifdef MIPI_INTERFACE
    .preview_vt_clk = 950,//1040,
    .capture_vt_clk = 950,//1118,
#else
    .preview_vt_clk = 520,
    .capture_vt_clk = 520,
#endif
    .video_mode = KAL_TRUE,
    .NightMode = KAL_TRUE,
	.FixedFps = 30,
	.frame_height	= A2030MIPI_PV_PERIOD_PIXEL_NUMS,  // 2352
	.line_length	= A2030MIPI_PV_PERIOD_LINE_NUMS , //1347
};

kal_uint16 A2030MIPI_dummy_pixels=0, A2030MIPI_dummy_lines=0;
kal_uint16 A2030MIPI_PV_dummy_pixels=0, A2030MIPI_PV_dummy_lines=0;

kal_uint16 A2030MIPI_exposure_lines = 0x100;
kal_uint16 A2030MIPI_sensor_global_gain = BASEGAIN, A2030MIPI_sensor_gain_base = BASEGAIN;
kal_uint16 A2030MIPI_sensor_gain_array[2][5] = {{0x0204,0x0208, 0x0206, 0x020C, 0x020A},{0x08,0x8, 0x8, 0x8, 0x8}};


MSDK_SENSOR_CONFIG_STRUCT A2030MIPISensorConfigData;
kal_uint32 A2030MIPI_FAC_SENSOR_REG;

/* FIXME: old factors and DIDNOT use now. s*/
SENSOR_REG_STRUCT A2030MIPISensorCCT[FACTORY_END_ADDR]=CAMERA_SENSOR_CCT_DEFAULT_VALUE;
SENSOR_REG_STRUCT A2030MIPISensorReg[ENGINEER_END]=CAMERA_SENSOR_REG_DEFAULT_VALUE;
/* FIXME: old factors and DIDNOT use now. e*/
MSDK_SCENARIO_ID_ENUM A2030_CurrentScenarioId = MSDK_SCENARIO_ID_CAMERA_PREVIEW;

typedef enum
{
    A2030MIPI_MODE_INIT,
    A2030MIPI_MODE_PREVIEW,
    A2030MIPI_MODE_CAPTURE
} A2030MIPI_MODE;

A2030MIPI_MODE g_iA2030MIPI_Mode = A2030MIPI_MODE_PREVIEW;
kal_bool A2030MIPI_AutoFlicker_Mode=KAL_FALSE;
kal_uint8  A2030test_pattern_flag=0;
#define A2030_TEST_PATTERN_CHECKSUM (0xa3658b61)


static void A2030MIPI_SetDummy(kal_bool mode,const kal_uint16 iDummyPixels, const kal_uint16 iDummyLines);


extern int iReadRegI2C(u8 *a_pSendData , u16 a_sizeSendData, u8 * a_pRecvData, u16 a_sizeRecvData, u16 i2cId);
extern int iWriteRegI2C(u8 *a_pSendData , u16 a_sizeSendData, u16 i2cId);

kal_uint16 A2030MIPI_read_cmos_sensor(kal_uint32 addr)
{
    kal_uint16 get_byte=0;
    char puSendCmd[2] = {(char)(addr >> 8) , (char)(addr & 0xFF) };
    iReadRegI2C(puSendCmd , 2, (u8*)&get_byte, 2, A2030MIPI_sensor.i2c_write_id);
    return ((get_byte<<8)&0xff00)|((get_byte>>8)&0x00ff);
}


void A2030MIPI_write_cmos_sensor(kal_uint32 addr, kal_uint32 para)
{
    char puSendCmd[4] = {(char)(addr >> 8) , (char)(addr & 0xFF) ,(char)(para >> 8),(char)(para & 0xFF)};
    iWriteRegI2C(puSendCmd , 4,A2030MIPI_sensor.i2c_write_id);
}


kal_uint16 A2030MIPI_read_cmos_sensor_8(kal_uint32 addr)
{
    kal_uint16 get_byte=0;
    char puSendCmd[2] = {(char)(addr >> 8) , (char)(addr & 0xFF) };
    iReadRegI2C(puSendCmd , 2, (u8*)&get_byte,1,A2030MIPI_sensor.i2c_write_id);
    return get_byte;
}

void A2030MIPI_write_cmos_sensor_8(kal_uint32 addr, kal_uint32 para)
{
    char puSendCmd[4] = {(char)(addr >> 8) , (char)(addr & 0xFF) ,(char)(para & 0xFF)};
    iWriteRegI2C(puSendCmd , 3,A2030MIPI_sensor.i2c_write_id);
}


/*******************************************************************************
* 
********************************************************************************/
static kal_uint16 A2030reg2gain(kal_uint16 reg_gain)
{
    kal_uint16 gain;
    kal_uint16 collumn_gain, asc1_gain, initial_gain;
    kal_uint16 collumn_gain_shift = 10, asc1_gain_shift = 8;
    kal_uint16 collumn_gain_value = 1, asc1_gain_value = 1;

    collumn_gain = (reg_gain & (0x03 << collumn_gain_shift)) >> collumn_gain_shift;
    asc1_gain = (reg_gain & (0x03 << asc1_gain_shift)) >> asc1_gain_shift;
    initial_gain = reg_gain & 0x7F;

    if (collumn_gain == 0) {
        collumn_gain_value = 1;
    } else if (collumn_gain == 1) {
        collumn_gain_value = 3;
    } else if (collumn_gain == 2) {
        collumn_gain_value = 2;
    } else if (collumn_gain == 3) {
        collumn_gain_value = 4;
    }

    if (asc1_gain == 0) {
        asc1_gain_value = 1;
    } else if (asc1_gain == 1) {
        asc1_gain_value = 0xFFFF;
    } else if (asc1_gain == 2) {
        asc1_gain_value = 2;
    } else {
        // not exist
        SENSORDB("error gain setting");
    }

    if ( asc1_gain_value == 0xFFFF) {
        gain = BASEGAIN * initial_gain * collumn_gain_value * 4 / (32 * 3);
    } else {
        gain = BASEGAIN * initial_gain * collumn_gain_value * asc1_gain_value / (32);
    }

    return gain;
}


/*******************************************************************************
* 
********************************************************************************/
static kal_uint16 A2030gain2reg(kal_uint16 gain)
{
    kal_uint16 reg_gain;
    kal_uint16 collumn_gain = 0, asc1_gain = 0, initial_gain = 0;
    kal_uint16 collumn_gain_shift = 10, asc1_gain_shift = 8;

    if (gain < (4 * BASEGAIN) / 3) {
        collumn_gain = (0x00&0x03) << collumn_gain_shift;
        asc1_gain = (0x00&0x03) << asc1_gain_shift;
        initial_gain = (32 * gain / BASEGAIN) & 0x7F;
    } else if (gain < 2 * BASEGAIN) {
        collumn_gain = (0x00&0x03) << collumn_gain_shift;
        asc1_gain = (0x01&0x03) << asc1_gain_shift;
        initial_gain = (32 * gain * 3 / (BASEGAIN * 4)) & 0x7F;
    } else if (gain < (8 * BASEGAIN)/3 + 1) {
        collumn_gain = (0x02&0x03) << collumn_gain_shift;
        asc1_gain = (0x00&0x03) << asc1_gain_shift;
        initial_gain = (32 * gain / (BASEGAIN * 2)) & 0x7F;
    } else if (gain < 3 * BASEGAIN) {
        collumn_gain = (0x02&0x03) << collumn_gain_shift;
        asc1_gain = (0x01&0x03) << asc1_gain_shift;
        initial_gain = (32 * gain * 3 / (BASEGAIN * 2 * 4)) & 0x7F;
    } else if (gain < 4 * BASEGAIN) {
        collumn_gain = (0x01&0x03) << collumn_gain_shift;
        asc1_gain = (0x00&0x03) << asc1_gain_shift;
        initial_gain = (32 * gain / (BASEGAIN * 3)) & 0x7F;
    } else if (gain < (16 * BASEGAIN) / 3 + 1) {
        collumn_gain = (0x03&0x03) << collumn_gain_shift;
        asc1_gain = (0x00&0x03) << asc1_gain_shift;
        initial_gain = (32 * gain / (BASEGAIN * 4)) & 0x7F;
    } else if (gain < 8 * BASEGAIN) {
        collumn_gain = (0x03&0x03) << collumn_gain_shift;
        asc1_gain = (0x01&0x03) << asc1_gain_shift;
        initial_gain = (32 * gain * 3 / (BASEGAIN * 4 * 4)) & 0x7F;
    } else if (gain < 32 * BASEGAIN) {
        collumn_gain = (0x03&0x03) << collumn_gain_shift;
        asc1_gain = (0x02&0x03) << asc1_gain_shift;
        initial_gain = (32 * gain / (BASEGAIN * 4 * 2)) & 0x7F;
    } else {
        // not exist
        SENSORDB("error gain setting");
    }

    reg_gain = collumn_gain | asc1_gain | initial_gain;

    return reg_gain;
}


/*************************************************************************
* FUNCTION
*    read_A2030MIPI_gain
*
* DESCRIPTION
*    This function is to set global gain to sensor.
*
* PARAMETERS
*    None
*
* RETURNS
*    gain : sensor global gain(base: 0x40)
*
* GLOBALS AFFECTED
*
*************************************************************************/
kal_uint16 read_A2030MIPI_gain(void)
{
    /*
    // Aptina Gain Model
    kal_uint16 reg_gain=0, gain=0;
         
    reg_gain = A2030MIPI_read_cmos_sensor(0x305E);
    gain = A2030reg2gain(reg_gain); //change reg gain to mtk gain
    
    SENSORDB("read_A2030MIPI_gain: reg_gain =0x%x, gain = %d \n", reg_gain, gain);

    return gain;
    */
    
    volatile signed char i;
    kal_uint16 temp_reg=0, sensor_gain=0,temp_reg_base=0;
    
    temp_reg_base=A2030MIPISensorCCT[SENSOR_BASEGAIN].Para;

    for(i=0;i<4;i++)
    {
        temp_reg=A2030MIPISensorCCT[PRE_GAIN_R_INDEX+i].Para;

        if(temp_reg>=0x08 && temp_reg<=0x78)  // 0x78 is 15 by 8 ,means max gain is 15 multiple
            A2030MIPI_sensor_gain_array[1][PRE_GAIN_R_INDEX+i]=((((temp_reg*BASEGAIN)/8)*temp_reg_base)/8); //change to MTK basegain
        else if(temp_reg>0x78)
            SENSORDB("error gain setting");
    }

    sensor_gain=(temp_reg_base*BASEGAIN)/8;

    return sensor_gain;   //mtk gain unit
    
}

/*******************************************************************************
* 
********************************************************************************/
void write_A2030MIPI_gain(kal_uint16 gain)
{
    kal_uint16 reg_gain;
  
    A2030MIPI_write_cmos_sensor_8(0x0104, 0x01);        //parameter_hold
    if(gain >= BASEGAIN && gain <= 32*BASEGAIN)
    {   
        reg_gain = 8 * gain/BASEGAIN;        //change mtk gain base to aptina gain base
        A2030MIPI_write_cmos_sensor(0x0204,reg_gain);
        
        SENSORDB("reg_gain =%d, gain = %d", reg_gain, gain);
    }
    else
        SENSORDB("error gain setting");
    A2030MIPI_write_cmos_sensor_8(0x0104, 0x00);        //parameter_hold
    

    /*
    // Aptina Gain Model
    kal_uint16 reg_gain, reg;
    
    A2030MIPI_write_cmos_sensor_8(0x0104, 0x01);        //parameter_hold
    
    if(gain >= BASEGAIN && gain <= 32*BASEGAIN)
    {
        reg_gain = A2030gain2reg(gain);                //change mtk gain base to aptina gain base

        reg = A2030MIPI_read_cmos_sensor(0x305E);
        reg = (reg & 0xF000) | reg_gain;
        A2030MIPI_write_cmos_sensor(0x305E, reg);

        SENSORDB("reg =0x%x, gain = %d",reg, gain);
    }
    else
        SENSORDB("error gain setting");

    A2030MIPI_write_cmos_sensor_8(0x0104, 0x00);        //parameter_hold
    */
}


/*************************************************************************
* FUNCTION
* set_A2030MIPI_gain
*
* DESCRIPTION
* This function is to set global gain to sensor.
*
* PARAMETERS
* gain : sensor global gain(base: 0x40)
*
* RETURNS
* the actually gain set to sensor.
*
* GLOBALS AFFECTED
*
*************************************************************************/
kal_uint16 A2030MIPI_Set_gain(kal_uint16 gain)
{
    write_A2030MIPI_gain(gain);
}


/*************************************************************************
* FUNCTION
*   A2030MIPI_SetShutter
*
* DESCRIPTION
*   This function set e-shutter of A2030MIPI to change exposure time.
*
* PARAMETERS
*   shutter : exposured lines
*
* RETURNS
*   None
*
* GLOBALS AFFECTED
*
*************************************************************************/
void A2030MIPI_SetShutter(kal_uint16 iShutter)
{
    unsigned long flags;

    SENSORDB("iShutter: %u", iShutter);

    //spin_lock_irqsave(&A2030mipiraw_drv_lock, flags);
    //if(A2030MIPI_exposure_lines == iShutter){
      //  spin_unlock_irqrestore(&A2030mipiraw_drv_lock, flags);
      
      //SENSORDB("iShutter: Return !!!!!!\n");
        //return;
    //}
    //A2030MIPI_exposure_lines=iShutter;
    //spin_unlock_irqrestore(&A2030mipiraw_drv_lock, flags);
    
    A2030MIPI_write_cmos_sensor_8(0x0104, 0x01);    // GROUPED_PARAMETER_HOLD
    A2030MIPI_write_cmos_sensor(0x0202, iShutter);  /* course_integration_time */
    A2030MIPI_write_cmos_sensor_8(0x0104, 0x00);    // GROUPED_PARAMETER_HOLD
}


/*************************************************************************
* FUNCTION
*   A2030MIPI_read_shutter
*
* DESCRIPTION
*   This function to  Get exposure time.
*
* PARAMETERS
*   None
*
* RETURNS
*   shutter : exposured lines
*
* GLOBALS AFFECTED
*
*************************************************************************/
UINT16 A2030MIPI_read_shutter(void)
{
    kal_uint16 ishutter;
    
    ishutter = A2030MIPI_read_cmos_sensor(0x0202); /* course_integration_time */
    
    return ishutter;
}



UINT32 A2030MIPISetMaxFramerateByScenario(MSDK_SCENARIO_ID_ENUM scenarioId,UINT32 frameRate)
{
    kal_uint32 pclk;
    kal_int16 dummyLine;
    kal_uint16 lineLength,frameHeight;

    printk("A2030SetMaxFramerate: scenarioID = %d, frame rate = %d\n",scenarioId,frameRate);
    switch(scenarioId) {
        case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
            pclk = 104000000;  
            lineLength = A2030MIPI_PV_PERIOD_PIXEL_NUMS;  //3151
            frameHeight = (10*pclk)/frameRate/lineLength;
            dummyLine = frameHeight - A2030MIPI_PV_PERIOD_PIXEL_NUMS;
            A2030MIPI_SetDummy(0, 1855,dummyLine);
            break;
        case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
            pclk = 104000000;  
            lineLength = A2030MIPI_PV_PERIOD_PIXEL_NUMS;  // 3151
            frameHeight = (10*pclk)/frameRate/lineLength;
            dummyLine = frameHeight - A2030MIPI_PV_PERIOD_PIXEL_NUMS;
            A2030MIPI_SetDummy(0, 1855,dummyLine);
            break;  
        case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
            pclk = 111800000; 
            lineLength = A2030MIPI_FULL_PERIOD_PIXEL_NUMS;  //3694
            frameHeight = (10*pclk)/frameRate/lineLength;
            dummyLine = frameHeight - A2030MIPI_FULL_PERIOD_PIXEL_NUMS;
            A2030MIPI_SetDummy(0, 1102,dummyLine);
            break;  
        case MSDK_SCENARIO_ID_CAMERA_3D_PREVIEW:
            break;
        case MSDK_SCENARIO_ID_CAMERA_3D_VIDEO:
            break;
        case MSDK_SCENARIO_ID_CAMERA_3D_CAPTURE:
            break;  
            
        default:
            break;
            
        }
    return ERROR_NONE;
    
}

UINT32 A2030MIPIGetDefaultFramerateByScenario(MSDK_SCENARIO_ID_ENUM scenarioId,UINT32 *pframeRate)
{
    switch(scenarioId) {
        case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
        case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
            *pframeRate = 300;
            break;  
        case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
        case MSDK_SCENARIO_ID_CAMERA_ZSD:   
            *pframeRate = 150;
            break;  
        case MSDK_SCENARIO_ID_CAMERA_3D_PREVIEW:
        case MSDK_SCENARIO_ID_CAMERA_3D_VIDEO:
        case MSDK_SCENARIO_ID_CAMERA_3D_CAPTURE:
            *pframeRate = 300;
            break;  
            
        default:
            break;
            
        }
    return ERROR_NONE;
}


/*******************************************************************************
* 
********************************************************************************/
void A2030MIPI_camera_para_to_sensor(void)
{
    kal_uint32    i;

    
    for(i=0; 0xFFFFFFFF!=A2030MIPISensorReg[i].Addr; i++)
    {
        A2030MIPI_write_cmos_sensor(A2030MIPISensorReg[i].Addr, A2030MIPISensorReg[i].Para);
    }
    for(i=ENGINEER_START_ADDR; 0xFFFFFFFF!=A2030MIPISensorReg[i].Addr; i++)
    {
        A2030MIPI_write_cmos_sensor(A2030MIPISensorReg[i].Addr, A2030MIPISensorReg[i].Para);
    }
    for(i=FACTORY_START_ADDR; i<FACTORY_END_ADDR; i++)
    {
        A2030MIPI_write_cmos_sensor(A2030MIPISensorCCT[i].Addr, A2030MIPISensorCCT[i].Para);
    }
}


/*************************************************************************
* FUNCTION
*    A2030MIPI_sensor_to_camera_para
*
* DESCRIPTION
*    // update camera_para from sensor register
*
* PARAMETERS
*    None
*
* RETURNS
*    gain : sensor global gain(base: 0x40)
*
* GLOBALS AFFECTED
*
*************************************************************************/
void A2030MIPI_sensor_to_camera_para(void)
{
    kal_uint32    i;
    
    for(i=0; 0xFFFFFFFF!=A2030MIPISensorReg[i].Addr; i++)
    {
        spin_lock(&A2030mipiraw_drv_lock);
        A2030MIPISensorReg[i].Para = A2030MIPI_read_cmos_sensor(A2030MIPISensorReg[i].Addr);
        spin_unlock(&A2030mipiraw_drv_lock);        
    }
    for(i=ENGINEER_START_ADDR; 0xFFFFFFFF!=A2030MIPISensorReg[i].Addr; i++)
    {   
        spin_lock(&A2030mipiraw_drv_lock);
        A2030MIPISensorReg[i].Para = A2030MIPI_read_cmos_sensor(A2030MIPISensorReg[i].Addr);
        spin_unlock(&A2030mipiraw_drv_lock);
    }
}


/*************************************************************************
* FUNCTION
*    A2030MIPI_get_sensor_group_count
*
* DESCRIPTION
*    //
*
* PARAMETERS
*    None
*
* RETURNS
*    gain : sensor global gain(base: 0x40)
*
* GLOBALS AFFECTED
*
*************************************************************************/
kal_int32  A2030MIPI_get_sensor_group_count(void)
{
    return GROUP_TOTAL_NUMS;
}


void A2030MIPI_get_sensor_group_info(kal_uint16 group_idx, kal_int8* group_name_ptr, kal_int32* item_count_ptr)
{
    switch (group_idx)
    {
        case PRE_GAIN:
            sprintf((char *)group_name_ptr, "CCT");
            *item_count_ptr = 5;
            break;
        case CMMCLK_CURRENT:
            sprintf((char *)group_name_ptr, "CMMCLK Current");
            *item_count_ptr = 1;
            break;
        case FRAME_RATE_LIMITATION:
            sprintf((char *)group_name_ptr, "Frame Rate Limitation");
            *item_count_ptr = 2;
            break;
        case REGISTER_EDITOR:
            sprintf((char *)group_name_ptr, "Register Editor");
            *item_count_ptr = 2;
            break;
        default:
            ASSERT(0);
    }
}


void A2030MIPI_get_sensor_item_info(kal_uint16 group_idx,kal_uint16 item_idx, MSDK_SENSOR_ITEM_INFO_STRUCT* info_ptr)
{
    kal_int16 temp_reg=0;
    kal_uint16 temp_gain=0, temp_addr=0, temp_para=0;
    
    switch (group_idx)
    {
        case PRE_GAIN:
           switch (item_idx)
          {
              case 0:
                sprintf((char *)info_ptr->ItemNamePtr,"Pregain-R");
                  temp_addr = PRE_GAIN_R_INDEX;
              break;
              case 1:
                sprintf((char *)info_ptr->ItemNamePtr,"Pregain-Gr");
                  temp_addr = PRE_GAIN_Gr_INDEX;
              break;
              case 2:
                sprintf((char *)info_ptr->ItemNamePtr,"Pregain-Gb");
                  temp_addr = PRE_GAIN_Gb_INDEX;
              break;
              case 3:
                sprintf((char *)info_ptr->ItemNamePtr,"Pregain-B");
                  temp_addr = PRE_GAIN_B_INDEX;
              break;
              case 4:
                 sprintf((char *)info_ptr->ItemNamePtr,"SENSOR_BASEGAIN");
                 temp_addr = SENSOR_BASEGAIN;
              break;
              default:
                 ASSERT(0);
          }

            temp_para=A2030MIPISensorCCT[temp_addr].Para;

           if(temp_para>=0x08 && temp_para<=0x78)
                temp_gain=(temp_para*BASEGAIN)/8;
            else
                ASSERT(0);

            temp_gain=(temp_gain*1000)/BASEGAIN;

            info_ptr->ItemValue=temp_gain;
            info_ptr->IsTrueFalse=KAL_FALSE;
            info_ptr->IsReadOnly=KAL_FALSE;
            info_ptr->IsNeedRestart=KAL_FALSE;
            info_ptr->Min=1000;
            info_ptr->Max=15000;
            break;
        case CMMCLK_CURRENT:
            switch (item_idx)
            {
                case 0:
                    sprintf((char *)info_ptr->ItemNamePtr,"Drv Cur[2,4,6,8]mA");
                
                    //temp_reg=A2030MIPISensorReg[CMMCLK_CURRENT_INDEX].Para;
                    temp_reg = ISP_DRIVING_2MA;
                    if(temp_reg==ISP_DRIVING_2MA)
                    {
                        info_ptr->ItemValue=2;
                    }
                    else if(temp_reg==ISP_DRIVING_4MA)
                    {
                        info_ptr->ItemValue=4;
                    }
                    else if(temp_reg==ISP_DRIVING_6MA)
                    {
                        info_ptr->ItemValue=6;
                    }
                    else if(temp_reg==ISP_DRIVING_8MA)
                    {
                        info_ptr->ItemValue=8;
                    }
                
                    info_ptr->IsTrueFalse=KAL_FALSE;
                    info_ptr->IsReadOnly=KAL_FALSE;
                    info_ptr->IsNeedRestart=KAL_TRUE;
                    info_ptr->Min=2;
                    info_ptr->Max=8;
                    break;
                default:
                    ASSERT(0);
            }
            break;
        case FRAME_RATE_LIMITATION:
            switch (item_idx)
            {
                case 0:
                    sprintf((char *)info_ptr->ItemNamePtr,"Max Exposure Lines");
                    info_ptr->ItemValue=    111;  //A2030MIPI_MAX_EXPOSURE_LINES;
                    info_ptr->IsTrueFalse=KAL_FALSE;
                    info_ptr->IsReadOnly=KAL_TRUE;
                    info_ptr->IsNeedRestart=KAL_FALSE;
                    info_ptr->Min=0;
                    info_ptr->Max=0;
                    break;
                case 1:
                    sprintf((char *)info_ptr->ItemNamePtr,"Min Frame Rate");
                    info_ptr->ItemValue=12;
                    info_ptr->IsTrueFalse=KAL_FALSE;
                    info_ptr->IsReadOnly=KAL_TRUE;
                    info_ptr->IsNeedRestart=KAL_FALSE;
                    info_ptr->Min=0;
                    info_ptr->Max=0;
                    break;
                default:
                    ASSERT(0);
            }
            break;
        case REGISTER_EDITOR:
            switch (item_idx)
            {
                case 0:
                    sprintf((char *)info_ptr->ItemNamePtr,"REG Addr.");
                    info_ptr->ItemValue=0;
                    info_ptr->IsTrueFalse=KAL_FALSE;
                    info_ptr->IsReadOnly=KAL_FALSE;
                    info_ptr->IsNeedRestart=KAL_FALSE;
                    info_ptr->Min=0;
                    info_ptr->Max=0xFFFF;
                    break;
                case 1:
                    sprintf((char *)info_ptr->ItemNamePtr,"REG Value");
                    info_ptr->ItemValue=0;
                    info_ptr->IsTrueFalse=KAL_FALSE;
                    info_ptr->IsReadOnly=KAL_FALSE;
                    info_ptr->IsNeedRestart=KAL_FALSE;
                    info_ptr->Min=0;
                    info_ptr->Max=0xFFFF;
                    break;
                default:
                ASSERT(0);
            }
            break;
        default:
            ASSERT(0);
    }
}


kal_bool A2030MIPI_set_sensor_item_info(kal_uint16 group_idx, kal_uint16 item_idx, kal_int32 ItemValue)
{
//   kal_int16 temp_reg;
   kal_uint16  temp_gain=0,temp_addr=0, temp_para=0;

   switch (group_idx)
    {
        case PRE_GAIN:
            switch (item_idx)
            {
              case 0:
                temp_addr = PRE_GAIN_R_INDEX;
              break;
              case 1:
                temp_addr = PRE_GAIN_Gr_INDEX;
              break;
              case 2:
                temp_addr = PRE_GAIN_Gb_INDEX;
              break;
              case 3:
                temp_addr = PRE_GAIN_B_INDEX;
              break;
              case 4:
                temp_addr = SENSOR_BASEGAIN;
              break;
              default:
                 ASSERT(0);
          }

            temp_gain=((ItemValue*BASEGAIN+500)/1000);          //+500:get closed integer value

          if(temp_gain>=1*BASEGAIN && temp_gain<=15*BASEGAIN)
          {
             temp_para=(temp_gain*8+BASEGAIN/2)/BASEGAIN;
          }          
          else
              ASSERT(0);
            spin_lock(&A2030mipiraw_drv_lock);
            A2030MIPISensorCCT[temp_addr].Para = temp_para;
            spin_unlock(&A2030mipiraw_drv_lock);

            A2030MIPI_write_cmos_sensor(A2030MIPISensorCCT[temp_addr].Addr,temp_para);

            spin_lock(&A2030mipiraw_drv_lock);
            A2030MIPI_sensor_gain_base=read_A2030MIPI_gain();
            spin_unlock(&A2030mipiraw_drv_lock);

            break;
        case CMMCLK_CURRENT:
            switch (item_idx)
            {
                case 0:
                    //no need to apply this item for driving current
                    break;
                default:
                    ASSERT(0);
            }
            break;
        case FRAME_RATE_LIMITATION:
            ASSERT(0);
            break;
        case REGISTER_EDITOR:
            switch (item_idx)
            {
                case 0:
                    spin_lock(&A2030mipiraw_drv_lock);
                    A2030MIPI_FAC_SENSOR_REG=ItemValue;
                    spin_unlock(&A2030mipiraw_drv_lock);
                    break;
                case 1:
                    A2030MIPI_write_cmos_sensor(A2030MIPI_FAC_SENSOR_REG,ItemValue);
                    break;
                default:
                    ASSERT(0);
            }
            break;
        default:
            ASSERT(0);
    }
   
    return KAL_TRUE;
}


/*******************************************************************************
*
********************************************************************************/
static void A2030MIPI_Init_setting(void)
{
    kal_uint16 status = 0;
    
    SENSORDB( "Enter!");
    
    A2030MIPI_write_cmos_sensor_8(0x0103, 0x01);    //SOFTWARE_RESET (clears itself)
    mDELAY(300);      //Initialization Time
    
    //[Demo Initialization 1296 x 972 MCLK= 26MHz, PCLK=104MHz]
    //stop_streaming
	A2030MIPI_write_cmos_sensor_8(0x0100,0x00  ); // MODE_SELECT				 
	A2030MIPI_write_cmos_sensor(0x301A,0x4110); // RESET_REGISTER			 
	A2030MIPI_write_cmos_sensor(0x3064,0x0805); // SMIA_TEST				 
	A2030MIPI_write_cmos_sensor(0x31BE,0xC007); // MIPI_CONFIG_STATUS		 
	A2030MIPI_write_cmos_sensor(0x3E00,0x0430); // DYNAMIC_SEQRAM_00		 
	A2030MIPI_write_cmos_sensor(0x3E02,0x3FFF); // DYNAMIC_SEQRAM_02		 
	A2030MIPI_write_cmos_sensor(0x3E1E,0x67CA); // DYNAMIC_SEQRAM_1E		 
	A2030MIPI_write_cmos_sensor(0x3E2A,0xCA67); // DYNAMIC_SEQRAM_2A		 
	A2030MIPI_write_cmos_sensor(0x3E2E,0x8054); // DYNAMIC_SEQRAM_2E		 
	A2030MIPI_write_cmos_sensor(0x3E30,0x8255); // DYNAMIC_SEQRAM_30		 
	A2030MIPI_write_cmos_sensor(0x3E32,0x8410); // DYNAMIC_SEQRAM_32		 
	A2030MIPI_write_cmos_sensor(0x3E36,0x5FB0); // DYNAMIC_SEQRAM_36		 
	A2030MIPI_write_cmos_sensor(0x3E38,0x4C82); // DYNAMIC_SEQRAM_38		 
	A2030MIPI_write_cmos_sensor(0x3E3A,0x4DB0); // DYNAMIC_SEQRAM_3A		 
	A2030MIPI_write_cmos_sensor(0x3E3C,0x5F82); // DYNAMIC_SEQRAM_3C		 
	A2030MIPI_write_cmos_sensor(0x3E3E,0x1170); // DYNAMIC_SEQRAM_3E		 
	A2030MIPI_write_cmos_sensor(0x3E40,0x8055); // DYNAMIC_SEQRAM_40		 
	A2030MIPI_write_cmos_sensor(0x3E42,0x8061); // DYNAMIC_SEQRAM_42		 
	A2030MIPI_write_cmos_sensor(0x3E44,0x68D8); // DYNAMIC_SEQRAM_44		 
	A2030MIPI_write_cmos_sensor(0x3E46,0x6882); // DYNAMIC_SEQRAM_46		 
	A2030MIPI_write_cmos_sensor(0x3E48,0x6182); // DYNAMIC_SEQRAM_48		 
	A2030MIPI_write_cmos_sensor(0x3E4A,0x4D82); // DYNAMIC_SEQRAM_4A		 
	A2030MIPI_write_cmos_sensor(0x3E4C,0x4C82); // DYNAMIC_SEQRAM_4C		 
	A2030MIPI_write_cmos_sensor(0x3E4E,0x6368); // DYNAMIC_SEQRAM_4E		 
	A2030MIPI_write_cmos_sensor(0x3E50,0xD868); // DYNAMIC_SEQRAM_50		 
	A2030MIPI_write_cmos_sensor(0x3E52,0x8263); // DYNAMIC_SEQRAM_52		 
	A2030MIPI_write_cmos_sensor(0x3E54,0x824D); // DYNAMIC_SEQRAM_54		 
	A2030MIPI_write_cmos_sensor(0x3E56,0x8203); // DYNAMIC_SEQRAM_56		 
	A2030MIPI_write_cmos_sensor(0x3E58,0x9D66); // DYNAMIC_SEQRAM_58		 
	A2030MIPI_write_cmos_sensor(0x3E5A,0x8045); // DYNAMIC_SEQRAM_5A		 
	A2030MIPI_write_cmos_sensor(0x3E5C,0x4E7C); // DYNAMIC_SEQRAM_5C		 
	A2030MIPI_write_cmos_sensor(0x3E5E,0x0970); // DYNAMIC_SEQRAM_5E		 
	A2030MIPI_write_cmos_sensor(0x3E60,0x8072); // DYNAMIC_SEQRAM_60		 
	A2030MIPI_write_cmos_sensor(0x3E62,0x5484); // DYNAMIC_SEQRAM_62		 
	A2030MIPI_write_cmos_sensor(0x3E64,0x2037); // DYNAMIC_SEQRAM_64		 
	A2030MIPI_write_cmos_sensor(0x3E66,0x8216); // DYNAMIC_SEQRAM_66		 
	A2030MIPI_write_cmos_sensor(0x3E68,0x0486); // DYNAMIC_SEQRAM_68		 
	A2030MIPI_write_cmos_sensor(0x3E6A,0x1070); // DYNAMIC_SEQRAM_6A		 
	A2030MIPI_write_cmos_sensor(0x3E6C,0x825E); // DYNAMIC_SEQRAM_6C		 
	A2030MIPI_write_cmos_sensor(0x3E6E,0xEE54); // DYNAMIC_SEQRAM_6E		 
	A2030MIPI_write_cmos_sensor(0x3E70,0x825E); // DYNAMIC_SEQRAM_70		 
	A2030MIPI_write_cmos_sensor(0x3E72,0x8212); // DYNAMIC_SEQRAM_72		 
	A2030MIPI_write_cmos_sensor(0x3E74,0x7086); // DYNAMIC_SEQRAM_74		 
	A2030MIPI_write_cmos_sensor(0x3E76,0x1404); // DYNAMIC_SEQRAM_76		 
	A2030MIPI_write_cmos_sensor(0x3E78,0x8220); // DYNAMIC_SEQRAM_78		 
	A2030MIPI_write_cmos_sensor(0x3E7A,0x377C); // DYNAMIC_SEQRAM_7A		 
	A2030MIPI_write_cmos_sensor(0x3E7C,0x6170); // DYNAMIC_SEQRAM_7C		 
	A2030MIPI_write_cmos_sensor(0x3E7E,0x8082); // DYNAMIC_SEQRAM_7E		 
	A2030MIPI_write_cmos_sensor(0x3E80,0x4F82); // DYNAMIC_SEQRAM_80		 
	A2030MIPI_write_cmos_sensor(0x3E82,0x4E82); // DYNAMIC_SEQRAM_82		 
	A2030MIPI_write_cmos_sensor(0x3E84,0x5FCA); // DYNAMIC_SEQRAM_84		 
	A2030MIPI_write_cmos_sensor(0x3E86,0x5F82); // DYNAMIC_SEQRAM_86		 
	A2030MIPI_write_cmos_sensor(0x3E88,0x4E82); // DYNAMIC_SEQRAM_88		 
	A2030MIPI_write_cmos_sensor(0x3E8A,0x4F81); // DYNAMIC_SEQRAM_8A		 
	A2030MIPI_write_cmos_sensor(0x3E8C,0x7C7F); // DYNAMIC_SEQRAM_8C		 
	A2030MIPI_write_cmos_sensor(0x3E8E,0x7000); // DYNAMIC_SEQRAM_8E		 
	A2030MIPI_write_cmos_sensor(0x30D4,0xE200); // COLUMN_CORRECTION		 
	A2030MIPI_write_cmos_sensor(0x3174,0x8000); // ANALOG_CONTROL3			 
	A2030MIPI_write_cmos_sensor(0x3EE0,0x0020); // DAC_LD_20_21 			 
	A2030MIPI_write_cmos_sensor(0x3EE2,0x0016); // DAC_LD_22_23 			 
	A2030MIPI_write_cmos_sensor(0x3F00,0x0002); // BM_T0					 
	A2030MIPI_write_cmos_sensor(0x3F02,0x0028); // BM_T1					 
	A2030MIPI_write_cmos_sensor(0x3F0A,0x0300); // NOISE_FLOOR10			 
	A2030MIPI_write_cmos_sensor(0x3F0C,0x1008); // NOISE_FLOOR32			 
	A2030MIPI_write_cmos_sensor(0x3F10,0x0405); // SINGLE_K_FACTOR0 		 
	A2030MIPI_write_cmos_sensor(0x3F12,0x0101); // SINGLE_K_FACTOR1 		 
	A2030MIPI_write_cmos_sensor(0x3F14,0x0000); // SINGLE_K_FACTOR2 		 
	A2030MIPI_write_cmos_sensor(0x1140,0x0093); // MIN_FRAME_LENGTH_LINES	 
	A2030MIPI_write_cmos_sensor(0x114A,0x0093); // MIN_FRAME_BLANKING_LINES  
	A2030MIPI_write_cmos_sensor(0x3600,0x0130); // P_GR_P0Q0				 
	A2030MIPI_write_cmos_sensor(0x3602,0x618B); // P_GR_P0Q1				 
	A2030MIPI_write_cmos_sensor(0x3604,0x12B2); // P_GR_P0Q2				 
	A2030MIPI_write_cmos_sensor(0x3606,0x810F); // P_GR_P0Q3				 
	A2030MIPI_write_cmos_sensor(0x3608,0xC152); // P_GR_P0Q4				 
	A2030MIPI_write_cmos_sensor(0x360A,0x00D0); // P_RD_P0Q0				 
	A2030MIPI_write_cmos_sensor(0x360C,0x1D6D); // P_RD_P0Q1				 
	A2030MIPI_write_cmos_sensor(0x360E,0x5FB2); // P_RD_P0Q2				 
	A2030MIPI_write_cmos_sensor(0x3610,0xEBD0); // P_RD_P0Q3				 
	A2030MIPI_write_cmos_sensor(0x3612,0xB3B3); // P_RD_P0Q4				 
	A2030MIPI_write_cmos_sensor(0x3614,0x0190); // P_BL_P0Q0				 
	A2030MIPI_write_cmos_sensor(0x3616,0xD98C); // P_BL_P0Q1				 
	A2030MIPI_write_cmos_sensor(0x3618,0x0891); // P_BL_P0Q2				 
	A2030MIPI_write_cmos_sensor(0x361A,0xBC8C); // P_BL_P0Q3				 
	A2030MIPI_write_cmos_sensor(0x361C,0x9151); // P_BL_P0Q4				 
	A2030MIPI_write_cmos_sensor(0x361E,0x02D0); // P_GB_P0Q0				 
	A2030MIPI_write_cmos_sensor(0x3620,0x30CC); // P_GB_P0Q1				 
	A2030MIPI_write_cmos_sensor(0x3622,0x3B72); // P_GB_P0Q2				 
	A2030MIPI_write_cmos_sensor(0x3624,0xDBF0); // P_GB_P0Q3				 
	A2030MIPI_write_cmos_sensor(0x3626,0xA7D3); // P_GB_P0Q4				 
	A2030MIPI_write_cmos_sensor(0x3640,0xF048); // P_GR_P1Q0				 
	A2030MIPI_write_cmos_sensor(0x3642,0x024D); // P_GR_P1Q1				 
	A2030MIPI_write_cmos_sensor(0x3644,0xCCCE); // P_GR_P1Q2				 
	A2030MIPI_write_cmos_sensor(0x3646,0x908A); // P_GR_P1Q3				 
	A2030MIPI_write_cmos_sensor(0x3648,0x7A10); // P_GR_P1Q4				 
	A2030MIPI_write_cmos_sensor(0x364A,0x444A); // P_RD_P1Q0				 
	A2030MIPI_write_cmos_sensor(0x364C,0x0FCE); // P_RD_P1Q1				 
	A2030MIPI_write_cmos_sensor(0x364E,0x0B49); // P_RD_P1Q2				 
	A2030MIPI_write_cmos_sensor(0x3650,0x2D4F); // P_RD_P1Q3				 
	A2030MIPI_write_cmos_sensor(0x3652,0x220F); // P_RD_P1Q4				 
	A2030MIPI_write_cmos_sensor(0x3654,0x224C); // P_BL_P1Q0				 
	A2030MIPI_write_cmos_sensor(0x3656,0x4A4D); // P_BL_P1Q1				 
	A2030MIPI_write_cmos_sensor(0x3658,0x92EE); // P_BL_P1Q2				 
	A2030MIPI_write_cmos_sensor(0x365A,0x20AF); // P_BL_P1Q3				 
	A2030MIPI_write_cmos_sensor(0x365C,0x3F10); // P_BL_P1Q4				 
	A2030MIPI_write_cmos_sensor(0x365E,0x41AC); // P_GB_P1Q0				 
	A2030MIPI_write_cmos_sensor(0x3660,0x30AD); // P_GB_P1Q1				 
	A2030MIPI_write_cmos_sensor(0x3662,0x54AE); // P_GB_P1Q2				 
	A2030MIPI_write_cmos_sensor(0x3664,0xE0AD); // P_GB_P1Q3				 
	A2030MIPI_write_cmos_sensor(0x3666,0xE030); // P_GB_P1Q4				 
	A2030MIPI_write_cmos_sensor(0x3680,0x20B2); // P_GR_P2Q0				 
	A2030MIPI_write_cmos_sensor(0x3682,0xB6AF); // P_GR_P2Q1				 
	A2030MIPI_write_cmos_sensor(0x3684,0x6EF2); // P_GR_P2Q2				 
	A2030MIPI_write_cmos_sensor(0x3686,0xFAF3); // P_GR_P2Q3				 
	A2030MIPI_write_cmos_sensor(0x3688,0x9AF7); // P_GR_P2Q4				 
	A2030MIPI_write_cmos_sensor(0x368A,0x5632); // P_RD_P2Q0				 
	A2030MIPI_write_cmos_sensor(0x368C,0xF50F); // P_RD_P2Q1				 
	A2030MIPI_write_cmos_sensor(0x368E,0x51D3); // P_RD_P2Q2				 
	A2030MIPI_write_cmos_sensor(0x3690,0x8874); // P_RD_P2Q3				 
	A2030MIPI_write_cmos_sensor(0x3692,0xDF97); // P_RD_P2Q4				 
	A2030MIPI_write_cmos_sensor(0x3694,0x3931); // P_BL_P2Q0				 
	A2030MIPI_write_cmos_sensor(0x3696,0x83CF); // P_BL_P2Q1				 
	A2030MIPI_write_cmos_sensor(0x3698,0x3292); // P_BL_P2Q2				 
	A2030MIPI_write_cmos_sensor(0x369A,0xC452); // P_BL_P2Q3				 
	A2030MIPI_write_cmos_sensor(0x369C,0xB4D6); // P_BL_P2Q4				 
	A2030MIPI_write_cmos_sensor(0x369E,0x1F92); // P_GB_P2Q0				 
	A2030MIPI_write_cmos_sensor(0x36A0,0xA350); // P_GB_P2Q1				 
	A2030MIPI_write_cmos_sensor(0x36A2,0x3FF1); // P_GB_P2Q2				 
	A2030MIPI_write_cmos_sensor(0x36A4,0xD472); // P_GB_P2Q3				 
	A2030MIPI_write_cmos_sensor(0x36A6,0x8B17); // P_GB_P2Q4				 
	A2030MIPI_write_cmos_sensor(0x36C0,0x11CF); // P_GR_P3Q0				 
	A2030MIPI_write_cmos_sensor(0x36C2,0x1090); // P_GR_P3Q1				 
	A2030MIPI_write_cmos_sensor(0x36C4,0x0CF2); // P_GR_P3Q2				 
	A2030MIPI_write_cmos_sensor(0x36C6,0x8E51); // P_GR_P3Q3				 
	A2030MIPI_write_cmos_sensor(0x36C8,0x9873); // P_GR_P3Q4				 
	A2030MIPI_write_cmos_sensor(0x36CA,0x4050); // P_RD_P3Q0				 
	A2030MIPI_write_cmos_sensor(0x36CC,0x566F); // P_RD_P3Q1				 
	A2030MIPI_write_cmos_sensor(0x36CE,0x8752); // P_RD_P3Q2				 
	A2030MIPI_write_cmos_sensor(0x36D0,0xCC13); // P_RD_P3Q3				 
	A2030MIPI_write_cmos_sensor(0x36D2,0xAC4D); // P_RD_P3Q4				 
	A2030MIPI_write_cmos_sensor(0x36D4,0x9E0E); // P_BL_P3Q0				 
	A2030MIPI_write_cmos_sensor(0x36D6,0x1010); // P_BL_P3Q1				 
	A2030MIPI_write_cmos_sensor(0x36D8,0x5F12); // P_BL_P3Q2				 
	A2030MIPI_write_cmos_sensor(0x36DA,0xF673); // P_BL_P3Q3				 
	A2030MIPI_write_cmos_sensor(0x36DC,0xCA15); // P_BL_P3Q4				 
	A2030MIPI_write_cmos_sensor(0x36DE,0xF64C); // P_GB_P3Q0				 
	A2030MIPI_write_cmos_sensor(0x36E0,0x2110); // P_GB_P3Q1				 
	A2030MIPI_write_cmos_sensor(0x36E2,0x2231); // P_GB_P3Q2				 
	A2030MIPI_write_cmos_sensor(0x36E4,0xBD72); // P_GB_P3Q3				 
	A2030MIPI_write_cmos_sensor(0x36E6,0x8F35); // P_GB_P3Q4				 
	A2030MIPI_write_cmos_sensor(0x3700,0xC452); // P_GR_P4Q0				 
	A2030MIPI_write_cmos_sensor(0x3702,0xA913); // P_GR_P4Q1				 
	A2030MIPI_write_cmos_sensor(0x3704,0xE677); // P_GR_P4Q2				 
	A2030MIPI_write_cmos_sensor(0x3706,0x1A16); // P_GR_P4Q3				 
	A2030MIPI_write_cmos_sensor(0x3708,0x17B9); // P_GR_P4Q4				 
	A2030MIPI_write_cmos_sensor(0x370A,0xCA71); // P_RD_P4Q0				 
	A2030MIPI_write_cmos_sensor(0x370C,0xC093); // P_RD_P4Q1				 
	A2030MIPI_write_cmos_sensor(0x370E,0xA9B8); // P_RD_P4Q2				 
	A2030MIPI_write_cmos_sensor(0x3710,0x1AD6); // P_RD_P4Q3				 
	A2030MIPI_write_cmos_sensor(0x3712,0x3B39); // P_RD_P4Q4				 
	A2030MIPI_write_cmos_sensor(0x3714,0x8B72); // P_BL_P4Q0				 
	A2030MIPI_write_cmos_sensor(0x3716,0xF5F1); // P_BL_P4Q1				 
	A2030MIPI_write_cmos_sensor(0x3718,0x99D7); // P_BL_P4Q2				 
	A2030MIPI_write_cmos_sensor(0x371A,0x1B35); // P_BL_P4Q3				 
	A2030MIPI_write_cmos_sensor(0x371C,0x0F79); // P_BL_P4Q4				 
	A2030MIPI_write_cmos_sensor(0x371E,0xDD92); // P_GB_P4Q0				 
	A2030MIPI_write_cmos_sensor(0x3720,0xA952); // P_GB_P4Q1				 
	A2030MIPI_write_cmos_sensor(0x3722,0xCA17); // P_GB_P4Q2				 
	A2030MIPI_write_cmos_sensor(0x3724,0x2775); // P_GB_P4Q3				 
	A2030MIPI_write_cmos_sensor(0x3726,0x3DF8); // P_GB_P4Q4				 
	A2030MIPI_write_cmos_sensor(0x3782,0x0360); // POLY_ORIGIN_C			 
	A2030MIPI_write_cmos_sensor(0x3784,0x0258); // POLY_ORIGIN_R			 
	A2030MIPI_write_cmos_sensor(0x3780,0x8000); // POLY_SC_ENABLE			 
	A2030MIPI_write_cmos_sensor(0x0112,0x0A0A); // CCP_DATA_FORMAT			 
	A2030MIPI_write_cmos_sensor(0x0300,0x0008); // VT_PIX_CLK_DIV			 
	A2030MIPI_write_cmos_sensor(0x0302,0x0001); // VT_SYS_CLK_DIV			 
	A2030MIPI_write_cmos_sensor(0x0304,0x0003); // PRE_PLL_CLK_DIV			 
	A2030MIPI_write_cmos_sensor(0x0306,0x005F); // PLL_MULTIPLIER			 
	A2030MIPI_write_cmos_sensor(0x0308,0x000A); // OP_PIX_CLK_DIV			 
	A2030MIPI_write_cmos_sensor(0x030A,0x0001); // OP_SYS_CLK_DIV	
	mDELAY(50); 
	A2030MIPI_write_cmos_sensor(0x31B4,0x0D66); // MIPI_TIMING_0			 
	A2030MIPI_write_cmos_sensor(0x31B6,0x0918); // MIPI_TIMING_1			 
	A2030MIPI_write_cmos_sensor(0x31B8,0x010C); // MIPI_TIMING_2			 
	A2030MIPI_write_cmos_sensor(0x31BA,0x050A); // MIPI_TIMING_3			 
	A2030MIPI_write_cmos_sensor(0x31BC,0x0A08); // MIPI_TIMING_4			 
	A2030MIPI_write_cmos_sensor_8(0x0104,0x01  );// GROUPED_PARAMETER_HOLD    
	A2030MIPI_write_cmos_sensor(0x0200,0x01E5); // FINE_INTEGRATION_TIME	 
	A2030MIPI_write_cmos_sensor(0x0202,0x0543); // COARSE_INTEGRATION_TIME	 
	A2030MIPI_write_cmos_sensor(0x3010,0x0094); // FINE_CORRECTION			 
	A2030MIPI_write_cmos_sensor(0x3040,0x0041); // READ_MODE				 
	A2030MIPI_write_cmos_sensor(0x0340,0x0543); // FRAME_LENGTH_LINES		 
	A2030MIPI_write_cmos_sensor(0x0342,0x0930); // LINE_LENGTH_PCK			 
	A2030MIPI_write_cmos_sensor(0x0344,0x0004); // X_ADDR_START 			 
	A2030MIPI_write_cmos_sensor(0x0346,0x0004); // Y_ADDR_START 			 
	A2030MIPI_write_cmos_sensor(0x0348,0x0643); // X_ADDR_END				 
	A2030MIPI_write_cmos_sensor(0x034A,0x04B3); // Y_ADDR_END				 
	A2030MIPI_write_cmos_sensor(0x034C,0x0640); // X_OUTPUT_SIZE			 
	A2030MIPI_write_cmos_sensor(0x034E,0x04B0); // Y_OUTPUT_SIZE			 
	A2030MIPI_write_cmos_sensor(0x0400,0x0000); // SCALING_MODE 			 
	A2030MIPI_write_cmos_sensor(0x0404,0x0010); // SCALE_M					 
	A2030MIPI_write_cmos_sensor(0x305E,0x102C); // GLOBAL_GAIN				 
	A2030MIPI_write_cmos_sensor(0x301A,0x4118); // RESET_REGISTER			 
	A2030MIPI_write_cmos_sensor_8(0x0104,0x00  ); // GROUPED_PARAMETER_HOLD	 
	A2030MIPI_write_cmos_sensor_8(0x0100,0x01  ); // MODE_SELECT				 

    mDELAY(50);              // Allow PLL to lock
}   /*  A2030MIPI_Sensor_Init  */


/*************************************************************************
* FUNCTION
*   A2030MIPIGetSensorID
*
* DESCRIPTION
*   This function get the sensor ID 
*
* PARAMETERS
*   *sensorID : return the sensor ID 
*
* RETURNS
*   None
*
* GLOBALS AFFECTED
*
*************************************************************************/
UINT32 A2030MIPIGetSensorID(UINT32 *sensorID) 
{
    const kal_uint8 i2c_addr[] = {A2030MIPI_WRITE_ID_1, A2030MIPI_WRITE_ID_2}; 
    kal_uint16 sensor_id = 0xFFFF;
    kal_uint16 i;

    // A2030 may have different i2c address
    for(i = 0; i < sizeof(i2c_addr) / sizeof(i2c_addr[0]); i++)
    {
        spin_lock(&A2030mipiraw_drv_lock);
        A2030MIPI_sensor.i2c_write_id = i2c_addr[i];
        spin_unlock(&A2030mipiraw_drv_lock);

        SENSORDB( "i2c address is %x ", A2030MIPI_sensor.i2c_write_id);
        
        sensor_id = A2030MIPI_read_cmos_sensor(0x0000);
        if(sensor_id == A2030MIPI_SENSOR_ID)
            break;
    }

    *sensorID  = sensor_id;
        
    SENSORDB("sensor_id is %x ", *sensorID );
 
    if (*sensorID != A2030MIPI_SENSOR_ID) {
        *sensorID = 0xFFFFFFFF; 
        return ERROR_SENSOR_CONNECT_FAIL;
    }
    
    return ERROR_NONE;
}


/*************************************************************************
* FUNCTION
*   A2030MIPIOpen
*
* DESCRIPTION
*   This function initialize the registers of CMOS sensor
*
* PARAMETERS
*   None
*
* RETURNS
*   None
*
* GLOBALS AFFECTED
*
*************************************************************************/
UINT32 A2030MIPIOpen(void)
{
    kal_uint16 sensor_id = 0;

    A2030MIPIGetSensorID(&sensor_id);
    
    SENSORDB("sensor_id is %x ", sensor_id);
    
    if (sensor_id != A2030MIPI_SENSOR_ID){
        return ERROR_SENSOR_CONNECT_FAIL;
    }

    A2030MIPI_Init_setting();

    spin_lock(&A2030mipiraw_drv_lock);
    A2030MIPI_sensor_gain_base = read_A2030MIPI_gain();
    g_iA2030MIPI_Mode = A2030MIPI_MODE_INIT;
    spin_unlock(&A2030mipiraw_drv_lock);
    
    return ERROR_NONE;
}


/*************************************************************************
* FUNCTION
*   A2030MIPI_night_mode
*
* DESCRIPTION
*   This function night mode of A2030MIPI.
*
* PARAMETERS
*   none
*
* RETURNS
*   None
*
* GLOBALS AFFECTED
*
*************************************************************************/
void A2030MIPI_NightMode(kal_bool bEnable)
{
    // frame rate will be control by AE table
    
}


/*************************************************************************
* FUNCTION
*   A2030MIPIClose
*
* DESCRIPTION
*   This function is to turn off sensor module power.
*
* PARAMETERS
*   None
*
* RETURNS
*   None
*
* GLOBALS AFFECTED
*
*************************************************************************/
UINT32 A2030MIPIClose(void)
{
    return ERROR_NONE;
}   /* A2030MIPIClose() */


void A2030MIPI_Set_Mirror_Flip(kal_uint8 image_mirror)
{
    SENSORDB("image_mirror = %d", image_mirror);
    
    switch (image_mirror)
    {
        case IMAGE_NORMAL:
            A2030MIPI_write_cmos_sensor_8(0x0101,0x00);
        break;
        case IMAGE_H_MIRROR:
            A2030MIPI_write_cmos_sensor_8(0x0101,0x01);
        break;
        case IMAGE_V_MIRROR:
            A2030MIPI_write_cmos_sensor_8(0x0101,0x02);
        break;
        case IMAGE_HV_MIRROR:
            A2030MIPI_write_cmos_sensor_8(0x0101,0x03);
        break;
    }
}


/*************************************************************************
* FUNCTION
*   A2030MIPI_SetDummy
*
* DESCRIPTION
*   This function initialize the registers of CMOS sensor
*
* PARAMETERS
*   mode  ture : preview mode
*             false : capture mode
*
* RETURNS
*   None
*
* GLOBALS AFFECTED
*
*************************************************************************/
static void A2030MIPI_SetDummy(kal_bool mode,const kal_uint16 iDummyPixels, const kal_uint16 iDummyLines)
{
    kal_uint16 Line_length, Frame_length;
    
    if(mode == KAL_TRUE) //preview
    {
        Line_length   = A2030MIPI_PV_PERIOD_PIXEL_NUMS + iDummyPixels;
        Frame_length = A2030MIPI_PV_PERIOD_LINE_NUMS  + iDummyLines;
    }
    else   //capture
    {
        Line_length   = A2030MIPI_FULL_PERIOD_PIXEL_NUMS + iDummyPixels;
        Frame_length = A2030MIPI_FULL_PERIOD_LINE_NUMS  + iDummyLines;
    }
    
    spin_lock(&A2030mipiraw_drv_lock);
    A2030_Frame_Length_preview = Frame_length;
    spin_unlock(&A2030mipiraw_drv_lock);
    
    SENSORDB("[A2030MIPI_SetDummy]Frame_length = %d, Line_length = %d", Frame_length, Line_length);

    A2030MIPI_write_cmos_sensor_8(0x0104, 0x01); // GROUPED_PARAMETER_HOLD
    A2030MIPI_write_cmos_sensor(0x0340, Frame_length);
    A2030MIPI_write_cmos_sensor(0x0342, Line_length);
    A2030MIPI_write_cmos_sensor_8(0x0104, 0x00); //GROUPED_PARAMETER_HOLD
    
}   /*  A2030MIPI_SetDummy */

UINT32 A2030MIPISetTestPatternMode(kal_bool bEnable)
{
    SENSORDB("A2030MIPISetTestPatternMode function bEnable=%d \n",bEnable);   
    if(bEnable) 
	{   // enable color bar
		A2030test_pattern_flag=TRUE; 
		A2030MIPI_write_cmos_sensor(0x0600,0x02);
    } 
	else 
	{   
		A2030test_pattern_flag=FALSE;
		A2030MIPI_write_cmos_sensor(0x0600,0x00);
    }
	mdelay(50);
    return ERROR_NONE;
}



/*************************************************************************
* FUNCTION
*   A2030MIPIPreview
*
* DESCRIPTION
*   This function start the sensor preview.
*
* PARAMETERS
*   *image_window : address pointer of pixel numbers in one period of HSYNC
*  *sensor_config_data : address pointer of line numbers in one period of VSYNC
*
* RETURNS
*   None
*
* GLOBALS AFFECTED
*
*************************************************************************/
UINT32 A2030MIPIPreview(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
                                                MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
    //kal_uint32 temp;

    SENSORDB("Enter!");
   
    A2030MIPI_Set_Mirror_Flip(sensor_config_data->SensorImageMirror);

    spin_lock(&A2030mipiraw_drv_lock);
    g_iA2030MIPI_Mode = A2030MIPI_MODE_PREVIEW;
	A2030MIPI_sensor.video_mode = KAL_FALSE;//   add 
    //temp = A2030MIPI_exposure_lines;
    spin_unlock(&A2030mipiraw_drv_lock);

    // Insert dummy pixels or dummy lines
    spin_lock(&A2030mipiraw_drv_lock);
    A2030MIPI_PV_dummy_pixels = 0;
    A2030MIPI_PV_dummy_lines  = 0;
    spin_unlock(&A2030mipiraw_drv_lock);
    A2030MIPI_SetDummy(KAL_TRUE, A2030MIPI_PV_dummy_pixels, A2030MIPI_PV_dummy_lines);

    #if 0
    A2030MIPI_write_cmos_sensor_8(0x0104, 0x01);    // GROUPED_PARAMETER_HOLD
    A2030MIPI_write_cmos_sensor(0x0202, temp); /* course_integration_time */
    A2030MIPI_write_cmos_sensor_8(0x0104, 0x00);    // GROUPED_PARAMETER_HOLD

    memcpy(&A2030MIPISensorConfigData, sensor_config_data, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
    #endif
    
    return ERROR_NONE;
}   /* A2030MIPIPreview() */


/*******************************************************************************
*
********************************************************************************/
UINT32 A2030MIPICapture(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
                                                MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
    //kal_uint32 shutter = A2030MIPI_exposure_lines;

    SENSORDB("Enter!");

    A2030MIPI_Set_Mirror_Flip(sensor_config_data->SensorImageMirror);

    spin_lock(&A2030mipiraw_drv_lock);
    g_iA2030MIPI_Mode = A2030MIPI_MODE_CAPTURE; 
	A2030MIPI_sensor.video_mode = KAL_FALSE;//add 
	A2030MIPI_AutoFlicker_Mode=KAL_FALSE;
    spin_unlock(&A2030mipiraw_drv_lock);

    // Insert dummy pixels or dummy lines
    spin_lock(&A2030mipiraw_drv_lock);
    A2030MIPI_dummy_pixels = 0;
    A2030MIPI_dummy_lines  = 0;
    spin_unlock(&A2030mipiraw_drv_lock);
    A2030MIPI_SetDummy(KAL_FALSE, A2030MIPI_dummy_pixels, A2030MIPI_dummy_lines);

    #if 0
    SENSORDB("preview shutter =%d ",shutter);

    shutter = shutter * (A2030MIPI_PV_PERIOD_PIXEL_NUMS + A2030MIPI_PV_dummy_pixels)/(A2030MIPI_FULL_PERIOD_PIXEL_NUMS +A2030MIPI_dummy_pixels);
    shutter = shutter * A2030MIPI_sensor.capture_vt_clk / A2030MIPI_sensor.preview_vt_clk;

    SENSORDB("capture  shutter =%d , gain = %d\n",shutter, read_A2030MIPI_gain());
    
    A2030MIPI_write_cmos_sensor_8(0x0104, 0x01);    // GROUPED_PARAMETER_HOLD
    A2030MIPI_write_cmos_sensor(0x0202, shutter);   /* coarse_integration_time */
    A2030MIPI_write_cmos_sensor_8(0x0104, 0x00);    // GROUPED_PARAMETER_HOLD
    #endif
    
    return ERROR_NONE;
}   /* A2030MIPICapture() */


UINT32 A2030MIPIGetResolution(MSDK_SENSOR_RESOLUTION_INFO_STRUCT *pSensorResolution)
{
    pSensorResolution->SensorFullWidth     =  A2030MIPI_IMAGE_SENSOR_FULL_WIDTH;
    pSensorResolution->SensorFullHeight    =  A2030MIPI_IMAGE_SENSOR_FULL_HEIGHT;
    
    pSensorResolution->SensorPreviewWidth  =  A2030MIPI_IMAGE_SENSOR_PV_WIDTH;
    pSensorResolution->SensorPreviewHeight =  A2030MIPI_IMAGE_SENSOR_PV_HEIGHT;
    
    pSensorResolution->SensorVideoWidth     =  A2030MIPI_IMAGE_SENSOR_PV_WIDTH;
    pSensorResolution->SensorVideoHeight    =  A2030MIPI_IMAGE_SENSOR_PV_HEIGHT;        

    return ERROR_NONE;
}   /* A2030MIPIGetResolution() */


UINT32 A2030MIPIGetInfo(MSDK_SCENARIO_ID_ENUM ScenarioId,
                                                MSDK_SENSOR_INFO_STRUCT *pSensorInfo,
                                                MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData)
{
    switch(ScenarioId)
    {
        case MSDK_SCENARIO_ID_CAMERA_ZSD:
            pSensorInfo->SensorPreviewResolutionX = A2030MIPI_IMAGE_SENSOR_FULL_WIDTH; /* not use */
            pSensorInfo->SensorPreviewResolutionY = A2030MIPI_IMAGE_SENSOR_FULL_HEIGHT; /* not use */
            pSensorInfo->SensorCameraPreviewFrameRate = 15; /* not use */
        break;

        default:
            pSensorInfo->SensorPreviewResolutionX = A2030MIPI_IMAGE_SENSOR_PV_WIDTH; /* not use */
            pSensorInfo->SensorPreviewResolutionY = A2030MIPI_IMAGE_SENSOR_PV_HEIGHT; /* not use */
            pSensorInfo->SensorCameraPreviewFrameRate = 30; /* not use */
        break;
    }
    pSensorInfo->SensorFullResolutionX = A2030MIPI_IMAGE_SENSOR_FULL_WIDTH; /* not use */
    pSensorInfo->SensorFullResolutionY = A2030MIPI_IMAGE_SENSOR_FULL_HEIGHT; /* not use */

    pSensorInfo->SensorVideoFrameRate = 30; /* not use */
    pSensorInfo->SensorStillCaptureFrameRate= 15; /* not use */
    pSensorInfo->SensorWebCamCaptureFrameRate= 15; /* not use */

    pSensorInfo->SensorClockPolarity = SENSOR_CLOCK_POLARITY_LOW;
    pSensorInfo->SensorClockFallingPolarity = SENSOR_CLOCK_POLARITY_LOW; /* not use */
    pSensorInfo->SensorHsyncPolarity = SENSOR_CLOCK_POLARITY_LOW; // inverse with datasheet
    pSensorInfo->SensorVsyncPolarity = SENSOR_CLOCK_POLARITY_HIGH;
    pSensorInfo->SensorInterruptDelayLines = 1; /* not use */
    pSensorInfo->SensorResetActiveHigh = FALSE; /* not use */
    pSensorInfo->SensorResetDelayCount = 5; /* not use */

    #ifdef MIPI_INTERFACE
        pSensorInfo->SensroInterfaceType        = SENSOR_INTERFACE_TYPE_MIPI;
    #else
        pSensorInfo->SensroInterfaceType        = SENSOR_INTERFACE_TYPE_PARALLEL;
    #endif
    pSensorInfo->SensorOutputDataFormat     = A2030MIPI_DATA_FORMAT;

    pSensorInfo->CaptureDelayFrame = 1; 
    pSensorInfo->PreviewDelayFrame = 2; 
    pSensorInfo->VideoDelayFrame = 5; 
    pSensorInfo->SensorMasterClockSwitch = 0; /* not use */
    pSensorInfo->SensorDrivingCurrent = ISP_DRIVING_6MA;      
    pSensorInfo->AEShutDelayFrame = 0;          /* The frame of setting shutter default 0 for TG int */
    pSensorInfo->AESensorGainDelayFrame = 1;    /* The frame of setting sensor gain */
    pSensorInfo->AEISPGainDelayFrame = 2;
       
    switch (ScenarioId)
    {
        case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
        case MSDK_SCENARIO_ID_CAMERA_ZSD:
            pSensorInfo->SensorClockFreq=24;//24;
            pSensorInfo->SensorClockDividCount= 3; /* not use */
            pSensorInfo->SensorClockRisingCount= 0;
            pSensorInfo->SensorClockFallingCount= 2; /* not use */
            pSensorInfo->SensorPixelClockCount= 3; /* not use */
            pSensorInfo->SensorDataLatchCount= 2; /* not use */
            pSensorInfo->SensorGrabStartX = A2030MIPI_FULL_START_X; 
            pSensorInfo->SensorGrabStartY = A2030MIPI_FULL_START_Y;   
            
            #ifdef MIPI_INTERFACE
                pSensorInfo->SensorMIPILaneNumber = SENSOR_MIPI_1_LANE;         
                pSensorInfo->MIPIDataLowPwr2HighSpeedTermDelayCount = 0; 
                pSensorInfo->MIPIDataLowPwr2HighSpeedSettleDelayCount = 14;//130;//14;  
                pSensorInfo->MIPICLKLowPwr2HighSpeedTermDelayCount = 0; 
                pSensorInfo->SensorWidthSampling = 0;
                pSensorInfo->SensorHightSampling = 0;
                pSensorInfo->SensorPacketECCOrder = 1;
            #endif
            break;
        default:
            pSensorInfo->SensorClockFreq=24;
            pSensorInfo->SensorClockDividCount= 3; /* not use */
            pSensorInfo->SensorClockRisingCount= 0;
            pSensorInfo->SensorClockFallingCount= 2; /* not use */
            pSensorInfo->SensorPixelClockCount= 3; /* not use */
            pSensorInfo->SensorDataLatchCount= 2; /* not use */
            pSensorInfo->SensorGrabStartX = A2030MIPI_PV_START_X; 
            pSensorInfo->SensorGrabStartY = A2030MIPI_PV_START_Y;    
            
            #ifdef MIPI_INTERFACE
                pSensorInfo->SensorMIPILaneNumber = SENSOR_MIPI_1_LANE;         
                pSensorInfo->MIPIDataLowPwr2HighSpeedTermDelayCount = 0; 
                pSensorInfo->MIPIDataLowPwr2HighSpeedSettleDelayCount = 14;//130;//14;  
                pSensorInfo->MIPICLKLowPwr2HighSpeedTermDelayCount = 0;
                pSensorInfo->SensorWidthSampling = 0;
                pSensorInfo->SensorHightSampling = 0;
                pSensorInfo->SensorPacketECCOrder = 1;
            #endif
            break;
    }

    //memcpy(pSensorConfigData, &A2030MIPISensorConfigData, sizeof(MSDK_SENSOR_CONFIG_STRUCT));

    return ERROR_NONE;
}   /* A2030MIPIGetInfo() */


UINT32 A2030MIPIControl(MSDK_SCENARIO_ID_ENUM ScenarioId, MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *pImageWindow,
                                                MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData)
{
    spin_lock(&A2030mipiraw_drv_lock);
    A2030_CurrentScenarioId = ScenarioId;
    spin_unlock(&A2030mipiraw_drv_lock);
    
    switch (ScenarioId)
    {
        case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
        case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
            A2030MIPIPreview(pImageWindow, pSensorConfigData);
            break;
        case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
        case MSDK_SCENARIO_ID_CAMERA_ZSD:
            A2030MIPICapture(pImageWindow, pSensorConfigData);
            break;
        default:
            return ERROR_INVALID_SCENARIO_ID;  
    }
    
    return ERROR_NONE;
} /* A2030MIPIControl() */

UINT32 A2030SetMaxFrameRate(UINT16 u2FrameRate)
{
	kal_int16 dummy_line;
	kal_uint16 FrameHeight = A2030MIPI_sensor.frame_height;
	unsigned long flags;
		
	SENSORDB("[A2030SetMaxFrameRate]u2FrameRate=%d",u2FrameRate);

	FrameHeight= (10 * A2030MIPI_sensor.capture_vt_clk *100000) / u2FrameRate / (A2030MIPI_PV_PERIOD_PIXEL_NUMS);

	spin_lock(&A2030mipiraw_drv_lock);
	A2030MIPI_sensor.frame_height = FrameHeight;
	spin_unlock(&A2030mipiraw_drv_lock);
	dummy_line = FrameHeight - (A2030MIPI_PV_PERIOD_LINE_NUMS);
	if (dummy_line < 0) dummy_line = 0;

	/* to fix VSYNC, to fix frame rate */
	A2030MIPI_SetDummy(KAL_TRUE,0, dummy_line); /* modify dummy_pixel must gen AE table again */
	return (UINT32)u2FrameRate;
}

#if 0
UINT32 A2030MIPISetVideoMode(UINT16 u2FrameRate)
{
    kal_uint16 MAX_Frame_length =0;

    SENSORDB("u2FrameRate =%d", u2FrameRate);

	
	A2030MIPI_sensor.video_mode = KAL_TRUE;
	A2030MIPI_sensor.FixedFps= u2FrameRate;
	
	SENSORDB("enter A2030MIPISetVideoMode functionu 2FrameRate=%d\n",u2FrameRate);

    if(u2FrameRate >30 || u2FrameRate <5)
        SENSORDB("Error frame rate seting");

    if (A2030MIPI_MODE_PREVIEW == g_iA2030MIPI_Mode)
    {
        MAX_Frame_length = A2030MIPI_sensor.preview_vt_clk*100000/(A2030MIPI_PV_PERIOD_PIXEL_NUMS+A2030MIPI_PV_dummy_pixels)/u2FrameRate;
        //if(A2030MIPI_PV_dummy_lines <(MAX_Frame_length - A2030MIPI_PV_PERIOD_LINE_NUMS))  //original dummy length < current needed dummy length 
        if(MAX_Frame_length < A2030MIPI_PV_PERIOD_LINE_NUMS )
            MAX_Frame_length = A2030MIPI_PV_PERIOD_LINE_NUMS;

        spin_lock(&A2030mipiraw_drv_lock);
        A2030MIPI_PV_dummy_lines = MAX_Frame_length - A2030MIPI_PV_PERIOD_LINE_NUMS  ;
        spin_unlock(&A2030mipiraw_drv_lock);

        A2030MIPI_SetDummy(KAL_TRUE, A2030MIPI_PV_dummy_pixels, A2030MIPI_PV_dummy_lines);
    }
    else if (A2030MIPI_MODE_CAPTURE == g_iA2030MIPI_Mode)
    {
        MAX_Frame_length = A2030MIPI_sensor.capture_vt_clk*100000/(A2030MIPI_FULL_PERIOD_PIXEL_NUMS+A2030MIPI_dummy_pixels)/u2FrameRate;
        if(MAX_Frame_length < A2030MIPI_FULL_PERIOD_LINE_NUMS )
            MAX_Frame_length = A2030MIPI_FULL_PERIOD_LINE_NUMS;

        spin_lock(&A2030mipiraw_drv_lock);
        A2030MIPI_dummy_lines = MAX_Frame_length - A2030MIPI_FULL_PERIOD_LINE_NUMS  ;
        spin_unlock(&A2030mipiraw_drv_lock);

        A2030MIPI_SetDummy(KAL_FALSE, A2030MIPI_dummy_pixels, A2030MIPI_dummy_lines);
    }

    return KAL_TRUE;
}
#endif
UINT32 A2030MIPISetVideoMode(UINT16 u2FrameRate)
{
	kal_int16 dummy_line;
    /* to fix VSYNC, to fix frame rate */
	SENSORDB("enter A2030MIPISetVideoMode functionu 2FrameRate=%d\n",u2FrameRate);

	spin_lock(&A2030mipiraw_drv_lock);
	if(u2FrameRate == 30){
		A2030MIPI_sensor.NightMode = KAL_FALSE;;
	}else if(u2FrameRate == 15){
		A2030MIPI_sensor.NightMode = KAL_TRUE;
	}else if(u2FrameRate == 0){
		//For Dynamic frame rate,Nothing to do
		A2030MIPI_sensor.video_mode = KAL_FALSE;
		spin_unlock(&A2030mipiraw_drv_lock);
		SENSORDB("Do not fix framerate\n");
		return KAL_TRUE;
	}else{
		// TODO:
		//return TRUE;
	}

	A2030MIPI_sensor.video_mode = KAL_TRUE;
	A2030MIPI_sensor.FixedFps= u2FrameRate;
	spin_unlock(&A2030mipiraw_drv_lock);

	if((u2FrameRate == 30)&&(A2030MIPI_AutoFlicker_Mode==KAL_TRUE))
		u2FrameRate = 303;	
	else if((u2FrameRate == 15)&&(A2030MIPI_AutoFlicker_Mode==KAL_TRUE))
		u2FrameRate= 148;//148;
	else
		u2FrameRate = 10 * u2FrameRate;
	
	A2030SetMaxFrameRate(u2FrameRate);

	SENSORDB("exit A2030MIPISetVideoMode functionu \n");
    return TRUE;
}


UINT32 A2030MIPISetAutoFlickerMode(kal_bool bEnable, UINT16 u2FrameRate)
{
    SENSORDB("frame rate(10base) = %d %d", bEnable, u2FrameRate);	

	SENSORDB("enter A2030MIPISetAutoFlickerMode functionu bEnable=%d\,u2FrameRate=%d\n",bEnable,u2FrameRate);

    if(bEnable) 
    {   
		 spin_lock(&A2030mipiraw_drv_lock);
		A2030MIPI_AutoFlicker_Mode=KAL_TRUE;
		spin_unlock(&A2030mipiraw_drv_lock);		
		if((A2030MIPI_sensor.FixedFps == 30)&&(A2030MIPI_sensor.video_mode==KAL_TRUE))
			A2030SetMaxFrameRate(303);
		if((A2030MIPI_sensor.FixedFps == 15)&&(A2030MIPI_sensor.video_mode==KAL_TRUE))
			A2030SetMaxFrameRate(148);
    } else 
    {
		 spin_lock(&A2030mipiraw_drv_lock);
		A2030MIPI_AutoFlicker_Mode=KAL_FALSE;
		spin_unlock(&A2030mipiraw_drv_lock);		
		if((A2030MIPI_sensor.FixedFps == 30)&&(A2030MIPI_sensor.video_mode==KAL_TRUE))
			A2030SetMaxFrameRate(300);
		if((A2030MIPI_sensor.FixedFps == 15)&&(A2030MIPI_sensor.video_mode==KAL_TRUE))
			A2030SetMaxFrameRate(150);
    }
    return KAL_TRUE;
}


UINT32 A2030MIPIFeatureControl(MSDK_SENSOR_FEATURE_ENUM FeatureId,
                                                                UINT8 *pFeaturePara,UINT32 *pFeatureParaLen)
{    
    UINT16 *pFeatureReturnPara16=(UINT16 *) pFeaturePara;
    UINT16 *pFeatureData16=(UINT16 *) pFeaturePara;
    UINT32 *pFeatureReturnPara32=(UINT32 *) pFeaturePara;
    UINT32 *pFeatureData32=(UINT32 *) pFeaturePara;
    UINT32 SensorRegNumber;
    UINT32 i;
    PNVRAM_SENSOR_DATA_STRUCT pSensorDefaultData=(PNVRAM_SENSOR_DATA_STRUCT) pFeaturePara;
    MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData=(MSDK_SENSOR_CONFIG_STRUCT *) pFeaturePara;
    MSDK_SENSOR_REG_INFO_STRUCT *pSensorRegData=(MSDK_SENSOR_REG_INFO_STRUCT *) pFeaturePara;
    MSDK_SENSOR_GROUP_INFO_STRUCT *pSensorGroupInfo=(MSDK_SENSOR_GROUP_INFO_STRUCT *) pFeaturePara;
    MSDK_SENSOR_ITEM_INFO_STRUCT *pSensorItemInfo=(MSDK_SENSOR_ITEM_INFO_STRUCT *) pFeaturePara;
    MSDK_SENSOR_ENG_INFO_STRUCT *pSensorEngInfo=(MSDK_SENSOR_ENG_INFO_STRUCT *) pFeaturePara;

    switch (FeatureId)
    {
        case SENSOR_FEATURE_GET_RESOLUTION:
            *pFeatureReturnPara16++=A2030MIPI_IMAGE_SENSOR_FULL_WIDTH;
            *pFeatureReturnPara16=A2030MIPI_IMAGE_SENSOR_FULL_HEIGHT;
            *pFeatureParaLen=4;
            break;
        case SENSOR_FEATURE_GET_PERIOD:
            switch(A2030_CurrentScenarioId)
            {
                case MSDK_SCENARIO_ID_CAMERA_ZSD:
                case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
                    *pFeatureReturnPara16++= A2030MIPI_FULL_PERIOD_PIXEL_NUMS + A2030MIPI_dummy_pixels;//A2030MIPI_PV_PERIOD_PIXEL_NUMS+A2030MIPI_dummy_pixels;
                    *pFeatureReturnPara16=A2030MIPI_FULL_PERIOD_LINE_NUMS + A2030MIPI_dummy_lines;
                    *pFeatureParaLen=4;
                     break;
                default:
                     *pFeatureReturnPara16++= A2030MIPI_PV_PERIOD_PIXEL_NUMS + A2030MIPI_PV_dummy_pixels;//A2030MIPI_PV_PERIOD_PIXEL_NUMS+A2030MIPI_dummy_pixels;
                    *pFeatureReturnPara16=A2030MIPI_PV_PERIOD_LINE_NUMS + A2030MIPI_PV_dummy_lines;
                    *pFeatureParaLen=4;
                     break;
            }
            break;
        case SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ:
            switch(A2030_CurrentScenarioId)
            {
                case MSDK_SCENARIO_ID_CAMERA_ZSD:
                case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
                    *pFeatureReturnPara32 = A2030MIPI_sensor.capture_vt_clk * 100000;
                    *pFeatureParaLen=4;
                    break;
                default:
                    *pFeatureReturnPara32 = A2030MIPI_sensor.preview_vt_clk * 100000;
                    *pFeatureParaLen=4;
                    break;
            }
            break;
        case SENSOR_FEATURE_SET_ESHUTTER:
            A2030MIPI_SetShutter(*pFeatureData16);
            break;
        case SENSOR_FEATURE_SET_NIGHTMODE:
            A2030MIPI_NightMode((BOOL) *pFeatureData16);
            break;
        case SENSOR_FEATURE_SET_GAIN:
            A2030MIPI_Set_gain((UINT16) *pFeatureData16);
            break;
        case SENSOR_FEATURE_SET_FLASHLIGHT:
            break;
        case SENSOR_FEATURE_SET_ISP_MASTER_CLOCK_FREQ:
            // A2030MIPI_isp_master_clock=*pFeatureData32;
            break;
        case SENSOR_FEATURE_SET_REGISTER:
            A2030MIPI_write_cmos_sensor(pSensorRegData->RegAddr, pSensorRegData->RegData);
            break;
        case SENSOR_FEATURE_GET_REGISTER:
            pSensorRegData->RegData = A2030MIPI_read_cmos_sensor(pSensorRegData->RegAddr);
            break;
        case SENSOR_FEATURE_SET_CCT_REGISTER:
            SensorRegNumber=FACTORY_END_ADDR;   
            for (i=0;i<SensorRegNumber;i++)
            {
                spin_lock(&A2030mipiraw_drv_lock);
                A2030MIPISensorCCT[i].Addr=*pFeatureData32++;
                A2030MIPISensorCCT[i].Para=*pFeatureData32++;
                spin_unlock(&A2030mipiraw_drv_lock);
            }
            break;
        case SENSOR_FEATURE_GET_CCT_REGISTER:
            SensorRegNumber=FACTORY_END_ADDR;
            if (*pFeatureParaLen<(SensorRegNumber*sizeof(SENSOR_REG_STRUCT)+4))
                return FALSE;
            *pFeatureData32++=SensorRegNumber;
            
            for (i=0;i<SensorRegNumber;i++)
            {
                spin_lock(&A2030mipiraw_drv_lock);
                *pFeatureData32++=A2030MIPISensorCCT[i].Addr;
                *pFeatureData32++=A2030MIPISensorCCT[i].Para;
                spin_unlock(&A2030mipiraw_drv_lock);
            }
            
            break;
        case SENSOR_FEATURE_SET_ENG_REGISTER:
            SensorRegNumber=ENGINEER_END;
            for (i=0;i<SensorRegNumber;i++)
            {
                spin_lock(&A2030mipiraw_drv_lock);
                A2030MIPISensorReg[i].Addr=*pFeatureData32++;
                A2030MIPISensorReg[i].Para=*pFeatureData32++;
                spin_unlock(&A2030mipiraw_drv_lock);
            }
            break;
        case SENSOR_FEATURE_GET_ENG_REGISTER:
            SensorRegNumber=ENGINEER_END;
            if (*pFeatureParaLen<(SensorRegNumber*sizeof(SENSOR_REG_STRUCT)+4))
                return FALSE;
            *pFeatureData32++=SensorRegNumber;
            for (i=0;i<SensorRegNumber;i++)
            {
                spin_lock(&A2030mipiraw_drv_lock);
                *pFeatureData32++=A2030MIPISensorReg[i].Addr;
                *pFeatureData32++=A2030MIPISensorReg[i].Para;
                spin_unlock(&A2030mipiraw_drv_lock);
            }
            break;
        case SENSOR_FEATURE_GET_REGISTER_DEFAULT:
            if (*pFeatureParaLen>=sizeof(NVRAM_SENSOR_DATA_STRUCT))
            {
                pSensorDefaultData->Version=NVRAM_CAMERA_SENSOR_FILE_VERSION;
                pSensorDefaultData->SensorId=A2030MIPI_SENSOR_ID;
                memcpy(pSensorDefaultData->SensorEngReg, A2030MIPISensorReg, sizeof(SENSOR_REG_STRUCT)*ENGINEER_END);
                memcpy(pSensorDefaultData->SensorCCTReg, A2030MIPISensorCCT, sizeof(SENSOR_REG_STRUCT)*FACTORY_END_ADDR);
            }
            else
                return FALSE;
            *pFeatureParaLen=sizeof(NVRAM_SENSOR_DATA_STRUCT);
            break;
        case SENSOR_FEATURE_GET_CONFIG_PARA:
            memcpy(pSensorConfigData, &A2030MIPISensorConfigData, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
            *pFeatureParaLen=sizeof(MSDK_SENSOR_CONFIG_STRUCT);
            break;
        case SENSOR_FEATURE_CAMERA_PARA_TO_SENSOR:
            A2030MIPI_camera_para_to_sensor();
            break;
            
        case SENSOR_FEATURE_SENSOR_TO_CAMERA_PARA:
            A2030MIPI_sensor_to_camera_para();
            break;
        case SENSOR_FEATURE_GET_GROUP_COUNT:
            *pFeatureReturnPara32++=A2030MIPI_get_sensor_group_count();
            *pFeatureParaLen=4;
            break;
        case SENSOR_FEATURE_GET_GROUP_INFO:
            A2030MIPI_get_sensor_group_info(pSensorGroupInfo->GroupIdx, pSensorGroupInfo->GroupNamePtr, &pSensorGroupInfo->ItemCount);
            *pFeatureParaLen=sizeof(MSDK_SENSOR_GROUP_INFO_STRUCT);
            break;
        case SENSOR_FEATURE_GET_ITEM_INFO:
            A2030MIPI_get_sensor_item_info(pSensorItemInfo->GroupIdx,pSensorItemInfo->ItemIdx, pSensorItemInfo);
            *pFeatureParaLen=sizeof(MSDK_SENSOR_ITEM_INFO_STRUCT);
            break;

        case SENSOR_FEATURE_SET_ITEM_INFO:
            A2030MIPI_set_sensor_item_info(pSensorItemInfo->GroupIdx, pSensorItemInfo->ItemIdx, pSensorItemInfo->ItemValue);
            *pFeatureParaLen=sizeof(MSDK_SENSOR_ITEM_INFO_STRUCT);
            break;

        case SENSOR_FEATURE_GET_ENG_INFO:
            pSensorEngInfo->SensorId = 221;
            pSensorEngInfo->SensorType = CMOS_SENSOR;
            pSensorEngInfo->SensorOutputDataFormat = A2030MIPI_DATA_FORMAT;
            *pFeatureParaLen=sizeof(MSDK_SENSOR_ENG_INFO_STRUCT);
            break;
        case SENSOR_FEATURE_GET_LENS_DRIVER_ID:
            // get the lens driver ID from EEPROM or just return LENS_DRIVER_ID_DO_NOT_CARE
            // if EEPROM does not exist in camera module.
            *pFeatureReturnPara32=LENS_DRIVER_ID_DO_NOT_CARE;
            *pFeatureParaLen=4;
            break;
        case SENSOR_FEATURE_SET_VIDEO_MODE:
            A2030MIPISetVideoMode(*pFeatureData16);
            break;
        case SENSOR_FEATURE_CHECK_SENSOR_ID:
            A2030MIPIGetSensorID(pFeatureReturnPara32); 
            break; 
        case SENSOR_FEATURE_SET_AUTO_FLICKER_MODE:
            A2030MIPISetAutoFlickerMode((BOOL) *pFeatureData16, *(pFeatureData16 + 1));
            break; 
        case SENSOR_FEATURE_SET_MAX_FRAME_RATE_BY_SCENARIO:
            A2030MIPISetMaxFramerateByScenario((MSDK_SCENARIO_ID_ENUM)*pFeatureData32, *(pFeatureData32+1));
            break;
        case SENSOR_FEATURE_GET_DEFAULT_FRAME_RATE_BY_SCENARIO:
            A2030MIPIGetDefaultFramerateByScenario((MSDK_SCENARIO_ID_ENUM)*pFeatureData32, (MUINT32 *)(*(pFeatureData32+1)));
            break; 
		case SENSOR_FEATURE_SET_TEST_PATTERN:
	         A2030MIPISetTestPatternMode((BOOL)*pFeatureData16);  
			 break;
		case SENSOR_FEATURE_GET_TEST_PATTERN_CHECKSUM_VALUE:
			*pFeatureReturnPara32 =  A2030_TEST_PATTERN_CHECKSUM;
			 *pFeatureParaLen=4;
			break;
        default:
            break;
    }
    
    return ERROR_NONE;
}   /* A2030MIPIFeatureControl() */


SENSOR_FUNCTION_STRUCT  SensorFuncA2030MIPI=
{
    A2030MIPIOpen,
    A2030MIPIGetInfo,
    A2030MIPIGetResolution,
    A2030MIPIFeatureControl,
    A2030MIPIControl,
    A2030MIPIClose
};

UINT32 A2030_MIPI_RAW_SensorInit(PSENSOR_FUNCTION_STRUCT *pfFunc)
{
    /* To Do : Check Sensor status here */
    if (pfFunc!=NULL)
        *pfFunc=&SensorFuncA2030MIPI;

    return ERROR_NONE;
}/* SensorInit() */

