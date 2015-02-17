#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/types.h>
#include <linux/xlog.h>
#include <linux/mutex.h>
#include <mach/mt_clkmgr.h>

#include "ddp_drv.h"
#include "ddp_reg.h"
#include "ddp_debug.h"
#include "ddp_bls.h"
#include "disp_drv.h"

#include <cust_leds.h>
#include <cust_leds_def.h>

#define DDP_GAMMA_SUPPORT

#define POLLING_TIME_OUT 1000

#define PWM_DEFAULT_DIV_VALUE 0x0

unsigned int bls_dbg_log = 0;
#define BLS_DBG(string, args...) if(bls_dbg_log) printk("[BLS]"string,##args)  // default off, use "adb shell "echo dbg_log:1 > sys/kernel/debug/dispsys" to enable
#define BLS_MSG(string, args...) printk("[BLS]"string,##args)  // default on, important msg, not err
#define BLS_ERR(string, args...) printk("[BLS]error:"string,##args)  //default on, err msg

#if !defined(MTK_AAL_SUPPORT)
#ifdef USE_DISP_BLS_MUTEX
static int gBLSMutexID = 3;
#endif
static int gBLSPowerOn = 0;
#endif
static int gMaxLevel = 1023;
static int gPWMDiv = PWM_DEFAULT_DIV_VALUE;
extern int g_NrScreenPixels;

static DEFINE_MUTEX(backlight_mutex);

static DISPLAY_PWM_T g_pwm_lut;
static DISPLAY_GAMMA_T g_gamma_lut;
static DISPLAY_GAMMA_T g_gamma_index = 
{
entry:
{
    {
          0,    2,    4,    6,    8,   10,   12,   14,   16,   18,   20,   22,   24,   26,   28,   30,
         32,   34,   36,   38,   40,   42,   44,   46,   48,   50,   52,   54,   56,   58,   60,   62,
         64,   66,   68,   70,   72,   74,   76,   78,   80,   82,   84,   86,   88,   90,   92,   94,
         96,   98,  100,  102,  104,  106,  108,  110,  112,  114,  116,  118,  120,  122,  124,  126,
        128,  130,  132,  134,  136,  138,  140,  142,  144,  146,  148,  150,  152,  154,  156,  158,
        160,  162,  164,  166,  168,  170,  172,  174,  176,  178,  180,  182,  184,  186,  188,  190,
        192,  194,  196,  198,  200,  202,  204,  206,  208,  210,  212,  214,  216,  218,  220,  222,
        224,  226,  228,  230,  232,  234,  236,  238,  240,  242,  244,  246,  248,  250,  252,  254,
        256,  258,  260,  262,  264,  266,  268,  270,  272,  274,  276,  278,  280,  282,  284,  286,
        288,  290,  292,  294,  296,  298,  300,  302,  304,  306,  308,  310,  312,  314,  316,  318,
        320,  322,  324,  326,  328,  330,  332,  334,  336,  338,  340,  342,  344,  346,  348,  350,
        352,  354,  356,  358,  360,  362,  364,  366,  368,  370,  372,  374,  376,  378,  380,  382,
        384,  386,  388,  390,  392,  394,  396,  398,  400,  402,  404,  406,  408,  410,  412,  414,
        416,  418,  420,  422,  424,  426,  428,  430,  432,  434,  436,  438,  440,  442,  444,  446,
        448,  450,  452,  454,  456,  458,  460,  462,  464,  466,  468,  470,  472,  474,  476,  478,
        480,  482,  484,  486,  488,  490,  492,  494,  496,  498,  500,  502,  504,  506,  508,  510,
        512,  514,  516,  518,  520,  522,  524,  526,  528,  530,  532,  534,  536,  538,  540,  542,
        544,  546,  548,  550,  552,  554,  556,  558,  560,  562,  564,  566,  568,  570,  572,  574,
        576,  578,  580,  582,  584,  586,  588,  590,  592,  594,  596,  598,  600,  602,  604,  606,
        608,  610,  612,  614,  616,  618,  620,  622,  624,  626,  628,  630,  632,  634,  636,  638,
        640,  642,  644,  646,  648,  650,  652,  654,  656,  658,  660,  662,  664,  666,  668,  670,
        672,  674,  676,  678,  680,  682,  684,  686,  688,  690,  692,  694,  696,  698,  700,  702,
        704,  706,  708,  710,  712,  714,  716,  718,  720,  722,  724,  726,  728,  730,  732,  734,
        736,  738,  740,  742,  744,  746,  748,  750,  752,  754,  756,  758,  760,  762,  764,  766,
        768,  770,  772,  774,  776,  778,  780,  782,  784,  786,  788,  790,  792,  794,  796,  798,
        800,  802,  804,  806,  808,  810,  812,  814,  816,  818,  820,  822,  824,  826,  828,  830,
        832,  834,  836,  838,  840,  842,  844,  846,  848,  850,  852,  854,  856,  858,  860,  862,
        864,  866,  868,  870,  872,  874,  876,  878,  880,  882,  884,  886,  888,  890,  892,  894,
        896,  898,  900,  902,  904,  906,  908,  910,  912,  914,  916,  918,  920,  922,  924,  926,
        928,  930,  932,  934,  936,  938,  940,  942,  944,  946,  948,  950,  952,  954,  956,  958,
        960,  962,  964,  966,  968,  970,  972,  974,  976,  978,  980,  982,  984,  986,  988,  990,
        992,  994,  996,  998, 1000, 1002, 1004, 1006, 1008, 1010, 1012, 1014, 1016, 1018, 1020, 1022
    },
    {
          0,    2,    4,    6,    8,   10,   12,   14,   16,   18,   20,   22,   24,   26,   28,   30,
         32,   34,   36,   38,   40,   42,   44,   46,   48,   50,   52,   54,   56,   58,   60,   62,
         64,   66,   68,   70,   72,   74,   76,   78,   80,   82,   84,   86,   88,   90,   92,   94,
         96,   98,  100,  102,  104,  106,  108,  110,  112,  114,  116,  118,  120,  122,  124,  126,
        128,  130,  132,  134,  136,  138,  140,  142,  144,  146,  148,  150,  152,  154,  156,  158,
        160,  162,  164,  166,  168,  170,  172,  174,  176,  178,  180,  182,  184,  186,  188,  190,
        192,  194,  196,  198,  200,  202,  204,  206,  208,  210,  212,  214,  216,  218,  220,  222,
        224,  226,  228,  230,  232,  234,  236,  238,  240,  242,  244,  246,  248,  250,  252,  254,
        256,  258,  260,  262,  264,  266,  268,  270,  272,  274,  276,  278,  280,  282,  284,  286,
        288,  290,  292,  294,  296,  298,  300,  302,  304,  306,  308,  310,  312,  314,  316,  318,
        320,  322,  324,  326,  328,  330,  332,  334,  336,  338,  340,  342,  344,  346,  348,  350,
        352,  354,  356,  358,  360,  362,  364,  366,  368,  370,  372,  374,  376,  378,  380,  382,
        384,  386,  388,  390,  392,  394,  396,  398,  400,  402,  404,  406,  408,  410,  412,  414,
        416,  418,  420,  422,  424,  426,  428,  430,  432,  434,  436,  438,  440,  442,  444,  446,
        448,  450,  452,  454,  456,  458,  460,  462,  464,  466,  468,  470,  472,  474,  476,  478,
        480,  482,  484,  486,  488,  490,  492,  494,  496,  498,  500,  502,  504,  506,  508,  510,
        512,  514,  516,  518,  520,  522,  524,  526,  528,  530,  532,  534,  536,  538,  540,  542,
        544,  546,  548,  550,  552,  554,  556,  558,  560,  562,  564,  566,  568,  570,  572,  574,
        576,  578,  580,  582,  584,  586,  588,  590,  592,  594,  596,  598,  600,  602,  604,  606,
        608,  610,  612,  614,  616,  618,  620,  622,  624,  626,  628,  630,  632,  634,  636,  638,
        640,  642,  644,  646,  648,  650,  652,  654,  656,  658,  660,  662,  664,  666,  668,  670,
        672,  674,  676,  678,  680,  682,  684,  686,  688,  690,  692,  694,  696,  698,  700,  702,
        704,  706,  708,  710,  712,  714,  716,  718,  720,  722,  724,  726,  728,  730,  732,  734,
        736,  738,  740,  742,  744,  746,  748,  750,  752,  754,  756,  758,  760,  762,  764,  766,
        768,  770,  772,  774,  776,  778,  780,  782,  784,  786,  788,  790,  792,  794,  796,  798,
        800,  802,  804,  806,  808,  810,  812,  814,  816,  818,  820,  822,  824,  826,  828,  830,
        832,  834,  836,  838,  840,  842,  844,  846,  848,  850,  852,  854,  856,  858,  860,  862,
        864,  866,  868,  870,  872,  874,  876,  878,  880,  882,  884,  886,  888,  890,  892,  894,
        896,  898,  900,  902,  904,  906,  908,  910,  912,  914,  916,  918,  920,  922,  924,  926,
        928,  930,  932,  934,  936,  938,  940,  942,  944,  946,  948,  950,  952,  954,  956,  958,
        960,  962,  964,  966,  968,  970,  972,  974,  976,  978,  980,  982,  984,  986,  988,  990,
        992,  994,  996,  998, 1000, 1002, 1004, 1006, 1008, 1010, 1012, 1014, 1016, 1018, 1020, 1022
    },
    {
          0,    2,    4,    6,    8,   10,   12,   14,   16,   18,   20,   22,   24,   26,   28,   30,
         32,   34,   36,   38,   40,   42,   44,   46,   48,   50,   52,   54,   56,   58,   60,   62,
         64,   66,   68,   70,   72,   74,   76,   78,   80,   82,   84,   86,   88,   90,   92,   94,
         96,   98,  100,  102,  104,  106,  108,  110,  112,  114,  116,  118,  120,  122,  124,  126,
        128,  130,  132,  134,  136,  138,  140,  142,  144,  146,  148,  150,  152,  154,  156,  158,
        160,  162,  164,  166,  168,  170,  172,  174,  176,  178,  180,  182,  184,  186,  188,  190,
        192,  194,  196,  198,  200,  202,  204,  206,  208,  210,  212,  214,  216,  218,  220,  222,
        224,  226,  228,  230,  232,  234,  236,  238,  240,  242,  244,  246,  248,  250,  252,  254,
        256,  258,  260,  262,  264,  266,  268,  270,  272,  274,  276,  278,  280,  282,  284,  286,
        288,  290,  292,  294,  296,  298,  300,  302,  304,  306,  308,  310,  312,  314,  316,  318,
        320,  322,  324,  326,  328,  330,  332,  334,  336,  338,  340,  342,  344,  346,  348,  350,
        352,  354,  356,  358,  360,  362,  364,  366,  368,  370,  372,  374,  376,  378,  380,  382,
        384,  386,  388,  390,  392,  394,  396,  398,  400,  402,  404,  406,  408,  410,  412,  414,
        416,  418,  420,  422,  424,  426,  428,  430,  432,  434,  436,  438,  440,  442,  444,  446,
        448,  450,  452,  454,  456,  458,  460,  462,  464,  466,  468,  470,  472,  474,  476,  478,
        480,  482,  484,  486,  488,  490,  492,  494,  496,  498,  500,  502,  504,  506,  508,  510,
        512,  514,  516,  518,  520,  522,  524,  526,  528,  530,  532,  534,  536,  538,  540,  542,
        544,  546,  548,  550,  552,  554,  556,  558,  560,  562,  564,  566,  568,  570,  572,  574,
        576,  578,  580,  582,  584,  586,  588,  590,  592,  594,  596,  598,  600,  602,  604,  606,
        608,  610,  612,  614,  616,  618,  620,  622,  624,  626,  628,  630,  632,  634,  636,  638,
        640,  642,  644,  646,  648,  650,  652,  654,  656,  658,  660,  662,  664,  666,  668,  670,
        672,  674,  676,  678,  680,  682,  684,  686,  688,  690,  692,  694,  696,  698,  700,  702,
        704,  706,  708,  710,  712,  714,  716,  718,  720,  722,  724,  726,  728,  730,  732,  734,
        736,  738,  740,  742,  744,  746,  748,  750,  752,  754,  756,  758,  760,  762,  764,  766,
        768,  770,  772,  774,  776,  778,  780,  782,  784,  786,  788,  790,  792,  794,  796,  798,
        800,  802,  804,  806,  808,  810,  812,  814,  816,  818,  820,  822,  824,  826,  828,  830,
        832,  834,  836,  838,  840,  842,  844,  846,  848,  850,  852,  854,  856,  858,  860,  862,
        864,  866,  868,  870,  872,  874,  876,  878,  880,  882,  884,  886,  888,  890,  892,  894,
        896,  898,  900,  902,  904,  906,  908,  910,  912,  914,  916,  918,  920,  922,  924,  926,
        928,  930,  932,  934,  936,  938,  940,  942,  944,  946,  948,  950,  952,  954,  956,  958,
        960,  962,  964,  966,  968,  970,  972,  974,  976,  978,  980,  982,  984,  986,  988,  990,
        992,  994,  996,  998, 1000, 1002, 1004, 1006, 1008, 1010, 1012, 1014, 1016, 1018, 1020, 1022
    }
}
};

DISPLAY_GAMMA_T * get_gamma_index(void)
{
    BLS_DBG("get_gamma_index!\n");
    return &g_gamma_index;
}

DISPLAY_PWM_T * get_pwm_lut(void)
{
    BLS_DBG("get_pwm_lut!\n");
    return &g_pwm_lut;
}

static unsigned int brightness_mapping(unsigned int level);
void disp_onConfig_bls(DISP_AAL_PARAM *param)
{
    unsigned long prevSetting = DISP_REG_GET(DISP_REG_BLS_BLS_SETTING);
    unsigned int level = brightness_mapping(param->pwmDuty);

    if (level > 1023)
        level = 1023;

    BLS_DBG("disp_onConfig_bls!\n");

    BLS_MSG("pwm duty = %lu\n", param->pwmDuty);
    if (level == 0)
    {
        DISP_REG_SET(DISP_REG_PWM_CON_1, 0x3FF);
        if (DISP_REG_GET(DISP_REG_PWM_EN))
            DISP_REG_SET(DISP_REG_PWM_EN, 0);
    }
    else
    {
        DISP_REG_SET(DISP_REG_PWM_CON_1, (level << 16) | 0x3FF);
        if (!DISP_REG_GET(DISP_REG_PWM_EN))
            DISP_REG_SET(DISP_REG_PWM_EN, 1);
    }


    BLS_DBG("bls setting = %lu\n", param->setting);

    if (param->setting & ENUM_FUNC_BLS)
    {
        BLS_DBG("distion threshold = %lu\n", param->maxClrDistThd);
        BLS_DBG("predistion threshold = %lu\n", param->preDistThd);
        // TODO: BLS porting
    }

    if (prevSetting & 0x10100) 
    {
        // TODO: BLS porting
    }
    else if (param->setting & ENUM_FUNC_BLS)
    {
        disp_set_aal_alarm(1);
    }

}


static unsigned int brightness_mapping(unsigned int level)
{
    unsigned int mapped_level;

    // PWM duty input =  PWM_DUTY_IN / 1024
    mapped_level = level;

    if (mapped_level > gMaxLevel)
        mapped_level = gMaxLevel;

    return mapped_level;
}

#if !defined(MTK_AAL_SUPPORT)
#ifdef USE_DISP_BLS_MUTEX
static int disp_poll_for_reg(unsigned int addr, unsigned int value, unsigned int mask, unsigned int timeout)
{
    unsigned int cnt = 0;
    
    while ((DISP_REG_GET(addr) & mask) != value)
    {
        msleep(1);
        cnt++;
        if (cnt > timeout)
        {
            return -1;
        }
    }

    return 0;
}


static int disp_bls_get_mutex(void)
{
    if (gBLSMutexID < 0)
        return -1;

    DISP_REG_SET(DISP_REG_CONFIG_MUTEX(gBLSMutexID), 1);
    if(disp_poll_for_reg(DISP_REG_CONFIG_MUTEX(gBLSMutexID), 0x2, 0x2, POLLING_TIME_OUT))
    {
        BLS_ERR("get mutex timeout! \n");
        disp_dump_reg(DISP_MODULE_CONFIG);
        return -1;
    }
    return 0;
}

static int disp_bls_release_mutex(void)
{ 
    if (gBLSMutexID < 0)
        return -1;
    
    DISP_REG_SET(DISP_REG_CONFIG_MUTEX(gBLSMutexID), 0);
    if(disp_poll_for_reg(DISP_REG_CONFIG_MUTEX(gBLSMutexID), 0, 0x2, POLLING_TIME_OUT))
    {
        BLS_ERR("release mutex timeout! \n");
        disp_dump_reg(DISP_MODULE_CONFIG);
        return -1;
    }
    return 0;
}
#endif
#endif

void disp_bls_update_gamma_lut(void)
{
#if defined(DDP_GAMMA_SUPPORT)
    int index, i;
    unsigned long CurVal, Count;

    BLS_MSG("disp_bls_update_gamma_lut!\n");

    if (DISP_REG_GET(DISP_REG_BLS_EN) & 0x1)
    {
        BLS_ERR("try to update gamma lut while BLS is active\n");
        return;
    }

    // init gamma table
    for(index = 0; index < 3; index++)
    {    
        for(Count = 0; Count < GAMMA_LUT_ENTRY; Count ++)
        {  
             g_gamma_lut.entry[index][Count] = g_gamma_index.entry[index][Count];
        }
    }

    DISP_REG_SET(DISP_REG_BLS_LUT_UPDATE, 0x1);
        
    for (i = 0; i < GAMMA_LUT_ENTRY; i ++)
    {
        CurVal = (((g_gamma_lut.entry[0][i] & 0x3FF) << 20) | ((g_gamma_lut.entry[1][i] & 0x3FF) << 10) | (g_gamma_lut.entry[2][i] & 0x3FF));
        DISP_REG_SET(DISP_REG_BLS_GAMMA_LUT(i), CurVal);
        BLS_DBG("[%d] GAMMA LUT = 0x%x, (%lu, %lu, %lu)\n", i, DISP_REG_GET(DISP_REG_BLS_GAMMA_LUT(i)), 
            g_gamma_lut.entry[0][i], g_gamma_lut.entry[1][i], g_gamma_lut.entry[2][i]);
    }
    
    /* Set Gamma Last point*/    
    DISP_REG_SET(DISP_REG_BLS_GAMMA_SETTING, 0x00000001);
    DISP_REG_SET(DISP_REG_BLS_LUT_UPDATE, 0);
#endif
}

void disp_bls_update_pwm_lut(void)
{
    int i;
    unsigned int regValue;

    BLS_MSG("disp_bls_update_pwm_lut!\n");

    regValue = DISP_REG_GET(DISP_REG_BLS_EN);
    if (regValue & 0x1) {
        BLS_ERR("update PWM LUT while BLS func enabled!\n");
        disp_dump_reg(DISP_MODULE_BLS);
    }
    //DISP_REG_SET(DISP_REG_BLS_EN, (regValue & 0x00010000));

    for (i = 0; i < PWM_LUT_ENTRY; i++)
    {
        DISP_REG_SET(DISP_REG_BLS_LUMINANCE(i), g_pwm_lut.entry[i]);
        BLS_DBG("[%d] PWM LUT = 0x%x (%lu)\n", i, DISP_REG_GET(DISP_REG_BLS_LUMINANCE(i)), g_pwm_lut.entry[i]);

    }
    DISP_REG_SET(DISP_REG_BLS_LUMINANCE_255, g_pwm_lut.entry[PWM_LUT_ENTRY-1]);
    //DISP_REG_SET(DISP_REG_BLS_EN, regValue);
}

void disp_bls_init(unsigned int srcWidth, unsigned int srcHeight)
{
    struct cust_mt65xx_led *cust_led_list = get_cust_led_list();
    struct cust_mt65xx_led *cust = NULL;
    struct PWM_config *config_data = NULL;

    if(cust_led_list)
    {
        cust = &cust_led_list[MT65XX_LED_TYPE_LCD];
        if((strcmp(cust->name,"lcd-backlight") == 0) && (cust->mode == MT65XX_LED_MODE_CUST_BLS_PWM))
        {
            config_data = &cust->config_data;
            if (config_data->clock_source >= 0 && config_data->clock_source <= 1)
            {
                unsigned int regVal = DISP_REG_GET(CLK_MUX_SEL0);
                if (config_data->clock_source == 0)
                    clkmux_sel(MT_CLKMUX_PWM_MM_MUX_SEL, MT_CG_SYS_26M, "DISP_PWM");
                else
                    clkmux_sel(MT_CLKMUX_PWM_MM_MUX_SEL, MT_CG_UPLL_D12, "DISP_PWM");
                BLS_DBG("disp_bls_init : CLK_MUX_SEL0 0x%x => 0x%x\n", regVal, DISP_REG_GET(CLK_MUX_SEL0));
            }
            gPWMDiv = (config_data->div == 0) ? PWM_DEFAULT_DIV_VALUE : config_data->div;
            gPWMDiv &= 0x3FF;
            BLS_MSG("disp_bls_init : PWM config data (%d,%d)\n", config_data->clock_source, config_data->div);
        }
    }
        
    BLS_DBG("disp_bls_init : srcWidth = %d, srcHeight = %d\n", srcWidth, srcHeight);
    BLS_MSG("disp_bls_init : BLS_EN=0x%x, CON_0=0x%x, CON_1=0x%x, CG=0x%x, %d, %d\n", 
        DISP_REG_GET(DISP_REG_BLS_EN),
        DISP_REG_GET(DISP_REG_PWM_CON_0),
        DISP_REG_GET(DISP_REG_PWM_CON_1),
        DISP_REG_GET(DISP_REG_CONFIG_MMSYS_CG_CON0),
        clock_is_on(MT_CG_DISP_PWM_26M_SW_CG),
        clock_is_on(MT_CG_DISP_BLS_SW_CG));
      
    DISP_REG_SET(DISP_REG_BLS_SRC_SIZE, (srcHeight << 16) | srcWidth);
    DISP_REG_SET(DISP_REG_PWM_CON_0, 0x0 | (gPWMDiv << 16));
    DISP_REG_SET(DISP_REG_BLS_BLS_SETTING, 0x0);
    DISP_REG_SET(DISP_REG_BLS_INTEN, 0xF);
    if (!DISP_REG_GET(DISP_REG_PWM_EN))
        DISP_REG_SET(DISP_REG_PWM_CON_1, 0x000003FF);

    disp_bls_update_gamma_lut();
    //disp_bls_update_pwm_lut();

    disp_bls_config_full(srcWidth, srcHeight);
    printk("BLS Config = 0x%08X\n", DISP_REG_GET(DISP_REG_BLS_SRC_SIZE));

    if (dbg_log)
        disp_dump_reg(DISP_MODULE_BLS);

    g_NrScreenPixels = srcWidth * srcHeight;
}

int disp_bls_config(void)
{
#if !defined(MTK_AAL_SUPPORT)
    struct cust_mt65xx_led *cust_led_list = get_cust_led_list();
    struct cust_mt65xx_led *cust = NULL;
    struct PWM_config *config_data = NULL;

    if(cust_led_list)
    {
        cust = &cust_led_list[MT65XX_LED_TYPE_LCD];
        if((strcmp(cust->name,"lcd-backlight") == 0) && (cust->mode == MT65XX_LED_MODE_CUST_BLS_PWM))
        {
            config_data = &cust->config_data;
            if (config_data->clock_source >= 0 && config_data->clock_source <= 1)
            {
                unsigned int regVal = DISP_REG_GET(CLK_MUX_SEL0);
                if (config_data->clock_source == 0)
                    clkmux_sel(MT_CLKMUX_PWM_MM_MUX_SEL, MT_CG_SYS_26M, "DISP_PWM");
                else
                    clkmux_sel(MT_CLKMUX_PWM_MM_MUX_SEL, MT_CG_UPLL_D12, "DISP_PWM");
                BLS_DBG("disp_bls_config : CLK_MUX_SEL0 0x%x => 0x%x\n", regVal, DISP_REG_GET(CLK_MUX_SEL0));
            }
            gPWMDiv = (config_data->div == 0) ? PWM_DEFAULT_DIV_VALUE : config_data->div;
            gPWMDiv &= 0x3FF;
            BLS_MSG("disp_bls_config : PWM config data (%d,%d)\n", config_data->clock_source, config_data->div);
        }
    }
    
    if (!clock_is_on(MT_CG_DISP_PWM_26M_SW_CG) || !gBLSPowerOn)
    {
        ASSERT(0);
    }

    BLS_MSG("disp_bls_config : BLS_EN=0x%x, CON_0=0x%x, CON_1=0x%x, CG=0x%x, %d, %d\n", 
        DISP_REG_GET(DISP_REG_BLS_EN),
        DISP_REG_GET(DISP_REG_PWM_CON_0),
        DISP_REG_GET(DISP_REG_PWM_CON_1),
        DISP_REG_GET(DISP_REG_CONFIG_MMSYS_CG_CON0),
        clock_is_on(MT_CG_DISP_PWM_26M_SW_CG),
        clock_is_on(MT_CG_DISP_BLS_SW_CG));

#ifdef USE_DISP_BLS_MUTEX 
    BLS_MSG("disp_bls_config : gBLSMutexID = %d\n", gBLSMutexID);

    DISP_REG_SET(DISP_REG_CONFIG_MUTEX_RST(gBLSMutexID), 1);
    DISP_REG_SET(DISP_REG_CONFIG_MUTEX_RST(gBLSMutexID), 0);
    DISP_REG_SET(DISP_REG_CONFIG_MUTEX_MOD(gBLSMutexID), DDP_MOD_DISP_PWM);    // PWM
    DISP_REG_SET(DISP_REG_CONFIG_MUTEX_SOF(gBLSMutexID), 0);        // single mode
    DISP_REG_SET(DISP_REG_CONFIG_MUTEX_EN(gBLSMutexID), 1);

    if (disp_bls_get_mutex() == 0)
    {
    
#else
    BLS_MSG("disp_bls_config\n");
    DISP_REG_SET(DISP_REG_PWM_DEBUG, 0x3);
#endif

        if (!DISP_REG_GET(DISP_REG_PWM_EN))
            DISP_REG_SET(DISP_REG_PWM_CON_1, 0x000003FF);
        DISP_REG_SET(DISP_REG_PWM_CON_0, 0x0 | (gPWMDiv << 16));

#ifdef USE_DISP_BLS_MUTEX 

        if (disp_bls_release_mutex() == 0)
            return 0;
    }
    return -1;
#else
    DISP_REG_SET(DISP_REG_PWM_DEBUG, 0x0);
#endif

#endif
    BLS_MSG("disp_bls_config:-\n");
    return 0;
}

void disp_bls_config_full(unsigned int width, unsigned int height)
{
    unsigned int dither_bpp = DISP_GetOutputBPPforDithering(); 

    BLS_DBG("disp_bls_config_full, width=%d, height=%d, reg=0x%x \n", 
        width, height, ((height<<16) + width));


#if defined(DDP_GAMMA_SUPPORT)
    DISP_REG_SET(DISP_REG_BLS_BLS_SETTING       ,0x00100007);
#else
    DISP_REG_SET(DISP_REG_BLS_BLS_SETTING       ,0x00100000);
#endif
    DISP_REG_SET(DISP_REG_BLS_SRC_SIZE          ,((height<<16) + width));
    DISP_REG_SET(DISP_REG_BLS_GAMMA_SETTING     ,0x00000001);

/* BLS Luminance LUT */
    DISP_REG_SET(DISP_REG_BLS_LUMINANCE(0)      ,0x00000000);
    DISP_REG_SET(DISP_REG_BLS_LUMINANCE(1)      ,0x00000004);
    DISP_REG_SET(DISP_REG_BLS_LUMINANCE(2)      ,0x00000010);
    DISP_REG_SET(DISP_REG_BLS_LUMINANCE(3)      ,0x00000024);
    DISP_REG_SET(DISP_REG_BLS_LUMINANCE(4)      ,0x00000040);
    DISP_REG_SET(DISP_REG_BLS_LUMINANCE(5)      ,0x00000064);
    DISP_REG_SET(DISP_REG_BLS_LUMINANCE(6)      ,0x00000090);
    DISP_REG_SET(DISP_REG_BLS_LUMINANCE(7)      ,0x000000C4);
    DISP_REG_SET(DISP_REG_BLS_LUMINANCE(8)      ,0x00000100);
    DISP_REG_SET(DISP_REG_BLS_LUMINANCE(9)      ,0x00000144);
    DISP_REG_SET(DISP_REG_BLS_LUMINANCE(10)     ,0x00000190);
    DISP_REG_SET(DISP_REG_BLS_LUMINANCE(11)     ,0x000001E4);
    DISP_REG_SET(DISP_REG_BLS_LUMINANCE(12)     ,0x00000240);
    DISP_REG_SET(DISP_REG_BLS_LUMINANCE(13)     ,0x00000244);
    DISP_REG_SET(DISP_REG_BLS_LUMINANCE(14)     ,0x00000310);
    DISP_REG_SET(DISP_REG_BLS_LUMINANCE(15)     ,0x00000384);
    DISP_REG_SET(DISP_REG_BLS_LUMINANCE(16)     ,0x00000400);
    DISP_REG_SET(DISP_REG_BLS_LUMINANCE(17)     ,0x00000484);
    DISP_REG_SET(DISP_REG_BLS_LUMINANCE(18)     ,0x00000510);
    DISP_REG_SET(DISP_REG_BLS_LUMINANCE(19)     ,0x000005A4);
    DISP_REG_SET(DISP_REG_BLS_LUMINANCE(20)     ,0x00000640);
    DISP_REG_SET(DISP_REG_BLS_LUMINANCE(21)     ,0x000006E4);
    DISP_REG_SET(DISP_REG_BLS_LUMINANCE(22)     ,0x00000790);
    DISP_REG_SET(DISP_REG_BLS_LUMINANCE(23)     ,0x00000843);
    DISP_REG_SET(DISP_REG_BLS_LUMINANCE(24)     ,0x000008FF);
    DISP_REG_SET(DISP_REG_BLS_LUMINANCE(25)     ,0x000009C3);
    DISP_REG_SET(DISP_REG_BLS_LUMINANCE(26)     ,0x00000A8F);
    DISP_REG_SET(DISP_REG_BLS_LUMINANCE(27)     ,0x00000B63);
    DISP_REG_SET(DISP_REG_BLS_LUMINANCE(28)     ,0x00000C3F);
    DISP_REG_SET(DISP_REG_BLS_LUMINANCE(29)     ,0x00000D23);
    DISP_REG_SET(DISP_REG_BLS_LUMINANCE(30)     ,0x00000E0F);
    DISP_REG_SET(DISP_REG_BLS_LUMINANCE(31)     ,0x00000F03);
    DISP_REG_SET(DISP_REG_BLS_LUMINANCE(32)     ,0x00000FFF);
/* BLS Luminance 255 */
    DISP_REG_SET(DISP_REG_BLS_LUMINANCE_255     ,0x00000FDF);
    
/* Dither */
    DISP_REG_SET(DISP_REG_BLS_DITHER(5)         ,0x00000000);
    DISP_REG_SET(DISP_REG_BLS_DITHER(6)         ,0x00003004);
    DISP_REG_SET(DISP_REG_BLS_DITHER(7)         ,0x00000000);
    DISP_REG_SET(DISP_REG_BLS_DITHER(8)         ,0x00000000);
    DISP_REG_SET(DISP_REG_BLS_DITHER(9)         ,0x00000000);
    DISP_REG_SET(DISP_REG_BLS_DITHER(10)        ,0x00000000);
    DISP_REG_SET(DISP_REG_BLS_DITHER(11)        ,0x00000000);
    DISP_REG_SET(DISP_REG_BLS_DITHER(12)        ,0x00000011);
    DISP_REG_SET(DISP_REG_BLS_DITHER(13)        ,0x00000000);
    DISP_REG_SET(DISP_REG_BLS_DITHER(14)        ,0x00000000);
/* output RGB888 */
    if (dither_bpp == 16) // 565
    {
        DISP_REG_SET(DISP_REG_BLS_DITHER(15), 0x50500001);
        DISP_REG_SET(DISP_REG_BLS_DITHER(16), 0x50504040);
        DISP_REG_SET(DISP_REG_BLS_DITHER(0), 0x00000001);
    }
    else if (dither_bpp == 18) // 666
    {
        DISP_REG_SET(DISP_REG_BLS_DITHER(15), 0x40400001);
        DISP_REG_SET(DISP_REG_BLS_DITHER(16), 0x40404040);
        DISP_REG_SET(DISP_REG_BLS_DITHER(0), 0x00000001);
    }
    else if (dither_bpp == 24) // 888
    {
        DISP_REG_SET(DISP_REG_BLS_DITHER(15), 0x20200001);
        DISP_REG_SET(DISP_REG_BLS_DITHER(16), 0x20202020);
        DISP_REG_SET(DISP_REG_BLS_DITHER(0), 0x00000001);
    }
    else
    {
        BLS_MSG("error diter bpp = %d\n", dither_bpp);        
        DISP_REG_SET(DISP_REG_BLS_DITHER(0), 0x00000000);
    }


    DISP_REG_SET(DISP_REG_BLS_INTEN             ,0x0000000f); // no scene change

    DISP_REG_SET(DISP_REG_BLS_EN                ,0x00000001);
}


int disp_bls_set_max_backlight(unsigned int level)
{
    mutex_lock(&backlight_mutex);
    BLS_MSG("disp_bls_set_max_backlight: level = %d, current level = %d\n", level * 1023 / 255, gMaxLevel);
    //PWM duty input =  PWM_DUTY_IN / 1024
    gMaxLevel = level * 1023 / 255;
    mutex_unlock(&backlight_mutex);
    return 0;
}


#if !defined(MTK_AAL_SUPPORT)
int disp_bls_set_backlight(unsigned int level)
{
    unsigned int mapped_level;
    BLS_MSG("disp_bls_set_backlight: %d, gBLSPowerOn = %d\n", level, gBLSPowerOn);

    mutex_lock(&backlight_mutex);

#ifdef USE_DISP_BLS_MUTEX 
    disp_bls_get_mutex();
#else
    DISP_REG_SET(DISP_REG_PWM_DEBUG, 0x3);
#endif

    mapped_level = brightness_mapping(level);
    BLS_MSG("after mapping, mapped_level: %d\n", mapped_level);
    DISP_REG_SET(DISP_REG_PWM_CON_1, brightness_mapping(level) << 16 | 0x3FF);
    if (level != 0)
    {
        if (!DISP_REG_GET(DISP_REG_PWM_EN))
            DISP_REG_SET(DISP_REG_PWM_EN, 1);
    }
    else
    {
        if (DISP_REG_GET(DISP_REG_PWM_EN))
            DISP_REG_SET(DISP_REG_PWM_EN, 0);
    }
    BLS_MSG("after SET, CON_1: 0x%x\n", DISP_REG_GET(DISP_REG_PWM_CON_1));

#ifdef USE_DISP_BLS_MUTEX 
    disp_bls_release_mutex();
#else
    DISP_REG_SET(DISP_REG_PWM_DEBUG, 0x0);
#endif

    mutex_unlock(&backlight_mutex);
    return 0;    
}
#else
int disp_bls_set_backlight(unsigned int level)
{
    DISP_AAL_PARAM *param;
    BLS_MSG("disp_bls_set_backlight: %d\n", level);

    mutex_lock(&backlight_mutex);
    disp_aal_lock();
    param = get_aal_config();
    param->pwmDuty = level;
    disp_aal_unlock();
    mutex_unlock(&backlight_mutex);
    return 0;
}
#endif


int disp_bls_reset(void)
{
    unsigned int regValue;

    regValue = DISP_REG_GET(DISP_REG_BLS_RST);
    DISP_REG_SET(DISP_REG_BLS_RST, regValue | 0x1);
    DISP_REG_SET(DISP_REG_BLS_RST, regValue & (~0x1));

    return 0;
}


