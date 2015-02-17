
#ifndef __MT_SPM_MTCMOS_H__
#define __MT_SPM_MTCMOS_H__

//
// Register Base Address
//

//
// INFRACFG_AO
//

#define INFRACFG_AO_BASE            INFRA_SYS_CFG_AO_BASE

#define TOPAXI_SI0_CTL              (INFRACFG_AO_BASE + 0x0200) // TODO: review it
#define INFRA_TOPAXI_PROTECTEN      (INFRACFG_AO_BASE + 0x0220)
#define INFRA_TOPAXI_PROTECTSTA1    (INFRACFG_AO_BASE + 0x0228)


#define PWR_RST_B_BIT               BIT(0)          // @ SPM_XXX_PWR_CON
#define PWR_ISO_BIT                 BIT(1)          // @ SPM_XXX_PWR_CON
#define PWR_ON_BIT                  BIT(2)          // @ SPM_XXX_PWR_CON
#define PWR_ON_S_BIT                BIT(3)          // @ SPM_XXX_PWR_CON
#define PWR_CLK_DIS_BIT             BIT(4)          // @ SPM_XXX_PWR_CON
#define SRAM_CKISO_BIT              BIT(5)          // @ SPM_FC0_PWR_CON or SPM_CPU_PWR_CON
#define SRAM_ISOINT_B_BIT           BIT(6)          // @ SPM_FC0_PWR_CON or SPM_CPU_PWR_CON
#define SRAM_PDN_BITS               BITMASK(11:8)   // @ SPM_XXX_PWR_CON
#define SRAM_PDN_ACK_BITS           BITMASK(15:12)  // @ SPM_XXX_PWR_CON

#define DIS_PWR_STA_MASK            BIT(3)

#endif

