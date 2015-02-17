/*****************************************************************************
 *
 * Filename:
 * ---------
 *    tp_hw.h
 *
 * Project:
 * --------
 *   Maui_Software
 *
 * Description:
 * ------------
 *   This file is intends for GPT driver.
 *
 * Author:
 * -------
 *  Simon Shih
 *
 *============================================================================
 *             HISTORY
 * Below this line, this part is controlled by PVCS VM. DO NOT MODIFY!!
 *------------------------------------------------------------------------------
 *------------------------------------------------------------------------------
 * Upper this line, this part is controlled by PVCS VM. DO NOT MODIFY!!
 *============================================================================
 ****************************************************************************/
#ifndef TP_HW_H
#define TP_HW_H

#define AUXADC_TS_POSITION_YP 1
#define AUXADC_TS_POSITION_XP 2
#define AUXADC_TS_POSITION_Z1 3
#define AUXADC_TS_POSITION_Z2 4
#define AUXADC_TS_POSITION_XN 5
#define AUXADC_TS_POSITION_YN 6
#define AUXADC_TS_POSITION_B  7
#define AUXADC_TS_POSITION_F  8

#define AUXADC_CALI_XP		1//1215/1000	
#define AUXADC_CALI_XN		1//12412/1000
#define AUXADC_CALI_YP		1//1166/1000
#define AUXADC_CALI_YN		1//1162/1000


#if 0
#if defined(DRV_ADC_TOUCH_SCREEN)
#if !defined(DRV_TS_OFF)

#if defined(DRV_ADC_OFF)
#error "ADC module should exist!! "
#endif
#include "reg_base.h"
 #ifdef DRV_ADC_TOUCH_SCREEN_OFFSET_0X50
#ifdef DRV_TP_AUXADC_MOVE_TO_ADIE_DEBUG
#define AUX_TS_CMD					(AUXADC_base+0x00d4)
#define AUX_TS_CON					(AUXADC_base+0x0058)
#define AUX_TS_DATA0				(AUXADC_base+0x00d8)
#else
 //  #define AUX_TS_DEBT                   (AUXADC_base+0x0050)  //lxg delete
   #define AUX_TS_CMD                    (AUXADC_base+0x0054)
   #define AUX_TS_CON                    (AUXADC_base+0x0058)
   #define AUX_TS_DATA0                  (AUXADC_base+0x005c)
#endif
 #else // #ifdef DRV_ADC_TOUCH_SCREEN_OFFSET_0X50
   #define AUX_TS_DEBT                   (AUXADC_base+0x0030)
   #define AUX_TS_CMD                    (AUXADC_base+0x0034)
   #define AUX_TS_CON                    (AUXADC_base+0x0038)
   #define AUX_TS_DATA0                  (AUXADC_base+0x003c)
 #endif // #ifdef DRV_ADC_TOUCH_SCREEN_OFFSET_0X50

#ifndef DRV_TP_ADIE_DDIE_STRCUTURE
    #define TS_CON_STATUS_MASK  0x0002
    #define TS_DEBT_MASK		0x3fff
#else
    #define TS_CON_STATUS_MASK 	0x4000
    #define TS_DEBT_MASK		0x3fff
#endif 
 
#ifndef DRV_TP_ADIE_DDIE_STRCUTURE
    #define AUX_TS_DEBT               (AUXADC_base+0x0050)
#else

#ifdef DRV_TP_AUXADC_MOVE_TO_ADIE
	#define AUX_TS_DEBT_STATUS		  (PMU_SD_base+0x0098)
#else
    #define AUX_TS_DEBT_STATUS        (ABBSYS_base+0x0098)
#endif

#endif 

#ifdef DRV_TP_ADIE_DDIE_STRCUTURE
#ifdef DRV_TP_AUXADC_MOVE_TO_ADIE
#if 0
#define TS_D2D_D_APC_AUX_CON1               (D2D_D_base + 0x04)
#define TS_D2D_A_APC_AUD_CON1               (D2D_A_base + 0x04)

#define TS_D2D_D_F26M_AUX_EN                0x0001
#define TS_D2D_D_AUX_EN 0x8000
#define TS_D2D_A_AUX_EN 0x8000
#endif
#else
#define TS_ABBA_WR_PATH0            (ABBSYS_base + 0x0)
#define TS_ABB_RSV_CON1				(ABB_D_base + 0x0004)
#define TS_ABB_WR_PATH0				(ABB_D_base + 0x0010)
#define TS_ABB_DD_WR_PATH0    		(ABB_D_base + 0x0020)
#define TS_ABB_AUX_CON0				(ABB_D_base + 0x0028)

#define TS_ABBA_AUX_PWDB                 0x0100
#define TS_F26M_CLK_EN                 	 0x8000
#define TS_AUX_PWDB                      0x0100
#define TS_AUX_FIFO_CLK_EN			     0x8000
#define TS_AUX_FIFO_EN				     0x0080
#define TS_FSM_CTRL_3              0x80
#define TS_26M_CLK_CTRL_3          0x8
#define TS_AAF_EN                        0x8000
#endif 
#endif

#endif   /*#if !defined(DRV_TS_OFF)*/

//#define TS_DEBT_MASK           0x3fff //lxg delete

#define TS_CMD_PD_MASK         0x0003
   #define TS_CMD_PD_YDRV_SH     0x0000
   #define TS_CMD_PD_IRQ_SH      0x0001
   #define TS_CMD_PD_IRQ         0x0003
#define TS_CMD_SE_DF_MASK      0x0004
   #define TS_CMD_DIFFERENTIAL   0x0000
   #define TS_CMD_SINGLE_END     0x0004
#define TS_CMD_MODE_MASK       0x0008
   #define TS_CMD_MODE_10BIT     0x0000
   #define TS_CMD_MODE_8BIT      0x0008
#define TS_CMD_ADDR_MASK       0x0070
   #define TS_CMD_ADDR_Y         0x0010
   #define TS_CMD_ADDR_YN        0x0020
   #define TS_CMD_ADDR_Z1        0x0030
   #define TS_CMD_ADDR_Z2        0x0040
   #define TS_CMD_ADDR_X         0x0050
   #define TS_CMD_ADDR_XN        0x0060
#if (defined(DRV_TP_AUXADC_MOVE_TO_ADIE)) && (!defined(DRV_TP_AUXADC_MOVE_TO_ADIE_DEBUG))
#define TS_CON_SPL_TRIGGER     0x8000
#define TS_CON_MAGIC_NUM       0x5500
#define TS_CON_SPL_MASK        0x8000 
#define TS_DAT0_DAT_MASK       0x03ff
#else
#define TS_CON_SPL_MASK        0x0001
#if defined(DRV_TP_PENUP_FIXED)
   #define TS_CON_SPL_TRIGGER    0x8001
 #else
   #define TS_CON_SPL_TRIGGER    0x0001
#endif
//#define TS_CON_STATUS_MASK     0x0002 //lxg delete
#define TS_DAT0_DAT_MASK       0x03ff
#endif 

#if defined(DRV_TP_SPL_NUM_ABB_1708)
#define AUX_SPL_NUM (PLL_base+0x1708)
#define AUX_SPL_NUM_SHIFT 8
#define AUX_SPL_NUM_MASK 0xFF00

#elif defined(DRV_TP_SPL_NUM_ABB_8708)
#define AUX_SPL_NUM (ABBSYS_base+0x8708)
#define AUX_SPL_NUM_SHIFT 8
#define AUX_SPL_NUM_MASK 0xFF00

#elif defined(DRV_TP_SPL_NUM_ABB_0094)
#define AUX_SPL_NUM (ABBSYS_SD_base+0x94)
#define AUX_SPL_NUM_SHIFT 0
#define AUX_SPL_NUM_MASK 0x00FF

#elif defined(DRV_TP_SPL_NUM_PMU_0094)
#define AUX_SPL_NUM (PMU_SD_base+0x94)
#define AUX_SPL_NUM_SHIFT 0
#define AUX_SPL_NUM_MASK 0x00FF

#else
#define AUX_SPL_NUM 0
#define AUX_SPL_NUM_SHIFT 0
#define AUX_SPL_NUM_MASK 0
#endif//#if defined(DRV_TP_SPL_NUM_ABB_1708)

#if !defined(DRV_TS_OFF)

#if defined(__DRV_COMM_REG_DBG__) && defined(__DRV_TP_REG_DBG__)
#define DRV_TP_WriteReg(addr,data)              DRV_DBG_WriteReg(addr,data)
#define DRV_TP_Reg(addr)                        DRV_DBG_Reg(addr)                      
#define DRV_TP_WriteReg32(addr,data)            DRV_DBG_WriteReg32(addr,data)          
#define DRV_TP_Reg32(addr)                      DRV_DBG_Reg32(addr)                    
#define DRV_TP_WriteReg8(addr,data)             DRV_DBG_WriteReg8(addr,data)           
#define DRV_TP_Reg8(addr)                       DRV_DBG_Reg8(addr)                     
#define DRV_TP_ClearBits(addr,data)             DRV_DBG_ClearBits(addr,data)           
#define DRV_TP_SetBits(addr,data)               DRV_DBG_SetBits(addr,data)             
#define DRV_TP_SetData(addr, bitmask, value)    DRV_DBG_SetData(addr, bitmask, value)  
#define DRV_TP_ClearBits32(addr,data)           DRV_DBG_ClearBits32(addr,data)         
#define DRV_TP_SetBits32(addr,data)             DRV_DBG_SetBits32(addr,data)           
#define DRV_TP_SetData32(addr, bitmask, value)  DRV_DBG_SetData32(addr, bitmask, value)
#define DRV_TP_ClearBits8(addr,data)            DRV_DBG_ClearBits8(addr,data)          
#define DRV_TP_SetBits8(addr,data)              DRV_DBG_SetBits8(addr,data)            
#define DRV_TP_SetData8(addr, bitmask, value)   DRV_DBG_SetData8(addr, bitmask, value) 
#else
#define DRV_TP_WriteReg(addr,data)              DRV_WriteReg(addr,data)
#define DRV_TP_Reg(addr)                        DRV_Reg(addr)                      
#define DRV_TP_WriteReg32(addr,data)            DRV_WriteReg32(addr,data)          
#define DRV_TP_Reg32(addr)                      DRV_Reg32(addr)                    
#define DRV_TP_WriteReg8(addr,data)             DRV_WriteReg8(addr,data)           
#define DRV_TP_Reg8(addr)                       DRV_Reg8(addr)                     
#define DRV_TP_ClearBits(addr,data)             DRV_ClearBits(addr,data)           
#define DRV_TP_SetBits(addr,data)               DRV_SetBits(addr,data)             
#define DRV_TP_SetData(addr, bitmask, value)    DRV_SetData(addr, bitmask, value)  
#define DRV_TP_ClearBits32(addr,data)           DRV_ClearBits32(addr,data)         
#define DRV_TP_SetBits32(addr,data)             DRV_SetBits32(addr,data)           
#define DRV_TP_SetData32(addr, bitmask, value)  DRV_SetData32(addr, bitmask, value)
#define DRV_TP_ClearBits8(addr,data)            DRV_ClearBits8(addr,data)          
#define DRV_TP_SetBits8(addr,data)              DRV_SetBits8(addr,data)            
#define DRV_TP_SetData8(addr, bitmask, value)   DRV_SetData8(addr, bitmask, value) 
#endif //#if defined(__DRV_COMM_REG_DBG__) && defined(__DRV_TP_REG_DBG__)

#else //!defined(DRV_TS_OFF)

#define DRV_TP_WriteReg(addr,data)            
#define DRV_TP_Reg(addr)                        drv_dummy_return()
#define DRV_TP_WriteReg32(addr,data)          
#define DRV_TP_Reg32(addr)                      drv_dummy_return()
#define DRV_TP_WriteReg8(addr,data)           
#define DRV_TP_Reg8(addr)                       drv_dummy_return()
#define DRV_TP_ClearBits(addr,data)           
#define DRV_TP_SetBits(addr,data)             
#define DRV_TP_SetData(addr, bitmask, value)  
#define DRV_TP_ClearBits32(addr,data)         
#define DRV_TP_SetBits32(addr,data)           
#define DRV_TP_SetData32(addr, bitmask, value)
#define DRV_TP_ClearBits8(addr,data)          
#define DRV_TP_SetBits8(addr,data)            
#define DRV_TP_SetData8(addr, bitmask, value) 

#endif //!defined(DRV_TS_OFF)

#endif /*defined(DRV_ADC_TOUCH_SCREEN)*/

#endif
#endif   /*TP_HW_H*/

