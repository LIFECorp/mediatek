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
#include "mt6571.h"

void share_uart_with_i2c_dbg(void)
{
    #define I2C_DBTOOL_MISC_OFFSET      (0x0518)
    #define BIT_REG_DBG_UART_MASK_TX    (4U)
    #define MASK_REG_DBG_UART_MASK_TX   (1U << BIT_REG_DBG_UART_MASK_TX)
    #define BIT_REG_DBG_UART_MASK_RX    (8U)
    #define MASK_REG_DBG_UART_MASK_RX   (1U << BIT_REG_DBG_UART_MASK_RX)
    #define I2C_DBTOOL_MISC             (CONFIG_BASE + I2C_DBTOOL_MISC_OFFSET)

    /*
     * Mode            Setting
     * Uart RX only    gpio_96[2:0] = 3'h1(0x1000_53C0[2:0])
     * I2C SCL only    gpio_96[2:0] = 3'h2(0x1000_53C0[2:0])
     *                 reg_dbg_uart_mask_rx = 1 (0x1000_1518[8])
     * Share           gpio_96[2:0] = 3'h2(0x1000_53C0[2:0])
     *                 reg_dbg_uart_mask_rx = 0 (0x1000_1518[8])
     *
     * Uart TX only    gpio_95[2:0] = 3'h1(0x1000_53B0[30:28])
     * I2C SDA only    gpio_95[2:0] = 3'h2(0x1000_53B0[30:28])
     *                 reg_dbg_uart_mask_tx = 1 (0x1000_1518[4])
     * Share           gpio_95[2:0] = 3'h2(0x1000_53B0[30:28])
     *                 reg_dbg_uart_mask_tx = 0 (0x1000_1518[4])
     */

    /* This setting takes effect only if GPIO96 mode = 2 */
    /* reg_dbg_uart_mask_rx = 0 */
    unsigned int value = 0;

    value = DRV_Reg32(I2C_DBTOOL_MISC);
    value &= ~MASK_REG_DBG_UART_MASK_RX;
    DRV_WriteReg32(I2C_DBTOOL_MISC, value);
    
    /* This setting takes effect only if GPIO95 mode = 2 */
    /* reg_dbg_uart_mask_tx = 0 */
    value = DRV_Reg32(I2C_DBTOOL_MISC);
    value &= ~MASK_REG_DBG_UART_MASK_TX;
    DRV_WriteReg32(I2C_DBTOOL_MISC, value);
}

