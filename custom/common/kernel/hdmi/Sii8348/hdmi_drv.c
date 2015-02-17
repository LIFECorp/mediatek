#if defined(MTK_HDMI_SUPPORT)
#include <linux/kernel.h>

#include <linux/xlog.h>

#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/kthread.h>

#include <mtk_kpd.h>        /* custom file */

#include "si_timing_defs.h"

#include "hdmi_drv.h"
#include <mach/irqs.h>
#include "mach/eint.h"
#include <mach/mt_gpio.h>
#include <cust_gpio_usage.h>
#include <cust_eint.h>
#include <mach/mt_pm_ldo.h>


///#define sii8348_debug_on
static size_t hdmi_log_on = true;
#define HDMI_LOG(fmt, arg...)  \
	do { \
		if (hdmi_log_on) printk("[hdmi_drv]%s,%d ", __func__, __LINE__); printk(fmt, ##arg); \
	}while (0)

#define HDMI_FUNC()    \
	do { \
		if(hdmi_log_on) printk("[hdmi_drv] %s\n", __func__); \
	}while (0)

// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH  (480)
#define FRAME_HEIGHT (800)

// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

//static struct task_struct *hdmi_event_task = NULL;
//static struct task_struct *hdmi_hpd_detect_task = NULL;
//static int hdmi_event_status = HDMI_STATE_NO_DEVICE;

wait_queue_head_t hdmi_event_wq;
atomic_t hdmi_event = ATOMIC_INIT(0);

static HDMI_UTIL_FUNCS hdmi_util = {0};

#define SET_RESET_PIN(v)    (hdmi_util.set_reset_pin((v)))

#define UDELAY(n) (hdmi_util.udelay(n))
#define MDELAY(n) (hdmi_util.mdelay(n))

extern int SiiMhlTxInitialize(unsigned char pollIntervalMs );
extern void SiiMhlTxHwReset(uint16_t hwResetPeriod,uint16_t hwResetDelay);
extern unsigned int pmic_config_interface (unsigned int RegNum, unsigned int val, unsigned int MASK, unsigned int SHIFT);
extern void SwitchToD3( void );
extern void SiiMhlTxHwResetKeepLow(void);
extern void ForceSwitchToD3( void );
extern void SiiMhlTxHwGpioSuspend(void);
extern void SiiMhlTxHwGpioResume(void);
extern struct mhl_dev_context *si_dev_context;

extern int si_mhl_tx_initialize(struct mhl_dev_context *dev_context, bool bootup);
extern void siHdmiTx_AudioSel (int AduioMode);
extern void	si_mhl_tx_reset_states(struct mhl_dev_context *dev_context);
extern int si_mhl_tx_chip_initialize(struct drv_hw_context *hw_context);

static void hdmi_drv_set_util_funcs(const HDMI_UTIL_FUNCS *util)
{
	memcpy(&hdmi_util, util, sizeof(HDMI_UTIL_FUNCS));
}

static void hdmi_drv_get_params(HDMI_PARAMS *params)
{
    HDMI_FUNC();
	memset(params, 0, sizeof(HDMI_PARAMS));
    params->init_config.vformat         = HDMI_VIDEO_1280x720p_60Hz;
    params->init_config.aformat         = HDMI_AUDIO_PCM_16bit_44100;

	params->clk_pol           = HDMI_POLARITY_FALLING;
	params->de_pol            = HDMI_POLARITY_RISING;
	params->vsync_pol         = HDMI_POLARITY_RISING;
	params->hsync_pol         = HDMI_POLARITY_RISING;

	params->hsync_front_porch = 110;
	params->hsync_pulse_width = 40;
	params->hsync_back_porch  = 220;

	params->vsync_front_porch = 5;
	params->vsync_pulse_width = 5;
	params->vsync_back_porch  = 20;

	params->rgb_order         = HDMI_COLOR_ORDER_RGB;

	params->io_driving_current = IO_DRIVING_CURRENT_2MA;
	params->intermediat_buffer_num = 4;
    params->scaling_factor = 0;
    params->cabletype = MHL_2_CABLE;
}

void hdmi_drv_suspend(void)
{
}

void hdmi_drv_resume(void)
{
}

extern void siHdmiTx_VideoSel (int vmode);
extern bool si_mhl_tx_set_path_en_I(struct mhl_dev_context *dev_context);

static int hdmi_drv_video_config(HDMI_VIDEO_RESOLUTION vformat, HDMI_VIDEO_INPUT_FORMAT vin, HDMI_VIDEO_OUTPUT_FORMAT vout)
{

	if(vformat == HDMI_VIDEO_720x480p_60Hz)
	{
		HDMI_LOG("[hdmi_drv]480p\n");
		siHdmiTx_VideoSel(HDMI_480P60_4X3);
	}
	else if(vformat == HDMI_VIDEO_1280x720p_60Hz)
	{
		HDMI_LOG("[hdmi_drv]720p\n");
		siHdmiTx_VideoSel(HDMI_720P60);
	}
	else if(vformat == HDMI_VIDEO_1920x1080p_30Hz)
	{
		HDMI_LOG("[hdmi_drv]1080p\n");
		siHdmiTx_VideoSel(HDMI_1080P30);
	}
	else
	{
		HDMI_LOG("%s, video format not support now\n", __func__);
	}
///	si_dev_context->drv_info->mhl_start_video((struct drv_hw_context *)
///				(&si_dev_context->drv_context));;
	//si_mhl_tx_set_path_en_I(si_dev_context);
    return 0;
}

static int hdmi_drv_audio_config(HDMI_AUDIO_FORMAT aformat)
{
    return 0;
}

static int hdmi_drv_video_enable(bool enable)
{
    return 0;
}

static int hdmi_drv_audio_enable(bool enable)
{
    return 0;
}

extern int si_8348_init(void);
extern int HalOpenI2cDevice(char const *DeviceName, char const *DriverName);
static bool chip_power_on = false;

static int hdmi_drv_init(void)
{
    printk("hdmi_drv_init +\n" );

    hwPowerOn(MT6331_POWER_LDO_VGP2, VOL_1200,"MHL");
    HalOpenI2cDevice("Sil_MHL", "sii8338drv");
    
    si_8348_init();

    printk("hdmi_drv_init -\n" );
    return 0;
}

static int hdmi_drv_enter(void)
{
    HDMI_FUNC();
    return 0;
}

static int hdmi_drv_exit(void)
{
    HDMI_FUNC();
    return 0;
}

extern unsigned char SiiTxReadConnectionStatus(void);
HDMI_STATE hdmi_drv_get_state(void)
{
	int ret = SiiTxReadConnectionStatus();
	printk("mhl sii status:%d\n", ret);
	if(ret == 1)
		return HDMI_STATE_ACTIVE;
	else
		return HDMI_STATE_NO_DEVICE;
}

extern int	chip_device_id;
int hdmi_drv_power_on(void)
{
    int ret = 1;
    unsigned int dpi_pin_start = 0;
	HDMI_FUNC();

#ifdef GPIO_MHL_I2S_OUT_WS_PIN
    mt_set_gpio_mode(GPIO_MHL_I2S_OUT_WS_PIN, GPIO_MHL_I2S_OUT_WS_PIN_M_I2S3_WS);
    mt_set_gpio_mode(GPIO_MHL_I2S_OUT_CK_PIN, GPIO_MHL_I2S_OUT_CK_PIN_M_I2S3_BCK);
    mt_set_gpio_mode(GPIO_MHL_I2S_OUT_DAT_PIN, GPIO_MHL_I2S_OUT_DAT_PIN_M_I2S3_DO);
#else
    printk("%s,%d Error: GPIO_MHL_I2S_OUT_WS_PIN is not defined\n", __func__, __LINE__);
#endif

#ifdef GPIO_EXT_DISP_DPI0_PIN
    for(dpi_pin_start = GPIO_EXT_DISP_DPI0_PIN; dpi_pin_start < GPIO_EXT_DISP_DPI0_PIN + 16; dpi_pin_start++)
    {
        mt_set_gpio_mode(dpi_pin_start, GPIO_EXT_DISP_DPI0_PIN_M_DPI_CK);
    }
#else
    printk("%s,%d Error: GPIO_EXT_DISP_DPI0_PIN is not defined\n", __func__, __LINE__);
#endif

#if 0
    si_mhl_tx_reset_states(si_dev_context);
    ret = si_dev_context->drv_info->mhl_device_initialize(
            (struct drv_hw_context *)(&si_dev_context->drv_context));
    ///ret = si_mhl_tx_chip_initialize((struct drv_hw_context *)(si_dev_context->drv_context));
#endif	

#ifdef CUST_EINT_MHL_NUM
	mt_eint_mask(CUST_EINT_MHL_NUM);
#else
    printk("%s,%d Error: CUST_EINT_MHL_NUM is not defined\n", __func__, __LINE__);
#endif

#ifndef GPIO_MHL_RST_B_PIN
    printk("%s,%d Error: GPIO_MHL_RST_B_PIN is not defined\n", __func__, __LINE__);
#endif

    if(chip_device_id >0)
        ret = 0;

    printk("MHL hdmi_drv_power_on status %d,chipid: %X, ret: %d\n", SiiTxReadConnectionStatus() , chip_device_id, ret);
	    
#ifdef 	CUST_EINT_MHL_NUM    
	mt_eint_unmask(CUST_EINT_MHL_NUM);
#endif	
    return ret;
}

void hdmi_drv_power_off(void)
{
    unsigned int dpi_pin_start = 0;
	HDMI_FUNC();

#ifdef GPIO_MHL_I2S_OUT_WS_PIN
    mt_set_gpio_pull_enable(GPIO_MHL_I2S_OUT_WS_PIN, GPIO_PULL_DISABLE);
    mt_set_gpio_pull_enable(GPIO_MHL_I2S_OUT_CK_PIN, GPIO_PULL_DISABLE);
    mt_set_gpio_pull_enable(GPIO_MHL_I2S_OUT_DAT_PIN, GPIO_PULL_DISABLE);
#endif

#ifdef GPIO_EXT_DISP_DPI0_PIN
    for(dpi_pin_start = GPIO_EXT_DISP_DPI0_PIN; dpi_pin_start < GPIO_EXT_DISP_DPI0_PIN + 16; dpi_pin_start++)
    {
        mt_set_gpio_pull_enable(dpi_pin_start, GPIO_PULL_DISABLE);
    }
#endif

	//SwitchToD3();
	//SiiMhlTxHwResetKeepLow();
	//pmic_config_interface(0x87,0x0,0x01,0x0);
}



void hdmi_drv_log_enable(bool enable)
{
    hdmi_log_on = enable;
}

const HDMI_DRIVER* HDMI_GetDriver(void)
{
	static const HDMI_DRIVER HDMI_DRV =
	{
		.set_util_funcs = hdmi_drv_set_util_funcs,
		.get_params     = hdmi_drv_get_params,
		.init           = hdmi_drv_init,
        .enter          = hdmi_drv_enter,
        .exit           = hdmi_drv_exit,
		.suspend        = hdmi_drv_suspend,
		.resume         = hdmi_drv_resume,
        .video_config   = hdmi_drv_video_config,
        .audio_config   = hdmi_drv_audio_config,
        .video_enable   = hdmi_drv_video_enable,
        .audio_enable   = hdmi_drv_audio_enable,
        .power_on       = hdmi_drv_power_on,
        .power_off      = hdmi_drv_power_off,
        .get_state      = hdmi_drv_get_state,
        .log_enable     = hdmi_drv_log_enable
	};

    HDMI_FUNC();
	return &HDMI_DRV;
}
#endif
