/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * MediaTek Inc. (C) 2010. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#include "typedefs.h"
#include "mtk_timer.h"


/* register address */
#define RST_CTL0                    (0xc0009010)
#define PWR_CTL1                    (0xc0009024)
#define PWR_MON                     (0xc00090a8)
#define BOOTROM_BOOT_ADDR           (0xc0002030)
#define SCU_CPU_PWR_STATUS          (0xc000a008)


void power_off_cpu1(void)
{
    /* 1.1 Polling PWR_MON[13] = 1 to make sure CPU1 is in WFI */
    while (!(DRV_Reg32(PWR_MON) & (1U << 13)));
    
    /* 1.2 turn off neon1 */
    DRV_WriteReg32(PWR_CTL1, DRV_Reg32(PWR_CTL1) | 0x00000400);
    DRV_WriteReg32(PWR_CTL1, DRV_Reg32(PWR_CTL1) | 0x00000100);
    DRV_WriteReg32(PWR_CTL1, DRV_Reg32(PWR_CTL1) | 0x00000200);
    DRV_WriteReg32(PWR_CTL1, DRV_Reg32(PWR_CTL1) & 0xfffff7ff);
    
    /* 2. Set PWR_CTL1[14] = 1 */
    DRV_WriteReg32(PWR_CTL1, DRV_Reg32(PWR_CTL1) | 0x00004000);
    
    /* 3. Set PWR_CTL1[12] = 1 */
    DRV_WriteReg32(PWR_CTL1, DRV_Reg32(PWR_CTL1) | 0x00001000);
    
    /* 4. Set PWR_CTL1[13] = 1 */
    DRV_WriteReg32(PWR_CTL1, DRV_Reg32(PWR_CTL1) | 0x00002000);
    
    /* 5. Set PWR_CTL1[4] = 0 for dormant mode, or set PWR_CTL1[3:0]=4¡¦b1111 for shutdown mode (shutdown mode now) */
    DRV_WriteReg32(PWR_CTL1, DRV_Reg32(PWR_CTL1) | 0x0000000f);
    //dsb();
    mdelay(2); //delay 2ms
    
    /* 6.1 Set PWR_CTL1[15] = 0 */
    DRV_WriteReg32(PWR_CTL1, DRV_Reg32(PWR_CTL1) & 0xffff7fff);
    
    /* 6.2 Set PWR_CTL1[25:24] = 2¡¦b10 for dormant mode or set 2¡¦b11 for shutdown mode (shutdown mode now) */
    DRV_WriteReg32(PWR_CTL1, DRV_Reg32(PWR_CTL1) | 0x03000000);
    
    /* 6.3 Set CPU1 power status register in SCU to dormant mode or shut-down mode (shutdown mode now) */
    DRV_WriteReg32(SCU_CPU_PWR_STATUS, DRV_Reg32(SCU_CPU_PWR_STATUS) | 0x00000300);
    //dsb();
}


