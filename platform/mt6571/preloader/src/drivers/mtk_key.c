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
#include "platform.h"
#include "mtk_key.h"
#include "mtk_pmic.h"
#include <gpio.h>


void mtk_kpd_gpio_set(void);
extern U32 pmic_read_interface(U32 RegNum, U32 *val, U32 MASK, U32 SHIFT);
extern U32 pmic_config_interface(U32 RegNum, U32 val, U32 MASK, U32 SHIFT);


void mtk_kpd_gpios_get(unsigned int ROW_REG[], unsigned int COL_REG[], unsigned int GPIO_MODE[])
{
	int i;
	for (i = 0; i < 8; i++)
	{
		ROW_REG[i] = 0;
		COL_REG[i] = 0;
		GPIO_MODE[i] = 0;
	}
	#ifdef GPIO_KPD_KROW0_PIN
		ROW_REG[0] = GPIO_KPD_KROW0_PIN;
		GPIO_MODE[0] = GPIO_KPD_KROW0_PIN_M_KROW;
	#endif

	#ifdef GPIO_KPD_KROW1_PIN
		ROW_REG[1] = GPIO_KPD_KROW1_PIN;
		GPIO_MODE[1] = GPIO_KPD_KROW1_PIN_M_KROW;
	#endif

	#ifdef GPIO_KPD_KROW2_PIN
		ROW_REG[2] = GPIO_KPD_KROW2_PIN;
		GPIO_MODE[2] = GPIO_KPD_KROW2_PIN_M_KROW;
	#endif

	#ifdef GPIO_KPD_KROW3_PIN
		ROW_REG[3] = GPIO_KPD_KROW3_PIN;
		GPIO_MODE[3] = GPIO_KPD_KROW3_PIN_M_KROW;
	#endif

	#ifdef GPIO_KPD_KROW4_PIN
		ROW_REG[4] = GPIO_KPD_KROW4_PIN;
		GPIO_MODE[4] = GPIO_KPD_KROW4_PIN_M_KROW;
	#endif

	#ifdef GPIO_KPD_KROW5_PIN
		ROW_REG[5] = GPIO_KPD_KROW5_PIN;
		GPIO_MODE[5] = GPIO_KPD_KROW5_PIN_M_KROW;
	#endif

	#ifdef GPIO_KPD_KROW6_PIN
		ROW_REG[6] = GPIO_KPD_KROW6_PIN;
		GPIO_MODE[6] = GPIO_KPD_KROW6_PIN_M_KROW;
	#endif

	#ifdef GPIO_KPD_KROW7_PIN
		ROW_REG[7] = GPIO_KPD_KROW7_PIN;
		GPIO_MODE[7] = GPIO_KPD_KROW7_PIN_M_KROW;
	#endif


	#ifdef GPIO_KPD_KCOL0_PIN
		COL_REG[0] = GPIO_KPD_KCOL0_PIN;
		GPIO_MODE[0] |= (GPIO_KPD_KCOL0_PIN_M_KCOL << 4);
	#endif

	#ifdef GPIO_KPD_KCOL1_PIN
		COL_REG[1] = GPIO_KPD_KCOL1_PIN;
		GPIO_MODE[1] |= (GPIO_KPD_KCOL1_PIN_M_KCOL << 4);
	#endif

	#ifdef GPIO_KPD_KCOL2_PIN
		COL_REG[2] = GPIO_KPD_KCOL2_PIN;
		GPIO_MODE[2] |= (GPIO_KPD_KCOL2_PIN_M_KCOL << 4);
	#endif

	#ifdef GPIO_KPD_KCOL3_PIN
		COL_REG[3] = GPIO_KPD_KCOL3_PIN;
		GPIO_MODE[3] |= (GPIO_KPD_KCOL3_PIN_M_KCOL << 4);
	#endif

	#ifdef GPIO_KPD_KCOL4_PIN
		COL_REG[4] = GPIO_KPD_KCOL4_PIN;
		GPIO_MODE[4] |= (GPIO_KPD_KCOL4_PIN_M_KCOL << 4);
	#endif

	#ifdef GPIO_KPD_KCOL5_PIN
		COL_REG[5] = GPIO_KPD_KCOL5_PIN;
		GPIO_MODE[5] |= (GPIO_KPD_KCOL5_PIN_M_KCOL << 4);
	#endif

	#ifdef GPIO_KPD_KCOL6_PIN
		COL_REG[6] = GPIO_KPD_KCOL6_PIN;
		GPIO_MODE[6] |= (GPIO_KPD_KCOL6_PIN_M_KCOL << 4);
	#endif

	#ifdef GPIO_KPD_KCOL7_PIN
		COL_REG[7] = GPIO_KPD_KCOL7_PIN;
		GPIO_MODE[7] |= (GPIO_KPD_KCOL7_PIN_M_KCOL << 4);
	#endif
}

void mtk_kpd_gpio_set(void)
{
	unsigned int ROW_REG[8];
	unsigned int COL_REG[8];
	unsigned int GPIO_MODE[8];
	int i;
	// int idx;

	//print("Enter mtk_kpd_gpio_set! \n");
	mtk_kpd_gpios_get(ROW_REG, COL_REG, GPIO_MODE);

	for(i = 0; i < 8; i++)
	{
		if (COL_REG[i] != 0)
		{
			/* KCOL: GPIO INPUT + PULL ENABLE + PULL UP */
			mt_set_gpio_mode(COL_REG[i], ((GPIO_MODE[i] >> 4) & 0x0f));
			mt_set_gpio_dir(COL_REG[i], 0);
			mt_set_gpio_pull_enable(COL_REG[i], 1);
			mt_set_gpio_pull_select(COL_REG[i], 1);
		}

		if (ROW_REG[i] != 0)
		{
			/* KROW: GPIO output + pull disable + pull down */
			mt_set_gpio_mode(ROW_REG[i], (GPIO_MODE[i] & 0x0f));
			mt_set_gpio_dir(ROW_REG[i], 1);
			mt_set_gpio_pull_enable(ROW_REG[i], 0);
			mt_set_gpio_pull_select(ROW_REG[i], 0);
		}
	}

#if !KPD_USE_EXTEND_TYPE //single keypad for kcol IO_CONFIG(IES), for input mode
	/* digital IO would refined again on LK, so enable all case here */
	/* No need to set IES because all default values are 1 */
#endif
	mdelay(33);
}

void set_kpd_pmic_mode(void)
{
	unsigned int ret;
	unsigned int temp_reg = 0;

	ret = pmic_config_interface(STRUP_CON3, 0x0, PMIC_RG_USBDL_EN_MASK, PMIC_RG_USBDL_EN_SHIFT);

	if (ret != 0)
		print("kpd write fail, addr: 0x%x\n", STRUP_CON3);

	mtk_kpd_gpio_set();

	temp_reg = DRV_Reg16(KP_SEL);

#if KPD_USE_EXTEND_TYPE	//double keypad
	/* select specific cols for double keypad */
	#ifndef GPIO_KPD_KCOL0_PIN
		temp_reg &= ~(KP_COL0_SEL);
	#endif

	#ifndef GPIO_KPD_KCOL1_PIN
		temp_reg &= ~(KP_COL1_SEL);
	#endif

	#ifndef GPIO_KPD_KCOL2_PIN
		temp_reg &= ~(KP_COL2_SEL);
	#endif

	temp_reg |= 0x1;

#else //single keypad
	temp_reg &= ~(0x1);
#endif

	DRV_WriteReg16(KP_SEL, temp_reg);
	DRV_WriteReg16(KP_EN, 0x1);
	print("after set KP_EN: KP_SEL = 0x%x\n", DRV_Reg16(KP_SEL));

#ifdef MTK_PMIC_RST_KEY
	print("MTK_PMIC_RST_KEY is used\n");
	pmic_config_interface(STRUP_CON3, 0x01, PMIC_RG_FCHR_PU_EN_MASK, PMIC_RG_FCHR_PU_EN_SHIFT);//pull up homekey pin of PMIC
	pmic_config_interface(STRUP_CON3, 0, PMIC_RG_FCHR_KEYDET_EN_MASK, PMIC_RG_FCHR_KEYDET_EN_SHIFT);//disable homekey pin FCHR mode of PMIC
	//mdelay(100);
#endif

	return;
}

void disable_PMIC_kpd_clock(void)
{
#if 0
	int rel = 0;
	//print("kpd disable_PMIC_kpd_clock register!\n");
	rel = pmic_config_interface(WRP_CKPDN,0x1, PMIC_RG_WRP_32K_PDN_MASK, PMIC_RG_WRP_32K_PDN_SHIFT);
	if(rel !=  0){
		print("kpd disable_PMIC_kpd_clock register fail!\n");
	}
#endif
}

void enable_PMIC_kpd_clock(void)
{
#if 0
	int rel = 0;
	//print("kpd enable_PMIC_kpd_clock register!\n");
	rel = pmic_config_interface(WRP_CKPDN,0x0, PMIC_RG_WRP_32K_PDN_MASK, PMIC_RG_WRP_32K_PDN_SHIFT);
	if(rel !=  0){
		print("kpd enable_PMIC_kpd_clock register fail!\n");
	}
#endif
}

bool mtk_detect_key(unsigned short key)  /* key: HW keycode */
{
#if CFG_FPGA_PLATFORM /* FPGA doesn't include keypad HW */
	return false;
#else
	unsigned short idx, bit, din;
	U32 just_rst;

	if (key >= KPD_NUM_KEYS)
		return false;
#if 0
	if (key % 9 == 8)
		key = 8;
#endif
#if 0 //KPD_PWRKEY_USE_EINT
#define GPIO_DIN_BASE	(GPIO_BASE + 0x0a00)

	if (key == 8) /* Power key */
	{
		idx = KPD_PWRKEY_EINT_GPIO / 16;
		bit = KPD_PWRKEY_EINT_GPIO % 16;

		din = DRV_Reg16 (GPIO_DIN_BASE + (idx << 4)) & (1U << bit);
		din >>= bit;
		if (din == KPD_PWRKEY_GPIO_DIN)
		{
			print ("power key is pressed\n");
			return true;
		}
		return false;
	}
#else // check by PMIC
	if (key == MTK_PMIC_PWR_KEY) /* Power key */
	{
		#if 0 // for long press reboot, not boot up from a reset
		pmic_read_interface(STRUP_CON8, &just_rst, PMIC_JUST_PWRKEY_RST_MASK, PMIC_JUST_PWRKEY_RST_SHIFT);
		if(just_rst)
		{
			pmic_config_interface(STRUP_CON8, 0x01, PMIC_CLR_JUST_RST_MASK, PMIC_CLR_JUST_RST_SHIFT);
			print("Just recover from a reset\n");
			return false;
		}
		#endif
		if (1 == pmic_detect_powerkey())
		{
			print ("power key is pressed\n");
			return true;
		}
		return false;
	}
#endif

#ifdef MTK_PMIC_RST_KEY
	if (key == MTK_PMIC_RST_KEY)
	{
		//print("mtk detect key function pmic_detect_homekey MTK_PMIC_RST_KEY = %d\n",MTK_PMIC_RST_KEY);
		if (1 == pmic_detect_homekey())
		{
			print("home key is pressed\n");
			return TRUE;
		}
		return FALSE;
	}
#endif

	idx = key / 16;
	bit = key % 16;

	din = DRV_Reg16(KP_MEM1 + (idx << 2)) & (1U << bit);
	if (!din) /* key is pressed */
	{
		print("key %d is pressed\n", key);
		return true;
	}
	return false;
#endif /* CFG_FPGA_PLATFORM */
}

bool mtk_detect_dl_keys(void)
{
#ifdef KPD_DL_KEY1
	if (mtk_detect_key (KPD_DL_KEY1)
#ifdef KPD_DL_KEY2
		&& mtk_detect_key (KPD_DL_KEY2)
#ifdef KPD_DL_KEY3
		&& mtk_detect_key (KPD_DL_KEY3)
#endif
#endif
		)
	{
		print("download keys are pressed\n");
		return true;
	}
#endif
	return false;
}
