/*****************************************************************************

 ****************************************************************************/

#include <linux/videodev2.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <asm/atomic.h>
#include <asm/system.h>


#include "kd_camera_hw.h"
#include "kd_imgsensor.h"
#include "kd_imgsensor_define.h"
#include "kd_imgsensor_errcode.h"

#include "gc5004mipiraw_Sensor.h"
#include "gc5004mipiraw_Camera_Sensor_para.h"
#include "gc5004mipiraw_CameraCustomized.h"

kal_bool  GC5004MIPI_MPEG4_encode_mode = KAL_FALSE;
kal_bool  GC5004MIPI_Auto_Flicker_mode = KAL_FALSE;


kal_uint8 GC5004MIPI_sensor_write_I2C_address = GC5004MIPI_WRITE_ID;
kal_uint8 GC5004MIPI_sensor_read_I2C_address = GC5004MIPI_READ_ID;
kal_bool GC5004MIPI_Lock = KAL_FALSE;


	

static struct GC5004MIPI_sensor_STRUCT GC5004MIPI_sensor;


static MSDK_SCENARIO_ID_ENUM CurrentScenarioId = MSDK_SCENARIO_ID_CAMERA_PREVIEW;


#define ANALOG_GAIN_1 64  // 1.00x
#define ANALOG_GAIN_2 90  // 1.41x
#define ANALOG_GAIN_3 128  // 2.00x
#define ANALOG_GAIN_4 180  // 2.81x
#define ANALOG_GAIN_5 250  // 3.91x
#define ANALOG_GAIN_6 341  // 5.33x
#define ANALOG_GAIN_7 454  // 7.09x
#define GC5004_TEST_PATTERN_CHECKSUM (0x40ba6bc)

kal_uint16  GC5004MIPI_sensor_gain_base=0x0;
/* MAX/MIN Explosure Lines Used By AE Algorithm */
kal_uint16 GC5004MIPI_MAX_EXPOSURE_LINES = GC5004MIPI_PV_FRAME_LENGTH_LINES-5;//;
kal_uint32 GC5004MIPI_isp_master_clock;
//kal_uint16 GC5004MIPI_CURRENT_FRAME_LINES = GC5004MIPI_PV_PERIOD_LINE_NUMS;//;
static DEFINE_SPINLOCK(gc5004_drv_lock);

#define SENSORDB(fmt, arg...) printk( "[GC5004MIPIRaw] "  fmt, ##arg)
#define RETAILMSG(x,...)
#define TEXT
UINT8 GC5004MIPIPixelClockDivider=0;
kal_uint16 GC5004MIPI_sensor_id=0;
MSDK_SENSOR_CONFIG_STRUCT GC5004MIPISensorConfigData;
kal_uint32 GC5004MIPI_FAC_SENSOR_REG;
kal_uint16 GC5004MIPI_sensor_flip_value; 
#define GC5004MIPI_MaxGainIndex 72		
// Gain Indexs
kal_uint16 GC5004MIPI_sensorGainMapping[GC5004MIPI_MaxGainIndex][2] = {
	{64 ,1	},
	{67 ,13 },
	{70 ,24 },
	{74 ,34 },
	{77 ,43 },
	{80 ,52 },
	{83 ,59 },
	{87 ,67 },
	{90 ,73 },
	{93 ,80 },
	{96 ,85 },
	{99 ,91 },
	{102,96 },
	{106,101},
	{109,105},
	{112,110},
	{115,114},
	{119,118},
	{122,121},
	{125,125},
	{128,128},
	{131,131},
	{134,134},
	{138,137},
	{141,140},
	{143,141},
	{144,142},
	{148,145},
	{150,147},
	{153,149},
	{157,152},
	{159,153},
	{164,156},
	{167,158},
	{171,160},
	{174,162},
	{178,164},
	{182,166},
	{186,168},
	{191,170},
	{195,172},
	{200,174},
	{205,176},
	{210,178}, 
	{216,180}, 
	{221,182}, 
	{228,184}, 
	{234,186}, 
	{241,188}, 
	{248,190}, 
	{256,192}, 
	{264,194},
	{273,196},
	{282,198},
	{292,200},
	{303,202},
	{328,206},
	{341,208},
	{356,210},
	{372,212},
	{381,213},
	{390,214},
	{399,215},
	{410,216},
	{420,217},
	{431,218},
	{443,219},
	{455,220},
	{468,221},
	{482,222},
	{497,223},
	{512,224},
	{512,224},
	{546,226},
	{585,228},
	{630,230},
	{683,232},
	{712,233},
	{745,234},
	{780,235},
	{819,236},
	{862,237},
	{910,238},
	{964,239},
	{1024,240}
		
};
/* FIXME: old factors and DIDNOT use now. s*/
SENSOR_REG_STRUCT GC5004MIPISensorCCT[]=CAMERA_SENSOR_CCT_DEFAULT_VALUE;
SENSOR_REG_STRUCT GC5004MIPISensorReg[ENGINEER_END]=CAMERA_SENSOR_REG_DEFAULT_VALUE;
/* FIXME: old factors and DIDNOT use now. e*/


extern int iReadRegI2C(u8 *a_pSendData , u16 a_sizeSendData, u8 * a_pRecvData, u16 a_sizeRecvData, u16 i2cId);
extern int iWriteRegI2C(u8 *a_pSendData , u16 a_sizeSendData, u16 i2cId);

/*************************************************************************
* FUNCTION
*    GC5004MIPI_write_cmos_sensor
*
* DESCRIPTION
*    This function wirte data to CMOS sensor through I2C
*
* PARAMETERS
*    addr: the 16bit address of register
*    para: the 8bit value of register
*
* RETURNS
*    None
*
* LOCAL AFFECTED
*
*********GC5004MIPI_Lock****************************************************************/
static void GC5004MIPI_write_cmos_sensor(kal_uint8 addr, kal_uint8 para)
{
	kal_uint8 out_buff[2];

	out_buff[0] = addr;
	out_buff[1] = para;

	if(addr == 0xff)
	{
		if(para == 0x01)
			GC5004MIPI_Lock = KAL_TRUE;
		else
			GC5004MIPI_Lock = KAL_FALSE;
	}
	else
		iWriteRegI2C((u8*)out_buff , (u16)sizeof(out_buff), GC5004MIPI_WRITE_ID); 
}

/*************************************************************************
* FUNCTION
*    GC2035_read_cmos_sensor
*
* DESCRIPTION
*    This function read data from CMOS sensor through I2C.
*
* PARAMETERS
*    addr: the 16bit address of register
*
* RETURNS
*    8bit data read through I2C
*
* LOCAL AFFECTED
*
*************************************************************************/
static kal_uint8 GC5004MIPI_read_cmos_sensor(kal_uint8 addr)
{
	kal_uint8 in_buff[1] = {0xFF};
	kal_uint8 out_buff[1];

	out_buff[0] = addr;

	if (0 != iReadRegI2C((u8*)out_buff , (u16) sizeof(out_buff), (u8*)in_buff, (u16) sizeof(in_buff), GC5004MIPI_WRITE_ID)) {
	SENSORDB("ERROR: GC5004MIPI_read_cmos_sensor \n");
	}
	return in_buff[0];
}


static kal_uint16 GC5004MIPIReg2Gain(const kal_uint8 iReg)
{

    kal_uint8 iI;
    // Range: 1x to 16x
    for (iI = 0; iI < GC5004MIPI_MaxGainIndex; iI++) {
        if(iReg <= GC5004MIPI_sensorGainMapping[iI][1]){
            break;
        }
    }
    return GC5004MIPI_sensorGainMapping[iI][0];

}
static kal_uint8 GC5004MIPIGain2Reg(const kal_uint16 iGain)
{

	kal_uint8 iI;
    
    for (iI = 0; iI < (GC5004MIPI_MaxGainIndex-1); iI++) {
        if(iGain <= GC5004MIPI_sensorGainMapping[iI][0]){    
            break;
        }
    }
    if(iGain != GC5004MIPI_sensorGainMapping[iI][0])
    {
         printk("[GC5004MIPIGain2Reg] Gain mapping don't correctly:%d %d \n", iGain, GC5004MIPI_sensorGainMapping[iI][0]);
    }
    return GC5004MIPI_sensorGainMapping[iI][1];
	/*return NONE;

kal_uint16 Gain;
	Gain=(256*iGain-(256*64+iGain*1/2))/iGain;
    return Gain;
	*/	
}

/*************************************************************************
* FUNCTION
*    GC5004MIPI_SetGain
*
* DESCRIPTION
*    This function is to set global gain to sensor.
*
* PARAMETERS
*    gain : sensor global gain(base: 0x40)
*
* RETURNS
*    the actually gain set to sensor.
*
* GLOBALS AFFECTED
*
*************************************************************************/
void GC5004MIPI_SetGain(UINT16 iGain)
{

	kal_uint16 iReg,temp;
	SENSORDB("GC5004MIPI_SetGain iGain = %d \n",iGain);
	GC5004MIPI_write_cmos_sensor(0xfe,0x00);
	GC5004MIPI_write_cmos_sensor(0xb1, 0x01);
	GC5004MIPI_write_cmos_sensor(0xb2, 0x00);

	iReg = iGain;
#if 0 //digital gain
	GC5004MIPI_write_cmos_sensor(0xb1, iReg>>6);
	GC5004MIPI_write_cmos_sensor(0xb2, (iReg<<2)&0xfc);
#else //analog gain
	if(iReg < 0x40)
		iReg = 0x40;
	else if((ANALOG_GAIN_1<= iReg)&&(iReg < ANALOG_GAIN_2))
	{
		//analog gain
		GC5004MIPI_write_cmos_sensor(0xb6,  0x00);// 
		temp = iReg;
		GC5004MIPI_write_cmos_sensor(0xb1, temp>>6);
		GC5004MIPI_write_cmos_sensor(0xb2, (temp<<2)&0xfc);
		SENSORDB("GC5004MIPI analogic gain 1x , GC5004MIPI add pregain = %d\n",temp);
	}
	else if((ANALOG_GAIN_2<= iReg)&&(iReg < ANALOG_GAIN_3))
	{
		//analog gain
		GC5004MIPI_write_cmos_sensor(0xb6,  0x01);// 
		temp = 64*iReg/ANALOG_GAIN_2;
		GC5004MIPI_write_cmos_sensor(0xb1, temp>>6);
		GC5004MIPI_write_cmos_sensor(0xb2, (temp<<2)&0xfc);
		SENSORDB("GC5004MIPI analogic gain 1.45x , GC5004MIPI add pregain = %d\n",temp);
	}

	else if((ANALOG_GAIN_3<= iReg)&&(iReg < ANALOG_GAIN_4))
	{
		//analog gain
		GC5004MIPI_write_cmos_sensor(0xb6,  0x02);//
		temp = 64*iReg/ANALOG_GAIN_3;
		GC5004MIPI_write_cmos_sensor(0xb1, temp>>6);
		GC5004MIPI_write_cmos_sensor(0xb2, (temp<<2)&0xfc);
		SENSORDB("GC5004MIPI analogic gain 2.02x , GC5004MIPI add pregain = %d\n",temp);
	}
	else if((ANALOG_GAIN_4<= iReg)&&(iReg < ANALOG_GAIN_5))
	{
		//analog gain
		GC5004MIPI_write_cmos_sensor(0xb6,  0x03);//
		temp = 64*iReg/ANALOG_GAIN_4;
		GC5004MIPI_write_cmos_sensor(0xb1, temp>>6);
		GC5004MIPI_write_cmos_sensor(0xb2, (temp<<2)&0xfc);
		SENSORDB("GC5004MIPI analogic gain 2.86x , GC5004MIPI add pregain = %d\n",temp);
	}

	else if((ANALOG_GAIN_5<= iReg)&&(iReg)&&(iReg < ANALOG_GAIN_6))
//	else if(ANALOG_GAIN_5<= iReg)
	{
		//analog gain
		GC5004MIPI_write_cmos_sensor(0xb6,  0x04);//
		temp = 64*iReg/ANALOG_GAIN_5;
		GC5004MIPI_write_cmos_sensor(0xb1, temp>>6);
		GC5004MIPI_write_cmos_sensor(0xb2, (temp<<2)&0xfc);
		SENSORDB("GC5004MIPI analogic gain 3.95x , GC5004MIPI add pregain = %d\n",temp);
	}


	else if((ANALOG_GAIN_6<= iReg)&&(iReg < ANALOG_GAIN_7))
	{
		//analog gain
		GC5004MIPI_write_cmos_sensor(0xb6,  0x05);//
		temp = 64*iReg/ANALOG_GAIN_6;
		GC5004MIPI_write_cmos_sensor(0xb1, temp>>6);
		GC5004MIPI_write_cmos_sensor(0xb2, (temp<<2)&0xfc);
		//SENSORDB("GC5004MIPI analogic gain 5.46x , GC5004MIPI add pregain = %d\n",temp);
	}
	else if(ANALOG_GAIN_7<= iReg)
	{
		//analog gain
		GC5004MIPI_write_cmos_sensor(0xb6,  0x06);//
		temp = 64*iReg/ANALOG_GAIN_7;
		GC5004MIPI_write_cmos_sensor(0xb1, temp>>6);
		GC5004MIPI_write_cmos_sensor(0xb2, (temp<<2)&0xfc);
		//SENSORDB("GC5004MIPI analogic gain 7.5x");
	}
	   //change by zxl 7.19

#endif
}   /*  GC5004MIPI_SetGain_SetGain  */


/*************************************************************************
* FUNCTION
*    read_GC5004MIPI_gain
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
kal_uint16 read_GC5004MIPI_gain(void)
{
   // return (kal_uint16)((GC5004MIPI_read_cmos_sensor(0x0204)<<8) | GC5004MIPI_read_cmos_sensor(0x0205)) ;
}  /* read_GC5004MIPI_gain */

void GC5004MIPI_camera_para_to_sensor(void)
{

	kal_uint32    i;
    for(i=0; 0xFFFFFFFF!=GC5004MIPISensorReg[i].Addr; i++)
    {
        GC5004MIPI_write_cmos_sensor(GC5004MIPISensorReg[i].Addr, GC5004MIPISensorReg[i].Para);
    }
    for(i=ENGINEER_START_ADDR; 0xFFFFFFFF!=GC5004MIPISensorReg[i].Addr; i++)
    {
        GC5004MIPI_write_cmos_sensor(GC5004MIPISensorReg[i].Addr, GC5004MIPISensorReg[i].Para);
    }
    for(i=FACTORY_START_ADDR; i<FACTORY_END_ADDR; i++)
    {
        GC5004MIPI_write_cmos_sensor(GC5004MIPISensorCCT[i].Addr, GC5004MIPISensorCCT[i].Para);
    }

}


/*************************************************************************
* FUNCTION
*    GC5004MIPI_sensor_to_camera_para
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
void GC5004MIPI_sensor_to_camera_para(void)
{

	kal_uint32    i,temp_data;
    for(i=0; 0xFFFFFFFF!=GC5004MIPISensorReg[i].Addr; i++)
    {
		temp_data=GC5004MIPI_read_cmos_sensor(GC5004MIPISensorReg[i].Addr);
		spin_lock(&gc5004_drv_lock);
		GC5004MIPISensorReg[i].Para = temp_data;
		spin_unlock(&gc5004_drv_lock);
    }
    for(i=ENGINEER_START_ADDR; 0xFFFFFFFF!=GC5004MIPISensorReg[i].Addr; i++)
    {
    	temp_data=GC5004MIPI_read_cmos_sensor(GC5004MIPISensorReg[i].Addr);
         spin_lock(&gc5004_drv_lock);
        GC5004MIPISensorReg[i].Para = temp_data;
		spin_unlock(&gc5004_drv_lock);
    }

}

/*************************************************************************
* FUNCTION
*    GC5004MIPI_get_sensor_group_count
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
kal_int32  GC5004MIPI_get_sensor_group_count(void)
{
    return GROUP_TOTAL_NUMS;
}

void GC5004MIPI_get_sensor_group_info(kal_uint16 group_idx, kal_int8* group_name_ptr, kal_int32* item_count_ptr)
{
   switch (group_idx)
   {
        case PRE_GAIN:
            sprintf((char *)group_name_ptr, "CCT");
            *item_count_ptr = 2;
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

void GC5004MIPI_get_sensor_item_info(kal_uint16 group_idx,kal_uint16 item_idx, MSDK_SENSOR_ITEM_INFO_STRUCT* info_ptr)
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
                 SENSORDB("[IMX105MIPI][Error]get_sensor_item_info error!!!\n");
          }
           	spin_lock(&gc5004_drv_lock);    
            temp_para=GC5004MIPISensorCCT[temp_addr].Para;	
			spin_unlock(&gc5004_drv_lock);
            temp_gain = GC5004MIPIReg2Gain(temp_para);
            temp_gain=(temp_gain*1000)/BASEGAIN;
            info_ptr->ItemValue=temp_gain;
            info_ptr->IsTrueFalse=KAL_FALSE;
            info_ptr->IsReadOnly=KAL_FALSE;
            info_ptr->IsNeedRestart=KAL_FALSE;
            info_ptr->Min=1000;
            info_ptr->Max=15875;
            break;
        case CMMCLK_CURRENT:
            switch (item_idx)
            {
                case 0:
                    sprintf((char *)info_ptr->ItemNamePtr,"Drv Cur[2,4,6,8]mA");
                
                    //temp_reg=GC5004MIPISensorReg[CMMCLK_CURRENT_INDEX].Para;
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
                    info_ptr->ItemValue=GC5004MIPI_MAX_EXPOSURE_LINES;
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

//void GC5004MIPI_set_isp_driving_current(kal_uint8 current)
//{

//}

kal_bool GC5004MIPI_set_sensor_item_info(kal_uint16 group_idx, kal_uint16 item_idx, kal_int32 ItemValue)
{
//   kal_int16 temp_reg;
   kal_uint16 temp_addr=0, temp_para=0;

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
                 SENSORDB("[IMX105MIPI][Error]set_sensor_item_info error!!!\n");
          }
            temp_para = GC5004MIPIGain2Reg(ItemValue);
            spin_lock(&gc5004_drv_lock);    
            GC5004MIPISensorCCT[temp_addr].Para = temp_para;
			spin_unlock(&gc5004_drv_lock);
            GC5004MIPI_write_cmos_sensor(GC5004MIPISensorCCT[temp_addr].Addr,temp_para);
			temp_para=read_GC5004MIPI_gain();	
            spin_lock(&gc5004_drv_lock);    
            GC5004MIPI_sensor_gain_base=temp_para;
			spin_unlock(&gc5004_drv_lock);

            break;
        case CMMCLK_CURRENT:
            switch (item_idx)
            {
                case 0:
                    if(ItemValue==2)
                    {			
                    spin_lock(&gc5004_drv_lock);    
                        GC5004MIPISensorReg[CMMCLK_CURRENT_INDEX].Para = ISP_DRIVING_2MA;
					spin_unlock(&gc5004_drv_lock);
                        //GC5004MIPI_set_isp_driving_current(ISP_DRIVING_2MA);
                    }
                    else if(ItemValue==3 || ItemValue==4)
                    {
                    	spin_lock(&gc5004_drv_lock);    
                        GC5004MIPISensorReg[CMMCLK_CURRENT_INDEX].Para = ISP_DRIVING_4MA;
						spin_unlock(&gc5004_drv_lock);
                        //GC5004MIPI_set_isp_driving_current(ISP_DRIVING_4MA);
                    }
                    else if(ItemValue==5 || ItemValue==6)
                    {
                    	spin_lock(&gc5004_drv_lock);    
                        GC5004MIPISensorReg[CMMCLK_CURRENT_INDEX].Para = ISP_DRIVING_6MA;
						spin_unlock(&gc5004_drv_lock);
                        //GC5004MIPI_set_isp_driving_current(ISP_DRIVING_6MA);
                    }
                    else
                    {
                    	spin_lock(&gc5004_drv_lock);    
                        GC5004MIPISensorReg[CMMCLK_CURRENT_INDEX].Para = ISP_DRIVING_8MA;
						spin_unlock(&gc5004_drv_lock);
                        //GC5004MIPI_set_isp_driving_current(ISP_DRIVING_8MA);
                    }
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
					spin_lock(&gc5004_drv_lock);    
                    GC5004MIPI_FAC_SENSOR_REG=ItemValue;
					spin_unlock(&gc5004_drv_lock);
                    break;
                case 1:
                    GC5004MIPI_write_cmos_sensor(GC5004MIPI_FAC_SENSOR_REG,ItemValue);
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

static void GC5004MIPI_SetDummy(const kal_uint16 iPixels, const kal_uint16 iLines)
{

   kal_uint16 Hblanking=0;
   kal_uint16 Vblanking=0;
   kal_uint16 SensorVB=0;
   SensorVB=iLines/4;
   SensorVB=SensorVB*4;
   
    if(GC5004MIPI_sensor.pv_mode == KAL_TRUE)
   	{
	 //
   	 spin_lock(&gc5004_drv_lock);    
	   GC5004MIPI_sensor.pv_HB=431;
	   GC5004MIPI_sensor.pv_VB=144;
  	 spin_unlock(&gc5004_drv_lock);   
	 Hblanking=GC5004MIPI_sensor.pv_HB+iPixels;
	 Vblanking=GC5004MIPI_sensor.pv_VB+SensorVB;
	 spin_lock(&gc5004_drv_lock); 
   	 GC5004MIPI_sensor.pv_HB = Hblanking;
	 GC5004MIPI_sensor.pv_VB = Vblanking;
	 spin_unlock(&gc5004_drv_lock);
	 GC5004MIPI_write_cmos_sensor(0xfe, 0x00);//page0
	 if(iPixels!=0){
	 GC5004MIPI_write_cmos_sensor(0x05, (Hblanking>>8)&0x0F); //HB
	 GC5004MIPI_write_cmos_sensor(0x06, Hblanking&0xFF); }
	 GC5004MIPI_write_cmos_sensor(0x07, (Vblanking>>8)&0x1F); //VB
	 GC5004MIPI_write_cmos_sensor(0x08, Vblanking&0xFF);
   	}
   else if(GC5004MIPI_sensor.video_mode== KAL_TRUE)
   	{
   	// spin_lock(&gc5004_drv_lock);    
   	   spin_lock(&gc5004_drv_lock);    
	   GC5004MIPI_sensor.video_HB=431;
	   GC5004MIPI_sensor.video_VB=144;
        spin_unlock(&gc5004_drv_lock);
   	 Hblanking=GC5004MIPI_sensor.video_HB+iPixels;
	 Vblanking=GC5004MIPI_sensor.video_VB+SensorVB;
	 
//	 SENSORDB("[GC5004MIPIr]%s(),Hblanking:%d\n",__FUNCTION__,Hblanking);
//	 SENSORDB("[GC5004MIPIr]%s(),Vblanking:%d\n",__FUNCTION__,Vblanking);
	 spin_lock(&gc5004_drv_lock);	
   	 GC5004MIPI_sensor.video_HB = Hblanking;
	 GC5004MIPI_sensor.video_VB = Vblanking;
     spin_unlock(&gc5004_drv_lock);
	 GC5004MIPI_write_cmos_sensor(0xfe, 0x00);//page0
	 if(iPixels!=0)
	 	{
	 GC5004MIPI_write_cmos_sensor(0x05, (Hblanking>>8)&0x0F); //HB
	 GC5004MIPI_write_cmos_sensor(0x06, Hblanking&0xFF); 
	 	}
	 GC5004MIPI_write_cmos_sensor(0x07, (Vblanking>>8)&0x1F); //VB
	 GC5004MIPI_write_cmos_sensor(0x08, Vblanking&0xFF);
	 

	 
   	}
	else if(GC5004MIPI_sensor.capture_mode== KAL_TRUE) 
		{
	 // spin_lock(&gc5004_drv_lock);	
	 spin_lock(&gc5004_drv_lock);    
	 GC5004MIPI_sensor.cp_HB=506;
	   GC5004MIPI_sensor.cp_VB=28;
   spin_unlock(&gc5004_drv_lock);
   	  Hblanking=GC5004MIPI_sensor.cp_HB+iPixels;
	  Vblanking=GC5004MIPI_sensor.cp_VB+SensorVB;
	  spin_lock(&gc5004_drv_lock);	 
	  GC5004MIPI_sensor.cp_HB = Hblanking;
	  GC5004MIPI_sensor.cp_VB = Vblanking;
	  spin_unlock(&gc5004_drv_lock);
	  GC5004MIPI_write_cmos_sensor(0xfe, 0x00);//page0
	  if(iPixels!=0){
	  GC5004MIPI_write_cmos_sensor(0x05, (Hblanking>>8)&0x0F); //HB
	  GC5004MIPI_write_cmos_sensor(0x06, Hblanking&0xFF); }
	  GC5004MIPI_write_cmos_sensor(0x07, (Vblanking>>8)&0x1F); //VB
	  GC5004MIPI_write_cmos_sensor(0x08, Vblanking&0xFF);
    }
	else
		{
		 SENSORDB("[GC5004MIPIr]%s(),sensor mode error",__FUNCTION__);
		}
	
}   /*  GC5004MIPI_SetDummy */
void GC5004MIPI_Para_Init(void)
{
	GC5004MIPI_sensor.i2c_write_id=GC5004MIPI_WRITE_ID;
	GC5004MIPI_sensor.pv_mode=KAL_TRUE;
	GC5004MIPI_sensor.video_mode=KAL_FALSE;
	GC5004MIPI_sensor.capture_mode=KAL_FALSE;
	GC5004MIPI_sensor.night_mode=KAL_FALSE;
	GC5004MIPI_sensor.mirror_flip=IMAGE_NORMAL;
	GC5004MIPI_sensor.pv_pclk=288000000;
#if 0
	GC5004MIPI_sensor.video_pclk=120000000;
#else
	GC5004MIPI_sensor.video_pclk=288000000;
#endif
	GC5004MIPI_sensor.cp_pclk=120000000;
	GC5004MIPI_sensor.pv_shutter=800;
	GC5004MIPI_sensor.video_shutter=800;
	GC5004MIPI_sensor.cp_shutter=800;
	GC5004MIPI_sensor.pv_gain=64;
	GC5004MIPI_sensor.video_gain=64;
	GC5004MIPI_sensor.cp_gain=64;
	GC5004MIPI_sensor.pv_line_length=4500;
	GC5004MIPI_sensor.pv_frame_length=1988;
#if 0
	GC5004MIPI_sensor.video_line_length=3252;
	GC5004MIPI_sensor.video_frame_length=1124;
#else
	GC5004MIPI_sensor.video_line_length=4500;
	GC5004MIPI_sensor.video_frame_length=1988;
#endif
	GC5004MIPI_sensor.cp_line_length=4800;
	GC5004MIPI_sensor.cp_frame_length=1988;
	GC5004MIPI_sensor.pv_HB=431;
	GC5004MIPI_sensor.pv_VB=28;
#if 0
	GC5004MIPI_sensor.video_HB=275;
	GC5004MIPI_sensor.video_VB=28;
#else
	GC5004MIPI_sensor.video_HB=431;
	GC5004MIPI_sensor.video_VB=28;
#endif
	GC5004MIPI_sensor.cp_HB=506;
	GC5004MIPI_sensor.cp_VB=28;
	GC5004MIPI_sensor.video_current_frame_rate=30;

}

static void GC5004MIPI_Sensor_Init(void)
	{
	//2592x1944
	/////////////////////////////////////////////////////
	//////////////////////   SYS   //////////////////////
	/////////////////////////////////////////////////////
	GC5004MIPI_write_cmos_sensor(0xfe, 0x80);
	GC5004MIPI_write_cmos_sensor(0xfe, 0x80);
	GC5004MIPI_write_cmos_sensor(0xfe, 0x80);
	GC5004MIPI_write_cmos_sensor(0xf2, 0x00); //sync_pad_io_ebi
	GC5004MIPI_write_cmos_sensor(0xf6, 0x00); //up down
	GC5004MIPI_write_cmos_sensor(0xfc, 0x06);
	GC5004MIPI_write_cmos_sensor(0xf7, 0x37); //pll enable
	GC5004MIPI_write_cmos_sensor(0xf8, 0x97); //Pll mode 2
	GC5004MIPI_write_cmos_sensor(0xf9, 0xfe); //[0] pll enable  change at 17:37 04/19
	GC5004MIPI_write_cmos_sensor(0xfa, 0x11); //div
	GC5004MIPI_write_cmos_sensor(0xfe, 0x00);

	/////////////////////////////////////////////////////
	////////////////   ANALOG & CISCTL   ////////////////
	/////////////////////////////////////////////////////
	GC5004MIPI_write_cmos_sensor(0x00, 0x00); //10/[4]rowskip_skip_sh
	GC5004MIPI_write_cmos_sensor(0x03, 0x07); //15fps
	GC5004MIPI_write_cmos_sensor(0x04, 0xd0); 
	GC5004MIPI_write_cmos_sensor(0x05, 0x01); //HB
	GC5004MIPI_write_cmos_sensor(0x06, 0xb7); 
	GC5004MIPI_write_cmos_sensor(0x07, 0x00); //VB
	GC5004MIPI_write_cmos_sensor(0x08, 0x90);
	GC5004MIPI_write_cmos_sensor(0x09, 0x00);
	GC5004MIPI_write_cmos_sensor(0x0a, 0x02); //02//row start
	GC5004MIPI_write_cmos_sensor(0x0b, 0x00);
	GC5004MIPI_write_cmos_sensor(0x0c, 0x00); //0c//col start
	GC5004MIPI_write_cmos_sensor(0x0d, 0x07); 
	GC5004MIPI_write_cmos_sensor(0x0e, 0xa8); 
	GC5004MIPI_write_cmos_sensor(0x0f, 0x0a); //Window setting
	GC5004MIPI_write_cmos_sensor(0x10, 0x50);  
	GC5004MIPI_write_cmos_sensor(0x17, 0x15); //01//14//[0]mirror [1]flip
	GC5004MIPI_write_cmos_sensor(0x18, 0x02); //sdark off
	GC5004MIPI_write_cmos_sensor(0x19, 0x0c); 
	GC5004MIPI_write_cmos_sensor(0x1a, 0x13); 
	GC5004MIPI_write_cmos_sensor(0x1b, 0x48); 
	GC5004MIPI_write_cmos_sensor(0x1c, 0x05); 
	GC5004MIPI_write_cmos_sensor(0x1e, 0xb8);
	GC5004MIPI_write_cmos_sensor(0x1f, 0x78); 
	GC5004MIPI_write_cmos_sensor(0x20, 0xc5); //03/[7:6]ref_r [3:1]comv_r 
	GC5004MIPI_write_cmos_sensor(0x21, 0x4f); //7f
	GC5004MIPI_write_cmos_sensor(0x22, 0x82); //b2 
	GC5004MIPI_write_cmos_sensor(0x23, 0x43); //f1/[7:3]opa_r [1:0]sRef
	GC5004MIPI_write_cmos_sensor(0x24, 0x2f); //PAD drive 
	GC5004MIPI_write_cmos_sensor(0x2b, 0x01); 
	GC5004MIPI_write_cmos_sensor(0x2c, 0x68); //[6:4]rsgh_r 

	/////////////////////////////////////////////////////
	//////////////////////   ISP   //////////////////////
	/////////////////////////////////////////////////////
	GC5004MIPI_write_cmos_sensor(0x86, 0x0a);
	GC5004MIPI_write_cmos_sensor(0x8a, 0x83);
	GC5004MIPI_write_cmos_sensor(0x8c, 0x10);
	GC5004MIPI_write_cmos_sensor(0x8d, 0x01);
	GC5004MIPI_write_cmos_sensor(0x90, 0x01);
	GC5004MIPI_write_cmos_sensor(0x92, 0x00); //00/crop win y
	GC5004MIPI_write_cmos_sensor(0x94, 0x0c); //04/crop win x
	GC5004MIPI_write_cmos_sensor(0x95, 0x07); //crop win height
	GC5004MIPI_write_cmos_sensor(0x96, 0x98);
	GC5004MIPI_write_cmos_sensor(0x97, 0x0a); //crop win width
	GC5004MIPI_write_cmos_sensor(0x98, 0x20);

	/////////////////////////////////////////////////////
	//////////////////////   BLK   //////////////////////
	/////////////////////////////////////////////////////
	GC5004MIPI_write_cmos_sensor(0x40, 0x22);
	GC5004MIPI_write_cmos_sensor(0x41, 0x00);//38
	
	GC5004MIPI_write_cmos_sensor(0x50, 0x00);
	GC5004MIPI_write_cmos_sensor(0x51, 0x00);
	GC5004MIPI_write_cmos_sensor(0x52, 0x00);
	GC5004MIPI_write_cmos_sensor(0x53, 0x00);
	GC5004MIPI_write_cmos_sensor(0x54, 0x00);
	GC5004MIPI_write_cmos_sensor(0x55, 0x00);
	GC5004MIPI_write_cmos_sensor(0x56, 0x00);
	GC5004MIPI_write_cmos_sensor(0x57, 0x00);
	GC5004MIPI_write_cmos_sensor(0x58, 0x00);
	GC5004MIPI_write_cmos_sensor(0x59, 0x00);
	GC5004MIPI_write_cmos_sensor(0x5a, 0x00);
	GC5004MIPI_write_cmos_sensor(0x5b, 0x00);
	GC5004MIPI_write_cmos_sensor(0x5c, 0x00);
	GC5004MIPI_write_cmos_sensor(0x5d, 0x00);
	GC5004MIPI_write_cmos_sensor(0x5e, 0x00);
	GC5004MIPI_write_cmos_sensor(0x5f, 0x00);
	GC5004MIPI_write_cmos_sensor(0xd0, 0x00);
	GC5004MIPI_write_cmos_sensor(0xd1, 0x00);
	GC5004MIPI_write_cmos_sensor(0xd2, 0x00);
	GC5004MIPI_write_cmos_sensor(0xd3, 0x00);
	GC5004MIPI_write_cmos_sensor(0xd4, 0x00);
	GC5004MIPI_write_cmos_sensor(0xd5, 0x00);
	GC5004MIPI_write_cmos_sensor(0xd6, 0x00);
	GC5004MIPI_write_cmos_sensor(0xd7, 0x00);
	GC5004MIPI_write_cmos_sensor(0xd8, 0x00);
	GC5004MIPI_write_cmos_sensor(0xd9, 0x00);
	GC5004MIPI_write_cmos_sensor(0xda, 0x00);
	GC5004MIPI_write_cmos_sensor(0xdb, 0x00);
	GC5004MIPI_write_cmos_sensor(0xdc, 0x00);
	GC5004MIPI_write_cmos_sensor(0xdd, 0x00);
	GC5004MIPI_write_cmos_sensor(0xde, 0x00);
	GC5004MIPI_write_cmos_sensor(0xdf, 0x00);
	
	GC5004MIPI_write_cmos_sensor(0x70, 0x00);
	GC5004MIPI_write_cmos_sensor(0x71, 0x00);
	GC5004MIPI_write_cmos_sensor(0x72, 0x00);
	GC5004MIPI_write_cmos_sensor(0x73, 0x00);
	GC5004MIPI_write_cmos_sensor(0x74, 0x20);
	GC5004MIPI_write_cmos_sensor(0x75, 0x20);
	GC5004MIPI_write_cmos_sensor(0x76, 0x20);
	GC5004MIPI_write_cmos_sensor(0x77, 0x20);


	/////////////////////////////////////////////////////
	//////////////////////   GAIN   /////////////////////
	/////////////////////////////////////////////////////
	GC5004MIPI_write_cmos_sensor(0xb0, 0x50);
	GC5004MIPI_write_cmos_sensor(0xb1, 0x01);
	GC5004MIPI_write_cmos_sensor(0xb2, 0x02);
	GC5004MIPI_write_cmos_sensor(0xb3, 0x40);
	GC5004MIPI_write_cmos_sensor(0xb4, 0x40);
	GC5004MIPI_write_cmos_sensor(0xb5, 0x40);

	/////////////////////////////////////////////////////
	//////////////////////   SCALER   /////////////////////
	/////////////////////////////////////////////////////
	GC5004MIPI_write_cmos_sensor(0xfe, 0x00);
	GC5004MIPI_write_cmos_sensor(0x18, 0x02);
	GC5004MIPI_write_cmos_sensor(0x80, 0x08);
	GC5004MIPI_write_cmos_sensor(0x84, 0x03);//scaler CFA
	GC5004MIPI_write_cmos_sensor(0x87, 0x12);
	GC5004MIPI_write_cmos_sensor(0x95, 0x07);
	GC5004MIPI_write_cmos_sensor(0x96, 0x98);
	GC5004MIPI_write_cmos_sensor(0x97, 0x0a);
	GC5004MIPI_write_cmos_sensor(0x98, 0x20);
	GC5004MIPI_write_cmos_sensor(0xfe, 0x02);
	GC5004MIPI_write_cmos_sensor(0x86, 0x00);
	
	/////////////////////////////////////////////////////
	//////////////////////   MIPI   /////////////////////
	/////////////////////////////////////////////////////
	GC5004MIPI_write_cmos_sensor(0xfe, 0x03);
	GC5004MIPI_write_cmos_sensor(0x01, 0x07);
	GC5004MIPI_write_cmos_sensor(0x02, 0x33);
	GC5004MIPI_write_cmos_sensor(0x03, 0x93);
	GC5004MIPI_write_cmos_sensor(0x04, 0x80);
	GC5004MIPI_write_cmos_sensor(0x05, 0x02);
	GC5004MIPI_write_cmos_sensor(0x06, 0x80);
	GC5004MIPI_write_cmos_sensor(0x10, 0x93);
	GC5004MIPI_write_cmos_sensor(0x11, 0x2b);
	GC5004MIPI_write_cmos_sensor(0x12, 0xa8);
	GC5004MIPI_write_cmos_sensor(0x13, 0x0c);
	GC5004MIPI_write_cmos_sensor(0x15, 0x12);
	GC5004MIPI_write_cmos_sensor(0x17, 0xb0);
	GC5004MIPI_write_cmos_sensor(0x18, 0x00);
	GC5004MIPI_write_cmos_sensor(0x19, 0x00);
	GC5004MIPI_write_cmos_sensor(0x1a, 0x00);
	GC5004MIPI_write_cmos_sensor(0x1d, 0x00);
	GC5004MIPI_write_cmos_sensor(0x42, 0x20);
	GC5004MIPI_write_cmos_sensor(0x43, 0x0a);
	
	GC5004MIPI_write_cmos_sensor(0x21, 0x01);
	GC5004MIPI_write_cmos_sensor(0x22, 0x02);
	GC5004MIPI_write_cmos_sensor(0x23, 0x01);
	GC5004MIPI_write_cmos_sensor(0x29, 0x02);
	GC5004MIPI_write_cmos_sensor(0x2a, 0x01);
	GC5004MIPI_write_cmos_sensor(0xfe, 0x00);

}
#if 0
{
    
    // The register only need to enable 1 time.    
    spin_lock(&gc5004_drv_lock);  
    GC5004MIPI_Auto_Flicker_mode = KAL_FALSE;     // reset the flicker status    
	spin_unlock(&gc5004_drv_lock);
	{
	//2592x1944
	/////////////////////////////////////////////////////
	//////////////////////   SYS   //////////////////////
	/////////////////////////////////////////////////////
	GC5004MIPI_write_cmos_sensor(0xfe, 0x80);
	GC5004MIPI_write_cmos_sensor(0xfe, 0x80);
	GC5004MIPI_write_cmos_sensor(0xfe, 0x80);
	GC5004MIPI_write_cmos_sensor(0xf2, 0x00); //sync_pad_io_ebi
	GC5004MIPI_write_cmos_sensor(0xf6, 0x00); //up down
	GC5004MIPI_write_cmos_sensor(0xfc, 0x06);
	GC5004MIPI_write_cmos_sensor(0xf7, 0x37); //pll enable
	GC5004MIPI_write_cmos_sensor(0xf8, 0x97); //Pll mode 2
	GC5004MIPI_write_cmos_sensor(0xf9, 0xfe); //[0] pll enable  change at 17:37 04/19
	GC5004MIPI_write_cmos_sensor(0xfa, 0x11); //div
	GC5004MIPI_write_cmos_sensor(0xfe, 0x00);

	/////////////////////////////////////////////////////
	////////////////   ANALOG & CISCTL   ////////////////
	/////////////////////////////////////////////////////
	GC5004MIPI_write_cmos_sensor(0x00, 0x00); //10/[4]rowskip_skip_sh
	GC5004MIPI_write_cmos_sensor(0x03, 0x07); //15fps
	GC5004MIPI_write_cmos_sensor(0x04, 0xd0); 
	GC5004MIPI_write_cmos_sensor(0x05, 0x01); //HB
	GC5004MIPI_write_cmos_sensor(0x06, 0xaf); 
	GC5004MIPI_write_cmos_sensor(0x07, 0x00); //VB
	GC5004MIPI_write_cmos_sensor(0x08, 0x1c);
	GC5004MIPI_write_cmos_sensor(0x0a, 0x02); //02//row start
	GC5004MIPI_write_cmos_sensor(0x0c, 0x00); //0c//col start
	GC5004MIPI_write_cmos_sensor(0x0d, 0x07); 
	GC5004MIPI_write_cmos_sensor(0x0e, 0xa8); 
	GC5004MIPI_write_cmos_sensor(0x0f, 0x0a); //Window setting
	GC5004MIPI_write_cmos_sensor(0x10, 0x50);  
	GC5004MIPI_write_cmos_sensor(0x17, 0x15); //01//14//[0]mirror [1]flip
	GC5004MIPI_write_cmos_sensor(0x18, 0x02); //sdark off
	GC5004MIPI_write_cmos_sensor(0x19, 0x0c); 
	GC5004MIPI_write_cmos_sensor(0x1a, 0x13); 
	GC5004MIPI_write_cmos_sensor(0x1b, 0x48); 
	GC5004MIPI_write_cmos_sensor(0x1c, 0x05); 
	GC5004MIPI_write_cmos_sensor(0x1e, 0xb8);
	GC5004MIPI_write_cmos_sensor(0x1f, 0x78); 
	GC5004MIPI_write_cmos_sensor(0x20, 0xc5); //03/[7:6]ref_r [3:1]comv_r 
	GC5004MIPI_write_cmos_sensor(0x21, 0x4f); //7f
	GC5004MIPI_write_cmos_sensor(0x22, 0x82); //b2 
	GC5004MIPI_write_cmos_sensor(0x23, 0x43); //f1/[7:3]opa_r [1:0]sRef
	GC5004MIPI_write_cmos_sensor(0x24, 0x2f); //PAD drive 
	GC5004MIPI_write_cmos_sensor(0x2b, 0x01); 
	GC5004MIPI_write_cmos_sensor(0x2c, 0x68); //[6:4]rsgh_r 

	/////////////////////////////////////////////////////
	//////////////////////   ISP   //////////////////////
	/////////////////////////////////////////////////////
	GC5004MIPI_write_cmos_sensor(0x86, 0x0a);
	GC5004MIPI_write_cmos_sensor(0x8a, 0x83);
	GC5004MIPI_write_cmos_sensor(0x8c, 0x10);
	GC5004MIPI_write_cmos_sensor(0x8d, 0x01);
	GC5004MIPI_write_cmos_sensor(0x90, 0x01);
	GC5004MIPI_write_cmos_sensor(0x92, 0x00); //00/crop win y
	GC5004MIPI_write_cmos_sensor(0x94, 0x0d); //04/crop win x
	GC5004MIPI_write_cmos_sensor(0x95, 0x07); //crop win height//1944
	GC5004MIPI_write_cmos_sensor(0x96, 0x98);
	GC5004MIPI_write_cmos_sensor(0x97, 0x0a); //crop win width//2560
	GC5004MIPI_write_cmos_sensor(0x98, 0x20);

	/////////////////////////////////////////////////////
	//////////////////////   BLK   //////////////////////
	/////////////////////////////////////////////////////
	GC5004MIPI_write_cmos_sensor(0x40, 0x82);
	GC5004MIPI_write_cmos_sensor(0x41, 0x00);//38
	
	GC5004MIPI_write_cmos_sensor(0x50, 0x00);
	GC5004MIPI_write_cmos_sensor(0x51, 0x00);
	GC5004MIPI_write_cmos_sensor(0x52, 0x00);
	GC5004MIPI_write_cmos_sensor(0x53, 0x00);
	GC5004MIPI_write_cmos_sensor(0x54, 0x00);
	GC5004MIPI_write_cmos_sensor(0x55, 0x00);
	GC5004MIPI_write_cmos_sensor(0x56, 0x00);
	GC5004MIPI_write_cmos_sensor(0x57, 0x00);
	GC5004MIPI_write_cmos_sensor(0x58, 0x00);
	GC5004MIPI_write_cmos_sensor(0x59, 0x00);
	GC5004MIPI_write_cmos_sensor(0x5a, 0x00);
	GC5004MIPI_write_cmos_sensor(0x5b, 0x00);
	GC5004MIPI_write_cmos_sensor(0x5c, 0x00);
	GC5004MIPI_write_cmos_sensor(0x5d, 0x00);
	GC5004MIPI_write_cmos_sensor(0x5e, 0x00);
	GC5004MIPI_write_cmos_sensor(0x5f, 0x00);
	GC5004MIPI_write_cmos_sensor(0xd0, 0x00);
	GC5004MIPI_write_cmos_sensor(0xd1, 0x00);
	GC5004MIPI_write_cmos_sensor(0xd2, 0x00);
	GC5004MIPI_write_cmos_sensor(0xd3, 0x00);
	GC5004MIPI_write_cmos_sensor(0xd4, 0x00);
	GC5004MIPI_write_cmos_sensor(0xd5, 0x00);
	GC5004MIPI_write_cmos_sensor(0xd6, 0x00);
	GC5004MIPI_write_cmos_sensor(0xd7, 0x00);
	GC5004MIPI_write_cmos_sensor(0xd8, 0x00);
	GC5004MIPI_write_cmos_sensor(0xd9, 0x00);
	GC5004MIPI_write_cmos_sensor(0xda, 0x00);
	GC5004MIPI_write_cmos_sensor(0xdb, 0x00);
	GC5004MIPI_write_cmos_sensor(0xdc, 0x00);
	GC5004MIPI_write_cmos_sensor(0xdd, 0x00);
	GC5004MIPI_write_cmos_sensor(0xde, 0x00);
	GC5004MIPI_write_cmos_sensor(0xdf, 0x00);
	
	GC5004MIPI_write_cmos_sensor(0x70, 0x00);
	GC5004MIPI_write_cmos_sensor(0x71, 0x00);
	GC5004MIPI_write_cmos_sensor(0x72, 0x00);
	GC5004MIPI_write_cmos_sensor(0x73, 0x00);
	GC5004MIPI_write_cmos_sensor(0x74, 0x20);
	GC5004MIPI_write_cmos_sensor(0x75, 0x20);
	GC5004MIPI_write_cmos_sensor(0x76, 0x20);
	GC5004MIPI_write_cmos_sensor(0x77, 0x20);


	/////////////////////////////////////////////////////
	//////////////////////   GAIN   /////////////////////
	/////////////////////////////////////////////////////
	GC5004MIPI_write_cmos_sensor(0xb0, 0x50);
	GC5004MIPI_write_cmos_sensor(0xb1, 0x01);
	GC5004MIPI_write_cmos_sensor(0xb2, 0x02);
	GC5004MIPI_write_cmos_sensor(0xb3, 0x40);
	GC5004MIPI_write_cmos_sensor(0xb4, 0x40);
	GC5004MIPI_write_cmos_sensor(0xb5, 0x40);

	/////////////////////////////////////////////////////
	//////////////////////   SCALER   /////////////////////
	/////////////////////////////////////////////////////
	GC5004MIPI_write_cmos_sensor(0xfe, 0x00);
	GC5004MIPI_write_cmos_sensor(0x18, 0x02);
	GC5004MIPI_write_cmos_sensor(0x80, 0x08);
	GC5004MIPI_write_cmos_sensor(0x84, 0x03);//scaler CFA
	GC5004MIPI_write_cmos_sensor(0x87, 0x12);
	GC5004MIPI_write_cmos_sensor(0x95, 0x07);
	GC5004MIPI_write_cmos_sensor(0x96, 0x98);
	GC5004MIPI_write_cmos_sensor(0x97, 0x0a);
	GC5004MIPI_write_cmos_sensor(0x98, 0x20);
	GC5004MIPI_write_cmos_sensor(0xfe, 0x02);
	GC5004MIPI_write_cmos_sensor(0x86, 0x00);
	
	/////////////////////////////////////////////////////
	//////////////////////   MIPI   /////////////////////
	/////////////////////////////////////////////////////
	GC5004MIPI_write_cmos_sensor(0xfe, 0x03);
	GC5004MIPI_write_cmos_sensor(0x01, 0x07);
	GC5004MIPI_write_cmos_sensor(0x02, 0x33);
	GC5004MIPI_write_cmos_sensor(0x03, 0x93);
	GC5004MIPI_write_cmos_sensor(0x04, 0x80);
	GC5004MIPI_write_cmos_sensor(0x05, 0x02);
	GC5004MIPI_write_cmos_sensor(0x06, 0x80);
	GC5004MIPI_write_cmos_sensor(0x10, 0x93);
	GC5004MIPI_write_cmos_sensor(0x11, 0x2b);
	GC5004MIPI_write_cmos_sensor(0x12, 0xa8);
	GC5004MIPI_write_cmos_sensor(0x13, 0x0c);
	GC5004MIPI_write_cmos_sensor(0x15, 0x12);
	GC5004MIPI_write_cmos_sensor(0x17, 0xb0);
	GC5004MIPI_write_cmos_sensor(0x18, 0x00);
	GC5004MIPI_write_cmos_sensor(0x19, 0x00);
	GC5004MIPI_write_cmos_sensor(0x1a, 0x00);
	GC5004MIPI_write_cmos_sensor(0x1d, 0x00);
	GC5004MIPI_write_cmos_sensor(0x42, 0x20);
	GC5004MIPI_write_cmos_sensor(0x43, 0x0a);
	
	GC5004MIPI_write_cmos_sensor(0x21, 0x01);
	GC5004MIPI_write_cmos_sensor(0x22, 0x02);
	GC5004MIPI_write_cmos_sensor(0x23, 0x01);
	GC5004MIPI_write_cmos_sensor(0x29, 0x02);
	GC5004MIPI_write_cmos_sensor(0x2a, 0x01);
	GC5004MIPI_write_cmos_sensor(0xfe, 0x00);

}

   // printk("[GC5004MIPIRaw] Init Success \n");
}   /*  GC5004MIPI_Sensor_Init  */
#endif

void GC5004VideoFullSizeSetting(void)//16:9   6M
#if 0
{
	//1296x972
	GC5004MIPI_write_cmos_sensor(0xf7, 0xff);
	GC5004MIPI_write_cmos_sensor(0x18, 0x42);//skip on
	GC5004MIPI_write_cmos_sensor(0x80, 0x18);//08//scaler on
	GC5004MIPI_write_cmos_sensor(0x89, 0x03);//03
	GC5004MIPI_write_cmos_sensor(0x8b, 0x61);//ad
	GC5004MIPI_write_cmos_sensor(0x40, 0x02);
  

	GC5004MIPI_write_cmos_sensor(0x95, 0x03);
	GC5004MIPI_write_cmos_sensor(0x96, 0xcc);
	GC5004MIPI_write_cmos_sensor(0x97, 0x05);
	GC5004MIPI_write_cmos_sensor(0x98, 0x10);

	GC5004MIPI_write_cmos_sensor(0xfe, 0x03);
	GC5004MIPI_write_cmos_sensor(0x04, 0x40);
	GC5004MIPI_write_cmos_sensor(0x05, 0x01);
	GC5004MIPI_write_cmos_sensor(0x12, 0x54);
	GC5004MIPI_write_cmos_sensor(0x13, 0x06);
	GC5004MIPI_write_cmos_sensor(0x42, 0x10);
	GC5004MIPI_write_cmos_sensor(0x43, 0x05);
	GC5004MIPI_write_cmos_sensor(0xfe, 0x00);
}

#endif
#if 0
{
	//1920x1080
	GC5004MIPI_write_cmos_sensor(0xf7, 0x1d);
	GC5004MIPI_write_cmos_sensor(0xf8, 0x84); //Pll mode 2
	GC5004MIPI_write_cmos_sensor(0xfa, 0x00);
	
	GC5004MIPI_write_cmos_sensor(0xfe, 0x00);//page0
	GC5004MIPI_write_cmos_sensor(0x05, 0x01); //HB
	GC5004MIPI_write_cmos_sensor(0x06, 0x13); 
	GC5004MIPI_write_cmos_sensor(0x07, 0x00); //VB
	GC5004MIPI_write_cmos_sensor(0x08, 0x1c);
	GC5004MIPI_write_cmos_sensor(0x09, 0x01);
	GC5004MIPI_write_cmos_sensor(0x0a, 0xb0); //02//row start
	GC5004MIPI_write_cmos_sensor(0x0b, 0x01);
	GC5004MIPI_write_cmos_sensor(0x0c, 0x10); //0c//col start
	GC5004MIPI_write_cmos_sensor(0x0d, 0x04); 
	GC5004MIPI_write_cmos_sensor(0x0e, 0x48); 
	GC5004MIPI_write_cmos_sensor(0x0f, 0x07); //Window setting
	GC5004MIPI_write_cmos_sensor(0x10, 0xd0);  

	GC5004MIPI_write_cmos_sensor(0x18, 0x02);//skip off
	GC5004MIPI_write_cmos_sensor(0x80, 0x10);//scaler off
	GC5004MIPI_write_cmos_sensor(0x89, 0x03);
	GC5004MIPI_write_cmos_sensor(0x8b, 0x61);
	GC5004MIPI_write_cmos_sensor(0x40, 0x22);
  
	GC5004MIPI_write_cmos_sensor(0x94, 0x4d);
	GC5004MIPI_write_cmos_sensor(0x95, 0x04);
	GC5004MIPI_write_cmos_sensor(0x96, 0x38);
	GC5004MIPI_write_cmos_sensor(0x97, 0x07);
	GC5004MIPI_write_cmos_sensor(0x98, 0x80);

	GC5004MIPI_write_cmos_sensor(0xfe, 0x03);
	GC5004MIPI_write_cmos_sensor(0x04, 0xe0);
	GC5004MIPI_write_cmos_sensor(0x05, 0x01);
	GC5004MIPI_write_cmos_sensor(0x12, 0x60);
	GC5004MIPI_write_cmos_sensor(0x13, 0x09);
	GC5004MIPI_write_cmos_sensor(0x42, 0x80);
	GC5004MIPI_write_cmos_sensor(0x43, 0x07);
	GC5004MIPI_write_cmos_sensor(0xfe, 0x00);
}
#else
{
	GC5004MIPI_write_cmos_sensor(0xf7, 0xff);
	GC5004MIPI_write_cmos_sensor(0xf8, 0x97); //Pll mode 2
	GC5004MIPI_write_cmos_sensor(0xfa, 0x11);
	GC5004MIPI_write_cmos_sensor(0xfe, 0x00);//page0
//	GC5004MIPI_write_cmos_sensor(0x03, 0x07); //15fps
//	GC5004MIPI_write_cmos_sensor(0x04, 0xd0); 
	GC5004MIPI_write_cmos_sensor(0x05, 0x01); //HB
	GC5004MIPI_write_cmos_sensor(0x06, 0xaf); 
	GC5004MIPI_write_cmos_sensor(0x07, 0x00); //VB
	GC5004MIPI_write_cmos_sensor(0x08, 0x90);
	GC5004MIPI_write_cmos_sensor(0x09, 0x00);
	GC5004MIPI_write_cmos_sensor(0x0a, 0x02); //02//row start
	GC5004MIPI_write_cmos_sensor(0x0b, 0x00);
	GC5004MIPI_write_cmos_sensor(0x0c, 0x00); //0c//col start
	GC5004MIPI_write_cmos_sensor(0x0d, 0x07); 
	GC5004MIPI_write_cmos_sensor(0x0e, 0xa8); 
	GC5004MIPI_write_cmos_sensor(0x0f, 0x0a); //Window setting
	GC5004MIPI_write_cmos_sensor(0x10, 0x50);  
	GC5004MIPI_write_cmos_sensor(0x18, 0x42);//skip on
	GC5004MIPI_write_cmos_sensor(0x80, 0x18);//08//scaler on
	GC5004MIPI_write_cmos_sensor(0x89, 0x03);//03
	GC5004MIPI_write_cmos_sensor(0x8b, 0x61);//ad
	GC5004MIPI_write_cmos_sensor(0x40, 0x22);
	GC5004MIPI_write_cmos_sensor(0x94, 0x0c);
	GC5004MIPI_write_cmos_sensor(0x95, 0x03);//output winow
	GC5004MIPI_write_cmos_sensor(0x96, 0xcc);
	GC5004MIPI_write_cmos_sensor(0x97, 0x05);
	GC5004MIPI_write_cmos_sensor(0x98, 0x10);
	GC5004MIPI_write_cmos_sensor(0xfe, 0x03);
	GC5004MIPI_write_cmos_sensor(0x04, 0x40);
	GC5004MIPI_write_cmos_sensor(0x05, 0x01);
	GC5004MIPI_write_cmos_sensor(0x12, 0x54);
	GC5004MIPI_write_cmos_sensor(0x13, 0x06);
	GC5004MIPI_write_cmos_sensor(0x42, 0x10);
	GC5004MIPI_write_cmos_sensor(0x43, 0x05);
	GC5004MIPI_write_cmos_sensor(0xfe, 0x00);
	mdelay(200);
}
#endif

void GC5004PreviewSetting(void)
#if 0
{
	//1296x972
	GC5004MIPI_write_cmos_sensor(0xf7, 0xff);
	GC5004MIPI_write_cmos_sensor(0x18, 0x42);//skip on
	GC5004MIPI_write_cmos_sensor(0x80, 0x18);//08//scaler on
	GC5004MIPI_write_cmos_sensor(0x89, 0x03);//03
	GC5004MIPI_write_cmos_sensor(0x8b, 0x61);//ad
	GC5004MIPI_write_cmos_sensor(0x40, 0x02);
  
	

  
	GC5004MIPI_write_cmos_sensor(0x95, 0x03);//output winow
	GC5004MIPI_write_cmos_sensor(0x96, 0xcc);
	GC5004MIPI_write_cmos_sensor(0x97, 0x05);
	GC5004MIPI_write_cmos_sensor(0x98, 0x10);

	GC5004MIPI_write_cmos_sensor(0xfe, 0x03);
	GC5004MIPI_write_cmos_sensor(0x04, 0x40);
	GC5004MIPI_write_cmos_sensor(0x05, 0x01);
	GC5004MIPI_write_cmos_sensor(0x12, 0x54);
	GC5004MIPI_write_cmos_sensor(0x13, 0x06);
	GC5004MIPI_write_cmos_sensor(0x42, 0x10);
	GC5004MIPI_write_cmos_sensor(0x43, 0x05);
	GC5004MIPI_write_cmos_sensor(0xfe, 0x00);
}
#endif
{
	//1296x972
	GC5004MIPI_write_cmos_sensor(0xf7, 0xff);
	GC5004MIPI_write_cmos_sensor(0xf8, 0x97); //Pll mode 2
	GC5004MIPI_write_cmos_sensor(0xfa, 0x11);
	GC5004MIPI_write_cmos_sensor(0xfe, 0x00);//page0
//	GC5004MIPI_write_cmos_sensor(0x03, 0x07); //15fps
//	GC5004MIPI_write_cmos_sensor(0x04, 0xd0); 
	GC5004MIPI_write_cmos_sensor(0x05, 0x01); //HB
	GC5004MIPI_write_cmos_sensor(0x06, 0xaf); 
	GC5004MIPI_write_cmos_sensor(0x07, 0x00); //VB
	GC5004MIPI_write_cmos_sensor(0x08, 0x90);
	GC5004MIPI_write_cmos_sensor(0x09, 0x00);
	GC5004MIPI_write_cmos_sensor(0x0a, 0x02); //02//row start
	GC5004MIPI_write_cmos_sensor(0x0b, 0x00);
	GC5004MIPI_write_cmos_sensor(0x0c, 0x00); //0c//col start
	GC5004MIPI_write_cmos_sensor(0x0d, 0x07); 
	GC5004MIPI_write_cmos_sensor(0x0e, 0xa8); 
	GC5004MIPI_write_cmos_sensor(0x0f, 0x0a); //Window setting

	GC5004MIPI_write_cmos_sensor(0x10, 0x50);  
	GC5004MIPI_write_cmos_sensor(0x18, 0x42);//skip on
	GC5004MIPI_write_cmos_sensor(0x80, 0x18);//08//scaler on
	GC5004MIPI_write_cmos_sensor(0x89, 0x03);//03
	GC5004MIPI_write_cmos_sensor(0x8b, 0x61);//ad
	GC5004MIPI_write_cmos_sensor(0x40, 0x22);
	
	GC5004MIPI_write_cmos_sensor(0x94, 0x0c);
	GC5004MIPI_write_cmos_sensor(0x95, 0x03);
	GC5004MIPI_write_cmos_sensor(0x96, 0xcc);
	GC5004MIPI_write_cmos_sensor(0x97, 0x05);
	GC5004MIPI_write_cmos_sensor(0x98, 0x10);

	GC5004MIPI_write_cmos_sensor(0xfe, 0x03);
	GC5004MIPI_write_cmos_sensor(0x04, 0x40);
	GC5004MIPI_write_cmos_sensor(0x05, 0x01);
	GC5004MIPI_write_cmos_sensor(0x12, 0x54);
	GC5004MIPI_write_cmos_sensor(0x13, 0x06);
	GC5004MIPI_write_cmos_sensor(0x42, 0x10);
	GC5004MIPI_write_cmos_sensor(0x43, 0x05);
	GC5004MIPI_write_cmos_sensor(0xfe, 0x00);
	mdelay(200);
}


void GC5004MIPI_set_5M(void)
#if 0
{
		//2592x1944
		GC5004MIPI_write_cmos_sensor(0xf7, 0x37);
		GC5004MIPI_write_cmos_sensor(0x18, 0x02);//skip off
		GC5004MIPI_write_cmos_sensor(0x80, 0x10);//scaler off
		GC5004MIPI_write_cmos_sensor(0x89, 0x03);//13
		GC5004MIPI_write_cmos_sensor(0x8b, 0x61);//e9
		GC5004MIPI_write_cmos_sensor(0x40, 0x82);
		
		
		GC5004MIPI_write_cmos_sensor(0x95, 0x07);
		GC5004MIPI_write_cmos_sensor(0x96, 0x98);
		GC5004MIPI_write_cmos_sensor(0x97, 0x0a);
		GC5004MIPI_write_cmos_sensor(0x98, 0x20);
	
		GC5004MIPI_write_cmos_sensor(0xfe, 0x03);
		GC5004MIPI_write_cmos_sensor(0x04, 0x80);
		GC5004MIPI_write_cmos_sensor(0x05, 0x02);
		GC5004MIPI_write_cmos_sensor(0x12, 0xa8);
		GC5004MIPI_write_cmos_sensor(0x13, 0x0c);
		GC5004MIPI_write_cmos_sensor(0x42, 0x20);
		GC5004MIPI_write_cmos_sensor(0x43, 0x0a);
		GC5004MIPI_write_cmos_sensor(0xfe, 0x00);
	}
#endif
{
	//2592x1944
	GC5004MIPI_write_cmos_sensor(0xf7, 0x1d);
	GC5004MIPI_write_cmos_sensor(0xf8, 0x84); //pll enable
	GC5004MIPI_write_cmos_sensor(0xfa, 0x00);
	GC5004MIPI_write_cmos_sensor(0xfe, 0x00);//page0
	//GC5004MIPI_write_cmos_sensor(0x03, 0x07); //15fps
	//GC5004MIPI_write_cmos_sensor(0x04, 0xd0); 

	GC5004MIPI_write_cmos_sensor(0x05, 0x01); //HB
	GC5004MIPI_write_cmos_sensor(0x06, 0xfa); 
	GC5004MIPI_write_cmos_sensor(0x07, 0x00); //VB
	GC5004MIPI_write_cmos_sensor(0x08, 0x1c);
	GC5004MIPI_write_cmos_sensor(0x09, 0x00);
	GC5004MIPI_write_cmos_sensor(0x0a, 0x02); //02//row start
	GC5004MIPI_write_cmos_sensor(0x0b, 0x00);
	GC5004MIPI_write_cmos_sensor(0x0c, 0x00); //0c//col start
	GC5004MIPI_write_cmos_sensor(0x0d, 0x07); 
	GC5004MIPI_write_cmos_sensor(0x0e, 0xa8); 
	GC5004MIPI_write_cmos_sensor(0x0f, 0x0a); //Window setting
	GC5004MIPI_write_cmos_sensor(0x10, 0x50);  

	GC5004MIPI_write_cmos_sensor(0x18, 0x02);//skip off
	GC5004MIPI_write_cmos_sensor(0x80, 0x10);//scaler off
	GC5004MIPI_write_cmos_sensor(0x89, 0x03);//13
	GC5004MIPI_write_cmos_sensor(0x8b, 0x61);//e9
	GC5004MIPI_write_cmos_sensor(0x40, 0x22);
	
	GC5004MIPI_write_cmos_sensor(0x94, 0x0c);
	GC5004MIPI_write_cmos_sensor(0x95, 0x07);
	GC5004MIPI_write_cmos_sensor(0x96, 0x98);
	GC5004MIPI_write_cmos_sensor(0x97, 0x0a);
	GC5004MIPI_write_cmos_sensor(0x98, 0x20);

	GC5004MIPI_write_cmos_sensor(0xfe, 0x03);
	GC5004MIPI_write_cmos_sensor(0x04, 0x80);
	GC5004MIPI_write_cmos_sensor(0x05, 0x02);
	GC5004MIPI_write_cmos_sensor(0x12, 0xa8);
	GC5004MIPI_write_cmos_sensor(0x13, 0x0c);
	GC5004MIPI_write_cmos_sensor(0x42, 0x20);
	GC5004MIPI_write_cmos_sensor(0x43, 0x0a);
	GC5004MIPI_write_cmos_sensor(0xfe, 0x00);
	mdelay(200);
}

/*****************************************************************************/
/* Windows Mobile Sensor Interface */
/*****************************************************************************/
/*************************************************************************
* FUNCTION
*   GC5004MIPIOpen
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

UINT32 GC5004MIPIOpen(void)
{
   int  retry = 0; 
	kal_uint16 sensorid;
    // check if sensor ID correct
    retry = 3; 
    do {
       SENSORDB("Read ID in the Open function"); 
	   sensorid=(kal_uint16)((GC5004MIPI_read_cmos_sensor(0xf0)<<8) | GC5004MIPI_read_cmos_sensor(0xf1));  
	   spin_lock(&gc5004_drv_lock);    
	   GC5004MIPI_sensor_id =sensorid;
	   spin_unlock(&gc5004_drv_lock);
		if (GC5004MIPI_sensor_id == GC5004MIPI_SENSOR_ID)
		break; 
		SENSORDB("Read Sensor ID Fail = 0x%04x\n", GC5004MIPI_sensor_id); 
		retry--; 
	    }
	while (retry > 0);
    SENSORDB("Read Sensor ID = 0x%04x\n", GC5004MIPI_sensor_id); 
    if (GC5004MIPI_sensor_id != GC5004MIPI_SENSOR_ID)
        return ERROR_SENSOR_CONNECT_FAIL;
    GC5004MIPI_Sensor_Init();
    GC5004MIPI_Para_Init();
	sensorid=read_GC5004MIPI_gain();
	spin_lock(&gc5004_drv_lock);	
    GC5004MIPI_sensor_gain_base = sensorid;
	spin_unlock(&gc5004_drv_lock);
	
    return ERROR_NONE;
}

/*************************************************************************
* FUNCTION
*   GC5004MIPIGetSensorID
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
UINT32 GC5004MIPIGetSensorID(UINT32 *sensorID) 
{
   int  retry = 3; 
    // check if sensor ID correct
    do {		
	   *sensorID =(kal_uint16)((GC5004MIPI_read_cmos_sensor(0xf0)<<8) | GC5004MIPI_read_cmos_sensor(0xf1)); 
        if (*sensorID == GC5004MIPI_SENSOR_ID)
            break;
        SENSORDB("Read Sensor ID Fail = 0x%04x\n", *sensorID); 
        retry--; 
		
    	} while (retry > 0);

    if (*sensorID != GC5004MIPI_SENSOR_ID) {
        *sensorID = 0xFFFFFFFF; 
        return ERROR_SENSOR_CONNECT_FAIL;
    }
	 *sensorID =(kal_uint16)((GC5004MIPI_read_cmos_sensor(0xf0)<<8) | GC5004MIPI_read_cmos_sensor(0xf1)); 
 	SENSORDB("Read Sensor ID  = 0x%04x\n", *sensorID); 
    return ERROR_NONE;
}


/*************************************************************************
* FUNCTION
*   GC5004MIPI_SetShutter
*
* DESCRIPTION
*   This function set e-shutter of GC5004MIPI to change exposure time.
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
void GC5004MIPI_SetShutter(kal_uint16 iShutter)
{

	 SENSORDB("[GC5004MIPI]%s():shutter=%d\n",__FUNCTION__,iShutter);
	 spin_lock(&gc5004_drv_lock);	 
	 GC5004MIPI_sensor.pv_shutter = iShutter;
	 spin_unlock(&gc5004_drv_lock);
	// SENSORDB("[GC5004MIPI]%s():pvmode=%d,videomode=%d,capturemode=%d\n",__FUNCTION__,GC5004MIPI_sensor.pv_mode,GC5004MIPI_sensor.video_mode,GC5004MIPI_sensor.capture_mode);
	// if(GC5004MIPI_sensor.capture_mode!=TRUE)
	 //	{
	
	//if(iShutter < 1) iShutter = 1;
	///if(iShutter > 8191) iShutter = 8191;//2^13
		iShutter = iShutter / 4+(((iShutter%4)/4)+1/2);
		iShutter = iShutter * 4;
		
		if (iShutter<4) iShutter = 4; /* avoid 0 */
		if(iShutter > 8191) iShutter = 8188;//2^13
		//}
	//	}
	SENSORDB("[GC5004MIPI]%s():after *2shutter=%d\n",__FUNCTION__,iShutter);

	//Update Shutter
	GC5004MIPI_write_cmos_sensor(0xfe,0x00);
	GC5004MIPI_write_cmos_sensor(0x04, (iShutter) & 0xFF);
	GC5004MIPI_write_cmos_sensor(0x03, (iShutter >> 8) & 0x1F);
  //  kal_uint16 debugshutter,debugHB,debugVB;
	//debugshutter=(GC5004MIPI_read_cmos_sensor(0x03)&0x1F)<<8|GC5004MIPI_read_cmos_sensor(0x04);
	//debugHB=(GC5004MIPI_read_cmos_sensor(0x05)&0x0F)<<8|GC5004MIPI_read_cmos_sensor(0x06);
	//debugVB=(GC5004MIPI_read_cmos_sensor(0x07)&0x1F)<<8|GC5004MIPI_read_cmos_sensor(0x08);
	//SENSORDB("[GC5004MIPI]%s():debugshutter=%d,debugsHB=%d,debugVB=%d\n",__FUNCTION__,debugshutter,debugHB,debugVB);
}   /*  GC5004MIPI_SetShutter   */



/*************************************************************************
* FUNCTION
*   GC5004MIPI_read_shutter
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
UINT16 GC5004MIPI_read_shutter(void)
{
	kal_uint16 shutter,temp_reg1,temp_reg2;
	temp_reg1 = GC5004MIPI_read_cmos_sensor(0x03);
	temp_reg2 = GC5004MIPI_read_cmos_sensor(0x04);
	shutter = ((temp_reg1<<8)&0x1F00)|(temp_reg2&0xFF);
	return shutter;
}


/*************************************************************************
* FUNCTION
*   GC5004MIPI_night_mode
*
* DESCRIPTION
*   This function night mode of GC5004MIPI.
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
void GC5004MIPI_NightMode(kal_bool bEnable)
{
#if 0
    /************************************************************************/
    /*                      Auto Mode: 30fps                                                                                          */
    /*                      Night Mode:15fps                                                                                          */
    /************************************************************************/
    if(bEnable)
    {
        if(OV5642_MPEG4_encode_mode==KAL_TRUE)
        {
            OV5642_MAX_EXPOSURE_LINES = (kal_uint16)((OV5642_sensor_pclk/15)/(OV5642_PV_PERIOD_PIXEL_NUMS+OV5642_PV_dummy_pixels));
            OV5642_write_cmos_sensor(0x350C, (OV5642_MAX_EXPOSURE_LINES >> 8) & 0xFF);
            OV5642_write_cmos_sensor(0x350D, OV5642_MAX_EXPOSURE_LINES & 0xFF);
            OV5642_CURRENT_FRAME_LINES = OV5642_MAX_EXPOSURE_LINES;
            OV5642_MAX_EXPOSURE_LINES = OV5642_CURRENT_FRAME_LINES - OV5642_SHUTTER_LINES_GAP;
        }
    }
    else// Fix video framerate 30 fps
    {
        if(OV5642_MPEG4_encode_mode==KAL_TRUE)
        {
            OV5642_MAX_EXPOSURE_LINES = (kal_uint16)((OV5642_sensor_pclk/30)/(OV5642_PV_PERIOD_PIXEL_NUMS+OV5642_PV_dummy_pixels));
            if(OV5642_pv_exposure_lines < (OV5642_MAX_EXPOSURE_LINES - OV5642_SHUTTER_LINES_GAP)) // for avoid the shutter > frame_lines,move the frame lines setting to shutter function
            {
                OV5642_write_cmos_sensor(0x350C, (OV5642_MAX_EXPOSURE_LINES >> 8) & 0xFF);
                OV5642_write_cmos_sensor(0x350D, OV5642_MAX_EXPOSURE_LINES & 0xFF);
                OV5642_CURRENT_FRAME_LINES = OV5642_MAX_EXPOSURE_LINES;
            }
            OV5642_MAX_EXPOSURE_LINES = OV5642_MAX_EXPOSURE_LINES - OV5642_SHUTTER_LINES_GAP;
        }
    }
#endif	
}/*	GC5004MIPI_NightMode */



/*************************************************************************
* FUNCTION
*   GC5004MIPIClose
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
UINT32 GC5004MIPIClose(void)
{
   // GC5004MIPI_write_cmos_sensor(0x0100,0x00);
    return ERROR_NONE;
}	/* GC5004MIPIClose() */

void GC5004MIPISetFlipMirror(kal_int32 imgMirror)
{
	GC5004MIPI_sensor.mirror_flip=imgMirror;
    switch (imgMirror)
	{
		case IMAGE_NORMAL://IMAGE_V_MIRROR:
		   GC5004MIPI_write_cmos_sensor(0x17,0x14);
		   GC5004MIPI_write_cmos_sensor(0x92,0x03);
		   GC5004MIPI_write_cmos_sensor(0x94,0x07);
		    break;
		case IMAGE_H_MIRROR://IMAGE_NORMAL:
		   GC5004MIPI_write_cmos_sensor(0x17,0x15);
		   GC5004MIPI_write_cmos_sensor(0x92,0x03);
		   GC5004MIPI_write_cmos_sensor(0x94,0x06);
		    break;
		case IMAGE_V_MIRROR://IMAGE_HV_MIRROR:
		   GC5004MIPI_write_cmos_sensor(0x17,0x16);
		   GC5004MIPI_write_cmos_sensor(0x92,0x02);
		   GC5004MIPI_write_cmos_sensor(0x94,0x07);
		    break;
		case IMAGE_HV_MIRROR://IMAGE_H_MIRROR:
		   GC5004MIPI_write_cmos_sensor(0x17,0x17);
		   GC5004MIPI_write_cmos_sensor(0x92,0x02);
		   GC5004MIPI_write_cmos_sensor(0x94,0x06);
		    break;
	}
}


/*************************************************************************
* FUNCTION
*   GC5004MIPIPreview
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
UINT32 GC5004MIPIPreview(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
                                                MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
    kal_uint16 iStartX = 0, iStartY = 0;
	
	SENSORDB("shutter&gain test by hhl: enter the preview function GC5004MIPIPreview\n"); 
    
	 
    	spin_lock(&gc5004_drv_lock);    
		GC5004MIPI_sensor.video_mode=KAL_FALSE;
		GC5004MIPI_sensor.pv_mode=KAL_TRUE;
		GC5004MIPI_sensor.capture_mode=KAL_FALSE;
		GC5004MIPI_sensor.pv_HB=431;
		GC5004MIPI_sensor.pv_VB=144;
		spin_unlock(&gc5004_drv_lock);
        GC5004PreviewSetting();
		//iStartX += GC5004MIPI_IMAGE_SENSOR_PV_STARTX;
		//iStartY += GC5004MIPI_IMAGE_SENSOR_PV_STARTY;
		spin_lock(&gc5004_drv_lock);	
		memcpy(&GC5004MIPISensorConfigData, sensor_config_data, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
		spin_unlock(&gc5004_drv_lock);
		//image_window->GrabStartX= iStartX;
		//image_window->GrabStartY= iStartY;
	//	image_window->ExposureWindowWidth= GC5004MIPI_IMAGE_SENSOR_PV_WIDTH - 2*iStartX;
	//	image_window->ExposureWindowHeight= GC5004MIPI_IMAGE_SENSOR_PV_HEIGHT - 2*iStartY;
		SENSORDB("Preview resolution:%d %d %d %d\n", image_window->GrabStartX, image_window->GrabStartY, image_window->ExposureWindowWidth, image_window->ExposureWindowHeight); 
		SENSORDB("shutter&gain test by hhl: exit the preview function GC5004MIPIPreview\n"); 
	return ERROR_NONE;
}	/* GC5004MIPIPreview() */



/*************************************************************************
* FUNCTION
* RETURNS
*   None
*
* GLOBALS AFFECTED
*
*************************************************************************/
UINT32 GC5004MIPIVideo(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
                                                MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
    kal_uint16 iStartX = 0, iStartY = 0;
	SENSORDB("shutter&gain test by hhl: enter the video function GC5004MIPIPreview\n"); 
	   	
    	spin_lock(&gc5004_drv_lock);    
		GC5004MIPI_sensor.video_mode=KAL_TRUE;
		GC5004MIPI_sensor.pv_mode=KAL_FALSE;
		GC5004MIPI_sensor.capture_mode=KAL_FALSE;
		GC5004MIPI_sensor.video_HB=431;
		GC5004MIPI_sensor.video_VB=144;

		spin_unlock(&gc5004_drv_lock);
		GC5004VideoFullSizeSetting();
		//iStartX += GC5004MIPI_IMAGE_SENSOR_VIDEO_STARTX;
		//iStartY += GC5004MIPI_IMAGE_SENSOR_VIDEO_STARTY;
		spin_lock(&gc5004_drv_lock);	
		memcpy(&GC5004MIPISensorConfigData, sensor_config_data, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
		spin_unlock(&gc5004_drv_lock);
		//image_window->GrabStartX= iStartX;
		//image_window->GrabStartY= iStartY;
	//	image_window->ExposureWindowWidth= GC5004MIPI_IMAGE_SENSOR_PV_WIDTH - 2*iStartX;
	//	image_window->ExposureWindowHeight= GC5004MIPI_IMAGE_SENSOR_PV_HEIGHT - 2*iStartY;
		SENSORDB("VideoPreview resolution:%d %d %d %d\n", image_window->GrabStartX, image_window->GrabStartY, image_window->ExposureWindowWidth, image_window->ExposureWindowHeight); 
		SENSORDB("shutter&gain test by hhl: exit the preview function GC5004MIPIPreview\n");    
	return ERROR_NONE;
}	/* GC5004MIPIPreview() */



UINT32 GC5004MIPICapture(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
                                                MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{

  //  kal_uint32 shutter=GC5004MIPI_sensor.pv_shutter;
    kal_uint16 iStartX = 0, iStartY = 0;
	
	SENSORDB("shutter&gain test by hhl: enter the capture function GC5004MIPICapture\n"); 
	spin_lock(&gc5004_drv_lock);	
	GC5004MIPI_sensor.video_mode=KAL_FALSE;
	GC5004MIPI_sensor.pv_mode=KAL_FALSE;
	GC5004MIPI_sensor.capture_mode=KAL_TRUE;
	GC5004MIPI_sensor.cp_HB=506;
	GC5004MIPI_sensor.cp_VB=28;
	spin_unlock(&gc5004_drv_lock);
     GC5004MIPI_set_5M();
	iStartX = GC5004MIPI_IMAGE_SENSOR_CAP_STARTX;
	iStartY = GC5004MIPI_IMAGE_SENSOR_CAP_STARTY;
	spin_lock(&gc5004_drv_lock);	
    memcpy(&GC5004MIPISensorConfigData, sensor_config_data, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
	spin_unlock(&gc5004_drv_lock);
	SENSORDB("shutter&gain test by hhl: exit the capture function GC5004MIPICapture\n"); 

    return ERROR_NONE;
	
}	/* GC5004MIPICapture() */

UINT32 GC5004MIPIGetResolution(MSDK_SENSOR_RESOLUTION_INFO_STRUCT *pSensorResolution)
{

    pSensorResolution->SensorPreviewWidth	= GC5004MIPI_REAL_PV_WIDTH;
    pSensorResolution->SensorPreviewHeight	= GC5004MIPI_REAL_PV_HEIGHT;
    pSensorResolution->SensorFullWidth		= GC5004MIPI_REAL_CAP_WIDTH;
    pSensorResolution->SensorFullHeight		= GC5004MIPI_REAL_CAP_HEIGHT;
    pSensorResolution->SensorVideoWidth		= GC5004MIPI_REAL_VIDEO_WIDTH;
    pSensorResolution->SensorVideoHeight    = GC5004MIPI_REAL_VIDEO_HEIGHT;
    SENSORDB("GC5004MIPIGetResolution :8-14");    

    return ERROR_NONE;
}   /* GC5004MIPIGetResolution() */

UINT32 GC5004MIPIGetInfo(MSDK_SCENARIO_ID_ENUM ScenarioId,
                                                MSDK_SENSOR_INFO_STRUCT *pSensorInfo,
                                                MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData)
{
	switch(ScenarioId){
			case MSDK_SCENARIO_ID_CAMERA_ZSD:
			case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG://hhl 2-28
				pSensorInfo->SensorFullResolutionX=GC5004MIPI_REAL_CAP_WIDTH;
				pSensorInfo->SensorFullResolutionY=GC5004MIPI_REAL_CAP_HEIGHT;
				pSensorInfo->SensorStillCaptureFrameRate=22;
			break;//hhl 2-28
			case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
				pSensorInfo->SensorPreviewResolutionX=GC5004MIPI_REAL_VIDEO_WIDTH;
				pSensorInfo->SensorPreviewResolutionY=GC5004MIPI_REAL_VIDEO_HEIGHT;
				pSensorInfo->SensorCameraPreviewFrameRate=30;
			break;
		default:
        pSensorInfo->SensorPreviewResolutionX=GC5004MIPI_REAL_PV_WIDTH;
        pSensorInfo->SensorPreviewResolutionY=GC5004MIPI_REAL_PV_HEIGHT;
		pSensorInfo->SensorCameraPreviewFrameRate=30;
			break;
	}
//	pSensorInfo->SensorPreviewResolutionX=GC5004MIPI_REAL_CAP_WIDTH;
   //     pSensorInfo->SensorPreviewResolutionY=GC5004MIPI_REAL_CAP_HEIGHT;

    pSensorInfo->SensorVideoFrameRate=30;
//    pSensorInfo->SensorStillCaptureFrameRate=22;
//    pSensorInfo->SensorWebCamCaptureFrameRate=22;//hhl 2-28
	
    pSensorInfo->SensorStillCaptureFrameRate=22;
    pSensorInfo->SensorWebCamCaptureFrameRate=22;
    pSensorInfo->SensorResetActiveHigh=FALSE;
    pSensorInfo->SensorResetDelayCount=5;
    pSensorInfo->SensorOutputDataFormat=SENSOR_OUTPUT_FORMAT_RAW_Gb;
    pSensorInfo->SensorClockPolarity=SENSOR_CLOCK_POLARITY_LOW; /*??? */
    pSensorInfo->SensorClockFallingPolarity=SENSOR_CLOCK_POLARITY_LOW;
    pSensorInfo->SensorHsyncPolarity = SENSOR_CLOCK_POLARITY_LOW;
    pSensorInfo->SensorVsyncPolarity = SENSOR_CLOCK_POLARITY_LOW;
    pSensorInfo->SensorInterruptDelayLines = 1;
    pSensorInfo->SensroInterfaceType=SENSOR_INTERFACE_TYPE_MIPI;
    pSensorInfo->CaptureDelayFrame = 3; 
    pSensorInfo->PreviewDelayFrame = 5; 
    pSensorInfo->VideoDelayFrame = 5; 
    pSensorInfo->SensorMasterClockSwitch = 0; 
    pSensorInfo->SensorDrivingCurrent = ISP_DRIVING_4MA;      
    pSensorInfo->AEShutDelayFrame = 0;		    /* The frame of setting shutter default 0 for TG int */
    pSensorInfo->AESensorGainDelayFrame = 0;     /* The frame of setting sensor gain */
    pSensorInfo->AEISPGainDelayFrame = 2;	
	   
    switch (ScenarioId)
    {
        case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
     //   case MSDK_SCENARIO_ID_VIDEO_CAPTURE_MPEG4:
            pSensorInfo->SensorClockFreq=24;
            pSensorInfo->SensorClockDividCount=	5;
            pSensorInfo->SensorClockRisingCount= 0;
            pSensorInfo->SensorClockFallingCount= 2;
            pSensorInfo->SensorPixelClockCount= 3;
            pSensorInfo->SensorDataLatchCount= 2;
            pSensorInfo->SensorGrabStartX = GC5004MIPI_IMAGE_SENSOR_PV_STARTX; 
            pSensorInfo->SensorGrabStartY = GC5004MIPI_IMAGE_SENSOR_PV_STARTY;           		
            pSensorInfo->SensorMIPILaneNumber = SENSOR_MIPI_2_LANE;			
            pSensorInfo->MIPIDataLowPwr2HighSpeedTermDelayCount = 0; 
		    pSensorInfo->MIPIDataLowPwr2HighSpeedSettleDelayCount = 14; 
		    pSensorInfo->MIPICLKLowPwr2HighSpeedTermDelayCount = 0;
            pSensorInfo->SensorWidthSampling = 0;  // 0 is default 1x
            pSensorInfo->SensorHightSampling = 0;   // 0 is default 1x 
            pSensorInfo->SensorPacketECCOrder = 1;
            break;
		
        case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			   pSensorInfo->SensorClockFreq=24;
			   pSensorInfo->SensorClockDividCount= 5;
			   pSensorInfo->SensorClockRisingCount= 0;
			   pSensorInfo->SensorClockFallingCount= 2;
			   pSensorInfo->SensorPixelClockCount= 3;
			   pSensorInfo->SensorDataLatchCount= 2;
			   pSensorInfo->SensorGrabStartX = GC5004MIPI_IMAGE_SENSOR_VIDEO_STARTX; 
			   pSensorInfo->SensorGrabStartY = GC5004MIPI_IMAGE_SENSOR_VIDEO_STARTY;				   
			   pSensorInfo->SensorMIPILaneNumber = SENSOR_MIPI_2_LANE;		   
			   pSensorInfo->MIPIDataLowPwr2HighSpeedTermDelayCount = 0; 
				pSensorInfo->MIPIDataLowPwr2HighSpeedSettleDelayCount = 14; 
				pSensorInfo->MIPICLKLowPwr2HighSpeedTermDelayCount = 0;
			   pSensorInfo->SensorWidthSampling = 0;  // 0 is default 1x
			   pSensorInfo->SensorHightSampling = 0;   // 0 is default 1x 
			   pSensorInfo->SensorPacketECCOrder = 1;

			break;
        case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
       // case MSDK_SCENARIO_ID_CAMERA_CAPTURE_MEM:
				case MSDK_SCENARIO_ID_CAMERA_ZSD:
            pSensorInfo->SensorClockFreq=24;
            pSensorInfo->SensorClockDividCount=	5;
            pSensorInfo->SensorClockRisingCount= 0;
            pSensorInfo->SensorClockFallingCount= 2;
            pSensorInfo->SensorPixelClockCount= 3;
            pSensorInfo->SensorDataLatchCount= 2;
            pSensorInfo->SensorGrabStartX = GC5004MIPI_IMAGE_SENSOR_CAP_STARTX;	//2*GC5004MIPI_IMAGE_SENSOR_PV_STARTX; 
            pSensorInfo->SensorGrabStartY = GC5004MIPI_IMAGE_SENSOR_CAP_STARTY;	//2*GC5004MIPI_IMAGE_SENSOR_PV_STARTY;          			
            pSensorInfo->SensorMIPILaneNumber = SENSOR_MIPI_2_LANE;			
            pSensorInfo->MIPIDataLowPwr2HighSpeedTermDelayCount = 0; 
            pSensorInfo->MIPIDataLowPwr2HighSpeedSettleDelayCount = 14; 
            pSensorInfo->MIPICLKLowPwr2HighSpeedTermDelayCount = 0; 
            pSensorInfo->SensorWidthSampling = 0;  // 0 is default 1x
            pSensorInfo->SensorHightSampling = 0;   // 0 is default 1x
            pSensorInfo->SensorPacketECCOrder = 1;
            break;
        default:
			 pSensorInfo->SensorClockFreq=24;
			 pSensorInfo->SensorClockDividCount= 5;
			 pSensorInfo->SensorClockRisingCount= 0;
			 pSensorInfo->SensorClockFallingCount= 2;
			 pSensorInfo->SensorPixelClockCount= 3;
			 pSensorInfo->SensorDataLatchCount= 2;
			 pSensorInfo->SensorGrabStartX = GC5004MIPI_IMAGE_SENSOR_PV_STARTX; 
			 pSensorInfo->SensorGrabStartY = GC5004MIPI_IMAGE_SENSOR_PV_STARTY; 				 
			 pSensorInfo->SensorMIPILaneNumber = SENSOR_MIPI_2_LANE;		 
			 pSensorInfo->MIPIDataLowPwr2HighSpeedTermDelayCount = 0; 
		  pSensorInfo->MIPIDataLowPwr2HighSpeedSettleDelayCount = 14; 
		  pSensorInfo->MIPICLKLowPwr2HighSpeedTermDelayCount = 0;
			 pSensorInfo->SensorWidthSampling = 1;	// 0 is default 1x
			 pSensorInfo->SensorHightSampling = 1;	 // 0 is default 1x 
			 pSensorInfo->SensorPacketECCOrder = 2;

            break;
    }
	spin_lock(&gc5004_drv_lock);	

    GC5004MIPIPixelClockDivider=pSensorInfo->SensorPixelClockCount;
    memcpy(pSensorConfigData, &GC5004MIPISensorConfigData, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
	spin_unlock(&gc5004_drv_lock);

    return ERROR_NONE;
}   /* GC5004MIPIGetInfo() */


UINT32 GC5004MIPIControl(MSDK_SCENARIO_ID_ENUM ScenarioId, MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *pImageWindow,
                                                MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData)
{
		spin_lock(&gc5004_drv_lock);	
		CurrentScenarioId = ScenarioId;
		spin_unlock(&gc5004_drv_lock);
        SENSORDB("[GC5004MIPIControl]  = %d\n", CurrentScenarioId);
    switch (ScenarioId)
    {
        case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
            GC5004MIPIPreview(pImageWindow, pSensorConfigData);
            break;
        case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			GC5004MIPIVideo(pImageWindow, pSensorConfigData);
            break;
        case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
	    case MSDK_SCENARIO_ID_CAMERA_ZSD:
            GC5004MIPICapture(pImageWindow, pSensorConfigData);//hhl 2-28
            break;
        default:
            return ERROR_INVALID_SCENARIO_ID;
    }
    return ERROR_NONE;
} /* GC5004MIPIControl() */

UINT32 GC5004MIPISetVideoMode(UINT16 u2FrameRate)
{
        SENSORDB("[GC5004MIPISetVideoMode] frame rate = %d\n", u2FrameRate);
		kal_uint16 GC5004MIPI_Video_Max_Expourse_Time = 0;
		kal_uint16 Vblanking = 0;
		SENSORDB("[GC5004MIPI]%s():fix_frame_rate=%d\n",__FUNCTION__,u2FrameRate);
		
	//	VideoFullSizeSetting()
		if(u2FrameRate==0 ||u2FrameRate==30 )//dynamic frame rate or fix 30fps
			return TRUE;
		//spin_lock(&gc5004_drv_lock);
		//GC5004MIPI_sensor.fix_video_fps = KAL_TRUE;
		//spin_unlock(&gc5004_drv_lock);
		u2FrameRate=u2FrameRate*10;//10*FPS
		//SENSORDB("[GC5004MIPI][Enter Fix_fps func] GC5004MIPI_Fix_Video_Frame_Rate = %d\n", u2FrameRate/10);
	
		GC5004MIPI_Video_Max_Expourse_Time = (kal_uint16)((GC5004MIPI_sensor.video_pclk*10/u2FrameRate)/GC5004MIPI_sensor.video_line_length);
		
		SENSORDB("[GC5004MIPI][GC5004MIPI_Video_Max_Expourse_Time = %d\n", GC5004MIPI_Video_Max_Expourse_Time);
		if (GC5004MIPI_Video_Max_Expourse_Time > GC5004MIPI_VIDEO_FRAME_LENGTH_LINES) //GC5004MIPI_sensor.pv_frame_lengt
			{
				spin_lock(&gc5004_drv_lock);    
				GC5004MIPI_sensor.video_frame_length = GC5004MIPI_Video_Max_Expourse_Time;
				
				spin_unlock(&gc5004_drv_lock);
				Vblanking= GC5004MIPI_sensor.video_frame_length-GC5004MIPI_VIDEO_FRAME_LENGTH_LINES;
				SENSORDB("[GC5004MIPI][GC5004MIPI_Video_Max_Expourse_Time = %d\n", GC5004MIPI_Video_Max_Expourse_Time);
				SENSORDB("[GC5004MIPI]%s():Vblanking=%d\n",__FUNCTION__,Vblanking);
				GC5004MIPI_SetDummy(0,Vblanking);
			}
		spin_lock(&gc5004_drv_lock);    
	    GC5004MIPI_MPEG4_encode_mode = KAL_TRUE; 
		spin_unlock(&gc5004_drv_lock);
		mdelay(100);
    return TRUE;
}

UINT32 GC5004MIPISetAutoFlickerMode(kal_bool bEnable, UINT16 u2FrameRate)
{
/*	kal_uint32 pv_max_frame_rate_lines=0;

if(GC5004MIPI_sensor.pv_mode==TRUE)
	pv_max_frame_rate_lines=GC5004MIPI_PV_FRAME_LENGTH_LINES;
else
    pv_max_frame_rate_lines=GC5004MIPI_VIDEO_FRAME_LENGTH_LINES	;
//kal_uint32 pv_max_frame_rate_lines = GC5004MIPI_MAX_EXPOSURE_LINES;

    SENSORDB("[GC5004MIPISetAutoFlickerMode] frame rate(10base) = %d %d\n", bEnable, u2FrameRate);
    if(bEnable) {   // enable auto flicker   
    	spin_lock(&gc5004_drv_lock);    
        GC5004MIPI_Auto_Flicker_mode = KAL_TRUE; 
		spin_unlock(&gc5004_drv_lock);
        if(GC5004MIPI_MPEG4_encode_mode == KAL_TRUE) { // in the video mode, reset the frame rate
        GC5004MIPI_MAX_EXPOSURE_LINES=pv_max_frame_rate_lines-5;
            pv_max_frame_rate_lines = GC5004MIPI_MAX_EXPOSURE_LINES + (GC5004MIPI_MAX_EXPOSURE_LINES>>7);            
            GC5004MIPI_write_cmos_sensor(0x0104, 1);        
            GC5004MIPI_write_cmos_sensor(0x0340, (pv_max_frame_rate_lines >>8) & 0xFF);
            GC5004MIPI_write_cmos_sensor(0x0341, pv_max_frame_rate_lines & 0xFF);	
            GC5004MIPI_write_cmos_sensor(0x0104, 0);        	
        }
    } else {
    	/*spin_lock(&gc5004_drv_lock);    
        GC5004MIPI_Auto_Flicker_mode = KAL_FALSE; 
		spin_unlock(&gc5004_drv_lock);
        if(GC5004MIPI_MPEG4_encode_mode == KAL_TRUE) {    // in the video mode, restore the frame rate
            GC5004MIPI_write_cmos_sensor(0x0104, 1);        
            GC5004MIPI_write_cmos_sensor(0x0340, (GC5004MIPI_MAX_EXPOSURE_LINES >>8) & 0xFF);
            GC5004MIPI_write_cmos_sensor(0x0341, GC5004MIPI_MAX_EXPOSURE_LINES & 0xFF);	
            GC5004MIPI_write_cmos_sensor(0x0104, 0);        	
        }
        printk("Disable Auto flicker\n");    
    }*/
	return ERROR_NONE;
}




UINT32 GC5004MIPISetMaxFramerateByScenario(MSDK_SCENARIO_ID_ENUM scenarioId, MUINT32 frameRate) {
	kal_uint32 pclk;
	kal_int16 dummyLine;
	kal_uint16 lineLength,frameHeight;
		
	SENSORDB("GC5004MIPISetMaxFramerateByScenario: scenarioId = %d, frame rate = %d\n",scenarioId,frameRate);
	switch (scenarioId) {
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
			pclk = 288000000;
			lineLength = GC5004MIPI_PV_LINE_LENGTH_PIXELS;
			frameHeight = (10 * pclk)/frameRate/lineLength;
			dummyLine = frameHeight - GC5004MIPI_PV_FRAME_LENGTH_LINES;

			
			if(dummyLine<0)
				dummyLine = 0;
			//spin_lock(&gc5004_drv_lock);	
			//GC5004MIPI_sensor.pv_mode=TRUE;
			//spin_unlock(&gc5004_drv_lock);
			//GC5004MIPI_SetDummy(0, dummyLine);			
			break;			
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			pclk = 120000000;
			lineLength = GC5004MIPI_VIDEO_LINE_LENGTH_PIXELS;
			frameHeight = (10 * pclk)/frameRate/lineLength;
			dummyLine = frameHeight - GC5004MIPI_VIDEO_FRAME_LENGTH_LINES;
			if(dummyLine<0)
				dummyLine = 0;
		//	spin_lock(&gc5004_drv_lock);	
		//	GC5004MIPI_sensor.pv_mode=TRUE;
		//	spin_unlock(&gc5004_drv_lock);
			//GC5004MIPI_SetDummy(0, dummyLine);			
			break;			
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
		case MSDK_SCENARIO_ID_CAMERA_ZSD:			
			pclk = 120000000;
			lineLength = GC5004MIPI_FULL_LINE_LENGTH_PIXELS;
			frameHeight = (10 * pclk)/frameRate/lineLength;
			dummyLine = frameHeight - GC5004MIPI_FULL_FRAME_LENGTH_LINES;
			if(dummyLine<0)
				dummyLine = 0;
			
			//spin_lock(&gc5004_drv_lock);	
			//GC5004MIPI_sensor.pv_mode=FALSE;
			//spin_unlock(&gc5004_drv_lock);
			//GC5004MIPI_SetDummy(0, dummyLine);			
			break;		
        case MSDK_SCENARIO_ID_CAMERA_3D_PREVIEW: //added
            break;
        case MSDK_SCENARIO_ID_CAMERA_3D_VIDEO:
			break;
        case MSDK_SCENARIO_ID_CAMERA_3D_CAPTURE: //added   
			break;		
		default:
			break;
	}	
	return ERROR_NONE;
}



UINT32 GC5004MIPIGetDefaultFramerateByScenario(MSDK_SCENARIO_ID_ENUM scenarioId, MUINT32 *pframeRate) 
{

	switch (scenarioId) {
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			 *pframeRate = 300;
			 break;
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
		case MSDK_SCENARIO_ID_CAMERA_ZSD:
			 *pframeRate = 150;
			break;		//hhl 2-28
        case MSDK_SCENARIO_ID_CAMERA_3D_PREVIEW: //added
        case MSDK_SCENARIO_ID_CAMERA_3D_VIDEO:
        case MSDK_SCENARIO_ID_CAMERA_3D_CAPTURE: //added   
			 *pframeRate = 300;
			break;		
		default:
			break;
	}

	return ERROR_NONE;
}




UINT32 GC5004MIPISetTestPatternMode(kal_bool bEnable)
{
    SENSORDB("[GC5004MIPISetTestPatternMode] Test pattern enable:%d\n", bEnable);
    
	GC5004MIPI_write_cmos_sensor(0xfe, 0x00);//page0
    if(bEnable) {   // enable color bar   
        GC5004MIPI_write_cmos_sensor(0x8c, 0x14);  // ouput color bar test pattern
    } else {
        GC5004MIPI_write_cmos_sensor(0x8c, 0x10);  //disable  color bar test pattern
    }
    return TRUE;
}

UINT32 GC5004MIPIFeatureControl(MSDK_SENSOR_FEATURE_ENUM FeatureId,
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
    MSDK_SENSOR_ENG_INFO_STRUCT	*pSensorEngInfo=(MSDK_SENSOR_ENG_INFO_STRUCT *) pFeaturePara;

    switch (FeatureId)
    {
        case SENSOR_FEATURE_GET_RESOLUTION:
            *pFeatureReturnPara16++=GC5004MIPI_REAL_CAP_WIDTH;
            *pFeatureReturnPara16=GC5004MIPI_REAL_CAP_HEIGHT;
            *pFeatureParaLen=4;
            break;
        case SENSOR_FEATURE_GET_PERIOD:
        		switch(CurrentScenarioId)
        		{
        			case MSDK_SCENARIO_ID_CAMERA_ZSD:
        		    case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
 		            *pFeatureReturnPara16++=GC5004MIPI_sensor.cp_line_length;  
 		            *pFeatureReturnPara16=GC5004MIPI_sensor.cp_frame_length;
		            SENSORDB("Sensor period:%d %d\n",GC5004MIPI_sensor.cp_line_length, GC5004MIPI_sensor.cp_frame_length); 
		            *pFeatureParaLen=4;        				
        				break;
        			case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
					*pFeatureReturnPara16++=GC5004MIPI_sensor.video_line_length;  
					*pFeatureReturnPara16=GC5004MIPI_sensor.video_frame_length;
					 SENSORDB("Sensor period:%d %d\n", GC5004MIPI_sensor.video_line_length, GC5004MIPI_sensor.video_frame_length); 
					 *pFeatureParaLen=4;
						break;
        			default:	
					*pFeatureReturnPara16++=GC5004MIPI_sensor.pv_line_length;  
					*pFeatureReturnPara16=GC5004MIPI_sensor.pv_frame_length;
		            SENSORDB("Sensor period:%d %d\n", GC5004MIPI_sensor.pv_line_length, GC5004MIPI_sensor.pv_frame_length); 
		            *pFeatureParaLen=4;
	            break;
          	}
          	break;
        case SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ:
        		switch(CurrentScenarioId)
        		{
        			case MSDK_SCENARIO_ID_CAMERA_ZSD:
        			case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
		            *pFeatureReturnPara32 = GC5004MIPI_sensor.cp_pclk; //134400000
		            *pFeatureParaLen=4;		         	
					
		            SENSORDB("Sensor CPCLK:%dn",GC5004MIPI_sensor.cp_pclk); 
		         		break; //hhl 2-28
					case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
						*pFeatureReturnPara32 = GC5004MIPI_sensor.video_pclk;//200000000; 
						*pFeatureParaLen=4;
						SENSORDB("Sensor videoCLK:%d\n",GC5004MIPI_sensor.video_pclk); 
						break;
		         		default:
		            *pFeatureReturnPara32 = GC5004MIPI_sensor.pv_pclk;//134400000; //;
		            *pFeatureParaLen=4;
					SENSORDB("Sensor pvclk:%d\n",GC5004MIPI_sensor.pv_pclk); 
		            break;
		         }
		         break;
        case SENSOR_FEATURE_SET_ESHUTTER:
            GC5004MIPI_SetShutter(*pFeatureData16);
		//	SENSORDB("shutter&gain test by hhl:GC5004MIPI_SetShutter in feature ctrl\n"); 
            break;
		case SENSOR_FEATURE_SET_SENSOR_SYNC:
		//	SENSORDB("hhl'test the function of the sync cased\n"); 
			break;
        case SENSOR_FEATURE_SET_NIGHTMODE:
            GC5004MIPI_NightMode((BOOL) *pFeatureData16);
            break;
        case SENSOR_FEATURE_SET_GAIN:
           GC5004MIPI_SetGain((UINT16) *pFeatureData16);
            
		//	SENSORDB("shutter&gain test by hhl:GC5004MIPI_SetGain in feature ctrl\n"); 
            break;
        case SENSOR_FEATURE_SET_FLASHLIGHT:
            break;
        case SENSOR_FEATURE_SET_ISP_MASTER_CLOCK_FREQ:
			spin_lock(&gc5004_drv_lock);    
            GC5004MIPI_isp_master_clock=*pFeatureData32;
			spin_unlock(&gc5004_drv_lock);
            break;
        case SENSOR_FEATURE_SET_REGISTER:
			//iWriteReg((u16) pSensorRegData->RegAddr , (u32) pSensorRegData->RegData , 1, 0x66);//to test the AF
			GC5004MIPI_write_cmos_sensor(pSensorRegData->RegAddr, pSensorRegData->RegData);
            break;
        case SENSOR_FEATURE_GET_REGISTER:
            pSensorRegData->RegData = GC5004MIPI_read_cmos_sensor(pSensorRegData->RegAddr);
            break;
        case SENSOR_FEATURE_SET_CCT_REGISTER:
            SensorRegNumber=FACTORY_END_ADDR;
            for (i=0;i<SensorRegNumber;i++)
            {
            	spin_lock(&gc5004_drv_lock);    
                GC5004MIPISensorCCT[i].Addr=*pFeatureData32++;
                GC5004MIPISensorCCT[i].Para=*pFeatureData32++; 
				spin_unlock(&gc5004_drv_lock);
            }
            break;
        case SENSOR_FEATURE_GET_CCT_REGISTER:
            SensorRegNumber=FACTORY_END_ADDR;
            if (*pFeatureParaLen<(SensorRegNumber*sizeof(SENSOR_REG_STRUCT)+4))
                return FALSE;
            *pFeatureData32++=SensorRegNumber;
            for (i=0;i<SensorRegNumber;i++)
            {
                *pFeatureData32++=GC5004MIPISensorCCT[i].Addr;
                *pFeatureData32++=GC5004MIPISensorCCT[i].Para; 
            }
            break;
        case SENSOR_FEATURE_SET_ENG_REGISTER:
            SensorRegNumber=ENGINEER_END;
            for (i=0;i<SensorRegNumber;i++)
            {	spin_lock(&gc5004_drv_lock);    
                GC5004MIPISensorReg[i].Addr=*pFeatureData32++;
                GC5004MIPISensorReg[i].Para=*pFeatureData32++;
				spin_unlock(&gc5004_drv_lock);
            }
            break;
        case SENSOR_FEATURE_GET_ENG_REGISTER:
            SensorRegNumber=ENGINEER_END;
            if (*pFeatureParaLen<(SensorRegNumber*sizeof(SENSOR_REG_STRUCT)+4))
                return FALSE;
            *pFeatureData32++=SensorRegNumber;
            for (i=0;i<SensorRegNumber;i++)
            {
                *pFeatureData32++=GC5004MIPISensorReg[i].Addr;
                *pFeatureData32++=GC5004MIPISensorReg[i].Para;
            }
            break;
        case SENSOR_FEATURE_GET_REGISTER_DEFAULT:
            if (*pFeatureParaLen>=sizeof(NVRAM_SENSOR_DATA_STRUCT))
            {
                pSensorDefaultData->Version=NVRAM_CAMERA_SENSOR_FILE_VERSION;
                pSensorDefaultData->SensorId=GC5004MIPI_SENSOR_ID;
                memcpy(pSensorDefaultData->SensorEngReg, GC5004MIPISensorReg, sizeof(SENSOR_REG_STRUCT)*ENGINEER_END);
                memcpy(pSensorDefaultData->SensorCCTReg, GC5004MIPISensorCCT, sizeof(SENSOR_REG_STRUCT)*FACTORY_END_ADDR);
            }
            else
                return FALSE;
            *pFeatureParaLen=sizeof(NVRAM_SENSOR_DATA_STRUCT);
            break;
        case SENSOR_FEATURE_GET_CONFIG_PARA:
            memcpy(pSensorConfigData, &GC5004MIPISensorConfigData, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
            *pFeatureParaLen=sizeof(MSDK_SENSOR_CONFIG_STRUCT);
            break;
        case SENSOR_FEATURE_CAMERA_PARA_TO_SENSOR:
            GC5004MIPI_camera_para_to_sensor();
            break;

        case SENSOR_FEATURE_SENSOR_TO_CAMERA_PARA:
            GC5004MIPI_sensor_to_camera_para();
            break;
        case SENSOR_FEATURE_GET_GROUP_COUNT:
            *pFeatureReturnPara32++=GC5004MIPI_get_sensor_group_count();
            *pFeatureParaLen=4;
            break;
        case SENSOR_FEATURE_GET_GROUP_INFO:
            GC5004MIPI_get_sensor_group_info(pSensorGroupInfo->GroupIdx, pSensorGroupInfo->GroupNamePtr, &pSensorGroupInfo->ItemCount);
            *pFeatureParaLen=sizeof(MSDK_SENSOR_GROUP_INFO_STRUCT);
            break;
        case SENSOR_FEATURE_GET_ITEM_INFO:
            GC5004MIPI_get_sensor_item_info(pSensorItemInfo->GroupIdx,pSensorItemInfo->ItemIdx, pSensorItemInfo);
            *pFeatureParaLen=sizeof(MSDK_SENSOR_ITEM_INFO_STRUCT);
            break;

        case SENSOR_FEATURE_SET_ITEM_INFO:
            GC5004MIPI_set_sensor_item_info(pSensorItemInfo->GroupIdx, pSensorItemInfo->ItemIdx, pSensorItemInfo->ItemValue);
            *pFeatureParaLen=sizeof(MSDK_SENSOR_ITEM_INFO_STRUCT);
            break;

        case SENSOR_FEATURE_GET_ENG_INFO:
            pSensorEngInfo->SensorId = 129;
            pSensorEngInfo->SensorType = CMOS_SENSOR;
            pSensorEngInfo->SensorOutputDataFormat=SENSOR_OUTPUT_FORMAT_RAW_Gb;
            *pFeatureParaLen=sizeof(MSDK_SENSOR_ENG_INFO_STRUCT);
            break;
        case SENSOR_FEATURE_GET_LENS_DRIVER_ID:
            // get the lens driver ID from EEPROM or just return LENS_DRIVER_ID_DO_NOT_CARE
            // if EEPROM does not exist in camera module.
            *pFeatureReturnPara32=LENS_DRIVER_ID_DO_NOT_CARE;
            *pFeatureParaLen=4;
            break;

        case SENSOR_FEATURE_INITIALIZE_AF:
            break;
        case SENSOR_FEATURE_CONSTANT_AF:
            break;
        case SENSOR_FEATURE_MOVE_FOCUS_LENS:
            break;
        case SENSOR_FEATURE_SET_VIDEO_MODE:
            GC5004MIPISetVideoMode(*pFeatureData16);
            break;
        case SENSOR_FEATURE_CHECK_SENSOR_ID:
            GC5004MIPIGetSensorID(pFeatureReturnPara32); 
            break;             
        case SENSOR_FEATURE_SET_AUTO_FLICKER_MODE:
            GC5004MIPISetAutoFlickerMode((BOOL)*pFeatureData16, *(pFeatureData16+1));            
	        break;
        case SENSOR_FEATURE_SET_TEST_PATTERN:
            GC5004MIPISetTestPatternMode((BOOL)*pFeatureData16);        	
            break;
		case SENSOR_FEATURE_GET_TEST_PATTERN_CHECKSUM_VALUE:
				*pFeatureReturnPara32 = GC5004_TEST_PATTERN_CHECKSUM;
				*pFeatureParaLen=4;
				break;
		case SENSOR_FEATURE_SET_MAX_FRAME_RATE_BY_SCENARIO:
			GC5004MIPISetMaxFramerateByScenario((MSDK_SCENARIO_ID_ENUM)*pFeatureData32, *(pFeatureData32+1));
			break;
		case SENSOR_FEATURE_GET_DEFAULT_FRAME_RATE_BY_SCENARIO:
			GC5004MIPIGetDefaultFramerateByScenario((MSDK_SCENARIO_ID_ENUM)*pFeatureData32, (MUINT32 *)(*(pFeatureData32+1)));
			break;
        default:
            break;
    }
    return ERROR_NONE;
}	/* GC5004MIPIFeatureControl() */


SENSOR_FUNCTION_STRUCT	SensorFuncGC5004MIPI=
{
    GC5004MIPIOpen,
    GC5004MIPIGetInfo,
    GC5004MIPIGetResolution,
    GC5004MIPIFeatureControl,
    GC5004MIPIControl,
    GC5004MIPIClose
};

UINT32 GC5004_MIPI_RAW_SensorInit(PSENSOR_FUNCTION_STRUCT *pfFunc)
{
    /* To Do : Check Sensor status here */
    if (pfFunc!=NULL)
        *pfFunc=&SensorFuncGC5004MIPI;

    return ERROR_NONE;
}   /* SensorInit() */

