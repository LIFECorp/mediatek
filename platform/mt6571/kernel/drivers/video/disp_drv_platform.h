#ifndef __DISP_DRV_PLATFORM_H__
#define __DISP_DRV_PLATFORM_H__

#include <linux/dma-mapping.h>
#include <linux/disp_assert_layer.h>
#include <mach/mt_typedefs.h>
#include <mach/mt_gpio.h>
#include <mach/mt_reg_base.h>
#include <mach/mt_clkmgr.h>
#include <mach/mt_irq.h>

///LCD HW feature options
//#define MTK_LCD_HW_3D_SUPPORT

#define ALIGN_TO(x, n)   (((x) + ((n) - 1)) & ~((n) - 1))

#define MTK_FB_ALIGNMENT 16
#define MTK_FB_SYNC_SUPPORT
#if !defined(MTK_LCA_RAM_OPTIMIZE)
    #define MTK_OVL_DECOUPLE_SUPPORT
#endif

#define MTK_HDMI_MAIN_PATH 0

#endif //__DISP_DRV_PLATFORM_H__
