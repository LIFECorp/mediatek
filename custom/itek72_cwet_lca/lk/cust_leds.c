//#include <platform/cust_leds.h>
#include <cust_leds.h>
//#include <asm/arch/mt6577_pwm.h>

//extern int DISP_SetBacklight(int level);

extern int disp_bls_set_backlight(unsigned int level);
static struct cust_mt65xx_led cust_led_list[MT65XX_LED_TYPE_TOTAL] = {
	{"red",               MT65XX_LED_MODE_NONE, -1, {0}},
	{"green",             MT65XX_LED_MODE_NONE, -1, {0}},
	{"blue",              MT65XX_LED_MODE_NONE, -1, {0}},
	{"jogball-backlight", MT65XX_LED_MODE_NONE, -1, {0}},
	{"keyboard-backlight",MT65XX_LED_MODE_NONE, -1, {0}},
	{"button-backlight",  MT65XX_LED_MODE_NONE, -1, {0}},
	{"lcd-backlight",     MT65XX_LED_MODE_PMIC, MT65XX_LED_PMIC_LCD_ISINK, {0}},
};

struct cust_mt65xx_led *get_cust_led_list(void)
{
	return cust_led_list;
}

