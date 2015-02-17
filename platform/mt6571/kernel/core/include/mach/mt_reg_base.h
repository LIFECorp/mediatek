#ifndef __MT_REG_BASE__
#define __MT_REG_BASE__
/* MM sub-system */
#define MMSYS_CONFIG_BASE            0xF4000000
#define MDP_RDMA_BASE                0xF4001000
#define MDP_RSZ_BASE                 0xF4002000
#define MDP_SHP_BASE                 0xF4003000
#define MDP_WROT_BASE                0xF4004000
#define DISP_OVL_BASE                0xF4005000
#define DISP_WDMA_BASE               0xF4006000
#define DISP_RDMA_BASE               0xF4007000
#define DISP_PQ_BASE                 0xF4008000
#define DISP_BLS_BASE                0xF4009000
#define DISP_DSI_BASE                0xF400A000
#define DISP_DPI_BASE                0xF400B000
#define DISP_DBI_BASE                0xF400C000
#define DISP_PWM_BASE                0xF400D000
#define MM_MUTEX_BASE                0xF400E000
#define MM_CMDQ_BASE                 0xF400F000
#define SMI_LARB0_BASE               0xF4010000
#define SMI_COMMON_BASE              0xF4011000
#define SENINF_BASE                  0xF4012000
#define MIPI_RX_CONFIG_BASE          0xF4013000
#define CAM_BASE                     0xF4014000
#define VENC_BASE                    0xF4016000
#define VDEC_BASE                    0xF4017000

/* G3D */
#define G3D_CONFIG_BASE              0xF3000000
#define MALI_BASE                    0xF3010000

/* perisys */
#define AP_DMA_BASE                  0xF1000000
#define NFI_BASE                     0xF1001000
#define NFIECC_BASE                  0xF1002000
#define AUXADC_BASE                  0xF1003000
#define FHCTL_BASE                   0xF1004000
#define UART1_BASE                   0xF1005000
#define UART2_BASE                   0xF1006000
#define PWM_BASE                     0xF1008000
#define I2C0_BASE                    0xF1009000
#define I2C1_BASE                    0xF100A000
#define SPI_BASE                     0xF100C000
#define THERMAL_BASE                 0xF100D000
#define BTIF_BASE                    0xF100E000
#define USB_BASE                     0xF1100000
#define USB_SIF_BASE                 0xF1110000
#define MSDC_0_BASE                  0xF1120000
#define MSDC_1_BASE                  0xF1130000
#ifdef FPGA_EARLY_PORTING
#define AUDIO_BASE                   0xF6000000
#else
#define AUDIO_BASE                   0xF1140000
#endif //FPGA_EARLY_PORTING
#define AHBABT_BASE                  0xF1150000

/* infrasys AO */
#define TOPCKGEN_BASE                0xF0000000
#define INFRA_SYS_CFG_AO_BASE        0xF0001000
#define SRAMROM_BASE                 0xF0001400
#define KP_BASE                      0xF0002000
#define PERICFG_BASE                 0xF0003000
#define EMI_BASE                     0xF0004000
#define GPIO_BASE                    0xF0005000
#define SPM_BASE                     0xF0006000
#define AP_RGU_BASE                  0xF0007000
#define TOPRGU_BASE                  AP_RGU_BASE
#define APMCU_GPTIMER_BASE           0xF0008000
#define HACC_BASE                    0xF000A000
#define EINT_BASE                    0xF000B000
#define AP_CCIF_BASE                 0xF000C000
#define SMI_BASE                     0xF000E000
#define PMIC_WRAP_BASE               0xF000F000
#define DEVICE_APC_AO_BASE           0xF0010000
#define MIPI_TX_CONFIG_BASE          0xF0011000
#define INFRA_TOP_MBIST_CTRL_BASE    0xF0012000

/* infrasys */
#define APARM_BASE                   0xF0170000
#define MCUSYS_CFGREG_BASE           0xF0200000
#define INFRA_SYS_CFG_BASE           0xF0201000
#define SYS_CIRQ_BASE                0xF0202000
#define M4U_CFG_BASE                 0xF0203000
#define DEVICE_APC_BASE              0xF0204000
#define IO_CFG_TOP_BASE              0xF0014000
#define IO_CFG_BOTTOM_BASE           0xF0015000
#define IO_CFG_LEFT_BASE             0xF0016000
#define IO_CFG_RIGHT_BASE            0xF0017000
#define APMIXED_BASE                 0xF0018000
#define GIC_BASE                     0xF0210000

/* CONNSYS */
#define CONN_BT_PKV_BASE             0xF8000000
#define CONN_BT_TIMCON_BASE          0xF8010000
#define CONN_BT_RF_CONTROL_BASE      0xF8020000
#define CONN_BT_MODEM_BASE           0xF8030000
#define CONN_BT_CONFIG_BASE          0xF8040000
#define CONN_MCU_CONFIG_BASE         0xF8070000
#define CONN_SYSRAM_BANK2_BASE       0xF8080000
#define CONN_SYSRAM_BANK3_BASE       0xF8090000
#define CONN_SYSRAM_BANK4_BASE       0xF80A0000
#define CONN_TOP_CR_BASE             0xF80B0000
#define CONN_HIF_BASE                0xF80F0000

/* Ram Console */
#define RAM_CONSOLE_BASE             0xF2000000

/* Device Info */
#define DEVINFO_BASE                 0xF5000000

#endif
