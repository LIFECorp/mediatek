// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

/**
 * @file mt_clkmgr_internal.h
 * @brief Clock Manager driver private interface
 */

#ifndef __MT_CLKMGR_INTERNAL_H__
#define __MT_CLKMGR_INTERNAL_H__

#ifdef __cplusplus
// extern "C" { // TODO: disable temp
#endif

#if defined(__WIN32) || defined(__CYGWIN__) // MinGW32 or Cygwin gcc

#define CONFIG_CLKMGR_EMULATION

#endif

/*=============================================================*/
// Include files
/*=============================================================*/

// system includes
#include <linux/bitops.h>

// project includes
#include "mach/mt_reg_base.h"

// local includes

// forward references


/*=============================================================*/
// Macro definition
/*=============================================================*/

//
// Register Base Address
//

//
// APMIXED
//

#ifndef APMIXEDSYS_BASE
#define APMIXEDSYS_BASE         APMIXED_BASE
#endif

#define AP_PLL_CON0             (APMIXEDSYS_BASE + 0x0000)
#define AP_PLL_CON1             (APMIXEDSYS_BASE + 0x0004)
#define AP_PLL_CON2             (APMIXEDSYS_BASE + 0x0008)
#define AP_PLL_CON3             (APMIXEDSYS_BASE + 0x000C)

#define PLL_HP_CON0             (APMIXEDSYS_BASE + 0x0014)

#define ARMPLL_CON0             (APMIXEDSYS_BASE + 0x0100)
#define ARMPLL_CON1             (APMIXEDSYS_BASE + 0x0104)
#define ARMPLL_CON2             (APMIXEDSYS_BASE + 0x0108)
#define ARMPLL_PWR_CON0         (APMIXEDSYS_BASE + 0x0110)

#define MAINPLL_CON0            (APMIXEDSYS_BASE + 0x0120)
#define MAINPLL_CON1            (APMIXEDSYS_BASE + 0x0124)
#define MAINPLL_CON2            (APMIXEDSYS_BASE + 0x0128)
#define MAINPLL_PWR_CON0        (APMIXEDSYS_BASE + 0x0130)

#define UNIVPLL_CON0            (APMIXEDSYS_BASE + 0x0140)
#define UNIVPLL_PWR_CON0        (APMIXEDSYS_BASE + 0x0150)

#define WHPLL_CON0              (APMIXEDSYS_BASE + 0x0240)
#define WHPLL_CON1              (APMIXEDSYS_BASE + 0x0244)
#define WHPLL_CON2              (APMIXEDSYS_BASE + 0x0248)
#define WHPLL_PWR_CON0          (APMIXEDSYS_BASE + 0x0250)
#define WHPLL_PATHSEL_CON       (APMIXEDSYS_BASE + 0x0254)
#define RSV_RW0_CON1            (APMIXEDSYS_BASE + 0x0F04)

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

//
// MFG_CONFIG
//

#define MFG_CONFIG_BASE         G3D_CONFIG_BASE

#define MFG_CG_CON              (MFG_CONFIG_BASE + 0x0000)
#define MFG_CG_SET              (MFG_CONFIG_BASE + 0x0004)
#define MFG_CG_CLR              (MFG_CONFIG_BASE + 0x0008)

//
// AUDIO_SYS_TOP
//

#define AUDIO_SYS_TOP_BASE      AUDIO_BASE

#define AUDIO_TOP_CON0          (AUDIO_SYS_TOP_BASE + 0x0000)

//
// Register Address & Mask
//

// CG_MIXED
#define USB48M_EN_BIT           BIT(28)
#define UNIV48M_EN_BIT          BIT(29)

// TODO: set @ init is better
#define CG_MIXED_MASK           USB48M_EN_BIT \
                              | UNIV48M_EN_BIT

// CG_MPLL
#define MPLL_D2_EN_BIT          BIT(0)
#define MPLL_D3_EN_BIT          BIT(1)
#define MPLL_D5_EN_BIT          BIT(2)
#define MPLL_D7_EN_BIT          BIT(3)

#define MPLL_D4_EN_BIT          BIT(8)
#define MPLL_D6_EN_BIT          BIT(9)
#define MPLL_D10_EN_BIT         BIT(10)
#define MPLL_D14_EN_BIT         BIT(11)

#define MPLL_D8_EN_BIT          BIT(16)
#define MPLL_D12_EN_BIT         BIT(17)
#define MPLL_D20_EN_BIT         BIT(18)
#define MPLL_D28_EN_BIT         BIT(19)

#define MPLL_D16_EN_BIT         BIT(23)
#define MPLL_D24_EN_BIT         BIT(24)
#define MPLL_D40_EN_BIT         BIT(25)

// TODO: set @ init is better
#define CG_MPLL_MASK           (MPLL_D2_EN_BIT  \
                              | MPLL_D3_EN_BIT  \
                              | MPLL_D5_EN_BIT  \
                              | MPLL_D7_EN_BIT  \
                              | MPLL_D4_EN_BIT  \
                              | MPLL_D6_EN_BIT  \
                              | MPLL_D10_EN_BIT \
                              | MPLL_D14_EN_BIT \
                              | MPLL_D8_EN_BIT  \
                              | MPLL_D12_EN_BIT \
                              | MPLL_D20_EN_BIT \
                              | MPLL_D28_EN_BIT \
                              | MPLL_D16_EN_BIT \
                              | MPLL_D24_EN_BIT \
                              | MPLL_D40_EN_BIT)

// CG_UPLL
#define UPLL_D2_EN_BIT          BIT(0)
#define UPLL_D3_EN_BIT          BIT(1)
#define UPLL_D5_EN_BIT          BIT(2)
#define UPLL_D7_EN_BIT          BIT(3)

#define UPLL_D4_EN_BIT          BIT(8)
#define UPLL_D6_EN_BIT          BIT(9)
#define UPLL_D10_EN_BIT         BIT(10)

#define UPLL_D8_EN_BIT          BIT(16)
#define UPLL_D12_EN_BIT         BIT(17)
#define UPLL_D20_EN_BIT         BIT(18)

#define UPLL_D16_EN_BIT         BIT(24)
#define UPLL_D24_EN_BIT         BIT(25)

// TODO: set @ init is better
#define CG_UPLL_MASK           (UPLL_D2_EN_BIT  \
                              | UPLL_D3_EN_BIT  \
                              | UPLL_D5_EN_BIT  \
                              | UPLL_D7_EN_BIT  \
                              | UPLL_D4_EN_BIT  \
                              | UPLL_D6_EN_BIT  \
                              | UPLL_D10_EN_BIT \
                              | UPLL_D8_EN_BIT  \
                              | UPLL_D12_EN_BIT \
                              | UPLL_D20_EN_BIT \
                              | UPLL_D16_EN_BIT \
                              | UPLL_D24_EN_BIT)

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

// TODO: set @ init is better
#define CG_CTRL0_MASK          (PWM_MM_SW_CG_BIT        \
                              | CAM_MM_SW_CG_BIT        \
                              | MFG_MM_SW_CG_BIT        \
                              | SPM_52M_SW_CG_BIT       \
                              | MIPI_26M_DBG_EN_BIT     \
                              | DBI_BCLK_SW_CG_BIT      \
                              | SC_26M_CK_SEL_EN_BIT    \
                              | SC_MEM_CK_OFF_EN_BIT    \
                              | DBI_PAD0_SW_CG_BIT      \
                              | DBI_PAD1_SW_CG_BIT      \
                              | DBI_PAD2_SW_CG_BIT      \
                              | DBI_PAD3_SW_CG_BIT      \
                              | GPU_491P52M_EN_BIT      \
                              | GPU_500P5M_EN_BIT       \
                              | ARMDCM_CLKOFF_EN_BIT)

// enable bit @ CG_CTRL0
#define CG_CTRL0_EN_MASK       (MIPI_26M_DBG_EN_BIT     \
                              | SC_26M_CK_SEL_EN_BIT    \
                              | SC_MEM_CK_OFF_EN_BIT    \
                              | GPU_491P52M_EN_BIT      \
                              | GPU_500P5M_EN_BIT       \
                              | ARMDCM_CLKOFF_EN_BIT)

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

// TODO: set @ init is better
#define CG_CTRL1_MASK          (EFUSE_SW_CG_BIT         \
                              | THEM_SW_CG_BIT          \
                              | APDMA_SW_CG_BIT         \
                              | I2C0_SW_CG_BIT          \
                              | I2C1_SW_CG_BIT          \
                              | NFI_SW_CG_BIT           \
                              | NFIECC_SW_CG_BIT        \
                              | DEBUGSYS_SW_CG_BIT      \
                              | PWM_SW_CG_BIT           \
                              | UART0_SW_CG_BIT         \
                              | UART1_SW_CG_BIT         \
                              | BTIF_SW_CG_BIT          \
                              | USB_SW_CG_BIT           \
                              | FHCTL_SW_CG_BIT         \
                              | SPINFI_SW_CG_BIT        \
                              | MSDC0_SW_CG_BIT         \
                              | MSDC1_SW_CG_BIT         \
                              | NFI2X_SW_CG_BIT         \
                              | PMIC_SW_CG_AP_BIT       \
                              | SEJ_SW_CG_BIT           \
                              | MEMSLP_DLYER_SW_CG_BIT  \
                              | SPI_SW_CG_BIT           \
                              | APXGPT_SW_CG_BIT        \
                              | AUDIO_SW_CG_BIT         \
                              | SPM_SW_CG_BIT           \
                              | PMIC_26M_SW_CG_BIT      \
                              | AUX_SW_CG_ADC_BIT)

/*
                              | AUX_SW_CG_THERM_BIT     \
                              | AUX_SW_CG_MD_BIT        \
                              | AUX_SW_CG_TP_BIT        \
                              | PMIC_SW_CG_MD_BIT       \
                              | PMIC_SW_CG_CONN_BIT     \
*/

// CG_CTRL7
#define XIU2AHB0_SW_CG_BIT      BIT(0)
#define RBIST_SW_CG_BIT         BIT(1)
#define NFI_HCLK_SW_CG_BIT      BIT(2)

#define CG_CTRL7_MASK          (XIU2AHB0_SW_CG_BIT      \
                              | RBIST_SW_CG_BIT         \
                              | NFI_HCLK_SW_CG_BIT)

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

#ifdef EMULATION_EARLY_PORTING
    #define MDP_WDMA_SW_CG_BIT          BIT(11)
    #define MDP_RSZ1_SW_CG_BIT          BIT(12)
    #define MDP_RSZ0_SW_CG_BIT          BIT(13)
    #define MDP_RDMA_SW_CG_BIT          BIT(14)
    #define MDP_BLS_26M_SW_CG_BIT       BIT(15)
    #define MM_CAM_SW_CG_BIT            BIT(16)
    #define MM_SENINF_SW_CG_BIT         BIT(17)
    #define MM_CAMTG_SW_CG_BIT          BIT(18)
    #define MM_CODEC_SW_CG_BIT          BIT(19)
    #define DISP_FAKE_ENG_SW_CG_BIT     BIT(20)
    #define MUTEX_SLOW_CLOCK_SW_CG_BIT  BIT(21)
#else
    // FIXME: should not be used
    #define MDP_WDMA_SW_CG_BIT          0
    #define MDP_RSZ1_SW_CG_BIT          0
    #define MDP_RSZ0_SW_CG_BIT          0
    #define MDP_BLS_26M_SW_CG_BIT       0
    #define MM_CAM_SW_CG_BIT            0

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
#endif

#ifdef EMULATION_EARLY_PORTING
// TODO: set @ init is better
#define CG_MMSYS0_MASK         (SMI_COMMON_SW_CG_BIT        \
                              | SMI_LARB0_SW_CG_BIT         \
                              | MM_CMDQ_ENGINE_SW_CG_BIT    \
                              | MM_CMDQ_SMI_SW_CG_BIT    \
                              | DISP_COLOR_SW_CG_BIT        \
                              | DISP_BLS_SW_CG_BIT          \
                              | DISP_WDMA_SW_CG_BIT         \
                              | DISP_RDMA_SW_CG_BIT         \
                              | DISP_OVL_SW_CG_BIT          \
                              | MDP_TDSHP_SW_CG_BIT         \
                              | MDP_WROT_SW_CG_BIT          \
                              | MDP_WDMA_SW_CG_BIT          \
                              | MDP_RSZ1_SW_CG_BIT          \
                              | MDP_RSZ0_SW_CG_BIT          \
                              | MDP_RDMA_SW_CG_BIT          \
                              | MDP_BLS_26M_SW_CG_BIT       \
                              | MM_CAM_SW_CG_BIT            \
                              | MM_SENINF_SW_CG_BIT         \
                              | MM_CAMTG_SW_CG_BIT          \
                              | MM_CODEC_SW_CG_BIT          \
                              | DISP_FAKE_ENG_SW_CG_BIT     \
                              | MUTEX_SLOW_CLOCK_SW_CG_BIT)

#else

#define CG_MMSYS0_MASK           \
    ( SMI_COMMON_SW_CG_BIT       \
    | SMI_LARB0_SW_CG_BIT        \
    | MM_CMDQ_ENGINE_SW_CG_BIT   \
    | MM_CMDQ_SMI_SW_CG_BIT      \
    | DISP_PQ_SW_CG_BIT          \
    | DISP_BLS_SW_CG_BIT         \
    | DISP_WDMA_SW_CG_BIT        \
    | DISP_RDMA_SW_CG_BIT        \
    | DISP_OVL_SW_CG_BIT         \
    | MDP_SHP_SW_CG_BIT          \
    | MDP_WROT_SW_CG_BIT         \
    | MDP_RSZ_SW_CG_BIT          \
    | MDP_RDMA_SW_CG_BIT         \
    | DISP_PWM_SW_CG_BIT     \
    | DISP_PWM_26M_SW_CG_BIT \
    | MM_CAM_DP_SW_CG_BIT        \
    | MM_CAM_DP2_SW_CG_BIT       \
    | MM_CAM_SMI_SW_CG_BIT       \
    | MM_SENINF_SW_CG_BIT        \
    | MM_CAMTG_SW_CG_BIT         \
    | MM_CODEC_SW_CG_BIT         \
    | DISP_FAKE_ENG_SW_CG_BIT    \
    | MUTEX_SLOW_CLOCK_SW_CG_BIT )
#endif

// CG_MMSYS1
#define DSI_ENGINE_SW_CG_BIT        BIT(0)
#define DSI_DIGITAL_SW_CG_BIT       BIT(1)
#define DISP_DPI_ENGINE_SW_CG_BIT   BIT(2)
#define DISP_DPI_IF_SW_CG_BIT       BIT(3)
#define DISP_DBI_ENGINE_SW_CG_BIT   BIT(4)

#ifdef EMULATION_EARLY_PORTING
#define DISP_DBI_SMI_SW_CG_BIT      BIT(5)
#define DISP_DBI_IF_SW_CG_BIT       BIT(6)
#else
/* DISP_DBI_SMI_SW_CG_BIT is removed in MT6571 */
#define DISP_DBI_SMI_SW_CG_BIT      0
#define DISP_DBI_IF_SW_CG_BIT       BIT(5)
#endif

// TODO: set @ init is better
#define CG_MMSYS1_MASK         (DSI_ENGINE_SW_CG_BIT        \
                              | DSI_DIGITAL_SW_CG_BIT       \
                              | DISP_DPI_ENGINE_SW_CG_BIT   \
                              | DISP_DPI_IF_SW_CG_BIT       \
                              | DISP_DBI_ENGINE_SW_CG_BIT   \
                              | DISP_DBI_SMI_SW_CG_BIT      \
                              | DISP_DBI_IF_SW_CG_BIT)

// CG_MFG
#define MFG_PDN_BG3D_SW_CG_BIT  BIT(0)

// TODO: set @ init is better
#define CG_MFG_MASK            (MFG_PDN_BG3D_SW_CG_BIT)

// CG_AUDIO
#define AUD_PDN_AFE_EN_BIT          BIT(2)
#define AUD_PDN_I2S_EN_BIT          BIT(6)
#define AUD_PDN_ADC_EN_BIT          BIT(24)
#define AUD_PDN_DAC_EN_BIT          BIT(25)
#define AUD_PDN_DAC_PREDIS_EN_BIT   BIT(26)
#define AUD_PDN_TML_EN_BIT          BIT(27)

// TODO: set @ init is better
#define CG_AUDIO_MASK          (AUD_PDN_AFE_EN_BIT          \
                              | AUD_PDN_I2S_EN_BIT          \
                              | AUD_PDN_ADC_EN_BIT          \
                              | AUD_PDN_DAC_EN_BIT          \
                              | AUD_PDN_DAC_PREDIS_EN_BIT   \
                              | AUD_PDN_TML_EN_BIT)


// CG_VIRTUAL
#define VCG_BUS_BIT            BIT(0)

// virtual CG for MT_CG_APDMA_SW_CG
#define VCG_APDMA_WLAN_BIT     BIT(1)
#define VCG_APDMA_BTIF_BIT     BIT(2)
#define VCG_APDMA_I2C_BIT      BIT(3)
#define VCG_APDMA_UART_BIT     BIT(4)

// virtual CG for MT_CG_AUX_SW_CG_ADC
#define VCG_AUX_ADC_BIT        BIT(5)
#define VCG_AUX_TP_BIT         BIT(6)
#define VCG_AUX_THERM_BIT      BIT(7)

// virtual CG for MT_CG_MUTEX_SLOW_CLOCK
#define VCG_MUTEX_SLOW_CLOCK_DISP_BIT    BIT(8)
#define VCG_MUTEX_SLOW_CLOCK_M4U_BIT     BIT(9)

#define CG_VIRTUAL_MASK        (VCG_BUS_CG_BIT                \
                              | VCG_APDMA_WLAN_BIT            \
                              | VCG_APDMA_BTIF_BIT            \
                              | VCG_APDMA_I2C_BIT             \
                              | VCG_APDMA_UART_BIT            \
                              | VCG_AUX_ADC_BIT               \
                              | VCG_AUX_TP_BIT                \
                              | VCG_AUX_THERM_BIT             \
                              | VCG_MUTEX_SLOW_CLOCK_DISP_BIT \
                              | VCG_MUTEX_SLOW_CLOCK_M4U_BIT)

/*=============================================================*/
// Type definition
/*=============================================================*/


/*=============================================================*/
// Global variable definition
/*=============================================================*/


/*=============================================================*/
// Global function definition
/*=============================================================*/


#ifdef __cplusplus
// } // TODO: disable temp
#endif

#endif // __MT_CLKMGR_INTERNAL_H__

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

/**
 * @file mt_clkmgr.h
 * @brief Clock Manager driver public interface
 */

#ifndef __MT_CLKMGR_H__
#define __MT_CLKMGR_H__

#ifdef __cplusplus
// extern "C" { // TODO: disable temp
#endif

#ifdef __MT_CLKMGR_C__
  #define EXTERN
#else
  #define EXTERN extern
#endif

/*=============================================================*/
// Include files
/*=============================================================*/

// system includes
#include <linux/list.h>

// project includes
#include "mach/mt_typedefs.h"

// local includes

// forward references


/*=============================================================*/
// Macro definition
/*=============================================================*/


/*=============================================================*/
// Type definition
/*=============================================================*/

#define PWR_DOWN    0
#define PWR_ON      1

//
// CG_GPR ID
//
typedef enum
{
    CG_MIXED    = 0,
    CG_MPLL     = 1,
    CG_UPLL     = 2,
    CG_CTRL0    = 3,
    CG_CTRL1    = 4,
    CG_CTRL7    = 5,
    CG_MMSYS0   = 6,
    CG_MMSYS1   = 7,
    CG_MFG      = 8,
    CG_AUDIO    = 9,
    CG_VIRTUAL  = 10,

    NR_GRPS,
} cg_grp_id;

//
// CG_CLK ID
//
#define BEG_CG_GRP(grp_name, item)      item, grp_name##_FROM = item
#define END_CG_GRP(grp_name, item)      item, grp_name##_TO = item

#define BEG_END_CG_GRP(grp_name, item)  item, grp_name##_FROM = item, grp_name##_TO = item

#define FROM_CG_GRP_ID(grp_name)        grp_name##_FROM
#define TO_CG_GRP_ID(grp_name)          grp_name##_TO
#define NR_CG_GRP_CLKS(grp_name)        (grp_name##_TO - grp_name##_FROM + 1)


/*
 * FIXME: These are renamed or removed in 6571.
 */

#if 0
/* CG_MMSYS0 */
#define MT_CG_DISP_COLOR_SW_CG       MT_CG_DISP_PQ_SW_CG
#define MT_CG_MDP_TDSHP_SW_CG        MT_CG_MDP_SHP_SW_CG

#ifndef EMULATION_EARLY_PORTING
/* CG_MMSYS0 */
#define MT_CG_MDP_WDMA_SW_CG          MT_CG_INVALID
#define MT_CG_MDP_RSZ1_SW_CG          MT_CG_INVALID
#define MT_CG_MDP_RSZ0_SW_CG          MT_CG_INVALID
#define MT_CG_MDP_BLS_26M_SW_CG       MT_CG_INVALID
#define MT_CG_MM_CAM_SW_CG            MT_CG_INVALID
#endif

/* CG_MMSYS1 */
#define MT_CG_DISP_DBI_SMI_SW_CG     MT_CG_INVALID

#endif

typedef enum cg_clk_id
{
    // CG_MIXED
    BEG_CG_GRP(CG_MIXED, MT_CG_SYS_26M),
                      // MT_CG_UNIV_48M,
    END_CG_GRP(CG_MIXED, MT_CG_USB_48M),

    // CG_MPLL
    BEG_CG_GRP(CG_MPLL,  MT_CG_MPLL_D2),
                         MT_CG_MPLL_D3,
                         MT_CG_MPLL_D5,
                         MT_CG_MPLL_D7,

                         MT_CG_MPLL_D4,
                         MT_CG_MPLL_D6,
                         MT_CG_MPLL_D10,
                         MT_CG_MPLL_D14,

                         MT_CG_MPLL_D8,
                         MT_CG_MPLL_D12,
                         MT_CG_MPLL_D20,
                         MT_CG_MPLL_D28,

                         MT_CG_MPLL_D16,
                         MT_CG_MPLL_D24,
    END_CG_GRP(CG_MPLL,  MT_CG_MPLL_D40),

    // CG_UPLL
    BEG_CG_GRP(CG_UPLL,  MT_CG_UPLL_D2),
                         MT_CG_UPLL_D3,
                         MT_CG_UPLL_D5,
                         MT_CG_UPLL_D7,

                         MT_CG_UPLL_D4,
                         MT_CG_UPLL_D6,
                         MT_CG_UPLL_D10,

                         MT_CG_UPLL_D8,
                         MT_CG_UPLL_D12,
                         MT_CG_UPLL_D20,

                         MT_CG_UPLL_D16,
    END_CG_GRP(CG_UPLL,  MT_CG_UPLL_D24),

    // CG_CTRL0 (PERI/INFRA)
    BEG_CG_GRP(CG_CTRL0, MT_CG_PWM_MM_SW_CG),
                         MT_CG_CAM_MM_SW_CG,
                         MT_CG_MFG_MM_SW_CG,
                         MT_CG_SPM_52M_SW_CG,
                         MT_CG_MIPI_26M_DBG_EN,
                         MT_CG_DBI_BCLK_SW_CG,
                         MT_CG_SC_26M_CK_SEL_EN,
                         MT_CG_SC_MEM_CK_OFF_EN,

                         MT_CG_DBI_PAD0_SW_CG,
                         MT_CG_DBI_PAD1_SW_CG,
                         MT_CG_DBI_PAD2_SW_CG,
                         MT_CG_DBI_PAD3_SW_CG,
                         MT_CG_GPU_491P52M_EN,
                         MT_CG_GPU_500P5M_EN,

    END_CG_GRP(CG_CTRL0, MT_CG_ARMDCM_CLKOFF_EN),

    // CG_CTRL1 (PERI/INFRA)
    BEG_CG_GRP(CG_CTRL1, MT_CG_EFUSE_SW_CG),
                         MT_CG_THEM_SW_CG,
                         MT_CG_APDMA_SW_CG,
                         MT_CG_I2C0_SW_CG,
                         MT_CG_I2C1_SW_CG,
                      // MT_CG_AUX_SW_CG_MD,
                         MT_CG_NFI_SW_CG,
                      // MT_CG_NFIECC_SW_CG,
                         MT_CG_DEBUGSYS_SW_CG,
                         MT_CG_PWM_SW_CG,
                         MT_CG_UART0_SW_CG,
                         MT_CG_UART1_SW_CG,
                         MT_CG_BTIF_SW_CG,
                         MT_CG_USB_SW_CG,
                         MT_CG_FHCTL_SW_CG,
                      // MT_CG_AUX_SW_CG_THERM,
                         MT_CG_SPINFI_SW_CG,
                         MT_CG_MSDC0_SW_CG,
                         MT_CG_MSDC1_SW_CG,
                         MT_CG_NFI2X_SW_CG,
                         MT_CG_PMIC_SW_CG_AP,
                         MT_CG_SEJ_SW_CG,
                         MT_CG_MEMSLP_DLYER_SW_CG,
                         MT_CG_SPI_SW_CG,
                         MT_CG_APXGPT_SW_CG,
                         MT_CG_AUDIO_SW_CG,
                         MT_CG_SPM_SW_CG,
                      // MT_CG_PMIC_SW_CG_MD,
                      // MT_CG_PMIC_SW_CG_CONN,
                         MT_CG_PMIC_26M_SW_CG,
    END_CG_GRP(CG_CTRL1, MT_CG_AUX_SW_CG_ADC),
                      // MT_CG_AUX_SW_CG_TP,

    BEG_CG_GRP(CG_CTRL7, MT_CG_XIU2AHB0_SW_CG),
                         MT_CG_RBIST_SW_CG,
    END_CG_GRP(CG_CTRL7, MT_CG_NFI_HCLK_SW_CG),

    // CG_MMSYS0
                       // MT_CG_SMI_COMMON_SW_CG,
                       // MT_CG_SMI_LARB0_SW_CG,
                       // MT_CG_MM_CMDQ_ENGINE_SW_CG,
                       // MT_CG_MM_CMDQ_SMI_SW_CG,
    BEG_CG_GRP(CG_MMSYS0, MT_CG_DISP_PQ_SW_CG),
                          MT_CG_DISP_BLS_SW_CG,
                          MT_CG_DISP_WDMA_SW_CG,
                          MT_CG_DISP_RDMA_SW_CG,
                          MT_CG_DISP_OVL_SW_CG,
                          MT_CG_MDP_SHP_SW_CG,
                          MT_CG_MDP_WROT_SW_CG,
#ifdef EMULATION_EARLY_PORTING
                          MT_CG_MDP_WDMA_SW_CG,
                          MT_CG_MDP_RSZ1_SW_CG,
                          MT_CG_MDP_RSZ0_SW_CG,
                          MT_CG_MDP_RDMA_SW_CG,
                          MT_CG_MDP_BLS_26M_SW_CG,
                          MT_CG_MM_CAM_SW_CG,
#else
                          MT_CG_MDP_RSZ_SW_CG,
                          MT_CG_MDP_RDMA_SW_CG,
                          MT_CG_DISP_PWM_SW_CG,
                          MT_CG_DISP_PWM_26M_SW_CG,
                          MT_CG_MM_CAM_DP_SW_CG,
                          MT_CG_MM_CAM_DP2_SW_CG,
                          MT_CG_MM_CAM_SMI_SW_CG,
#endif
                          MT_CG_MM_SENINF_SW_CG,
                          MT_CG_MM_CAMTG_SW_CG,
                          MT_CG_MM_CODEC_SW_CG,
                          MT_CG_DISP_FAKE_ENG_SW_CG,
    END_CG_GRP(CG_MMSYS0, MT_CG_MUTEX_SLOW_CLOCK_SW_CG),

    // CG_MMSYS1
    BEG_CG_GRP(CG_MMSYS1, MT_CG_DSI_ENGINE_SW_CG),
                          MT_CG_DSI_DIGITAL_SW_CG,
                          MT_CG_DISP_DPI_ENGINE_SW_CG,
                          MT_CG_DISP_DPI_IF_SW_CG,
                          MT_CG_DISP_DBI_ENGINE_SW_CG,
#ifdef EMULATION_EARLY_PORTING
                          MT_CG_DISP_DBI_SMI_SW_CG,
#endif
    END_CG_GRP(CG_MMSYS1, MT_CG_DISP_DBI_IF_SW_CG),

    // CG_MFG
    BEG_END_CG_GRP(CG_MFG, MT_CG_MFG_PDN_BG3D_SW_CG),

    // CG_AUDIO
    BEG_CG_GRP(CG_AUDIO, MT_CG_AUD_PDN_AFE_EN),
                         MT_CG_AUD_PDN_I2S_EN,
                         MT_CG_AUD_PDN_ADC_EN,
                         MT_CG_AUD_PDN_DAC_EN,
                         MT_CG_AUD_PDN_DAC_PREDIS_EN,
    END_CG_GRP(CG_AUDIO, MT_CG_AUD_PDN_TML_EN),

    // virtual CG
    BEG_CG_GRP(CG_VIRTUAL, MT_VCG_BUS),
                           MT_VCG_APDMA_WLAN,
                           MT_VCG_APDMA_BTIF,
                           MT_VCG_APDMA_I2C,
                           MT_VCG_APDMA_UART,
                           MT_VCG_AUX_ADC,
                           MT_VCG_AUX_TP,
                           MT_VCG_AUX_THERM,
                           MT_VCG_MUTEX_SLOW_CLOCK_DISP,
    END_CG_GRP(CG_VIRTUAL, MT_VCG_MUTEX_SLOW_CLOCK_M4U),

    NR_CLKS,

    // for AUTO SEL (DDR2: rg_axibus_gfmux_sel[2] = 1'b1, DDR3: rg_axibus_gfmux_sel[2] = 1'b0)
    MT_CG_SRC_DBI,      // MT_CG_MPLL_D12 (for DDR2) or MT_CG_MPLL_D10 (for DDR3)
    MT_CG_SRC_SMI,      // MT_CG_MPLL_D6 (for DDR2) or MT_CG_MPLL_D5 (for DDR3)

    // Special
    MT_CG_CLOCK_OFF,
    MT_CG_INVALID,
} cg_clk_id;

#define MT_CG_UNIV_48M MT_CG_USB_48M // alias

//
// CLKMUX ID
//
typedef enum
{
    /* CLK_MUX_SEL0 */
    MT_CLKMUX_UART0_GFMUX_SEL,      // MT_CG_SYS_26M, MT_CG_UPLL_D24
    MT_CLKMUX_EMI2X_GFMUX_SEL,      // MT_CG_SYS_26M, MT_CG_MPLL_D3, MT_CG_MPLL_D4, MT_CG_MPLL_D2
    MT_CLKMUX_AXIBUS_GFMUX_SEL,     // MT_CG_SYS_26M, MT_CG_MPLL_D10, MT_CG_MPLL_D12
    MT_CLKMUX_MFG_MUX_SEL,          // MT_CG_UPLL_D3, MT_CG_GPU_491P52M_EN, MT_CG_GPU_500P5M_EN, MT_CG_MPLL_D3, MT_CG_UPLL_D2, MT_CG_SYS_26M, MT_CG_MPLL_D2 (MT_CLKMUX_MFG_GFMUX_SEL)
    MT_CLKMUX_MSDC0_MUX_SEL,        // MT_CG_MPLL_D12, MT_CG_MPLL_D10, MT_CG_MPLL_D8, MT_CG_UPLL_D7, MT_CG_MPLL_D7, MT_CG_MPLL_D8, MT_CG_SYS_26M, MT_CG_UPLL_D6
    MT_CLKMUX_SPINFI_MUX_SEL,       // MT_CG_SYS_26M, MT_CG_MPLL_D24, MT_CG_MPLL_D20, MT_CG_UPLL_D20, MT_CG_UPLL_D16, MT_CG_UPLL_D12, MT_CG_UPLL_D10, MT_CG_MPLL_D12, MT_CG_MPLL_D10 (MT_CLKMUX_SPINFI_GFMUX_SEL)
    MT_CLKMUX_CAM_MUX_SEL,          // MT_CG_UNIV_48M, MT_CG_UPLL_D6
    MT_CLKMUX_PWM_MM_MUX_SEL,       // MT_CG_SYS_26M, MT_CG_UPLL_D12
    MT_CLKMUX_UART1_GFMUX_SEL,      // MT_CG_SYS_26M, MT_CG_UPLL_D24
    MT_CLKMUX_MSDC1_MUX_SEL,        // MT_CG_MPLL_D12, MT_CG_MPLL_D10, MT_CG_MPLL_D8, MT_CG_UPLL_D7, MT_CG_MPLL_D7, MT_CG_MPLL_D8, MT_CG_SYS_26M, MT_CG_UPLL_D6
    MT_CLKMUX_SPM_52M_CK_SEL,       // MT_CG_SYS_26M, MT_CG_UPLL_D24
    MT_CLKMUX_PMICSPI_MUX_SEL,      // MT_CG_MPLL_D24, MT_CG_UNIV_48M, MT_CG_UPLL_D16, MT_CG_SYS_26M
    // MT_CLKMUX_AUD_HF_26M_SEL,       // MT_CG_SYS_26M
    MT_CLKMUX_AUD_INTBUS_SEL,       // MT_CG_SYS_26M, MT_CG_MPLL_D24, MT_CG_MPLL_D12

    /* CLK_MUX_SEL1 */
    MT_CLKMUX_NFI2X_MUX_SEL,
    MT_CLKMUX_NFI_MUX_SEL,

    NR_CLKMUXS,

    // Special
    MT_CLKMUX_INVALID,
} clkmux_id;

//
// PLL ID
//
typedef enum
{
    ARMPLL,
    MAINPLL,
    UNIVPLL,
    WHPLL,

    NR_PLLS,
} pll_id;

//
// SUBSYS ID
//
typedef enum
{
    SYS_MD1,
    SYS_CON,
    SYS_DIS,
    SYS_MFG,

    NR_SYSS,
} subsys_id;

//
// LARB ID
//
typedef enum
{
    MT_LARB0,

    NR_LARBS,
} larb_id;

/* larb monitor mechanism definition*/
enum
{
    LARB_MONITOR_LEVEL_HIGH     = 10,
    LARB_MONITOR_LEVEL_MEDIUM   = 20,
    LARB_MONITOR_LEVEL_LOW      = 30,
};

struct larb_monitor
{
    struct list_head link;
    int level;
    void (*backup)(struct larb_monitor *h, int larb_idx);       /* called before disable larb clock */
    void (*restore)(struct larb_monitor *h, int larb_idx);      /* called after enable larb clock */
};


/*=============================================================*/
// Global variable definition
/*=============================================================*/


/*=============================================================*/
// Global function definition
/*=============================================================*/

EXTERN int enable_clock(enum cg_clk_id id, char *name);
EXTERN int disable_clock(enum cg_clk_id id, char *name);
EXTERN int mt_enable_clock(cg_clk_id id, const char *name);
EXTERN int mt_disable_clock(cg_clk_id id, const char *name);
EXTERN int clock_is_on(cg_clk_id id);
EXTERN void dump_clk_info_by_id(cg_clk_id id);

EXTERN int clkmux_sel(clkmux_id id, cg_clk_id clksrc, const char *name);
EXTERN int clkmux_get(clkmux_id id, const char *name);

EXTERN int enable_pll(pll_id id, const char *name);
EXTERN int disable_pll(pll_id id, const char *name);
EXTERN int pll_fsel(pll_id id, unsigned int value);
EXTERN int pll_is_on(pll_id id);

EXTERN int enable_subsys(subsys_id id, const char *name);
EXTERN int disable_subsys(subsys_id id, const char *name);
EXTERN int disable_subsys_force(subsys_id id, const char *name);
EXTERN void register_larb_monitor(struct larb_monitor *handler);
EXTERN void unregister_larb_monitor(struct larb_monitor *handler);
EXTERN int md_power_on(subsys_id id);
EXTERN int md_power_off(subsys_id id, unsigned int timeout);
EXTERN int conn_power_on(subsys_id id);
EXTERN int conn_power_off(subsys_id id, unsigned int timeout);
EXTERN int subsys_is_on(subsys_id id);

EXTERN const char *grp_get_name(cg_grp_id id);
EXTERN int grp_dump_regs(cg_grp_id id, unsigned int *ptr);
EXTERN const char *pll_get_name(pll_id id);
EXTERN int pll_dump_regs(pll_id id, unsigned int *ptr);
EXTERN const char *subsys_get_name(subsys_id id);
EXTERN int subsys_dump_regs(subsys_id id, unsigned int *ptr);

EXTERN unsigned int mt_get_emi_freq(void);
EXTERN unsigned int mt_get_bus_freq(void);
EXTERN unsigned int mt_get_cpu_freq(void);

EXTERN int snapshot_golden_setting(const char *func, const unsigned int line);

EXTERN bool is_ddr3(void);

EXTERN bool clkmgr_idle_can_enter(unsigned int *condition_mask, unsigned int *block_mask);

EXTERN cg_grp_id clk_id_to_grp_id(cg_clk_id id);
EXTERN unsigned int clk_id_to_mask(cg_clk_id id);

EXTERN int clkmgr_is_locked(void);

EXTERN int mt_clkmgr_init(void);

EXTERN int spm_isp_sram_power(int on);
EXTERN int mtcmos_is_force_on(subsys_id id);
EXTERN int mtcmos_clr_force_on(subsys_id id);
EXTERN int mtcmos_set_force_on(subsys_id id);


#undef EXTERN

#ifdef __cplusplus
// } // TODO: disable temp
#endif

#endif // __MT_CLKMGR_H__
