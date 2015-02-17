/******************************************************************************
 * mt65xx_leds.c
 * 
 * Copyright 2010 MediaTek Co.,Ltd.
 * 
 * DESCRIPTION:
 *
 ******************************************************************************/

//#include <common.h>
//#include <platform/mt.h>

#include <platform/mt_reg_base.h>
#include <platform/mt_typedefs.h>

// FIXME: should include power related APIs

#include <platform/mt_pwm.h>
#include <platform/mt_gpio.h>
#include <platform/mt_leds.h>
#include <platform/mt_gpt.h>
//#include <asm/io.h>

#include <platform/mt_pmic.h> 


//extern void mt_power_off (U32 pwm_no);
//extern S32 mt_set_pwm_disable ( U32 pwm_no );
extern void mt_pwm_disable(U32 pwm_no, BOOL pmic_pad);
extern int strcmp(const char *cs, const char *ct);

// Use Old Mode of PWM to suppoort 256 backlight level
#define BACKLIGHT_LEVEL_PWM_256_SUPPORT 256
extern unsigned int Cust_GetBacklightLevelSupport_byPWM(void);

/****************************************************************************
 * DEBUG MACROS
 ***************************************************************************/
static int debug_enable = 0;

#define LEDS_DEBUG(format, args...) do{ \
		if(debug_enable) \
		{\
			printf(format, ##args); \
		}\
	}while(0)

static int detail_debug_enable = 0;
#define LEDS_DETAIL_DEBUG(format, args...) do{ \
	        if(detail_debug_enable) \
        	{\
        		printf(format, ##args); \
        	}\
	}while(0)

/****************************************************************************
 * structures
 ***************************************************************************/
static int g_lastlevel[MT65XX_LED_TYPE_TOTAL] = {-1, -1, -1, -1, -1, -1, -1};
int backlight_PWM_div = CLK_DIV1;
/****************************************************************************
 * function prototypes
 ***************************************************************************/

/* import functions */
// FIXME: should extern from pmu driver
//void pmic_backlight_on(void) {}
//void pmic_backlight_off(void) {}
//void pmic_config_interface(kal_uint16 RegNum, kal_uint8 val, kal_uint16 MASK, kal_uint16 SHIFT) {}

/* internal functions */
static int brightness_set_pwm(int pwm_num, enum led_brightness level,struct PWM_config *config_data);
static int led_set_pwm(int pwm_num, enum led_brightness level);
static int brightness_set_pmic(enum mt65xx_led_pmic pmic_type, enum led_brightness level);
//static int brightness_set_gpio(int gpio_num, enum led_brightness level);
static int mt65xx_led_set_cust(struct cust_mt65xx_led *cust, int level);

extern S32 pwrap_read( U32  adr, U32 *rdata );
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

/****************************************************************************
 * global variables
 ***************************************************************************/
static unsigned int limit = 255;
u32 isink_step[] =
{
		ISINK_STEP_04MA,	ISINK_STEP_08MA,        ISINK_STEP_12MA,   ISINK_STEP_16MA,   ISINK_STEP_20MA,   ISINK_STEP_24MA,
		ISINK_STEP_32MA,        ISINK_STEP_40MA,        ISINK_STEP_48MA
};
static struct PMIC_CUST_ISINK g_isink_data[ISINK_NUM];
static unsigned int isink_indicator_en = 0;
static unsigned int isink_backlight_en = 0;

#define ISINK_STEP_SIZE         (sizeof(isink_step)/sizeof(isink_step[0]))
/****************************************************************************
 * internal functions
 ***************************************************************************/

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

unsigned int brightness_mapping(unsigned int level)
{
        unsigned int mapped_level;
        
        mapped_level = level;
        
        return mapped_level;
}

static int brightness_set_pwm(int pwm_num, enum led_brightness level,struct PWM_config *config_data)
{
	struct pwm_spec_config pwm_setting;
	unsigned int BacklightLevelSupport = Cust_GetBacklightLevelSupport_byPWM();

	pwm_setting.pwm_no = pwm_num;
	if (BacklightLevelSupport == BACKLIGHT_LEVEL_PWM_256_SUPPORT)
		pwm_setting.mode = PWM_MODE_OLD;
	else
		pwm_setting.mode = PWM_MODE_FIFO; // New mode fifo and periodical mode

	pwm_setting.pmic_pad = config_data->pmic_pad;

	if(config_data->div)
	{
		pwm_setting.clk_div = config_data->div;
		backlight_PWM_div = config_data->div;
	}
	else
	{
		pwm_setting.clk_div = CLK_DIV1;
	}

	if(BacklightLevelSupport== BACKLIGHT_LEVEL_PWM_256_SUPPORT)
	{
		if(config_data->clock_source)
		{
			pwm_setting.clk_src = PWM_CLK_OLD_MODE_BLOCK;
		}
		else
		{
			pwm_setting.clk_src = PWM_CLK_OLD_MODE_32K; // actually. it's block/1625 = 26M/1625 = 16KHz @ MT6571
		}

		pwm_setting.pwm_mode.PWM_MODE_OLD_REGS.IDLE_VALUE = 0;
		pwm_setting.pwm_mode.PWM_MODE_OLD_REGS.GUARD_VALUE = 0;
		pwm_setting.pwm_mode.PWM_MODE_OLD_REGS.GDURATION = 0;
		pwm_setting.pwm_mode.PWM_MODE_OLD_REGS.WAVE_NUM = 0;
		pwm_setting.pwm_mode.PWM_MODE_OLD_REGS.DATA_WIDTH = 255; // 256 level
		pwm_setting.pwm_mode.PWM_MODE_OLD_REGS.THRESH = level;

		printf("[LEDS][%d] LK: backlight_set_pwm:duty is %d/%d\n", BacklightLevelSupport, level, pwm_setting.pwm_mode.PWM_MODE_OLD_REGS.DATA_WIDTH);
		printf("[LEDS][%d] LK: backlight_set_pwm:clk_src/div is %d%d\n", BacklightLevelSupport, pwm_setting.clk_src, pwm_setting.clk_div);
		if(level >0 && level < 256)
		{
			pwm_set_spec_config(&pwm_setting);
			printf("[LEDS][%d] LK: backlight_set_pwm: old mode: thres/data_width is %d/%d\n", BacklightLevelSupport, pwm_setting.pwm_mode.PWM_MODE_OLD_REGS.THRESH, pwm_setting.pwm_mode.PWM_MODE_OLD_REGS.DATA_WIDTH);
		}
		else
		{
			printf("[LEDS][%d] LK: Error level in backlight\n", BacklightLevelSupport);
			mt_pwm_disable(pwm_setting.pwm_no, config_data->pmic_pad);
		}
		return 0;

	}
	else
	{
		if(config_data->clock_source)
		{
			pwm_setting.clk_src = PWM_CLK_NEW_MODE_BLOCK;
		}
		else
		{
			pwm_setting.clk_src = PWM_CLK_NEW_MODE_BLOCK_DIV_BY_1625;
		}

		if(config_data->High_duration && config_data->low_duration)
		{
			pwm_setting.pwm_mode.PWM_MODE_FIFO_REGS.HDURATION = config_data->High_duration;
			pwm_setting.pwm_mode.PWM_MODE_FIFO_REGS.LDURATION = pwm_setting.pwm_mode.PWM_MODE_FIFO_REGS.HDURATION;
		}
		else
		{
			pwm_setting.pwm_mode.PWM_MODE_FIFO_REGS.HDURATION = 4;
			pwm_setting.pwm_mode.PWM_MODE_FIFO_REGS.LDURATION = 4;
		}

		pwm_setting.pwm_mode.PWM_MODE_FIFO_REGS.IDLE_VALUE = 0;
		pwm_setting.pwm_mode.PWM_MODE_FIFO_REGS.GUARD_VALUE = 0;
		pwm_setting.pwm_mode.PWM_MODE_FIFO_REGS.STOP_BITPOS_VALUE = 31;
		pwm_setting.pwm_mode.PWM_MODE_FIFO_REGS.GDURATION = (pwm_setting.pwm_mode.PWM_MODE_FIFO_REGS.HDURATION + 1) * 32 - 1;
		pwm_setting.pwm_mode.PWM_MODE_FIFO_REGS.WAVE_NUM = 0;

		printf("[LEDS] LK: backlight_set_pwm:duty is %d\n", level);
		printf("[LEDS] LK: backlight_set_pwm:clk_src/div/high/low is %d%d%d%d\n", pwm_setting.clk_src, pwm_setting.clk_div, pwm_setting.pwm_mode.PWM_MODE_FIFO_REGS.HDURATION, pwm_setting.pwm_mode.PWM_MODE_FIFO_REGS.LDURATION);

		if(level > 0 && level <= 32)
		{
			pwm_setting.pwm_mode.PWM_MODE_FIFO_REGS.GUARD_VALUE = 0;
			pwm_setting.pwm_mode.PWM_MODE_FIFO_REGS.SEND_DATA0 = (1 << level) - 1;
			pwm_set_spec_config(&pwm_setting);
		}else if(level > 32 && level <= 64)
		{
			pwm_setting.pwm_mode.PWM_MODE_FIFO_REGS.GUARD_VALUE = 1;
			level -= 32;
			pwm_setting.pwm_mode.PWM_MODE_FIFO_REGS.SEND_DATA0 = (1 << level) - 1 ;
			pwm_set_spec_config(&pwm_setting);
		}else
		{
			printf("[LEDS] LK: Error level in backlight\n");
			mt_pwm_disable(pwm_setting.pwm_no, config_data->pmic_pad);
		}

		return 0;
	}
}

static int led_set_pwm(int pwm_num, enum led_brightness level)
{
	struct pwm_spec_config pwm_setting;
	pwm_setting.pwm_no = pwm_num;
	pwm_setting.clk_div = CLK_DIV1; 		

	pwm_setting.pwm_mode.PWM_MODE_OLD_REGS.DATA_WIDTH = 50;
    
	// In 6571, we won't choose 32K to be the clock src of old mode because of system performance.
	// The setting here will be clock src = 26MHz, CLKSEL = 26M/1625 (i.e. 16K)
	pwm_setting.clk_src = PWM_CLK_OLD_MODE_32K;
    
	if(level)
	{
		pwm_setting.pwm_mode.PWM_MODE_OLD_REGS.THRESH = 15;
	}else
	{
		pwm_setting.pwm_mode.PWM_MODE_OLD_REGS.THRESH = 0;
	}
	printf("[LEDS] LK: brightness_set_pwm: level=%d, clk=%d \n\r", level, pwm_setting.clk_src);

	pwm_set_spec_config(&pwm_setting);

	return 0;
}

static int brightness_set_pmic(enum mt65xx_led_pmic pmic_type, enum led_brightness level)
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

	printf("[LEDS] LK: PMIC Type: %d, Level: %d\n", pmic_type, level);

        if (pmic_type == MT65XX_LED_PMIC_LCD_ISINK || \
            pmic_type == MT65XX_LED_PMIC_LCD_ISINK_GROUP0 || pmic_type == MT65XX_LED_PMIC_LCD_ISINK_GROUP1 || \
            pmic_type == MT65XX_LED_PMIC_LCD_ISINK_GROUP2 || pmic_type == MT65XX_LED_PMIC_LCD_ISINK_GROUP3)
	{
                        LEDS_DETAIL_DEBUG("[LEDS] LK: Backlight Configuration");
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
			if(level == ERROR_BL_LEVEL)
				level = limit;
#if 0            
                        if(((level << 5) / limit) < 1)
                        {
                        	level = 0;
                        }
                        else
                        {
                        	level = ((level << 5) / limit) - 1;
                        }
#endif      
#ifdef CONTROL_BL_TEMPERATURE
                        if(level >= limit && limit_flag != 0)
                        {
                                level = limit; // Backlight cooler will limit max level
                        }
#endif
                        if(level == 255)
                        {
                                level = PMIC_ISINK_BACKLIGHT_LEVEL;
                        }
                        else
                        {
                                level = ((level * PMIC_ISINK_BACKLIGHT_LEVEL) / 255) + 1;
			}
                        LEDS_DEBUG("[LEDS] LK:Level Mapping = %d \n", level);
                        LEDS_DEBUG("[LEDS] LK:ISINK DIM Duty = %d \n", duty_mapping[level - 1]);
                        LEDS_DEBUG("[LEDS] LK:ISINK Current = %d \n", current_mapping[level - 1]);
                        isink_config.duty = duty_mapping[level - 1];
                        isink_config.step = current_mapping[level - 1];
                        LEDS_DEBUG("[LEDS] LK:isink_config.duty = %d \n", isink_config.duty);
                        LEDS_DEBUG("[LEDS] LK:isink_config.step\n", isink_config.step);                        
                        PMIC_Backlight_Control(pmic_type, ISINK_BACKLIGHT_ADJUST, &isink_config);
		}
		else 
		{
                        PMIC_Backlight_Control(pmic_type, ISINK_BACKLIGHT_DISABLE, &isink_config);
		}
        
		return 0;
	}
	else if(pmic_type == MT65XX_LED_PMIC_NLED_ISINK0 || pmic_type == MT65XX_LED_PMIC_NLED_ISINK1 || \
                pmic_type == MT65XX_LED_PMIC_NLED_ISINK2 || pmic_type == MT65XX_LED_PMIC_NLED_ISINK3 || \
                pmic_type == MT65XX_LED_PMIC_NLED_ISINK_GROUP0 || pmic_type == MT65XX_LED_PMIC_NLED_ISINK_GROUP1 || \
                pmic_type == MT65XX_LED_PMIC_NLED_ISINK_GROUP2 || pmic_type == MT65XX_LED_PMIC_NLED_ISINK_GROUP3)
	{
                LEDS_DETAIL_DEBUG("[LEDS] LK: LED Configuration");
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

static void PMIC_ISINK_Enable(PMIC_ISINK_CHANNEL channel)
{
        LEDS_DEBUG("[LEDS] LK: %s->ISINK%d: Enable\n", __func__, channel);

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
        
        if(g_isink_data[channel].usage == ISINK_AS_INDICATOR)
        {
                isink_indicator_en |= (1 << channel);
                upmu_set_rg_drv_32k_ck_pdn(0x0); // Disable power down                    
        }
        else if(g_isink_data[channel].usage == ISINK_AS_BACKLIGHT)
        {
                isink_backlight_en |= (1 << channel);
                upmu_set_rg_drv_2m_ck_pdn(0x0); // Disable power down
        }
        LEDS_DETAIL_DEBUG("[LEDS] LK: ISINK Usage: Indicator->%x, Backlight->%x\n", isink_indicator_en, isink_backlight_en);        
}

static void PMIC_ISINK_Disable(PMIC_ISINK_CHANNEL channel)
{
        LEDS_DETAIL_DEBUG("[LEDS] LK: %s->ISINK%d: Disable\n", __func__, channel);
        
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
 
        if(g_isink_data[channel].usage == ISINK_AS_INDICATOR)
        {
                isink_indicator_en &= ~(1 << channel);
        }
        else if(g_isink_data[channel].usage == ISINK_AS_BACKLIGHT)
        {
                isink_backlight_en &= ~(1 << channel);
        }
        LEDS_DETAIL_DEBUG("[LEDS] LK: ISINK Usage: Indicator->%x, Backlight->%x\n", isink_indicator_en, isink_backlight_en);             
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
        PMIC_ISINK_USAGE usage = g_isink_data[channel].usage;
        LEDS_DETAIL_DEBUG("[LEDS] LK: PMIC_ISINK_Easy_Config->ISINK%d: usage = %d\n", channel, usage);
        LEDS_DETAIL_DEBUG("[LEDS] LK: config_data->mode %d\n", config_data->mode);
        LEDS_DETAIL_DEBUG("[LEDS] LK: config_data->duty %d\n", config_data->duty);
        LEDS_DETAIL_DEBUG("[LEDS] LK: config_data->dim_fsel %d\n", config_data->dim_fsel);
        LEDS_DETAIL_DEBUG("[LEDS] LK: config_data->sfstr_tc %d\n", config_data->sfstr_tc);
        LEDS_DETAIL_DEBUG("[LEDS] LK: config_data->sfstr_en %d\n", config_data->sfstr_en);
        LEDS_DETAIL_DEBUG("[LEDS] LK: config_data->breath_trf_sel %d\n", config_data->breath_trf_sel);
        LEDS_DETAIL_DEBUG("[LEDS] LK: config_data->breath_ton_sel %d\n", config_data->breath_ton_sel); 
        LEDS_DETAIL_DEBUG("[LEDS] LK: config_data->breath_toff_sel %d\n", config_data->breath_toff_sel);        
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
        PMIC_ISINK_STEP step = g_isink_data[channel].step;
        LEDS_DEBUG("[LEDS] LK: %s\n", __func__);
        if(step == ISINK_STEP_FOR_BACK_LIGHT && g_isink_data[channel].usage == ISINK_AS_BACKLIGHT)
        {
                LEDS_DETAIL_DEBUG("[LEDS] LK: ISINK_STEP_48MA\n");
                step = ISINK_STEP_48MA;
        }
        if(step >  ISINK_STEP_24MA)
        {
                LEDS_DETAIL_DEBUG("[LEDS] LK: Enable Double Bit\n");
                isink_current = step >> 1;
                double_en = 1;
        }
        idx = Value_to_RegisterIdx(isink_current, isink_step);
        if(idx == 0xFFFFFFFF)
        {
                LEDS_DETAIL_DEBUG("[LEDS] LK: ISINK%d Current not available\n", channel);
        }
        LEDS_DETAIL_DEBUG("[LEDS] LK: ISINK%d: step = %d, double_en = %d\n", channel, idx, double_en);
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
        LEDS_DETAIL_DEBUG("[LEDS] LK: %s, duty = %d, step = %d\n", __func__, config_data->duty, config_data->step);
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

        LEDS_DEBUG("[LEDS] LK: %s\n", __func__);        
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
        LEDS_DEBUG("[LEDS] LK: %s\n", __func__);
        if(pmic_type >= MT65XX_LED_PMIC_LCD_ISINK_GROUP_BEGIN && pmic_type < MT65XX_LED_PMIC_LCD_ISINK_GROUP_END)
        {
                if(control == ISINK_BACKLIGHT_INIT)
                {
                        LEDS_DETAIL_DEBUG("[LEDS] LK: Backlight Init\n");
                        PMIC_ISINK_GROUP_CONTROL(pmic_type, ISINK_EASY_CONFIG_API,  config_data);
                        PMIC_ISINK_GROUP_CONTROL(pmic_type, ISINK_CURRENT_CONFIG_API, NULL);                       
                }
                else if(control == ISINK_BACKLIGHT_ADJUST)
                {
                        LEDS_DETAIL_DEBUG("[LEDS] LK: Backlight Adjust\n");
                        PMIC_ISINK_GROUP_CONTROL(pmic_type, ISINK_BRIGHTNESS_CONFIG_API, config_data);   
                        PMIC_ISINK_GROUP_CONTROL(pmic_type, ISINK_ENABLE_API, NULL);                            
                }
                else if(control == ISINK_BACKLIGHT_DISABLE)
                {
                        LEDS_DETAIL_DEBUG("[LEDS] LK: Backlight Disable\n");
                        PMIC_ISINK_GROUP_CONTROL(pmic_type, ISINK_DISABLE_API, NULL);
                }   
      
        }
        else if(pmic_type == MT65XX_LED_PMIC_LCD_ISINK)
        {       
                for(i = ISINK0; i < ISINK_NUM; i++)
                {
                        if(g_isink_data[i].usage == ISINK_AS_BACKLIGHT)
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
                if(g_isink_data[i - start_point].data == (int)pmic_type)
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
        LEDS_DETAIL_DEBUG("[LEDS] LK: ISINK Group %d Operation: %x Handler = %d\n", pmic_type, operation_channel, handler);

        for(i = 0; i < ISINK_NUM; i++)
        {
                LEDS_DETAIL_DEBUG("[LEDS] LK: Channel = %d\n", operation_channel & (1 << i));
                if(operation_channel & (1 << i))
                {
                        LEDS_DETAIL_DEBUG("[LEDS] LK: ISINK Channel %d is configured\n", i);
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
        LEDS_DETAIL_DEBUG("[LEDS] LK: Current = %d, size = %d\n", val, size);
        for(i = 0; i < size; i++)
        {
                LEDS_DETAIL_DEBUG("[LEDS] LK: Current = %d\n", index_array[i]);
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

        LEDS_DETAIL_DEBUG("[LEDS] LK: RG_DRV_32K_CK_PDN [11]: %x\n", led_pmic_pwrap_read(0x0102));        
        LEDS_DETAIL_DEBUG("[LEDS] LK: RG_DRV_2M_CK_PDN [6]: %x\n", led_pmic_pwrap_read(0x0108));
        LEDS_DETAIL_DEBUG("[LEDS] LK: RG_ISINK#_CK_PDN [3:0]: %x\n", led_pmic_pwrap_read(0x010E));
        LEDS_DETAIL_DEBUG("[LEDS] LK: RG_ISINK#_CK_SEL [13:10]: %x\n", led_pmic_pwrap_read(0x0126));
        for(i = ISINK_REG_OFFSET; i <= ISINK_REG_END; i += 2)
        {
                LEDS_DETAIL_DEBUG("[LEDS] LK: ISINK_REG %x = %x\n", i, led_pmic_pwrap_read(i));
        }        
}

#if 0
static int brightness_set_gpio(int gpio_num, enum led_brightness level)
{
//	LEDS_INFO("LED GPIO#%d:%d\n", gpio_num, level);
	mt_set_gpio_mode(gpio_num, GPIO_MODE_00);// GPIO MODE
	mt_set_gpio_dir(gpio_num, GPIO_DIR_OUT);

	if (level)
		mt_set_gpio_out(gpio_num, GPIO_OUT_ONE);
	else
		mt_set_gpio_out(gpio_num, GPIO_OUT_ZERO);

	return 0;
}
#endif

static int mt65xx_led_set_cust(struct cust_mt65xx_led *cust, int level)
{
	unsigned int BacklightLevelSupport = Cust_GetBacklightLevelSupport_byPWM();

	if (level > LED_FULL)
		level = LED_FULL;
	else if (level < 0)
		level = 0;

        LEDS_DEBUG("[LEDS] LK: %s\n", __func__);
        led_register_dump();
	switch (cust->mode) {
		
		case MT65XX_LED_MODE_PWM:
			if(level == 0)
			{
				//printf("[LEDS]LK: mt65xx_leds_set_cust: enter mt_pwm_disable()\n");
				mt_pwm_disable(cust->data, cust->config_data.pmic_pad);
				return 1;
			}
			if(strcmp(cust->name, "lcd-backlight") == 0)
			{
				if (BacklightLevelSupport == BACKLIGHT_LEVEL_PWM_256_SUPPORT)
					level = brightness_mapping(level);
				else
					level = brightness_mapto64(level);						
					//printf("[LEDS]LK: mt65xx_led_set_cust: mode=%d, level=%d \n\r", cust->mode, level);
			    return brightness_set_pwm(cust->data, level, &cust->config_data);
			}
			else
			{
				return led_set_pwm(cust->data, level);
			}
		
		case MT65XX_LED_MODE_GPIO:
			return ((cust_brightness_set)(cust->data))(level);
		case MT65XX_LED_MODE_PMIC:
			return brightness_set_pmic(cust->data, level);
		case MT65XX_LED_MODE_CUST_LCM:
			return ((cust_brightness_set)(cust->data))(level);
		case MT65XX_LED_MODE_CUST_BLS_PWM:
			return ((cust_brightness_set)(cust->data))(level);
		case MT65XX_LED_MODE_NONE:
		default:
			break;
	}
	return -1;
}

/****************************************************************************
 * external functions
 ***************************************************************************/
int mt65xx_leds_brightness_set(enum mt65xx_led_type type, enum led_brightness level)
{
	struct cust_mt65xx_led *cust_led_list = get_cust_led_list();
        
	if (type >= MT65XX_LED_TYPE_TOTAL)
		return -1;

	if (level > LED_FULL)
		level = LED_FULL;
//	else if (level < 0)  //level cannot < 0
//		level = 0;

	if (g_lastlevel[type] != (int)level) {
		g_lastlevel[type] = level;
		printf("[LEDS] LK: %s level is %d \n\r", cust_led_list[type].name, level);
		return mt65xx_led_set_cust(&cust_led_list[type], level);
	}
	else {
		return -1;
	}

}

void leds_battery_full_charging(void)
{
	mt65xx_leds_brightness_set(MT65XX_LED_TYPE_RED, LED_OFF);
	mt65xx_leds_brightness_set(MT65XX_LED_TYPE_GREEN, LED_FULL);
	mt65xx_leds_brightness_set(MT65XX_LED_TYPE_BLUE, LED_OFF);
}

void leds_battery_low_charging(void)
{
	mt65xx_leds_brightness_set(MT65XX_LED_TYPE_RED, LED_FULL);
	mt65xx_leds_brightness_set(MT65XX_LED_TYPE_GREEN, LED_OFF);
	mt65xx_leds_brightness_set(MT65XX_LED_TYPE_BLUE, LED_OFF);
}

void leds_battery_medium_charging(void)
{
	mt65xx_leds_brightness_set(MT65XX_LED_TYPE_RED, LED_FULL);
	mt65xx_leds_brightness_set(MT65XX_LED_TYPE_GREEN, LED_OFF);
	mt65xx_leds_brightness_set(MT65XX_LED_TYPE_BLUE, LED_OFF);
}

void leds_init(void)
{
     	int i;
        struct PMIC_CUST_ISINK *isink_config = get_cust_isink_config();
        
        printf("[LEDS] LK: leds_init: mt65xx_backlight_off \n\r");
        for (i = 0; i < ISINK_NUM; i++) {
                g_isink_data[i].usage = isink_config[i].usage;
                g_isink_data[i].step = isink_config[i].step;
                g_isink_data[i].data = isink_config[i].data;
 	LEDS_DETAIL_DEBUG("[LEDS] LK: usage = %d, step =%d, data = %d\n",  g_isink_data[i].usage, g_isink_data[i].step, g_isink_data[i].data );        
        }        
	mt65xx_backlight_off();
}

void isink0_init(void)
{
    /*
    printf("[LEDS]LK: isink_init: turn on PMIC6323 isink \n\r");
    upmu_set_rg_drv_2m_ck_pdn(0x0); // Disable power down
    upmu_set_rg_drv_32k_ck_pdn(0x0); // Disable power down (backlight no need?)    

    // For backlight: Current: 24mA, PWM frequency: 20K, Duty: 20~100, Soft start: off, Phase shift: on
    // ISINK0
    upmu_set_rg_isink0_ck_pdn(0x0); // Disable power down    
    upmu_set_rg_isink0_ck_sel(0x1); // Freq = 1Mhz for Backlight
	upmu_set_isink_ch0_mode(ISINK_REGISTER_MODE);
    upmu_set_isink_ch0_step(0x5); // 24mA
    upmu_set_isink_sfstr0_en(0x0); // Disable soft start
	upmu_set_rg_isink0_double_en(0x0); // Disable double current
	upmu_set_isink_phase0_dly_en(0x1); // Enable phase delay
    upmu_set_isink_chop0_en(0x0); // Disable CHOP clk          
    upmu_set_isink_ch0_en(0x1); // Turn on ISINK Channel 0
    */
}

void leds_deinit(void)
{
    printf("[LEDS]LK: leds_deinit: LEDS off \n\r");
	mt65xx_leds_brightness_set(MT65XX_LED_TYPE_RED, LED_OFF);
	mt65xx_leds_brightness_set(MT65XX_LED_TYPE_GREEN, LED_OFF);
	mt65xx_leds_brightness_set(MT65XX_LED_TYPE_BLUE, LED_OFF);
}

void mt65xx_backlight_on(void)
{      
	printf("[LEDS]LK: mt65xx_backlight_on \n\r");
	mt65xx_leds_brightness_set(MT65XX_LED_TYPE_LCD, LED_FULL);
}

void mt65xx_backlight_off(void)
{
	printf("[LEDS] LK: mt65xx_backlight_off \n\r");
	mt65xx_leds_brightness_set(MT65XX_LED_TYPE_LCD, LED_OFF);
}

