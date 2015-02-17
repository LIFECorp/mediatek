#ifndef __MT_LEDS_H__
#define __MT_LEDS_H__

#define ISINK_CHOP_CLK
//#include <platform/cust_leds.h>
#include <cust_leds.h>

#define ERROR_BL_LEVEL 0xFFFFFFFF

enum led_color {
	LED_RED,
	LED_GREEN,
	LED_BLUE,
};

enum led_brightness {
	LED_OFF		= 0,
	LED_HALF	= 127,
	LED_FULL	= 255,
};

#define GET_ARR_SIZE(array) (sizeof(array) / sizeof(array[0]))

typedef enum{
        ISINK0 = 0,
        ISINK1 = 1,
        ISINK2 = 2,
        ISINK3 = 3,
        ISINK_NUM
}PMIC_ISINK_CHANNEL;

typedef enum{
        ISINK_STEP_04MA = 4,
        ISINK_STEP_08MA = 8,
        ISINK_STEP_12MA = 12,
        ISINK_STEP_16MA = 16,
        ISINK_STEP_20MA = 20,
        ISINK_STEP_24MA = 24,
        ISINK_STEP_32MA = 32,
        ISINK_STEP_40MA = 40,
        ISINK_STEP_48MA = 48,
        ISINK_STEP_FOR_BACK_LIGHT = 0xFFFF
}PMIC_ISINK_STEP;

typedef enum{  
    ISINK_PWM_MODE = 0,  
    ISINK_BREATH_MODE = 1,  
    ISINK_REGISTER_MODE = 2
}PMIC_ISINK_MODE;

typedef enum{
        ISINK_AS_BACKLIGHT = 0,
        ISINK_AS_INDICATOR = 1,
        ISINK_AS_NONE      = 0xFFFF
}PMIC_ISINK_USAGE;

struct PMIC_ISINK_CONFIG
{
        PMIC_ISINK_MODE mode;
        unsigned int duty;
        unsigned int dim_fsel;
        unsigned int step;
        unsigned int sfstr_tc;
        unsigned int sfstr_en;
        unsigned int breath_trf_sel;
        unsigned int breath_ton_sel;
        unsigned int breath_toff_sel;
};

struct PMIC_CUST_ISINK{
        PMIC_ISINK_USAGE        usage;
        PMIC_ISINK_STEP         step;
        int                     data;
};

typedef enum{
        ISINK_ENABLE_API = 0,
        ISINK_DISABLE_API = 1,
        ISINK_EASY_CONFIG_API = 2,
        ISINK_CURRENT_CONFIG_API = 3,
        ISINK_BRIGHTNESS_CONFIG_API = 4,
        ISINK_CONTROL_API_NUM
}PMIC_ISINK_CONTROL_API;

typedef enum{
        ISINK_BACKLIGHT_INIT = 0,
        ISINK_BACKLIGHT_ADJUST = 1,
        ISINK_BACKLIGHT_DISABLE
}PMIC_ISINK_BACKLIGHT_CONTROL;


extern struct PMIC_CUST_ISINK *get_cust_isink_config(void);

int mt65xx_leds_brightness_set(enum mt65xx_led_type type, enum led_brightness level);
void leds_battery_full_charging(void);
void leds_battery_low_charging(void);
void leds_battery_medium_charging(void);
void mt65xx_backlight_on(void);
void mt65xx_backlight_off(void);
void leds_init(void);
void leds_deinit(void);

#endif // __MT_LEDS_H__
