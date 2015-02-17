#ifndef __LCD_REG_H__
#define __LCD_REG_H__

#include <stddef.h>
#include "disp_drv_platform.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    unsigned RUN            : 1;
    unsigned rsv_1          : 1;
    unsigned WAIT_HTT       : 1;
    unsigned WAIT_SYNC      : 1;
    unsigned BUSY           : 1;
    unsigned rsv_5          : 27;
} LCD_REG_STATUS, *PLCD_REG_STATUS;

typedef struct
{
    unsigned COMPLETED      : 1;
    unsigned rsv_1          : 1;
    unsigned HTT            : 1;
    unsigned SYNC           : 1;
    unsigned TE             : 1;
    unsigned rsv_5          : 27;
} LCD_REG_INTERRUPT, *PLCD_REG_INTERRUPT;

typedef struct
{
    unsigned RESET          : 1;
    unsigned rsv_1          : 14;
    unsigned START          : 1;
    unsigned rsv_16         : 16;
} LCD_REG_START, *PLCD_REG_START;

typedef struct 
{
    unsigned WR_2ND         : 4;
    unsigned WR_1ST         : 4;
    unsigned RD_2ND         : 4;
    unsigned RD_1ST         : 4;
    unsigned CSH            : 4;
    unsigned CSS            : 4;
    unsigned rsv_24         : 8;
} LCD_REG_SIF_TIMING, *PLCD_REG_SIF_TIMING;

typedef struct
{
    unsigned SIZE_0         : 3;
    unsigned THREE_WIRE_0   : 1;
    unsigned SDI_0          : 1;
    unsigned FIRST_POL_0    : 1;
    unsigned SCK_DEF_0      : 1;
    unsigned DIV2_0         : 1;
    unsigned SIZE_1         : 3;
    unsigned THREE_WIRE_1   : 1;
    unsigned SDI_1          : 1;
    unsigned FIRST_POL_1    : 1;
    unsigned SCK_DEF_1      : 1;
    unsigned DIV2_1         : 1;
    unsigned rsv_16         : 8;
    unsigned HW_CS          : 1;
    unsigned rsv_25         : 7;
} LCD_REG_SCNF, *PLCD_REG_SCNF;

typedef struct
{
    unsigned WST            : 6;
    unsigned rsv_6          : 2;
    unsigned C2WS           : 4;
    unsigned C2WH           : 4;
    unsigned RLT            : 6;
    unsigned rsv_22         : 2;
    unsigned C2RS           : 4;
    unsigned C2RH           : 4;
} LCD_REG_PCNF, *PLCD_REG_PCNF;

typedef struct
{
    unsigned PCNF0_DW       : 3;
    unsigned rsv_3          : 1;
    unsigned PCNF1_DW       : 3;
    unsigned rsv_7          : 1;
    unsigned PCNF2_DW       : 3;
    unsigned rsv_11         : 5;
    unsigned PCNF0_CHW      : 4;
    unsigned PCNF1_CHW      : 4;
    unsigned PCNF2_CHW      : 4;
    unsigned rsv_28         : 4;
} LCD_REG_PCNFDW, *PLCD_REG_PCNFDW;

typedef struct
{
    unsigned ENABLE         : 1;
    unsigned EDGE_SEL       : 1;
    unsigned MODE           : 1;
    unsigned rsv_3          : 12;
    unsigned SW_TE          : 1;
    unsigned rsv_16         : 16;
} LCD_REG_TECON, *PLCD_REG_TECON;

typedef struct
{
    unsigned WIDTH          : 11;
    unsigned rsv_11         : 5;
    unsigned HEIGHT         : 11;
    unsigned rsv_27         : 5;
} LCD_REG_SIZE, *PLCD_REG_SIZE;

typedef struct
{
    unsigned rsv_0          : 4;
    unsigned addr           : 4;
    unsigned rsv_8          : 24;
} LCD_REG_CMD_ADDR, *PLCD_REG_CMD_ADDR;

typedef struct
{
    unsigned rsv_0          : 4;
    unsigned addr           : 4;
    unsigned rsv_8          : 24;
} LCD_REG_DAT_ADDR, *PLCD_REG_DAT_ADDR;


typedef struct
{
    unsigned RGB_ORDER      : 1;
    unsigned BYTE_ORDER     : 1;
    unsigned PADDING        : 1;
    unsigned DATA_FMT       : 3;
    unsigned IF_FMT         : 2;
    unsigned rsv_8          : 16;
    unsigned RSPX           : 1;
    unsigned IF_24          : 1;
    unsigned rsv_6          : 6;
}LCD_REG_WROI_CON, *PLCD_REG_WROI_CON;

typedef struct
{
    unsigned CS0            : 1;
    unsigned CS1            : 1;
    unsigned rsv30          : 30;
} LCD_REG_SIF_CS, *PLCD_REG_SIF_CS;

typedef struct 
{
    unsigned TIME_OUT       : 12;
    unsigned rsv_12         : 4;
    unsigned COUNT          : 12;
    unsigned rsv_28         : 4;
} LCD_REG_CALC_HTT, *PLCD_REG_CALC_HTT;

typedef struct 
{
    unsigned HTT            : 10;
    unsigned rsv_10         : 6;
    unsigned VTT            : 12;
    unsigned rsv_28         : 4;
} LCD_REG_SYNC_LCM_SIZE, *PLCD_REG_SYNC_LCM_SIZE;


typedef struct 
{
    unsigned WAITLINE       : 12;
    unsigned rsv_12         : 4;
    unsigned SCANLINE       : 12;
    unsigned rsv_28         : 4;
} LCD_REG_SYNC_CNT, *PLCD_REG_SYNC_CNT;

typedef struct
{
    unsigned DBI_ULTRA      : 1;
    unsigned rsv_1          : 31;
}LCD_REG_ULTRA_CON;

typedef struct 
{
    unsigned S_CHKSUM       : 24;
    unsigned rsv_24         : 7;
    unsigned CHKSUM_EN      : 1;
} LCD_SERIAL_CHKSUM, *PLCD_SERIAL_CHKSUM;

typedef struct 
{
    unsigned P_CHKSUM       : 24;
    unsigned rsv_24         : 8;
} LCD_PARALLEL_CHKSUM, *PLCD_PARALLEL_CHKSUM;

typedef struct
{
    unsigned DBI_TH_LOW     : 16;
    unsigned DBI_TH_HIGH    : 16;
}LCD_REG_DBI_ULTRA_TH;

typedef struct
{
    LCD_REG_STATUS          STATUS;             // 0000
    LCD_REG_INTERRUPT       INT_ENABLE;         // 0004
    LCD_REG_INTERRUPT       INT_STATUS;         // 0008
    LCD_REG_START           START;              // 000C
    UINT32                  RESET;              // 0010
    UINT32                  rsv_0014[2];        // 0014..0018
    LCD_REG_SIF_TIMING      SIF_TIMING[2];      // 001C..0020
    UINT32                  rsv_0024;           // 0024
    LCD_REG_SCNF            SERIAL_CFG;         // 0028
    LCD_REG_SIF_CS          SIF_CS;             // 002C
    LCD_REG_PCNF            PARALLEL_CFG[3];    // 0030..0038
    LCD_REG_PCNFDW          PARALLEL_DW;        // 003C
    LCD_REG_TECON           TEARING_CFG;        // 0040
    LCD_REG_CALC_HTT        CALC_HTT;           // 0044
    LCD_REG_SYNC_LCM_SIZE   SYNC_LCM_SIZE;      // 0048
    LCD_REG_SYNC_CNT        SYNC_CNT;           // 004C
    UINT32                  rsv_0050[4];        // 0050..005C
    LCD_REG_WROI_CON        WROI_CONTROL;       // 0060
    LCD_REG_CMD_ADDR        WROI_CMD_ADDR;      // 0064
    LCD_REG_DAT_ADDR        WROI_DATA_ADDR;     // 0068
    LCD_REG_SIZE            WROI_SIZE;          // 006C
    UINT32                  rsv_0070[8];        // 0070..008C
    LCD_REG_ULTRA_CON       ULTRA_CON;          // 0090
    UINT32                  CONSUME_RATE;       // 0094
    LCD_REG_DBI_ULTRA_TH    DBI_ULTRA_TH;       // 0098
    UINT32                  rsv_009C[3];        // 009C..00A4
    UINT32                  DB_LCM;             // 00A8
    UINT32                  rsv_00AC[13];       // 00AC..00DC
    UINT32                  SERIAL_CHKSUM;      // 00E0
    UINT32                  PARALLEL_CHKSUM;    // 00E4
    UINT32                  PATTERN;            // 00E8
    UINT32                  rsv_00EC[837];      // 00EC..0DFC
    UINT32                  DITHER_0;           // 0E00
    UINT32                  rsv_0E04[4];        // 0E04..0E10
    UINT32                  DITHER_5;           // 0E14
    UINT32                  DITHER_6;           // 0E18
    UINT32                  DITHER_7;           // 0E1C
    UINT32                  DITHER_8;           // 0E20
    UINT32                  DITHER_9;           // 0E24
    UINT32                  DITHER_10;          // 0E28
    UINT32                  DITHER_11;          // 0E2C
    UINT32                  DITHER_12;          // 0E30
    UINT32                  DITHER_13;          // 0E34
    UINT32                  DITHER_14;          // 0E38
    UINT32                  DITHER_15;          // 0E3C
    UINT32                  DITHER_16;          // 0E40
    UINT32                  DITHER_17;          // 0E44
    UINT32                  rsv_0E48[46];       // 0E48..0EFC
    UINT32                  PCMD0;              // 0F00
    UINT32                  rsv_0F04[3];        // 0F04..0F0C
    UINT32                  PDAT0;              // 0F10
    UINT32                  rsv_0F14[3];        // 0F14..0F1C
    UINT32                  PCMD1;              // 0F20
    UINT32                  rsv_0F24[3];        // 0F24..0F2C
    UINT32                  PDAT1;              // 0F30
    UINT32                  rsv_0F34[3];        // 0F34..0F3C
    UINT32                  PCMD2;              // 0F40
    UINT32                  rsv_0F44[3];        // 0F44..0F4C
    UINT32                  PDAT2;              // 0F50
    UINT32                  rsv_0F54[11];       // 0F54..0F7C
    UINT32                  SCMD0;              // 0F80
    UINT32                  rsv_0F84[3];        // 0F84..0F8C
    UINT32                  SDAT0;              // 0F90
    UINT32                  rsv_0F94[3];        // 0F94..0F9C
    UINT32                  SCMD1;              // 0FA0
    UINT32                  rsv_0FA4[3];        // 0FA4..0FAC
    UINT32                  SDAT1;              // 0FB0
    UINT32                  rsv_0FB4[3];        // 0FB4..0FBC
} volatile LCD_REGS, *PLCD_REGS;

STATIC_ASSERT(0x0000 == offsetof(LCD_REGS, STATUS));
STATIC_ASSERT(0x0004 == offsetof(LCD_REGS, INT_ENABLE));
STATIC_ASSERT(0x0028 == offsetof(LCD_REGS, SERIAL_CFG));
STATIC_ASSERT(0x0030 == offsetof(LCD_REGS, PARALLEL_CFG));
STATIC_ASSERT(0x0040 == offsetof(LCD_REGS, TEARING_CFG));
STATIC_ASSERT(0x0F00 == offsetof(LCD_REGS, PCMD0));
STATIC_ASSERT(0x0F80 == offsetof(LCD_REGS, SCMD0));
STATIC_ASSERT(0x0FC0 == sizeof(LCD_REGS));

#define LCD_A0_LOW_OFFSET  (0x0)
#define LCD_A0_HIGH_OFFSET (0x10)

#ifdef __cplusplus
}
#endif

#endif // __LCD_REG_H__
