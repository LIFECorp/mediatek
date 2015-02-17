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
#include "boot_device.h"
#include "mtk_nand.h"
#include "mmc_common_inter.h"

#include "uart.h"
#include "mtk_nand_core.h"
#include "mtk_pll.h"
#include "mtk_nand_core.h"
#include "mt_i2c.h"
#include "mt_rtc.h"
#include "mt_emi.h"
#include "mtk_pmic.h"
#include "mt_cpu_power.h"
#include "mtk_wdt.h"
#include "ram_console.h"
#include "cust_sec_ctrl.h"
#include "gpio.h"
#include "mt_pmic_wrap_init.h"
#include "mtk_key.h"
#include "mt_pl_ptp.h"
#include "mt_mmmfg_sram_repair.h"
#include "dram_buffer.h"
/*============================================================================*/
/* CONSTAND DEFINITIONS                                                       */
/*============================================================================*/
#define MOD "[PLF]"

#define MEM_PRESERVED_MAGIC         (0x504D454D)
#define RGUS_PRESERVED_MAGIC        (0x53554752)    //store RGU status, RGUS

/*============================================================================*/
/* GLOBAL VARIABLES                                                           */
/*============================================================================*/
unsigned int sys_stack[CFG_SYS_STACK_SZ >> 2];
const unsigned int sys_stack_sz = CFG_SYS_STACK_SZ;
boot_mode_t g_boot_mode;
boot_dev_t g_boot_dev;
meta_com_t g_meta_com_type = META_UNKNOWN_COM;
u32 g_meta_com_id = 0;
boot_reason_t g_boot_reason;
ulong g_boot_time;
extern unsigned int part_num;
extern part_hdr_t   part_info[PART_MAX_NUM];

#if defined (MTK_KERNEL_POWER_OFF_CHARGING)
kal_bool kpoc_flag = false;
#endif

#if CFG_BOOT_ARGUMENT
#define bootarg g_dram_buf->bootarg
#endif

#if CFG_USB_AUTO_DETECT
bool g_usbdl_flag;
#endif


/*============================================================================*/
/* EXTERNAL FUNCTIONS                                                         */
/*============================================================================*/
#ifdef MTK_POWER_EXT_DETECT

#define GPIO_PHONE_EVB_DETECT (GPIO143|0x80000000)

int mt_get_board_type(void)
{
#if 1
	 static int board_type = MT_BOARD_NONE;

	 if (board_type != MT_BOARD_NONE)
	 	return board_type;

	 /* Enable AUX_IN0 as GPI */
	 mt_set_gpio_ies(GPIO_PHONE_EVB_DETECT, GPIO_IES_ENABLE);

	 /* Set internal pull-down for AUX_IN0 */
	 mt_set_gpio_pull_select(GPIO_PHONE_EVB_DETECT, GPIO_PULL_DOWN);
	 mt_set_gpio_pull_enable(GPIO_PHONE_EVB_DETECT, GPIO_PULL_ENABLE);

	 /* Wait 20us */
	 udelay(20);

	 /* Read AUX_INO's GPI value*/
	 mt_set_gpio_mode(GPIO_PHONE_EVB_DETECT, GPIO_MODE_00);
	 mt_set_gpio_dir(GPIO_PHONE_EVB_DETECT, GPIO_DIR_IN);

	 if (mt_get_gpio_in(GPIO_PHONE_EVB_DETECT) == 1) {
		 /* Disable internal pull-down if external pull-up on PCB(leakage) */
		 mt_set_gpio_pull_enable(GPIO_PHONE_EVB_DETECT, GPIO_PULL_DISABLE);
		 board_type = MT_BOARD_EVB;
	 } else {
	 	 /* Disable internal pull-down if external pull-up on PCB(leakage) */
		 mt_set_gpio_pull_enable(GPIO_PHONE_EVB_DETECT, GPIO_PULL_DISABLE);
		 board_type = MT_BOARD_PHONE;
	 }
 	 print("[PLF]Board:%s\n", (board_type == MT_BOARD_EVB) ? "EVB" : "PHONE");
	 return board_type;
#else
	 return MT_BOARD_EVB;
#endif
}
#endif //MTK_POWER_EXT_DETECT

static u32 boot_device_init(void)
{
    #if (CFG_BOOT_DEV == BOOTDEV_SDMMC)
    return (u32)mmc_init_device();
    #else
    return (u32)nand_init_device();
    #endif
}

void platform_set_chrg_cur(int ma)
{
    hw_set_cc(ma);
}

int usb_accessory_in(void)
{
#if !CFG_FPGA_PLATFORM
    int exist = 0;

    if (PMIC_CHRDET_EXIST == pmic_IsUsbCableIn()) {
        exist = 1;
        #if !CFG_USBIF_COMPLIANCE
        /* enable charging current as early as possible to avoid can't enter
         * following battery charging flow when low battery
         */
        platform_set_chrg_cur(450);
        #endif
    }
    return exist;
#else
    return 1;
#endif
}

extern bool is_uart_cable_inserted(void);

int usb_cable_in(void)
{
#if !CFG_FPGA_PLATFORM
    int exist = 0;
    CHARGER_TYPE ret;

    if ((g_boot_reason == BR_USB) || usb_accessory_in()) {
        ret = mt_charger_type_detection();
        if (ret == STANDARD_HOST || ret == CHARGING_HOST) {
            print("\n%s USB cable in\n", MOD);
            mt_usb_phy_poweron();
            mt_usb_phy_savecurrent();

            /* enable pmic hw charger detection */
            #ifdef MTK_POWER_EXT_DETECT
			if (MT_BOARD_PHONE == mt_get_board_type())
			#endif
			{
            	if (hw_check_battery())
                	pl_hw_ulc_det();
			}

            exist = 1;
        } else if (ret == NONSTANDARD_CHARGER || ret == STANDARD_CHARGER) {
            #if CFG_USBIF_COMPLIANCE
            platform_set_chrg_cur(450);
            #endif
        }
    }

    return exist;
#else
    print("\n%s USB cable in\n", MOD);
    mt_usb_phy_poweron();
    mt_usb_phy_savecurrent();

    return 1;
#endif
}

void platform_vusb_on(void)
{
#if !CFG_FPGA_PLATFORM
    U32 ret=0;

    ret=pmic_config_interface( (U32)(DIGLDO_CON2),
                               (U32)(1),
                               (U32)(PMIC_RG_VUSB_EN_MASK),
                               (U32)(PMIC_RG_VUSB_EN_SHIFT)
	                         );
    if(ret!=0)
    {
        print("[PLF_vusb_on]Fail\n");
    }
    else
    {
        print("[PLF_vusb_on]OK\n");
    }
    // enable USB clock, clear cg bit (BIT13)
    //DRV_ClrReg32(0x10000024, 0x2000); //72
    //DRV_SetReg32(0x10000084, 0x2000);   //for 71 & 72, write 1 clear
    DRV_SetReg32(CLK_CLRCG_1, 0x2000);   //for 71 & 72, write 1 clear
#endif
    return;
}

void platform_set_boot_args(void)
{
#if CFG_BOOT_ARGUMENT
    bootarg.magic = BOOT_ARGUMENT_MAGIC;
    bootarg.mode  = g_boot_mode;
    //efuse should use seclib_get_devinfo_with_index(),
    //no need check 3G in 72
    bootarg.e_flag = 0;
    bootarg.log_port = CFG_UART_LOG;
    bootarg.log_baudrate = CFG_LOG_BAUDRATE;
    bootarg.log_enable = (u8)log_status();
    bootarg.dram_rank_num = get_dram_rank_nr();
    get_dram_rank_size(bootarg.dram_rank_size);
    bootarg.boot_reason = g_boot_reason;
    bootarg.meta_com_type = (u32)g_meta_com_type;
    bootarg.meta_com_id = g_meta_com_id;
    bootarg.boot_time = get_timer(g_boot_time);

    bootarg.part_num =  g_dram_buf->part_num;
    bootarg.part_info = g_dram_buf->part_info;

    bootarg.ddr_reserve_enable = 0;
    bootarg.ddr_reserve_success= 0;

#if CFG_WORLD_PHONE_SUPPORT
    print("%smd_type[0]=%d\n", MOD, bootarg.md_type[0]);
    print("%smd_type[1]=%d\n", MOD, bootarg.md_type[1]);
#endif

    print("\n%s boot reason:%d\n", MOD, g_boot_reason);
    print("%sboot mode:%d\n", MOD, g_boot_mode);
    print("%s META COM%d:%d\n", MOD, bootarg.meta_com_id, bootarg.meta_com_type);
    print("%s<0x%x>:0x%x\n", MOD, &bootarg.e_flag, bootarg.e_flag);
    print("%sboot time:%dms\n", MOD, bootarg.boot_time);
    print("%sDDR presv mode:en=%d,ok=%d\n", MOD, bootarg.ddr_reserve_enable, bootarg.ddr_reserve_success);
#endif

}

void platform_set_dl_boot_args(da_info_t *da_info)
{
#if CFG_BOOT_ARGUMENT
    if (da_info->addr != BA_FIELD_BYPASS_MAGIC)
	bootarg.da_info.addr = da_info->addr;

    if (da_info->arg1 != BA_FIELD_BYPASS_MAGIC)
	bootarg.da_info.arg1 = da_info->arg1;

    if (da_info->arg2 != BA_FIELD_BYPASS_MAGIC)
	bootarg.da_info.arg2 = da_info->arg2;

    if (da_info->len != BA_FIELD_BYPASS_MAGIC)
	bootarg.da_info.len = da_info->len;

    if (da_info->sig_len != BA_FIELD_BYPASS_MAGIC)
	bootarg.da_info.sig_len = da_info->sig_len;
#endif

    return;
}

void platform_wdt_all_kick(void)
{
    /* kick watchdog to avoid cpu reset */
    mtk_wdt_restart();

#if !CFG_FPGA_PLATFORM
    /* kick PMIC watchdog to keep charging */
    pl_kick_chr_wdt();
#endif
}

void platform_wdt_kick(void)
{
    /* kick hardware watchdog */
    mtk_wdt_restart();
}

#if CFG_DT_MD_DOWNLOAD
void platform_modem_download(void)
{
    print("[%s]modem dwld\n", MOD);

    while (1) {
        platform_wdt_all_kick();
    }
}
#endif

#if CFG_USB_AUTO_DETECT
void platform_usbdl_flag_check()
{
    U32 usbdlreg = 0;
    usbdlreg = DRV_Reg32(SRAMROM_USBDL_REG);
     /*Set global variable to record the usbdl flag*/
    if(usbdlreg & USBDL_BIT_EN)
        g_usbdl_flag = 1;
    else
        g_usbdl_flag = 0;
}

void platform_usb_auto_detect_flow()
{

    print("USB DL Flag is %d\n",g_usbdl_flag);

    /*usb download flag haven't set */
	if(g_usbdl_flag == 0 && g_boot_reason != BR_RTC){
        /*set up usbdl flag*/
        platform_safe_mode(1,CFG_USB_AUTO_DETECT_TIMEOUT_MS);
        print("PL go reset,trig BROM usb auto detect\n");

        /* WDT by pass powerkey reboot */
        /* keep the previous status, pass it into reset function */
        if (WDT_BY_PASS_PWK_REBOOT == mtk_wdt_boot_check())
            mtk_arch_reset(1);
        else
            mtk_arch_reset(0);

	}else{
    /*usb download flag have been set*/
    }

}
#endif


void platform_safe_mode(int en, u32 timeout)
{
    U32 usbdlreg;
    U32 usbdlmagic;

    usbdlreg = 0;
    usbdlmagic = 0;

    /* if anything is wrong and caused wdt reset, enter bootrom download mode */
    /* div function must be relocate in mem preloader,
       use left shit 10 bits instead divide 1000  */
#if defined(CFG_SRAM_PRELOADER_MODE) || defined(CFG_MEM_PRESERVED_MODE)
    timeout = !timeout ? USBDL_TIMEOUT_MAX : timeout >> 10;
#else
    timeout = !timeout ? USBDL_TIMEOUT_MAX : timeout / 1000;
#endif
    timeout <<= 2;
    timeout &= USBDL_TIMEOUT_MASK; /* usbdl timeout cannot exceed max value */

    usbdlreg |= timeout;
    if (en) {
        usbdlreg |= USBDL_BIT_EN;
        usbdlmagic = SRAMROM_USBDL_MAGIC;
    } else {
    	usbdlreg &= ~USBDL_BIT_EN;
        usbdlmagic = 0;
    }

    usbdlreg &= ~USBDL_PL;

    DRV_WriteReg32(SRAMROM_USBDL_REG, usbdlreg);
    DRV_WriteReg32(SRAMROM_USBDL_MAGIC_REG, usbdlmagic);

    return;
}

#if CFG_EMERGENCY_DL_SUPPORT
void platform_emergency_download(u32 timeout)
{
    /* enter download mode */
    print("%s emgcy dwld mode>TMO:%ds\n", MOD, timeout / 1000);
    platform_safe_mode(1, timeout);

#if !CFG_FPGA_PLATFORM
    mtk_arch_reset(0); /* don't bypass power key */
#endif

    while(1);
}
#endif


int platform_get_mcp_id(u8 *id, u32 len, u32 *fw_id_len)
{
    int ret = -1;

    memset(id, 0, len);

#if (CFG_BOOT_DEV == BOOTDEV_SDMMC)
    ret = mmc_get_device_id(id, len,fw_id_len);
#else
    ret = nand_get_device_id(id, len);
#endif

    return ret;
}

static boot_reason_t platform_boot_status(void)
{
#if defined (MTK_KERNEL_POWER_OFF_CHARGING)
	ulong begin = get_timer(0);
	do  {
		if (rtc_boot_check()) {
			print("%sRTC boot\n", MOD);
			return BR_RTC;
		}
		if(!kpoc_flag)
			break;
	} while (get_timer(begin) < 1000 && kpoc_flag);
#else
    if (rtc_boot_check()) {
        print("%sRTC boot\n", MOD);
        return BR_RTC;
    }

#endif
    if (mtk_wdt_boot_check() == WDT_NORMAL_REBOOT) {
        print("%sWDT normal boot\n", MOD);
        return BR_WDT;
    } else if (mtk_wdt_boot_check() == WDT_BY_PASS_PWK_REBOOT){
        print("%sWDT reboot bypass PWR key\n", MOD);
        return BR_WDT_BY_PASS_PWK;
    }
#if !CFG_FPGA_PLATFORM
    /* check power key */
    if (mtk_detect_key(MTK_PMIC_PWR_KEY)) {
        print("%sPWR key boot\n", MOD);
        rtc_mark_bypass_pwrkey();
        return BR_POWER_KEY;
    }
#endif

#ifdef MTK_POWER_EXT_DETECT
	if (MT_BOARD_PHONE == mt_get_board_type())
#endif
	{
		if (usb_accessory_in()) {
        	print("%sUSB boot\n", MOD);
        	return BR_USB;
    	}
#if ((RTC_2SEC_REBOOT_ENABLE) && !(CFG_FPGA_PLATFORM))
    	else {
        	if (rtc_2sec_reboot_check()) {
            	print("%s2sec reboot\n", MOD);
            	return BR_2SEC_REBOOT;
        	}
    	}
#endif //#ifdef RTC_2SEC_REBOOT_ENABLE

    	print("%sUnknown boot\n", MOD);
   		pl_power_off();
    	/* should nerver be reached */
	}

    print("%sPWR key boot\n", MOD);

    return BR_POWER_KEY;
}

#if CFG_LOAD_DSP_ROM || CFG_LOAD_MD_ROM
int platform_is_three_g(void)
{
    u32 tmp = sp_check_platform();

    return (tmp & 0x1) ? 0 : 1;
}
#endif

chip_ver_t platform_chip_ver(void)
{
//    unsigned int hw_subcode = DRV_Reg32(APHW_SUBCODE);
//    unsigned int sw_ver = DRV_Reg32(APSW_VER);
    unsigned int ver = DRV_Reg32(APHW_VER);
    return ver;
}

// ------------------------------------------------
// detect download mode
// ------------------------------------------------

bool platform_com_wait_forever_check(void)
{
#ifdef USBDL_DETECT_VIA_KEY
    /* check download key */
    if (TRUE == mtk_detect_key(COM_WAIT_KEY)) {
        print("%s COM handshk t-out force disable: Key\n", MOD);
        return TRUE;
    }
#endif

#ifdef USBDL_DETECT_VIA_AT_COMMAND
    print("PLF_com_wait_forever_chk\n");
    /* check SRAMROM_USBDL_TO_DIS */
    if (USBDL_TO_DIS == (INREG32(SRAMROM_USBDL_TO_DIS) & USBDL_TO_DIS)) {
	print("%s COM handshk t-out force disable:AT Cmd\n", MOD);
	CLRREG32(SRAMROM_USBDL_TO_DIS, USBDL_TO_DIS);
	return TRUE;
    }
#endif

    return FALSE;
}

// ------------------------------------------------
// check if sram repair enable
// ------------------------------------------------

unsigned int platform_sram_repair_enable_check(void)
{
    unsigned int sram_repair_enable;
    sram_repair_enable = 0;

    if (SRAM_REPAIR_ENABLE_BIT == (DRV_Reg32(SRAM_REPAIR_REG) & SRAM_REPAIR_ENABLE_BIT) )
        sram_repair_enable = 1;
    else
        sram_repair_enable = 0;
    return sram_repair_enable;
}

void platform_pre_init(void)
{
    u32 ret;
    u32 pmic_ret;
    u32 pwrap_ret,i;
#if defined(CFG_MEM_PRESERVED_MODE)
    u8 *p_tcm;
#endif //#if defined(CFG_MEM_PRESERVED_MODE)
    #ifdef PL_PROFILING
    u32 profiling_time;
    profiling_time = 0;
    #endif

    pwrap_ret = 0;
    i = 0;
    ret = 0;
#if defined(CFG_MEM_PRESERVED_MODE)
    // bss init
    for (p_tcm=(u8 *)&bss_init_emi_baseaddr;p_tcm < (u8 *)BSS_TCM_END;p_tcm = p_tcm +4)
    {
        *(u32 *)p_tcm = 0;
    }
#endif
    /* move pll code to audio_sys_ram */
    memcpy((char *)&Image$$PLL_INIT$$Base, &__load_start_pll_text,
            &__load_stop_pll_text - &__load_start_pll_text);

    /* init timer */
    mtk_timer_init();

    /* init boot time */
    g_boot_time = get_timer(0);

#if 0 /* FIXME */
    /*
     * NoteXXX: CPU 1 may not be reset clearly after power-ON.
     *          Need to apply a S/W workaround to manualy reset it first.
     */
    {
	U32 val;
	val = DRV_Reg32(0xC0009010);
	DRV_WriteReg32(0xC0009010, val | 0x2);
	gpt_busy_wait_us(10);
	DRV_WriteReg32(0xC0009010, val & ~0x2);
	gpt_busy_wait_us(10);
    }
#ifndef SLT_BOOTLOADER
    /* power off cpu1 for power saving */
    power_off_cpu1();
#endif
#endif
    ptp_init1();
    /* init pll */
    /* for memory preserved mode */
    // do not init pll/emi in memory preserved mode, due to code is located in EMI
    // set all pll except EMI
    mtk_pll_init();

    /*GPIO init*/
#if (!(CFG_FPGA_PLATFORM) && (defined(DUMMY_AP) || defined(TINY)))
    mt_gpio_set_default();
#endif

    /* init uart baudrate when pll on */
    mtk_uart_init(UART_SRC_CLK_FRQ, CFG_LOG_BAUDRATE);


#if (!(CFG_FPGA_PLATFORM) && defined(TINY))
    // set UART1 to MD
    // for TINY load only
    #ifdef GPIO_UART_URXD1_PIN
    mt_set_gpio_mode(GPIO_UART_URXD1_PIN, GPIO_UART_URXD1_PIN_M_MD_URXD);
    #endif
    #ifdef GPIO_UART_UTXD1_PIN
    mt_set_gpio_mode(GPIO_UART_UTXD1_PIN, GPIO_UART_UTXD1_PIN_M_MD_UTXD);
    #endif
#endif

    /* init pmic i2c interface and pmic */
    /* no need */
    //i2c_ret = i2c_v1_init();
    //retry 3 times for pmic wrapper init
    #ifdef PL_PROFILING
    profiling_time = get_timer(0);
    #endif
    pwrap_init_preloader();


		/* check is uart cable in*/
#if (CFG_USB_UART_SWITCH)
		platform_vusb_on();
		if (is_uart_cable_inserted()) {
			print("\n%sSwap to UART\n", MOD);
			mt_usb_set_to_uart_mode();
		} else {
			print("\n%sKeep in USB\n", MOD);
		}
#endif

#if 0
    if (platform_sram_repair_enable_check())
    {
        //MM SRAM Repair
        ret = MFG_MM_SRAM_repair();
        if (ret < 0 )
            printf("MFG_MM_SRAM_repair fail\n");
        else
            printf("MFG_MM_SRAM_repair OK\n");
    }
#endif

    #ifdef PL_PROFILING
    printf("#T#pwrap_init=%d\n", get_timer(profiling_time));
    profiling_time = get_timer(0);  //for next
    #endif
    pmic_ret = pmic_init();
    usbdl_wo_battery_forced(1);

    //enable long press reboot function
    #ifdef KPD_PMIC_LPRST_TD
        #ifdef ONEKEY_REBOOT_NORMAL_MODE_PL
        printf("ONEKEY_REBOOT_PL OK\n");
        pmic_config_interface(TOP_RST_MISC, 0x01, PMIC_RG_PWRKEY_RST_EN_MASK, PMIC_RG_PWRKEY_RST_EN_SHIFT);
        pmic_config_interface(TOP_RST_MISC, (U32)KPD_PMIC_LPRST_TD, PMIC_RG_PWRKEY_RST_TD_MASK, PMIC_RG_PWRKEY_RST_TD_SHIFT);
        #endif

        #ifdef TWOKEY_REBOOT_NORMAL_MODE_PL
        printf("TWOKEY_REBOOT_PL OK\n");
        pmic_config_interface(TOP_RST_MISC, 0x01, PMIC_RG_PWRKEY_RST_EN_MASK, PMIC_RG_PWRKEY_RST_EN_SHIFT);
        pmic_config_interface(TOP_RST_MISC, 0x01, PMIC_RG_HOMEKEY_RST_EN_MASK, PMIC_RG_HOMEKEY_RST_EN_SHIFT);
        pmic_config_interface(STRUP_CON3,	0x01, PMIC_RG_FCHR_PU_EN_MASK, PMIC_RG_FCHR_PU_EN_SHIFT);//pull up homekey pin of PMIC
        pmic_config_interface(STRUP_CON3,	0, PMIC_RG_FCHR_KEYDET_EN_MASK, PMIC_RG_FCHR_KEYDET_EN_SHIFT);//disable homekey pin FCHR mode of PMIC
        pmic_config_interface(TOP_RST_MISC, (U32)KPD_PMIC_LPRST_TD, PMIC_RG_PWRKEY_RST_TD_MASK, PMIC_RG_PWRKEY_RST_TD_SHIFT);
        #endif
    #endif

    #ifdef PL_PROFILING
    printf("#T#pmic_init=%d\n", get_timer(profiling_time));
    #endif

    print("%sInit PWRAP:%s\n", MOD, pwrap_ret ? "FAIL" : "OK");
    print("%sInit PMIC:%s\n", MOD, pmic_ret ? "FAIL" : "OK");
    print("%schip[%x]\n", MOD, platform_chip_ver());
}


#ifdef MTK_MT8193_SUPPORT
extern int mt8193_init(void);
#endif


void platform_init(void)
{
    u32 ret, tmp;
    boot_reason_t reason;
#if defined(CFG_MEM_PRESERVED_MODE)
    u32 new_stack;
#endif //#if defined(CFG_MEM_PRESERVED_MODE)

    #ifdef PL_PROFILING
    u32 profiling_time;
    profiling_time = 0;
    #endif

    #ifdef PL_PROFILING
    profiling_time = get_timer(0);
    #endif
    /* init watch dog, will enable AP watch dog */
    mtk_wdt_init();

    #ifdef PL_PROFILING
    printf("#T#wdt_init=%d\n", get_timer(profiling_time));
    profiling_time = get_timer(0);  //for next
    #endif
    /*init kpd PMIC mode support*/
    set_kpd_pmic_mode();

    #ifdef PL_PROFILING
    printf("#T#kpd_pmic=%d\n", get_timer(profiling_time));
    #endif

#if CFG_MDWDT_DISABLE
    /* no need to disable MD WDT, the code here is for backup reason */
    /* disable MD0 watch dog. */
    DRV_WriteReg32(0x20050000, 0x2200);
#endif

#if !defined(CFG_MEM_PRESERVED_MODE)
    #ifdef PL_PROFILING
    profiling_time = get_timer(0);
    #endif
//ALPS00427972, implement the analog register formula
    //Set the calibration after power on
    //Add here for eFuse, chip version checking -> analog register calibration
    mt_usb_calibraion();
//ALPS00427972, implement the analog register formula

    /* make usb11 phy enter savecurrent mode */
    // USB11, USB host need it, 72 with host ip, but not list in feature list,
    // USB20, USB target for target,
    // access with UM (throught CPU) will mett all '?', means protect
    // access with PM (BUS, not CPU) will mett all '0', means clock or Power is gating
    //mt_usb11_phy_savecurrent();

    #ifdef PL_PROFILING
    printf("#T#usb calib=%d\n", get_timer(profiling_time));
    profiling_time = get_timer(0);  //for next
    #endif

    g_boot_reason = reason = platform_boot_status();
    if (reason == BR_RTC || reason == BR_POWER_KEY || reason == BR_USB || reason == BR_WDT || reason == BR_WDT_BY_PASS_PWK
#ifdef RTC_2SEC_REBOOT_ENABLE
        || reason == BR_2SEC_REBOOT
#endif //#ifdef RTC_2SEC_REBOOT_ENABLE
        )
        rtc_bbpu_power_on();

    #ifdef PL_PROFILING
    printf("#T#BR&bbpu on=%d\n", get_timer(profiling_time));
    profiling_time = get_timer(0);  //for next
    #endif
    enable_PMIC_kpd_clock();

    #ifdef PL_PROFILING
    printf("#T#enable PMIC kpd clk=%d\n", get_timer(profiling_time));
    #endif
#else
    platform_boot_status();
    rtc_bbpu_power_on();

  // clear memory preserved mode reboot status,
     mtk_wdt_presrv_mode_config(0,0);

#endif

    #ifdef PL_PROFILING
    profiling_time = get_timer(0);
    #endif

    //flush cache
#if defined(CFG_MEM_PRESERVED_MODE)
    //wake up core 1 and flush core 1 cache
    print("%scpu1 flush start\n", MOD);
    bootup_slave_cpu();
    print("%scpu1 flush ok\n", MOD);
    //flush core 1 cache
    print("%scpu0 flush start\n", MOD);
#if 0
    {
        u32 i;
        volatile u32 tmp;

        tmp = 1;
        // for verify cache flush, write in LK, flush in preloader
        do {
        }while(tmp);

        for (i=0;i<0x100;i=i+4)
        {
            *(volatile u32 *)(CFG_DRAM_ADDR + 0x120000 + i) = 0xFFFFFFFF;
        }
    }
#endif
    apmcu_dcache_clean_invalidate();
    print("%scpu0 flush ok\n", MOD);
//    while(1);
#endif //#if defined(CFG_MEM_PRESERVED_MODE)


#if defined(CFG_MEM_PRESERVED_MODE)
#if 0
    while( 0x22000001 != DRV_Reg32(0x10007000))
    {
        mdelay(100);
    }
#endif

#if 0
    {
        u32 i;

        DRV_WriteReg32(0x10007000, 0x22000000);
        printf("start fill in EMI init data pattern:\n");
        for (i=0x100000;i<0x20000000;i=i+4)
        {
            *(volatile u32 *)(CFG_DRAM_ADDR + i) = (i & 0xFFFFFFFF);
        }
    }
#endif
    new_stack = (BSS_TCM_END - 4);
    __asm__ volatile(
    "str    sp, [%1]\n\t"
    "sub    %0, #4\n\t"
    "mov    sp, %1\n\t"
    : "=r" (new_stack)
    : "0" (new_stack)
    : "memory");
#endif //#if defined(CFG_MEM_PRESERVED_MODE)

    // use normal boot preloader to avoid display fail issue,
    // when memory preserved mode, normal preloader do not init EMI
#if !defined(CFG_MEM_PRESERVED_MODE)
    if (TRUE != mtk_wdt_is_mem_preserved())
#endif
    {
        /* init memory */
        mt_mem_init();
    }

#if defined(CFG_MEM_PRESERVED_MODE)
    __asm__ volatile(
    "mov    %0, sp\n\t"
    "add    %0, #4\n\t"
    "ldr    sp, [%1]\n\t"
    : "=r" (new_stack)
    : "0" (new_stack)
    : "memory");


printf("stack switch ok\n");

#if 0
    {
        u32 i;

        printf("start EMI init data verify:\n");
        for (i=0x100000;i<0x20000000;i=i+4)
        {
            if (*(volatile u32 *)(CFG_DRAM_ADDR + i) != (i & 0xFFFFFFFF))
            {
                printf("EMI init data verify fail\n");
                while(1);
            }
        }
        printf("EMI init data verify pass\n");
        while(1);
    }
#endif
#if 0
    while( 0x22000001 != DRV_Reg32(0x10007000))
    {
        mdelay(100);
    }
#endif
#endif //#if defined(CFG_MEM_PRESERVED_MODE)
    #ifdef PL_PROFILING
    printf("#T#mem_init&tst=%d\n", get_timer(profiling_time));
    #endif

    init_dram_buffer();
    /* switch log buffer to dram */
    log_buf_ctrl(1);

#ifdef MTK_MT8193_SUPPORT
	mt8193_init();
#endif

    #ifdef PL_PROFILING
    profiling_time = get_timer(0);
    #endif
    /* init device storeage */
    ret = boot_device_init();
    print("%sInit Boot Dev:%s(%d)\n", MOD, ret ? "FAIL" : "OK", ret);

    #ifdef PL_PROFILING
    printf("#T#bootdev_init=%d\n", get_timer(profiling_time));
    #endif

#if CFG_EMERGENCY_DL_SUPPORT && !defined(CFG_MEM_PRESERVED_MODE)
    #ifdef PL_PROFILING
    profiling_time = get_timer(0);
    #endif
    /* check if to enter emergency download mode */
    /* Move after dram_inital and boot_device_init.
       Use the excetution time to remove delay time in mtk_kpd_gpio_set()*/
    if (mtk_detect_dl_keys()) {
        platform_emergency_download(CFG_EMERGENCY_DL_TIMEOUT_MS);
    }

    #ifdef PL_PROFILING
    printf("#T#chk_emgdwl=%d\n", get_timer(profiling_time));
    #endif
#endif

#if CFG_REBOOT_TEST
    mtk_wdt_sw_reset();
    while(1);
#endif

}

void platform_post_init(void)
{
    struct ram_console_buffer *ram_console;

    #ifdef PL_PROFILING
    u32 profiling_time;
    profiling_time = 0;
    #endif

    usbdl_wo_battery_forced(0);

#ifdef MTK_POWER_EXT_DETECT
	if (MT_BOARD_PHONE == mt_get_board_type())
#endif
	{
	    #ifdef PL_PROFILING
	    profiling_time = get_timer(0);
	    #endif
	    /* normal boot to check battery exists or not */
	    if (g_boot_mode == NORMAL_BOOT && !hw_check_battery() && usb_accessory_in()) {
	        print("%sWait for bat inst\n", MOD);
	        /* disable pmic pre-charging led */
	        pl_close_pre_chr_led();
	        /* enable force charging mode */
	        pl_charging(1);
	        do {
	            mdelay(300);
	            /* check battery exists or not */
	            if (hw_check_battery())
	                break;
	            /* kick all watchdogs */
	            platform_wdt_all_kick();
	        } while(1);
	        /* disable force charging mode */
	        pl_charging(0);
	    }

	    #ifdef PL_PROFILING
	    printf("#T#bat_detc=%d\n", get_timer(profiling_time));
	    #endif
}

#if CFG_RAM_CONSOLE
    ram_console = (struct ram_console_buffer *)RAM_CONSOLE_ADDR;

    if (ram_console->sig == RAM_CONSOLE_SIG) {
        print("%sram_console->start=0x%x\n", MOD, ram_console->start);
        if (ram_console->start > RAM_CONSOLE_MAX_SIZE)
            ram_console->start = 0;

        // if in memory preserved mode, store the previous rgu status,
        // we will restore it next reboot
        if (TRUE == mtk_wdt_is_mem_preserved()){
            DRV_WriteReg32(SRAMROM_BASE + 0x28, RGUS_PRESERVED_MAGIC);
            DRV_WriteReg32(SRAMROM_BASE + 0x24, g_rgu_status);
            ram_console->hw_status = g_rgu_status;
            //print("cS\n");
        }else if(RGUS_PRESERVED_MAGIC == DRV_Reg32(SRAMROM_BASE + 0x28)){
            ram_console->hw_status = DRV_Reg32(SRAMROM_BASE + 0x24);
            //print("cG\n");
        }else{
            ram_console->hw_status = g_rgu_status;
            //print("cN\n");
        }

        print("%sram_console(0x%x)=0x%x(boot reason)\n", MOD,
            ram_console->hw_status, g_rgu_status);
    }
#endif

    platform_set_boot_args();

#if (!(CFG_FPGA_PLATFORM) && defined(DUMMY_AP))
    // set UART1 to MD
    // for Dummy AP load only
    #ifdef GPIO_UART_URXD1_PIN
    mt_set_gpio_mode(GPIO_UART_URXD1_PIN, GPIO_UART_URXD1_PIN_M_MD_URXD);
    #endif
    #ifdef GPIO_UART_UTXD1_PIN
    mt_set_gpio_mode(GPIO_UART_UTXD1_PIN, GPIO_UART_UTXD1_PIN_M_MD_UTXD);
    #endif
#endif
}

void platform_error_handler(void)
{
    int i = 0;
    /* if log is disabled, re-init log port and enable it */
    if (log_status() == 0) {
        mtk_uart_init(UART_SRC_CLK_FRQ, CFG_LOG_BAUDRATE);
        log_ctrl(1);
    }
    print("PL fatal error...\n");
    sec_util_brom_download_recovery_check();

#if defined(ONEKEY_REBOOT_NORMAL_MODE_PL) || defined(TWOKEY_REBOOT_NORMAL_MODE_PL)
    /* add delay for Long Preessed Reboot count down
       only pressed power key will have this delay */
    print("PL delay for Long Press Reboot\n");
    for ( i=3; i > 0;i-- ) {
        if (mtk_detect_key(MTK_PMIC_PWR_KEY)) {
        //if (mtk_detect_key(8)) {
            platform_wdt_kick();
            mdelay(5000);   //delay 5s/per kick,
        } else {
            break; //Power Key Release,
        }
    }
#endif

    /* enter emergency download mode */
    #if CFG_EMERGENCY_DL_SUPPORT
    platform_emergency_download(CFG_EMERGENCY_DL_TIMEOUT_MS);
    #endif

    while(1);
}

void platform_assert(char *file, int line, char *expr)
{
    print("<ASSERT>%s:line:%d %s\n", file, line, expr);
    platform_error_handler();
}

static int gic_init(void)
{
    unsigned int max_irq, i;
    unsigned int cpumask = 1;

    //1. Initialize Distributor.
    cpumask |= cpumask << 8;
    cpumask |= cpumask << 16;

    *(volatile unsigned int *)(GIC_DIST_BASE + GIC_DIST_CTRL) = 0;

    /*
     * Find out how many interrupts are supported.
     */
    max_irq = *(volatile unsigned int *)(GIC_DIST_BASE + GIC_DIST_CTR) & 0x1f;
    max_irq = (max_irq + 1) * 32;

    /*
     * The GIC only supports up to 1020 interrupt sources.
     * Limit this to either the architected maximum, or the
     * platform maximum.
     */
    if (max_irq > (unsigned int)max(1020, NUM_IRQ_SOURCES))
                max_irq = max(1020, NUM_IRQ_SOURCES);

    /*
     * Set all global interrupts to be level triggered, active low.
     */
    for (i = 32; i < max_irq; i += 16)
        *(volatile unsigned int *)(GIC_DIST_BASE + GIC_DIST_CONFIG + i * 4 / 16) = 0;

    /*
     * Set all global interrupts to this CPU only.
     */
    for (i = 32; i < max_irq; i += 4)
        *(volatile unsigned int *)(GIC_DIST_BASE + GIC_DIST_TARGET + i * 4 / 4) = cpumask;

    /*
     * Set priority on all interrupts.
     */
    for (i = 32; i < max_irq; i += 4)
        *(volatile unsigned int *)(GIC_DIST_BASE + GIC_DIST_PRI + i * 4 / 4) = 0xa0a0a0a0;

    /*
     * Clear-Enable all interrupts.
     */
    for (i = 0; i < max_irq; i += 32)
        *(volatile unsigned int *)(GIC_DIST_BASE + GIC_DIST_ENABLE_CLEAR + i * 4 / 32) = 0xffffffff;

    *(volatile unsigned int *)(GIC_DIST_BASE + GIC_DIST_CTRL) = 1;

    //2. Initialize CPU Interface.

	/* set priority on PPI and SGI interrupts */
	for (i = 0; i < 32; i += 4)
	    *(volatile unsigned int *)(GIC_DIST_BASE + GIC_DIST_PRI + i * 4 / 4) = 0x80808080;


    *(volatile unsigned int *)(GIC_CPU_BASE + GIC_CPU_PRIMASK) = 0xf0;
    *(volatile unsigned int *)(GIC_CPU_BASE + GIC_CPU_CTRL) = 0x1;

	__asm__ __volatile__ ("dsb");
    return 0;
}

static void __bootup_slave_cpu(u32 cpu, u32 irq)
{
    int satt;
    u32  val;

    //satt = (irq == CPU_BRINGUP_SGI)? 0: (1 << 15);
    satt = 0 << 15;

    val = (cpu << 16) | satt | irq;
    /*
     * Ensure that stores to Normal memory are visible to the
     * other CPUs before issuing the IPI.
     */
	 __asm__ __volatile__ ("dsb");
    *(volatile unsigned int *)(GIC_DIST_BASE + 0xf00) = val;
	__asm__ __volatile__ ("dsb");
}

void bootup_slave_cpu(void)
{
    gic_init();
#if !(CFG_FPGA_PLATFORM)
    *(volatile unsigned int *)(SLAVE_JUMP_REG)   = (unsigned int) OtherCoreHandler;
    *(volatile unsigned int *)(SLAVE1_MAGIC_REG) = SLAVE1_MAGIC_NUM;

    __asm__ __volatile__ ("dsb");
    __bootup_slave_cpu(0x2, 14);
#else
// mask for early porting
//    slave_cpu_trigger = 0x1;
//    __clean_dcache();
    __asm__ __volatile__ ("dsb");
#endif
}

unsigned int mtk_wdt_is_mem_preserved(void)
{
    volatile unsigned int wdt_dramc_ctl;

    wdt_dramc_ctl  = DRV_Reg32(MTK_WDT_DRAMC_CTL);
    if (MTK_WDT_MCU_RG_DRAMC_SREF == (wdt_dramc_ctl & MTK_WDT_MCU_RG_DRAMC_SREF)) {
        return TRUE;
    }
    else {
        return FALSE;
    }
}

#if defined(CFG_SRAM_PRELOADER_MODE) || defined(CFG_MEM_PRESERVED_MODE)
void mtk_wdt_presrv_mode_config(unsigned int ddr_resrv_en, unsigned int mcu_latch_en)
{
    unsigned int tmp;

    tmp = DRV_Reg32(MTK_WDT_MODE);
    tmp |= MTK_WDT_MODE_KEY;

	if(0 == ddr_resrv_en)
		tmp &= ~MTK_WDT_MODE_DDRRSV_MODE;
    else
		tmp |= MTK_WDT_MODE_DDRRSV_MODE;
    DRV_WriteReg32(MTK_WDT_MODE,tmp);
}
/*
 * 10200000
 * 4		L2RSTDISABLE	0: L2 cache is reset by hardware.
 *                          1: L2 cache is not reset by hardware..
 * 1:0		L1RSTDISABLE	0: L1 cache is reset by hardware.
 *                          1: L1 cache is not reset by hardware..
*/

#define MCUSYS_L2RSTDISABLE_MASK    (0x10)
#define MCUSYS_L1RSTDISABLE_MASK    (0x03)
#define EMI_GENA                    (EMI_BASE + 0x70)
#define MEM_PSRV_REQ_EN             (0x4)
#define MEM_PRESERVED_MAGIC         (0x504D454D)

static void mtk_mcusys_l1_rst_config(unsigned int rst_enable)
{
    volatile unsigned int cfgreg;

    cfgreg  = DRV_Reg32(MCUSYS_CFGREG_BASE);
    if (0 == rst_enable)    //reset disable
        cfgreg |= MCUSYS_L1RSTDISABLE_MASK;
    else
        cfgreg &= (~MCUSYS_L1RSTDISABLE_MASK);

    DRV_WriteReg32(MCUSYS_CFGREG_BASE, cfgreg);
}

static void mtk_mcusys_l2_rst_config(unsigned int rst_enable)
{
    volatile unsigned int cfgreg;

    cfgreg  = DRV_Reg32(MCUSYS_CFGREG_BASE);
    if (0 == rst_enable)    //reset disable
        cfgreg |= MCUSYS_L2RSTDISABLE_MASK;
    else
        cfgreg &= (~MCUSYS_L2RSTDISABLE_MASK);

    DRV_WriteReg32(MCUSYS_CFGREG_BASE, cfgreg);
}

void platform_mem_preserved_disable(void)
{
    volatile unsigned int cfgreg;
    unsigned int enable;
    enable = 0;
/*
    printf("====WDT register 0x80, 0x02====\n");
    printf("REG(0x%X)=0x%X\n", TOP_RGU_BASE, DRV_Reg32(TOP_RGU_BASE));
    printf("REG(0x%X)=0x%X\n", (TOP_RGU_BASE + 0x40), DRV_Reg32((TOP_RGU_BASE + 0x40)));
    printf("====MCUSYS register 0x13====\n");
    printf("REG(0x%X)=0x%X\n", MCUSYS_CFGREG_BASE, DRV_Reg32(MCUSYS_CFGREG_BASE));
    printf("====EMI GENA register 0x4 ====\n");
    printf("REG(0x%X)=0x%X\n", EMI_GENA, DRV_Reg32(EMI_GENA));
    printf("====SRAMROM_BASE 0x24, 0x28 ====\n");
    printf("REG(0x%X)=0x%X\n", SRAMROM_BASE + 0x24, DRV_Reg32(SRAMROM_BASE + 0x24));
    printf("REG(0x%X)=0x%X\n", SRAMROM_BASE + 0x28, DRV_Reg32(SRAMROM_BASE + 0x28));
*/
    //cannot do here, we must clear it after get boot status
//    mtk_wdt_presrv_mode_config(enable,enable);

    //enable memory preserved mode, do not reset
    mtk_mcusys_l1_rst_config(!(enable));
    mtk_mcusys_l2_rst_config(!(enable));

    cfgreg  = DRV_Reg32(EMI_GENA);
    if (0 == enable)
        cfgreg &= (~MEM_PSRV_REQ_EN);
    else
        cfgreg |= MEM_PSRV_REQ_EN;

    DRV_WriteReg32(EMI_GENA, cfgreg);

    if (0 == enable){
        //prepare the entry for BROM jump(0x10001424)
        DRV_WriteReg32(SRAMROM_BASE + 0x24, 0x0);
        DRV_WriteReg32(SRAMROM_BASE + 0x28, 0x0);
    }
/*
    printf("====WDT register 0x80, 0x02====\n");
    printf("REG(0x%X)=0x%X\n", TOP_RGU_BASE, DRV_Reg32(TOP_RGU_BASE));
    printf("REG(0x%X)=0x%X\n", (TOP_RGU_BASE + 0x40), DRV_Reg32((TOP_RGU_BASE + 0x40)));
    printf("====MCUSYS register 0x13====\n");
    printf("REG(0x%X)=0x%X\n", MCUSYS_CFGREG_BASE, DRV_Reg32(MCUSYS_CFGREG_BASE));
    printf("====EMI GENA register 0x4 ====\n");
    printf("REG(0x%X)=0x%X\n", EMI_GENA, DRV_Reg32(EMI_GENA));
    printf("====SRAMROM_BASE 0x24, 0x28 ====\n");
    printf("REG(0x%X)=0x%X\n", SRAMROM_BASE + 0x24, DRV_Reg32(SRAMROM_BASE + 0x24));
    printf("REG(0x%X)=0x%X\n", SRAMROM_BASE + 0x28, DRV_Reg32(SRAMROM_BASE + 0x28));
*/
}

#endif //#if defined(CFG_SRAM_PRELOADER_MODE) || defined(CFG_MEM_PRESERVED_MODE)
