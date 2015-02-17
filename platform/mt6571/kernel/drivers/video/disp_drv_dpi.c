#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/semaphore.h>
#include <mach/m4u.h>

#include "disp_drv.h"
#include "disp_drv_platform.h"
#include "disp_drv_log.h"

#include "lcd_drv.h"
#include "dpi_drv.h"
#include "dsi_drv.h"

#include "lcm_drv.h"

// ---------------------------------------------------------------------------
//  Private Variables
// ---------------------------------------------------------------------------

extern LCM_DRIVER *lcm_drv;
extern LCM_PARAMS *lcm_params;
extern unsigned int is_video_mode_running;
extern BOOL DISP_IsDecoupleMode(void);

static BOOL DDMS_capturing = FALSE;
static DPI_FB_FORMAT dpiTmpBufFormat = DPI_FB_FORMAT_RGB888;
static LCD_FB_FORMAT lcdTmpBufFormat = LCD_FB_FORMAT_RGB888;

// ---------------------------------------------------------------------------
//  Private Functions
// ---------------------------------------------------------------------------

static __inline DPI_FB_FORMAT get_tmp_buffer_dpi_format(void)
{
    switch(lcm_params->dpi.format)
    {
    case LCM_DPI_FORMAT_RGB565 : return DPI_FB_FORMAT_RGB565;
    case LCM_DPI_FORMAT_RGB666 :
    case LCM_DPI_FORMAT_RGB888 : return DPI_FB_FORMAT_RGB888;
    default : ASSERT(0);
    }
    return DPI_FB_FORMAT_RGB888;
}


static __inline UINT32 get_tmp_buffer_bpp(void)
{
    static const UINT32 TO_BPP[] = {2, 3};
    return TO_BPP[dpiTmpBufFormat];
}


static __inline LCD_FB_FORMAT get_tmp_buffer_lcd_format(void)
{
    static const UINT32 TO_LCD_FORMAT[] = {
        LCD_FB_FORMAT_RGB565,
        LCD_FB_FORMAT_RGB888
    };
    
    return TO_LCD_FORMAT[dpiTmpBufFormat];
}


static BOOL disp_drv_dpi_init_context(void)
{
    dpiTmpBufFormat = get_tmp_buffer_dpi_format();
    lcdTmpBufFormat = get_tmp_buffer_lcd_format();

    if (lcm_drv != NULL && lcm_params!= NULL) 
        return TRUE;
    else 
        printk("%s, lcm_drv=0x%08x, lcm_params=0x%08x\n", __func__, (unsigned int)lcm_drv, (unsigned int)lcm_params);

    if (NULL == lcm_drv) {
        printk("%s, lcm_drv is NULL\n", __func__);

        return FALSE;
    }

 
    return TRUE;
}


static void init_mipi_pll(void)
{
    DPI_CHECK_RET(DPI_Init_PLL(lcm_params));
}


static void init_io_pad(void)
{
    LCD_CHECK_RET(LCD_Init_IO_pad(lcm_params));
}


static void init_io_driving_current(void)
{
    DPI_CHECK_RET(DPI_Set_DrivingCurrent(lcm_params));
}


static void init_dpi(BOOL isDpiPoweredOn)
{
    const LCM_DPI_PARAMS *dpi = &(lcm_params->dpi);

    DPI_CHECK_RET(DPI_Init(isDpiPoweredOn));
    DPI_CHECK_RET(DPI_ConfigHsync((DPI_POLARITY)dpi->hsync_pol,
                                  dpi->hsync_pulse_width,
                                  dpi->hsync_back_porch,
                                  dpi->hsync_front_porch));
    DPI_CHECK_RET(DPI_ConfigVsync((DPI_POLARITY)dpi->vsync_pol,
                                  dpi->vsync_pulse_width,
                                  dpi->vsync_back_porch,
                                  dpi->vsync_front_porch));
    DPI_CHECK_RET(DPI_FBSetSize(DISP_GetScreenWidth(), DISP_GetScreenHeight()));
    DPI_CHECK_RET(DPI_OutputSetting(&(lcm_params->dpi)));
    DPI_CHECK_RET(DPI_EnableClk());
}


// ---------------------------------------------------------------------------
//  DPI Display Driver Public Functions
// ---------------------------------------------------------------------------

static DISP_STATUS dpi_init(UINT32 fbVA, UINT32 fbPA, BOOL isLcmInited)
{
    if (!disp_drv_dpi_init_context()) 
        return DISP_STATUS_NOT_IMPLEMENTED;

    init_mipi_pll();
    init_io_pad();
    init_io_driving_current();

    if (isLcmInited)
    {
        is_video_mode_running = true;
    }
    init_dpi(isLcmInited);

    if (NULL != lcm_drv->init && !isLcmInited) {
        lcm_drv->init();
    }

    {
        struct disp_path_config_struct config = {0};

        if (DISP_IsDecoupleMode())
        {
            config.srcModule = DISP_MODULE_RDMA0;
        }
        else
        {
            config.srcModule = DISP_MODULE_OVL;
        }

        config.bgROI.x = 0;
        config.bgROI.y = 0;
        config.bgROI.width = DISP_GetScreenWidth();
        config.bgROI.height = DISP_GetScreenHeight();
        config.bgColor = 0x0;  // background color

        config.pitch = DISP_GetScreenWidth()*2;
        config.srcROI.x = 0;config.srcROI.y = 0;
        config.srcROI.height= DISP_GetScreenHeight();config.srcROI.width= DISP_GetScreenWidth();
        config.ovl_config.source = OVL_LAYER_SOURCE_MEM; 
    
        // Disable FB layer (Layer3)
        {
            config.ovl_config.layer = FB_LAYER;
            config.ovl_config.layer_en = 0;
//            disp_path_get_mutex();
            disp_path_config_layer(&config.ovl_config);
//            disp_path_release_mutex();
//            disp_path_wait_reg_update();
        }

        // Disable LK UI layer (Layer2)
        {
            config.ovl_config.layer = FB_LAYER-1;
            config.ovl_config.layer_en = 0;
//            disp_path_get_mutex();
            disp_path_config_layer(&config.ovl_config);
//            disp_path_release_mutex();
//            disp_path_wait_reg_update();
        }

        config.ovl_config.layer = FB_LAYER;
        config.ovl_config.layer_en = 1;
        config.ovl_config.fmt = eRGB565;
        config.ovl_config.addr = fbPA;
        config.ovl_config.vaddr = fbVA;	
        config.ovl_config.source = OVL_LAYER_SOURCE_MEM;
        config.ovl_config.src_x = 0;
        config.ovl_config.src_y = 0;
        config.ovl_config.src_w = DISP_GetScreenWidth();
        config.ovl_config.src_h = DISP_GetScreenHeight();
        config.ovl_config.dst_x = 0;	   // ROI
        config.ovl_config.dst_y = 0;
        config.ovl_config.dst_w = DISP_GetScreenWidth();
        config.ovl_config.dst_h = DISP_GetScreenHeight();
        config.ovl_config.src_pitch = ALIGN_TO(DISP_GetScreenWidth(), MTK_FB_ALIGNMENT) * 2; //pixel number
        config.ovl_config.keyEn = 0;
        config.ovl_config.key = 0xFF;	   // color key
        config.ovl_config.aen = 0;			  // alpha enable
        config.ovl_config.alpha = 0;

        LCD_LayerSetAddress(FB_LAYER, fbPA);
        LCD_LayerSetFormat(FB_LAYER, LCD_LAYER_FORMAT_RGB565);
        LCD_LayerSetOffset(FB_LAYER, 0, 0);
        LCD_LayerSetSize(FB_LAYER,DISP_GetScreenWidth(),DISP_GetScreenHeight());
        LCD_LayerSetPitch(FB_LAYER, ALIGN_TO(DISP_GetScreenWidth(), MTK_FB_ALIGNMENT) * 2);
        LCD_LayerEnable(FB_LAYER, TRUE);

        config.dstModule = DISP_MODULE_DPI0;
        config.outFormat = RDMA_OUTPUT_FORMAT_ARGB;

        DPI_DisableClk();

        disp_path_get_mutex();

        // Config FB_Layer port to be physical.
        #if 1  // defined(MTK_M4U_SUPPORT)
        {
            M4U_PORT_STRUCT portStruct;

            portStruct.ePortID = M4U_PORT_LCD_OVL;		   //hardware port ID, defined in M4U_PORT_ID_ENUM
            portStruct.Virtuality = 1;
            portStruct.Security = 0;
            portStruct.domain = 3;			  //domain : 0 1 2 3
            portStruct.Distance = 1;
            portStruct.Direction = 0;
            m4u_config_port(&portStruct);
        }
        // hook m4u debug callback function
        m4u_set_tf_callback(M4U_CLNTMOD_DISP, &disp_m4u_dump_reg);
        #endif

        disp_path_config(&config);

        disp_path_release_mutex();

        DPI_EnableClk();
    }

    return DISP_STATUS_OK;
}


static DISP_STATUS dpi_enable_power(BOOL enable)
{
    if (enable) 
    {
        // initialize MIPI PLL
        DPI_CHECK_RET(DPI_MIPI_PowerOn());
        init_mipi_pll();
        DPI_mipi_switch(TRUE, lcm_params); 

        // enable MMSYS CG
        init_io_pad();
        init_io_driving_current();

        if (lcm_params->ctrl == LCM_CTRL_SERIAL_DBI)
        {
            LCD_CHECK_RET(LCD_PowerOn());
            LCD_CHECK_RET(LCD_RestoreRegisters());
        }

        DPI_CHECK_RET(DPI_PowerOn());

        // restore dpi register
        DPI_CHECK_RET(DPI_RestoreRegisters());

        DPI_CHECK_RET(DPI_EnableClk());

        // enable interrupt
        DPI_EnableIrq();
    }
    else 
    {
        is_video_mode_running = false;

        // backup dpi regsiter
        DPI_CHECK_RET(DPI_BackupRegisters());

        // disable interrupt
        DPI_DisableIrq();

        // disable clock
        DPI_CHECK_RET(DPI_DisableClk());
        
        // disable MMSYS CG
        DPI_CHECK_RET(DPI_PowerOff());

        if (lcm_params->ctrl == LCM_CTRL_SERIAL_DBI)
        {
            LCD_CHECK_RET(LCD_BackupRegisters());
            LCD_CHECK_RET(LCD_PowerOff());
        }

        // disable MIPI PLL
        DPI_mipi_switch(FALSE, lcm_params);
        DPI_CHECK_RET(DPI_MIPI_PowerOff());
    }

    return DISP_STATUS_OK;
}


static DISP_STATUS dpi_update_screen(BOOL isMuextLocked)
{
    disp_drv_dpi_init_context();

    if (!DDMS_capturing)
    {
        DPI_CHECK_RET(DPI_StartTransfer(isMuextLocked));
        is_video_mode_running = true;
    }
    
    if (DDMS_capturing)
        DISP_LOG_PRINT(ANDROID_LOG_INFO, "DPI", "[DISP] kernel - dpi_update_screen. DDMS is capturing. Skip one frame. \n");		
    
    return DISP_STATUS_OK;
}


static UINT32 dpi_get_working_buffer_size(void)
{
    return 0;
}


static UINT32 dpi_get_working_buffer_bpp(void)
{
    return 0;
}


static PANEL_COLOR_FORMAT dpi_get_panel_color_format(void)
{
    disp_drv_dpi_init_context();

    switch(lcm_params->dpi.format)
    {
    case LCM_DPI_FORMAT_RGB565 : return PANEL_COLOR_FORMAT_RGB565;
    case LCM_DPI_FORMAT_RGB666 : return PANEL_COLOR_FORMAT_RGB666;
    case LCM_DPI_FORMAT_RGB888 : return PANEL_COLOR_FORMAT_RGB888;
    default : ASSERT(0);
    }
    return PANEL_COLOR_FORMAT_RGB888;
}


DISP_STATUS dpi_capture_framebuffer(UINT32 pvbuf, UINT32 bpp)
{
    return DISP_STATUS_OK;	
}

#ifndef MIN
#define MIN(x,y) ((x)>(y)?(y):(x))
#endif


static UINT32 dpi_get_dithering_bpp(void)
{
    return MIN(get_tmp_buffer_bpp() * 8,PANEL_COLOR_FORMAT_TO_BPP(dpi_get_panel_color_format()));
}


static BOOL dpi_esd_check(void)
{
    BOOL result = false;

    result = lcm_drv->esd_check();

    return result;
}


const DISP_IF_DRIVER *DISP_GetDriverDPI(void)
{
    static const DISP_IF_DRIVER DPI_DISP_DRV =
    {
        .init                   = dpi_init,
        .enable_power           = dpi_enable_power,
        .update_screen          = dpi_update_screen,
        
        .get_working_buffer_size = dpi_get_working_buffer_size,
        .get_working_buffer_bpp = dpi_get_working_buffer_bpp,
        .get_panel_color_format = dpi_get_panel_color_format,
        .get_dithering_bpp		= dpi_get_dithering_bpp,
        .capture_framebuffer	= dpi_capture_framebuffer,
        .esd_check              = dpi_esd_check,
    };

    return &DPI_DISP_DRV;
}


