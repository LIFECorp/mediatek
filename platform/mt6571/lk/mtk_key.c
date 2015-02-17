/*
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */


#include <printf.h>
#include <platform/mt_typedefs.h>
#include <platform/mtk_key.h>
#include <platform/boot_mode.h>
#include <platform/mt_pmic.h>
#include <platform/mt_gpio.h>
#include <platform/mt_pmic_wrap_init.h>
#include <platform/sync_write.h>
#include <target/cust_key.h>


void set_kpd_pmic_mode(void)
{
	unsigned int ret;

	ret = pmic_config_interface(STRUP_CON3, 0x0, PMIC_RG_USBDL_EN_MASK, PMIC_RG_USBDL_EN_SHIFT);

	if (ret != 0)
		printf("kpd write fail, addr: 0x%x\n", STRUP_CON3);

#ifdef MTK_PMIC_RST_KEY
	pmic_config_interface(STRUP_CON3, 0x01, PMIC_RG_FCHR_PU_EN_MASK, PMIC_RG_FCHR_PU_EN_SHIFT);//pull up homekey pin of PMIC
	pmic_config_interface(STRUP_CON3, 0, PMIC_RG_FCHR_KEYDET_EN_MASK, PMIC_RG_FCHR_KEYDET_EN_SHIFT);//disable homekey pin FCHR mode of PMIC
#endif

	return;
}

void disable_PMIC_kpd_clock(void)
{
#if 0
	int rel = 0;
	rel = pmic_config_interface(WRP_CKPDN,0x1, PMIC_RG_WRP_32K_PDN_MASK, PMIC_RG_WRP_32K_PDN_SHIFT);
	if(rel !=  0){
		printf("kpd disable_PMIC_kpd_clock register fail!\n");
	}
#endif
}

void enable_PMIC_kpd_clock(void)
{
#if 0
	int rel = 0;
	rel = pmic_config_interface(WRP_CKPDN,0x0, PMIC_RG_WRP_32K_PDN_MASK, PMIC_RG_WRP_32K_PDN_SHIFT);
	if(rel !=  0){
		printf("kpd enable_PMIC_kpd_clock register fail!\n");
	}
#endif
}

BOOL mtk_detect_key(unsigned short key)	/* key: HW keycode */
{
#ifdef MACH_FPGA /* FPGA doesn't include keypad HW */
	return FALSE;
#else
	unsigned short idx, bit, din;

	if (key >= KPD_NUM_KEYS)
		return FALSE;
#if 0
	if (key % 9 == 8)
		key = 8;
#endif
	if (key == MTK_PMIC_PWR_KEY)
    	{ /* Power key */
		if (1 == pmic_detect_powerkey())
		{
			//dbg_print ("power key is pressed\n");
			return TRUE;
		}
		return FALSE;
	}

#ifdef MTK_PMIC_RST_KEY
	if (key == MTK_PMIC_RST_KEY)
	{
		if (1 == pmic_detect_homekey())
		{
			printf("mtk detect key function pmic_detect_homekey pressed\n");
			return TRUE;
		}
		return FALSE;
	}
#endif

	idx = key / 16;
	bit = key % 16;

	din = DRV_Reg16(KP_MEM1 + (idx << 2)) & (1U << bit);
	if (!din) {
		printf("key %d is pressed\n", key);
		return TRUE;
	}
	return FALSE;
#endif /* MACH_FPGA */
}

BOOL mtk_detect_pmic_just_rst(void)
{
	kal_uint32 just_rst = 0;

	printf("detecting pmic just reset\n");

	pmic_read_interface(STRUP_CON8, &just_rst, PMIC_JUST_PWRKEY_RST_MASK, PMIC_JUST_PWRKEY_RST_SHIFT);
	if (just_rst)
	{
		printf("Just recover from a reset\n");
		pmic_config_interface(STRUP_CON8, 0x01, PMIC_CLR_JUST_RST_MASK, PMIC_CLR_JUST_RST_SHIFT);
		return TRUE;
	}
	return FALSE;
}
