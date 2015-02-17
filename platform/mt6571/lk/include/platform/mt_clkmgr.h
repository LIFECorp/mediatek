
#ifndef __MT_CLKMGR_H__
#define __MT_CLKMGR_H__


#define BIT(_bit_)                  (unsigned int)(1 << (_bit_))
#define BITS(_bits_, _val_)         ((BIT(((1)?_bits_)+1)-BIT(((0)?_bits_))) & (_val_<<((0)?_bits_)))
#define BITMASK(_bits_)             (BIT(((1)?_bits_)+1)-BIT(((0)?_bits_)))


//
// TOP_CLOCK_CTRL
//

#define TOP_CLOCK_CTRL_BASE     TOPCKGEN_BASE

#define CLK_MUX_SEL0            (TOP_CLOCK_CTRL_BASE + 0x0000)
#define CLK_MUX_SEL1            (TOP_CLOCK_CTRL_BASE + 0x0004)
#define CLK_GATING_CTRL0        (TOP_CLOCK_CTRL_BASE + 0x0020)
#define CLK_GATING_CTRL1        (TOP_CLOCK_CTRL_BASE + 0x0024)
#define INFRABUS_DCMCTL1        (TOP_CLOCK_CTRL_BASE + 0x002C)
#define MPLL_FREDIV_EN          (TOP_CLOCK_CTRL_BASE + 0x0030)
#define UPLL_FREDIV_EN          (TOP_CLOCK_CTRL_BASE + 0x0034)
#define CLK_GATING_CTRL7        (TOP_CLOCK_CTRL_BASE + 0x003C)
#define SET_CLK_GATING_CTRL0    (TOP_CLOCK_CTRL_BASE + 0x0050)
#define SET_CLK_GATING_CTRL1    (TOP_CLOCK_CTRL_BASE + 0x0054)
#define SET_MPLL_FREDIV_EN      (TOP_CLOCK_CTRL_BASE + 0x0060)
#define SET_UPLL_FREDIV_EN      (TOP_CLOCK_CTRL_BASE + 0x0064)
#define SET_CLK_GATING_CTRL7    (TOP_CLOCK_CTRL_BASE + 0x006C)
#define CLR_CLK_GATING_CTRL0    (TOP_CLOCK_CTRL_BASE + 0x0080)
#define CLR_CLK_GATING_CTRL1    (TOP_CLOCK_CTRL_BASE + 0x0084)
#define CLR_MPLL_FREDIV_EN      (TOP_CLOCK_CTRL_BASE + 0x0090)
#define CLR_UPLL_FREDIV_EN      (TOP_CLOCK_CTRL_BASE + 0x0094)
#define CLR_CLK_GATING_CTRL7    (TOP_CLOCK_CTRL_BASE + 0x009C)

//
// MMSYS_CONFIG
//

#define MMSYS_CG_CON0           (MMSYS_CONFIG_BASE + 0x100)
#define MMSYS_CG_SET0           (MMSYS_CONFIG_BASE + 0x104)
#define MMSYS_CG_CLR0           (MMSYS_CONFIG_BASE + 0x108)
#define MMSYS_CG_CON1           (MMSYS_CONFIG_BASE + 0x110)
#define MMSYS_CG_SET1           (MMSYS_CONFIG_BASE + 0x114)
#define MMSYS_CG_CLR1           (MMSYS_CONFIG_BASE + 0x118)

// CG_CTRL0 (clk_swcg_0)
#define PWM_MM_SW_CG_BIT        BIT(0)
#define CAM_MM_SW_CG_BIT        BIT(1)
#define MFG_MM_SW_CG_BIT        BIT(2)
#define SPM_52M_SW_CG_BIT       BIT(3)
#define MIPI_26M_DBG_EN_BIT     BIT(4)
#define DBI_BCLK_SW_CG_BIT      BIT(5)
#define SC_26M_CK_SEL_EN_BIT    BIT(6)
#define SC_MEM_CK_OFF_EN_BIT    BIT(7)

#define DBI_PAD0_SW_CG_BIT      BIT(16)
#define DBI_PAD1_SW_CG_BIT      BIT(17)
#define DBI_PAD2_SW_CG_BIT      BIT(18)
#define DBI_PAD3_SW_CG_BIT      BIT(19)
#define GPU_491P52M_EN_BIT      BIT(20)
#define GPU_500P5M_EN_BIT       BIT(21)

#define ARMDCM_CLKOFF_EN_BIT    BIT(31)


// CG_CTRL1 (rg_peri_sw_cg)
#define EFUSE_SW_CG_BIT         BIT(0)
#define THEM_SW_CG_BIT          BIT(1)
#define APDMA_SW_CG_BIT         BIT(2)
#define I2C0_SW_CG_BIT          BIT(3)
#define I2C1_SW_CG_BIT          BIT(4)
#define AUX_SW_CG_MD_BIT        BIT(5) // XXX: NOT USED @ AP SIDE
#define NFI_SW_CG_BIT           BIT(6)
#define NFIECC_SW_CG_BIT        BIT(7)

#define DEBUGSYS_SW_CG_BIT      BIT(8)
#define PWM_SW_CG_BIT           BIT(9)
#define UART0_SW_CG_BIT         BIT(10)
#define UART1_SW_CG_BIT         BIT(11)
#define BTIF_SW_CG_BIT          BIT(12)
#define USB_SW_CG_BIT           BIT(13)
#define FHCTL_SW_CG_BIT         BIT(14)
#define AUX_SW_CG_THERM_BIT     BIT(15)

#define SPINFI_SW_CG_BIT        BIT(16)
#define MSDC0_SW_CG_BIT         BIT(17)
#define MSDC1_SW_CG_BIT         BIT(18)
#define NFI2X_SW_CG_BIT         BIT(19)
#define PMIC_SW_CG_AP_BIT       BIT(20)
#define SEJ_SW_CG_BIT           BIT(21)
#define MEMSLP_DLYER_SW_CG_BIT  BIT(22)
#define SPI_SW_CG_BIT           BIT(23)
#define APXGPT_SW_CG_BIT        BIT(24)
#define AUDIO_SW_CG_BIT         BIT(25)
#define SPM_SW_CG_BIT           BIT(26)
#define PMIC_SW_CG_MD_BIT       BIT(27) // XXX: NOT USED @ AP SIDE
#define PMIC_SW_CG_CONN_BIT     BIT(28) // XXX: NOT USED @ AP SIDE
#define PMIC_26M_SW_CG_BIT      BIT(29)
#define AUX_SW_CG_ADC_BIT       BIT(30)
#define AUX_SW_CG_TP_BIT        BIT(31)

// CG_CTRL7
#define XIU2AHB0_SW_CG_BIT      BIT(0)
#define RBIST_SW_CG_BIT         BIT(1)
#define NFI_HCLK_SW_CG_BIT      BIT(2)

// CG_MMSYS0
#define SMI_COMMON_SW_CG_BIT        BIT(0)
#define SMI_LARB0_SW_CG_BIT         BIT(1)
#define MM_CMDQ_ENGINE_SW_CG_BIT    BIT(2)
//#define MM_CMDQ_SW_CG_BIT           MM_CMDQ_ENGINE_SW_CG_BIT
#define MM_CMDQ_SMI_SW_CG_BIT       BIT(3)
//#define MM_CMDQ_SMI_IF_SW_CG_BIT    MM_CMDQ_SMI_SW_CG_BIT
#define DISP_PQ_SW_CG_BIT           BIT(4)
#define DISP_COLOR_SW_CG_BIT        DISP_PQ_SW_CG_BIT
#define DISP_BLS_SW_CG_BIT          BIT(5)
#define DISP_WDMA_SW_CG_BIT         BIT(6)
#define DISP_RDMA_SW_CG_BIT         BIT(7)
#define DISP_OVL_SW_CG_BIT          BIT(8)
#define MDP_SHP_SW_CG_BIT           BIT(9)
#define MDP_TDSHP_SW_CG_BIT         MDP_SHP_SW_CG_BIT
#define MDP_WROT_SW_CG_BIT          BIT(10)
#define MDP_RSZ_SW_CG_BIT           BIT(11)
#define MDP_RDMA_SW_CG_BIT          BIT(12)
#define DISP_PWM_SW_CG_BIT          BIT(13)
#define DISP_PWM_26M_SW_CG_BIT      BIT(14)
#define MM_CAM_DP_SW_CG_BIT         BIT(15)
#define MM_CAM_DP2_SW_CG_BIT        BIT(16)
#define MM_CAM_SMI_SW_CG_BIT        BIT(17)
#define MM_SENINF_SW_CG_BIT         BIT(18)
#define MM_CAMTG_SW_CG_BIT          BIT(19)
#define MM_CODEC_SW_CG_BIT          BIT(20)
#define DISP_FAKE_ENG_SW_CG_BIT     BIT(21)
#define MUTEX_SLOW_CLOCK_SW_CG_BIT  BIT(22)

// CG_MMSYS1
#define DSI_ENGINE_SW_CG_BIT        BIT(0)
#define DSI_DIGITAL_SW_CG_BIT       BIT(1)
#define DISP_DPI_ENGINE_SW_CG_BIT   BIT(2)
#define DISP_DPI_IF_SW_CG_BIT       BIT(3)
#define DISP_DBI_ENGINE_SW_CG_BIT   BIT(4)
/* DISP_DBI_SMI_SW_CG_BIT is removed in MT6571 */
#define DISP_DBI_IF_SW_CG_BIT       BIT(5)

#define clk_writel                  spm_write

#endif

