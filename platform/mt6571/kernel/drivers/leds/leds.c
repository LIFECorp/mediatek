/*
 * drivers/leds/leds-mt65xx.c
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file COPYING in the main directory of this archive for
 * more details.
 *
 * mt65xx leds driver
 *
 */

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/string.h>
#include <linux/ctype.h>
#include <linux/leds.h>
#include <linux/leds-mt65xx.h>
#include <linux/workqueue.h>
#include <linux/wakelock.h>
#include <linux/slab.h>
//#include <cust_leds.h>
//#include <cust_leds_def.h>
#include <mach/mt_pwm.h>
#include <mach/mt_leds.h>
#include <mach/mt_boot.h>
//#include <mach/mt_gpio.h>
#include <mach/pmic_mt6329_hw_bank1.h> 
#include <mach/pmic_mt6329_sw_bank1.h> 
#include <mach/pmic_mt6329_hw.h>
#include <mach/pmic_mt6329_sw.h>
#include <mach/upmu_common_sw.h>
#include <mach/upmu_hw.h>
#include "leds_sw.h"
//#include <linux/leds_sw.h>
//#include <mach/mt_pmic_feature_api.h>
//#include <mach/mt_boot.h>

static DEFINE_MUTEX(leds_mutex);
static DEFINE_MUTEX(leds_pmic_mutex);
//#define ISINK_CHOP_CLK
/****************************************************************************
 * variables
 ***************************************************************************/
//struct cust_mt65xx_led* bl_setting_hal = NULL;
 static unsigned int bl_brightness_hal = 102;
static unsigned int bl_duty_hal = 21;
static unsigned int bl_div_hal = CLK_DIV1;
static unsigned int bl_frequency_hal = 32000;
struct wake_lock leds_suspend_lock;

u32 isink_step[] =
{
		ISINK_STEP_04MA,	ISINK_STEP_08MA,        ISINK_STEP_12MA,   ISINK_STEP_16MA,   ISINK_STEP_20MA,   ISINK_STEP_24MA,
		ISINK_STEP_32MA,        ISINK_STEP_40MA,        ISINK_STEP_48MA
};
#define ISINK_STEP_SIZE         (sizeof(isink_step)/sizeof(isink_step[0]))

static struct PMIC_CUST_ISINK *g_isink_data[ISINK_NUM];
static unsigned int isink_indicator_en = 0;
static unsigned int isink_backlight_en = 0;

/****************************************************************************
 * DEBUG MACROS
 ***************************************************************************/
static int debug_enable_led_hal = 1;
#define LEDS_DEBUG(format, args...) do{ \
	if(debug_enable_led_hal) \
	{\
		printk(KERN_WARNING format,##args);\
	}\
}while(0)

static int detail_debug_enable = 0;
#define LEDS_DETAIL_DEBUG(format, args...) do{ \
	if(detail_debug_enable) \
	{\
		printk(KERN_EMERG format,##args);\
	}\
}while(0)

/****************************************************************************
 * custom APIs
***************************************************************************/
extern unsigned int brightness_mapping(unsigned int level);
extern S32 pwrap_read( U32  adr, U32 *rdata );
// Use Old Mode of PWM to suppoort 256 backlight level
#define BACKLIGHT_LEVEL_PWM_256_SUPPORT 256
extern unsigned int Cust_GetBacklightLevelSupport_byPWM(void);

/*****************PWM *************************************************/
static int time_array_hal[PWM_DIV_NUM]={256,512,1024,2048,4096,8192,16384,32768};
static unsigned int div_array_hal[PWM_DIV_NUM] = {1,2,4,8,16,32,64,128}; 
static unsigned int backlight_PWM_div_hal = CLK_DIV1;// this para come from cust_leds.

/******************************************************************************
   for DISP backlight High resolution 
******************************************************************************/
#ifdef LED_INCREASE_LED_LEVEL_MTKPATCH
#define MT_LED_INTERNAL_LEVEL_BIT_CNT 10
#endif
/****************************************************************************
 * func:return global variables
 ***************************************************************************/

void mt_leds_wake_lock_init(void)
{
	wake_lock_init(&leds_suspend_lock, WAKE_LOCK_SUSPEND, "leds wakelock");
}

unsigned int mt_get_bl_brightness(void)
{
	return bl_brightness_hal;
}

unsigned int mt_get_bl_duty(void)
{
	return bl_duty_hal;
}
unsigned int mt_get_bl_div(void)
{
	return bl_div_hal;
}
unsigned int mt_get_bl_frequency(void)
{
	return bl_frequency_hal;
}

unsigned int *mt_get_div_array(void)
{
	return &div_array_hal[0];
}

void mt_set_bl_duty(unsigned int level)
{
	bl_duty_hal = level;
}

void mt_set_bl_div(unsigned int div)
{
	bl_div_hal = div;
}

void mt_set_bl_frequency(unsigned int freq)
{
	 bl_frequency_hal = freq;
}

struct cust_mt65xx_led * mt_get_cust_led_list(void)
{
	return get_cust_led_list();
}


/****************************************************************************
 * internal functions
 ***************************************************************************/
static void PMIC_Led_Control(enum mt65xx_led_pmic pmic_type, struct PMIC_ISINK_CONFIG *config_data);
static void PMIC_Backlight_Control(enum mt65xx_led_pmic pmic_type, PMIC_ISINK_BACKLIGHT_CONTROL control, struct PMIC_ISINK_CONFIG *config_data);
static void PMIC_ISINK_Enable(PMIC_ISINK_CHANNEL channel);
static void PMIC_ISINK_Disable(PMIC_ISINK_CHANNEL channel);
static void PMIC_ISINK_Easy_Config(PMIC_ISINK_CHANNEL channel, struct PMIC_ISINK_CONFIG *config_data);
static void PMIC_ISINK_Current_Config(PMIC_ISINK_CHANNEL channel);
static void PMIC_ISINK_Brightness_Config(PMIC_ISINK_CHANNEL channel, struct PMIC_ISINK_CONFIG *config_data);
static void PMIC_Led_Brightness_Control(enum mt65xx_led_pmic pmic_type, struct PMIC_ISINK_CONFIG *config_data, u32 level);
static unsigned int Value_to_RegisterIdx(u32 val, u32 *table);
static unsigned int PMIC_ISINK_CHANNEL_LOOKUP_BY_GROUP(enum mt65xx_led_pmic pmic_type);
static void PMIC_ISINK_GROUP_CONTROL(enum mt65xx_led_pmic pmic_type, PMIC_ISINK_CONTROL_API handler, struct PMIC_ISINK_CONFIG *config_data);
static unsigned int led_pmic_pwrap_read(U32 addr);
static void led_register_dump(void);

//#if 0
static int brightness_mapto64(int level)
{
        if (level < 30)
                return (level >> 1) + 7;
        else if (level <= 120)
                return (level >> 2) + 14;
        else if (level <= 160)
                return level / 5 + 20;
        else
                return (level >> 3) + 33;
}



static int find_time_index(int time)
{	
	int index = 0;	
	while(index < 8)	
	{		
		if(time<time_array_hal[index])			
			return index;		
		else
			index++;
	}	
	return PWM_DIV_NUM-1;
}

int mt_led_set_pwm(int pwm_num, struct nled_setting* led)
{
	struct pwm_spec_config pwm_setting;
	int time_index = 0;
	pwm_setting.pwm_no = pwm_num;
    pwm_setting.mode = PWM_MODE_OLD;
    
        LEDS_DEBUG("[LED]led_set_pwm: mode=%d,pwm_no=%d\n", led->nled_mode, pwm_num);  
	// In 6571, we won't choose 32K to be the clock src of old mode because of system performance.
	// The setting here will be clock src = 26MHz, CLKSEL = 26M/1625 (i.e. 16K)
	pwm_setting.clk_src = PWM_CLK_OLD_MODE_32K;
    
	switch (led->nled_mode)
	{
		case NLED_OFF : // Actually, the setting still can not to turn off NLED. We should disable PWM to turn off NLED.
			pwm_setting.PWM_MODE_OLD_REGS.THRESH = 0;
			pwm_setting.clk_div = CLK_DIV1;
			pwm_setting.PWM_MODE_OLD_REGS.DATA_WIDTH = 100;
			break;
            
		case NLED_ON :
			pwm_setting.PWM_MODE_OLD_REGS.THRESH = 30/2;
			pwm_setting.clk_div = CLK_DIV1;			
			pwm_setting.PWM_MODE_OLD_REGS.DATA_WIDTH = 100/2;
			break;
            
		case NLED_BLINK :
			LEDS_DEBUG("[LED]LED blink on time = %d, offtime = %d\n", led->blink_on_time, led->blink_off_time);
			time_index = find_time_index(led->blink_on_time + led->blink_off_time);
			LEDS_DEBUG("[LED]LED div is %d\n",time_index);
			pwm_setting.clk_div = time_index;
			pwm_setting.PWM_MODE_OLD_REGS.DATA_WIDTH = (led->blink_on_time + led->blink_off_time) * MIN_FRE_OLD_PWM / div_array_hal[time_index];
			pwm_setting.PWM_MODE_OLD_REGS.THRESH = (led->blink_on_time*100) / (led->blink_on_time + led->blink_off_time);
	}
	
	pwm_setting.PWM_MODE_FIFO_REGS.IDLE_VALUE = 0;
	pwm_setting.PWM_MODE_FIFO_REGS.GUARD_VALUE = 0;
	pwm_setting.PWM_MODE_FIFO_REGS.GDURATION = 0;
	pwm_setting.PWM_MODE_FIFO_REGS.WAVE_NUM = 0;
	pwm_set_spec_config(&pwm_setting);

	return 0;
	
}
#if 0
static int led_breath_pmic(enum mt65xx_led_pmic pmic_type, struct nled_setting* led)
{
        struct PMIC_ISINK_CONFIG isink_config;
	LEDS_DEBUG("[LED]mt_led_blink_pmic: pmic_type = %d\n", pmic_type);  
	
	if((pmic_type != MT65XX_LED_PMIC_NLED_ISINK0 && pmic_type!= MT65XX_LED_PMIC_NLED_ISINK1 \
        && pmic_type != MT65XX_LED_PMIC_NLED_ISINK2 && pmic_type != MT65XX_LED_PMIC_NLED_ISINK3 \
        && pmic_type != MT65XX_LED_PMIC_NLED_ISINK_GROUP0 && pmic_type != MT65XX_LED_PMIC_NLED_ISINK_GROUP1 \
        && pmic_type != MT65XX_LED_PMIC_NLED_ISINK_GROUP2 && pmic_type != MT65XX_LED_PMIC_NLED_ISINK_GROUP3) \
        || led->nled_mode != NLED_BLINK) {
		return -1;
	}

        upmu_set_rg_drv_2m_ck_pdn(0x0); // Disable power down (Indicator no need?)
        upmu_set_rg_drv_32k_ck_pdn(0x0); // Disable power down
        upmu_set_isink_phase_dly_tc(0x0); // TC = 0.5us
    
        isink_config.mode = ISINK_BREATH_MODE;
        isink_config.duty = 31; // useless for breath mode
        isink_config.dim_fsel = 0; // 1K = 32000 / (0 + 1) / 32 (useless for breath mode)
        isink_config.sfstr_tc = 0; // 0.5us
        isink_config.sfstr_en = 0; // Disable soft start
        isink_config.breath_trf_sel = 0x04; // 0.926s
        isink_config.breath_ton_sel = 0x02; // 0.523s
        isink_config.breath_toff_sel = 0x03; // 1.417s
        PMIC_Led_Control(pmic_type, &isink_config);
	return 0;
}
#endif // End of led_breath_pmic

#define PMIC_PERIOD_NUM 9
// 0.01 Hz -> 1 / 0.01 * 1000 = 100000 ms
//int pmic_period_array[] = {250,500,1000,1250,1666,2000,2500,10000};
//int pmic_freqsel_array[] = {0, 4, 199, 499, 999, 1999, 1999, 1999};
#if 0
static int find_time_index_pmic(int time_ms) {
	int i;
        //time_ms = time_ms / 100;
	for(i = 0; i < PMIC_PERIOD_NUM; i++) {
		if(time_ms <= pmic_period_array[i]) {
			return i;
		} else {
			continue;
		}
	}
	return PMIC_PERIOD_NUM - 1;
}
#endif
int mt_led_blink_pmic(enum mt65xx_led_pmic pmic_type, struct nled_setting* led) {
	int time_index = 0;
	int duty = 0;
        struct PMIC_ISINK_CONFIG isink_config;
	LEDS_DEBUG("[LED]mt_led_blink_pmic: pmic_type = %d\n", pmic_type);  
	
	if((pmic_type != MT65XX_LED_PMIC_NLED_ISINK0 && pmic_type != MT65XX_LED_PMIC_NLED_ISINK1 && \
            pmic_type != MT65XX_LED_PMIC_NLED_ISINK2 && pmic_type != MT65XX_LED_PMIC_NLED_ISINK3) || \
            led->nled_mode != NLED_BLINK) {
		return -1;
	}
				
	LEDS_DEBUG("[LED]LED blink on time = %d, off time = %d\n", led->blink_on_time, led->blink_off_time);
	time_index = (led->blink_on_time + led->blink_off_time) - 1;
	// LEDS_DEBUG("[LED]LED index is %d, freqsel = %d\n", time_index, pmic_freqsel_array[time_index]);
	duty = 32 * led->blink_on_time / (led->blink_on_time + led->blink_off_time);
        isink_config.mode = ISINK_PWM_MODE;
        isink_config.duty = duty;
        isink_config.dim_fsel = time_index;
        isink_config.sfstr_tc = 0; // 0.5us
        isink_config.sfstr_en = 0; // Disable soft start
        isink_config.breath_trf_sel = 0x00; // 0.123s
        isink_config.breath_ton_sel = 0x00; // 0.123s
        isink_config.breath_toff_sel = 0x00; // 0.246s
        PMIC_Led_Control(pmic_type, &isink_config);
	return 0;
}


int mt_backlight_set_pwm(int pwm_num, u32 level, u32 div, struct PWM_config *config_data)
{
	struct pwm_spec_config pwm_setting;
	unsigned int BacklightLevelSupport = Cust_GetBacklightLevelSupport_byPWM();

	pwm_setting.pwm_no = pwm_num;
	if (BacklightLevelSupport == BACKLIGHT_LEVEL_PWM_256_SUPPORT)
		pwm_setting.mode = PWM_MODE_OLD;
	else
		pwm_setting.mode = PWM_MODE_FIFO; //new mode fifo and periodical mode

	pwm_setting.pmic_pad = config_data->pmic_pad;
		
	if(config_data->div)
	{
		pwm_setting.clk_div = config_data->div;
		backlight_PWM_div_hal = config_data->div;
	}
	else
	{
		pwm_setting.clk_div = div;
	}	
		
	if(BacklightLevelSupport== BACKLIGHT_LEVEL_PWM_256_SUPPORT)
	{
		if(config_data->clock_source)
		{
				pwm_setting.clk_src = PWM_CLK_OLD_MODE_BLOCK;
			}
			else
			{
				pwm_setting.clk_src = PWM_CLK_OLD_MODE_32K;	 // actually. it's block/1625 = 26M/1625 = 16KHz @ MT6571
			}
	
			pwm_setting.PWM_MODE_OLD_REGS.IDLE_VALUE = 0;
			pwm_setting.PWM_MODE_OLD_REGS.GUARD_VALUE = 0;
			pwm_setting.PWM_MODE_OLD_REGS.GDURATION = 0;
			pwm_setting.PWM_MODE_OLD_REGS.WAVE_NUM = 0;
			pwm_setting.PWM_MODE_OLD_REGS.DATA_WIDTH = 255; // 256 level
			pwm_setting.PWM_MODE_OLD_REGS.THRESH = level;
	
			LEDS_DEBUG("[LEDS][%d]backlight_set_pwm:duty is %d/%d\n", BacklightLevelSupport, level, pwm_setting.PWM_MODE_OLD_REGS.DATA_WIDTH);
			LEDS_DEBUG("[LEDS][%d]backlight_set_pwm:clk_src/div is %d%d\n", BacklightLevelSupport, pwm_setting.clk_src, pwm_setting.clk_div);
			if(level >0 && level < 256)
			{
				pwm_set_spec_config(&pwm_setting);
				LEDS_DEBUG("[LEDS][%d]backlight_set_pwm: old mode: thres/data_width is %d/%d\n", BacklightLevelSupport, pwm_setting.PWM_MODE_OLD_REGS.THRESH, pwm_setting.PWM_MODE_OLD_REGS.DATA_WIDTH);
			}
			else
			{
				LEDS_DEBUG("[LEDS][%d]Error level in backlight\n", BacklightLevelSupport);
				mt_pwm_disable(pwm_setting.pwm_no, config_data->pmic_pad);
			}
			return 0;
	}
	else
	{
		if(config_data->clock_source)
			pwm_setting.clk_src = PWM_CLK_NEW_MODE_BLOCK;
		else
			pwm_setting.clk_src = PWM_CLK_NEW_MODE_BLOCK_DIV_BY_1625;
		
		if(config_data->High_duration && config_data->low_duration)
			{
				pwm_setting.PWM_MODE_FIFO_REGS.HDURATION = config_data->High_duration;
				pwm_setting.PWM_MODE_FIFO_REGS.LDURATION = pwm_setting.PWM_MODE_FIFO_REGS.HDURATION;
			}
		else
			{
				pwm_setting.PWM_MODE_FIFO_REGS.HDURATION = 4;
				pwm_setting.PWM_MODE_FIFO_REGS.LDURATION = 4;
			}
		pwm_setting.PWM_MODE_FIFO_REGS.IDLE_VALUE = 0;
		pwm_setting.PWM_MODE_FIFO_REGS.GUARD_VALUE = 0;
		pwm_setting.PWM_MODE_FIFO_REGS.STOP_BITPOS_VALUE = 31;
		pwm_setting.PWM_MODE_FIFO_REGS.GDURATION = (pwm_setting.PWM_MODE_FIFO_REGS.HDURATION+1)*32 - 1;
		pwm_setting.PWM_MODE_FIFO_REGS.WAVE_NUM = 0;
			
		LEDS_DEBUG("[LEDS]backlight_set_pwm:duty is %d\n", level);
		LEDS_DEBUG("[LEDS]backlight_set_pwm:clk_src/div/high/low is %d%d%d%d\n", pwm_setting.clk_src,pwm_setting.clk_div,pwm_setting.PWM_MODE_FIFO_REGS.HDURATION,pwm_setting.PWM_MODE_FIFO_REGS.LDURATION);
		if(level>0 && level <= 32)
		{
			pwm_setting.PWM_MODE_FIFO_REGS.GUARD_VALUE = 0;
			pwm_setting.PWM_MODE_FIFO_REGS.SEND_DATA0 =  (1 << level) - 1 ;
			//pwm_setting.PWM_MODE_FIFO_REGS.SEND_DATA1 = 0 ;
			pwm_set_spec_config(&pwm_setting);
		}else if(level>32 && level <=64)
		{
			pwm_setting.PWM_MODE_FIFO_REGS.GUARD_VALUE = 1;
			level -= 32;
			pwm_setting.PWM_MODE_FIFO_REGS.SEND_DATA0 = (1 << level) - 1 ;
			//pwm_setting.PWM_MODE_FIFO_REGS.SEND_DATA0 =  0xFFFFFFFF ;
			//pwm_setting.PWM_MODE_FIFO_REGS.SEND_DATA1 = (1 << level) - 1;
			pwm_set_spec_config(&pwm_setting);
		}else
		{
			LEDS_DEBUG("[LED]Error level in backlight\n");
			//mt_set_pwm_disable(pwm_setting.pwm_no);
			//mt_pwm_power_off(pwm_setting.pwm_no);
			mt_pwm_disable(pwm_setting.pwm_no, config_data->pmic_pad);
		}
			//printk("[LED]PWM con register is %x \n", INREG32(PWM_BASE + 0x0150));

		return 0;
	}
}

void mt_led_pwm_disable(int pwm_num)
{
	struct cust_mt65xx_led *cust_led_list = get_cust_led_list();
	mt_pwm_disable(pwm_num, cust_led_list->config_data.pmic_pad);
}

void mt_backlight_set_pwm_duty(int pwm_num, u32 level, u32 div, struct PWM_config *config_data)
{
	mt_backlight_set_pwm(pwm_num, level, div, config_data);
}

void mt_backlight_set_pwm_div(int pwm_num, u32 level, u32 div, struct PWM_config *config_data)
{
	mt_backlight_set_pwm(pwm_num, level, div, config_data);
}

void mt_backlight_get_pwm_fsel(unsigned int bl_div, unsigned int *bl_frequency)
{

}

void mt_store_pwm_register(unsigned int addr, unsigned int value)
{

}

unsigned int mt_show_pwm_register(unsigned int addr)
{
	return 0;
}
#if 0
static void pmic_isink_power_set(int enable)
{
	int i = 0;
	struct cust_mt65xx_led *cust_led_list = get_cust_led_list();
	if(enable) {
		for (i = 0; i < MT65XX_LED_TYPE_TOTAL; i++) {
			if ((cust_led_list[i].mode == MT65XX_LED_MODE_PMIC) && (cust_led_list[i].data == MT65XX_LED_PMIC_LCD_ISINK0 || 
				cust_led_list[i].data == MT65XX_LED_PMIC_LCD_ISINK1 || cust_led_list[i].data == MT65XX_LED_PMIC_LCD_ISINK2 || 
				cust_led_list[i].data == MT65XX_LED_PMIC_LCD_ISINK3)) {
				upmu_set_rg_drv_2m_ck_pdn(0);
				break;
		    }
		}
		for (i = 0; i < MT65XX_LED_TYPE_TOTAL; i++) {
			if ((cust_led_list[i].mode == MT65XX_LED_MODE_PMIC) && (cust_led_list[i].data == MT65XX_LED_PMIC_NLED_ISINK0 ||
				cust_led_list[i].data == MT65XX_LED_PMIC_NLED_ISINK1 || cust_led_list[i].data == MT65XX_LED_PMIC_NLED_ISINK2 || 
				cust_led_list[i].data == MT65XX_LED_PMIC_NLED_ISINK3)) {
				upmu_set_rg_drv_32k_ck_pdn(0);
				break;
		    }
		}
    }else {
    	upmu_set_rg_drv_2m_ck_pdn(1);
		upmu_set_rg_drv_32k_ck_pdn(1);
    }
}
// define for 15 level mapping(backlight controlled by PMIC ISINK channel)

static void brightness_set_pmic_isink_duty(unsigned int level)
{
	if(level<11) {
		upmu_set_isink_dim0_duty(level);
	}else {
		switch(level) {
			case 11:
				upmu_set_isink_dim0_duty(12);
				break;
			case 12:
				upmu_set_isink_dim0_duty(16);
				break;
			case 13:
				upmu_set_isink_dim0_duty(20);
				break;
			case 14:
				upmu_set_isink_dim0_duty(25);
				break;
			case 15:
				upmu_set_isink_dim0_duty(31);
				break;
			default:
				LEDS_DEBUG("mapping BLK level is error for ISINK!\n");
			break;
		}
	}
}
#endif
int mt_brightness_set_pmic(enum mt65xx_led_pmic pmic_type, u32 level, u32 div)
{
#define PMIC_ISINK_BACKLIGHT_LEVEL    80
	
	u32 tmp_level = level;
	static bool backlight_init_flag = false;
//	static bool first_time = true;
        struct PMIC_ISINK_CONFIG isink_config;
        static unsigned char duty_mapping[PMIC_ISINK_BACKLIGHT_LEVEL] = {
                0,	1,	2,	3,	4,	5,	6,	7,	8,	9,	10,	11,	
                12,     13,     14,     15,     16,     17,     18,     19,     20,     21,     22,     23,     
                24,     25,     26,     27,     28,     29,     30,     31,     16,     17,     18,     19,     
                20,     21,     22,     23,     24,     25,     26,     27,     28,     29,     30,     31,     
                21,     22,     23,     24,     25,     26,     27,     28,     29,     30,     31,     24,     
                25,     26,     27,     28,     29,     30,     31,     25,     26,     27,     28,     29,     
                30,     31,     26,     27,     28,     29,     30,     31,

    };
        static unsigned char current_mapping[PMIC_ISINK_BACKLIGHT_LEVEL] = {
                0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
                0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      
                0,      0,      0,      0,      0,      0,      0,      0,      1,      1,      1,      1,      
                1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      
                2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      3,      
                3,      3,      3,      3,      3,      3,      3,      4,      4,      4,      4,      4,      
                4,      4,      5,      5,      5,      5,      5,      5,
    };
	
	LEDS_DEBUG("[LED] PMIC Type: %d, Level: %d\n", pmic_type, level);
        led_register_dump();

        if (pmic_type == MT65XX_LED_PMIC_LCD_ISINK || \
            pmic_type == MT65XX_LED_PMIC_LCD_ISINK_GROUP0 || pmic_type == MT65XX_LED_PMIC_LCD_ISINK_GROUP1 || \
            pmic_type == MT65XX_LED_PMIC_LCD_ISINK_GROUP2 || pmic_type == MT65XX_LED_PMIC_LCD_ISINK_GROUP3)
	{
                LEDS_DETAIL_DEBUG("[LED] Backlight Configuration");
                // For backlight: Current: 24mA, PWM frequency: 20K, Soft start: off, Phase shift: on                
                isink_config.mode = ISINK_PWM_MODE;
                isink_config.dim_fsel = 0x2; // 20KHz
                isink_config.sfstr_tc = 0; // 0.5us
                isink_config.sfstr_en = 0; // Disable soft start
                isink_config.breath_trf_sel = 0x00; // 0.123s
                isink_config.breath_ton_sel = 0x00; // 0.123s
                isink_config.breath_toff_sel = 0x00; // 0.246s                
                
		if(backlight_init_flag == false)
		{
			upmu_set_isink_phase_dly_tc(0x0); // TC = 0.5us
                        PMIC_Backlight_Control(pmic_type, ISINK_BACKLIGHT_INIT, &isink_config);     
			backlight_init_flag = true;
		}
		
		if (level) 
		{
			level = brightness_mapping(tmp_level);
            if(level == 255)
            {
                                level = PMIC_ISINK_BACKLIGHT_LEVEL;
            }
            else
            {
                                level = ((level * PMIC_ISINK_BACKLIGHT_LEVEL) / 255) + 1;
            }

            LEDS_DEBUG("[LED]Level Mapping = %d \n", level);
            LEDS_DEBUG("[LED]ISINK DIM Duty = %d \n", duty_mapping[level-1]);
            LEDS_DEBUG("[LED]ISINK Current = %d \n", current_mapping[level-1]);
                        isink_config.duty = duty_mapping[level - 1];
                        isink_config.step = current_mapping[level - 1];
                        LEDS_DEBUG("[LED] isink_config.duty = %d \n", isink_config.duty);
                        LEDS_DEBUG("[LED] isink_config.step = %d \n", isink_config.step);                        
                        PMIC_Backlight_Control(pmic_type, ISINK_BACKLIGHT_ADJUST, &isink_config);
			bl_duty_hal = level;	
		}
		else 
		{
                        PMIC_Backlight_Control(pmic_type, ISINK_BACKLIGHT_DISABLE, &isink_config);
			bl_duty_hal = level;	
		}
        
		return 0;
	}
	else if(pmic_type == MT65XX_LED_PMIC_NLED_ISINK0 || pmic_type == MT65XX_LED_PMIC_NLED_ISINK1 || \
                pmic_type == MT65XX_LED_PMIC_NLED_ISINK2 || pmic_type == MT65XX_LED_PMIC_NLED_ISINK3 || \
                pmic_type == MT65XX_LED_PMIC_NLED_ISINK_GROUP0 || pmic_type == MT65XX_LED_PMIC_NLED_ISINK_GROUP1 || \
                pmic_type == MT65XX_LED_PMIC_NLED_ISINK_GROUP2 || pmic_type == MT65XX_LED_PMIC_NLED_ISINK_GROUP3)
	{
                LEDS_DETAIL_DEBUG("[LED] LED Configuration");
                isink_config.mode = ISINK_PWM_MODE;
                isink_config.duty = 15; // 16 / 32
                isink_config.dim_fsel = 0; // 1KHz
                isink_config.sfstr_tc = 0; // 0.5us
                isink_config.sfstr_en = 0; // Disable soft start
                isink_config.breath_trf_sel = 0x00; // 0.123s
                isink_config.breath_ton_sel = 0x00; // 0.123s
                isink_config.breath_toff_sel = 0x00; // 0.246s                
                PMIC_Led_Brightness_Control(pmic_type, &isink_config, level);
		return 0;
	}

	return -1;
}

int mt_brightness_set_pmic_duty_store(u32 level, u32 div)
{
	return -1;
}
int mt_mt65xx_led_set_cust(struct cust_mt65xx_led *cust, int level)
{
	struct nled_setting led_tmp_setting = {0,0,0};
	int tmp_level = level;
	unsigned int BacklightLevelSupport = Cust_GetBacklightLevelSupport_byPWM();

	//Mark out since the level is already cliped before sending in
	/*
		if (level > LED_FULL)
			level = LED_FULL;
		else if (level < 0)
			level = 0;
	*/	
    LEDS_DEBUG("mt65xx_leds_set_cust: set brightness, name:%s, mode:%d, level:%d\n", 
		cust->name, cust->mode, level);
	switch (cust->mode) {
		
		case MT65XX_LED_MODE_PWM:
			if(strcmp(cust->name,"lcd-backlight") == 0)
			{
				bl_brightness_hal = level;
				if(level == 0)
				{
					mt_pwm_disable(cust->data, cust->config_data.pmic_pad);
					
				}else
				{
					if (BacklightLevelSupport == BACKLIGHT_LEVEL_PWM_256_SUPPORT)
						level = brightness_mapping(tmp_level);
					else
						level = brightness_mapto64(tmp_level);
						
					mt_backlight_set_pwm(cust->data, level, bl_div_hal,&cust->config_data);
				}
                bl_duty_hal = level;	
				
			}else
			{
				if(level == 0)
				{
					led_tmp_setting.nled_mode = NLED_OFF;
					mt_led_set_pwm(cust->data,&led_tmp_setting);
					mt_pwm_disable(cust->data, cust->config_data.pmic_pad);
				}else
				{
					led_tmp_setting.nled_mode = NLED_ON;
					mt_led_set_pwm(cust->data,&led_tmp_setting);
				}
			}
			return 1;
          
		case MT65XX_LED_MODE_GPIO:
			LEDS_DEBUG("brightness_set_cust:go GPIO mode!!!!!\n");
			return ((cust_set_brightness)(cust->data))(level);
              
		case MT65XX_LED_MODE_PMIC:
			return mt_brightness_set_pmic(cust->data, level, bl_div_hal);
            
		case MT65XX_LED_MODE_CUST_LCM:
            if(strcmp(cust->name,"lcd-backlight") == 0)
			{
			    bl_brightness_hal = level;
            }
			LEDS_DEBUG("brightness_set_cust:backlight control by LCM\n");
			return ((cust_brightness_set)(cust->data))(level, bl_div_hal);

		
		case MT65XX_LED_MODE_CUST_BLS_PWM:
			if(strcmp(cust->name,"lcd-backlight") == 0)
			{
				bl_brightness_hal = level;
			}
			//LEDS_DEBUG("brightness_set_cust:backlight control by BLS_PWM!!\n");
			//#if !defined (MTK_AAL_SUPPORT)
			return ((cust_set_brightness)(cust->data))(level);
			//LEDS_DEBUG("brightness_set_cust:backlight control by BLS_PWM done!!\n");
			//#endif
            
		case MT65XX_LED_MODE_NONE:
		default:
			break;
	}
	return -1;
}

void mt_mt65xx_led_work(struct work_struct *work)
{
	struct mt65xx_led_data *led_data =
		container_of(work, struct mt65xx_led_data, work);

	LEDS_DEBUG("[LED]%s:%d\n", led_data->cust.name, led_data->level);
	mutex_lock(&leds_mutex);
	mt_mt65xx_led_set_cust(&led_data->cust, led_data->level);
	mutex_unlock(&leds_mutex);;
}

void mt_mt65xx_led_set(struct led_classdev *led_cdev, enum led_brightness level)
{
	struct mt65xx_led_data *led_data =
		container_of(led_cdev, struct mt65xx_led_data, cdev);
	//unsigned long flags;
	//spin_lock_irqsave(&leds_lock, flags);
	
#ifdef LED_INCREASE_LED_LEVEL_MTKPATCH
	
		if(level >> LED_RESERVEBIT_SHIFT)
		{
			if(LED_RESERVEBIT_PATTERN != (level >> LED_RESERVEBIT_SHIFT))
			{
				//sanity check for hidden code
				printk("incorrect input : %d,%d\n" , level , (level >> LED_RESERVEBIT_SHIFT));
				return;
			}
	
			if(MT65XX_LED_MODE_CUST_BLS_PWM != led_data->cust.mode)
			{
				//only BLS PWM support expand bit
				printk("Not BLS PWM %d \n" , led_data->cust.mode);
				return;
			}
	
			level &= ((1 << LED_RESERVEBIT_SHIFT) - 1);
	
			if((level +1) > (1 << MT_LED_INTERNAL_LEVEL_BIT_CNT))
			{
				//clip to max value
				level = (1 << MT_LED_INTERNAL_LEVEL_BIT_CNT) - 1;
			}
	
			led_cdev->brightness = ((level + (1 << (MT_LED_INTERNAL_LEVEL_BIT_CNT - 9))) >> (MT_LED_INTERNAL_LEVEL_BIT_CNT - 8));//brightness is 8 bit level
			if(led_cdev->brightness > led_cdev->max_brightness)
			{
				led_cdev->brightness = led_cdev->max_brightness;
			}
	
			if(led_data->level != level)
			{
				led_data->level = level;
				mt_mt65xx_led_set_cust(&led_data->cust, led_data->level);
			}
		}
		else
		{
			if(led_data->level != level)
			{
				led_data->level = level;
				if(strcmp(led_data->cust.name,"lcd-backlight") != 0)
				{
					LEDS_DEBUG("[LED]Set NLED directly %d at time %lu\n",led_data->level,jiffies);
					schedule_work(&led_data->work);				
				}
			    else
				{
					LEDS_DEBUG("[LED]Set Backlight directly %d at time %lu\n",led_data->level,jiffies);
					if(MT65XX_LED_MODE_CUST_BLS_PWM == led_data->cust.mode)
					{
						mt_mt65xx_led_set_cust(&led_data->cust, ((((1 << MT_LED_INTERNAL_LEVEL_BIT_CNT) - 1)*level + 127)/255));
						//mt_mt65xx_led_set_cust(&led_data->cust, led_data->level);	
					}
					else
					{
						mt_mt65xx_led_set_cust(&led_data->cust, led_data->level);	
					}	
				}
			}
		}						
#else
	// do something only when level is changed
	if (led_data->level != level) {
		led_data->level = level;
		if(strcmp(led_data->cust.name,"lcd-backlight"))
		{
				schedule_work(&led_data->work);
		}else
		{
				LEDS_DEBUG("[LED]Set Backlight directly %d at time %lu\n",led_data->level,jiffies);
				mt_mt65xx_led_set_cust(&led_data->cust, led_data->level);	
		}
	}
	//spin_unlock_irqrestore(&leds_lock, flags);
#endif
}

int  mt_mt65xx_blink_set(struct led_classdev *led_cdev,
			     unsigned long *delay_on,
			     unsigned long *delay_off)
{
	struct mt65xx_led_data *led_data =
		container_of(led_cdev, struct mt65xx_led_data, cdev);
	static int got_wake_lock = 0;
	struct nled_setting nled_tmp_setting = {0,0,0};

	// only allow software blink when delay_on or delay_off changed
	if (*delay_on != led_data->delay_on || *delay_off != led_data->delay_off) {
		led_data->delay_on = *delay_on;
		led_data->delay_off = *delay_off;
		if (led_data->delay_on && led_data->delay_off) { // enable blink
			led_data->level = 255; // when enable blink  then to set the level  (255)
			//if(led_data->cust.mode == MT65XX_LED_MODE_PWM && 
			//(led_data->cust.data != PWM3 && led_data->cust.data != PWM4 && led_data->cust.data != PWM5))

			//AP PWM all support OLD mode in MT6589
			if(led_data->cust.mode == MT65XX_LED_MODE_PWM)
			{
				nled_tmp_setting.nled_mode = NLED_BLINK;
				nled_tmp_setting.blink_off_time = led_data->delay_off;
				nled_tmp_setting.blink_on_time = led_data->delay_on;
				mt_led_set_pwm(led_data->cust.data,&nled_tmp_setting);
				return 0;
			}
			else if((led_data->cust.mode == MT65XX_LED_MODE_PMIC) && \
                                (led_data->cust.data == MT65XX_LED_PMIC_NLED_ISINK0 || led_data->cust.data == MT65XX_LED_PMIC_NLED_ISINK1 || \
                                 led_data->cust.data == MT65XX_LED_PMIC_NLED_ISINK2 || led_data->cust.data == MT65XX_LED_PMIC_NLED_ISINK3 || \
                                 led_data->cust.data == MT65XX_LED_PMIC_NLED_ISINK_GROUP0 || led_data->cust.data == MT65XX_LED_PMIC_NLED_ISINK_GROUP1 || \
                                 led_data->cust.data == MT65XX_LED_PMIC_NLED_ISINK_GROUP2 || led_data->cust.data == MT65XX_LED_PMIC_NLED_ISINK_GROUP3))
			{			
				nled_tmp_setting.nled_mode = NLED_BLINK;
				nled_tmp_setting.blink_off_time = led_data->delay_off;
				nled_tmp_setting.blink_on_time = led_data->delay_on;
				mt_led_blink_pmic(led_data->cust.data, &nled_tmp_setting);
				return 0;
			}
			else if (!got_wake_lock) {
				wake_lock(&leds_suspend_lock);
				got_wake_lock = 1;
			}
		}
		else if (!led_data->delay_on && !led_data->delay_off) { // disable blink
			//if(led_data->cust.mode == MT65XX_LED_MODE_PWM && 
			//(led_data->cust.data != PWM3 && led_data->cust.data != PWM4 && led_data->cust.data != PWM5))

			//AP PWM all support OLD mode in MT6589
			
			if(led_data->cust.mode == MT65XX_LED_MODE_PWM)
			{
				nled_tmp_setting.nled_mode = NLED_OFF;
				mt_led_set_pwm(led_data->cust.data,&nled_tmp_setting);
				return 0;
			}
			else if((led_data->cust.mode == MT65XX_LED_MODE_PMIC) && \
                                (led_data->cust.data == MT65XX_LED_PMIC_NLED_ISINK0 || led_data->cust.data == MT65XX_LED_PMIC_NLED_ISINK1 || \
                                 led_data->cust.data == MT65XX_LED_PMIC_NLED_ISINK2 || led_data->cust.data == MT65XX_LED_PMIC_NLED_ISINK3 || \
                                 led_data->cust.data == MT65XX_LED_PMIC_NLED_ISINK_GROUP0 || led_data->cust.data == MT65XX_LED_PMIC_NLED_ISINK_GROUP1 || \
                                 led_data->cust.data == MT65XX_LED_PMIC_NLED_ISINK_GROUP2 || led_data->cust.data == MT65XX_LED_PMIC_NLED_ISINK_GROUP3))				
			{
				mt_brightness_set_pmic(led_data->cust.data, 0, 0);
				return 0;
			} 
			else if (got_wake_lock) {
				wake_unlock(&leds_suspend_lock);
				got_wake_lock = 0;
			}
		}
		return -1;
	}

	// delay_on and delay_off are not changed
	return 0;
}

static void PMIC_ISINK_Enable(PMIC_ISINK_CHANNEL channel)
{
        LEDS_DETAIL_DEBUG("[LED] %s->ISINK%d: Enable\n", __func__, channel);

        switch (channel) {
	        case ISINK0:
                        upmu_set_rg_isink0_ck_pdn(0x0); // Disable power down                        
                        upmu_set_isink_ch0_en(0x1); // Turn on ISINK Channel 0 
                        break;
                case ISINK1:
                        upmu_set_rg_isink1_ck_pdn(0x0); // Disable power down                        
                        upmu_set_isink_ch1_en(0x1); // Turn on ISINK Channel 1
                        break;                        
                case ISINK2:
                        upmu_set_rg_isink2_ck_pdn(0x0); // Disable power down                        
                        upmu_set_isink_ch2_en(0x1); // Turn on ISINK Channel 2
                        break;     
                case ISINK3:
                        upmu_set_rg_isink3_ck_pdn(0x0); // Disable power down                        
                        upmu_set_isink_ch3_en(0x1); // Turn on ISINK Channel 3
                        break;                                                     
		default:
			break;                        
        }
        
        if(g_isink_data[channel]->usage == ISINK_AS_INDICATOR)
        {
                isink_indicator_en |= (1 << channel);
                upmu_set_rg_drv_32k_ck_pdn(0x0); // Disable power down                    
        }
        else if(g_isink_data[channel]->usage == ISINK_AS_BACKLIGHT)
        {
                isink_backlight_en |= (1 << channel);
                upmu_set_rg_drv_2m_ck_pdn(0x0); // Disable power down
        }
        LEDS_DETAIL_DEBUG("[LED] ISINK Usage: Indicator->%x, Backlight->%x\n", isink_indicator_en, isink_backlight_en);        
}

static void PMIC_ISINK_Disable(PMIC_ISINK_CHANNEL channel)
{
        LEDS_DETAIL_DEBUG("[LED] %s->ISINK%d: Disable\n", __func__, channel);
        
        switch (channel) {
	        case ISINK0:
                        upmu_set_isink_ch0_en(0x0); // Turn off ISINK Channel 0
                        mdelay(1);
                        upmu_set_rg_isink0_ck_pdn(0x1); // Enable power down
                        break;
                case ISINK1:
                        upmu_set_isink_ch1_en(0x0); // Turn off ISINK Channel 1
                        mdelay(1);
                        upmu_set_rg_isink1_ck_pdn(0x1); // Enable power down                        
                        break;                        
                case ISINK2:
                        upmu_set_isink_ch2_en(0x0); // Turn off ISINK Channel 2
                        mdelay(1);
                        upmu_set_rg_isink2_ck_pdn(0x1); // Enable power down                        
                        break;     
                case ISINK3:
                        upmu_set_isink_ch3_en(0x0); // Turn off ISINK Channel 3
                        mdelay(1);                        
                        upmu_set_rg_isink3_ck_pdn(0x1); // Enable power down                        
                        break;                                                     
		default:
			break;                        
        } 
        if(g_isink_data[channel]->usage == ISINK_AS_INDICATOR)
        {
                isink_indicator_en &= ~(1 << channel);
        }
        else if(g_isink_data[channel]->usage == ISINK_AS_BACKLIGHT)
        {
                isink_backlight_en &= ~(1 << channel);
        }
        LEDS_DEBUG("[LED] ISINK Usage: Indicator->%x, Backlight->%x\n", isink_indicator_en, isink_backlight_en);             
        if(0 == isink_indicator_en)
        {
                upmu_set_rg_drv_32k_ck_pdn(0x1); // Enable power down
        }
        else if(0 == isink_backlight_en)
        {
                upmu_set_rg_drv_2m_ck_pdn(0x1); // Enable power down
        }
}

static void PMIC_ISINK_Easy_Config(PMIC_ISINK_CHANNEL channel, struct PMIC_ISINK_CONFIG *config_data)
{
        PMIC_ISINK_USAGE usage = g_isink_data[channel]->usage;
        LEDS_DETAIL_DEBUG("[LED] PMIC_ISINK_Easy_Config->ISINK%d: usage = %d\n", channel, usage);
        LEDS_DETAIL_DEBUG("[LED] config_data->mode %d\n", config_data->mode);
        LEDS_DETAIL_DEBUG("[LED] config_data->duty %d\n", config_data->duty);
        LEDS_DETAIL_DEBUG("[LED] config_data->dim_fsel %d\n", config_data->dim_fsel);
        LEDS_DETAIL_DEBUG("[LED] config_data->sfstr_tc %d\n", config_data->sfstr_tc);
        LEDS_DETAIL_DEBUG("[LED] config_data->sfstr_en %d\n", config_data->sfstr_en);
        LEDS_DETAIL_DEBUG("[LED] config_data->breath_trf_sel %d\n", config_data->breath_trf_sel);        
        LEDS_DETAIL_DEBUG("[LED] config_data->breath_ton_sel %d\n", config_data->breath_ton_sel); 
        LEDS_DETAIL_DEBUG("[LED] config_data->breath_toff_sel %d\n", config_data->breath_toff_sel);        
	switch (channel) {
		case ISINK0:
                        // upmu_set_rg_isink0_ck_pdn(0x0); // Disable power down    
                        
                        if(usage == ISINK_AS_INDICATOR)
                        {
                                upmu_set_rg_isink0_ck_sel(0x0); // Freq = 32KHz for Indicator
                                upmu_set_isink_dim0_duty(config_data->duty);
                                upmu_set_isink_ch0_mode(config_data->mode);
                                upmu_set_isink_dim0_fsel(config_data->dim_fsel);
                                upmu_set_isink_sfstr0_tc(config_data->sfstr_tc);
                                upmu_set_isink_sfstr0_en(config_data->sfstr_en);
                                upmu_set_isink_breath0_trf_sel(config_data->breath_trf_sel);
                                upmu_set_isink_breath0_ton_sel(config_data->breath_ton_sel);
                                upmu_set_isink_breath0_toff_sel(config_data->breath_toff_sel);
                                upmu_set_isink_phase0_dly_en(0x0); // Disable phase delay
                                upmu_set_isink_chop0_en(0x0); // Disable CHOP clk                                   
                        }
                        else
                        {
                                upmu_set_rg_isink0_ck_sel(0x1); // Freq = 1Mhz for Backlight
                                upmu_set_isink_ch0_mode(config_data->mode);
                                upmu_set_isink_dim0_fsel(config_data->dim_fsel);
                                upmu_set_isink_sfstr0_tc(config_data->sfstr_tc);
                                upmu_set_isink_sfstr0_en(config_data->sfstr_en);
                                upmu_set_isink_breath0_trf_sel(config_data->breath_trf_sel);
                                upmu_set_isink_breath0_ton_sel(config_data->breath_ton_sel);
                                upmu_set_isink_breath0_toff_sel(config_data->breath_toff_sel);
                                upmu_set_isink_phase0_dly_en(0x1); // Enable phase delay
                                upmu_set_isink_chop0_en(0x1); // Enable CHOP clk                                   
                        }
                        break;
                        
		case ISINK1:
                        // upmu_set_rg_isink1_ck_pdn(0x0); // Disable power down    
                        
                        if(usage == ISINK_AS_INDICATOR)
                        {
                                upmu_set_rg_isink1_ck_sel(0x0); // Freq = 32KHz for Indicator            
                                upmu_set_isink_dim1_duty(config_data->duty);
                                upmu_set_isink_ch1_mode(config_data->mode);
                                upmu_set_isink_dim1_fsel(config_data->dim_fsel);
                                upmu_set_isink_sfstr1_tc(config_data->sfstr_tc);
                                upmu_set_isink_sfstr1_en(config_data->sfstr_en);
                                upmu_set_isink_breath0_trf_sel(config_data->breath_trf_sel);
                                upmu_set_isink_breath0_ton_sel(config_data->breath_ton_sel);
                                upmu_set_isink_breath0_toff_sel(config_data->breath_toff_sel);                                
        			upmu_set_isink_phase1_dly_en(0x0); // Disable phase delay
                                upmu_set_isink_chop1_en(0x0); // Disable CHOP clk                                   
                        }
                        else
                        {
                                upmu_set_rg_isink1_ck_sel(0x1); // Freq = 1Mhz for Backlight
                                upmu_set_isink_ch1_mode(config_data->mode);
                                upmu_set_isink_dim1_fsel(config_data->dim_fsel);
                                upmu_set_isink_sfstr1_tc(config_data->sfstr_tc);
                                upmu_set_isink_sfstr1_en(config_data->sfstr_en);
                                upmu_set_isink_breath1_trf_sel(config_data->breath_trf_sel);
                                upmu_set_isink_breath1_ton_sel(config_data->breath_ton_sel);
                                upmu_set_isink_breath1_toff_sel(config_data->breath_toff_sel);
                                upmu_set_isink_phase1_dly_en(0x1); // Enable phase delay
                                upmu_set_isink_chop1_en(0x1); // Enable CHOP clk      
                        }
                        break;
                        
		case ISINK2:
                        // upmu_set_rg_isink2_ck_pdn(0x0); // Disable power down    
                        
                        if(usage == ISINK_AS_INDICATOR)
                        {
                                upmu_set_rg_isink2_ck_sel(0x0); // Freq = 32KHz for Indicator            
                                upmu_set_isink_dim1_duty(config_data->duty);                                
                                upmu_set_isink_ch2_mode(config_data->mode);
                                upmu_set_isink_dim2_fsel(config_data->dim_fsel);
                                upmu_set_isink_sfstr2_tc(config_data->sfstr_tc);
                                upmu_set_isink_sfstr2_en(config_data->sfstr_en);
                                upmu_set_isink_breath2_trf_sel(config_data->breath_trf_sel);
                                upmu_set_isink_breath2_ton_sel(config_data->breath_ton_sel);
                                upmu_set_isink_breath2_toff_sel(config_data->breath_toff_sel);                                
        			upmu_set_isink_phase2_dly_en(0x0); // Disable phase delay
                                upmu_set_isink_chop2_en(0x0); // Disable CHOP clk                                   
                        }
                        else
                        {
                                upmu_set_rg_isink2_ck_sel(0x1); // Freq = 1Mhz for Backlight
                                upmu_set_isink_ch2_mode(config_data->mode);
                                upmu_set_isink_dim2_fsel(config_data->dim_fsel);
                                upmu_set_isink_sfstr2_tc(config_data->sfstr_tc);
                                upmu_set_isink_sfstr2_en(config_data->sfstr_en);
                                upmu_set_isink_breath2_trf_sel(config_data->breath_trf_sel);
                                upmu_set_isink_breath2_ton_sel(config_data->breath_ton_sel);
                                upmu_set_isink_breath2_toff_sel(config_data->breath_toff_sel);
                                upmu_set_isink_phase2_dly_en(0x1); // Enable phase delay
                                upmu_set_isink_chop2_en(0x1); // Enable CHOP clk    
                        }
                        break;
                        
		case ISINK3:
                        // upmu_set_rg_isink3_ck_pdn(0x0); // Disable power down    
                        
                        if(usage == ISINK_AS_INDICATOR)
                        {
                                upmu_set_rg_isink3_ck_sel(0x0); // Freq = 32KHz for Indicator            
                                upmu_set_isink_dim3_duty(config_data->duty);                                 
                                upmu_set_isink_ch3_mode(config_data->mode);
                                upmu_set_isink_dim3_fsel(config_data->dim_fsel);
                                upmu_set_isink_sfstr3_tc(config_data->sfstr_tc);
                                upmu_set_isink_sfstr3_en(config_data->sfstr_en);
                                upmu_set_isink_breath3_trf_sel(config_data->breath_trf_sel);
                                upmu_set_isink_breath3_ton_sel(config_data->breath_ton_sel);
                                upmu_set_isink_breath3_toff_sel(config_data->breath_toff_sel);                                
        			upmu_set_isink_phase3_dly_en(0x0); // Disable phase delay
                                upmu_set_isink_chop3_en(0x0); // Disable CHOP clk                                   
                        }
                        else
                        {
                                upmu_set_rg_isink3_ck_sel(0x1); // Freq = 1Mhz for Backlight
                                upmu_set_isink_ch3_mode(config_data->mode);
                                upmu_set_isink_dim3_fsel(config_data->dim_fsel);
                                upmu_set_isink_sfstr3_tc(config_data->sfstr_tc);
                                upmu_set_isink_sfstr3_en(config_data->sfstr_en);
                                upmu_set_isink_breath3_trf_sel(config_data->breath_trf_sel);
                                upmu_set_isink_breath3_ton_sel(config_data->breath_ton_sel);
                                upmu_set_isink_breath3_toff_sel(config_data->breath_toff_sel);
                                upmu_set_isink_phase3_dly_en(0x1); // Enable phase delay
                                upmu_set_isink_chop3_en(0x1); // Enable CHOP clk    
                        }
                        break;                             
 		default:
			break;                        

	}
        
}

static void PMIC_ISINK_Current_Config(PMIC_ISINK_CHANNEL channel)
{
        unsigned int isink_current = 0;
        unsigned int double_en = 0;
        unsigned int idx = 0;
        PMIC_ISINK_STEP step = g_isink_data[channel]->step;
        LEDS_DEBUG("[LED] %s\n", __func__);
        if(step == ISINK_STEP_FOR_BACK_LIGHT && g_isink_data[channel]->usage == ISINK_AS_BACKLIGHT)
        {
                LEDS_DETAIL_DEBUG("[LED] ISINK_STEP_48MA\n");
                step = ISINK_STEP_48MA;
        }
        if(step >  ISINK_STEP_24MA)
        {
                LEDS_DETAIL_DEBUG("[LED] Enable Double Bit\n");
                isink_current = step >> 1;
                double_en = 1;
        }
        idx = Value_to_RegisterIdx(isink_current, isink_step);
        if(idx == 0xFFFFFFFF)
        {
                LEDS_DETAIL_DEBUG("[LED] ISINK%d Current not available\n", channel);
        }
        LEDS_DETAIL_DEBUG("[LED] ISINK%d: step = %d, double_en = %d\n", channel, idx, double_en);
        switch (channel) {
		case ISINK0:
                        upmu_set_isink_ch0_step(idx);
                        upmu_set_rg_isink0_double_en(double_en);
                        break;
		case ISINK1:
                        upmu_set_isink_ch1_step(idx);
                        upmu_set_rg_isink1_double_en(double_en);
                        break;
		case ISINK2:
                        upmu_set_isink_ch2_step(idx);
                        upmu_set_rg_isink2_double_en(double_en);
                        break;
		case ISINK3:
                        upmu_set_isink_ch3_step(idx);
                        upmu_set_rg_isink3_double_en(double_en);
                        break;    
                default:
                        break;
        }          
}

static void PMIC_ISINK_Brightness_Config(PMIC_ISINK_CHANNEL channel, struct PMIC_ISINK_CONFIG *config_data)
{
        LEDS_DETAIL_DEBUG("[LED] %s, duty = %d, step = %d\n", __func__, config_data->duty, config_data->step);
        switch (channel) {
		case ISINK0:
                        upmu_set_isink_dim0_duty(config_data->duty);
                        upmu_set_isink_ch0_step(config_data->step);
                        break;
		case ISINK1:
                        upmu_set_isink_dim1_duty(config_data->duty);
                        upmu_set_isink_ch1_step(config_data->step);
                        break;
		case ISINK2:
                        upmu_set_isink_dim2_duty(config_data->duty);
                        upmu_set_isink_ch2_step(config_data->step);
                        break;
		case ISINK3:
                        upmu_set_isink_dim3_duty(config_data->duty);
                        upmu_set_isink_ch3_step(config_data->step);
                        break;                        
                default:
                        break;
        }                  
}

static void PMIC_Led_Control(enum mt65xx_led_pmic pmic_type, struct PMIC_ISINK_CONFIG *config_data)
{
        PMIC_ISINK_CHANNEL isink_channel;
        if(pmic_type >= MT65XX_LED_PMIC_NLED_ISINK_GROUP_BEGIN && pmic_type < MT65XX_LED_PMIC_NLED_ISINK_GROUP_END)
        {
                PMIC_ISINK_GROUP_CONTROL(pmic_type, ISINK_EASY_CONFIG_API, config_data);
                PMIC_ISINK_GROUP_CONTROL(pmic_type, ISINK_CURRENT_CONFIG_API, NULL);
                PMIC_ISINK_GROUP_CONTROL(pmic_type, ISINK_ENABLE_API, NULL);
        }
        else
        {
                switch(pmic_type){
                        case MT65XX_LED_PMIC_NLED_ISINK0:        
                                isink_channel = ISINK0;
                                break;
                        case MT65XX_LED_PMIC_NLED_ISINK1:        
                                isink_channel = ISINK1;    
                                break;
                        case MT65XX_LED_PMIC_NLED_ISINK2:        
                                isink_channel = ISINK2;     
                                break;
                        case MT65XX_LED_PMIC_NLED_ISINK3:        
                                isink_channel = ISINK3;    
                                break;
                        default:
                                break;
                }                  
                PMIC_ISINK_Easy_Config(isink_channel, config_data);
                PMIC_ISINK_Current_Config(isink_channel);
                PMIC_ISINK_Enable(isink_channel);  
        }
}

static void PMIC_Led_Brightness_Control(enum mt65xx_led_pmic pmic_type, struct PMIC_ISINK_CONFIG *config_data, u32 level)
{
//        unsigned int i = 0;
//        unsigned int operation_channel = 0;
        static unsigned int led_first_time[ISINK_NUM] = {true, true, true, true};
        PMIC_ISINK_CHANNEL isink_channel = ISINK_NUM;

        LEDS_DEBUG("[LED] %s\n", __func__);        
        if(pmic_type >= MT65XX_LED_PMIC_NLED_ISINK_GROUP_BEGIN && pmic_type < MT65XX_LED_PMIC_NLED_ISINK_GROUP_END)
        {
                // operation_channel = PMIC_ISINK_CHANNEL_LOOKUP_BY_GROUP(pmic_type);

                PMIC_ISINK_GROUP_CONTROL(pmic_type, ISINK_EASY_CONFIG_API,  config_data);
                PMIC_ISINK_GROUP_CONTROL(pmic_type, ISINK_CURRENT_CONFIG_API, NULL);              
		
		if (level) 
		{
                        PMIC_ISINK_GROUP_CONTROL(pmic_type, ISINK_ENABLE_API, NULL);
		}
		else 
		{
                        PMIC_ISINK_GROUP_CONTROL(pmic_type, ISINK_DISABLE_API, NULL);
		}      
        }
        else
        {
                switch(pmic_type){
                        case MT65XX_LED_PMIC_NLED_ISINK0:        
                                isink_channel = ISINK0;
                                break;
                        case MT65XX_LED_PMIC_NLED_ISINK1:        
                                isink_channel = ISINK1;    
                                break;
                        case MT65XX_LED_PMIC_NLED_ISINK2:        
                                isink_channel = ISINK2;     
                                break;
                        case MT65XX_LED_PMIC_NLED_ISINK3:        
                                isink_channel = ISINK3;    
                                break;
                        default:
                                break;                                
                }   
                if(led_first_time[isink_channel] == true)
                {
                        PMIC_ISINK_Disable(isink_channel);
                        led_first_time[isink_channel] = false;
                }
                
                PMIC_ISINK_Easy_Config(isink_channel, config_data);
                PMIC_ISINK_Current_Config(isink_channel);
                
                if(level)
                {
                        PMIC_ISINK_Enable(isink_channel);  
                }
                else
                {
                        PMIC_ISINK_Disable(isink_channel);
                }
        }

}

static void PMIC_Backlight_Control(enum mt65xx_led_pmic pmic_type, PMIC_ISINK_BACKLIGHT_CONTROL control, struct PMIC_ISINK_CONFIG *config_data)
{
        unsigned int i;
        LEDS_DEBUG("[LED] %s\n", __func__);
        if(pmic_type >= MT65XX_LED_PMIC_LCD_ISINK_GROUP_BEGIN && pmic_type < MT65XX_LED_PMIC_LCD_ISINK_GROUP_END)
        {
                if(control == ISINK_BACKLIGHT_INIT)
                {
                        LEDS_DETAIL_DEBUG("[LED] Backlight Init\n");
                        PMIC_ISINK_GROUP_CONTROL(pmic_type, ISINK_EASY_CONFIG_API,  config_data);
                        PMIC_ISINK_GROUP_CONTROL(pmic_type, ISINK_CURRENT_CONFIG_API, NULL);                       
                }
                else if(control == ISINK_BACKLIGHT_ADJUST)
                {
                        LEDS_DETAIL_DEBUG("[LED] Backlight Adjust\n");
                        PMIC_ISINK_GROUP_CONTROL(pmic_type, ISINK_BRIGHTNESS_CONFIG_API, config_data);   
                        PMIC_ISINK_GROUP_CONTROL(pmic_type, ISINK_ENABLE_API, NULL);                            
                }
                else if(control == ISINK_BACKLIGHT_DISABLE)
                {
                        LEDS_DETAIL_DEBUG("[LED] Backlight Disable\n");
                        PMIC_ISINK_GROUP_CONTROL(pmic_type, ISINK_DISABLE_API, NULL);
                }   
      
        }
        else if(pmic_type == MT65XX_LED_PMIC_LCD_ISINK)
        {       
                for(i = ISINK0; i < ISINK_NUM; i++)
                {
                        if(g_isink_data[i]->usage == ISINK_AS_BACKLIGHT)
                        {
                                if(control == ISINK_BACKLIGHT_INIT)
                                {
                                        PMIC_ISINK_Easy_Config(i, config_data);
                                        PMIC_ISINK_Current_Config(i);                                
                                }
                                else if(control == ISINK_BACKLIGHT_ADJUST)
                                {
                                        PMIC_ISINK_Brightness_Config(i, config_data);
                                        PMIC_ISINK_Enable(i);                                
                                }
                                else if(control == ISINK_BACKLIGHT_DISABLE)
                                {
                                        PMIC_ISINK_Disable(i);                                 
                                }   
                        }
                }
        }
}

static unsigned int PMIC_ISINK_CHANNEL_LOOKUP_BY_GROUP(enum mt65xx_led_pmic pmic_type)
{
        unsigned int i;
        unsigned int start_point = 0, end_point = 0;
        unsigned int operation_channel = 0;

        if(pmic_type >= MT65XX_LED_PMIC_NLED_ISINK_GROUP_BEGIN && pmic_type < MT65XX_LED_PMIC_NLED_ISINK_GROUP_END)
        {
                start_point = MT65XX_LED_PMIC_NLED_ISINK_GROUP_BEGIN;
                end_point = MT65XX_LED_PMIC_NLED_ISINK_GROUP_END;
        }
        else if (pmic_type >= MT65XX_LED_PMIC_LCD_ISINK_GROUP_BEGIN && pmic_type < MT65XX_LED_PMIC_LCD_ISINK_GROUP_END)
        {
                start_point = MT65XX_LED_PMIC_LCD_ISINK_GROUP_BEGIN;
                end_point = MT65XX_LED_PMIC_LCD_ISINK_GROUP_END;
        }         
       
        for(i = start_point; i < end_point; i++)
        {
                if(g_isink_data[i - start_point]->data == pmic_type)
                {
                        operation_channel |= (1 << (i - start_point));
                }
        }

        return operation_channel;                                
}

static void PMIC_ISINK_GROUP_CONTROL(enum mt65xx_led_pmic pmic_type, PMIC_ISINK_CONTROL_API handler, struct PMIC_ISINK_CONFIG *config_data)
{
        unsigned int i = 0, operation_channel = 0;
        operation_channel = PMIC_ISINK_CHANNEL_LOOKUP_BY_GROUP(pmic_type);
        LEDS_DETAIL_DEBUG("[LED] ISINK Group %d Operation: %x Handler = %d\n", pmic_type, operation_channel, handler);

        for(i = 0; i < ISINK_NUM; i++)
        {
                LEDS_DETAIL_DEBUG("[LED] Channel = %d\n", operation_channel & (1 << i));
                if(operation_channel & (1 << i))
                {
                        LEDS_DETAIL_DEBUG("[LED] ISINK Channel %d is configured\n", i);
                        switch (handler) {
                                case ISINK_ENABLE_API:
                                        PMIC_ISINK_Enable(i);
                                        break;
                                case ISINK_DISABLE_API:
                                        PMIC_ISINK_Disable(i);
                                        break;
                                case ISINK_EASY_CONFIG_API:
                                        PMIC_ISINK_Easy_Config(i, config_data);
                                        break;
                                case ISINK_CURRENT_CONFIG_API:
                                        PMIC_ISINK_Current_Config(i);
                                        break;
                                case ISINK_BRIGHTNESS_CONFIG_API:
                                        PMIC_ISINK_Brightness_Config(i, config_data);
                                        break;
                                default:
                                        break;                                        
                        }
                }
        }
}

static unsigned int Value_to_RegisterIdx(u32 val, u32 *table)
{
        unsigned int *index_array = table;
        unsigned int i = 0, idx = 0, size = ISINK_STEP_SIZE;
        LEDS_DETAIL_DEBUG("[LED] Current = %d, size = %d\n", val, size);
        for(i = 0; i < size; i++)
        {
                LEDS_DETAIL_DEBUG("[LED] Current = %d\n", index_array[i]);
       		if (val == index_array[i])
       		{
   		        idx  = i;
                        break;
        	}                
                else
                {
                        idx = 0xFFFFFFFF;
                }
        } 
        return idx;
}

static unsigned int led_pmic_pwrap_read(U32 addr)
{

	U32 val =0;
	pwrap_read(addr, &val);
	return val;
	
}

static void led_register_dump(void)
{  
#define ISINK_REG_OFFSET        0x0330        
#define ISINK_REG_END           0x0356        
        unsigned int i = 0;

        LEDS_DETAIL_DEBUG("[LED] RG_DRV_32K_CK_PDN [11]: %x\n", led_pmic_pwrap_read(0x0102));        
        LEDS_DETAIL_DEBUG("[LED] RG_DRV_2M_CK_PDN [6]: %x\n", led_pmic_pwrap_read(0x0108));
        LEDS_DETAIL_DEBUG("[LED] RG_ISINK#_CK_PDN [3:0]: %x\n", led_pmic_pwrap_read(0x010E));
        LEDS_DETAIL_DEBUG("[LED] RG_ISINK#_CK_SEL [13:10]: %x\n", led_pmic_pwrap_read(0x0126));
        for(i = ISINK_REG_OFFSET; i <= ISINK_REG_END; i += 2)
        {
                LEDS_DETAIL_DEBUG("[LED] ISINK_REG %x = %x\n", i, led_pmic_pwrap_read(i));
        }        
}

static int __init mt_mt65xx_leds_init(void)
{
	int i;
	int ret=0;
	struct PMIC_CUST_ISINK *isink_config = get_cust_isink_config();

	for (i = 0; i < ISINK_NUM; i++) {
		g_isink_data[i] = kzalloc(sizeof(struct PMIC_ISINK_CONFIG), GFP_KERNEL);
		if (!g_isink_data[i]) {
			ret = -ENOMEM;
			goto err_isink;
		}                
		g_isink_data[i]->usage = isink_config[i].usage;
		g_isink_data[i]->step = isink_config[i].step;
		g_isink_data[i]->data = isink_config[i].data;
		if (ret)
			goto err_isink;                
	}
	return 0;
	
err_isink:
	if (i) {
		for (i = i-1; i >=0; i--) {
			if (!g_isink_data[i])
				continue;
			kfree(g_isink_data[i]);
			g_isink_data[i] = NULL;
		}
	}        
	return ret;
}

static void __exit mt_mt65xx_leds_exit(void)
{
	int i;
	for (i = 0; i < ISINK_NUM; i++) {
		if (!g_isink_data[i])
			continue;
		kfree(g_isink_data[i]);
		g_isink_data[i] = NULL;
	}
}

module_init(mt_mt65xx_leds_init);
module_exit(mt_mt65xx_leds_exit);
