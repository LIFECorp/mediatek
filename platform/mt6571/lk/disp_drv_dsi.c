#include <string.h>
#include <platform/mt_gpt.h>
#include <platform/disp_drv_platform.h>
#include <platform/ddp_path.h>

// ---------------------------------------------------------------------------
//  Private Variables
// ---------------------------------------------------------------------------
static UINT32 dsiTmpBufBpp = 0;
extern LCM_DRIVER *lcm_drv;
extern LCM_PARAMS *lcm_params;

static BOOL	dsi_vdo_streaming = FALSE;
static BOOL needStartDSI = TRUE;

// ---------------------------------------------------------------------------
//  Private Functions
// ---------------------------------------------------------------------------

static void init_lcd_te_control(void)
{
    const LCM_DBI_PARAMS *dbi = &(lcm_params->dbi);

    LCD_CHECK_RET(LCD_TE_Enable(FALSE));
	if(!DISP_IsLcmFound())
        return;

    {
        extern BOOTMODE g_boot_mode;
        printf("boot_mode = %d\n",g_boot_mode);
        if(g_boot_mode == META_BOOT)
            return;
    }

    if (LCM_DBI_TE_MODE_DISABLED == dbi->te_mode) {
        LCD_CHECK_RET(LCD_TE_Enable(FALSE));
        return;
    }

    if (LCM_DBI_TE_MODE_VSYNC_ONLY == dbi->te_mode) {
        LCD_CHECK_RET(LCD_TE_SetMode(LCD_TE_MODE_VSYNC_ONLY));
    } else if (LCM_DBI_TE_MODE_VSYNC_OR_HSYNC == dbi->te_mode) {
        LCD_CHECK_RET(LCD_TE_SetMode(LCD_TE_MODE_VSYNC_OR_HSYNC));
        LCD_CHECK_RET(LCD_TE_ConfigVHSyncMode(dbi->te_hs_delay_cnt,
                                              dbi->te_vs_width_cnt,
                     (LCD_TE_VS_WIDTH_CNT_DIV)dbi->te_vs_width_cnt_div));
    } else ASSERT(0);

    LCD_CHECK_RET(LCD_TE_SetEdgePolarity(dbi->te_edge_polarity));
    LCD_CHECK_RET(LCD_TE_Enable(TRUE));
}

__inline DPI_FB_FORMAT get_dsi_tmp_buffer_format(void)
{
    switch(lcm_params->dsi.data_format.format)
    {
    case LCM_DSI_FORMAT_RGB565 : return 0;
    case LCM_DSI_FORMAT_RGB666 :
    case LCM_DSI_FORMAT_RGB888 : return 1;
    default : ASSERT(0);
    }
    return 1;
}


__inline UINT32 get_dsi_tmp_buffer_bpp(void)
{
    static const UINT32 TO_BPP[] = {2, 3};
    return TO_BPP[get_dsi_tmp_buffer_format()];
}


__inline LCD_FB_FORMAT get_lcd_tmp_buffer_format(void)
{
    static const UINT32 TO_LCD_FORMAT[] = {
        LCD_FB_FORMAT_RGB565,
        LCD_FB_FORMAT_RGB888
    };
    
    return TO_LCD_FORMAT[get_dsi_tmp_buffer_format()];
}

static BOOL disp_drv_dsi_init_context(void)
{
    if (lcm_drv != NULL && lcm_params != NULL)
    {
        dsiTmpBufBpp=get_dsi_tmp_buffer_bpp();
        return TRUE;
    }

    if (NULL == lcm_drv) 
    {
        return FALSE;
    }

    lcm_drv->get_params(lcm_params);
    dsiTmpBufBpp=get_dsi_tmp_buffer_bpp();
    
    return TRUE;
}

void init_dsi(BOOL isDsiPoweredOn)
{
    DSI_PHY_clk_setting(lcm_params);

    DSI_CHECK_RET(DSI_Init(isDsiPoweredOn));

    if(1 == lcm_params->dsi.compatibility_for_nvk)
        DSI_CHECK_RET(DSI_TXRX_Control(TRUE,                    //cksm_en
                                       TRUE,                    //ecc_en
                                       lcm_params->dsi.LANE_NUM, //ecc_en
                                       0,                       //vc_num
                                       FALSE,                   //null_packet_en
                                       FALSE,                   //err_correction_en
                                       FALSE,                   //dis_eotp_en
                                       FALSE,
                                       0));                     //max_return_size
    else
        DSI_CHECK_RET(DSI_TXRX_Control(TRUE,                    //cksm_en
                                       TRUE,                    //ecc_en
                                       lcm_params->dsi.LANE_NUM, //ecc_en
                                       0,                       //vc_num
                                       FALSE,                   //null_packet_en
                                       FALSE,                   //err_correction_en
                                       FALSE,                   //dis_eotp_en
                                       (BOOL)!lcm_params->dsi.cont_clock,
                                       0));                     //max_return_size
    
    //initialize DSI_PHY
    DSI_PHY_clk_switch(TRUE, lcm_params);
    DSI_PHY_TIMCONFIG(lcm_params);
    DSI_CHECK_RET(DSI_PS_Control(lcm_params->dsi.PS, lcm_params->height, lcm_params->width * dsiTmpBufBpp));


	if(lcm_params->dsi.mode != CMD_MODE)
	{
		DSI_Set_VM_CMD(lcm_params);
		DSI_Config_VDO_Timing(lcm_params);
    }
}

// ---------------------------------------------------------------------------
//  DBI Display Driver Public Functions
// ---------------------------------------------------------------------------

BOOL DDMS_capturing=0;

static DISP_STATUS dsi_init(UINT32 fbVA, UINT32 fbPA, BOOL isLcmInited)
{
    if (!disp_drv_dsi_init_context()) 
        return DISP_STATUS_NOT_IMPLEMENTED;
    
    if(lcm_params->dsi.mode == CMD_MODE) {
        init_dsi(isLcmInited);
        mdelay(1);
        
        if (NULL != lcm_drv->init && !isLcmInited) 
        {
            lcm_drv->init();
            DSI_LP_Reset();
        }
        DSI_clk_HS_mode(1);
        DSI_SetMode(lcm_params->dsi.mode);
    }
    else {
        init_dsi(isLcmInited);
        mdelay(1);
        
        if (NULL != lcm_drv->init && !isLcmInited) {
            lcm_drv->init();
            DSI_LP_Reset();
        }
        
        DSI_SetMode(lcm_params->dsi.mode);
    }

    {
        struct disp_path_config_struct config;

        memset((void *)&config, 0, sizeof(struct disp_path_config_struct));

        config.srcModule = DISP_MODULE_OVL;
        
        if(config.srcModule == DISP_MODULE_RDMA0)
        {
            config.inFormat = RDMA_INPUT_FORMAT_RGB565;
            config.addr = fbPA; 
            config.pitch = DISP_GetScreenWidth()*2;
            config.srcROI.x = 0;config.srcROI.y = 0;
            config.srcROI.height= DISP_GetScreenHeight();config.srcROI.width= DISP_GetScreenWidth();
        }
        else
        {			
            config.bgROI.x = 0;
            config.bgROI.y = 0;
            config.bgROI.width = DISP_GetScreenWidth();
            config.bgROI.height = DISP_GetScreenHeight();
            config.bgColor = 0x0;	// background color
            
            config.pitch = DISP_GetScreenWidth()*2;
            config.srcROI.x = 0;config.srcROI.y = 0;
            config.srcROI.height= DISP_GetScreenHeight();config.srcROI.width= DISP_GetScreenWidth();
            config.ovl_config.source = OVL_LAYER_SOURCE_MEM; 
            
            {
                config.ovl_config.layer = 2;
                config.ovl_config.layer_en = 1; 
                config.ovl_config.fmt = OVL_INPUT_FORMAT_RGB565;
                config.ovl_config.addr = fbPA;	
                config.ovl_config.source = OVL_LAYER_SOURCE_MEM; 
                config.ovl_config.x = 0;	   // ROI
                config.ovl_config.y = 0;  
                config.ovl_config.w = DISP_GetScreenWidth();  
                config.ovl_config.h = DISP_GetScreenHeight();  
                config.ovl_config.pitch = (ALIGN_TO(DISP_GetScreenWidth(), MTK_FB_ALIGNMENT)) * 2; //pixel number
                config.ovl_config.keyEn = 0;
                config.ovl_config.key = 0xFF;	   // color key
                config.ovl_config.aen = 0;			  // alpha enable
                config.ovl_config.alpha = 0;			
            }
        }

        if(lcm_params->dsi.mode == CMD_MODE)
            config.dstModule = DISP_MODULE_DSI_CMD;// DISP_MODULE_WDMA1
        else
            config.dstModule = DISP_MODULE_DSI_VDO;// DISP_MODULE_WDMA1
        
        if(config.dstModule == DISP_MODULE_DSI_CMD || config.dstModule == DISP_MODULE_DSI_VDO)
            config.outFormat = RDMA_OUTPUT_FORMAT_ARGB; 
        else
            config.outFormat = WDMA_OUTPUT_FORMAT_ARGB; 		

        if(lcm_params->dsi.mode != CMD_MODE)
            disp_path_get_mutex();

        disp_path_config(&config);

        disp_bls_init(DISP_GetScreenWidth(), DISP_GetScreenHeight());

        if(lcm_params->dsi.mode != CMD_MODE)
            disp_path_release_mutex();
    }

    return DISP_STATUS_OK;
}


// protected by sem_early_suspend, sem_update_screen
static DISP_STATUS dsi_enable_power(BOOL enable)
{
    disp_drv_dsi_init_context();

    if(lcm_params->dsi.mode == CMD_MODE) 
    {
        if (enable) 
        {
            DSI_PHY_clk_switch(TRUE, lcm_params); 
            DSI_PHY_clk_setting(lcm_params);
            DSI_CHECK_RET(DSI_PowerOn());
            DSI_clk_ULP_mode(0);			
            DSI_lane0_ULP_mode(0);
            DSI_clk_HS_mode(1);	
        }
        else 
        {
            DSI_clk_HS_mode(0);
            DSI_lane0_ULP_mode(1);
            DSI_clk_ULP_mode(1);
            DSI_PHY_clk_switch(FALSE, lcm_params);
            DSI_CHECK_RET(DSI_PowerOff());
            //DSI_PHY_clk_switch(FALSE, lcm_params);
        }
    }
    else 
    {
        if (enable) 
        {
            needStartDSI = TRUE;
            DSI_PHY_clk_switch(TRUE, lcm_params); 
            DSI_PHY_clk_setting(lcm_params);
            DSI_CHECK_RET(DSI_PowerOn());
            DSI_clk_ULP_mode(0);			
            DSI_lane0_ULP_mode(0);
            DSI_clk_HS_mode(1);	
        }
        else 
        {
            DSI_clk_HS_mode(0);
            DSI_lane0_ULP_mode(1);
            DSI_clk_ULP_mode(1);	
            DSI_PHY_clk_switch(FALSE, lcm_params);
            DSI_CHECK_RET(DSI_PowerOff());
            //DSI_PHY_clk_switch(FALSE, lcm_params);
        }
    }
    
    return DISP_STATUS_OK;
}


// protected by sem_flipping, sem_early_suspend, sem_overlay_buffer, sem_update_screen
static DISP_STATUS dsi_update_screen(void)
{
    disp_drv_dsi_init_context();

    //DSI_CHECK_RET(DSI_handle_TE());
    
    DSI_SetMode(lcm_params->dsi.mode);

    if (lcm_params->type==LCM_TYPE_DSI && lcm_params->dsi.mode == CMD_MODE && !DDMS_capturing) {
        DSI_clk_HS_mode(1);
        DSI_CHECK_RET(DSI_StartTransfer(needStartDSI));
    }
    else if (lcm_params->type==LCM_TYPE_DSI && lcm_params->dsi.mode != CMD_MODE && !DDMS_capturing)
    {
        DSI_clk_HS_mode(1);
        DSI_CHECK_RET(DSI_StartTransfer(needStartDSI));
        if(needStartDSI)
            needStartDSI = FALSE;
    }
    
    if (DDMS_capturing)
        DISP_LOG_PRINT(ANDROID_LOG_INFO, "DSI", "[DISP] kernel - dsi_update_screen. DDMS is capturing. Skip one frame. \n");		
    
    return DISP_STATUS_OK;
}


static UINT32 dsi_get_working_buffer_size(void)
{
    return 0;
}

static UINT32 dsi_get_working_buffer_bpp(void)
{
    return 0;
}

static PANEL_COLOR_FORMAT dsi_get_panel_color_format(void)
{
    disp_drv_dsi_init_context();

    switch(lcm_params->dsi.data_format.format)
    {
        case LCM_DSI_FORMAT_RGB565 : return PANEL_COLOR_FORMAT_RGB565;
        case LCM_DSI_FORMAT_RGB666 : return PANEL_COLOR_FORMAT_RGB666;
        case LCM_DSI_FORMAT_RGB888 : return PANEL_COLOR_FORMAT_RGB888;
        default : ASSERT(0);
    }
}

static UINT32 dsi_get_dithering_bpp(void)
{
    return PANEL_COLOR_FORMAT_TO_BPP(dsi_get_panel_color_format());
}

DISP_STATUS dsi_capture_framebuffer(UINT32 pvbuf, UINT32 bpp)
{
    return DISP_STATUS_OK;	
}


// called by "esd_recovery_kthread"
// protected by sem_early_suspend, sem_update_screen
BOOL dsi_esd_check(void)
{
    BOOL result = FALSE;
    
    if(lcm_params->dsi.mode == CMD_MODE || !dsi_vdo_streaming)
        return FALSE;

    return result;
}


// called by "esd_recovery_kthread"
// protected by sem_early_suspend, sem_update_screen
void dsi_esd_reset(void)
{
    /// we assume the power is on here
    ///  what we need is some setting for LCM init
    if (lcm_params->dsi.mode == CMD_MODE)
    {
        DSI_clk_HS_mode(0);
        DSI_clk_ULP_mode(0);
        DSI_lane0_ULP_mode(0);
    }
    else
    {
        DSI_SetMode(CMD_MODE);
        DSI_clk_HS_mode(0);
        // clock/data lane go to Ideal
        DSI_Reset();
    }
}

const DISP_DRIVER *DISP_GetDriverDSI()
{
    static const DISP_DRIVER DSI_DISP_DRV =
    {
        .init                   = dsi_init,
        .enable_power           = dsi_enable_power,
        .update_screen          = dsi_update_screen,       
        .get_working_buffer_size = dsi_get_working_buffer_size,

        .get_panel_color_format = dsi_get_panel_color_format,
        .get_working_buffer_bpp = dsi_get_working_buffer_bpp,
        .init_te_control        = init_lcd_te_control,
        .get_dithering_bpp	= dsi_get_dithering_bpp,
        .capture_framebuffer	= dsi_capture_framebuffer,
        .esd_reset              = dsi_esd_reset,
        .esd_check				= dsi_esd_check,
    };

    return &DSI_DISP_DRV;
}

