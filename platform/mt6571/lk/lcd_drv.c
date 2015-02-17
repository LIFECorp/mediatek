/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2008
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/
#define ENABLE_LCD_INTERRUPT 0 

#include <string.h>
#include <platform/mt_clkmgr.h>
#include <platform/mt_gpt.h>
#include <platform/disp_drv_platform.h>
#include <platform/ddp_reg.h>
#include <platform/ddp_ovl.h>
#include <platform/ddp_path.h>
#include <platform/sync_write.h>
#include <platform/lcd_reg.h>
#include "disp_drv_log.h"

#ifdef OUTREG32
  #undef OUTREG32
  #define OUTREG32(x, y) mt65xx_reg_sync_writel(y, x)
#endif

#ifdef OUTREG16
  #undef OUTREG16
  #define OUTREG16(x, y) mt65xx_reg_sync_writew(y, x)
#endif

#ifdef OUTREG8
  #undef OUTREG8
  #define OUTREG8(x, y) mt65xx_reg_sync_writeb(y, x)
#endif

#ifndef OUTREGBIT
#define OUTREGBIT(TYPE,REG,bit,value)  \
                    do {    \
                        TYPE r = *((TYPE*)&INREG32(&REG));    \
                        r.bit = value;    \
                        OUTREG32(&REG, AS_UINT32(&r));    \
                    } while (0)
#endif

#define LCD_OUTREG32_R(type, addr2, addr1) DISP_OUTREG32_R(type, addr2, addr1)
#define LCD_OUTREG32_V(type, addr2, var)   DISP_OUTREG32_V(type, addr2, var)
#define LCD_OUTREG16_R(type, addr2, addr1) DISP_OUTREG16_R(type, addr2, addr1)
#define LCD_MASKREG32_T(type, addr, mask, data)	 DISP_MASKREG32_T(type, addr, mask, data)
#define LCD_OUTREGBIT(TYPE,REG,bit,value) DISP_OUTREGBIT(TYPE,REG,bit,value)
#define LCD_INREG32(type,addr) DISP_INREG32(type,addr)

#define LCD_OUTREG32(addr, data)	\
		{\
		OUTREG32(addr, data);}

#define LCD_OUTREG8(addr, data)	\
		{\
		OUTREG8(addr, data);}


#define LCD_OUTREG16(addr, data)	\
		{\
		OUTREG16(addr, data);}

#define LCD_MASKREG32(addr, mask, data)	\
		{\
		MASKREG32(addr, mask, data);}

#define DBI_CG_CTRL0        ( DBI_BCLK_SW_CG_BIT \
                            | DBI_PAD0_SW_CG_BIT \
                            | DBI_PAD1_SW_CG_BIT \
                            | DBI_PAD2_SW_CG_BIT \
                            | DBI_PAD3_SW_CG_BIT )

#define DBI_MMSYS_CG1       ( DISP_DBI_ENGINE_SW_CG_BIT \
                            | DISP_DBI_IF_SW_CG_BIT )

#define LCD_IODRV_REG_COMMON            (IO_CFG_RIGHT_BASE + 0x060)
#define LCD_IODRV_REG_MSBDATA           (IO_CFG_TOP_BASE + 0x110)
#define LCD_IODRV_REG_MSBDATA_NFI       (IO_CFG_BOTTOM_BASE + 0x060)
#define LCD_IODRV_REG_SPI               (IO_CFG_TOP_BASE + 0x0F0)
#define MIPI_TX_CONFIG_REG_DSI_GPIO_EN  (MIPI_TX_CONFIG_BASE + 0x074)

#define GPIO_REG_GPIO_MODE7             (GPIO_BASE + 0x370)
#define GPIO_REG_GPIO_MODE8             (GPIO_BASE + 0x380)

#define GPIO_61_MODE                    ((DISP_REG_GET(GPIO_REG_GPIO_MODE7) >> 20) & 0x7)
#define GPIO_62_MODE                    ((DISP_REG_GET(GPIO_REG_GPIO_MODE7) >> 24) & 0x7)
#define GPIO_63_MODE                    ((DISP_REG_GET(GPIO_REG_GPIO_MODE7) >> 28) & 0x7)
#define GPIO_64_MODE                    ((DISP_REG_GET(GPIO_REG_GPIO_MODE8) >> 0) & 0x7)
#define GPIO_65_MODE                    ((DISP_REG_GET(GPIO_REG_GPIO_MODE8) >> 4) & 0x7)
#define GPIO_66_MODE                    ((DISP_REG_GET(GPIO_REG_GPIO_MODE8) >> 8) & 0x7)

#if defined(DISP_DRV_DBG)
    unsigned int dbi_log_on = 1;
#else
    unsigned int dbi_log_on = 0;
#endif


void dbi_log_enable(int enable)
{
   dbi_log_on = enable;
   DBI_LOG("lcd log %s\n", enable?"enabled":"disabled");
}

static PLCD_REGS const LCD_REG = (PLCD_REGS)(DISP_DBI_BASE);
static const UINT32 TO_BPP[LCD_FB_FORMAT_NUM] = {2, 3, 4};
typedef struct
{
   LCD_FB_FORMAT fbFormat;
   UINT32 fbPitchInBytes;
   LCD_REG_SIZE roiWndSize;
   LCD_OUTPUT_MODE outputMode;
   LCD_REGS regBackup;
   void (*pIntCallback)(DISP_INTERRUPT_EVENTS);
} LCD_CONTEXT;

static LCD_CONTEXT _lcdContext;
static int wst_step_LCD = -1;//for LCD&FM de-sense
static BOOL is_get_default_write_cycle = FALSE;
static unsigned int default_write_cycle = 0;
static UINT32 default_wst = 0;
static OVL_CONFIG_STRUCT layer_config[4] = {{0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                                            {1,0,0,0,0,0,0,0,0,0,0,0,0,0},
                                            {2,0,0,0,0,0,0,0,0,0,0,0,0,0},
                                            {3,0,0,0,0,0,0,0,0,0,0,0,0,0}};

// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------
LCD_STATUS LCD_Init_IO_pad(LCM_PARAMS *lcm_params)
{
    if (lcm_params->type == LCM_TYPE_DSI)
    {
        MASKREG32(MIPI_TX_CONFIG_REG_DSI_GPIO_EN, 0x00001000, 0x00000000);
    }
    else
    {
        MASKREG32(MIPI_TX_CONFIG_REG_DSI_GPIO_EN, 0x00001000, 0x00001000);

        if ((GPIO_61_MODE == GPIO_62_MODE) &&
            (GPIO_62_MODE == GPIO_63_MODE) &&
            (GPIO_63_MODE == GPIO_64_MODE) &&
            (GPIO_64_MODE == GPIO_65_MODE) &&
            (GPIO_65_MODE == GPIO_66_MODE)) // NFI[6:1]
        {
            if (GPIO_61_MODE == GPIO_MODE_05) // LPD[23:18]
                MASKREG32(DISP_REG_CONFIG_DBI_DPI_IO_SEL, 0x00000001, 0x00000001);
            else
                MASKREG32(DISP_REG_CONFIG_DBI_DPI_IO_SEL, 0x00000001, 0x00000000);

            if (GPIO_61_MODE == GPIO_MODE_06) // DPI_R[1:0], DPI_G[1:0], DPI_B[1:0]
                MASKREG32(DISP_REG_CONFIG_DBI_DPI_IO_SEL, 0x00000002, 0x00000002);
            else
                MASKREG32(DISP_REG_CONFIG_DBI_DPI_IO_SEL, 0x00000002, 0x00000000);
        }
    }

    return LCD_STATUS_OK;
}


LCD_STATUS LCD_Set_DrivingCurrent(LCM_PARAMS *lcm_params)
{
    if (lcm_params->type == LCM_TYPE_DBI)
    {
        switch (lcm_params->dbi.io_driving_current)
        {
            case LCM_DRIVING_CURRENT_4MA :
                MASKREG32(LCD_IODRV_REG_COMMON, 0x00000333, 0x00000000);
                break;
            case LCM_DRIVING_CURRENT_8MA :
            case LCM_DRIVING_CURRENT_DEFAULT :
                MASKREG32(LCD_IODRV_REG_COMMON, 0x00000333, 0x00000111);
                break;
            case LCM_DRIVING_CURRENT_12MA :
                MASKREG32(LCD_IODRV_REG_COMMON, 0x00000333, 0x00000222);
                break;
            case LCM_DRIVING_CURRENT_16MA :
                MASKREG32(LCD_IODRV_REG_COMMON, 0x00000333, 0x00000333);
                break;
            default :
                MASKREG32(LCD_IODRV_REG_COMMON, 0x00000333, 0x00000111);
                printf("[WARNING] Driving current settings incorrect!\n");
                printf("dbi.io_driving_current must be 4 mA / 8 mA / 12 mA / 16 mA.\n");
                printf("Set to default (8 mA).\n");
                break;
        }

        if (lcm_params->dbi.data_width == LCM_DBI_DATA_WIDTH_24BITS)
        {
            if (GPIO_61_MODE == GPIO_MODE_05)
            {
                switch (lcm_params->dbi.msb_io_driving_current)
                {
                    case LCM_DRIVING_CURRENT_2MA :
                        MASKREG32(LCD_IODRV_REG_MSBDATA_NFI, 0x00003000, 0x00000000);
                        break;
                    case LCM_DRIVING_CURRENT_4MA :
                    case LCM_DRIVING_CURRENT_DEFAULT :
                        MASKREG32(LCD_IODRV_REG_MSBDATA_NFI, 0x00003000, 0x00001000);
                        break;
                    case LCM_DRIVING_CURRENT_6MA :
                        MASKREG32(LCD_IODRV_REG_MSBDATA_NFI, 0x00003000, 0x00002000);
                        break;
                    case LCM_DRIVING_CURRENT_8MA :
                        MASKREG32(LCD_IODRV_REG_MSBDATA_NFI, 0x00003000, 0x00003000);
                        break;
                    default :
                        MASKREG32(LCD_IODRV_REG_MSBDATA_NFI, 0x00003000, 0x00001000);
                        printf("[WARNING] Driving current settings incorrect!\n");
                        printf("dbi.msb_io_driving_current must be 2 mA / 4 mA / 6 mA / 8 mA.\n");
                        printf("Set to default (4 mA).\n");
                        break;
                }
            }
            else
            {
                switch (lcm_params->dbi.msb_io_driving_current)
                {
                    case LCM_DRIVING_CURRENT_2MA :
                        MASKREG32(LCD_IODRV_REG_MSBDATA, 0x00780000, 0x00000000);
                        break;
                    case LCM_DRIVING_CURRENT_4MA :
                    case LCM_DRIVING_CURRENT_DEFAULT :
                        MASKREG32(LCD_IODRV_REG_MSBDATA, 0x00780000, 0x00280000);
                        break;
                    case LCM_DRIVING_CURRENT_6MA :
                        MASKREG32(LCD_IODRV_REG_MSBDATA, 0x00780000, 0x00500000);
                        break;
                    case LCM_DRIVING_CURRENT_8MA :
                        MASKREG32(LCD_IODRV_REG_MSBDATA, 0x00780000, 0x00780000);
                        break;
                    default :
                        MASKREG32(LCD_IODRV_REG_MSBDATA, 0x00780000, 0x00280000);
                        printf("[WARNING] Driving current settings incorrect!\n");
                        printf("dbi.msb_io_driving_current must be 2 mA / 4 mA / 6 mA / 8 mA.\n");
                        printf("Set to default (4 mA).\n");
                        break;
                }
            }
        }
    }

    if (lcm_params->type == LCM_TYPE_DPI)
    {
        switch (lcm_params->dpi.io_driving_current)
        {
            case LCM_DRIVING_CURRENT_4MA :
                MASKREG32(LCD_IODRV_REG_COMMON, 0x00000333, 0x00000000);
                break;
            case LCM_DRIVING_CURRENT_8MA :
            case LCM_DRIVING_CURRENT_DEFAULT :
                MASKREG32(LCD_IODRV_REG_COMMON, 0x00000333, 0x00000111);
                break;
            case LCM_DRIVING_CURRENT_12MA :
                MASKREG32(LCD_IODRV_REG_COMMON, 0x00000333, 0x00000222);
                break;
            case LCM_DRIVING_CURRENT_16MA :
                MASKREG32(LCD_IODRV_REG_COMMON, 0x00000333, 0x00000333);
                break;
            default :
                MASKREG32(LCD_IODRV_REG_COMMON, 0x00000333, 0x00000111);
                printf("[WARNING] Driving current settings incorrect!\n");
                printf("dpi.io_driving_current must be 4 mA / 8 mA / 12 mA / 16 mA.\n");
                printf("Set to default (8 mA).\n");
                break;
        }

        if (lcm_params->ctrl == LCM_CTRL_SERIAL_DBI)
        {
            switch (lcm_params->dbi.io_driving_current)
            {
                case LCM_DRIVING_CURRENT_2MA :
                    MASKREG32(LCD_IODRV_REG_SPI, 0x00000003, 0x00000000);
                    break;
                case LCM_DRIVING_CURRENT_4MA :
                case LCM_DRIVING_CURRENT_DEFAULT :
                    MASKREG32(LCD_IODRV_REG_SPI, 0x00000003, 0x00000001);
                    break;
                case LCM_DRIVING_CURRENT_6MA :
                    MASKREG32(LCD_IODRV_REG_SPI, 0x00000003, 0x00000002);
                    break;
                case LCM_DRIVING_CURRENT_8MA :
                    MASKREG32(LCD_IODRV_REG_SPI, 0x00000003, 0x00000003);
                    break;
                default :
                    MASKREG32(LCD_IODRV_REG_SPI, 0x00000003, 0x00000001);
                    printf("[WARNING] Driving current settings incorrect!\n");
                    printf("dbi.io_driving_current must be 2 mA / 4 mA / 6 mA / 8 mA.\n");
                    printf("Set to default (4 mA).\n");
                    break;
            }
        }

        if (lcm_params->dpi.format == LCM_DPI_FORMAT_RGB888)
        {
            if (GPIO_61_MODE == GPIO_MODE_06)
            {
                switch (lcm_params->dpi.lsb_io_driving_current)
                {
                    case LCM_DRIVING_CURRENT_2MA :
                        MASKREG32(LCD_IODRV_REG_MSBDATA_NFI, 0x00003000, 0x00000000);
                        break;
                    case LCM_DRIVING_CURRENT_4MA :
                    case LCM_DRIVING_CURRENT_DEFAULT :
                        MASKREG32(LCD_IODRV_REG_MSBDATA_NFI, 0x00003000, 0x00001000);
                        break;
                    case LCM_DRIVING_CURRENT_6MA :
                        MASKREG32(LCD_IODRV_REG_MSBDATA_NFI, 0x00003000, 0x00002000);
                        break;
                    case LCM_DRIVING_CURRENT_8MA :
                        MASKREG32(LCD_IODRV_REG_MSBDATA_NFI, 0x00003000, 0x00003000);
                        break;
                    default :
                        MASKREG32(LCD_IODRV_REG_MSBDATA_NFI, 0x00003000, 0x00001000);
                        printf("[WARNING] Driving current settings incorrect!\n");
                        printf("dpi.lsb_io_driving_current must be 2 mA / 4 mA / 6 mA / 8 mA.\n");
                        printf("Set to default (4 mA).\n");
                        break;
                }
            }
            else
            {
                switch (lcm_params->dpi.lsb_io_driving_current)
                {
                    case LCM_DRIVING_CURRENT_2MA :
                        MASKREG32(LCD_IODRV_REG_MSBDATA, 0x00780000, 0x00000000);
                        break;
                    case LCM_DRIVING_CURRENT_4MA :
                    case LCM_DRIVING_CURRENT_DEFAULT :
                        MASKREG32(LCD_IODRV_REG_MSBDATA, 0x00780000, 0x00280000);
                        break;
                    case LCM_DRIVING_CURRENT_6MA :
                        MASKREG32(LCD_IODRV_REG_MSBDATA, 0x00780000, 0x00500000);
                        break;
                    case LCM_DRIVING_CURRENT_8MA :
                        MASKREG32(LCD_IODRV_REG_MSBDATA, 0x00780000, 0x00780000);
                        break;
                    default :
                        MASKREG32(LCD_IODRV_REG_MSBDATA, 0x00780000, 0x00280000);
                        printf("[WARNING] Driving current settings incorrect!\n");
                        printf("dpi.lsb_io_driving_current must be 2 mA / 4 mA / 6 mA / 8 mA.\n");
                        printf("Set to default (4 mA).\n");
                        break;
                }
            }
        }
    }

    if (lcm_params->type == LCM_TYPE_DSI)
    {
        MASKREG32(LCD_IODRV_REG_COMMON, 0x00000303, 0x00000000);
    }

    return LCD_STATUS_OK;
}


#if ENABLE_LCD_INTERRUPT
static irqreturn_t _LCD_InterruptHandler(int irq, void *dev_id)
{   
   LCD_REG_INTERRUPT status = LCD_REG->INT_STATUS;
   
   if (status.COMPLETED)
   {
      #ifdef CONFIG_MTPROF_APPLAUNCH  // eng enable, user disable   	
         LOG_PRINT(ANDROID_LOG_INFO, "AppLaunch", "LCD frame buffer update done !\n");
      #endif
      wake_up_interruptible(&_lcd_wait_queue);
      
      if(_lcdContext.pIntCallback)
         _lcdContext.pIntCallback(DISP_LCD_TRANSFER_COMPLETE_INT);
      
      DBG_OnLcdDone();
   }
   
   if (status.SYNC)// this is TE mode 0 interrupt
   {
      if(_lcdContext.pIntCallback)
         _lcdContext.pIntCallback(DISP_LCD_SYNC_INT);

      DBG_OnTeDelayDone();
   }

   LCD_OUTREG32(&LCD_REG->INT_STATUS, 0);

   return IRQ_HANDLED;
}
#endif


static BOOL _IsEngineBusy(void)
{
   LCD_REG_STATUS status;
   
   status = LCD_REG->STATUS;
   if (status.BUSY) 
      return TRUE;
   
   return FALSE;
}


BOOL LCD_IsBusy(void)
{
   return _IsEngineBusy();
}

static void _WaitForLCDEngineComplete(void)
{
   unsigned int TimeCount=0;  
   
   do
   {
      if (LCD_INREG32(PLCD_REG_INTERRUPT, &LCD_REG->INT_STATUS)&0x1)
      {
         break;
      }
      mdelay(10);
      TimeCount++;
      if(1000000==TimeCount)
      {
         printf("[WARNING] Wait for LCD engine complete timeout!!!\n");
         
         if(LCD_INREG32(PLCD_REG_INTERRUPT, &LCD_REG->STATUS)&0x8)
         {
            printf("[WARNING] reason is LCD can't wait TE signal!!!\n");
            LCD_TE_Enable(FALSE);
            
            //SW reset
            LCD_OUTREG32_V(PLCD_REG_START,&LCD_REG->START, 0);
            LCD_OUTREG32_V(PLCD_REG_START,&LCD_REG->START, 1);
            //Re-Start
            LCD_OUTREG32_V(PLCD_REG_START,&LCD_REG->START, 0);
            LCD_OUTREG32_V(PLCD_REG_START,&LCD_REG->START, (1 << 15));
            TimeCount=0;
         }
         else
         {
            printf("[ERROR] for unknown reason!!!\n");
            LCD_DumpRegisters();
            
            //SW reset
            LCD_OUTREG32_V(PLCD_REG_START,&LCD_REG->START, 0);
            LCD_OUTREG32_V(PLCD_REG_START,&LCD_REG->START, 1);
            TimeCount=0;
            break;
         }        
      }
   } while(1);
  
   TimeCount=0;
}


static void _WaitForEngineNotBusy(void)
{
   unsigned int cnt = 0;
#if ENABLE_LCD_INTERRUPT
   static const long WAIT_TIMEOUT = 2 * HZ;    // 2 sec

   if (in_interrupt())
   {
      // perform busy waiting if in interrupt context

      while (_IsEngineBusy())
      {
         if (cnt > 2000)
         {
             break;
         }
         mdelay(1);
         cnt ++;
      }
   }
   else
   {
      long ret = wait_event_interruptible_timeout(_lcd_wait_queue, !_IsEngineBusy(), WAIT_TIMEOUT);
      if (0 == ret)
      {
         DISP_LOG_PRINT(ANDROID_LOG_WARN, "LCD", "[WARNING] Wait for LCD engine not busy timeout!!!\n"); 
         LCD_DumpRegisters();

         if (LCD_REG->STATUS.WAIT_SYNC)
         {
            DISP_LOG_PRINT(ANDROID_LOG_WARN, "LCD", "reason is LCD can't wait TE signal!!!\n"); 
            LCD_TE_Enable(FALSE);
         }

         OUTREG16(&LCD_REG->START, 0);
         OUTREG16(&LCD_REG->START, 0x1);
      }
   }
#else
   while (_IsEngineBusy())
   {
      if (cnt > 2000)
      {
         break;
      }
      mdelay(1);
      cnt ++;
   }

   if (cnt > 2000)
   {
      printf("[WARNING] Wait for LCD engine not busy timeout!!!\n");
      LCD_DumpRegisters();

      if (LCD_REG->STATUS.WAIT_SYNC)
      {
         printf("reason is LCD can't wait TE signal!!!\n");
         LCD_TE_Enable(FALSE);
         return;
      }

      LCD_OUTREG32_V(PLCD_REG_START,&LCD_REG->START, 0);
      LCD_OUTREG32_V(PLCD_REG_START,&LCD_REG->START, 0x1);
   }

   LCD_OUTREG32_V(PLCD_REG_INTERRUPT,&LCD_REG->INT_STATUS, 0x0);
#endif
}


unsigned int vsync_wait_time = 0;


void LCD_WaitTE(void)
{

}


void LCD_InitVSYNC(unsigned int vsync_interval)
{

}


void LCD_PauseVSYNC(BOOL enable)
{
}


static void LCD_BackupRegisters(void)
{
   LCD_REGS *regs = &(_lcdContext.regBackup);
   UINT32 i;

   LCD_OUTREG32_R(PLCD_REG_INTERRUPT,&regs->INT_ENABLE, &LCD_REG->INT_ENABLE);
   LCD_OUTREG32_R(PLCD_REG_SCNF,&regs->SERIAL_CFG, &LCD_REG->SERIAL_CFG);
   
   for(i = 0; i < ARY_SIZE(LCD_REG->SIF_TIMING); ++i)
   {
      LCD_OUTREG32_R(PLCD_REG_SIF_TIMING, &regs->SIF_TIMING[i], &LCD_REG->SIF_TIMING[i]);
   }
   
   for(i = 0; i < ARY_SIZE(LCD_REG->PARALLEL_CFG); ++i)
   {
      LCD_OUTREG32_R(PLCD_REG_PCNF, &regs->PARALLEL_CFG[i], &LCD_REG->PARALLEL_CFG[i]);
   }
   
   LCD_OUTREG32_R(PLCD_REG_TECON,&regs->TEARING_CFG, &LCD_REG->TEARING_CFG);
   LCD_OUTREG32_R(PLCD_REG_PCNFDW,&regs->PARALLEL_DW, &LCD_REG->PARALLEL_DW);
   LCD_OUTREG32_R(PLCD_REG_CALC_HTT,&regs->CALC_HTT, &LCD_REG->CALC_HTT);
   LCD_OUTREG32_R(PLCD_REG_SYNC_LCM_SIZE,&regs->SYNC_LCM_SIZE, &LCD_REG->SYNC_LCM_SIZE);
   LCD_OUTREG32_R(PLCD_REG_SYNC_CNT,&regs->SYNC_CNT, &LCD_REG->SYNC_CNT);

   LCD_OUTREG32_R(PLCD_REG_WROI_CON,&regs->WROI_CONTROL, &LCD_REG->WROI_CONTROL);
   LCD_OUTREG32_R(PLCD_REG_CMD_ADDR,&regs->WROI_CMD_ADDR, &LCD_REG->WROI_CMD_ADDR);
   LCD_OUTREG32_R(PLCD_REG_DAT_ADDR,&regs->WROI_DATA_ADDR, &LCD_REG->WROI_DATA_ADDR);
   LCD_OUTREG32_R(PLCD_REG_SIZE,&regs->WROI_SIZE, &LCD_REG->WROI_SIZE);

   LCD_OUTREG32_R(PLCD_REG_ULTRA_CON,&regs->ULTRA_CON, &LCD_REG->ULTRA_CON);
   LCD_OUTREG32_R(PLCD_REG_DBI_ULTRA_TH,&regs->DBI_ULTRA_TH, &LCD_REG->DBI_ULTRA_TH);
}


static void LCD_RestoreRegisters(void)
{
   LCD_REGS *regs = &(_lcdContext.regBackup);
   UINT32 i;

   LCD_OUTREG32_R(PLCD_REG_INTERRUPT,&LCD_REG->INT_ENABLE, &regs->INT_ENABLE);
   LCD_OUTREG32_R(PLCD_REG_SCNF,&LCD_REG->SERIAL_CFG, &regs->SERIAL_CFG);
   
   for(i = 0; i < ARY_SIZE(LCD_REG->SIF_TIMING); ++i)
   {
      LCD_OUTREG32_R(PLCD_REG_SIF_TIMING, &LCD_REG->SIF_TIMING[i], &regs->SIF_TIMING[i]);
   }
   
   for(i = 0; i < ARY_SIZE(LCD_REG->PARALLEL_CFG); ++i)
   {
      LCD_OUTREG32_R(PLCD_REG_PCNF, &LCD_REG->PARALLEL_CFG[i], &regs->PARALLEL_CFG[i]);
   }
   
   LCD_OUTREG32_R(PLCD_REG_TECON,&LCD_REG->TEARING_CFG, &regs->TEARING_CFG);
   LCD_OUTREG32_R(PLCD_REG_PCNFDW,&LCD_REG->PARALLEL_DW, &regs->PARALLEL_DW);
   LCD_OUTREG32_R(PLCD_REG_CALC_HTT,&LCD_REG->CALC_HTT, &regs->CALC_HTT);
   LCD_OUTREG32_R(PLCD_REG_SYNC_LCM_SIZE,&LCD_REG->SYNC_LCM_SIZE, &regs->SYNC_LCM_SIZE);
   LCD_OUTREG32_R(PLCD_REG_SYNC_CNT,&LCD_REG->SYNC_CNT, &regs->SYNC_CNT);

   LCD_OUTREG32_R(PLCD_REG_WROI_CON,&LCD_REG->WROI_CONTROL, &regs->WROI_CONTROL);
   LCD_OUTREG32_R(PLCD_REG_CMD_ADDR,&LCD_REG->WROI_CMD_ADDR, &regs->WROI_CMD_ADDR);
   LCD_OUTREG32_R(PLCD_REG_DAT_ADDR,&LCD_REG->WROI_DATA_ADDR, &regs->WROI_DATA_ADDR);
   LCD_OUTREG32_R(PLCD_REG_SIZE,&LCD_REG->WROI_SIZE, &regs->WROI_SIZE);

   LCD_OUTREG32_R(PLCD_REG_ULTRA_CON,&LCD_REG->ULTRA_CON, &regs->ULTRA_CON);
   LCD_OUTREG32_R(PLCD_REG_DBI_ULTRA_TH,&LCD_REG->DBI_ULTRA_TH, &regs->DBI_ULTRA_TH);
}


static void _ResetBackupedLCDRegisterValues(void)
{
   LCD_REGS *regs = &_lcdContext.regBackup;

   memset((void*)regs, 0, sizeof(LCD_REGS));
}


// ---------------------------------------------------------------------------
//  LCD Controller API Implementations
// ---------------------------------------------------------------------------
LCD_STATUS LCD_Init(void)
{
   LCD_STATUS ret = LCD_STATUS_OK;
   
   memset(&_lcdContext, 0, sizeof(_lcdContext));
   
   // LCD controller would NOT reset register as default values
   // Do it by SW here
   //
   _ResetBackupedLCDRegisterValues();
   
   ret = LCD_PowerOn();
   
   LCD_OUTREG32_V(PLCD_REG_SYNC_LCM_SIZE,&LCD_REG->SYNC_LCM_SIZE, 0x00010001);
   LCD_OUTREG32_V(PLCD_REG_SYNC_CNT,&LCD_REG->SYNC_CNT, 0x1);
   
   ASSERT(ret == LCD_STATUS_OK);
   #if ENABLE_LCD_INTERRUPT
      if (request_irq(MT65XX_LCD_IRQ_ID,
          _LCD_InterruptHandler, IRQF_TRIGGER_LOW, "mtklcd", NULL) < 0)
      {
         DBI_LOG("[LCD][ERROR] fail to request LCD irq\n"); 
         return LCD_STATUS_ERROR;
      }

      //	mt65xx_irq_unmask(MT65XX_LCD_IRQ_ID);
      //	enable_irq(MT65XX_LCD_IRQ_ID);
      init_waitqueue_head(&_lcd_wait_queue);
      init_waitqueue_head(&_vsync_wait_queue);
      OUTREGBIT(LCD_REG_INTERRUPT,LCD_REG->INT_ENABLE,COMPLETED,1);
      OUTREGBIT(LCD_REG_INTERRUPT,LCD_REG->INT_ENABLE,CMDQ_COMPLETED,1);
      OUTREGBIT(LCD_REG_INTERRUPT,LCD_REG->INT_ENABLE,HTT,1);
      OUTREGBIT(LCD_REG_INTERRUPT,LCD_REG->INT_ENABLE,SYNC,1);
   #endif
   
   return LCD_STATUS_OK;
}


LCD_STATUS LCD_Deinit(void)
{
   LCD_STATUS ret = LCD_PowerOff();
   
   ASSERT(ret == LCD_STATUS_OK);
   return LCD_STATUS_OK;
}


static BOOL s_isLcdPowerOn = FALSE;


LCD_STATUS LCD_PowerOn(void)
{
    if (!s_isLcdPowerOn)
    {
        MASKREG32(CLR_CLK_GATING_CTRL0, DBI_CG_CTRL0, DBI_CG_CTRL0);
        MASKREG32(DISP_REG_CONFIG_MMSYS_CG_CLR1, DBI_MMSYS_CG1, DBI_MMSYS_CG1);

        DBI_LOG("[DISP] - LCD_PowerOn. 0x%8x, 0x%8x\n", INREG32(CLK_GATING_CTRL0), INREG32(DISP_REG_CONFIG_MMSYS_CG_CON1));
        LCD_RestoreRegisters();
        s_isLcdPowerOn = TRUE;
    }
   
    return LCD_STATUS_OK;
}


LCD_STATUS LCD_PowerOff(void)
{
    if (s_isLcdPowerOn)
    {
        LCD_BackupRegisters();
        MASKREG32(DISP_REG_CONFIG_MMSYS_CG_SET1, DBI_MMSYS_CG1, DBI_MMSYS_CG1);
        MASKREG32(SET_CLK_GATING_CTRL0, DBI_CG_CTRL0, DBI_CG_CTRL0);

        DBI_LOG("[DISP] - LCD_PowerOff. 0x%8x, 0x%8x\n", INREG32(CLK_GATING_CTRL0), INREG32(DISP_REG_CONFIG_MMSYS_CG_CON1));
        s_isLcdPowerOn = FALSE;
    }
   
    return LCD_STATUS_OK;
}


LCD_STATUS LCD_WaitForNotBusy(void)
{
   _WaitForEngineNotBusy();
   return LCD_STATUS_OK;
}


LCD_STATUS LCD_EnableInterrupt(DISP_INTERRUPT_EVENTS eventID)
{
#if ENABLE_LCD_INTERRUPT
   switch(eventID)
   {
      case DISP_LCD_TRANSFER_COMPLETE_INT:
            OUTREGBIT(LCD_REG_INTERRUPT,LCD_REG->INT_ENABLE,COMPLETED,1);
            break;

      case DISP_LCD_CDMQ_COMPLETE_INT:
            OUTREGBIT(LCD_REG_INTERRUPT,LCD_REG->INT_ENABLE,CMDQ_COMPLETED,1);
         break;

      case DISP_LCD_HTT_INT:
            OUTREGBIT(LCD_REG_INTERRUPT,LCD_REG->INT_ENABLE,HTT,1);
         break;

      case DISP_LCD_SYNC_INT:
            OUTREGBIT(LCD_REG_INTERRUPT,LCD_REG->INT_ENABLE,SYNC,1);
         break;

      default:
         return LCD_STATUS_ERROR;
   }
   
   return LCD_STATUS_OK;

#else
    ///TODO: warning log here
    return LCD_STATUS_ERROR;

#endif
}


LCD_STATUS LCD_SetInterruptCallback(void (*pCB)(DISP_INTERRUPT_EVENTS))
{
   _lcdContext.pIntCallback = pCB;
   return LCD_STATUS_OK;
}


// -------------------- LCD Controller Interface --------------------
LCD_STATUS LCD_ConfigParallelIF(LCD_IF_ID id,
                                LCD_IF_PARALLEL_BITS ifDataWidth,
                                LCD_IF_PARALLEL_CLK_DIV clkDivisor,
                                UINT32 writeSetup,
                                UINT32 writeHold,
                                UINT32 writeWait,
                                UINT32 readSetup,
                                UINT32 readHold,
                                UINT32 readLatency,
                                UINT32 waitPeriod,
                                UINT32 chw)
{
   ASSERT(id <= LCD_IF_PARALLEL_2);
   ASSERT(writeSetup <= 16U);
   ASSERT(writeHold <= 16U);
   ASSERT(writeWait <= 64U);
   ASSERT(readSetup <= 16U);
   ASSERT(readHold <= 16U);
   ASSERT(readLatency <= 64U);
   ASSERT(chw <= 16U);
   
   if (0 == writeHold)   writeHold = 1;
   if (0 == writeWait)   writeWait = 1;
   if (0 == readLatency) readLatency = 1;
   
   _WaitForEngineNotBusy();
   
   // (1) Config Data Width
   {
      LCD_REG_PCNFDW pcnfdw = LCD_REG->PARALLEL_DW;
      
      switch(id)
      {
         case LCD_IF_PARALLEL_0: 
            pcnfdw.PCNF0_DW = (UINT32)ifDataWidth; 
            pcnfdw.PCNF0_CHW = chw;
            break;

         case LCD_IF_PARALLEL_1: 
            pcnfdw.PCNF1_DW = (UINT32)ifDataWidth; 
            pcnfdw.PCNF1_CHW = chw;
            break;
            
         case LCD_IF_PARALLEL_2: 
            pcnfdw.PCNF2_DW = (UINT32)ifDataWidth; 
            pcnfdw.PCNF2_CHW = chw;
            break;

         default : 
            ASSERT(0);
      };
      
      LCD_OUTREG32_R(PLCD_REG_PCNFDW,&LCD_REG->PARALLEL_DW, &pcnfdw);
   }
   
   // (2) Config Timing
   {
      UINT32 i;
      LCD_REG_PCNF pcnf;
      
      i = (UINT32)id - LCD_IF_PARALLEL_0;
      pcnf = LCD_REG->PARALLEL_CFG[i];

      pcnf.WST = writeWait;
      pcnf.C2WS = writeSetup;
      pcnf.C2WH = writeHold;
      pcnf.RLT = readLatency;
      pcnf.C2RS = readSetup;
      pcnf.C2RH = readHold;

      LCD_OUTREG32_R(PLCD_REG_PCNF,&LCD_REG->PARALLEL_CFG[i], &pcnf);
   }
   
   return LCD_STATUS_OK;
}

LCD_STATUS LCD_ConfigIfFormat(LCD_IF_FMT_COLOR_ORDER order,
                                                             LCD_IF_FMT_TRANS_SEQ transSeq,
                                                             LCD_IF_FMT_PADDING padding,
                                                             LCD_IF_FORMAT format,
                                                             LCD_IF_WIDTH busWidth)
{
   LCD_REG_WROI_CON ctrl = LCD_REG->WROI_CONTROL;
   
   ctrl.RGB_ORDER  = order;
   ctrl.BYTE_ORDER = transSeq;
   ctrl.PADDING    = padding;
   ctrl.DATA_FMT   = (UINT32)format;
   ctrl.IF_FMT   = (UINT32)busWidth;
   ctrl.IF_24 = 0;

   if(busWidth == LCD_IF_WIDTH_24_BITS)
   {
      ctrl.IF_24 = 1;
   }
   LCD_OUTREG32_R(PLCD_REG_WROI_CON,&LCD_REG->WROI_CONTROL, &ctrl);
   
   return LCD_STATUS_OK;
}


LCD_STATUS LCD_ConfigSerialIF(LCD_IF_ID id,
                                                           LCD_IF_SERIAL_BITS bits,
                                                           UINT32 three_wire,
                                                           UINT32 sdi,
                                                           BOOL   first_pol,
                                                           BOOL   sck_def,
                                                           UINT32 div2,
                                                           UINT32 hw_cs,
                                                           UINT32 css,
                                                           UINT32 csh,
                                                           UINT32 rd_1st,
                                                           UINT32 rd_2nd,
                                                           UINT32 wr_1st,
                                                           UINT32 wr_2nd)
{
   LCD_REG_SCNF config;
   LCD_REG_SIF_TIMING sif_timing;
   unsigned int offset = 0;
   unsigned int sif_id = 0;
   
   ASSERT(id >= LCD_IF_SERIAL_0 && id <= LCD_IF_SERIAL_1);
   
   _WaitForEngineNotBusy();
   
   memset(&config, 0, sizeof(config));
   
   if(id == LCD_IF_SERIAL_1){
      offset = 8;
      sif_id = 1;
   }
   
   LCD_MASKREG32_T(PLCD_REG_SCNF,&config, 0x07 << offset, bits << offset);
   LCD_MASKREG32_T(PLCD_REG_SCNF,&config, 0x08 << offset, three_wire << (offset + 3));
   LCD_MASKREG32_T(PLCD_REG_SCNF,&config, 0x10 << offset, sdi << (offset + 4));
   LCD_MASKREG32_T(PLCD_REG_SCNF,&config, 0x20 << offset, first_pol << (offset + 5));
   LCD_MASKREG32_T(PLCD_REG_SCNF,&config, 0x40 << offset, sck_def << (offset + 6));
   LCD_MASKREG32_T(PLCD_REG_SCNF,&config, 0x80 << offset, div2 << (offset + 7));
   
   config.HW_CS = hw_cs;
   //	config.SIZE_0 = bits;
   LCD_OUTREG32_R(PLCD_REG_SCNF,&LCD_REG->SERIAL_CFG, &config);
   
   sif_timing.WR_2ND = wr_2nd;
   sif_timing.WR_1ST = wr_1st;
   sif_timing.RD_2ND = rd_2nd;
   sif_timing.RD_1ST = rd_1st;
   sif_timing.CSH = csh;
   sif_timing.CSS = css;
   
   LCD_OUTREG32_R(PLCD_REG_SIF_TIMING,&LCD_REG->SIF_TIMING[sif_id], &sif_timing);
   
   return LCD_STATUS_OK;
}


LCD_STATUS LCD_SetResetSignal(BOOL high)
{
    if (high)
        DISP_REG_SET(DISP_REG_CONFIG_LCD_RESET_CON, 0x1);
    else
        DISP_REG_SET(DISP_REG_CONFIG_LCD_RESET_CON, 0x0);

    return LCD_STATUS_OK;
}


LCD_STATUS LCD_SetChipSelect(BOOL high)
{
    if (high)
        LCD_OUTREGBIT(LCD_REG_SIF_CS, LCD_REG->SIF_CS, CS0, 1);
    else
        LCD_OUTREGBIT(LCD_REG_SIF_CS, LCD_REG->SIF_CS, CS0, 0);

    return LCD_STATUS_OK;
}


LCD_STATUS LCD_ConfigDSIIfFormat(LCD_DSI_IF_FMT_COLOR_ORDER order,
                              LCD_DSI_IF_FMT_TRANS_SEQ transSeq,
                              LCD_DSI_IF_FMT_PADDING padding,
                              LCD_DSI_IF_FORMAT format,
                              UINT32 packet_size,
                              BOOL DC_DSI)
{
   //not support
   return LCD_STATUS_OK;
}

// -------------------- Layer Configurations --------------------
LCD_STATUS LCD_LayerEnable(LCD_LAYER_ID id, BOOL enable)
{
   //not support, if be called, it should call OVL function
   if(LCD_LAYER_ALL == id)return LCD_STATUS_OK;

   layer_config[id].layer= id;
   layer_config[id].layer_en= enable;
   //	disp_path_config_layer(&layer_config[id]);

   return LCD_STATUS_OK;
}


LCD_STATUS LCD_ConfigDither(int lrs, int lgs, int lbs, int dbr, int dbg, int dbb)
{
   /*
      LCD_REG_DITHER_CON ctrl;
      
      //	_WaitForEngineNotBusy();
      
      ctrl = LCD_REG->DITHER_CON;
      
      ctrl.LFSR_R_SEED = lrs;
      ctrl.LFSR_G_SEED = lgs;
      ctrl.LFSR_B_SEED = lbs;
      ctrl.DB_R = dbr;
      ctrl.DB_G = dbg;
      ctrl.DB_B = dbb;
      
      OUTREG32(&LCD_REG->DITHER_CON, AS_UINT32(&ctrl));
   */

   return LCD_STATUS_OK;
}


LCD_STATUS LCD_LayerEnableDither(LCD_LAYER_ID id, UINT32 enable)
{
   ASSERT(id < LCD_LAYER_NUM || LCD_LAYER_ALL == id);
   
   return LCD_STATUS_OK;
}


LCD_STATUS LCD_LayerSetAddress(LCD_LAYER_ID id, UINT32 address)
{
   layer_config[id].addr = address;
   //	if(FB_LAYER == id || (FB_LAYER + 1) == id)
   //		disp_path_config_layer_addr((unsigned int)id,address);
   return LCD_STATUS_OK;
}


UINT32 LCD_LayerGetAddress(LCD_LAYER_ID id)
{
   return layer_config[id].addr;
}


LCD_STATUS LCD_LayerSetSize(LCD_LAYER_ID id, UINT32 width, UINT32 height)
{   
   layer_config[id].w = width;
   layer_config[id].h = height;
   return LCD_STATUS_OK;
}


LCD_STATUS LCD_LayerSetPitch(LCD_LAYER_ID id, UINT32 pitch)
{
   layer_config[id].pitch = pitch;
   return LCD_STATUS_OK;
}


LCD_STATUS LCD_LayerSetOffset(LCD_LAYER_ID id, UINT32 x, UINT32 y)
{
   layer_config[id].x = x;
   layer_config[id].y = y;
   return LCD_STATUS_OK;
}


LCD_STATUS LCD_LayerSetWindowOffset(LCD_LAYER_ID id, UINT32 x, UINT32 y)
{
   return LCD_STATUS_OK;
}


LCD_STATUS LCD_LayerSetFormat(LCD_LAYER_ID id, LCD_LAYER_FORMAT format)
{
   layer_config[id].fmt = format;
   return LCD_STATUS_OK;
}


LCD_STATUS LCD_LayerSetRotation(LCD_LAYER_ID id, LCD_LAYER_ROTATION rotation)
{
   //OVL driver not yet export this function
   return LCD_STATUS_OK;
}


LCD_STATUS LCD_LayerSetAlphaBlending(LCD_LAYER_ID id, BOOL enable, UINT8 alpha)
{
   layer_config[id].alpha = (unsigned char)alpha;
   layer_config[id].aen = enable;
   return LCD_STATUS_OK;
}


LCD_STATUS LCD_LayerSetSourceColorKey(LCD_LAYER_ID id, BOOL enable, UINT32 colorKey)
{
   layer_config[id].key = colorKey;
   layer_config[id].keyEn = enable;
   return LCD_STATUS_OK;
}


LCD_STATUS LCD_LayerSet3D(LCD_LAYER_ID id, BOOL enable, BOOL r_first, BOOL landscape)
{
   //OVL driver not yet export this function
   return LCD_STATUS_OK;
}


BOOL LCD_Is3DEnabled(void)
{
   //OVL driver not yet export this function
   return FALSE;
}


BOOL LCD_Is3DLandscapeMode(void)
{
   //OVL driver not yet export this function
   return FALSE;
}


LCD_STATUS LCD_SetRoiWindow(UINT32 x, UINT32 y, UINT32 width, UINT32 height)
{
   LCD_REG_SIZE size;
   
   size.WIDTH = (UINT16)width;
   size.HEIGHT = (UINT16)height;
   
   //    _WaitForEngineNotBusy();
   LCD_OUTREG32_R(PLCD_REG_SIZE,&LCD_REG->WROI_SIZE, &size);
   _lcdContext.roiWndSize = size;
    
   return LCD_STATUS_OK;
}

// -------------------- Tearing Control --------------------
LCD_STATUS LCD_TE_Enable(BOOL enable)
{
   LCD_REG_TECON tecon = LCD_REG->TEARING_CFG;

   tecon.ENABLE = enable ? 1 : 0;
   LCD_OUTREG32_R(PLCD_REG_TECON,&LCD_REG->TEARING_CFG, &tecon);
   
   return LCD_STATUS_OK;
}


BOOL LCD_TE_GetEnable(void)
{
   return (BOOL)(LCD_INREG32(PLCD_REG_TECON, &LCD_REG->TEARING_CFG) & 0x1);
}


LCD_STATUS LCD_TE_SetMode(LCD_TE_MODE mode)
{
   LCD_REG_TECON tecon = LCD_REG->TEARING_CFG;

   tecon.MODE = (LCD_TE_MODE_VSYNC_OR_HSYNC == mode) ? 1 : 0;
   LCD_OUTREG32_R(PLCD_REG_TECON,&LCD_REG->TEARING_CFG, &tecon);
   
   return LCD_STATUS_OK;
}


LCD_STATUS LCD_TE_SetEdgePolarity(BOOL polarity)
{
   LCD_REG_TECON tecon = LCD_REG->TEARING_CFG;

   tecon.EDGE_SEL = (polarity ? 1 : 0);
   LCD_OUTREG32_R(PLCD_REG_TECON,&LCD_REG->TEARING_CFG, &tecon);
   
   return LCD_STATUS_OK;
}


LCD_STATUS LCD_TE_ConfigVHSyncMode(UINT32 hsDelayCnt,
                                   UINT32 vsWidthCnt,
                                   LCD_TE_VS_WIDTH_CNT_DIV vsWidthCntDiv)
{
/*    LCD_REG_TECON tecon = LCD_REG->TEARING_CFG;
    tecon.HS_MCH_CNT = (hsDelayCnt ? hsDelayCnt - 1 : 0);
    tecon.VS_WLMT = (vsWidthCnt ? vsWidthCnt - 1 : 0);
    tecon.VS_CNT_DIV = vsWidthCntDiv;
    LCD_OUTREG32(&LCD_REG->TEARING_CFG, AS_UINT32(&tecon));
*/

    return LCD_STATUS_OK;
}


void LCD_WaitExternalTE(void)
{
#if ENABLE_LCD_INTERRUPT
   long ret;
   static const long WAIT_TIMEOUT = 2 * HZ;	// 2 sec
#else
   long int lcd_wait_time = 0;
   long int delay_ms = 20000;  // 2 sec
#endif


#if ENABLE_LCD_INTERRUPT

   //can not go to this line
   ASSERT(0);

#else

   _WaitForEngineNotBusy();

   while(1)
   {
      if((LCD_INREG32(PLCD_REG_INTERRUPT, &LCD_REG->INT_STATUS)& 0x10) == 0x10)
      {
         break;
      }
        
      udelay(100);
      lcd_wait_time++;
      if (lcd_wait_time>delay_ms)
      {
         printf("[WARNING] Wait for LCD external TE not ack timeout!!!\n");
         LCD_DumpRegisters();
         LCD_TE_Enable(0);
         break;
      }
   }

   // Write clear EXT_TE
   LCD_OUTREGBIT(LCD_REG_INTERRUPT,LCD_REG->INT_STATUS,TE,0);
#endif
}


// -------------------- Operations --------------------
LCD_STATUS LCD_SelectWriteIF(LCD_IF_ID id)
{
   LCD_REG_CMD_ADDR cmd_addr;
   LCD_REG_DAT_ADDR dat_addr;
   
   switch(id)
   {
      case LCD_IF_PARALLEL_0 : cmd_addr.addr = 0; break;
      case LCD_IF_PARALLEL_1 : cmd_addr.addr = 2; break;
      case LCD_IF_PARALLEL_2 : cmd_addr.addr = 4; break;
      case LCD_IF_SERIAL_0   : cmd_addr.addr = 8; break;
      case LCD_IF_SERIAL_1   : cmd_addr.addr = 0xA; break;
      default:
         ASSERT(0);
   }
   dat_addr.addr = cmd_addr.addr + 1;
   LCD_OUTREG16_R(PLCD_REG_CMD_ADDR,&LCD_REG->WROI_CMD_ADDR, &cmd_addr);
   LCD_OUTREG16_R(PLCD_REG_DAT_ADDR,&LCD_REG->WROI_DATA_ADDR, &dat_addr);
   
   return LCD_STATUS_OK;
}


__inline static void _LCD_WriteIF(DWORD baseAddr, UINT32 value, LCD_IF_MCU_WRITE_BITS bits)
{
   switch(bits)
   {
      case LCD_IF_MCU_WRITE_8BIT :
         LCD_OUTREG8(baseAddr, value);
         break;
      
      case LCD_IF_MCU_WRITE_16BIT :
         LCD_OUTREG16(baseAddr, value);
         break;
      
      case LCD_IF_MCU_WRITE_32BIT :
         LCD_OUTREG32(baseAddr, value);
         break;
      
      default:
         ASSERT(0);
   }
}


LCD_STATUS LCD_WriteIF(LCD_IF_ID id, LCD_IF_A0_MODE a0,
                       UINT32 value, LCD_IF_MCU_WRITE_BITS bits)
{
   DWORD baseAddr = 0;
   
   switch(id)
   {
      case LCD_IF_PARALLEL_0 : baseAddr = (DWORD)&LCD_REG->PCMD0; break;
      case LCD_IF_PARALLEL_1 : baseAddr = (DWORD)&LCD_REG->PCMD1; break;
      case LCD_IF_PARALLEL_2 : baseAddr = (DWORD)&LCD_REG->PCMD2; break;
      case LCD_IF_SERIAL_0   : baseAddr = (DWORD)&LCD_REG->SCMD0; break;
      case LCD_IF_SERIAL_1   : baseAddr = (DWORD)&LCD_REG->SCMD1; break;
      default:
         ASSERT(0);
   }
   
   if (LCD_IF_A0_HIGH == a0)
   {
      baseAddr += LCD_A0_HIGH_OFFSET;
   }
   
   _LCD_WriteIF(baseAddr, value, bits);
   
   return LCD_STATUS_OK;
}


__inline static UINT32 _LCD_ReadIF(DWORD baseAddr, LCD_IF_MCU_WRITE_BITS bits)
{
   switch(bits)
   {
      case LCD_IF_MCU_WRITE_8BIT :
         return (UINT32)INREG8(baseAddr);
      
      case LCD_IF_MCU_WRITE_16BIT :
         return (UINT32)INREG16(baseAddr);
      
      case LCD_IF_MCU_WRITE_32BIT :
         return (UINT32)INREG32(baseAddr);
      
      default:
         ASSERT(0);
   }
}


LCD_STATUS LCD_ReadIF(LCD_IF_ID id, LCD_IF_A0_MODE a0,
                      UINT32 *value, LCD_IF_MCU_WRITE_BITS bits)
{
   DWORD baseAddr = 0;
   
   if (NULL == value) return LCD_STATUS_ERROR;
   
   switch(id)
   {
      case LCD_IF_PARALLEL_0 : baseAddr = (DWORD)&LCD_REG->PCMD0; break;
      case LCD_IF_PARALLEL_1 : baseAddr = (DWORD)&LCD_REG->PCMD1; break;
      case LCD_IF_PARALLEL_2 : baseAddr = (DWORD)&LCD_REG->PCMD2; break;
      case LCD_IF_SERIAL_0   : baseAddr = (DWORD)&LCD_REG->SCMD0; break;
      case LCD_IF_SERIAL_1   : baseAddr = (DWORD)&LCD_REG->SCMD1; break;
      default:
         ASSERT(0);
   }
   
   if (LCD_IF_A0_HIGH == a0)
   {
      baseAddr += LCD_A0_HIGH_OFFSET;
   }
   
   *value = _LCD_ReadIF(baseAddr, bits);
   
   return LCD_STATUS_OK;
}


LCD_STATUS LCD_ReadHTT(LCD_IF_ID id, LCD_IF_A0_MODE a0,
                      UINT32 *value, LCD_IF_MCU_WRITE_BITS bits)
{
   DWORD baseAddr = 0;
   
   if (NULL == value) return LCD_STATUS_ERROR;
   
   switch(id)
   {
      case LCD_IF_PARALLEL_0 : baseAddr = (DWORD)(((UINT32)&LCD_REG->PCMD0)+3); break;
      case LCD_IF_PARALLEL_1 : baseAddr = (DWORD)(((UINT32)&LCD_REG->PCMD1)+3); break;
      case LCD_IF_PARALLEL_2 : baseAddr = (DWORD)(((UINT32)&LCD_REG->PCMD2)+3); break;
      case LCD_IF_SERIAL_0   : baseAddr = (DWORD)(((UINT32)&LCD_REG->SCMD0)+3); break;
      case LCD_IF_SERIAL_1   : baseAddr = (DWORD)(((UINT32)&LCD_REG->SCMD1)+3); break;
      default:
         ASSERT(0);
   }
   
   if (LCD_IF_A0_HIGH == a0)
   {
      baseAddr += LCD_A0_HIGH_OFFSET;
   }
   
   *value = _LCD_ReadIF(baseAddr, bits);
   
   return LCD_STATUS_OK;
}


BOOL LCD_IsLayerEnable(LCD_LAYER_ID id)
{
   ASSERT(id <= LCD_LAYER_NUM);
   return (BOOL)(layer_config[id].layer_en);
}


LCD_STATUS LCD_StartTransfer(BOOL blocking)
{
   DBI_FUNC();
   DBG_OnTriggerLcd();

    _WaitForEngineNotBusy();

   disp_path_get_mutex();

   LCD_ConfigOVL();

   disp_path_release_mutex();

   // clear up interrupt status
   LCD_OUTREG32_V(PLCD_REG_INTERRUPT,&LCD_REG->INT_STATUS, 0);
   LCD_OUTREG32_V(PLCD_REG_INTERRUPT,&LCD_REG->INT_ENABLE, 0x1f);
   LCD_OUTREG32_V(PLCD_REG_START,&LCD_REG->START, 0);
   LCD_OUTREG32_V(PLCD_REG_START,&LCD_REG->START, (1 << 15));

#if 0
   if (blocking)
   {
      _WaitForLCDEngineComplete();
   }
#endif

   return LCD_STATUS_OK;
}

LCD_STATUS LCD_ConfigOVL()
{
   unsigned int i;

   DBI_FUNC();

   for(i = 0;i<DDP_OVL_LAYER_MUN;i++){
      if(LCD_IsLayerEnable(i))
         disp_path_config_layer(&layer_config[i]);
   }

   return LCD_STATUS_OK;
}


// -------------------- Retrieve Information --------------------
LCD_OUTPUT_MODE LCD_GetOutputMode(void)
{
   return _lcdContext.outputMode;
}


LCD_STATE  LCD_GetState(void)
{
   if (!s_isLcdPowerOn)
   {
      return LCD_STATE_POWER_OFF;
   }
   
   if (_IsEngineBusy())
   {
      return LCD_STATE_BUSY;
   }
   
   return LCD_STATE_IDLE;
}


LCD_STATUS LCD_DumpRegisters(void)
{
   UINT32 i;
   
   DISP_LOG_PRINT(ANDROID_LOG_WARN, "LCD", "---------- Start dump LCD registers ----------\n");
   
   for (i = 0; i < offsetof(LCD_REGS, DBI_ULTRA_TH); i += 4)
   {
      printf("LCD+%04x : 0x%08x\n", i, INREG32(DISP_DBI_BASE + i));
   }
   
   return LCD_STATUS_OK;
}

LCD_STATUS LCD_Get_VideoLayerSize(unsigned int id, unsigned int *width, unsigned int *height)
{
   return LCD_STATUS_OK;
}


LCD_STATUS LCD_Capture_Videobuffer(unsigned int pvbuf, unsigned int bpp, unsigned int video_rotation)
{
    return LCD_STATUS_OK;   
}


LCD_STATUS LCD_FMDesense_Query()
{
   return LCD_STATUS_OK;
}


LCD_STATUS LCD_FM_Desense(LCD_IF_ID id, unsigned long freq)
{
   UINT32 a,b;
   UINT32 c,d;
   UINT32 wst,c2wh,chw,write_cycles;
   LCD_REG_PCNF config;
   //	LCD_REG_WROI_CON ctrl;    
   LCD_REG_PCNFDW pcnfdw;
   
   LCD_OUTREG32_R(PLCD_REG_PCNF,&config,&LCD_REG->PARALLEL_CFG[(UINT32)id]);
   DBI_LOG("[enter LCD_FM_Desense]:parallel IF = 0x%x, ctrl = 0x%x\n", LCD_INREG32(PLCD_REG_PCNF, &LCD_REG->PARALLEL_CFG[(UINT32)id]));
   wst = config.WST;
   c2wh = config.C2WH;
   // Config Delay Between Commands
   //	OUTREG32(&ctrl, AS_UINT32(&LCD_REG->WROI_CONTROL));
   LCD_OUTREG32_R(PLCD_REG_PCNFDW,&pcnfdw,&LCD_REG->PARALLEL_DW);
   
   switch(id)
   {
      case LCD_IF_PARALLEL_0: chw = pcnfdw.PCNF0_CHW; break;
      case LCD_IF_PARALLEL_1: chw = pcnfdw.PCNF1_CHW; break;
      case LCD_IF_PARALLEL_2: chw = pcnfdw.PCNF2_CHW; break;
      default : ASSERT(0);
   }
   
   a = 13000 - freq * 10 - 20;
   b = 13000 - freq * 10 + 20;
   write_cycles = wst + c2wh + chw + 2;
   c = (a * write_cycles)%13000;
   d = (b * write_cycles)%13000;
   a = (a * write_cycles)/13000;
   b = (b * write_cycles)/13000;
   
   if((b > a)||(c == 0)||(d == 0)){//need modify setting to avoid interference
      DBI_LOG("[LCD_FM_Desense] need to modify lcd setting, freq = %ld\n",freq);
      wst -= wst_step_LCD;
      wst_step_LCD = 0 - wst_step_LCD;
      
      config.WST = wst;
      LCD_WaitForNotBusy();
      LCD_OUTREG32_R(PLCD_REG_PCNF,&LCD_REG->PARALLEL_CFG[(UINT32)id], &config);
   }
   else{
      DBI_LOG("[LCD_FM_Desense] not need to modify lcd setting, freq = %ld\n",freq);
   }
   DBI_LOG("[leave LCD_FM_Desense]:parallel = 0x%x\n", LCD_INREG32(PLCD_REG_PCNF, &LCD_REG->PARALLEL_CFG[(UINT32)id]));

   return LCD_STATUS_OK;  
}


LCD_STATUS LCD_Reset_WriteCycle(LCD_IF_ID id)
{
   LCD_REG_PCNF config;
   UINT32 wst;

   DBI_LOG("[enter LCD_Reset_WriteCycle]:parallel = 0x%x\n", LCD_INREG32(PLCD_REG_PCNF, &LCD_REG->PARALLEL_CFG[(UINT32)id]));

   if(wst_step_LCD > 0){//have modify lcd setting, so when fm turn off, we must decrease wst to default setting
      DBI_LOG("[LCD_Reset_WriteCycle] need to reset lcd setting\n");
      LCD_OUTREG32_R(PLCD_REG_PCNF,&config, &LCD_REG->PARALLEL_CFG[(UINT32)id]);
      wst = config.WST;
      wst -= wst_step_LCD;
      wst_step_LCD = 0 - wst_step_LCD;
      
      config.WST = wst;
      LCD_WaitForNotBusy();
      LCD_OUTREG32_R(PLCD_REG_PCNF,&LCD_REG->PARALLEL_CFG[(UINT32)id], &config);
   }
   else{
      DBI_LOG("[LCD_Reset_WriteCycle] parallel is default setting, not need to reset it\n");
   }
   DBI_LOG("[leave LCD_Reset_WriteCycle]:parallel = 0x%x\n", LCD_INREG32(PLCD_REG_PCNF, &LCD_REG->PARALLEL_CFG[(UINT32)id]));

   return LCD_STATUS_OK; 
}


LCD_STATUS LCD_Get_Default_WriteCycle(LCD_IF_ID id, unsigned int *write_cycle)
{
   UINT32 wst,c2wh,chw;
   LCD_REG_PCNF config;
   //	LCD_REG_WROI_CON ctrl;    
   LCD_REG_PCNFDW pcnfdw;
   
   if(is_get_default_write_cycle){
      *write_cycle = default_write_cycle;
      return LCD_STATUS_OK;
   }
   
   LCD_OUTREG32_R(PLCD_REG_PCNF,&config, &LCD_REG->PARALLEL_CFG[(UINT32)id]);
   DBI_LOG("[enter LCD_Get_Default_WriteCycle]:parallel IF = 0x%x, ctrl = 0x%x\n", LCD_INREG32(PLCD_REG_PCNF, &LCD_REG->PARALLEL_CFG[(UINT32)id]));
   wst = config.WST;
   c2wh = config.C2WH;
   // Config Delay Between Commands
   //	OUTREG32(&ctrl, AS_UINT32(&LCD_REG->WROI_CONTROL));
   LCD_OUTREG32_R(PLCD_REG_PCNFDW,&pcnfdw, &LCD_REG->PARALLEL_DW);

   switch(id)
   {
      case LCD_IF_PARALLEL_0: chw = pcnfdw.PCNF0_CHW; break;
      case LCD_IF_PARALLEL_1: chw = pcnfdw.PCNF1_CHW; break;
      case LCD_IF_PARALLEL_2: chw = pcnfdw.PCNF2_CHW; break;
      default : ASSERT(0);
   }
   *write_cycle = wst + c2wh + chw + 2;
   default_write_cycle = *write_cycle;
   default_wst = wst;
   is_get_default_write_cycle = TRUE;
   DBI_LOG("[leave LCD_Get_Default_WriteCycle]:Default_Write_Cycle = %d\n", *write_cycle);   

   return LCD_STATUS_OK;  
}


LCD_STATUS LCD_Get_Current_WriteCycle(LCD_IF_ID id, unsigned int *write_cycle)
{
   UINT32 wst,c2wh,chw;
   LCD_REG_PCNF config;
   //	LCD_REG_WROI_CON ctrl;       
   LCD_REG_PCNFDW pcnfdw;
   
   LCD_OUTREG32_R(PLCD_REG_PCNF,&config, &LCD_REG->PARALLEL_CFG[(UINT32)id]);
   DBI_LOG("[enter LCD_Get_Current_WriteCycle]:parallel IF = 0x%x, ctrl = 0x%x\n", LCD_INREG32(PLCD_REG_PCNF, &LCD_REG->PARALLEL_CFG[(UINT32)id]));
   wst = config.WST;
   c2wh = config.C2WH;
   // Config Delay Between Commands
   //	OUTREG32(&ctrl, AS_UINT32(&LCD_REG->WROI_CONTROL));
   LCD_OUTREG32_R(PLCD_REG_PCNFDW,&pcnfdw, &LCD_REG->PARALLEL_DW);
   switch(id)
   {
      case LCD_IF_PARALLEL_0: chw = pcnfdw.PCNF0_CHW; break;
      case LCD_IF_PARALLEL_1: chw = pcnfdw.PCNF1_CHW; break;
      case LCD_IF_PARALLEL_2: chw = pcnfdw.PCNF2_CHW; break;
      default : ASSERT(0);
   }
   
   *write_cycle = wst + c2wh + chw + 2;
   DBI_LOG("[leave LCD_Get_Current_WriteCycle]:Default_Write_Cycle = %d\n", *write_cycle);   

   return LCD_STATUS_OK;  
}

LCD_STATUS LCD_Change_WriteCycle(LCD_IF_ID id, unsigned int write_cycle)
{
   UINT32 wst;
   LCD_REG_PCNF config;
   
   LCD_OUTREG32_R(PLCD_REG_PCNF,&config, &LCD_REG->PARALLEL_CFG[(UINT32)id]);
   DBI_LOG("[enter LCD_Change_WriteCycle]:parallel IF = 0x%x, ctrl = 0x%x\n",
   LCD_INREG32(PLCD_REG_PCNF, &LCD_REG->PARALLEL_CFG[(UINT32)id]), LCD_INREG32(PLCD_REG_WROI_CON, &LCD_REG->WROI_CONTROL));   
   
   DBI_LOG("[LCD_Change_WriteCycle] modify lcd setting\n");
   wst = write_cycle - default_write_cycle + default_wst;
   
   config.WST = wst;
   LCD_WaitForNotBusy();
   LCD_OUTREG32_R(PLCD_REG_PCNF,&LCD_REG->PARALLEL_CFG[(UINT32)id], &config);
   DBI_LOG("[leave LCD_Change_WriteCycle]:parallel = 0x%x\n", LCD_INREG32(PLCD_REG_PCNF, &LCD_REG->PARALLEL_CFG[(UINT32)id]));

   return LCD_STATUS_OK;  
}

