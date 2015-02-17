/*
 * Copyright (c) 2012, Code Aurora Forum. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  * Neither the name of Google, Inc. nor the names of its contributors
 *    may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <debug.h>
#include <dev/uart.h>
#include <arch/arm/mmu.h>
#include <arch/ops.h>

#include <platform/boot_mode.h>
#include <platform/mt_reg_base.h>
#include <mt_partition.h>
#include <platform/mt_pmic.h>
#include <platform/mt_i2c.h>
#include <video.h>
#include <stdlib.h>
#include <string.h>
#include <target/board.h>
#include <platform/mt_logo.h>
#include <platform/mt_gpio.h>
#include <platform/mtk_key.h>
#include <platform/mt_pmic_wrap_init.h>
#include <platform/pll.h>
//#define LK_DL_CHECK
#ifdef MTK_KERNEL_POWER_OFF_CHARGING
#include <platform/mt_rtc.h>
#include <platform/mt_leds.h>
#endif
#include <platform/mrdump.h>
#include <platform/env.h>
#include <platform/mtk_wdt.h>
#include <target/cust_key.h>    //MT65XX_MENU_OK_KEY

#include <platform/mt_clkmgr.h>
#include <platform/mt_spm.h>
#include <platform/mt_spm_mtcmos.h>
#include <platform/disp_drv.h>
#include <platform/mt_disp_drv.h>
#include <platform/mmc_common_inter.h>
#ifndef MTK_EMMC_SUPPORT
#include <platform/mtk_nand.h>
#endif

#ifdef LK_DL_CHECK
/*block if check dl fail*/
#undef LK_DL_CHECK_BLOCK_LEVEL
#endif

#ifdef DUMMY_AP
#define PART_MAX 10
part_hdr_t part_info_temp[PART_MAX];
#endif

extern void platform_early_init_timer();
extern void isink0_init(void);
extern int mboot_common_load_logo(unsigned long logo_addr, char* filename);
extern int sec_func_init(int dev_type);
extern int sec_usbdl_enabled (void);
extern int sec_usbdl_verify_da(unsigned char*, unsigned int, unsigned char*, unsigned int);
extern void mtk_wdt_disable(void);
extern void platform_deinit_interrupts(void);
extern unsigned int mtk_wdt_is_mem_preserved(void);
extern unsigned int mtk_wdt_mem_preserved_is_enabled(void);
extern int mboot_mem_preserved_load_part(char *part_name, unsigned long sram_addr, unsigned long mem_addr);
extern void mtk_wdt_clear_mem_preserved_status(void);
extern void mtk_wdt_restart(void);

void platform_uninit(void);

/* Transparent to DRAM customize */
int g_nr_bank;
int g_rank_size[4];
unsigned int g_fb_base;
unsigned int g_fb_size;
BOOT_ARGUMENT *g_boot_arg;
BOOT_ARGUMENT  boot_addr;
BI_DRAM bi_dram[MAX_NR_BANK];
BOOT_ARGUMENT g_addr;

extern void jump_da(u32 addr, u32 arg1, u32 arg2);
extern void enable_DA_sram(void);

int clk_init(void)
{
#ifdef MACH_FPGA
    //; SPM
    //; enable MTCMOS power
    //; enable MMSYS power
    //D.S SD:0x1000623C %LE %LONG 0xD
    spm_write(SPM_DIS_PWR_CON, 0xD);

    //; enable MFG power
    //D.S SD:0x10006214 %LE %LONG 0xD
    spm_write(SPM_MFG_PWR_CON, 0xD);
#else   // else of #ifndef MACH_FPGA

    {
        unsigned int i;

        clk_writel(CLR_CLK_GATING_CTRL0,
                   0
                 | GPU_491P52M_EN_BIT       // disable
                 | GPU_500P5M_EN_BIT
                 | SC_MEM_CK_OFF_EN_BIT
                   );

        clk_writel(SET_CLK_GATING_CTRL0,
                   0
                                            // enable
                 | SPM_52M_SW_CG_BIT        // disable
                   );

        clk_writel(CLR_CLK_GATING_CTRL1,
                   0
                 | SPM_SW_CG_BIT            // enable
                 | PMIC_SW_CG_AP_BIT
                   );

        clk_writel(SET_CLK_GATING_CTRL1,
                   0
                 | EFUSE_SW_CG_BIT          // disable
                 | MEMSLP_DLYER_SW_CG_BIT
                 | PMIC_26M_SW_CG_BIT
                 | AUDIO_SW_CG_BIT
                 | SPINFI_SW_CG_BIT
                   );

        clk_writel(SET_CLK_GATING_CTRL7,
                   0
                 | XIU2AHB0_SW_CG_BIT);     // disable

#if defined(MTK_EMMC_SUPPORT)
        // eMMC booting
        clk_writel(SET_CLK_GATING_CTRL1,
                   0
                 | NFI_SW_CG_BIT            // disable
                 | NFIECC_SW_CG_BIT
                 | NFI2X_SW_CG_BIT
                   );

        clk_writel(SET_CLK_GATING_CTRL7,
                   0
                 | NFI_HCLK_SW_CG_BIT    // disable
                   );
#else
    #if defined(MTK_SPI_NAND_SUPPORT)
        // SPI-NAND booting
        clk_writel(CLR_CLK_GATING_CTRL1,
                   0
                 | SPINFI_SW_CG_BIT         // enable
                   );

        clk_writel(SET_CLK_GATING_CTRL1,
                   0
                 | NFI2X_SW_CG_BIT         // disable
                   );
    #endif
        // NAND booting
        clk_writel(SET_CLK_GATING_CTRL1,
                   0
                 | MSDC0_SW_CG_BIT          // disable
                   );
#endif

        spm_write(SPM_POWERON_CONFIG_SET, (SPM_PROJECT_CODE << 16) | (1U << 0));

        spm_write(SPM_DIS_PWR_CON, spm_read(SPM_DIS_PWR_CON) | PWR_ON_BIT);                 // PWR_ON = 1
        udelay(1);                                                                          // delay 1 us
        spm_write(SPM_DIS_PWR_CON, spm_read(SPM_DIS_PWR_CON) | PWR_ON_S_BIT);               // PWR_ON_S = 1
        udelay(3);                                                                          // delay 3 us

        while (   !(spm_read(SPM_PWR_STATUS)   & DIS_PWR_STA_MASK)                          // wait until PWR_ACK = 1
               || !(spm_read(SPM_PWR_STATUS_S) & DIS_PWR_STA_MASK)
               );

        // SRAM_PDN                                                                         // MEM power on
        for (i = BIT(8); i <= BITMASK(11:8); i = (i << 1) + BIT(8))                         // set SRAM_PDN 0 one by one
        {
            spm_write(SPM_DIS_PWR_CON, spm_read(SPM_DIS_PWR_CON) & ~i);
        }

        spm_write(SPM_DIS_PWR_CON, spm_read(SPM_DIS_PWR_CON) & ~PWR_CLK_DIS_BIT);           // PWR_CLK_DIS = 0
        spm_write(SPM_DIS_PWR_CON, spm_read(SPM_DIS_PWR_CON) & ~PWR_ISO_BIT);               // PWR_ISO = 0

        while (   BITMASK(15:12)
               && (spm_read(SPM_DIS_PWR_CON) & BITMASK(15:12))                              // wait until SRAM_PDN_ACK all 0
               );

        spm_write(SPM_DIS_PWR_CON, spm_read(SPM_DIS_PWR_CON) | PWR_CLK_DIS_BIT);            // PWR_CLK_DIS = 1
        spm_write(SPM_DIS_PWR_CON, spm_read(SPM_DIS_PWR_CON) | PWR_RST_B_BIT);              // PWR_RST_B = 1
        spm_write(SPM_DIS_PWR_CON, spm_read(SPM_DIS_PWR_CON) & ~PWR_CLK_DIS_BIT);           // PWR_CLK_DIS = 0
        spm_write(SPM_DIS_PWR_CON, spm_read(SPM_DIS_PWR_CON) | PWR_CLK_DIS_BIT);            // PWR_CLK_DIS = 1
        spm_write(SPM_DIS_PWR_CON, spm_read(SPM_DIS_PWR_CON) | PWR_RST_B_BIT);              // PWR_RST_B = 1
        spm_write(SPM_POWER_ON_VAL0, spm_read(SPM_POWER_ON_VAL0) & ~BIT(4));                // Enable SMI clock
        spm_write(SPM_DIS_PWR_CON, spm_read(SPM_DIS_PWR_CON) & ~PWR_CLK_DIS_BIT);           // PWR_CLK_DIS = 0

        // BUS_PROTECT
        if (0 != BIT(11))
        {
            spm_write(INFRA_TOPAXI_PROTECTEN, spm_read(INFRA_TOPAXI_PROTECTEN) & ~BIT(11));
            while (spm_read(INFRA_TOPAXI_PROTECTSTA1) & BIT(11));
        }

        clk_writel(MMSYS_CG_CLR0,
                   SMI_COMMON_SW_CG_BIT
                 | SMI_LARB0_SW_CG_BIT
                 | MM_CMDQ_ENGINE_SW_CG_BIT
                 | MM_CMDQ_SMI_SW_CG_BIT
                   );
    }
#endif  //end of #ifndef MACH_FPGA
    return 0;
}

int dram_init(void)
{
    int i;
    unsigned int dram_rank_num;

    /* Get parameters from pre-loader. Get as early as possible
     * The address of BOOT_ARGUMENT_LOCATION will be used by Linux later
     * So copy the parameters from BOOT_ARGUMENT_LOCATION to LK's memory region
     */
    g_boot_arg = &boot_addr;
    memcpy(g_boot_arg, (void*)BOOT_ARGUMENT_LOCATION, sizeof(BOOT_ARGUMENT));

#ifdef MACH_FPGA
    g_nr_bank = 1;
    bi_dram[0].start = RESERVE_MEM_SIZE + DRAM_PHY_ADDR;
    //bi_dram[0].size = 0xCB00000;
    // solving low memory issue in FPGA stage
    bi_dram[0].size = 0x0FF00000;
    //bi_dram[1].start = 0x10000000;
    //bi_dram[1].size = 0x10000000;
#else

    dram_rank_num = g_boot_arg->dram_rank_num;

    g_nr_bank = dram_rank_num;

    for (i = 0; i < g_nr_bank; i++)
    {
        g_rank_size[i] = g_boot_arg->dram_rank_size[i];
    }

    if (g_nr_bank == 1)
    {
        bi_dram[0].start = RESERVE_MEM_SIZE + DRAM_PHY_ADDR;
        bi_dram[0].size = g_rank_size[0] - RESERVE_MEM_SIZE;
    } else if (g_nr_bank == 2)
    {
        bi_dram[0].start = RESERVE_MEM_SIZE + DRAM_PHY_ADDR;
        bi_dram[0].size = g_rank_size[0] - RESERVE_MEM_SIZE;
        bi_dram[1].start = g_rank_size[0] + DRAM_PHY_ADDR;
        bi_dram[1].size = g_rank_size[1];
    } else if (g_nr_bank == 3)
    {
        bi_dram[0].start = RESERVE_MEM_SIZE + DRAM_PHY_ADDR;
        bi_dram[0].size = g_rank_size[0] - RESERVE_MEM_SIZE;
        bi_dram[1].start = g_rank_size[0] + DRAM_PHY_ADDR;
        bi_dram[1].size = g_rank_size[1];
        bi_dram[2].start = bi_dram[1].start + bi_dram[1].size;
        bi_dram[2].size = g_rank_size[2];
    } else if (g_nr_bank == 4)
    {
        bi_dram[0].start = RESERVE_MEM_SIZE + DRAM_PHY_ADDR;;
        bi_dram[0].size = g_rank_size[0] - RESERVE_MEM_SIZE;
        bi_dram[1].start = g_rank_size[0] + DRAM_PHY_ADDR;
        bi_dram[1].size = g_rank_size[1];
        bi_dram[2].start = bi_dram[1].start + bi_dram[1].size;
        bi_dram[2].size = g_rank_size[2];
        bi_dram[3].start = bi_dram[2].start + bi_dram[2].size;
        bi_dram[3].size = g_rank_size[3];
    } else
    {
        //dprintf(CRITICAL, "[LK ERROR] DRAM bank number is not correct!!!");
        while (1) ;
    }
#endif
    return 0;
}

/*******************************************************
 * Routine: memory_size
 * Description: return DRAM size to LCM driver
 ******************************************************/
u32 memory_size(void)
{
    int nr_bank = g_nr_bank;
    int i, size = 0;

    for (i = 0; i < nr_bank; i++)
        size += bi_dram[i].size;
    size += RESERVE_MEM_SIZE;

    return size;
}

void sw_env()
{
#ifdef LK_DL_CHECK
    int dl_status = 0;
#ifdef MTK_EMMC_SUPPORT
    dl_status = mmc_get_dl_info();
#else
    dl_status = nand_get_dl_info();
#endif
    printf("mt65xx_sw_env--dl_status: %d\n", dl_status);
    if (dl_status != 0)
    {
        video_printf("=> TOOL DL image Fail!\n");
        printf("TOOL DL image Fail\n");
#ifdef LK_DL_CHECK_BLOCK_LEVEL
        printf("uboot is blocking by dl info\n");
        while (1) ;
#endif
    }
#endif

#ifndef USER_BUILD
    switch (g_boot_mode)
    {
      case META_BOOT:
          video_printf(" => META MODE\n");
          break;
      case FACTORY_BOOT:
          video_printf(" => FACTORY MODE\n");
          break;
      case RECOVERY_BOOT:
          video_printf(" => RECOVERY MODE\n");
          break;
      case SW_REBOOT:
          //video_printf(" => SW RESET\n");
          break;
      case NORMAL_BOOT:
          if(g_boot_arg->boot_reason != BR_RTC && get_env("hibboot") != NULL && atoi(get_env("hibboot")) == 1)
              video_printf(" => HIBERNATION BOOT\n");
          else
              video_printf(" => NORMAL BOOT\n");
          break;
      case ADVMETA_BOOT:
          video_printf(" => ADVANCED META MODE\n");
          break;
      case ATE_FACTORY_BOOT:
          video_printf(" => ATE FACTORY MODE\n");
          break;
#if defined (MTK_KERNEL_POWER_OFF_CHARGING)
	case KERNEL_POWER_OFF_CHARGING_BOOT:
		video_printf(" => POWER OFF CHARGING MODE\n");
		break;
	case LOW_POWER_OFF_CHARGING_BOOT:
		video_printf(" => LOW POWER OFF CHARGING MODE\n");
		break;
#endif
      case ALARM_BOOT:
          video_printf(" => ALARM BOOT\n");
          break;
      case FASTBOOT:
          video_printf(" => FASTBOOT mode...\n");
          break;
      default:
          video_printf(" => UNKNOWN BOOT\n");
    }
    return;
#endif

#ifdef USER_BUILD
    if(g_boot_mode == FASTBOOT)
        video_printf(" => FASTBOOT mode...\n");
#endif

}

void platform_init_mmu_mappings(void)
{
  /* configure available RAM banks */
  dram_init();

/* Enable D-cache  */

  unsigned int offset;
  unsigned int dram_size = 0;

  dram_size = memory_size();

  /* do some memory map initialization */
  for (offset = 0; offset < dram_size; offset += (1024*1024))
  {
    /*virtual to physical 1-1 mapping*/
    arm_mmu_map_section(DRAM_PHY_ADDR+offset, DRAM_PHY_ADDR+offset, MMU_MEMORY_TYPE_NORMAL_WRITE_BACK_ALLOCATE | MMU_MEMORY_AP_READ_WRITE);
  }

}

void platform_early_init(void)
{
#ifdef LK_PROFILING
    unsigned int time_led_init;
    unsigned int time_pmic6329_init;
    unsigned int time_i2c_init;
    unsigned int time_disp_init;
    unsigned int time_platform_early_init;
    unsigned int time_set_clock;
    unsigned int time_disp_preinit;
    unsigned int time_misc_init;
    unsigned int time_clock_init;
    unsigned int time_wdt_init;

    time_platform_early_init = get_timer(0);
    time_set_clock = get_timer(0);
#endif
    //mt_gpio_set_default();

    //Specific for MT6572. ARMPLL can't set to max speed when L2 is configured as SRAM.
    //preloader won't reach max speed. It will done by LK.
    if (g_boot_arg->boot_mode != DOWNLOAD_BOOT)
    {
    mtk_set_arm_clock();
    }

    /* initialize the uart */
    uart_init_early();

    printf("arm clock set finished\n");

#ifdef LK_PROFILING
    printf("[PROFILE] ------- set clock takes %d ms -------- \n", get_timer(time_set_clock));
    time_misc_init = get_timer(0);
#endif

    platform_init_interrupts();
    platform_early_init_timer();
    mt_gpio_set_default();

    /* initialize the uart */
    uart_init_early();
#ifdef LK_PROFILING
    printf("[PROFILE] -------misc init takes %d ms -------- \n", get_timer(time_misc_init));
    time_i2c_init = get_timer(0);
#endif

    mt_i2c_init();

#ifdef LK_PROFILING
    printf("[PROFILE] ------- i2c init takes %d ms -------- \n", get_timer(time_i2c_init));
    time_clock_init = get_timer(0);
#endif

    clk_init();

#ifdef LK_PROFILING
    printf("[PROFILE] ------- clock init takes %d ms -------- \n", get_timer(time_clock_init));
    time_wdt_init = get_timer(0);
#endif

    mtk_wdt_init();

#ifdef LK_PROFILING
    printf("[PROFILE] ------- wdt init takes %d ms -------- \n", get_timer(time_wdt_init));
    time_disp_preinit = get_timer(0);
#endif

#ifdef DUMMY_AP
    memcpy(&part_info_temp, (unsigned int*)g_boot_arg->part_info, (sizeof(part_hdr_t)*((unsigned int)g_boot_arg->part_num)));
    g_boot_arg->part_info = &part_info_temp;
#endif

    /* initialize the frame buffet information */
    g_fb_size = mt_disp_get_vram_size();
    g_fb_base = memory_size() - g_fb_size + DRAM_PHY_ADDR;

    dprintf(INFO, "FB base = 0x%x, FB size = %d\n", g_fb_base, g_fb_size);

#ifdef LK_PROFILING
    printf("[PROFILE] -------disp preinit takes %d ms -------- \n", get_timer(time_disp_preinit));
    time_led_init = get_timer(0);
#endif

#ifndef MACH_FPGA
    leds_init();
#endif

    isink0_init();              //turn on PMIC6329 isink0

#ifdef LK_PROFILING
    printf("[PROFILE] ------- led init takes %d ms -------- \n", get_timer(time_led_init));
    time_disp_init = get_timer(0);
#endif

    mt_disp_init((void *)g_fb_base);

#ifdef CONFIG_CFB_CONSOLE
    drv_video_init();
#endif

#ifdef LK_PROFILING
    printf("[PROFILE] ------- disp init takes %d ms -------- \n", get_timer(time_disp_init));
    time_pmic6329_init = get_timer(0);
#endif

//#ifndef MACH_FPGA
//    pwrap_init_lk();
//    pwrap_init_for_early_porting();
//#endif

    //pmic6320_init();  //[TODO] init PMIC

#ifdef LK_PROFILING
    printf("[PROFILE] ------- pmic6329_init takes %d ms -------- \n", get_timer(time_pmic6329_init));
    printf("[PROFILE] ------- platform_early_init takes %d ms -------- \n", get_timer(time_platform_early_init));
#endif
}

extern void mt65xx_bat_init(void);
#ifdef MTK_MT8193_SUPPORT
extern int mt8193_init(void);
#endif

#if defined (MTK_KERNEL_POWER_OFF_CHARGING)

int kernel_charging_boot(void)
{
	if((g_boot_mode == KERNEL_POWER_OFF_CHARGING_BOOT || g_boot_mode == LOW_POWER_OFF_CHARGING_BOOT) && upmu_is_chr_det() == KAL_TRUE)
	{
		printf("[%s] Kernel Power Off Charging with Charger/Usb \n", __func__);
		return  1;
	}
	else if((g_boot_mode == KERNEL_POWER_OFF_CHARGING_BOOT || g_boot_mode == LOW_POWER_OFF_CHARGING_BOOT) && upmu_is_chr_det() == KAL_FALSE)
	{
		printf("[%s] Kernel Power Off Charging without Charger/Usb \n", __func__);
		return -1;
	}
	else
		return 0;
}
#endif

void platform_init(void)
{

    /* init timer */
    //mtk_timer_init();
#ifdef LK_PROFILING
    unsigned int time_nand_emmc;
    unsigned int time_load_logo;
    unsigned int time_bat_init;
    unsigned int time_backlight;
    unsigned int time_show_logo;
    unsigned int time_boot_mode;
    unsigned int time_sw_env;
    unsigned int time_download_boot_check;
    unsigned int time_rtc_check;
    unsigned int time_platform_init;
    unsigned int time_sec_init;
    unsigned int time_env;
    time_platform_init = get_timer(0);
    time_nand_emmc = get_timer(0);
#endif
    dprintf(INFO, "platform_init()\n");

#ifdef DUMMY_AP
	dummy_ap_entry();
#endif

#ifdef MTK_MT8193_SUPPORT
	mt8193_init();
#endif

#if defined(MEM_PRESERVED_MODE_ENABLE)
    // init eMMC/NAND driver only in normal boot, not in memory dump
    if (TRUE != mtk_wdt_is_mem_preserved())
#endif
    {
#ifdef MTK_EMMC_SUPPORT
        mmc_legacy_init(1);
#else
        nand_init();
        nand_driver_test();
#endif
    }

#ifdef LK_PROFILING
    printf("[PROFILE] ------- NAND/EMMC init takes %d ms -------- \n", get_timer(time_nand_emmc));
    time_env = get_timer(0);
#endif

#ifdef MTK_KERNEL_POWER_OFF_CHARGING
	if((g_boot_arg->boot_reason == BR_USB) && (upmu_is_chr_det() == KAL_FALSE))
	{
		printf("[%s] Unplugged Charger/Usb between Pre-loader and Uboot in Kernel Charging Mode, Power Off \n", __func__);
		mt6575_power_off();
	}
#endif

	env_init();
	print_env();
#ifdef LK_PROFILING
	printf("[PROFILE] ------- ENV init takes %d ms -------- \n", get_timer(time_env));
	time_load_logo = get_timer(0);
#endif

#if defined(MEM_PRESERVED_MODE_ENABLE)
    // only init eMMC/NAND driver in normal boot, not in memory dump
    if (TRUE != mtk_wdt_is_mem_preserved())
#endif
    {
        mboot_common_load_logo((unsigned long)mt_get_logo_db_addr(), "logo");
    }
    dprintf(INFO, "Show BLACK_PICTURE\n");
    mt_disp_power(TRUE);
    mt_disp_fill_rect(0, 0, CFG_DISPLAY_WIDTH, CFG_DISPLAY_HEIGHT, 0x0);
    mt_disp_update(0, 0, CFG_DISPLAY_WIDTH, CFG_DISPLAY_HEIGHT);
    mt_disp_update(0, 0, CFG_DISPLAY_WIDTH, CFG_DISPLAY_HEIGHT);
#ifdef LK_PROFILING
    printf("[PROFILE] ------- load_logo takes %d ms -------- \n", get_timer(time_load_logo));
    time_backlight = get_timer(0);
#endif

    /*for kpd pmic mode setting*/
    set_kpd_pmic_mode();
#ifndef DISABLE_FOR_BRING_UP
    mt65xx_backlight_on();
#endif

#ifdef LK_PROFILING
    printf("[PROFILE] ------- backlight takes %d ms -------- \n", get_timer(time_backlight));
    time_boot_mode = get_timer(0);
#endif
    enable_PMIC_kpd_clock();
#if defined(MEM_PRESERVED_MODE_ENABLE)
    // when memory preserved mode, do not check KPOC (in boot_mode_selec()), which will power off the target
    if (TRUE != mtk_wdt_is_mem_preserved())
#endif
    {
        boot_mode_select();
    }
#ifdef LK_PROFILING
    printf("[PROFILE] ------- boot mode select takes %d ms -------- \n", get_timer(time_boot_mode));
    time_sec_init = get_timer(0);
#endif

    /* initialize security library */
#ifdef MTK_EMMC_SUPPORT
    sec_func_init(1);
#else
    sec_func_init(0);
#endif

#ifdef LK_PROFILING
        printf("[PROFILE] ------- sec init takes %d ms -------- \n", get_timer(time_sec_init));
        time_download_boot_check = get_timer(0);
#endif

    /*Show download logo & message on screen */
    if (g_boot_arg->boot_mode == DOWNLOAD_BOOT)
    {
	printf("[LK] boot mode is DOWNLOAD_BOOT\n");
	/* verify da before jumping to da*/
	if (sec_usbdl_enabled()) {
	    u8  *da_addr = (u8*)g_boot_arg->da_info.addr;
	    u32 da_sig_len = DRV_Reg32(SRAMROM_BASE + 0x30);
	    u32 da_len   = da_sig_len >> 10;
	    u32 sig_len  = da_sig_len & 0x3ff;
	    u8  *sig_addr = (unsigned char *)da_addr + (da_len - sig_len);

	    if (da_len == 0 || sig_len == 0) {
		printf("[LK] da argument is invalid\n");
		printf("da_addr = 0x%x\n", da_addr);
		printf("da_len  = 0x%x\n", da_len);
		printf("sig_len = 0x%x\n", sig_len);
	    }

	    if (sec_usbdl_verify_da(da_addr, (da_len - sig_len), sig_addr, sig_len)) {
		/* da verify fail */
                video_printf(" => Not authenticated tool, download stop...\n");
		DRV_WriteReg32(SRAMROM_BASE + 0x30, 0x0);
		while(1); /* fix me, should not be infinite loop in lk */
	    }
	}
	else {
	    printf(" DA verification disabled...\n");
	}

	/* clear da length and da signature length information */
	DRV_WriteReg32(SRAMROM_BASE + 0x30, 0x0);

        mt_disp_show_boot_logo();
        video_printf(" => Downloading...\n");
#ifndef DISABLE_FOR_BRING_UP
        mt65xx_backlight_on();
#endif
        mtk_wdt_disable();//Disable wdt before jump to DA
        platform_uninit();
#ifdef HAVE_CACHE_PL310
        l2_disable();
#endif
        arch_disable_cache(UCACHE);
        arch_disable_mmu();

        //enable sram for DA usage
        enable_DA_sram();
        jump_da(g_boot_arg->da_info.addr, g_boot_arg->da_info.arg1, g_boot_arg->da_info.arg2);
    }


#ifdef LK_PROFILING
    printf("[PROFILE] ------- download boot check takes %d ms -------- \n", get_timer(time_download_boot_check));
    time_bat_init = get_timer(0);
#endif
    mt65xx_bat_init();
#ifdef LK_PROFILING
    printf("[PROFILE] ------- battery init takes %d ms -------- \n", get_timer(time_bat_init));
    time_rtc_check = get_timer(0);
#endif

#ifndef CFG_POWER_CHARGING
    /* NOTE: if define CFG_POWER_CHARGING, will rtc_boot_check() in mt65xx_bat_init() */
    rtc_boot_check(false);
#endif

#ifdef LK_PROFILING
    printf("[PROFILE] ------- rtc check takes %d ms -------- \n", get_timer(time_rtc_check));
    time_show_logo = get_timer(0);
#endif

#if defined(MEM_PRESERVED_MODE_ENABLE)
    // when memory preserved mode, do not check KPOC (in boot_mode_selec()), which will power off the target
    if (TRUE != mtk_wdt_is_mem_preserved())
#endif
    {
#ifdef MTK_KERNEL_POWER_OFF_CHARGING
    	if(kernel_charging_boot() == 1)
    	{

    		mt_disp_power(TRUE);
    		mt_disp_show_low_battery();
    		mt_disp_wait_idle();
#ifndef DISABLE_FOR_BRING_UP
    		mt65xx_leds_brightness_set(6, 110);
#endif
    	}
    	else if(g_boot_mode != KERNEL_POWER_OFF_CHARGING_BOOT && g_boot_mode != LOW_POWER_OFF_CHARGING_BOOT)
    	{
#ifndef MACH_FPGA
    		if (g_boot_mode != ALARM_BOOT && (g_boot_mode != FASTBOOT))
    		{
    			mt_disp_show_boot_logo();
    		}
#endif
    	}
#else
    	if (g_boot_mode != ALARM_BOOT && (g_boot_mode != FASTBOOT))
    	{
    		mt_disp_show_boot_logo();
    	}
#endif
    }   //if (TRUE != mtk_wdt_is_mem_preserved())


#ifdef LK_PROFILING
    printf("[PROFILE] ------- show logo takes %d ms -------- \n", get_timer(time_show_logo));
    time_sw_env= get_timer(0);
#endif
	//mt_i2c_test();

    sw_env();

#ifdef LK_PROFILING
    printf("[PROFILE] ------- sw_env takes %d ms -------- \n", get_timer(time_sw_env));
    printf("[PROFILE] ------- platform_init takes %d ms -------- \n", get_timer(time_platform_init));
#endif
}

void platform_uninit(void)
{
#ifndef DISABLE_FOR_BRING_UP
    leds_deinit();
#endif
	platform_deinit_interrupts();
}

#ifdef ENABLE_L2_SHARING
#define L2C_SIZE_CFG_OFF 5
/* config L2 cache and sram to its size */
void config_L2_size(void)
{
    volatile unsigned int cache_cfg;
    /* set L2C size to 256KB */
    cache_cfg = DRV_Reg(MCUSYS_CFGREG_BASE);
    cache_cfg &= ~(0x7 << L2C_SIZE_CFG_OFF);
    cache_cfg |= 0x1 << L2C_SIZE_CFG_OFF;
    DRV_WriteReg(MCUSYS_CFGREG_BASE, cache_cfg);
}

void enable_DA_sram(void)
{
    volatile unsigned int cache_cfg;

    //enable L2 sram for DA
    cache_cfg = DRV_Reg(MCUSYS_CFGREG_BASE);
    cache_cfg &= ~(0x7 << L2C_SIZE_CFG_OFF);
    DRV_WriteReg(MCUSYS_CFGREG_BASE, cache_cfg);

    //enable audio sysram clk for DA
    *(volatile unsigned int *)(0x10000084) = 0x02000000;
}
#endif

#if defined(MEM_PRESERVED_MODE_ENABLE)
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
#define RGUS_PRESERVED_MAGIC        (0x53554752)    //store RGU status, RGUS
#define WAIT_CARD_INSERT_TIME       (120)    //250ms

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

void platform_mem_preserved_config(unsigned int enable)
{
    volatile unsigned int cfgreg;

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

    mtk_wdt_presrv_mode_config(enable,enable);

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
        // prepare the entry for BROM jump(0x10001424)
        // if magic num is equal to RGUS_PRESERVED_MAGIC,
        // do not clean the SRAMROM register, just keep for RGU status restore
        if ( RGUS_PRESERVED_MAGIC != DRV_Reg32(SRAMROM_BASE + 0x28)) {
            DRV_WriteReg32(SRAMROM_BASE + 0x24, 0x0);
            DRV_WriteReg32(SRAMROM_BASE + 0x28, 0x0);
        }
    }else{
        //jump to mem preloader directly, skip SRAM preloader in MT6571,
        // to avoid data corruption by Device APC
        DRV_WriteReg32(SRAMROM_BASE + 0x28, MEM_PRESERVED_MAGIC);
        //DRV_WriteReg32(SRAMROM_BASE + 0x24, MEM_SRAM_PRELOADER_START);
        DRV_WriteReg32(SRAMROM_BASE + 0x24, MEM_MEM_PRELOADER_START);
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

void platform_mem_preserved_load_img(void)
{
    //enter memory preserved mode
    //if ((TRUE == mtk_wdt_mem_preserved_is_enabled()) && (TRUE != mtk_wdt_is_mem_preserved()))
    //always load sram/mem preloader when sysfs decide enter memory preserved mode
    if (TRUE != mtk_wdt_is_mem_preserved())
    {
#if 0
        char * name;
        unsigned int start_addr;
        unsigned int size;
#endif
        unsigned int tmp;

        tmp = DRV_Reg32(TOP_RGU_BASE);
        tmp = (tmp & ~(0x1));
        tmp = (tmp | 0x22000000);
        DRV_WriteReg32(TOP_RGU_BASE,tmp);
        printf("mem pre mode enable=0x%x\n",mtk_wdt_mem_preserved_is_enabled());
#if 1
        mboot_mem_preserved_load_part(PART_UBOOT, MEM_SRAM_PRELOADER_START, MEM_PRELOADER_START);
#else
        name = PART_SRAM_PRELD;
        start_addr = MEM_PRELOADER_START;
        size = MEM_SRAM_PRELOADER_SIZE;
        printf("[MEM] ------- start DMA and Copy part[%s] start=0x%x, size=0x%x -------- \n",name,start_addr, size );
        mboot_recovery_load_raw_part(name, start_addr, size );


        /* relocate mem sram preloader into On-Chip SRAM */
        printf("[MEM] ------- start memcpy part[%s] start=0x%x, size=0x%x -------- \n",name,MEM_SRAM_PRELOADER_START, size );
        memcpy((char *)MEM_SRAM_PRELOADER_START, (char *)start_addr, size);

        name = PART_MEM_PRELD;
        start_addr = MEM_PRELOADER_START;
        size = MEM_PRELOADER_SIZE;
        printf("[MEM] ------- start DMA part[%s] start=0x%x, size=0x%x -------- \n",name,start_addr, size );
        mboot_recovery_load_raw_part(name, start_addr, size );
#endif
#if 0
        printf("[MEM] write pattern into EMI through MMU/cache \n");
        for (tmp=0;tmp<0x100;tmp=tmp+4)
        {
            *(volatile u32 *)(DRAM_PHY_ADDR + 0x120000 + tmp) = tmp;
        }
#endif
#if 0
        printf("[MEM] 72dead loop, waiting for WDT \n");
        mtk_wdt_mem_preserved_hw_reset();
        while(1);
#endif
    }
}

void platform_mem_preserved_dump_mem(void)
{
    //enter memory preserved mode
    if (TRUE == mtk_wdt_is_mem_preserved())
    {
        struct mrdump_regset per_cpu_regs[NR_CPUS];
        struct mrdump_regpair regpairs[9];

        unsigned int i, reg_addr, key_pressed;
        int timeout;

#if 0
        unsigned int tmp;
        // do not disable WDT
        tmp = DRV_Reg32(TOP_RGU_BASE);
        tmp = (tmp & ~(0x1));
        tmp = (tmp | 0x22000000);
        DRV_WriteReg32(TOP_RGU_BASE,tmp);
#endif
        printf ("wdt_flag=0x%x\n",mtk_wdt_is_mem_preserved());
        mtk_wdt_clear_mem_preserved_status();
        // for clear memory preserved mode setting,
        // clear in preloader (as early as possible)
        //platform_mem_preserved_config(0);
        printf ("after clear wdt_flag=0x%x\n",mtk_wdt_is_mem_preserved());

        memset(per_cpu_regs, 0, sizeof(per_cpu_regs));
        per_cpu_regs[0].pc = DRV_Reg32(DBG_CORE0_PC);
        per_cpu_regs[0].fp = DRV_Reg32(DBG_CORE0_FP);
        per_cpu_regs[0].sp = DRV_Reg32(DBG_CORE0_SP);

        per_cpu_regs[1].pc = DRV_Reg32(DBG_CORE1_PC);
        per_cpu_regs[1].fp = DRV_Reg32(DBG_CORE1_FP);
        per_cpu_regs[1].sp = DRV_Reg32(DBG_CORE1_SP);

        //dump AHBABT Monitor register
        memset(regpairs, 0, sizeof(regpairs));

        for ( i = 0 ; i < 8 ; i++)
        {
            reg_addr = AHBABT_ADDR1 + (i*4);
            regpairs[0].addr = reg_addr;
            regpairs[0].val = DRV_Reg32(reg_addr);
        }
        // end of regpairs, set addr and val to 0
        regpairs[8].addr = 0;
        regpairs[8].val = 0;

        printf ("==== 72we are in memory preserved mode====\n");

        mt_set_gpio_mode(CARD_DETECT_PIN,4);
        // if no card is insert, remind inser sd card
        if (0 != mt_get_gpio_in(CARD_DETECT_PIN))
        {
            printf("Please Insert SD card for dump\n");
            printf("Press[VOL DOWN] to continue\n");
#if defined(MEM_PRESERVED_MODE_VIDEO_PRINT)
            video_printf("Please Insert SD card for dump\n");
            video_printf("Press[VOL DOWN] to continue\n");
#endif
            mtk_wdt_restart();


            video_printf("Reset count down %d ...\n", (WAIT_CARD_INSERT_TIME>>2));
            mtk_wdt_restart();

            timeout = WAIT_CARD_INSERT_TIME;
            key_pressed = 0;
            while(timeout-- >= 0) {
                mdelay(250);
                mtk_wdt_restart();
                if (mtk_detect_key(MT65XX_MENU_OK_KEY)) {
                    key_pressed = 1;
                    break;
                }
                video_printf("\rsec %2d", (timeout>>2));
            }

            // timeout, reboot
            if ( 0 == key_pressed)
            {
                mtk_arch_reset(1);
            }

        }

#if defined(MEM_PRESERVED_MODE_VIDEO_PRINT)
        video_printf("[MEM_PRE]Start Dump\n");
#endif
        mrdump_run(per_cpu_regs, regpairs);
    }
}
#endif ////#if defined(MEM_PRESERVED_MODE_ENABLE)
