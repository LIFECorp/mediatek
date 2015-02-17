#ifndef BUILD_LK
#include <linux/string.h>
#else
#include <string.h>
#endif
#include "lcm_drv.h"

#ifdef BUILD_LK
	#include <platform/mt_gpio.h>
	#include <platform/mt_pmic.h>
	#include <platform/mt_i2c.h>
#elif defined(BUILD_UBOOT)
#else
	#include <mach/mt_gpio.h>
	#include <mach/mt_pm_ldo.h>
#endif

// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH  (2048) //(800)
#define FRAME_HEIGHT (1536) //(1280)

#define LCM_ID_NT35590 (0x90)
// TODO. This LCM ID is NT51012 not 35590.
#define GPIO_LCD_PWR_EN      (GPIO74|0x80000000) //GPIO142
#define GPIO_LCD_RST_EN      (GPIO80|0x80000000)
#define GPIO_LCD_STB_EN      (GPIO79|0x80000000)
#define GPIO_LCD_LED_EN      GPIO_LCM_LED_EN //GPIO76

#define GPIO_LCD_LED_PWM_EN      (GPIO116 | 0x80000000) //GPIO116


// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

//static LCM_UTIL_FUNCS lcm_util = {0};  //for fixed warning issue
static LCM_UTIL_FUNCS lcm_util = 
{
	.set_reset_pin = NULL,
	.udelay = NULL,
	.mdelay = NULL,
};


#define SET_RESET_PIN(v)    (lcm_util.set_reset_pin((v)))

#define UDELAY(n) (lcm_util.udelay(n))
#define MDELAY(n) (lcm_util.mdelay(n))


// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	        lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)										lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)					lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg(cmd)											lcm_util.dsi_dcs_read_lcm_reg(cmd)
#define read_reg_v2(cmd, buffer, buffer_size)   				lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)   


#define   LCM_DSI_CMD_MODE							0

#if 0
static void init_lcm_registers(void)
{
	unsigned int data_array[16];
		
#if 1

	data_array[0] = 0x00011500;  //software reset					 
	dsi_set_cmdq(data_array, 1, 1);
	
	MDELAY(20);
	
	data_array[0]=0x0bae1500;
	data_array[1]=0xeaee1500;
	data_array[2]=0x5fef1500;
	data_array[3]=0x68f21500;	
	data_array[4]=0x00ee1500;
	data_array[5]=0x00ef1500;
	dsi_set_cmdq(&data_array, 6, 1);

#if 0

	data_array[0] = 0x7DB21500;  					 
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x0BAE1500;  					 
	dsi_set_cmdq(data_array, 1, 1);
	
	data_array[0] = 0x18B61500;  					 
	dsi_set_cmdq(data_array, 1, 1);
	
	data_array[0] = 0xEAEE1500;  					 
	dsi_set_cmdq(data_array, 1, 1);
	
	data_array[0] = 0x5FEF1500;  					 
	dsi_set_cmdq(data_array, 1, 1);
	
	data_array[0] = 0x68F21500;  					 
	dsi_set_cmdq(data_array, 1, 1);
	
	data_array[0] = 0x00EE1500;  					 
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x00EF1500;  					 
	dsi_set_cmdq(data_array, 1, 1);
	
	data_array[0] = 0x64D21500;  					 
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x00101500;  //sleep out                        
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(20);
#endif

	//data_array[0] = 0xEFB11500;                       
	//dsi_set_cmdq(data_array, 1, 1);
	//MDELAY(1);

	//data_array[0] = 0x00290500;  //display on                        
	//dsi_set_cmdq(data_array, 1, 1);
#endif

#if 0
	data_array[0] = 0x00010500;  //software reset					 
	dsi_set_cmdq(data_array, 1, 1);
	
	MDELAY(20);

	data_array[0] = 0x00023902; 
	data_array[1] = 0x00000BAE; 					 
	dsi_set_cmdq(data_array, 2, 1);
	MDELAY(1);
	
	data_array[0] = 0x00023902; 
	data_array[1] = 0x0000EAEE; 					 
	dsi_set_cmdq(data_array, 2, 1);
	MDELAY(1);
	
	data_array[0] = 0x00023902; 
	data_array[1] = 0x00005FEF; 					 
	dsi_set_cmdq(data_array, 2, 1);
	MDELAY(1);
	
	data_array[0] = 0x00023902; 
	data_array[1] = 0x000068F2; 					 
	dsi_set_cmdq(data_array, 2, 1);
	MDELAY(1);
	
	data_array[0] = 0x00023902; 
	data_array[1] = 0x000000EE; 					 
	dsi_set_cmdq(data_array, 2, 1);
	MDELAY(1);
	
	data_array[0] = 0x00023902; 
	data_array[1] = 0x000000EF; 					 
	dsi_set_cmdq(data_array, 2, 1);
	MDELAY(1);
	
	data_array[0] = 0x00100500;  //sleep out                        
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(20);

	data_array[0] = 0x00290500;  //display on                        
	dsi_set_cmdq(data_array, 1, 1);
#endif
	//MDELAY(5);
}
#endif

// ---------------------------------------------------------------------------
//  LCM Driver Implementations
// ---------------------------------------------------------------------------

static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)
{
    memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));
}

#if 1
#ifdef BUILD_LK

#define IT6151_BUSNUM                I2C2
#define it6151_SLAVE_ADDR/*it6151_I2C_ADDR*/       0x58  //0x2d //write ;0x5b read


U32 it6151_reg_i2c_write_byte (U8 dev_addr,U8  cmd, U8 data)
{
    U8 cmdBufferLen = 1;
    U8 dataBufferLen = 1;	
    U32 ret_code = I2C_OK;
    U8 write_data[I2C_FIFO_SIZE];
    int transfer_len = 0;
    int i=0, cmdIndex=0, dataIndex=0;
	
    dev_addr = dev_addr<<1;  // for write
    
    transfer_len = cmdBufferLen + dataBufferLen;
    if(I2C_FIFO_SIZE < (cmdBufferLen + dataBufferLen))
    {
        dprintf(CRITICAL, "[it6151_i2c_write] exceed I2C FIFO length!! \n");
        return 0;
    }

    //write_data[0] = cmd;
    //write_data[1] = writeData;

    while(cmdIndex < cmdBufferLen)
    {
        write_data[i] = cmd;
        cmdIndex++;
        i++;
    }

    while(dataIndex < dataBufferLen)
    {
        write_data[i] = data;
        dataIndex++;
        i++;
    }


    /* dump write_data for check */
    for( i=0 ; i < transfer_len ; i++ )
    {
        dprintf(CRITICAL, "[it6151_i2c_write] write_data[%d]=%x\n", i, write_data[i]);
    }

    ret_code = mt_i2c_write(IT6151_BUSNUM, dev_addr, write_data, transfer_len,0);


    return ret_code;
}


static U32 it6151_reg_i2c_read_byte (U8 dev_addr,U8  *cmdBuffer, U8 *dataBuffer)
{
    U32 ret_code = I2C_OK;
    dev_addr = dev_addr<<1 + 1;  // for read
    ret_code = mt_i2c_write(IT6151_BUSNUM, dev_addr, cmdBuffer, 1, 0);    // set register command
    if (ret_code != I2C_OK)
        return ret_code;

    ret_code = mt_i2c_read(IT6151_BUSNUM, dev_addr, dataBuffer, 1, 0);

    if (ret_code != I2C_OK)
        return ret_code;	

    dbg_print("[it6151_read_byte] Done\n");

    return ret_code;
}

 //end
 
 /******************************************************************************
 *IIC drvier,:protocol type 2 add by chenguangjian end
 ******************************************************************************/
#else
extern UINT8 it6151_reg_i2c_read_byte(U8 dev_addr,U8  *cmdBuffer, U8 *dataBuffer);
extern void it6151_reg_i2c_write_byte(U8 dev_addr,U8  cmd, U8 data);
#endif
#endif

void vit6151_init(void)
{

it6151_reg_i2c_write_byte(0x5C,0xfd,0xd9);
it6151_reg_i2c_write_byte(0x6C,0x05,0x00);
it6151_reg_i2c_write_byte(0x6C,0x0c,0x31);
it6151_reg_i2c_write_byte(0x6C,0x11,0x00);
it6151_reg_i2c_write_byte(0x6C,0x12,0x03);
it6151_reg_i2c_write_byte(0x6C,0x19,0x02);
it6151_reg_i2c_write_byte(0x6C,0x29,0xdf);
it6151_reg_i2c_write_byte(0x6C,0x2e,0x40);
it6151_reg_i2c_write_byte(0x6C,0x2f,0x01);
it6151_reg_i2c_write_byte(0x6C,0x4e,0x03);
it6151_reg_i2c_write_byte(0x6C,0x80,0x22);
it6151_reg_i2c_write_byte(0x6C,0x84,0x8f);
MDELAY(5);
it6151_reg_i2c_write_byte(0x5C,0x05,0x00);
it6151_reg_i2c_write_byte(0x5C,0x0c,0x08);
it6151_reg_i2c_write_byte(0x5C,0x14,0x00);
it6151_reg_i2c_write_byte(0x5C,0x1a,0x55);
it6151_reg_i2c_write_byte(0x5C,0x21,0x00);
it6151_reg_i2c_write_byte(0x5C,0x3a,0x04);
it6151_reg_i2c_write_byte(0x5C,0x59,0x28);
it6151_reg_i2c_write_byte(0x5C,0x5c,0xf3);
it6151_reg_i2c_write_byte(0x5C,0x5f,0x06);
it6151_reg_i2c_write_byte(0x5C,0xb5,0x80);
it6151_reg_i2c_write_byte(0x5C,0xc5,0x00);
it6151_reg_i2c_write_byte(0x5C,0xc9,0xf5);
it6151_reg_i2c_write_byte(0x5C,0xca,0x4c);
it6151_reg_i2c_write_byte(0x5C,0xcb,0x37);
it6151_reg_i2c_write_byte(0x5C,0xd1,0x93);
it6151_reg_i2c_write_byte(0x5C,0xd2,0xcf);
it6151_reg_i2c_write_byte(0x5C,0xd3,0x03);
it6151_reg_i2c_write_byte(0x5C,0xd4,0x60);
it6151_reg_i2c_write_byte(0x5C,0xe8,0x11);
it6151_reg_i2c_write_byte(0x5C,0xec,0x00);
MDELAY(5);
it6151_reg_i2c_write_byte(0x5C,0x0f,0x02);
it6151_reg_i2c_write_byte(0x5C,0x59,0x28);
it6151_reg_i2c_write_byte(0x5C,0x23,0x42);
it6151_reg_i2c_write_byte(0x5C,0x24,0x07);
it6151_reg_i2c_write_byte(0x5C,0x25,0x01);
it6151_reg_i2c_write_byte(0x5C,0x26,0x00);
it6151_reg_i2c_write_byte(0x5C,0x27,0x10);
it6151_reg_i2c_write_byte(0x5C,0x2B,0x05);
it6151_reg_i2c_write_byte(0x5C,0x23,0x40);
it6151_reg_i2c_write_byte(0x5C,0x16,0x16);
it6151_reg_i2c_write_byte(0x5C,0x0f,0x03);
it6151_reg_i2c_write_byte(0x5C,0x76,0xa7);
it6151_reg_i2c_write_byte(0x5C,0x77,0xaf);
it6151_reg_i2c_write_byte(0x5C,0x7e,0x8f);
it6151_reg_i2c_write_byte(0x5C,0x7f,0x07);
it6151_reg_i2c_write_byte(0x5C,0x80,0xef);
it6151_reg_i2c_write_byte(0x5C,0x81,0x5f);
it6151_reg_i2c_write_byte(0x5C,0x82,0xef);
it6151_reg_i2c_write_byte(0x5C,0x83,0x07);
it6151_reg_i2c_write_byte(0x5C,0x0f,0x02);
it6151_reg_i2c_write_byte(0x5C,0x5c,0xf3);
it6151_reg_i2c_write_byte(0x5C,0xd3,0x03);
it6151_reg_i2c_write_byte(0x5C,0x17,0x01);
MDELAY(5);

}

static void lcm_get_params(LCM_PARAMS *params)
{

		memset(params, 0, sizeof(LCM_PARAMS));
	
		params->type   = LCM_TYPE_DSI;

		params->width  = FRAME_WIDTH;
		params->height = FRAME_HEIGHT;

#if 0
		// enable tearing-free
		params->dbi.te_mode 				= LCM_DBI_TE_MODE_VSYNC_ONLY;
		params->dbi.te_edge_polarity		= LCM_POLARITY_RISING;
#endif

        #if (LCM_DSI_CMD_MODE)
		params->dsi.mode   = CMD_MODE;
        #else
		params->dsi.mode   = SYNC_PULSE_VDO_MODE; //SYNC_PULSE_VDO_MODE; //BURST_VDO_MODE; //;
        #endif
	
		// DSI
		/* Command mode setting */
		//1 Three lane or Four lane
		params->dsi.LANE_NUM				= LCM_FOUR_LANE;
		//The following defined the fomat for data coming from LCD engine.
		params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
		params->dsi.data_format.trans_seq   = LCM_DSI_TRANS_SEQ_MSB_FIRST;
		params->dsi.data_format.padding     = LCM_DSI_PADDING_ON_LSB;
		params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;

		// Highly depends on LCD driver capability.
		// Not support in MT6573
		params->dsi.packet_size=256;

		// Video mode setting		
		params->dsi.intermediat_buffer_num = 0;//because DSI/DPI HW design change, this parameters should be 0 when video mode in MT658X; or memory leakage

		params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;
		params->dsi.word_count=1920*3; //720*3;	

		params->dsi.ufoe_enable = 1;
			
		params->dsi.vertical_sync_active				= 2; //4; //6; //(12-4-4); //1;
		params->dsi.vertical_backporch					= 1; //4; //6; //4; //6; //10;
		params->dsi.vertical_frontporch					= 2; //4;//6; //4//6; //10;
		params->dsi.vertical_active_line				= FRAME_HEIGHT; 

		params->dsi.horizontal_sync_active				= 8; //40; //44; //(120-40-40); //1;
		params->dsi.horizontal_backporch				= 8; //40;//44; //40; //44; //57;
		params->dsi.horizontal_frontporch				= 29; //40;//44; //40; //44; //32;
		params->dsi.horizontal_active_pixel				= FRAME_WIDTH;

		// Bit rate calculation
		//1 Every lane speed
		//params->dsi.pll_div1=1; //0;		// div1=0,1,2,3;div1_real=1,2,4,4 ----0: 546Mbps  1:273Mbps
		//params->dsi.pll_div2=1;		// div2=0,1,2,3;div1_real=1,2,4,4	
		//params->dsi.fbk_div =22;//31;    // fref=26MHz, fvco=fref*(fbk_div+1)*2/(div1_real*div2_real)	
		//params->dsi.fbk_sel =0;
		//params->dsi.fbk_div =45;
        params->dsi.PLL_CLOCK = LCM_DSI_6589_PLL_CLOCK_299;//LCM_DSI_6589_PLL_CLOCK_NULL; //LCM_DSI_6589_PLL_CLOCK_396_5;
    
		//params->dsi.CLK_ZERO = 262; //47;
		//params->dsi.HS_ZERO = 117; //36;

		params->dsi.cont_clock = 1;
		//params->dsi.noncont_clock = TRUE; 
		//params->dsi.noncont_clock_period = 2; // Unit : frames

	       params->dsi.HS_TRAIL = 0x8; //36;
	       params->dsi.HS_ZERO = 0xA; //36;
	       params->dsi.HS_PRPR = 0x6; //36;
	       params->dsi.LPX = 0x5; //36;

	       params->dsi.DA_HS_EXIT = 0x6; //36;
		
}
//extern void DSI_clk_HS_mode(unsigned char enter);
static void lcm_init(void)
{
	//unsigned int data_array[16];
#ifdef BUILD_LK
		dprintf(INFO, "[LK/LCM] lcm_init() enter\n");
	//VGP6 3.3V
	pmic_config_interface(0x424, 0x1, 0x1, 15); 
	pmic_config_interface(0x45a, 0x07, 0x07, 5);
	//vgp4 1.8V
	pmic_config_interface(0x420, 0x1, 0x1, 15); 
	pmic_config_interface(0x43c, 0x03, 0x07, 5);	
	
#endif
	mt_set_gpio_mode(GPIO_LCD_PWR_EN, GPIO_MODE_00);
	mt_set_gpio_dir(GPIO_LCD_PWR_EN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_LCD_PWR_EN, GPIO_OUT_ONE);
	MDELAY(20);
	mt_set_gpio_mode(GPIO_LCD_RST_EN, GPIO_MODE_00);
	mt_set_gpio_dir(GPIO_LCD_RST_EN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_LCD_RST_EN, GPIO_OUT_ONE);
	
	MDELAY(20);
	mt_set_gpio_mode(GPIO_LCD_STB_EN, GPIO_MODE_00);
	mt_set_gpio_dir(GPIO_LCD_STB_EN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_LCD_STB_EN, GPIO_OUT_ONE);
	MDELAY(200);
	
	mt_set_gpio_mode(GPIO_LCD_LED_EN, GPIO_MODE_00);
	mt_set_gpio_dir(GPIO_LCD_LED_EN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_LCD_LED_EN, GPIO_OUT_ONE);

	MDELAY(50); 
	
	SET_RESET_PIN(1);
	SET_RESET_PIN(0);
	MDELAY(1);
	SET_RESET_PIN(1);


	MDELAY(150);
    
        vit6151_init();


}



static void lcm_suspend(void)
{
	//unsigned int data_array[16];

#ifdef BUILD_LK
	dprintf(INFO, "[LK/LCM] lcm_suspend() enter\n");
#else
printk( "[LK/LCM] lcm_suspend() enter\n");
#endif
	MDELAY(20);
	//
	mt_set_gpio_mode(GPIO_LCD_LED_EN, GPIO_MODE_00);
	mt_set_gpio_dir(GPIO_LCD_LED_EN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_LCD_LED_EN, GPIO_OUT_ZERO);
	MDELAY(20);
	mt_set_gpio_mode(GPIO_LCD_STB_EN, GPIO_MODE_00);
	mt_set_gpio_dir(GPIO_LCD_STB_EN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_LCD_STB_EN, GPIO_OUT_ZERO);		
	MDELAY(20);
	mt_set_gpio_mode(GPIO_LCD_RST_EN, GPIO_MODE_00);
	mt_set_gpio_dir(GPIO_LCD_RST_EN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_LCD_RST_EN, GPIO_OUT_ZERO);
	MDELAY(20);		
	mt_set_gpio_mode(GPIO_LCD_PWR_EN, GPIO_MODE_00);
	mt_set_gpio_dir(GPIO_LCD_PWR_EN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_LCD_PWR_EN, GPIO_OUT_ZERO);	

	MDELAY(160);

}


static void lcm_resume(void)
{
#ifdef BUILD_LK
	dprintf(INFO, "[LK/LCM] lcm_resume() enter\n");
	//VGP6 3.3V
	pmic_config_interface(0x424, 0x1, 0x1, 15); 
	pmic_config_interface(0x45a, 0x07, 0x07, 5);
	//vgp4 1.8V
	pmic_config_interface(0x420, 0x1, 0x1, 15); 
	pmic_config_interface(0x43c, 0x03, 0x07, 5);		
		
#else
printk( "[LK/LCM] lcm_resume() enter\n");
#endif
	mt_set_gpio_mode(GPIO_LCD_PWR_EN, GPIO_MODE_00);
	mt_set_gpio_dir(GPIO_LCD_PWR_EN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_LCD_PWR_EN, GPIO_OUT_ONE);
	MDELAY(20);
	mt_set_gpio_mode(GPIO_LCD_RST_EN, GPIO_MODE_00);
	mt_set_gpio_dir(GPIO_LCD_RST_EN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_LCD_RST_EN, GPIO_OUT_ONE);
		
	MDELAY(20);	
		mt_set_gpio_mode(GPIO_LCD_STB_EN, GPIO_MODE_00);
		mt_set_gpio_dir(GPIO_LCD_STB_EN, GPIO_DIR_OUT);
		mt_set_gpio_out(GPIO_LCD_STB_EN, GPIO_OUT_ONE);
		MDELAY(200);

	mt_set_gpio_mode(GPIO_LCD_LED_EN, GPIO_MODE_00);
	mt_set_gpio_dir(GPIO_LCD_LED_EN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_LCD_LED_EN, GPIO_OUT_ONE);
	MDELAY(20);

	lcm_init();


}
         
#if (LCM_DSI_CMD_MODE)
static void lcm_update(unsigned int x, unsigned int y,
                       unsigned int width, unsigned int height)
{
	unsigned int x0 = x;
	unsigned int y0 = y;
	unsigned int x1 = x0 + width - 1;
	unsigned int y1 = y0 + height - 1;

	unsigned char x0_MSB = ((x0>>8)&0xFF);
	unsigned char x0_LSB = (x0&0xFF);
	unsigned char x1_MSB = ((x1>>8)&0xFF);
	unsigned char x1_LSB = (x1&0xFF);
	unsigned char y0_MSB = ((y0>>8)&0xFF);
	unsigned char y0_LSB = (y0&0xFF);
	unsigned char y1_MSB = ((y1>>8)&0xFF);
	unsigned char y1_LSB = (y1&0xFF);

	unsigned int data_array[16];

	data_array[0]= 0x00053902;
	data_array[1]= (x1_MSB<<24)|(x0_LSB<<16)|(x0_MSB<<8)|0x2a;
	data_array[2]= (x1_LSB);
	dsi_set_cmdq(data_array, 3, 1);
	
	data_array[0]= 0x00053902;
	data_array[1]= (y1_MSB<<24)|(y0_LSB<<16)|(y0_MSB<<8)|0x2b;
	data_array[2]= (y1_LSB);
	dsi_set_cmdq(data_array, 3, 1);

	data_array[0]= 0x00290508; //HW bug, so need send one HS packet
	dsi_set_cmdq(data_array, 1, 1);
	
	data_array[0]= 0x002c3909;
	dsi_set_cmdq(data_array, 1, 0);

}
#endif
#if 0
static unsigned int lcm_compare_id(void)
{
	unsigned int id=0;
	unsigned char buffer[2];
	unsigned int array[16];  

	SET_RESET_PIN(1);
	SET_RESET_PIN(0);
	MDELAY(1);
	
	SET_RESET_PIN(1);
	MDELAY(20); 

	array[0] = 0x00023700;// read id return two byte,version and id
	dsi_set_cmdq(array, 1, 1);
	
	read_reg_v2(0xF4, buffer, 2);
	id = buffer[0]; //we only need ID
    #ifdef BUILD_LK
		dprintf(INFO, "%s, LK nt35590 debug: nt35590 id = 0x%08x\n", __func__, id);
    #else
		printk("%s, kernel nt35590 horse debug: nt35590 id = 0x%08x\n", __func__, id);
    #endif

    if(id == LCM_ID_NT35590)
    	return 1;
    else
        return 0;


}
#endif

LCM_DRIVER it6151_edp_dsi_video_sharp_lcm_drv = 
{
    .name			= "it6151_epd_dsi_video_sharp",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
	//.compare_id     = lcm_compare_id,
#if (LCM_DSI_CMD_MODE)
    .update         = lcm_update,
#endif
    };
