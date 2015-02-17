#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/xlog.h>
#include <linux/spinlock.h>
#include <mach/mt_clkmgr.h>
#include <mach/mt_gpio.h>

#include "ddp_reg.h"
#include "ddp_path.h"
#include "ddp_pq.h"

#define R2Y_EN        (1 << 15)
#define Y2R_EN        (1 << 14)
#define LUMA_CURVE_EN (1 << 13)
#define C_BOOST_EN    (1 << 12)
#define SHP_EN        (1 << 10)
#define CONT_EN       (1 << 9)
#define SAT_EN        (1 << 8)
#define HIST_EN       (1 << 7)
#define HIST_WGT_EN   (1 << 6)
#define DEMO_SWAP     (1 << 5)
#define DEMO_EN       (1 << 4)
#define RELAY_MODE    (1 << 0)

#define DISP_PQ_CTRL_FLD_RESET REG_FLD(1, 1)

typedef enum
{
    SHARPNESS_PROT,
    SHARPNESS_LIMIT,
    SHARPNESS_HPF_GAIN,
    SHARPNESS_BPF_GAIN,
    SHARPNESS_DECAY_RATIO,
    SHARPNESS_SOFT_RATIO,
    SHARPNESS_BOUND
} PQ_CD_DISPSHARP_PARAM;

typedef enum
{
    CONTRAST_CP2_Y,
    CONTRAST_CP2_X,
    CONTRAST_CP1_Y,
    CONTRAST_CP1_X,
    CONTRAST_SLOPE3,
    CONTRAST_SLOPE2,
    CONTRAST_SLOPE1,
    CONTRAST_OFFSET
} PQ_CD_CONTRAST_PARAM;

typedef enum
{
    SATURATION_MID_POINT,
    SATURATION_LOW_POINT,
    SATURATION_HIGH_POINT,
    SATURATION_SAT_HIGH_GAIN,
    SATURATION_SAT_MID_GAIN,
    SATURATION_SAT_LOW_GAIN,
    SATURATION_SAT_SLOPE2,
    SATURATION_SAT_SLOPE1
} PQ_CD_SAT_PARAM;

extern unsigned char pq_debug_flag;

// initailize
static DISP_PQ_PARAM g_PQ_Param =
{
    u4SharpnessAdj:0,
    u4ContrastAdj:0,
    u4SaturationAdj:0
};

static DISP_PQ_PARAM g_PQ_Cam_Param =
{
    u4SharpnessAdj:0,
    u4ContrastAdj:0,
    u4SaturationAdj:0
};

static DISP_PQ_PARAM g_PQ_Gal_Param =
{
    u4SharpnessAdj:0,
    u4ContrastAdj:0,
    u4SaturationAdj:0
};

//initialize index (because system default is 0, need fill with 0x80)

static DISPLAY_PQ_T g_PQ_Index =
{

contrast :
{
// CP2_Y CP2_X CP1_Y CP1_X SLOPE3 SLOPE2 SLOPE1 OFFSET
    {0xC8, 0xC8, 0x32, 0x32, 0x80, 0x80, 0x80, 0}, // 0
    {0xC8, 0xC8, 0x32, 0x32, 0x80, 0x80, 0x80, 0}, // 1
    {0xC8, 0xC8, 0x32, 0x32, 0x80, 0x80, 0x80, 0}, // 2
    {0xC8, 0xC8, 0x32, 0x32, 0x80, 0x80, 0x80, 0}, // 3
    {0xC8, 0xC8, 0x32, 0x32, 0x80, 0x80, 0x80, 0}, // 4
    {0xC8, 0xC8, 0x32, 0x32, 0x80, 0x80, 0x80, 0}, // 5
    {0xC8, 0xC8, 0x32, 0x32, 0x80, 0x80, 0x80, 0}, // 6
    {0xC8, 0xC8, 0x32, 0x32, 0x80, 0x80, 0x80, 0}, // 7
    {0xC8, 0xC8, 0x32, 0x32, 0x80, 0x80, 0x80, 0}, // 8
    {0xC8, 0xC8, 0x32, 0x32, 0x80, 0x80, 0x80, 0}, // 9

    {0xC8, 0xC8, 0x32, 0x32, 0x80, 0x80, 0x80, 0}, // custom 0
    {0xC8, 0xC8, 0x32, 0x32, 0x80, 0x80, 0x80, 0}, // custom 1
    {0xC8, 0xC8, 0x32, 0x32, 0x80, 0x80, 0x80, 0}  // custom 2
},

saturation :
{
// MID_POINT LOW_POINT HIGH_POINT SAT_HIGH_GAIN SAT_MID_GAIN SAT_LOW_GAIN SAT_SLOPE2 SAT_SLOPE1
    {0, 0, 0, 0x80, 0x80, 0x80, 0, 0}, // 0
    {0, 0, 0, 0x80, 0x80, 0x80, 0, 0}, // 1
    {0, 0, 0, 0x80, 0x80, 0x80, 0, 0}, // 2
    {0, 0, 0, 0x80, 0x80, 0x80, 0, 0}, // 3
    {0, 0, 0, 0x80, 0x80, 0x80, 0, 0}, // 4
    {0, 0, 0, 0x80, 0x80, 0x80, 0, 0}, // 5
    {0, 0, 0, 0x80, 0x80, 0x80, 0, 0}, // 6
    {0, 0, 0, 0x80, 0x80, 0x80, 0, 0}, // 7
    {0, 0, 0, 0x80, 0x80, 0x80, 0, 0}, // 8
    {0, 0, 0, 0x80, 0x80, 0x80, 0, 0}, // 9

    {0, 0, 0, 0x80, 0x80, 0x80, 0, 0}, // custom 0
    {0, 0, 0, 0x80, 0x80, 0x80, 0, 0}, // custom 1
    {0, 0, 0, 0x80, 0x80, 0x80, 0, 0}  // custom 2
}

};


static unsigned long g_PQ_Window = 0;
static DISP_AAL_STATISTICS gHist;
static unsigned long gHistIndicator = 0;
spinlock_t gHistLock;


void disp_set_hist_readlock(unsigned long bLock)
{
    unsigned long flag;

    spin_lock_irqsave(&gHistLock , flag);

    gHistIndicator = bLock;

    spin_unlock_irqrestore(&gHistLock , flag);
}

int disp_get_hist(unsigned int * pHist)
{
    disp_set_hist_readlock(1);

    memcpy(pHist , gHist.histogram  , (LUMA_HIST_BIN*sizeof(unsigned int)));

    disp_set_hist_readlock(0);
    return 0;
}

DISP_AAL_STATISTICS * disp_get_hist_ptr()
{
    return &gHist;
}

void disp_update_hist()
{
    int i = 0;
    unsigned long flag;

    spin_lock_irqsave(&gHistLock , flag);

    if(~gHistIndicator)
    {
        for (i = 0; i < LUMA_HIST_BIN; i++)
        {
            gHist.histogram[i] = DISP_REG_GET(DISP_REG_PQ_LUMA_HIST(i));
        }

        gHist.ChromHist = DISP_REG_GET(DISP_REG_PQ_C_HIST_BIN);

        for (i = 0; i < BLS_HIST_BIN; i++)
        {
            gHist.BLSHist[i] = DISP_REG_GET(DISP_REG_BLS_HIS_BIN(i));
        }
    }

    spin_unlock_irqrestore(&gHistLock , flag);
}

void disp_pq_set_window(unsigned int sat_upper, unsigned int sat_lower,
                           unsigned int hue_upper, unsigned int hue_lower)
{
    g_PQ_Window = (sat_upper << 24) | (sat_lower << 16) | (hue_upper << 8) | (hue_lower);
}


void disp_onConfig_luma(unsigned long * luma)
{
    unsigned long u4Index;
    unsigned long u4Val;

    for(u4Index = 0 ; u4Index < 8; u4Index ++)
    {
        u4Val = luma[u4Index*2] | luma[u4Index*2+1] << 16;
        DISP_REG_SET(DISP_REG_PQ_Y_FTN_1_0 + (u4Index << 2), u4Val);
    }
    DISP_REG_SET(DISP_REG_PQ_Y_FTN_16, luma[16]);

    DISP_REG_SET(DISP_REG_PQ_CFG, DISP_REG_GET(DISP_REG_PQ_CFG) | LUMA_CURVE_EN);
}

/*
*g_Color_Param
*/

DISP_PQ_PARAM * get_PQ_config()
{
    return &g_PQ_Param;
}


DISP_PQ_PARAM * get_PQ_Cam_config(void)
{
    return &g_PQ_Cam_Param;
}

DISP_PQ_PARAM * get_PQ_Gal_config(void)
{
    return &g_PQ_Gal_Param;
}
/*
*g_Color_Index
*/

DISPLAY_PQ_T * get_PQ_index()
{
    return &g_PQ_Index;
}


void disp_pq_init(void)
{
    spin_lock_init(&gHistLock);

    DISP_REG_SET(DISP_REG_PQ_CTRL, 0x00000002);
    DISP_REG_SET(DISP_REG_PQ_CTRL, 0x00000000);

    DISP_REG_SET(DISP_REG_PQ_INTEN, 0x00000007);

    DISP_REG_SET(DISP_REG_PQ_CTRL, 0x00000001);
    DISP_REG_SET(DISP_REG_PQ_C_HIST_CON, 0);
}



void disp_pq_config(unsigned int srcWidth,unsigned int srcHeight)
{
    DISP_REG_SET(DISP_REG_PQ_CFG,         R2Y_EN | Y2R_EN | CONT_EN | SAT_EN | HIST_EN | HIST_WGT_EN);
    DISP_REG_SET(DISP_REG_PQ_INPUT_SIZE,  (srcWidth << 0x10) + srcHeight);
    DISP_REG_SET(DISP_REG_PQ_OUTPUT_SIZE, (srcWidth << 0x10) + srcHeight);

    if (g_PQ_Param.u4SharpnessAdj >= PQ_TUNING_INDEX ||
        g_PQ_Param.u4ContrastAdj >= PQ_TUNING_INDEX ||
        g_PQ_Param.u4SaturationAdj >= PQ_TUNING_INDEX)
    {
	    //XLOGD("PQ Tuning index range error !\n");
	    return;
    }

#if defined (MTK_AAL_SUPPORT)
    // TODO: config cboost
    DISP_REG_SET(DISP_REG_PQ_C_BOOST_CON, 0x00000000);
#endif

// Sharpness
#if 0
    DISP_REG_SET(DISP_REG_PQ_SHP_CON_00, (g_PQ_Index.sharpness[g_PQ_Param.u4SharpnessAdj][SHARPNESS_PROT] << 24) |
                                         (g_PQ_Index.sharpness[g_PQ_Param.u4SharpnessAdj][SHARPNESS_LIMIT] << 16) |
                                         (g_PQ_Index.sharpness[g_PQ_Param.u4SharpnessAdj][SHARPNESS_HPF_GAIN] << 8) |
                                         (g_PQ_Index.sharpness[g_PQ_Param.u4SharpnessAdj][SHARPNESS_BPF_GAIN]));
    DISP_REG_SET(DISP_REG_PQ_SHP_CON_01, (g_PQ_Index.sharpness[g_PQ_Param.u4SharpnessAdj][SHARPNESS_DECAY_RATIO] << 16) |
                                         (g_PQ_Index.sharpness[g_PQ_Param.u4SharpnessAdj][SHARPNESS_SOFT_RATIO] << 8) |
                                         (g_PQ_Index.sharpness[g_PQ_Param.u4SharpnessAdj][SHARPNESS_BOUND]));
#endif

// Contrast
    DISP_REG_SET(DISP_REG_PQ_CONT_CP, (g_PQ_Index.contrast[g_PQ_Param.u4ContrastAdj][CONTRAST_CP2_Y] << 24) |
                                      (g_PQ_Index.contrast[g_PQ_Param.u4ContrastAdj][CONTRAST_CP2_X] << 16) |
                                      (g_PQ_Index.contrast[g_PQ_Param.u4ContrastAdj][CONTRAST_CP1_Y] << 8) |
                                      (g_PQ_Index.contrast[g_PQ_Param.u4ContrastAdj][CONTRAST_CP1_X]));
    DISP_REG_SET(DISP_REG_PQ_CONT_SLOPE, (g_PQ_Index.contrast[g_PQ_Param.u4ContrastAdj][CONTRAST_SLOPE3] << 16) |
                                         (g_PQ_Index.contrast[g_PQ_Param.u4ContrastAdj][CONTRAST_SLOPE2] << 8) |
                                         (g_PQ_Index.contrast[g_PQ_Param.u4ContrastAdj][CONTRAST_SLOPE1]));
    DISP_REG_SET(DISP_REG_PQ_CONT_OFFSET, (g_PQ_Index.contrast[g_PQ_Param.u4ContrastAdj][CONTRAST_OFFSET]));

// Saturation
    DISP_REG_SET(DISP_REG_PQ_SAT_CON_00, (g_PQ_Index.saturation[g_PQ_Param.u4SaturationAdj][SATURATION_MID_POINT] << 16) |
                                         (g_PQ_Index.saturation[g_PQ_Param.u4SaturationAdj][SATURATION_LOW_POINT]));
    DISP_REG_SET(DISP_REG_PQ_SAT_CON_01, (g_PQ_Index.saturation[g_PQ_Param.u4SaturationAdj][SATURATION_HIGH_POINT]));
    DISP_REG_SET(DISP_REG_PQ_SAT_GAIN, (g_PQ_Index.saturation[g_PQ_Param.u4SaturationAdj][SATURATION_SAT_HIGH_GAIN] << 16) |
                                       (g_PQ_Index.saturation[g_PQ_Param.u4SaturationAdj][SATURATION_SAT_MID_GAIN] << 8) |
                                       (g_PQ_Index.saturation[g_PQ_Param.u4SaturationAdj][SATURATION_SAT_LOW_GAIN]));
    DISP_REG_SET(DISP_REG_PQ_SAT_SLOPE, (g_PQ_Index.saturation[g_PQ_Param.u4SaturationAdj][SATURATION_SAT_SLOPE2] << 16) |
                                        (g_PQ_Index.saturation[g_PQ_Param.u4SaturationAdj][SATURATION_SAT_SLOPE1]));

    // histogram analysis
    DISP_REG_SET(DISP_REG_PQ_HIST_X_CFG, srcWidth);
    DISP_REG_SET(DISP_REG_PQ_HIST_Y_CFG, srcHeight);
}

void disp_pq_reset(void)
{
    DISP_REG_SET_FIELD(DISP_PQ_CTRL_FLD_RESET, DISP_REG_PQ_CTRL, 1);
    DISP_REG_SET_FIELD(DISP_PQ_CTRL_FLD_RESET, DISP_REG_PQ_CTRL, 0);
}

