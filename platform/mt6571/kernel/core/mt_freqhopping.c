/*
 * Copyright (C) 2011 MediaTek, Inc.
 *
 * Author: Holmes Chiou <holmes.chiou@mediatek.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#define __MT_FREQHOPPING_C__

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <asm/sched_clock.h>
#include <linux/vmalloc.h>
#include <linux/dma-mapping.h>
#include <board-custom.h>


#include "mach/mt_fhreg.h"
#include "mach/mt_clkmgr.h"
#include "mach/mt_freqhopping.h"
#include "mach/mt_typedefs.h"
#include "mach/mt_gpio.h"
#include "mach/mt_gpufreq.h"
#include "mach/mt_cpufreq.h"
#include "mach/emi_bwl.h"
#include "mach/sync_write.h"
#include "mach/mt_sleep.h"
#include "mach/mt_idle.h"

#include <mach/mt_freqhopping_drv.h>
///////////////////////////////////////////////////////////
// Local Defination
///////////////////////////////////////////////////////////

#ifdef __FHCTL_CTP__
	#define FHCTL_STATIC
#else
	#define FHCTL_STATIC static
#endif //- __FHCTL_CTP__

#define FH_BUG_ON(x) \
do {    \
		if((x)){ \
			printk("BUGON %s:%d %s:%d\n",__FUNCTION__,__LINE__,current->comm,current->pid); \
        	} \
} while(0);

#define USING_XLOG

#ifdef __FHCTL_CTP__
	#define fhctl_err		must_print
	#define fhctl_warn		must_print
	#define fhctl_info		must_print
	#define fhctl_dbg		must_print
	#define fhctl_ver		must_print

	#define fhctl_read_reg(aDDR)			READ_REG(aDDR)
	#define fhctl_write_reg(vAL, aDDR)		WRITE_REG(vAL, aDDR)

#else

	#ifdef USING_XLOG

	#include <linux/xlog.h>
	#define TAG     "Pll/fhctl"

	#define fhctl_err(fmt, args...)       \
	    xlog_printk(ANDROID_LOG_ERROR, TAG, fmt, ##args)
	#define fhctl_warn(fmt, args...)      \
	    xlog_printk(ANDROID_LOG_WARN, TAG, fmt, ##args)
	#define fhctl_info(fmt, args...)      \
	    xlog_printk(ANDROID_LOG_INFO, TAG, fmt, ##args)
	#define fhctl_dbg(fmt, args...)       \
	    xlog_printk(ANDROID_LOG_DEBUG, TAG, fmt, ##args)
	#define fhctl_ver(fmt, args...)       \
	    xlog_printk(ANDROID_LOG_VERBOSE, TAG, fmt, ##args)

	#else /* !USING_XLOG */

	#define TAG     "[Pll/fhctl] "

	#define fhctl_err(fmt, args...)       \
	    printk(KERN_ERR TAG);           \
	    printk(KERN_CONT fmt, ##args)
	#define fhctl_warn(fmt, args...)      \
	    printk(KERN_WARNING TAG);       \
	    printk(KERN_CONT fmt, ##args)
	#define fhctl_info(fmt, args...)      \
	    printk(KERN_NOTICE TAG);        \
	    printk(KERN_CONT fmt, ##args)
	#define fhctl_dbg(fmt, args...)       \
	    printk(KERN_INFO TAG);          \
	    printk(KERN_CONT fmt, ##args)
	#define fhctl_ver(fmt, args...)       \
	    printk(KERN_DEBUG TAG);         \
	    printk(KERN_CONT fmt, ##args)

	#endif //- USING_XLOG

	#define fhctl_read_reg(aDDR)					DRV_Reg32(aDDR)
	#define fhctl_write_reg(vAL, aDDR)				DRV_WriteReg32(aDDR, vAL)
#endif //- __FHCTL_CTP__



#define fhctl_clr_field(vAL, rG)					vAL &= (~((rG##_MASK) << (rG##_BIT)));
#define fhctl_set_field(vAL, wVAL, rG)				vAL |= ( ((wVAL) & (rG##_MASK)) << (rG##_BIT) )
#define fhctl_clr_and_set_field(vAL, wVAL, rG)		fhctl_clr_field(vAL, rG);fhctl_set_field(vAL, wVAL, rG)


#define PERCENT_M10_TO_DDSLMT(dDS, pERCENT_M10)		((dDS * pERCENT_M10 >> 5) / 1000)

#define __REGXfer(tOKEN)	{.name = #tOKEN, .addr = tOKEN}
///////////////////////////////////////////////////////////
// Local variable
///////////////////////////////////////////////////////////
static UINT32 freqhop_initiated = 0;
static UINT32 fhctl_sm_store[NUM_OF_PLL_ID] = {0, 0};

FHCTL_STATIC const UINT32 fhctl_rfid2rfenbitmap[7]	= {	(1<<RG_FHCTL_2G1_ON_BIT),
													   	(1<<RG_FHCTL_2G2_ON_BIT),
														(1<<RG_FHCTL_INTGMD_ON_BIT) | (1<<RG_FHCTL_INTMD_OLD_ON_BIT),
														(1<<RG_FHCTL_EXTMD_ON_BIT) | (1<<RG_FHCTL_EXTMD_OLD_ON_BIT),
														(1<<RG_FHCTL_BT_ON_BIT),
														(1<<RG_FHCTL_WF_ON_BIT),
														(1<<RG_FHCTL_FM_ON_BIT)};

FHCTL_STATIC const char rfid_to_rfname[NUM_OF_RF_ID][4]		= {"2G1", "2G2", "IMD", "EMD", "BT", "WF", "FM"};
FHCTL_STATIC const char pllid_to_pllname[NUM_OF_PLL_ID][8]	= {"ARMPLL", "MAINPLL"};
FHCTL_STATIC const FREQHOP_REG_MAP fhctl_reg_map[NUM_OF_FHCTL_REGS]	= {
															__REGXfer(FHDMA_CFG			),
		                                                    __REGXfer(FHDMA_2G1BASE		),
		                                                    __REGXfer(FHDMA_2G2BASE		),
		                                                    __REGXfer(FHDMA_INTMDBASE	),
		                                                    __REGXfer(FHDMA_EXTMDBASE	),
		                                                    __REGXfer(FHDMA_BTBASE	 	),
			                                                __REGXfer(FHDMA_WFBASE	 	),
			                                                __REGXfer(FHDMA_FMBASE	 	),
			                                                __REGXfer(FHSRAM_CON		),
		                                                    __REGXfer(FHSRAM_WR	    	),
		                                                    __REGXfer(FHSRAM_RD	    	),
		                                                    __REGXfer(FHCTL_CFG			),
		                                                    __REGXfer(FHCTL_CON			),
		                                                    __REGXfer(FHCTL_2G1_CH	    ),
			                                                __REGXfer(FHCTL_2G2_CH	    ),
			                                                __REGXfer(FHCTL_INTMD_CH	),
			                                                __REGXfer(FHCTL_EXTMD_CH	),
			                                                __REGXfer(FHCTL_BT_CH		),
			                                                __REGXfer(FHCTL_WF_CH		),
		                                                    __REGXfer(FHCTL_FM_CH		),
		                                                    __REGXfer(FHCTL0_CFG 		),
		                                                    __REGXfer(FHCTL0_UPDNLMT	),
		                                                    __REGXfer(FHCTL0_DDS 		),
		                                                    __REGXfer(FHCTL0_DVFS 	    ),
		                                                    __REGXfer(FHCTL0_MON		),
			                                                __REGXfer(FHCTL1_CFG 		),
			                                                __REGXfer(FHCTL1_UPDNLMT	),
			                                                __REGXfer(FHCTL1_DDS 	    ),
			                                                __REGXfer(FHCTL1_DVFS 	    ),
			                                                __REGXfer(FHCTL1_MON		),
														 };

FHCTL_STATIC const UINT32 pll_con0[NUM_OF_PLL_ID] 		= {ARMPLL_CON0, MAINPLL_CON0};
FHCTL_STATIC const UINT32 pll_con1_addr[NUM_OF_PLL_ID] 	= {ARMPLL_CON1, MAINPLL_CON1};
FHCTL_STATIC const UINT32 pll_con2_addr[NUM_OF_PLL_ID] 	= {ARMPLL_CON2, MAINPLL_CON2};
FHCTL_STATIC const FREQHOP_REG_MAP fhpll_reg_map[7]	= {
															__REGXfer(PLL_HP_CON0	),
															__REGXfer(ARMPLL_CON0	),
															__REGXfer(ARMPLL_CON1	),
															__REGXfer(ARMPLL_CON2	),
															__REGXfer(MAINPLL_CON0	),
															__REGXfer(MAINPLL_CON1	),
															__REGXfer(MAINPLL_CON2	),
														 };

FHCTL_STATIC FREQHOP_PLLSettings freqhop_pll_settings[NUM_OF_PLL_ID][FREQHOP_PLLSETTINGS_MAXNUMBER] = {
		// { target_vco_freq, dt, df, uplimit, downlimit, dds_val }
		{// ----------- ARM PLL ----------- 0 ~ -4%
			{1001000, 0 ,9 ,0, 40, 0, 0x3AE, 0x9A000},// 1.001GHz , 0.27us, 0.023437500, +0% ~ -4%, 38.5
			{1612000, 0 ,9 ,0, 40, 0, 0x3AE, 0xF8000},// 0.806GHz , 0.27us, 0.023437500, +0% ~ -4%, 46
			{1196000, 0 ,9 ,0, 40, 0, 0x3AE, 0xB8000},// 0.598GHz , 0.27us, 0.023437500, +0% ~ -4%, 62
			{0,0,0,0,0,0,0,0} //EOF
		},
		{// ----------- MAIN PLL ----------- SSC : 0% ~ -4%
			{1599000, 0, 9, 0 ,40, 0, 0x4EB, 0xF6000}, // 1.599GHz, , 0.27us, 0.023437500, +0% ~ -4%, 61.5
			{1521000, 0, 9, 0 ,40, 0, 0x4EB, 0xF6000}, // 1.521GHz, , 0.27us, 0.023437500, +0% ~ -4%, 58.5
			{1443000, 0, 9, 0 ,40, 0, 0x4EB, 0xF6000}, // 1.443GHz, , 0.27us, 0.023437500, +0% ~ -4%, 55.5
			//- DVFS
			//-{1599000, 0, 9, 0, 0x4EB, 0xF6000}, // 1.599GHz, , 0.27us, 0.023437500, +0% ~ -4%, 61.5
			{0,0,0,0,0,0,0,0} //EOF
		}
};

#ifndef __FHCTL_CTP__
static DEFINE_SPINLOCK(fhctl_lock);
#define FHCTL_LOCK			spin_lock(&fhctl_lock);
#define FHCTL_UNLOCK		spin_unlock(&fhctl_lock);
#else
#define FHCTL_LOCK
#define FHCTL_UNLOCK
#endif

///////////////////////////////////////////////////////////
// Local function
///////////////////////////////////////////////////////////
#ifndef __FHCTL_CTP__
FHCTL_STATIC INT32 freqhop_init(UINT32 option);
FHCTL_STATIC const char rfid_to_rfname[NUM_OF_RF_ID][4];
FHCTL_STATIC const char pllid_to_pllname[NUM_OF_PLL_ID][8];
FHCTL_STATIC const FREQHOP_REG_MAP fhctl_reg_map[NUM_OF_FHCTL_REGS];
FHCTL_STATIC const FREQHOP_REG_MAP fhpll_reg_map[7];
FHCTL_STATIC FREQHOP_PLLSettings freqhop_pll_settings[NUM_OF_PLL_ID][FREQHOP_PLLSETTINGS_MAXNUMBER];
FHCTL_STATIC void freqhop_fhctlx_set_DVFS(UINT32 pll_id, UINT32 dds);
FHCTL_STATIC void freqhop_setbit_FHCTLx_cfg(UINT32 pll_id, UINT32 field, UINT32 mode);

#else //!__FHCTL_CTP__

FHCTL_STATIC void freqhop_delay_task(UINT32 time_in_ticks);
FHCTL_STATIC void freqhop_sram_blkcpy(UINT32 pll_id, UINT32 *pDDS);
FHCTL_STATIC void freqhop_rf_src_hopping_enable(UINT32 rf_id, UINT32 enable);
FHCTL_STATIC void freqhop_set_priority(UINT32 order, UINT32 order_md);
FHCTL_STATIC void freqhop_set_dma_mode(UINT32 mode);
FHCTL_STATIC void freqhop_rf_src_trigger_ch(UINT32 rf_id, UINT32 channel);
FHCTL_STATIC UINT32 freqhop_get_pll_mon_dss(UINT32 pll_id);
FHCTL_STATIC UINT32 freqhop_get_pll_fhctlx_dss(UINT32 pll_id);
FHCTL_STATIC void freqhop_sram_init(UINT32 *pDDS);
FHCTL_STATIC void freqhop_set_fhctlx_updnlmt(UINT32 pll_id, UINT32 uplimit, UINT32 downlimit);
FHCTL_STATIC void freqhop_set_fhctlx_slope(UINT32 pll_id, UINT32 dts, UINT32 dys);

#endif //END !__FHCTL_CTP__







// -----------------------------------------------------
// DRAM Setting
// -----------------------------------------------------
void freqhop_dram_init(UINT32 rf_id, UINT32 paddr)
{
	fhctl_write_reg (paddr, FHDMA_RFx_BASE(rf_id));
}


// -----------------------------------------------------
// SRAM Setting
// -----------------------------------------------------
void freqhop_sram_enable_with_offset(UINT32 offset)
{
	UINT32 reg = fhctl_read_reg (FHSRAM_CON);

	fhctl_clr_and_set_field(reg, 1, RG_UPSRAM_CE);
	fhctl_clr_and_set_field(reg, offset, RG_UPSRAM_RW_ADDR);

	fhctl_write_reg(reg, FHSRAM_CON);
}

void freqhop_sram_disable(void)
{
	UINT32 reg = fhctl_read_reg (FHSRAM_CON);

	fhctl_clr_field(reg, RG_UPSRAM_CE);

	fhctl_write_reg(reg, FHSRAM_CON);
}

void freqhop_sram_write_data(UINT32 value)
{
	fhctl_write_reg(value, FHSRAM_WR);
}

UINT32 freqhop_sram_read_data(void)
{
	return fhctl_read_reg(FHSRAM_RD) ;
}

void freqhop_sram_write_data_with_offset(UINT32 value, UINT32 offset)
{
	freqhop_sram_enable_with_offset(offset);

	//- write data
	fhctl_write_reg(value, FHSRAM_WR);
}

UINT32 freqhop_sram_read_data_with_offset(UINT32 offset)
{
	freqhop_sram_enable_with_offset(offset);

	return fhctl_read_reg(FHSRAM_RD);
}

// -----------------------------------------------------
// Misc
// -----------------------------------------------------



void freqhop_set_hopping_slope (UINT32 dy, UINT32 dt)
{
	UINT32 reg;

	reg = fhctl_read_reg(FHCTL_CON);
	fhctl_clr_and_set_field (reg, dy, RG_FHCTL_SFDY);
	fhctl_clr_and_set_field (reg, dt, RG_FHCTL_SFDT);
	fhctl_write_reg(reg, FHCTL_CON);
}

void freqhop_fhctlx_set_DVFS(UINT32 pll_id, UINT32 dds)
{
	UINT32 reg = fhctl_read_reg(FHCTLx_DVFS(pll_id));

	fhctl_set_field (reg, 1, RG_FHCTLx_PLL_DVFS_TRI);
	fhctl_clr_and_set_field (reg, dds, RG_FHCTLx_PLL_DVFS);

	fhctl_write_reg(dds, FHCTLx_DVFS(pll_id));
}

void freqhop_ddscpy_pllcon(UINT32 pll_id, UINT32 mode)
{
	UINT32 reg;
	UINT32 dds_ori = 0;

	if (mode == SWITCH_PLLCON2FHCTL)
	{
		//- read pllgp_dds
		dds_ori = fhctl_read_reg(pll_con1_addr[pll_id]) & RG_FHCTLx_DDS_MASK;
		fhctl_info("Read %s_DDS = 0x%08X\n", pllid_to_pllname[pll_id], dds_ori);

		reg = fhctl_read_reg(FHCTLx_DDS(pll_id));
		//- write pllgp_dds to fhctl_dds
		fhctl_clr_and_set_field (reg, 1, RG_FHCTLx_PLL_TGL_ORG);
		fhctl_clr_and_set_field (reg, dds_ori, RG_FHCTLx_PLL_ORG);
		fhctl_write_reg(reg, FHCTLx_DDS(pll_id));
		fhctl_info("Set %s_FHCTL_DDS = 0x%08X\n", pllid_to_pllname[pll_id], RG_FHCTLx_DDS_MASK & fhctl_read_reg(FHCTLx_DDS(pll_id)));
	}
	else
	{
		//- read fhctl_dds
		dds_ori = fhctl_read_reg(FHCTLx_DDS(pll_id)) & RG_FHCTLx_DDS_MASK;
		fhctl_info("Read %s_FHCTL_DDS = 0x%08X\n", pllid_to_pllname[pll_id], dds_ori);
		dds_ori = freqhop_pll_settings[pll_id][0].dds_val & RG_FHCTLx_DDS_MASK;
		fhctl_info("Read %s_FHCTL_DDS_Ori = 0x%08X\n", pllid_to_pllname[pll_id], dds_ori);

		//- write fhctl_dds to pllgp_dds
		fhctl_write_reg(dds_ori, pll_con1_addr[pll_id]);
		fhctl_info("Set %s_FHCTL_DDS = 0x%08X\n", pllid_to_pllname[pll_id], fhctl_read_reg(pll_con1_addr[pll_id]));
	}
}

void freqhop_fhctlx_pllmux(UINT32 pll_id, UINT32 mode)
{
	UINT32 reg;

	if (mode == SWITCH_PLLCON2FHCTL)
	{
		// switch pll dds control to fhctl
		reg = fhctl_read_reg(PLL_HP_CON0);
		reg |= (1 << pll_id);
		fhctl_write_reg(reg, PLL_HP_CON0);
	}
	else
	{
		// set pll_hp_con0 as pllgp control
		reg = fhctl_read_reg(PLL_HP_CON0);
		reg &= (~(1 << pll_id));
		fhctl_write_reg(reg, PLL_HP_CON0);
	}
}

void freqhop_fhctlx_enable (UINT32 pll_id, UINT32 mode, FREQHOP_PLLSettings *pSetting)
{
	UINT32 cfg_reg_val;
	UINT32 updnlmt_reg_val = 0;
	UINT32 fhctl_sm_ori;

	fhctl_info("Enter: %s\n",__func__);

	//uplmt, dnlmt
	fhctl_set_field (updnlmt_reg_val, pSetting->uplimit, RG_FRDDSx_UPLMT);
	fhctl_set_field (updnlmt_reg_val, pSetting->downlimit, RG_FRDDSx_DNLMT);
	fhctl_write_reg (updnlmt_reg_val, FHCTLx_UPDNLMT(pll_id));

	cfg_reg_val = fhctl_read_reg(FHCTLx_CFG(pll_id));
	fhctl_sm_ori = cfg_reg_val & (RG_FHCTLx_EN_MASK << RG_FHCTLx_EN_BIT);
	//clear mode & enable bit
	cfg_reg_val &= (~mode);
	cfg_reg_val |= (mode & FHCTL_MODE_MASK);

	//enable bit
	cfg_reg_val |= (1 << FHCTLx_EN);
	//dt, df
	fhctl_clr_and_set_field (cfg_reg_val, pSetting->dt, RG_FRDDSx_DTS);
	fhctl_clr_and_set_field (cfg_reg_val, pSetting->df, RG_FRDDSx_DYS);

	if (fhctl_sm_ori)
	{	//- FHCTL enabled originally, no need to switch pll mux control
		;
	}
	else
	{	//- FHCTL disabled originally, switch pll mux control
		freqhop_ddscpy_pllcon(pll_id, SWITCH_PLLCON2FHCTL);
		freqhop_fhctlx_pllmux(pll_id, SWITCH_PLLCON2FHCTL);
	}
	fhctl_write_reg(cfg_reg_val, FHCTLx_CFG(pll_id));

	//store sm mode
	fhctl_sm_store[pll_id] = cfg_reg_val & FHCTL_MODE_MASK;

}

void freqhop_fhctlx_disable (UINT32 pll_id, UINT32 mode, UINT32 switch_mux)
{
	UINT32 reg;
	UINT32 disable_sm = 0;
	UINT32 dds_mon;
	UINT32 dds_fhx;

	fhctl_info("Enter: %s\n",__func__);

	reg = fhctl_read_reg(FHCTLx_CFG(pll_id));
	mode &= FHCTL_MODE_MASK;

	if (((reg & FHCTL_SM) == 0) || (mode == 0))
	{	//- already disable or not disable
		return;
	}

	if ((mode & FHCTL_SM) || (mode == (FHCTL_SSC | FHCTL_HOPPING)))
	{	//- disable both SSC and Hopping
		disable_sm = 1;
	}
	else if (mode == FHCTL_SSC)
	{	//- disable SSC
		if (reg & (1 << SFSTRx_EN))
		{	//- hopping exist, disable SSC only
			freqhop_setbit_FHCTLx_cfg(pll_id, FRDDSx_EN, 0);
		}
		else
		{	//- disable both SSC and Hopping
			disable_sm = 1;
		}
	}
	else if (mode == FHCTL_HOPPING)
	{	//- disable Hopping
		if (reg & (1 << FRDDSx_EN))
		{	//- SSC exist, disable hopping only
			freqhop_setbit_FHCTLx_cfg(pll_id, SFSTRx_EN, 0);
		}
		else
		{	//- disable both SSC and Hopping
			disable_sm = 1;
		}
	}

	if (disable_sm)
	{
		//- disable both SSC and hopping
		//- clear all enable
		reg &= (~FHCTL_MODE_MASK);
		fhctl_write_reg(reg, FHCTLx_CFG(pll_id));

		//- wait DDS stable
		dds_fhx = fhctl_read_reg(FHCTLx_DDS(pll_id)) & RG_FHCTLx_DDS_MASK;
		do
		{
			//- polling pll_con2_addr[pll_id]
			dds_mon = fhctl_read_reg(pll_con2_addr[pll_id]) & RG_FHCTLx_DDS_MASK;
//-			dds_mon = (FHCTLx_MON(pll_id)) & RG_FHCTLx_DDS_MASK;
		} while (dds_mon != dds_fhx);

		if (switch_mux)
		{
			freqhop_ddscpy_pllcon(pll_id, SWITCH_FHCTL2PLLCON);
			freqhop_fhctlx_pllmux(pll_id, SWITCH_FHCTL2PLLCON);
		}
	}
}

void freqhop_setbit_FHCTLx_cfg(UINT32 pll_id, UINT32 field, UINT32 mode)
{
	UINT32 reg;

	reg = fhctl_read_reg(FHCTLx_CFG(pll_id));
	reg &= (~(1 << field));
	reg |= (mode << field);
	fhctl_write_reg(reg, FHCTLx_CFG(pll_id));

	//store sm mode
	fhctl_sm_store[pll_id] = reg & FHCTL_MODE_MASK;

}

void freqhop_get_all_pll_mon_dss(UINT32 *pBuffer)
{
	*pBuffer = fhctl_read_reg(FHCTLx_MON(ARMPLL_ID)) & RG_FHCTLx_DDS_MASK;
	fhctl_info ("[FreqHop] %s() MON_DDS_%d = 0x%08X\n", __func__, ARMPLL_ID, *pBuffer) ;

	pBuffer++;
	*pBuffer = fhctl_read_reg(FHCTLx_MON(MAINPLL_ID)) & RG_FHCTLx_DDS_MASK;
	fhctl_info ("[FreqHop] %s() MON_DDS_%d = 0x%08X\n", __func__, MAINPLL_ID, *pBuffer) ;

}

void freqhop_fill_setting_from_dds (UINT32 dds, FREQHOP_PLLSettings *pSetting)
{
	pSetting->dds_val	= dds & RG_FHCTLx_DDS_MASK;
	pSetting->uplimit	= PERCENT_M10_TO_DDSLMT (dds, pSetting->uplimit_percent_10);
	pSetting->downlimit	= PERCENT_M10_TO_DDSLMT (dds, pSetting->downlimit_percent_10);
}

//--------------------------------
//-  Init Funcs
//--------------------------------
//- bit 0:		ARMPLL SSC default on
//- bit 1:		MAINPLL SSC default on
INT32 freqhop_init(UINT32 option)
{
    UINT32 id;
    UINT32 reg_con0,reg_con1,reg_con2;

	if (freqhop_initiated)	return FHCTL_SUCCESS;

	freqhop_initiated = 1;

    //fhctl_write_reg(1, DMA_GLOBAL_SEC_EN);
    fhctl_info ("Initial Frequency Hopping Driver\n");

	//- ungate fhctl clock
	enable_clock (MT_CG_FHCTL_SW_CG, "FHCTL");
	enable_dpidle_by_bit(MT_CG_FHCTL_SW_CG);

	//- must check pllx_con1 dds is equal to fhctl_ seeting
    for(id=ARMPLL_ID; id<NUM_OF_PLL_ID; id++)
    {
    	FHCTL_LOCK;

	    reg_con0 = fhctl_read_reg(pll_con0[id]);
	    reg_con1 = fhctl_read_reg(pll_con1_addr[id]);
	    reg_con2 = fhctl_read_reg(pll_con2_addr[id]);
	    //- re-fill setting from PLL_CON1
		freqhop_pll_settings[id][0].dds_val = reg_con1 & RG_FHCTLx_DDS_MASK;
	    freqhop_fill_setting_from_dds ((reg_con1 & RG_FHCTLx_DDS_MASK), &(freqhop_pll_settings[id][0]));

		if (option & (1 << id))
		{	//- SSC default on ???
			freqhop_fhctlx_enable (id, FHCTL_SM | FHCTL_SSC, &(freqhop_pll_settings[id][0]));
		}

		FHCTL_UNLOCK;

	    fhctl_info("%s pll_con0 ori = 0x%08X\n", pllid_to_pllname[id], reg_con0);
	    fhctl_info("%s pll_con1 ori = 0x%08X\n", pllid_to_pllname[id], reg_con1);
	    fhctl_info("%s pll_con2 ori = 0x%08X\n", pllid_to_pllname[id], reg_con2);
    }




    return FHCTL_SUCCESS;
}


#ifndef __FHCTL_CTP__
//--------------------------------
//-  sysfs
//--------------------------------

static INT32 freqhop_proc_dbg_write (struct file *file, const char __user *buffer, unsigned long count, void *data)
{
	INT32 		ret;
	char 		kbuf[256];
	UINT32	 	len = 0;
	UINT32		p1,p2,p3,p4,p5,p6,p7,p8;

	fhctl_info("Enter: %s\n",__func__);
	len = min(count, (unsigned long)(sizeof(kbuf)-1));
	if (count == 0)		return -1;
	if(count > 255)		count = 255;
	ret = copy_from_user(kbuf, buffer, count);
	if (ret < 0)		return -1;
	kbuf[count] = '\0';
	sscanf(kbuf, "%x %x %x %x %x %x %d %d", &p1, &p2, &p3, &p4, &p5, &p6, &p7, &p8);

	FHCTL_LOCK;
	switch (p1)
	{
		case 1:{	//- config slope of hopping	=> 1   1   Hop_Delta_Freq   Hop_Delta_time   0   0   0   0
				freqhop_set_hopping_slope (p3, p4);
			}break;
		case 2:{	//- config hopping => 2   x   pll_id   dds   hop_dds   0   0
					//- disable FIRST
				freqhop_fhctlx_disable (p3, FHCTL_HOPPING, 1);
				if (p2 == 0)
				{	//- disable
				}
				else
				{	//- enable
					//- use dvfs to implement
					freqhop_fhctlx_enable (p3, FHCTL_HOPPING | FHCTL_SM, &(freqhop_pll_settings[p3][0]));
					freqhop_fhctlx_set_DVFS (p3, p5);
				}
			}break;
		case 3:{	//- config SSC => 3    x   pll_id   dds   ssc_df   ssc_dt   uplmt   dnlmt
				if (p2 == 0)
				{	//- disable FIRST
					freqhop_fhctlx_disable (p3, FHCTL_SSC, 1);
				}
				else if (p2 == 1)
				{	//- enable, but disable FIRST
					freqhop_fhctlx_disable (p3, FHCTL_SSC, 1);
					freqhop_pll_settings[p3][0].df 						= p5;
					freqhop_pll_settings[p3][0].dt 						= p6;
					freqhop_pll_settings[p3][0].uplimit_percent_10		= p7;
					freqhop_pll_settings[p3][0].downlimit_percent_10	= p8;
					freqhop_fill_setting_from_dds (p4, &(freqhop_pll_settings[p3][0]));
					freqhop_fhctlx_enable (p3, FHCTL_SSC | FHCTL_SM, &(freqhop_pll_settings[p3][0]));
				}
				else if (p2 == 3)
				{
					freqhop_fhctlx_disable (ARMPLL_ID, FHCTL_SSC, 1);
					freqhop_fill_setting_from_dds (freqhop_pll_settings[ARMPLL_ID][0].dds_val, &(freqhop_pll_settings[ARMPLL_ID][0]));
					freqhop_fhctlx_enable (ARMPLL_ID, FHCTL_SSC | FHCTL_SM, &(freqhop_pll_settings[ARMPLL_ID][0]));
					freqhop_fhctlx_disable (MAINPLL_ID, FHCTL_SSC, 1);
					freqhop_fill_setting_from_dds (freqhop_pll_settings[MAINPLL_ID][0].dds_val, &(freqhop_pll_settings[MAINPLL_ID][0]));
					freqhop_fhctlx_enable (MAINPLL_ID, FHCTL_SSC | FHCTL_SM, &(freqhop_pll_settings[MAINPLL_ID][0]));
				}	//- enable ALL SSC
				else if (p2 == 4)
				{	//- disable ALL SSC
					freqhop_fhctlx_disable (ARMPLL_ID, FHCTL_SSC, 1);
					freqhop_fhctlx_disable (MAINPLL_ID, FHCTL_SSC, 1);
				}
			}break;
		default:
			break;
	}
	FHCTL_UNLOCK;

	return (INT32)count;
}

static INT32 freqhop_proc_dbg_read (char *page, char **start, off_t off, INT32 count, INT32 *eof, void *data)
{
	char *p = page;
	UINT32 len = 0;
	UINT32 id, reg, dds, ssc, hop, dds_mon;

	if (off > 0)
	{
		*eof = 1;
		return 0;
	}

	fhctl_info("Enter: %s\n",__func__);

	for(id=ARMPLL_ID; id<NUM_OF_PLL_ID; id++)
	{
		ssc = 0;
		hop = 0;
		reg = fhctl_read_reg(FHCTLx_CFG(id));
		dds = fhctl_read_reg(pll_con1_addr[id]);
		if (reg & (1 << FHCTLx_EN))
		{
			//- dds = fhctl_read_reg (FHCTLx_DDS(id));
			if (reg & (1 << FRDDSx_EN))		{ssc = 1;}
			if (reg & (1 << SFSTRx_EN))		{hop = 1;}
			dds_mon = fhctl_read_reg (FHCTLx_MON(id));
		}
		else
		{
			dds_mon = 0;
		}
		p += sprintf(p, "%s :  ssc_en=%d, hop_en=%d, dds=0x%06X, dds_mon=0x%06X, df=0x%01X, dt=0x%01X, up=%2d, dn=%2d\n",	pllid_to_pllname[id],
																															ssc, hop, dds, dds_mon,
																															freqhop_pll_settings[id][0].df,
																															freqhop_pll_settings[id][0].dt,
																															freqhop_pll_settings[id][0].uplimit_percent_10,
																															freqhop_pll_settings[id][0].downlimit_percent_10
																															);
	}

	*start = page + off;
	len = p - page;
	if (len > off)	{len -= off;}
	else			{len = 0;}

	return len < count ? len : count;

}

static INT32 freqhop_proc_dumpregs_write (struct file *file, const char __user *buffer, unsigned long count, void *data)
{
	fhctl_info("Enter: %s\n",__func__);

	return 0;
}

static INT32 freqhop_proc_dumpregs_read (char *page, char **start, off_t off, INT32 count, INT32 *eof, void *data)
{
	char *p = page;
	UINT32 len = 0;
	UINT32 i;

	if (off > 0)
	{
		*eof = 1;
		return 0;
	}

	fhctl_info("Enter: %s\n",__func__);

	for (i=0; i<7; i++)
	{
		p += sprintf(p, "0x%08X, %s\r\n", fhctl_read_reg(fhpll_reg_map[i].addr), fhpll_reg_map[i].name);
	}

	for (i=0; i<NUM_OF_FHCTL_REGS; i++)
	{
		p += sprintf(p, "0x%08X, %s\r\n", fhctl_read_reg(fhctl_reg_map[i].addr), fhctl_reg_map[i].name);
	}

	*start = page + off;
	len = p - page;
	if (len > off)	{len -= off;}
	else			{len = 0;}

	return len < count ? len : count;

}

static INT32 freqhop_proc_help_write (struct file *file, const char __user *buffer, unsigned long count, void *data)
{
	fhctl_info("Enter: %s\n",__func__);

	return 0;
}

static INT32 freqhop_proc_help_read (char *page, char **start, off_t off, INT32 count, INT32 *eof, void *data)
{
	char *p = page;
	UINT32 len = 0;
	UINT32 i, num, pll_id, reg;

	if (off > 0)
	{
		*eof = 1;
		return 0;
	}

	fhctl_info("Enter: %s\n",__func__);

	//- slope (26f/16384)/((t+2)/26) =(26*26*f)/(16384*t+2)
	reg = fhctl_read_reg(FHCTL_CON);
	num = 825 * ((reg >> RG_FHCTL_SFDY_BIT) & RG_FHCTL_SFDY_MASK) / (20 * (2 + ((reg >> RG_FHCTL_SFDT_BIT) & RG_FHCTL_SFDT_MASK)));
	i = num / 1000;
	p += sprintf(p, "***Current Status***\nHop Slope %d.%d MHz/us\n", i, num - 1000*i);
	for(pll_id=ARMPLL_ID; pll_id<NUM_OF_PLL_ID; pll_id++)
	{
		p += sprintf(p, "%s :\n", pllid_to_pllname[pll_id]);
		p += sprintf(p, "sm   ssc   hop   \n");
		reg = fhctl_read_reg(FHCTLx_CFG(pll_id));


		if (reg & FHCTL_SM)			{p += sprintf(p, "on   ");}
		else						{p += sprintf(p, "---  ");}

		if (reg & FHCTL_SSC)		{p += sprintf(p, "  on  ");}
		else						{p += sprintf(p, "  --- ");}

		if (reg & FHCTL_HOPPING)	{p += sprintf(p, "   on  ");}
		else						{p += sprintf(p, "   --- ");}

		reg = fhctl_read_reg(pll_con2_addr[pll_id]) & RG_FHCTLx_DDS_MASK;
		p += sprintf(p, "\nPLLCON2 = 0x%05X\n\n", reg);
	}

	p += sprintf(p, "SSC Slope\n"
					"DY : in MHz\n"
					"0x0: 0.000938691\n"
					"0x1: 0.003173820\n"
					"0x2: 0.006347666\n"
					"0x3: 0.012695306\n"
					"0x4: 0.025390612\n"
					"0x5: 0.05078125\n"
					"0x6: 0.1015625\n"
					"0x7: 0.203125\n"
					"0x8: 0.40625\n"
					"0x9: 0.609375\n"
					"0xA: 0.8125\n"
					"0xB: 1.21875\n"
					"0xC: 1.625\n"
					"0xD: 2.4375\n"
					"0xE: 3.25\n"
					"0xF: 6.5\n"
					"DT : in us\n"
					"0x0: 0.27\n"
					"0x1: 0.50\n"
					"0x2: 0.77\n"
					"0x3: 1.00\n"
					"0x4: 1.26\n"
					"0x5: 1.50\n"
					"0x6: 1.73\n"
					"0x7: 2.00\n"
					"0x8: 2.23\n"
					"0x9: 2.50\n"
					"0xA: 2.76\n"
					"0xB: 3.00\n"
					"0xC: 4.00\n"
					"0xD: 8.00\n"
					"0xE: 10.00\n"
					"0xF: 20.00\n"
				);

	*start = page + off;
	len = p - page;
	if (len > off)	{len -= off;}
	else			{len = 0;}

	return len < count ? len : count;

}

#define fhctl_create_proc(fILENAME)															\
do {                                                                                        \
		struct proc_dir_entry *pEntry;		                                                \
		                                                                                    \
		pEntry = create_proc_entry(#fILENAME,  S_IRUGO | S_IWUSR | S_IWGRP, proc_dir); 		\
		if(pEntry)                                                                          \
		{                                                                                   \
			pEntry->read_proc  = freqhop_proc_##fILENAME##_read;                            \
			pEntry->write_proc = freqhop_proc_##fILENAME##_write;                           \
			fhctl_info("[%s]: successfully create /proc/fhctl/%s", __func__, #fILENAME);      \
		}else{                                                                             \
			fhctl_info("[%s]: failed to create /proc/fhctl/%s", __func__, #fILENAME);         \
		}																					\
} while (0)
#if 0
INT32 freqhopping_config(unsigned int pll_id, unsigned long vco_freq, unsigned int enable)
{
	FHCTL_LOCK;

	if (enable)
	{
		freqhop_fill_setting_from_dds (RG_FHCTLx_DDS_MASK & fhctl_read_reg(pll_con1_addr[pll_id]), &(freqhop_pll_settings[pll_id][0]));
		freqhop_fhctlx_enable (pll_id, fhctl_sm_store[pll_id], &(freqhop_pll_settings[pll_id][0]));
	}
	else
	{
		freqhop_fhctlx_disable (pll_id, FHCTL_SM, 1);
	}

	FHCTL_UNLOCK;

	return FHCTL_SUCCESS;
}
#endif

void mt_fh_query_SSC_boundary (UINT32 pll_id, UINT32* uplmt_10, UINT32* dnlmt_10)
{
	UINT32 reg;

	FHCTL_LOCK;
	reg = fhctl_read_reg(FHCTLx_CFG(pll_id));
	if (reg & FHCTL_SSC)
	{
		*uplmt_10 = freqhop_pll_settings[pll_id][0].uplimit_percent_10;
		*uplmt_10 = freqhop_pll_settings[pll_id][0].downlimit_percent_10;
	}
	else
	{
		*uplmt_10 = 0;
		*uplmt_10 = 0;
	}
	FHCTL_UNLOCK;
}
//--------------------------------
//-  HAL porting start
//--------------------------------
static void mt_fh_hal_init(void)
{
	struct proc_dir_entry *proc_dir = NULL;

	//TODO: check the permission!!

	fhctl_info("Enter: %s\n",__func__);

	proc_dir = proc_mkdir("fhctl", NULL);
	if (!proc_dir)
	{
		fhctl_err("[Err] proc_mkdir fhctl fail!\n");
		return;
	}
	else
	{
		fhctl_create_proc (dbg);
		fhctl_create_proc (dumpregs);
		fhctl_create_proc (help);
	}

	//- #if (defined (MT_FREQHOP_DEFAULT_ON) && defined(CONFIG_MTK_LDVT))
	#if (defined (MT_FREQHOP_DEFAULT_ON))
	freqhop_init (FHCTL_MAINPLL_SSC_ON);
	#else
	freqhop_init (0);
	#endif
}
static int freqhopping_clkgen_proc_read(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	FH_BUG_ON(1);
	return 0;
}

static int freqhopping_clkgen_proc_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
	FH_BUG_ON(1);
	return 0;
}

static int freqhopping_dramc_proc_read(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	FH_BUG_ON(1);
	return 0;
}

static int freqhopping_dramc_proc_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
	FH_BUG_ON(1);
	return 0;
}

static int freqhopping_dumpregs_proc_read(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	FH_BUG_ON(1);
	return 0;
}

static int freqhopping_dvfs_proc_read(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	FH_BUG_ON(1);
	return 0;
}

static int freqhopping_dvfs_proc_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
	FH_BUG_ON(1);
	return 0;
}

static int __freqhopping_ctrl(struct freqhopping_ioctl* fh_ctl,bool enable)
{
	FH_BUG_ON(1);
	return 0;
}

static void mt_fh_hal_lock(unsigned long *flags)
{
	FH_BUG_ON(1);
}

static void mt_fh_hal_unlock(unsigned long *flags)
{
	FH_BUG_ON(1);
}

static int mt_fh_hal_get_init(void)
{
	FH_BUG_ON(1);
	return 0;
}

static void mt_fh_hal_popod_restore(void)
{
	FH_BUG_ON(1);
}

static void mt_fh_hal_popod_save(void)
{
	FH_BUG_ON(1);
}

//mempll 200->266MHz using FHCTL
static int mt_fh_hal_l2h_mempll(void)  //mempll low to high (200->266MHz)
{
	FH_BUG_ON(1);
	return 0;
}

//mempll 266->200MHz using FHCTL
static int mt_fh_hal_h2l_mempll(void)  //mempll low to high (200->266MHz)
{
	FH_BUG_ON(1);
	return 0;
}

static int mt_fh_hal_l2h_dvfs_mempll(void)  //mempll low to high (200->266MHz)
{
	FH_BUG_ON(1);
	return 0;
}

static int mt_fh_hal_h2l_dvfs_mempll(void)  //mempll high to low(266->200MHz)
{
	FH_BUG_ON(1);
	return 0;
}

static int mt_fh_hal_dfs_armpll(unsigned int current_freq, unsigned int target_freq)  //armpll dfs mdoe
{
	FH_BUG_ON(1);
	return 0;
}

static int mt_fh_hal_is_support_DFS_mode(void)
{
	FH_BUG_ON(1);
	return FALSE;
}

static int mt_fh_hal_dram_overclock(int clk)
{
	FH_BUG_ON(1);
	return 0;
}

static int mt_fh_hal_get_dramc(void)
{
	FH_BUG_ON(1);
	return 0;
}
static void mt_fh_hal_default_conf(void){
  ;
}
static  fh_pll_t g_fh_pll[] = {
	{FH_FH_DISABLE,        FH_PLL_ENABLE   , 0, 0, 0},  //ARMPLL   default SSC disable
	{FH_FH_ENABLE_SSC,     FH_PLL_ENABLE   , 0, 0, 0}, //MAINPLL  default SSC enable
};
static struct freqhopping_ssc mt_ssc_fhpll_userdefined[]= {
	{0,0,0,0,0,0}, //ARMPLL
	{0,0,0,0,0,0}, //MAINPLL
};


static struct mt_fh_hal_driver g_fh_hal_drv;

struct mt_fh_hal_driver *mt_get_fh_hal_drv(void)
{
	memset(&g_fh_hal_drv, 0, sizeof(g_fh_hal_drv));
	
	g_fh_hal_drv.fh_pll			= g_fh_pll;
	g_fh_hal_drv.fh_usrdef			= mt_ssc_fhpll_userdefined;
	g_fh_hal_drv.pll_cnt 			= 2;
	g_fh_hal_drv.mempll 			= -1;
	g_fh_hal_drv.mainpll 			= -1;
	g_fh_hal_drv.msdcpll 			= -1;
	g_fh_hal_drv.mmpll 			= -1;
	g_fh_hal_drv.vencpll 			= -1;
	g_fh_hal_drv.lvdspll 			= -1;
	
	g_fh_hal_drv.mt_fh_hal_init 		=  mt_fh_hal_init;

	g_fh_hal_drv.proc.clk_gen_read 		=  freqhopping_clkgen_proc_read;
	g_fh_hal_drv.proc.clk_gen_write 	=  freqhopping_clkgen_proc_write;
	
	g_fh_hal_drv.proc.dramc_read 		=  freqhopping_dramc_proc_read;
	g_fh_hal_drv.proc.dramc_write 		=  freqhopping_dramc_proc_write;	
	g_fh_hal_drv.proc.dumpregs_read 	=  freqhopping_dumpregs_proc_read;

	g_fh_hal_drv.proc.dvfs_read 		=  freqhopping_dvfs_proc_read;
	g_fh_hal_drv.proc.dvfs_write 		=  freqhopping_dvfs_proc_write;	

	g_fh_hal_drv.mt_fh_hal_ctrl		= __freqhopping_ctrl;
	g_fh_hal_drv.mt_fh_lock			= mt_fh_hal_lock;
	g_fh_hal_drv.mt_fh_unlock		= mt_fh_hal_unlock;
	g_fh_hal_drv.mt_fh_get_init		= mt_fh_hal_get_init;

	g_fh_hal_drv.mt_fh_popod_restore 	= mt_fh_hal_popod_restore;
	g_fh_hal_drv.mt_fh_popod_save		= mt_fh_hal_popod_save;

	g_fh_hal_drv.mt_l2h_mempll		= mt_fh_hal_l2h_mempll;
	g_fh_hal_drv.mt_h2l_mempll		= mt_fh_hal_h2l_mempll;
	g_fh_hal_drv.mt_l2h_dvfs_mempll		= mt_fh_hal_l2h_dvfs_mempll;
	g_fh_hal_drv.mt_h2l_dvfs_mempll		= mt_fh_hal_h2l_dvfs_mempll;
	g_fh_hal_drv.mt_dfs_armpll		= mt_fh_hal_dfs_armpll;
	g_fh_hal_drv.mt_is_support_DFS_mode		= mt_fh_hal_is_support_DFS_mode;
	g_fh_hal_drv.mt_dram_overclock		= mt_fh_hal_dram_overclock;
	g_fh_hal_drv.mt_get_dramc		= mt_fh_hal_get_dramc;
  g_fh_hal_drv.mt_fh_default_conf	= mt_fh_hal_default_conf;
	return (&g_fh_hal_drv);
}
//--------------------------------
//-  HAL porting end
//--------------------------------
#else //!__FHCTL_CTP__
void freqhop_delay_task(UINT32 time_in_ticks)
{

    volatile UINT32 i;

    if (time_in_ticks)
    {
        for (i = 0; i < time_in_ticks; i++) {;}
    }
}
void freqhop_sram_blkcpy(UINT32 pll_id, UINT32 *pDDS)
{
	volatile UINT32 i;

	freqhop_sram_enable_with_offset(FREQHOP_PLLID2SRAMOFFSET(pll_id));
	for (i=0; i<SRAM_TABLE_SIZE_BY_PLL; i++)
	{
		freqhop_sram_write_data(*pDDS);
		pDDS ++;
	}
}
void freqhop_rf_src_hopping_enable(UINT32 rf_id, UINT32 enable)
{
	UINT32 mask;
	UINT32 reg;

	mask	= fhctl_rfid2rfenbitmap[rf_id];

	reg		= fhctl_read_reg(FHCTL_CFG);

	//- clear original value
	reg &= (~mask);
	if(enable)
	{	//- set value
		reg |= mask;
	}
	//- write value
	fhctl_write_reg(reg, FHCTL_CFG);
}
void freqhop_set_priority(UINT32 order, UINT32 order_md)
{
	UINT32 reg = fhctl_read_reg(FHCTL_CFG);

	fhctl_clr_and_set_field (reg, order, RG_FHCTL_ORDER);
	fhctl_clr_and_set_field (reg, order_md, RG_FHCTL_ORDER_MD);

	fhctl_write_reg(reg, FHCTL_CFG);
}
void freqhop_set_dma_mode(UINT32 mode)
{
	UINT32 reg 	= fhctl_read_reg(FHDMA_CFG);

	fhctl_clr_and_set_field (reg, mode, RG_FHDMA_MODE);

	fhctl_write_reg(reg, FHDMA_CFG);
}
void freqhop_rf_src_trigger_ch(UINT32 rf_id, UINT32 channel)
{
	fhctl_write_reg(channel, FHCTL_RFx_CH(rf_id));
}
UINT32 freqhop_get_pll_mon_dss(UINT32 pll_id)
{
	UINT32 reg = fhctl_read_reg(FHCTLx_MON(pll_id)) & RG_FHCTLx_DDS_MASK;

	//UINT32 dds = fhctl_read_reg(FHCTLx_MON(pll_id)) & 0x1fffff;
	fhctl_info("[FreqHop] %s() MON_DDS_%d = 0x%08X\n", __func__, pll_id, reg) ;

	return reg;
}
UINT32 freqhop_get_pll_fhctlx_dss(UINT32 pll_id)
{
	UINT32 reg = fhctl_read_reg(FHCTLx_DDS(pll_id)) & RG_FHCTLx_DDS_MASK;

	//UINT32 dds = fhctl_read_reg(FHCTLx_MON(pll_id)) & 0x1fffff;
	fhctl_info ("[FreqHop] %s() fhctl_DDS_%d = 0x%08X\n", __func__, pll_id, reg) ;

	return reg;
}
void freqhop_sram_init(UINT32 *pDDS)
{
	volatile UINT32 i;

	freqhop_sram_enable_with_offset(FREQHOP_PLLID2SRAMOFFSET(ARMPLL_ID));
	for (i=0; i<SRAM_TABLE_SIZE_BY_PLL*NUM_OF_PLL_ID; i++)
	{
		freqhop_sram_write_data(*pDDS);
		pDDS ++;
	}
}
void freqhop_set_fhctlx_updnlmt(UINT32 pll_id, UINT32 uplimit, UINT32 downlimit)
{
	UINT32 rval = fhctl_read_reg(FHCTLx_UPDNLMT(pll_id));

	fhctl_clr_and_set_field (rval, uplimit, RG_FRDDSx_UPLMT);
	fhctl_clr_and_set_field (rval, downlimit, RG_FRDDSx_DNLMT);

	fhctl_write_reg(rval, FHCTLx_UPDNLMT(pll_id));
}
void freqhop_set_fhctlx_slope(UINT32 pll_id, UINT32 dts, UINT32 dys)
{
	UINT32 rval = fhctl_read_reg(FHCTLx_CFG(pll_id));

	fhctl_clr_and_set_field (rval, dts, RG_FRDDSx_DTS);
	fhctl_clr_and_set_field (rval, dys, RG_FRDDSx_DYS);

	fhctl_write_reg(rval, FHCTLx_CFG(pll_id));
}

#endif //END !__FHCTL_CTP__