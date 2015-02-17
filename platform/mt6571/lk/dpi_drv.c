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

#define ENABLE_DPI_INTERRUPT        0
#define ENABLE_DPI_REFRESH_RATE_LOG 0

#include <string.h>
#include <platform/mt_clkmgr.h>
#include <platform/mt_gpt.h>
#include <platform/disp_drv_platform.h>
#include <platform/ddp_reg.h>
#include <platform/sync_write.h>
#include <platform/dpi_reg.h>
#include <platform/dsi_reg.h>

#ifdef OUTREG32
  #undef OUTREG32
  #define OUTREG32(x, y) mt65xx_reg_sync_writel(y, x)
#endif

#ifndef OUTREGBIT
#define OUTREGBIT(TYPE,REG,bit,value)  \
                    do {    \
                        TYPE r = *((TYPE*)&INREG32(&REG));    \
                        r.bit = value;    \
                        OUTREG32(&REG, AS_UINT32(&r));    \
                    } while (0)
#endif

#define DPI_OUTREG32_R(type, addr2, addr1) DISP_OUTREG32_R(type, addr2, addr1)

static PDPI_REGS const DPI_REG = (PDPI_REGS)(DISP_DPI_BASE);
static BOOL s_isDpiPowerOn = FALSE;
static DPI_REGS regBackup;
static void (*dpiIntCallback)(DISP_INTERRUPT_EVENTS);

#define DPI_MMSYS_CG1       ( DISP_DPI_ENGINE_SW_CG_BIT \
                            | DISP_DPI_IF_SW_CG_BIT )

#define DPI_REG_OFFSET(r)       offsetof(DPI_REGS, r)
#define REG_ADDR(base, offset)  (((BYTE *)(base)) + (offset))


const UINT32 BACKUP_DPI_REG_OFFSETS[] =
{
   DPI_REG_OFFSET(INT_ENABLE),
   DPI_REG_OFFSET(CON),
   DPI_REG_OFFSET(OUTPUT_SETTING),
   DPI_REG_OFFSET(SIZE),
   
   DPI_REG_OFFSET(TGEN_HWIDTH),
   DPI_REG_OFFSET(TGEN_HPORCH),
   DPI_REG_OFFSET(TGEN_VWIDTH),
   DPI_REG_OFFSET(TGEN_VPORCH),
   
   DPI_REG_OFFSET(BG_HCNTL),
   DPI_REG_OFFSET(BG_VCNTL),
   DPI_REG_OFFSET(BG_COLOR),

   DPI_REG_OFFSET(FIFO_CTL),
   DPI_REG_OFFSET(TMODE),
};


static void _BackupDPIRegisters(void)
{
   DPI_REGS *reg = &regBackup;
   UINT32 i;
   
   for (i = 0; i < ARY_SIZE(BACKUP_DPI_REG_OFFSETS); ++ i)
   {
      OUTREG32(REG_ADDR(reg, BACKUP_DPI_REG_OFFSETS[i]),
                        AS_UINT32(REG_ADDR(DPI_REG, BACKUP_DPI_REG_OFFSETS[i])));
   }
}


static void _RestoreDPIRegisters(void)
{
   DPI_REGS *reg = &regBackup;
   UINT32 i;
   
   for (i = 0; i < ARY_SIZE(BACKUP_DPI_REG_OFFSETS); ++ i)
   {
      OUTREG32(REG_ADDR(DPI_REG, BACKUP_DPI_REG_OFFSETS[i]),
                        AS_UINT32(REG_ADDR(reg, BACKUP_DPI_REG_OFFSETS[i])));
   }
}


static void _ResetBackupedDPIRegisterValues(void)
{
   DPI_REGS *regs = &regBackup;

   memset((void*)regs, 0, sizeof(DPI_REGS));
}


#if ENABLE_DPI_REFRESH_RATE_LOG
static void _DPI_LogRefreshRate(DPI_REG_INTERRUPT status)
{
   static unsigned long prevUs = 0xFFFFFFFF;
   
   if (status.VSYNC)
   {
      struct timeval curr;
      do_gettimeofday(&curr);
      
      if (prevUs < curr.tv_usec)
      {
         DISP_LOG_PRINT(ANDROID_LOG_INFO, "DPI", "Receive 1 vsync in %lu us\n", 
                                      curr.tv_usec - prevUs);
      }
      prevUs = curr.tv_usec;
   }
}
#else
#define _DPI_LogRefreshRate(x)  do {} while(0)
#endif


void DPI_DisableIrq(void)
{
#if ENABLE_DPI_INTERRUPT
   DPI_REG_INTERRUPT enInt = DPI_REG->INT_ENABLE;

   enInt.VSYNC = 0;
   enInt.VDE = 0;
   enInt.UNDERFLOW = 0;
   OUTREG32(&DPI_REG->INT_ENABLE, AS_UINT32(&enInt));
#endif
}


void DPI_EnableIrq(void)
{
#if ENABLE_DPI_INTERRUPT
   DPI_REG_INTERRUPT enInt = DPI_REG->INT_ENABLE;

   enInt.VSYNC = 1;
   enInt.VDE = 0;
   enInt.UNDERFLOW = 1;
   OUTREG32(&DPI_REG->INT_ENABLE, AS_UINT32(&enInt));
#endif
}


#if ENABLE_DPI_INTERRUPT
static irqreturn_t _DPI_InterruptHandler(int irq, void *dev_id)
{   
   static int counter = 0;
   DPI_REG_INTERRUPT status = DPI_REG->INT_STATUS;
   //    if (status.FIFO_EMPTY) ++ counter;
   
   if(status.VSYNC)
   {
      if(dpiIntCallback)
         dpiIntCallback(DISP_DPI_VSYNC_INT);
   }
   
   if (status.VSYNC && counter) {
      DISP_LOG_PRINT(ANDROID_LOG_ERROR, "DPI", "[Error] DPI FIFO is empty, "
      "received %d times interrupt !!!\n", counter);
      counter = 0;
   }
   
   _DPI_LogRefreshRate(status);
   OUTREG32(&DPI_REG->INT_STATUS, 0);

   return IRQ_HANDLED;
}
#endif


#define VSYNC_US_TO_NS(x) (x * 1000)
unsigned int vsync_timer_dpi = 0;


DPI_STATUS DPI_WaitVSYNC(void)
{
   UINT32 dpi_wait_time = 0;

   MASKREG32(DISP_REG_RDMA_INT_STATUS, 0x2, 0x0);
   while((INREG32(DISP_REG_RDMA_INT_STATUS)&0x2) != 0x2)	// polling RDMA start
   {
      udelay(50);//sleep 50us

      dpi_wait_time++;
      if(dpi_wait_time > 40000){
         DISP_LOG_PRINT(ANDROID_LOG_WARN, "DPI", "Wait for RDMA0 Start IRQ timeout!!!\n");
         break;
      }
   }
   MASKREG32(DISP_REG_RDMA_INT_STATUS, 0x2, 0x0);

   return DPI_STATUS_OK;
}


void DPI_PauseVSYNC(BOOL enable)
{
}


void DPI_InitVSYNC(unsigned int vsync_interval)
{

}


DPI_STATUS DPI_Init(BOOL isDpiPoweredOn)
{
   if (isDpiPoweredOn) {
      _BackupDPIRegisters();
   } else {
      _ResetBackupedDPIRegisterValues();
   }
   
   DPI_PowerOn();

#if ENABLE_DPI_INTERRUPT
   if (request_irq(MT65XX_DPI_IRQ_ID,
        _DPI_InterruptHandler, IRQF_TRIGGER_LOW, "mtkdpi", NULL) < 0)
   {
      DISP_LOG_PRINT(ANDROID_LOG_INFO, "DPI", "[ERROR] fail to request DPI irq\n"); 
      return DPI_STATUS_ERROR;
   }

   {
      DPI_REG_INTERRUPT enInt = DPI_REG->INT_ENABLE;
      enInt.VSYNC = 1;
      OUTREG32(&DPI_REG->INT_ENABLE, AS_UINT32(&enInt));
   }
#endif

   return DPI_STATUS_OK;
}


DPI_STATUS DPI_Deinit(void)
{
   DPI_DisableClk();
   DPI_PowerOff();
   
   return DPI_STATUS_OK;
}


void DPI_mipi_switch(bool on, LCM_PARAMS *lcm_params)
{
    DSI_PHY_clk_switch(on, lcm_params);
}


DPI_STATUS DPI_Init_PLL(LCM_PARAMS *lcm_params)
{
    DSI_PHY_clk_setting(lcm_params);

    return DPI_STATUS_OK;
}


DPI_STATUS DPI_Set_DrivingCurrent(LCM_PARAMS *lcm_params)
{
    LCD_Set_DrivingCurrent(lcm_params);

    return DPI_STATUS_OK;
}

DPI_STATUS DPI_PowerOn()
{
    if (!s_isDpiPowerOn)
    {
        MASKREG32(DISP_REG_CONFIG_MMSYS_CG_CLR1, DPI_MMSYS_CG1, DPI_MMSYS_CG1);

        _RestoreDPIRegisters();
        s_isDpiPowerOn = TRUE;
        printf("[DISP] - DPI_PowerOn. 0x%8x\n", INREG32(DISP_REG_CONFIG_MMSYS_CG_CON1));        
    }
   
    return DPI_STATUS_OK;
}


DPI_STATUS DPI_PowerOff()
{
    if (s_isDpiPowerOn)
    {
        _BackupDPIRegisters();
        MASKREG32(DISP_REG_CONFIG_MMSYS_CG_SET1, DPI_MMSYS_CG1, DPI_MMSYS_CG1);

        printf("[DISP] - DPI_PowerOff. 0x%8x\n", INREG32(DISP_REG_CONFIG_MMSYS_CG_CON1));
        s_isDpiPowerOn = FALSE;
    }
   
    return DPI_STATUS_OK;
}


DPI_STATUS DPI_EnableClk()
{
   DPI_REG_EN en = DPI_REG->DPI_EN;

   en.EN = 1;
   DPI_OUTREG32_R(PDPI_REG_EN, &DPI_REG->DPI_EN, &en);

   return DPI_STATUS_OK;
}


DPI_STATUS DPI_DisableClk()
{
   DPI_REG_EN en = DPI_REG->DPI_EN;
   en.EN = 0;
   DPI_OUTREG32_R(PDPI_REG_EN, &DPI_REG->DPI_EN, &en);
   
   return DPI_STATUS_OK;
}


DPI_STATUS DPI_ConfigVsync(DPI_POLARITY polarity, UINT32 pulseWidth, UINT32 backPorch,
                           UINT32 frontPorch)
{
   DPI_REG_TGEN_VPORCH vporch = DPI_REG->TGEN_VPORCH;
   
   vporch.VBP = backPorch;
   vporch.VFP = frontPorch;
   
   OUTREG32(&DPI_REG->TGEN_VWIDTH, AS_UINT32(&pulseWidth));
   DPI_OUTREG32_R(PDPI_REG_TGEN_VPORCH, &DPI_REG->TGEN_VPORCH, &vporch);
   
   return DPI_STATUS_OK;
}


DPI_STATUS DPI_ConfigHsync(DPI_POLARITY polarity, UINT32 pulseWidth, UINT32 backPorch,
                           UINT32 frontPorch)
{
   DPI_REG_TGEN_HPORCH hporch = DPI_REG->TGEN_HPORCH;
   
   hporch.HBP = backPorch;
   hporch.HFP = frontPorch;
   
   OUTREG32(&DPI_REG->TGEN_HWIDTH, AS_UINT32(&pulseWidth));
   DPI_OUTREG32_R(PDPI_REG_TGEN_HPORCH, &DPI_REG->TGEN_HPORCH, &hporch);
   
   return DPI_STATUS_OK;
}


DPI_STATUS DPI_OutputSetting(LCM_DPI_PARAMS *pConfig)
{
   DPI_REG_OUTPUT_SETTING output_setting = DPI_REG->OUTPUT_SETTING;

   if (pConfig->rgb_order == LCM_COLOR_ORDER_BGR)
      output_setting.OUT_CH_SWAP = 0x5;
   else
      output_setting.OUT_CH_SWAP = 0x0;
   output_setting.OUT_BIT_SWAP = 0;
   output_setting.B_MASK = 0;
   output_setting.G_MASK = 0;
   output_setting.R_MASK = 0;
   output_setting.DE_MASK = 0;
   output_setting.HS_MASK = 0;
   output_setting.VS_MASK = 0;
   if (pConfig->de_pol == LCM_POLARITY_FALLING)
      output_setting.DE_POL = 1;
   else
      output_setting.DE_POL = 0;
   if (pConfig->hsync_pol == LCM_POLARITY_FALLING)
      output_setting.HSYNC_POL = 1;
   else
      output_setting.HSYNC_POL = 0;
   if (pConfig->vsync_pol == LCM_POLARITY_FALLING)
      output_setting.VSYNC_POL = 1;
   else
      output_setting.VSYNC_POL = 0;
   if (pConfig->clk_pol == LCM_POLARITY_FALLING)
      output_setting.DPI_CK_POL = 1;
   else
      output_setting.DPI_CK_POL = 0;
   output_setting.DPI_OEN_OFF = 0;
   output_setting.DUAL_EDGE_SEL = 0;

   DPI_OUTREG32_R(PDPI_REG_OUTPUT_SETTING, &DPI_REG->OUTPUT_SETTING,&output_setting);

   return DPI_STATUS_OK;
}


DPI_STATUS DPI_FBSetSize(UINT32 width, UINT32 height)
{
   DPI_REG_SIZE size;
   size.WIDTH = width;
   size.HEIGHT = height;
   
   DPI_OUTREG32_R(PDPI_REG_SIZE, &DPI_REG->SIZE, &size);
   
   return DPI_STATUS_OK;
}


DPI_STATUS DPI_DumpRegisters(void)
{
   UINT32 i;
   
   DISP_LOG_PRINT(ANDROID_LOG_WARN, "DPI", "---------- Start dump DPI registers ----------\n");
   
   for (i = 0; i < sizeof(DPI_REGS); i += 4)
   {
      DISP_LOG_PRINT(ANDROID_LOG_WARN, "DPI", "DPI+%04x : 0x%08x\n", i, INREG32(DISP_DPI_BASE + i));
   }
   
   for (i = 0; i < sizeof(DSI_PHY_REGS); i += 4)
   {
      DISP_LOG_PRINT(ANDROID_LOG_INFO, "DPI", "DPI_PHY+%04x(%p) : 0x%08x\n", i, (UINT32*)(MIPI_TX_CONFIG_BASE + i), INREG32((MIPI_TX_CONFIG_BASE + i)));
   }

   return DPI_STATUS_OK;
}


DPI_STATUS DPI_EnableInterrupt(DISP_INTERRUPT_EVENTS eventID)
{
#if ENABLE_DPI_INTERRUPT
   switch(eventID)
   {
      case DISP_DPI_VSYNC_INT:
            OUTREGBIT(DPI_REG_INTERRUPT,DPI_REG->INT_ENABLE,VSYNC,1);
         break;
      default:
         return DPI_STATUS_ERROR;
   }
   
   return DPI_STATUS_OK;

#else
   ///TODO: warning log here
   return DPI_STATUS_ERROR;

#endif
}


DPI_STATUS DPI_SetInterruptCallback(void (*pCB)(DISP_INTERRUPT_EVENTS))
{
   dpiIntCallback = pCB;
   
   return DPI_STATUS_OK;
}


DPI_STATUS DPI_FMDesense_Query(void)
{
   return DPI_STATUS_ERROR;
}


DPI_STATUS DPI_FM_Desense(unsigned long freq)
{
   return DPI_STATUS_OK;
}


DPI_STATUS DPI_Reset_CLK(void)
{
   return DPI_STATUS_OK;
}


DPI_STATUS DPI_Get_Default_CLK(unsigned int *clk)
{
   return DPI_STATUS_OK;
}


DPI_STATUS DPI_Get_Current_CLK(unsigned int *clk)
{
   return DPI_STATUS_OK;
}


DPI_STATUS DPI_Change_CLK(unsigned int clk)
{
   return DPI_STATUS_OK;
}

