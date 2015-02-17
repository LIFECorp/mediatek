/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2008
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/



#ifdef BUILD_LK
#include <platform/mt_gpio.h>
#else
#include <mach/mt_gpio.h>
#include <linux/string.h>
#endif
#include <cust_gpio_usage.h>
#include "lcm_drv.h"
// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH			(320)
#define FRAME_HEIGHT			(480)

#define REGFLAG_DELAY             	0XAB
#define REGFLAG_END_OF_TABLE      	0xAA   // END OF REGISTERS MARKER

// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v)    								(lcm_util.set_reset_pin((v)))

#define UDELAY(n) 											(lcm_util.udelay(n))
#define MDELAY(n) 											(lcm_util.mdelay(n))


// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------

//#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
//#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
//#define wrtie_cmd(cmd)										lcm_util.dsi_write_cmd(cmd)
//#define write_regs(addr, pdata, byte_nums)					lcm_util.dsi_write_regs(addr, pdata, byte_nums)
//#define read_reg											lcm_util.dsi_read_reg()
#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)        lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)           lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)                                                                  lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)                              lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg(cmd)                                                                                   lcm_util.dsi_dcs_read_lcm_reg(cmd)
#define read_reg_v2(cmd, buffer, buffer_size)                                   lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)


struct LCM_setting_table {
    unsigned cmd;
    unsigned char count;
    unsigned char para_list[64];
};

static struct LCM_setting_table lcm_set_window[] = {
	//////{0x2A,	4,	{0x00, 0x00, (FRAME_WIDTH>>8), (FRAME_WIDTH&0xFF)}},
	//////{0x2B,	4,	{0x00, 0x00, (FRAME_HEIGHT>>8), (FRAME_HEIGHT&0xFF)}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};


static struct LCM_setting_table lcm_sleep_out_setting[] = {
    // Sleep Out
	{0x11, 1, {0x00}},
    {REGFLAG_DELAY, 120, {}},

    // Display ON
	{0x29, 1, {0x00}},
	{REGFLAG_DELAY, 20, {}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};


static struct LCM_setting_table lcm_deep_sleep_mode_in_setting[] = {
	// Display off sequence
	{0x28, 1, {0x00}},
    {REGFLAG_DELAY, 10, {}},

    // Sleep Mode On
	{0x10, 1, {0x00}},
    {REGFLAG_DELAY, 120, {}},

	{REGFLAG_END_OF_TABLE, 0x00, {}}
};


static struct LCM_setting_table lcm_backlight_level_setting[] = {
	{0x51, 1, {0xFF}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};

static struct LCM_setting_table lcm_backlight_mode_setting[] = {
	{0x55, 1, {0x1}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};


static void push_table(struct LCM_setting_table *table, unsigned int count, unsigned char force_update)
{
	unsigned int i;

    for(i = 0; i < count; i++) {
		
        unsigned cmd;
        cmd = table[i].cmd;
		
        switch (cmd) {
			
            case REGFLAG_DELAY :
                MDELAY(table[i].count);
                break;
				
            case REGFLAG_END_OF_TABLE :
                break;
				
            default:
		dsi_set_cmdq_V2(cmd, table[i].count, table[i].para_list, force_update);

                if (cmd != 0xFF && cmd != 0x2C && cmd != 0x3C) {
                    //#if defined(BUILD_UBOOT)
                    //  printf("[DISP] - uboot - REG_R(0x%x) = 0x%x. \n", cmd, table[i].para_list[0]);
                    //#endif
                    while(read_reg(cmd) != table[i].para_list[0]);
                }
       	}
    }
	
}


// ---------------------------------------------------------------------------
//  LCM Driver Implementations
// ---------------------------------------------------------------------------

static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)
{
    memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));
}


static void lcm_get_params(LCM_PARAMS *params)
{
		memset(params, 0, sizeof(LCM_PARAMS));
	
		params->type   = LCM_TYPE_DSI;

		params->width  = FRAME_WIDTH;
		params->height = FRAME_HEIGHT;

		// enable tearing-free
		params->dbi.te_mode 				= LCM_DBI_TE_MODE_DISABLED; //LCM_DBI_TE_MODE_VSYNC_ONLY;
		params->dbi.te_edge_polarity		= LCM_POLARITY_RISING;

		params->dsi.mode   = CMD_MODE;

		// DSI
		/* Command mode setting */
		params->dsi.LANE_NUM				= LCM_ONE_LANE;

		//The following defined the fomat for data coming from LCD engine.
		params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
		params->dsi.data_format.trans_seq   = LCM_DSI_TRANS_SEQ_MSB_FIRST;
		params->dsi.data_format.padding     = LCM_DSI_PADDING_ON_LSB;
		params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB666;

		// Video mode setting		
		params->dsi.PS=LCM_PACKED_PS_18BIT_RGB666;
#if 1
		params->dsi.word_count=FRAME_WIDTH*3;	
		params->dsi.vertical_sync_active=2;
		params->dsi.vertical_backporch=2;
		params->dsi.vertical_frontporch=2;
		params->dsi.vertical_active_line=FRAME_HEIGHT;
	
		params->dsi.line_byte=2180;		// 2256 = 752*3	NC
		params->dsi.horizontal_sync_active_byte=26;
		params->dsi.horizontal_backporch_byte=206;
		params->dsi.horizontal_frontporch_byte=206;	
		params->dsi.rgb_byte=(FRAME_WIDTH*3+6);		// NC
	
		params->dsi.horizontal_sync_active_word_count=20;	
		params->dsi.horizontal_backporch_word_count=200;
		params->dsi.horizontal_frontporch_word_count=200;
#else
		params->dsi.word_count=480*3;	
		params->dsi.vertical_sync_active=2;
		params->dsi.vertical_backporch=2;
		params->dsi.vertical_frontporch=2;
		params->dsi.vertical_active_line=800;
	
		params->dsi.line_byte=2180;		// 2256 = 752*3
		params->dsi.horizontal_sync_active_byte=26;
		params->dsi.horizontal_backporch_byte=206;
		params->dsi.horizontal_frontporch_byte=206;	
		params->dsi.rgb_byte=(480*3+6);	
	
		params->dsi.horizontal_sync_active_word_count=20;	
		params->dsi.horizontal_backporch_word_count=200;
		params->dsi.horizontal_frontporch_word_count=200;
#endif
		// Bit rate calculation
    params->dsi.pll_div1=1;             // div1=0,1,2,3;div1_real=1,2,4,4
    params->dsi.pll_div2=1;             // div2=0,1,2,3;div2_real=1,2,4,4
    params->dsi.fbk_div =8; //17           // fref=26MHz, fvco=fref*(fbk_div+1)*2/(div1_real*div2_real)
//		params->dsi.pll_div1=2;		// fref=26MHz, fvco=fref*(div1+1)	(div1=0~63, fvco=500MHZ~1GHz)
//		params->dsi.pll_div2=2;			// div2=0~15: fout=fvo/(2*div2) 300MHZ

//		params->dsi.HS_TRAIL=7;  //Mirror Note : (7-3)*15ns ~ 60ns	=> Suggest > 100ns	 
//		params->dsi.CLK_TRAIL=7;
}

static void lcm_init(void)
{
	unsigned int data_array[16];

//#ifdef BUILD_LK
#if 0
       lcm_util.set_gpio_mode(GPIO103, GPIO_MODE_01);
       lcm_util.set_gpio_mode(GPIO104, GPIO_MODE_01);

       lcm_util.set_gpio_mode(GPIO135, GPIO_MODE_00);
       lcm_util.set_gpio_dir(GPIO135, GPIO_DIR_OUT);
       lcm_util.set_gpio_out(GPIO135, GPIO_OUT_ONE);

       lcm_util.set_gpio_mode(GPIO134, GPIO_MODE_00);
       lcm_util.set_gpio_dir(GPIO134, GPIO_DIR_OUT);
       lcm_util.set_gpio_out(GPIO134, GPIO_OUT_ONE);

	MDELAY(100);
       lcm_util.set_gpio_mode(GPIO134, GPIO_MODE_00);
       lcm_util.set_gpio_dir(GPIO134, GPIO_DIR_OUT);
       lcm_util.set_gpio_out(GPIO134, GPIO_OUT_ZERO);
#endif
    SET_RESET_PIN(1);
    MDELAY(1);
    SET_RESET_PIN(0);
    MDELAY(10);
    SET_RESET_PIN(1);
    MDELAY(120);

		data_array[0]=0x00103902;
		data_array[1]=0x0E0400E0; 
		data_array[2]=0x400A1708;
		data_array[3]=0x0E074D79; 
		data_array[4]=0x0F1D1A0A;
		dsi_set_cmdq(&data_array, 5, 1);

		data_array[0]=0x00103902;
		data_array[1]=0x1F1B00E1; 
		data_array[2]=0x32051002;
		data_array[3]=0x0A024334; 
		data_array[4]=0x0F373309;
		dsi_set_cmdq(&data_array, 5, 1);

		data_array[0]=0x00033902;
		data_array[1]=0x001618C0; 
		dsi_set_cmdq(&data_array, 2, 1);

		data_array[0]=0x41C11500;
		dsi_set_cmdq(&data_array, 1, 1);

		data_array[0]=0x00043902;
		data_array[1]=0x802200C5; 
		dsi_set_cmdq(&data_array, 2, 1);

		data_array[0]=0xC8361500;
		dsi_set_cmdq(&data_array, 1, 1);

		data_array[0]=0x663A1500;
		dsi_set_cmdq(&data_array, 1, 1);

		data_array[0]=0x00B01500;
		dsi_set_cmdq(&data_array, 1, 1);

		data_array[0]=0xB0B11500;
		dsi_set_cmdq(&data_array, 1, 1);

		data_array[0]=0x02B41500;
		dsi_set_cmdq(&data_array, 1, 1);

		data_array[0]=0x00033902;
		data_array[1]=0x002202B6; 
		dsi_set_cmdq(&data_array, 2, 1);

		data_array[0]=0x00E91500;
		dsi_set_cmdq(&data_array, 1, 1);
 
		data_array[0]=0x00053902;
		data_array[1]=0x2C51A9F7; 
		data_array[2]=0x00000082; 
		dsi_set_cmdq(&data_array, 3, 1);

		data_array[0]=0x00110500;
		dsi_set_cmdq(&data_array, 1, 1);
    MDELAY(120);

		data_array[0]=0x00290500;
		dsi_set_cmdq(&data_array, 1, 1);
}


static void lcm_suspend(void)
{
#if 0
	lcm_util.set_gpio_mode(GPIO103, GPIO_MODE_00);
	lcm_util.set_gpio_dir(GPIO103, GPIO_DIR_IN); 
	lcm_util.set_gpio_pull_enable(GPIO103, GPIO_PULL_ENABLE);
	mt_set_gpio_pull_select(GPIO103, GPIO_PULL_DOWN);

       lcm_util.set_gpio_mode(GPIO104, GPIO_MODE_00);
       lcm_util.set_gpio_dir(GPIO104, GPIO_DIR_OUT);
       lcm_util.set_gpio_out(GPIO104, GPIO_OUT_ZERO);

       lcm_util.set_gpio_mode(GPIO135, GPIO_MODE_00);
       lcm_util.set_gpio_dir(GPIO135, GPIO_DIR_OUT);
       lcm_util.set_gpio_out(GPIO135, GPIO_OUT_ONE);

       lcm_util.set_gpio_mode(GPIO141, GPIO_MODE_00);
       lcm_util.set_gpio_dir(GPIO141, GPIO_DIR_OUT);
       lcm_util.set_gpio_out(GPIO141, GPIO_OUT_ONE);

       lcm_util.set_gpio_mode(GPIO134, GPIO_MODE_00);
       lcm_util.set_gpio_dir(GPIO134, GPIO_DIR_OUT);
       lcm_util.set_gpio_out(GPIO134, GPIO_OUT_ONE);
    		MDELAY(100);
       lcm_util.set_gpio_out(GPIO134, GPIO_OUT_ZERO);
    		MDELAY(100);

       lcm_util.set_gpio_out(GPIO135, GPIO_OUT_ZERO);
       lcm_util.set_gpio_out(GPIO141, GPIO_OUT_ZERO);
#endif

#if 0

       lcm_util.set_gpio_mode(GPIO135, GPIO_MODE_00);
       lcm_util.set_gpio_dir(GPIO135, GPIO_DIR_OUT);
       lcm_util.set_gpio_out(GPIO135, GPIO_OUT_ZERO);
#endif
	push_table(lcm_deep_sleep_mode_in_setting, sizeof(lcm_deep_sleep_mode_in_setting) / sizeof(struct LCM_setting_table), 1);
}

static void lcm_resume(void)
{
#if 0
	lcm_util.set_gpio_mode(GPIO103, GPIO_MODE_00);
	lcm_util.set_gpio_dir(GPIO103, GPIO_DIR_IN); 
	lcm_util.set_gpio_pull_enable(GPIO103, GPIO_PULL_DISABLE);
	lcm_util.set_gpio_mode(GPIO103, GPIO_MODE_01);

	lcm_util.set_gpio_mode(GPIO104, GPIO_MODE_01);
#endif

#if 0
       lcm_util.set_gpio_mode(GPIO135, GPIO_MODE_00);
       lcm_util.set_gpio_dir(GPIO135, GPIO_DIR_OUT);
       lcm_util.set_gpio_out(GPIO135, GPIO_OUT_ONE);

       lcm_util.set_gpio_mode(GPIO134, GPIO_MODE_00);
       lcm_util.set_gpio_dir(GPIO134, GPIO_DIR_OUT);
       lcm_util.set_gpio_out(GPIO134, GPIO_OUT_ONE);
	MDELAY(100);
       lcm_util.set_gpio_mode(GPIO134, GPIO_MODE_00);
       lcm_util.set_gpio_dir(GPIO134, GPIO_DIR_OUT);
       lcm_util.set_gpio_out(GPIO134, GPIO_OUT_ZERO);

#endif
	push_table(lcm_sleep_out_setting, sizeof(lcm_sleep_out_setting) / sizeof(struct LCM_setting_table), 1);
}

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

	data_array[0]= 0x00290508;//HW bug, so need send one HS packet
	dsi_set_cmdq(data_array, 1, 1);
	
	data_array[0]= 0x002c3909;
	dsi_set_cmdq(data_array, 1, 0);
}


static void lcm_setbacklight(unsigned int level)
{
	unsigned int default_level = 0;
	unsigned int mapped_level = 0;

	//for LGE backlight IC mapping table
	if(level > 255) 
			level = 255;

	if(level >0) 
			mapped_level = default_level+(level)*(255-default_level)/(255);
	else
			mapped_level=0;

	// Refresh value of backlight level.
	lcm_backlight_level_setting[0].para_list[0] = mapped_level;

	push_table(lcm_backlight_level_setting, sizeof(lcm_backlight_level_setting) / sizeof(struct LCM_setting_table), 1);
}

static void lcm_setbacklight_mode(unsigned int mode)
{
	lcm_backlight_mode_setting[0].para_list[0] = mode;
	push_table(lcm_backlight_mode_setting, sizeof(lcm_backlight_mode_setting) / sizeof(struct LCM_setting_table), 1);
}

static void lcm_setpwm(unsigned int divider)
{
	// TBD
}


static unsigned int lcm_getpwm(unsigned int divider)
{
	// ref freq = 15MHz, B0h setting 0x80, so 80.6% * freq is pwm_clk;
	// pwm_clk / 255 / 2(lcm_setpwm() 6th params) = pwm_duration = 23706
	unsigned int pwm_clk = 23706 / (1<<divider);	
	return pwm_clk;
}

static unsigned int lcm_compare_id(void)
{
	int array[4];
	char buffer[5];
	char id_high=0;
	char id_low=0;
	int id=0;

    SET_RESET_PIN(1);
    MDELAY(1);
    SET_RESET_PIN(0);
    MDELAY(10);
    SET_RESET_PIN(1);
    MDELAY(200);

	array[0] = 0x00043700;
	dsi_set_cmdq(array, 1, 1);
	read_reg_v2(0xD3, buffer, 4);

	id_high = buffer[2];
	id_low = buffer[3];
	id = (id_high<<8) | id_low;

#ifdef BUILD_LK
		printf("lcm_compare_id------------------: 0x%08x, 0x%08x, 0x%08x, 0x%08x\n", buffer[0], buffer[1], buffer[2], buffer[3]);
#else
		printk("lcm_compare_id------------------: 0x%08x, 0x%08x, 0x%08x, 0x%08x\n", buffer[0], buffer[1], buffer[2], buffer[3]);
#endif

	return (0x9488 == id)?1:0;
}

// ---------------------------------------------------------------------------
//  Get LCM Driver Hooks
// ---------------------------------------------------------------------------
LCM_DRIVER ili9488_lcm_drv = 
{
    .name			= "ili9488",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
	.compare_id    = lcm_compare_id,	
	.update         = lcm_update,
	.set_backlight	= lcm_setbacklight,
//	.set_backlight_mode = lcm_setbacklight_mode,
	//.set_pwm        = lcm_setpwm,
	//.get_pwm        = lcm_getpwm  
};

