#ifndef __MT_AHB_ABT_H
#define __MT_AHB_ABT_H

#include "mt_reg_base.h"

/*
 * Define hardware registers.
 */
#define AHBABT_CON          (AHBABT_BASE + 0x000)
#define AHBABT_CON_SET      (AHBABT_BASE + 0x004)
#define AHBABT_CON_CLR      (AHBABT_BASE + 0x008)
#define AHBABT_UNDEF        (AHBABT_BASE + 0x00C)
#define AHBABT_THRES        (AHBABT_BASE + 0x010)
#define AHBABT_MON_CLR      (AHBABT_BASE + 0x014)
#define AHBABT_ABOT_CLR     (AHBABT_BASE + 0x018)
#define AHBABT_ADDR1        (AHBABT_BASE + 0x020)
#define AHBABT_ADDR2        (AHBABT_BASE + 0x024)
#define AHBABT_ADDR3        (AHBABT_BASE + 0x028)
#define AHBABT_ADDR4        (AHBABT_BASE + 0x02C)
#define AHBABT_RDY_CNT1     (AHBABT_BASE + 0x030)
#define AHBABT_RDY_CNT2     (AHBABT_BASE + 0x034)
#define AHBABT_RDY_CNT3     (AHBABT_BASE + 0x038)
#define AHBABT_RDY_CNT4     (AHBABT_BASE + 0x03C)

/*
 * Define constants.
 */
#define AHBABT_CNT      (4)

/*
 * Declare functions.
 */
extern int mt_ahb_abt_dump(char *buf);

#endif

