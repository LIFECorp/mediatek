#include <stdio.h>  // for printf ...
#include <string.h> // for memcpy ...
#include <malloc.h> // for malloc ...
#include <config.h>
#include <platform/mt_typedefs.h>
#include <platform/mtk_nand.h>
#include <platform/mtk_snand_lk.h>
#include <mt_partition.h>
#include <platform/bmt.h>
#include "partition_define.h"
#include "cust_nand.h"
#include <arch/ops.h>
#include "snand_device_list.h"
#include <kernel/event.h>
#include <platform/mt_irq.h>
#include <mt_gpt.h>
//#include <defines.h>    // for CACHE_LINE

//#define NAND_LK_TEST
#ifdef NAND_LK_TEST
#include "mt_partition.h"
#endif

#ifdef CONFIG_CMD_SNAND

//#define _SNAND_DEBUG

#ifndef CACHE_LINE
    #ifdef MT6571
        #define CACHE_LINE (64)
    #else
        #error "[LK-SNAND] Please define CACHE_LINE size!"
    #endif
#endif

#define SNAND_WriteReg(reg, value) \
do { \
    *(reg) = (value); \
} while (0)

#define SNAND_REG_SET_BITS(reg, value) \
do { \
    *(reg) = *(reg) | (value); \
} while (0)

#define SNAND_REG_CLN_BITS(reg, value) \
do { \
    *(reg) = *(reg) & (~(value)); \
} while (0)

#ifndef PART_SIZE_BMTPOOL
#define BMT_POOL_SIZE (80)
#else
#define BMT_POOL_SIZE (PART_SIZE_BMTPOOL)
#endif

#define PMT_POOL_SIZE	(2)

// Read Split related definitions and variables
#define SNAND_RS_BOUNDARY_BLOCK                     (2)
#define SNAND_RS_BOUNDARY_KB                        (1024)

#define SNAND_RS_SPARE_PER_SECTOR_PART0_VAL         (16)                                        // MT6571 shoud fix this as 0
#define SNAND_RS_SPARE_PER_SECTOR_PART0_NFI         (PAGEFMT_SPARE_16 << PAGEFMT_SPARE_SHIFT)   // MT6571 shoud fix this as 16
#define SNAND_RS_ECC_BIT_PART0                      (0)                                         // MT6571 use device ECC in part 0

u32 g_snand_rs_ecc_bit_second_part;
u32 g_snand_rs_spare_per_sector_second_part_nfi;
u32 g_snand_rs_num_page = 0;
u32 g_snand_rs_cur_part = 0xFFFFFFFF;
u32 g_snand_rs_ecc_bit = 0;

u32 g_snand_spare_per_sector = 0;   // because Read Split feature will change spare_per_sector in run-time, thus use a variable to indicate current spare_per_sector

__attribute__((aligned(CACHE_LINE))) unsigned char g_snand_lk_temp[SNAND_MAX_PAGE_SIZE + SNAND_MAX_SPARE_SIZE];

bool        __nand_erase (u64 logical_addr);
int         check_data_empty(void *data, unsigned size);
bool        nand_erase_hw (u64 offset);
static void snand_dev_command(const u32 cmd, u8 outlen);
static void snand_dev_mac_op(SNAND_Mode mode);
void        snand_dump_reg(void);
static bool snand_reset_con(void);
bool        mark_block_bad (u64 logical_addr);
bool        mark_block_bad_hw(u64 offset);
extern int  mt_part_register_device(part_dev_t * dev);


static struct nand_ecclayout nand_oob_16 =
{
    .eccbytes = 8,
    .eccpos = {8, 9, 10, 11, 12, 13, 14, 15},
    .oobfree = {{1, 6}, {0, 0}}
};

struct nand_ecclayout nand_oob_64 =
{
    .eccbytes = 32,
    .eccpos = {
        32, 33, 34, 35, 36, 37, 38, 39,
        40, 41, 42, 43, 44, 45, 46, 47,
        48, 49, 50, 51, 52, 53, 54, 55,
        56, 57, 58, 59, 60, 61, 62, 63
    },
    .oobfree = {{1, 7}, {9, 7}, {17, 7}, {25, 6}, {0, 0}}
};

struct nand_ecclayout nand_oob_128 =
{
    .eccbytes = 64,
    .eccpos = {
        64, 65, 66, 67, 68, 69, 70, 71,
        72, 73, 74, 75, 76, 77, 78, 79,
        80, 81, 82, 83, 84, 85, 86, 86,
        88, 89, 90, 91, 92, 93, 94, 95,
        96, 97, 98, 99, 100, 101, 102, 103,
        104, 105, 106, 107, 108, 109, 110, 111,
        112, 113, 114, 115, 116, 117, 118, 119,
        120, 121, 122, 123, 124, 125, 126, 127
    },
    .oobfree = {{1, 7}, {9, 7}, {17, 7}, {25, 7}, {33, 7}, {41, 7}, {49, 7},
        {57, 6}
    }
};

u32 g_snand_lk_spare_format_1[][2] =
{
    {4, 8}
    ,{20, 8}
    ,{36, 8}
    ,{52, 8}
    ,{0, 0}
};


static bmt_struct *g_bmt = NULL;
static struct nand_chip g_nand_chip;
static int en_interrupt = 0;
static event_t nand_int_event;

typedef enum
{
    SNAND_RB_DEFAULT    = 0
                          ,SNAND_RB_READ_ID    = 1
                                  ,SNAND_RB_CMD_STATUS = 2
                                          ,SNAND_RB_PIO        = 3
} SNAND_Read_Byte_Mode;

SNAND_Read_Byte_Mode g_snand_read_byte_mode;

u8 g_snand_id_data[SNAND_MAX_ID + 1];
u8 g_snand_id_data_idx = 0;

snand_flashdev_info devinfo;

#define CHIPVER_ECO_1 (0x8a00)
#define CHIPVER_ECO_2 (0x8a01)

struct NAND_CMD g_kCMD;
static u32 g_i4ErrNum;
static bool g_bInitDone;
u64 total_size;
u64 g_nand_size = 0;

static unsigned char g_data_buf[4096+128] __attribute__ ((aligned(CACHE_LINE)));
static unsigned char g_spare_buf[256];
static u32 download_size = 0;

static inline unsigned int uffs(unsigned int x)
{
    unsigned int r = 1;

    if (!x)
    {
        return 0;
    }
    if (!(x & 0xffff))
    {
        x >>= 16;
        r += 16;
    }
    if (!(x & 0xff))
    {
        x >>= 8;
        r += 8;
    }
    if (!(x & 0xf))
    {
        x >>= 4;
        r += 4;
    }
    if (!(x & 3))
    {
        x >>= 2;
        r += 2;
    }
    if (!(x & 1))
    {
        x >>= 1;
        r += 1;
    }
    return r;
}

bool snand_get_device_info(u8*id, snand_flashdev_info *devinfo)
{
    u32 i,m,n,mismatch;
    int target=-1;
    u8 target_id_len=0;

    for (i = 0; i < SNAND_CHIP_CNT; i++)
    {
        mismatch=0;

        for (m=0; m<gen_snand_FlashTable[i].id_length; m++)
        {
            if(id[m]!=gen_snand_FlashTable[i].id[m])
            {
                mismatch=1;
                break;
            }
        }

        if (mismatch == 0 && gen_snand_FlashTable[i].id_length > target_id_len)
        {
            target=i;
            target_id_len=gen_snand_FlashTable[i].id_length;
        }
    }

    if (target != -1)
    {
        MSG(INIT, "Recognize NAND: ID [");

        for (n=0; n<gen_snand_FlashTable[target].id_length; n++)
        {
            devinfo->id[n] = gen_snand_FlashTable[target].id[n];
            MSG(INIT, "%x ",devinfo->id[n]);
        }

        MSG(INIT, "], Device Name [%s], Page Size [%d]B Spare Size [%d]B Total Size [%d]MB\n",gen_snand_FlashTable[target].devicename,gen_snand_FlashTable[target].pagesize,gen_snand_FlashTable[target].sparesize,gen_snand_FlashTable[target].totalsize);
        devinfo->id_length=gen_snand_FlashTable[target].id_length;
        devinfo->blocksize = gen_snand_FlashTable[target].blocksize;
        devinfo->advancedmode = gen_snand_FlashTable[target].advancedmode;
        devinfo->spareformat = gen_snand_FlashTable[target].spareformat;

        // SW workaround for SNAND_ADV_READ_SPLIT
        if (0xC8 == devinfo->id[0] && 0xF4 == devinfo->id[1])
        {
            devinfo->advancedmode |= (SNAND_ADV_READ_SPLIT | SNAND_ADV_VENDOR_RESERVED_BLOCKS);
        }

        devinfo->pagesize = gen_snand_FlashTable[target].pagesize;
        devinfo->sparesize = gen_snand_FlashTable[target].sparesize;
        devinfo->totalsize = gen_snand_FlashTable[target].totalsize;
        memcpy(devinfo->devicename, gen_snand_FlashTable[target].devicename, sizeof(devinfo->devicename));
        devinfo->SNF_DLY_CTL1 = gen_snand_FlashTable[target].SNF_DLY_CTL1;
        devinfo->SNF_DLY_CTL2 = gen_snand_FlashTable[target].SNF_DLY_CTL2;
        devinfo->SNF_DLY_CTL3 = gen_snand_FlashTable[target].SNF_DLY_CTL3;
        devinfo->SNF_DLY_CTL4 = gen_snand_FlashTable[target].SNF_DLY_CTL4;
        devinfo->SNF_MISC_CTL = gen_snand_FlashTable[target].SNF_MISC_CTL;
        devinfo->SNF_DRIVING = gen_snand_FlashTable[target].SNF_DRIVING;

        return 1;
    }
    else
    {
        MSG(INIT, "Not Found NAND: ID [");

        for(n=0; n<SNAND_MAX_ID; n++)
        {
            MSG(INIT, "%x ",id[n]);
        }

        MSG(INIT, "]\n");

        return 0;
    }
}

// Read Split related APIs
static bool snand_rs_lk_if_require_split() // must be executed after snand_rs_lk_reconfig_nfiecc()
{
    if (devinfo.advancedmode & SNAND_ADV_READ_SPLIT)
    {
        if (g_snand_rs_cur_part != 0)
        {
            /*
             * both 6572 & 6571 needs read split for special SPI-NAND device
             */

            return 1;
        }
        else
        {
            /*
             * MT6572: Part 0 uses 4-bit ECC, read split is not required
             * MT6571: Part 0 uses device ECC, read split is not required
             */
            return 0;
        }
    }
    else
    {
        return 0;
    }
}

void snand_dev_ecc_control(u8 enable)
{
    u32 cmd;
    u8  otp;
    u8  otp_new;

    // read original otp settings

    cmd = SNAND_CMD_GET_FEATURES | (SNAND_CMD_FEATURES_OTP << 8);
    SNAND_WriteReg(RW_SNAND_GPRAM_DATA, cmd);
    SNAND_WriteReg(RW_SNAND_MAC_OUTL, 2);
    SNAND_WriteReg(RW_SNAND_MAC_INL , 1);

    snand_dev_mac_op(SPI);

    otp = *(((P_U8)RW_SNAND_GPRAM_DATA + 2));

    if (1 == enable)
    {
        otp_new = otp | SNAND_OTP_ECC_ENABLE;
    }
    else
    {
        otp_new = otp & ~SNAND_OTP_ECC_ENABLE;
    }

    if (otp != otp_new)
    {
        // write enable

        snand_dev_command(SNAND_CMD_WRITE_ENABLE, 1);


        // set features
        cmd = SNAND_CMD_SET_FEATURES | (SNAND_CMD_FEATURES_OTP << 8) | (otp_new << 16);

        snand_dev_command(cmd, 3);
    }
}

static void snand_rs_lk_reconfig_nfiecc(u32 row_addr)
{
    // 1. only decode part should be re-configured
    // 2. only re-configure essential part (fixed register will not be re-configured)

    u16 reg16;
    u32 ecc_bit_cfg = 0;
    u32 u4DECODESize;

    if (0 == (devinfo.advancedmode & SNAND_ADV_READ_SPLIT))
    {
        return;
    }

    if (row_addr < g_snand_rs_num_page)
    {
        if (g_snand_rs_cur_part != 0)
        {
            g_snand_rs_ecc_bit = SNAND_RS_ECC_BIT_PART0;

            reg16 = *(NFI_PAGEFMT_REG16);
            reg16 &= ~PAGEFMT_SPARE_MASK;
            reg16 |= SNAND_RS_SPARE_PER_SECTOR_PART0_NFI;
            SNAND_WriteReg(NFI_PAGEFMT_REG16, reg16);

            g_snand_spare_per_sector = SNAND_RS_SPARE_PER_SECTOR_PART0_VAL;

            g_snand_rs_cur_part = 0;
        }
        else
        {
            return;
        }
    }
    else
    {
        if (g_snand_rs_cur_part != 1)
        {
            g_snand_rs_ecc_bit = g_snand_rs_ecc_bit_second_part;

            reg16 = *(NFI_PAGEFMT_REG16);
            reg16 &= ~PAGEFMT_SPARE_MASK;
            reg16 |= g_snand_rs_spare_per_sector_second_part_nfi;
            SNAND_WriteReg(NFI_PAGEFMT_REG16, reg16);

            g_snand_spare_per_sector = g_nand_chip.oobsize / (g_nand_chip.page_size / NAND_SECTOR_SIZE);

            g_snand_rs_cur_part = 1;
        }
        else
        {
            return;
        }
    }

    if (0 == g_snand_rs_ecc_bit)
    {
        snand_dev_ecc_control(1);   // enable device ECC

        return; // Use device in this partition. ECC re-configuration is not required
    }
    else
    {
        snand_dev_ecc_control(0);   // disable device ECC
    }

    switch(g_snand_rs_ecc_bit)
    {
        case 4:
            ecc_bit_cfg = ECC_CNFG_ECC4;
            break;
        case 8:
            ecc_bit_cfg = ECC_CNFG_ECC8;
            break;
        case 10:
            ecc_bit_cfg = ECC_CNFG_ECC10;
            break;
        case 12:
            ecc_bit_cfg = ECC_CNFG_ECC12;
            break;
        default:
            break;
    }

    SNAND_WriteReg(ECC_DECCON_REG16, DEC_DE);

    do
    {
        ;
    }
    while (!*(ECC_DECIDLE_REG16));

    u4DECODESize = ((NAND_SECTOR_SIZE + NAND_FDM_PER_SECTOR) << 3) + g_snand_rs_ecc_bit * 14;

    /* configure ECC decoder && encoder */
    SNAND_WriteReg(ECC_DECCNFG_REG32, DEC_CNFG_CORRECT | ecc_bit_cfg | DEC_CNFG_NFI | DEC_CNFG_EMPTY_EN | (u4DECODESize << DEC_CNFG_CODE_SHIFT));
}

static void snand_ecc_config(nand_ecc_level ecc_level)
{
    u32 u4ENCODESize;
    u32 u4DECODESize;
    u32 ecc_bit_cfg = ECC_CNFG_ECC4;

    switch(ecc_level)
    {
        case 4:
            ecc_bit_cfg = ECC_CNFG_ECC4;
            break;
        case 8:
            ecc_bit_cfg = ECC_CNFG_ECC8;
            break;
        case 10:
            ecc_bit_cfg = ECC_CNFG_ECC10;
            break;
        case 12:
            ecc_bit_cfg = ECC_CNFG_ECC12;
            break;
        default:
            break;
    }
    SNAND_WriteReg(ECC_DECCON_REG16, DEC_DE);
    do
    {
        ;
    }
    while (!*(ECC_DECIDLE_REG16));

    SNAND_WriteReg(ECC_ENCCON_REG16, ENC_DE);
    do
    {
        ;
    }
    while (!*(ECC_ENCIDLE_REG32));

    /* setup FDM register base */

    u4ENCODESize = (NAND_SECTOR_SIZE + NAND_FDM_PER_SECTOR) << 3;
    u4DECODESize = ((NAND_SECTOR_SIZE + NAND_FDM_PER_SECTOR) << 3) + ecc_level * 14;

    /* configure ECC decoder && encoder */
    SNAND_WriteReg(ECC_DECCNFG_REG32, ecc_bit_cfg | DEC_CNFG_NFI | DEC_CNFG_EMPTY_EN | (u4DECODESize << DEC_CNFG_CODE_SHIFT));
    SNAND_WriteReg(ECC_ENCCNFG_REG32, ecc_bit_cfg | ENC_CNFG_NFI | (u4ENCODESize << ENC_CNFG_MSG_SHIFT));

#ifndef MANUAL_CORRECT
    SNAND_REG_SET_BITS(ECC_DECCNFG_REG32, DEC_CNFG_CORRECT);
#else
    SNAND_REG_SET_BITS(ECC_DECCNFG_REG32, DEC_CNFG_EL);
#endif
}


static void snand_ecc_decode_start(void)
{
    /* wait for device returning idle */
    while (!(*(ECC_DECIDLE_REG16) & DEC_IDLE)) ;
    SNAND_WriteReg(ECC_DECCON_REG16, DEC_EN);
}

static void snand_ecc_decode_end(void)
{
    /* wait for device returning idle */
    while (!(*(ECC_DECIDLE_REG16) & DEC_IDLE)) ;
    SNAND_WriteReg(ECC_DECCON_REG16, DEC_DE);
}

//-------------------------------------------------------------------------------
static void snand_ecc_encode_start(void)
{
    /* wait for device returning idle */
    while (!(*(ECC_ENCIDLE_REG32) & ENC_IDLE)) ;
    SNAND_WriteReg(ECC_ENCCON_REG16, ENC_EN);
}

//-------------------------------------------------------------------------------
static void snand_ecc_encode_end(void)
{
    /* wait for device returning idle */
    while (!(*(ECC_ENCIDLE_REG32) & ENC_IDLE)) ;
    SNAND_WriteReg(ECC_ENCCON_REG16, ENC_DE);
}

u32 snand_nfi_if_empty_page(void)
{
   u32 reg_val = 0;

   reg_val = *(NFI_STA_REG32);

   if (0 != (reg_val & STA_READ_EMPTY)) // empty page
   {
      reg_val = 1;
   }
   else
   {
      reg_val = 0;
   }

   snand_reset_con();

   return reg_val;
}

//-------------------------------------------------------------------------------
static bool snand_check_bch_error(u8 * pDataBuf, u8 * pOOBBuf, u32 u4SecIndex, u32 u4PageAddr)
{
    bool bRet = 1;
    u32 err_num_sectors;
    u16 u2SectorDoneMask = 1 << u4SecIndex;
    u32 u4ErrorNumDebug0, u4ErrorNumDebug1, i, u4ErrNum;
    u32 timeout = 0xFFFF;

#ifdef MANUAL_CORRECT
    u32 au4ErrBitLoc[6];
    u32 u4ErrByteLoc, u4BitOffset;
    u32 u4ErrBitLoc1th, u4ErrBitLoc2nd;
#endif

    while (0 == (u2SectorDoneMask & *(ECC_DECDONE_REG16)))
    {
        timeout--;
        if (0 == timeout)
        {
            return 0;
        }
    }

#ifndef MANUAL_CORRECT
    u4ErrorNumDebug0 = *(ECC_DECENUM0_REG32);
    u4ErrorNumDebug1 = *(ECC_DECENUM1_REG32);

    if (0 != (u4ErrorNumDebug0 & 0x3F3F3F3F) || 0 != (u4ErrorNumDebug1 & 0x3F3F3F3F))
    {
        err_num_sectors = 0;

        for (i = 0; i <= u4SecIndex; ++i)
        {
            if (i < 4)
            {
                u4ErrNum = *(ECC_DECENUM0_REG32) >> (i * 8);
            }
            else
            {
                u4ErrNum = *(ECC_DECENUM1_REG32) >> ((i - 4) * 8);
            }

            u4ErrNum &= 0x3F;

            if (0x3F == u4ErrNum)
            {
                // check if all data are empty (MT6571 does not support DEC_EMPTY_EN)

                err_num_sectors++;

                #if 1

                #else   // manual check empty

                for (j = NAND_SECTOR_SIZE * i; j < NAND_SECTOR_SIZE * (i + 1); j++)
                {
                    if (pDataBuf[j] != 0xFF)
                    {
                        err_confirmed = 1;

                        break;
                    }
                }

                if (pOOBBuf)
                {
                    for (j = SNAND_FDM_DATA_SIZE_PER_SECTOR * i; j < SNAND_FDM_DATA_SIZE_PER_SECTOR * (i + 1); j++)
                    {
                        if (pOOBBuf[i] != 0xFF)
                        {
                            err_confirmed = 1;

                            break;
                        }
                    }
                }

                #endif
            }
            else
            {
                if (u4ErrNum)
                {
                    MSG(INFO, "[LK-SNAND] ECC-C #Err=%d at PageAddr=%d, Sector=%d\n", u4ErrNum, u4PageAddr, i);
                }
            }
        }

        if (err_num_sectors != 0)
        {
            if ((err_num_sectors == (u4SecIndex + 1)) &&
                (1 == snand_nfi_if_empty_page()))
            {
                //MSG(INFO, "[LK-SNAND] False ECC-U alarm at Page=%d\n", u4PageAddr);

                // false alarm, do nothing
            }
            else
            {
                MSG(INFO, "[LK-SNAND] ECC-U at Page=%d\n", u4PageAddr);

                bRet = 0;
            }
        }
    }
#else
    memset(au4ErrBitLoc, 0x0, sizeof(au4ErrBitLoc));
    u4ErrorNumDebug0 = *(ECC_DECENUM_REG32);
    u4ErrNum = *(ECC_DECENUM_REG32) >> (u4SecIndex << 2);
    u4ErrNum &= 0xF;
    if (u4ErrNum)
    {
        if (0xF == u4ErrNum)
        {
            MSG(ERR, "UnCorrectable at PageAddr=%d\n", u4PageAddr);
            bRet = 0;
        }
        else
        {
            for (i = 0; i < ((u4ErrNum + 1) >> 1); ++i)
            {
                au4ErrBitLoc[i] = *(ECC_DECEL0_REG32 + i);
                u4ErrBitLoc1th = au4ErrBitLoc[i] & 0x1FFF;
                if (u4ErrBitLoc1th < 0x1000)
                {
                    u4ErrByteLoc = u4ErrBitLoc1th / 8;
                    u4BitOffset = u4ErrBitLoc1th % 8;
                    pDataBuf[u4ErrByteLoc] = pDataBuf[u4ErrByteLoc] ^ (1 << u4BitOffset);
                }
                else
                {
                    MSG(ERR, "UnCorrectable ErrLoc=%d\n", au4ErrBitLoc[i]);
                }

                u4ErrBitLoc2nd = (au4ErrBitLoc[i] >> 16) & 0x1FFF;
                if (0 != u4ErrBitLoc2nd)
                {
                    if (u4ErrBitLoc2nd < 0x1000)
                    {
                        u4ErrByteLoc = u4ErrBitLoc2nd / 8;
                        u4BitOffset = u4ErrBitLoc2nd % 8;
                        pDataBuf[u4ErrByteLoc] = pDataBuf[u4ErrByteLoc] ^ (1 << u4BitOffset);
                    }
                    else
                    {
                        MSG(ERR, "UnCorrectable High ErrLoc=%d\n", au4ErrBitLoc[i]);
                    }
                }
            }
            bRet = 1;
        }

        if (0 == (*(ECC_DECFER_REG16) & (1 << u4SecIndex)))
        {
            bRet = 0;
        }
    }
#endif

    return bRet;
}

static bool snand_RFIFOValidSize(u16 u2Size)
{
    u32 timeout = 0xFFFF;
    while (FIFO_RD_REMAIN(*(NFI_FIFOSTA_REG16)) < u2Size)
    {
        timeout--;
        if (0 == timeout)
        {
            return 0;
        }
    }
    if (u2Size == 0)
    {
        while (FIFO_RD_REMAIN(*(NFI_FIFOSTA_REG16)))
        {
            timeout--;
            if (0 == timeout)
            {
                printf("snand_RFIFOValidSize failed: 0x%x\n", u2Size);
                return 0;
            }
        }
    }

    return 1;
}

//-------------------------------------------------------------------------------
static bool snand_WFIFOValidSize(u16 u2Size)
{
    u32 timeout = 0xFFFF;
    while (FIFO_WR_REMAIN(*(NFI_FIFOSTA_REG16)) > u2Size)
    {
        timeout--;
        if (0 == timeout)
        {
            return 0;
        }
    }
    if (u2Size == 0)
    {
        while (FIFO_WR_REMAIN(*(NFI_FIFOSTA_REG16)))
        {
            timeout--;
            if (0 == timeout)
            {
                printf("snand_RFIFOValidSize failed: 0x%x\n", u2Size);
                return 0;
            }
        }
    }

    return 1;
}

static bool snand_status_ready(u32 u4Status)
{
#if 0
    u32 timeout = 0xFFFF;
    while ((*(NFI_STA_REG32) & u4Status) != 0)
    {
        timeout--;
        if (0 == timeout)
        {
            return 0;
        }
    }
#endif

    return 1;
}

static void snand_wait_us(u32 us)
{
    gpt_busy_wait_us(us);
    //udelay(us);
}

static void snand_dev_mac_enable(SNAND_Mode mode)
{
    u32 mac;

    mac = *(RW_SNAND_MAC_CTL);

    // SPI
    if (mode == SPI)
    {
        mac &= ~SNAND_MAC_SIO_SEL;   // Clear SIO_SEL to send command in SPI style
        mac |= SNAND_MAC_EN;         // Enable Macro Mode
    }
    // QPI
    else
    {
        /*
         * SFI V2: QPI_EN only effects direct read mode, and it is moved into DIRECT_CTL in V2
         *         There's no need to clear the bit again.
         */
        mac |= (SNAND_MAC_SIO_SEL | SNAND_MAC_EN);  // Set SIO_SEL to send command in QPI style, and enable Macro Mode
    }

    SNAND_WriteReg(RW_SNAND_MAC_CTL, mac);
}

/**
 * @brief This funciton triggers SFI to issue command to serial flash, wait SFI until ready.
 *
 * @remarks: !NOTE! This function must be used with snand_dev_mac_enable in pair!
 */
static void snand_dev_mac_trigger(void)
{
    u32 mac;

    mac = *(RW_SNAND_MAC_CTL);

    // Trigger SFI: Set TRIG and enable Macro mode
    mac |= (SNAND_TRIG | SNAND_MAC_EN);
    SNAND_WriteReg(RW_SNAND_MAC_CTL, mac);

    /*
     * Wait for SFI ready
     * Step 1. Wait for WIP_READY becoming 1 (WIP register is ready)
     */
    while (!(*(RW_SNAND_MAC_CTL) & SNAND_WIP_READY));

    /*
     * Step 2. Wait for WIP becoming 0 (Controller finishes command write process)
     */
    while ((*(RW_SNAND_MAC_CTL) & SNAND_WIP));


}

/**
 * @brief This funciton leaves Macro mode and enters Direct Read mode
 *
 * @remarks: !NOTE! This function must be used after snand_dev_mac_trigger
 */
static void snand_dev_mac_leave(void)
{
    u32 mac;

    // clear SF_TRIG and leave mac mode
    mac = *(RW_SNAND_MAC_CTL);

    /*
     * Clear following bits
     * SF_TRIG: Confirm the macro command sequence is completed
     * SNAND_MAC_EN: Leaves macro mode, and enters direct read mode
     * SNAND_MAC_SIO_SEL: Always reset quad macro control bit at the end
     */
    mac &= ~(SNAND_TRIG | SNAND_MAC_EN | SNAND_MAC_SIO_SEL);
    SNAND_WriteReg(RW_SNAND_MAC_CTL, mac);
}

static void snand_dev_mac_op(SNAND_Mode mode)
{
    snand_dev_mac_enable(mode);
    snand_dev_mac_trigger();
    snand_dev_mac_leave();
}

static void snand_dev_command_ext(SNAND_Mode mode, const U8 cmd[], U8 data[], const u32 outl, const u32 inl)
{
    u32   tmp;
    u32   i, j;
    P_U8  p_data, p_tmp;

    p_tmp = (P_U8)(&tmp);

    // Moving commands into SFI GPRAM
    for (i = 0, p_data = ((P_U8)RW_SNAND_GPRAM_DATA); i < outl; p_data += 4)
    {
        // Using 4 bytes aligned copy, by moving the data into the temp buffer and then write to GPRAM
        for (j = 0, tmp = 0; i < outl && j < 4; i++, j++)
        {
            p_tmp[j] = cmd[i];
        }

        SNAND_WriteReg(p_data, tmp);
    }

    SNAND_WriteReg(RW_SNAND_MAC_OUTL, outl);
    SNAND_WriteReg(RW_SNAND_MAC_INL, inl);
    snand_dev_mac_op(mode);

    // for NULL data, this loop will be skipped
    for (i = 0, p_data = ((P_U8)RW_SNAND_GPRAM_DATA + outl); i < inl; ++i, ++data, ++p_data)
    {
        *data = *(p_data);
    }

    return;
}

static void snand_dev_command(const u32 cmd, u8 outlen)
{
    SNAND_WriteReg(RW_SNAND_GPRAM_DATA, cmd);
    SNAND_WriteReg(RW_SNAND_MAC_OUTL, outlen);
    SNAND_WriteReg(RW_SNAND_MAC_INL , 0);
    snand_dev_mac_op(SPI);

    return;
}

static void snand_reset_dev()
{
    u8 cmd = SNAND_CMD_SW_RESET;

    // issue SW RESET command to device
    snand_dev_command_ext(SPI, &cmd, NULL, 1, 0);

    // wait for awhile, then polling status register (required by spec)
    snand_wait_us(SNAND_DEV_RESET_LATENCY_US);

    *RW_SNAND_GPRAM_DATA = (SNAND_CMD_GET_FEATURES | (SNAND_CMD_FEATURES_STATUS << 8));
    *RW_SNAND_MAC_OUTL = 2;
    *RW_SNAND_MAC_INL = 1;

    // polling status register

    for (;;)
    {
        snand_dev_mac_op(SPI);

        cmd = *(((P_U8)RW_SNAND_GPRAM_DATA + 2));

        if (0 == (cmd & SNAND_STATUS_OIP))
        {
            break;
        }
    }
}

static bool snand_reset_con(void)
{
    int timeout = 0xFFFF;

    // part 1. SNF

    *RW_SNAND_MISC_CTL = *RW_SNAND_MISC_CTL | SNAND_SW_RST;
    *RW_SNAND_MISC_CTL = *RW_SNAND_MISC_CTL &= ~SNAND_SW_RST;

    // part 2. NFI

    if (*(NFI_MASTERSTA_REG16) & MASTERSTA_MASK) // master is busy
    {
        SNAND_WriteReg(NFI_CON_REG16, CON_FIFO_FLUSH | CON_NFI_RST);

        while (*(NFI_MASTERSTA_REG16))
        {
            timeout--;

            if (!timeout)
            {
                MSG(FUC, "Wait for NFI_MASTERSTA timeout\n");
            }
        }
    }
    /* issue reset operation */
    SNAND_WriteReg(NFI_CON_REG16, CON_FIFO_FLUSH | CON_NFI_RST);

    return snand_status_ready(STA_NFI_FSM_MASK | STA_NAND_BUSY) && snand_RFIFOValidSize(0) && snand_WFIFOValidSize(0);
}

//-------------------------------------------------------------------------------
static void snand_set_mode(u16 u2OpMode)
{
    u16 u2Mode = *(NFI_CNFG_REG16);
    u2Mode &= ~CNFG_OP_MODE_MASK;
    u2Mode |= u2OpMode;
    SNAND_WriteReg(NFI_CNFG_REG16, u2Mode);
}

//-------------------------------------------------------------------------------
static void snand_set_autoformat(bool bEnable)
{
    if (bEnable)
    {
        SNAND_REG_SET_BITS(NFI_CNFG_REG16, CNFG_AUTO_FMT_EN);
    }
    else
    {
        SNAND_REG_CLN_BITS(NFI_CNFG_REG16, CNFG_AUTO_FMT_EN);
    }
}

//-------------------------------------------------------------------------------
static void snand_configure_fdm(u16 u2FDMSize)
{
    SNAND_REG_CLN_BITS(NFI_PAGEFMT_REG16, PAGEFMT_FDM_MASK | PAGEFMT_FDM_ECC_MASK);
    SNAND_REG_SET_BITS(NFI_PAGEFMT_REG16, u2FDMSize << PAGEFMT_FDM_SHIFT);
    SNAND_REG_SET_BITS(NFI_PAGEFMT_REG16, u2FDMSize << PAGEFMT_FDM_ECC_SHIFT);
}

//-------------------------------------------------------------------------------
static bool snand_check_RW_count(u16 u2WriteSize)
{
    u32 timeout = 0xFFFF;
    u16 u2SecNum = u2WriteSize >> 9;

    while (ADDRCNTR_CNTR(*(NFI_ADDRCNTR_REG16)) < u2SecNum)
    {
        timeout--;
        if (0 == timeout)
        {
            return 0;
        }
    }
    return 1;
}

static u32 snand_reverse_byte_order(u32 num)
{
    u32 ret = 0;

    ret |= ((num >> 24) & 0x000000FF);
    ret |= ((num >> 8)  & 0x0000FF00);
    ret |= ((num << 8)  & 0x00FF0000);
    ret |= ((num << 24) & 0xFF000000);

    return ret;
}

static u32 snand_gen_c1a3(const u32 cmd, const u32 address)
{
    return ((snand_reverse_byte_order(address) & 0xFFFFFF00) | (cmd & 0xFF));
}

static void snand_dev_enable_spiq(bool enable)
{
    u8   regval;
    u32  cmd;

    // read QE in status register
    cmd = SNAND_CMD_GET_FEATURES | (SNAND_CMD_FEATURES_OTP << 8);
    SNAND_WriteReg(RW_SNAND_GPRAM_DATA, cmd);
    SNAND_WriteReg(RW_SNAND_MAC_OUTL, 2);
    SNAND_WriteReg(RW_SNAND_MAC_INL , 1);

    snand_dev_mac_op(SPI);

    regval = *(((volatile u8 *)RW_SNAND_GPRAM_DATA + 2));

    if (0 == enable)    // disable x4
    {
        if ((regval & SNAND_OTP_QE) == 0)
        {
            return;
        }
        else
        {
            regval = regval & ~SNAND_OTP_QE;
        }
    }
    else    // enable x4
    {
        if ((regval & SNAND_OTP_QE) == 1)
        {
            return;
        }
        else
        {
            regval = regval | SNAND_OTP_QE;
        }
    }

    // if goes here, it means QE needs to be set as new different value

    // write status register
    cmd = SNAND_CMD_SET_FEATURES | (SNAND_CMD_FEATURES_OTP << 8) | (regval << 16);
    SNAND_WriteReg(RW_SNAND_GPRAM_DATA, cmd);
    SNAND_WriteReg(RW_SNAND_MAC_OUTL, 3);
    SNAND_WriteReg(RW_SNAND_MAC_INL , 0);

    snand_dev_mac_op(SPI);
}

//-------------------------------------------------------------------------------
static bool snand_ready_for_read(struct nand_chip *nand, u32 u4RowAddr, u32 u4ColAddr, u32 u4SecNum, u8 * buf, u8 mtk_ecc, u8 auto_fmt, u8 ahb_mode)
{
    /* Reset NFI HW internal state machine and flush NFI in/out FIFO */
    bool        bRet = 1;
    u32         cmd;
    u32         reg;
    SNAND_Mode  mode = SPIQ;

    if (!snand_reset_con())
    {
        bRet = 0;

        goto cleanup;
    }

    // 1. Read page to cache

    cmd = snand_gen_c1a3(SNAND_CMD_PAGE_READ, u4RowAddr); // PAGE_READ command + 3-byte address

    SNAND_WriteReg(RW_SNAND_GPRAM_DATA, cmd);
    SNAND_WriteReg(RW_SNAND_MAC_OUTL, 1 + 3);
    SNAND_WriteReg(RW_SNAND_MAC_INL , 0);

    snand_dev_mac_op(SPI);

    // 2. Get features (status polling)

    cmd = SNAND_CMD_GET_FEATURES | (SNAND_CMD_FEATURES_STATUS << 8);

    SNAND_WriteReg(RW_SNAND_GPRAM_DATA, cmd);
    SNAND_WriteReg(RW_SNAND_MAC_OUTL, 2);
    SNAND_WriteReg(RW_SNAND_MAC_INL , 1);

    for (;;)
    {
        snand_dev_mac_op(SPI);

        cmd = *(((P_U8)RW_SNAND_GPRAM_DATA + 2));

        if ((cmd & SNAND_STATUS_OIP) == 0)
        {
            if (SNAND_STATUS_TOO_MANY_ERROR_BITS == (cmd & SNAND_STATUS_ECC_STATUS_MASK))
            {
                bRet = 0;
            }

            break;
        }
    }

    //------ SNF Part ------

    // set PAGE READ command & address
    reg = (SNAND_CMD_PAGE_READ << SNAND_PAGE_READ_CMD_OFFSET) | (u4RowAddr & SNAND_PAGE_READ_ADDRESS_MASK);
    SNAND_WriteReg(RW_SNAND_RD_CTL1, reg);

    // set DATA READ dummy cycle and command (use default value, ignored)
    if (mode == SPI)
    {
        reg = *(RW_SNAND_RD_CTL2);
        reg &= ~SNAND_DATA_READ_CMD_MASK;
        reg |= SNAND_CMD_RANDOM_READ & SNAND_DATA_READ_CMD_MASK;
        SNAND_WriteReg(RW_SNAND_RD_CTL2, reg);

    }
    else if (mode == SPIQ)
    {
        snand_dev_enable_spiq(1);

        reg = *(RW_SNAND_RD_CTL2);
        reg &= ~SNAND_DATA_READ_CMD_MASK;
        reg |= SNAND_CMD_RANDOM_READ_SPIQ & SNAND_DATA_READ_CMD_MASK;
        SNAND_WriteReg(RW_SNAND_RD_CTL2, reg);
    }

    // set DATA READ address
    SNAND_WriteReg(RW_SNAND_RD_CTL3, (u4ColAddr & SNAND_DATA_READ_ADDRESS_MASK));

    // set SNF data length (set in snand_xxx_read_data)

    // set SNF timing
    reg = *(RW_SNAND_MISC_CTL);

    reg |= SNAND_DATARD_CUSTOM_EN;

    if (mode == SPI)
    {
        reg &= ~SNAND_DATA_READ_MODE_MASK;
    }
    else if (mode == SPIQ)
    {
        reg &= ~SNAND_DATA_READ_MODE_MASK;
        reg |= ((SNAND_DATA_READ_MODE_X4 << SNAND_DATA_READ_MODE_OFFSET) & SNAND_DATA_READ_MODE_MASK);
    }

    SNAND_WriteReg(RW_SNAND_MISC_CTL, reg);

    reg = u4SecNum * (NAND_SECTOR_SIZE + g_snand_spare_per_sector);

    SNAND_WriteReg(RW_SNAND_MISC_CTL2, (reg | (reg << SNAND_PROGRAM_LOAD_BYTE_LEN_OFFSET)));

    arch_clean_invalidate_cache_range((addr_t)buf, (size_t)reg);

    //------ NFI Part ------

    snand_set_mode(CNFG_OP_CUST);
    SNAND_REG_SET_BITS(NFI_CNFG_REG16, CNFG_READ_EN);
    SNAND_WriteReg(NFI_CON_REG16, u4SecNum << CON_NFI_SEC_SHIFT);

    SNAND_WriteReg(NFI_SPIDMA_REG32, 0);

    if (ahb_mode)
    {
        SNAND_REG_SET_BITS(NFI_CNFG_REG16, CNFG_AHB);

    }
    else
    {
        SNAND_REG_CLN_BITS(NFI_CNFG_REG16, CNFG_AHB);
    }

    SNAND_WriteReg(NFI_STRADDR_REG32, (u32)buf);

    if (mtk_ecc)
    {
        SNAND_REG_SET_BITS(NFI_CNFG_REG16, CNFG_HW_ECC_EN);
    }
    else
    {
        SNAND_REG_CLN_BITS(NFI_CNFG_REG16, CNFG_HW_ECC_EN);
    }

#if 0
    if (bFull)  // read AUTO_FMT data
    {
#if USE_AHB_MODE
        SNAND_REG_SET_BITS(NFI_CNFG_REG16, CNFG_AHB);
#else
        SNAND_REG_CLN_BITS(NFI_CNFG_REG16, CNFG_AHB);
#endif
        SNAND_WriteReg(NFI_STRADDR_REG32, buf);
        SNAND_REG_SET_BITS(NFI_CNFG_REG16, CNFG_HW_ECC_EN);
    }
    else  // read raw data by MCU
    {
        SNAND_REG_CLN_BITS(NFI_CNFG_REG16, CNFG_AHB);
        SNAND_REG_CLN_BITS(NFI_CNFG_REG16, CNFG_HW_ECC_EN);
    }

    snand_set_autoformat(bFull);


#endif

    snand_set_autoformat(auto_fmt);

    if (mtk_ecc)
    {
        snand_ecc_decode_start();
    }

cleanup:

    return bRet;
}

#ifdef _SNAND_DEBUG
#include "mtk_wdt.h"
void snand_dump_mem(u32 * buf, u32 size)
{
    u32 i;

    return;

    mtk_wdt_disable();

    for (i = 0; i < (size / sizeof(u32)); i++)
    {
        printf("%0X ", buf[i]);

        if ((i % 8) == 7)
        {
            printf("\n");

            udelay(20);
        }
    }
}
#endif

//-----------------------------------------------------------------------------
static bool snand_ready_for_write(struct nand_chip *nand, u32 u4RowAddr, u8 * buf, u8 mtk_ecc, u8 auto_fmt, u8 ahb_mode)
{
    bool        bRet = 0;
    u16         sec_num = 1 << (nand->page_shift - 9);
    u32         reg;
    SNAND_Mode  mode = SPIQ;

    if (!snand_reset_con())
    {
        return 0;
    }

    #ifdef _SNAND_DEBUG
    snand_dump_mem((u32 *)buf, devinfo.pagesize);
    #endif

    // 1. Write Enable
    snand_dev_command(SNAND_CMD_WRITE_ENABLE, 1);

    //------ SNF Part ------

    // set SPI-NAND command
    if (SPI == mode)
    {
        reg = SNAND_CMD_WRITE_ENABLE | (SNAND_CMD_PROGRAM_LOAD << SNAND_PG_LOAD_CMD_OFFSET) | (SNAND_CMD_PROGRAM_EXECUTE << SNAND_PG_EXE_CMD_OFFSET);
        SNAND_WriteReg(RW_SNAND_PG_CTL1, reg);
    }
    else if (SPIQ == mode)
    {
        reg = SNAND_CMD_WRITE_ENABLE | (SNAND_CMD_PROGRAM_LOAD_X4<< SNAND_PG_LOAD_CMD_OFFSET) | (SNAND_CMD_PROGRAM_EXECUTE << SNAND_PG_EXE_CMD_OFFSET);
        SNAND_WriteReg(RW_SNAND_PG_CTL1, reg);
        snand_dev_enable_spiq(1);
    }

    // set program load address
    SNAND_WriteReg(RW_SNAND_PG_CTL2, 0 & SNAND_PG_LOAD_ADDR_MASK);  // col_addr = 0

    // set program execution address
    SNAND_WriteReg(RW_SNAND_PG_CTL3, u4RowAddr);

    // set SNF data length  (set in snand_xxx_write_data)

    // set SNF timing
    reg = *(RW_SNAND_MISC_CTL);

    reg |= SNAND_PG_LOAD_CUSTOM_EN;

    if (SPI == mode)
    {
        reg &= ~SNAND_DATA_READ_MODE_MASK;
        reg |= ((SNAND_DATA_READ_MODE_X1 << SNAND_DATA_READ_MODE_OFFSET) & SNAND_DATA_READ_MODE_MASK);
        reg &=~ SNAND_PG_LOAD_X4_EN;
    }
    else if (SPIQ == mode)
    {
        reg &= ~SNAND_DATA_READ_MODE_MASK;
        reg |= ((SNAND_DATA_READ_MODE_X4 << SNAND_DATA_READ_MODE_OFFSET) & SNAND_DATA_READ_MODE_MASK);
        reg |= SNAND_PG_LOAD_X4_EN;
    }

    SNAND_WriteReg(RW_SNAND_MISC_CTL, reg);

    reg = sec_num * (NAND_SECTOR_SIZE + g_snand_spare_per_sector);

    arch_clean_invalidate_cache_range((addr_t)buf, (size_t)reg);

    // set SNF data length
    SNAND_WriteReg(RW_SNAND_MISC_CTL2, reg | (reg << SNAND_PROGRAM_LOAD_BYTE_LEN_OFFSET));

    //------ NFI Part ------

    // reset NFI
    snand_reset_con();

    snand_set_mode(CNFG_OP_PRGM);

    SNAND_REG_CLN_BITS(NFI_CNFG_REG16, CNFG_READ_EN);
    SNAND_WriteReg(NFI_CON_REG16, sec_num << CON_NFI_SEC_SHIFT);

    SNAND_WriteReg(NFI_SPIDMA_REG32, 0);

    SNAND_WriteReg(NFI_STRADDR_REG32, (u32)buf);

    if (ahb_mode)
    {
        SNAND_REG_SET_BITS(NFI_CNFG_REG16, CNFG_AHB);
    }
    else
    {
        SNAND_REG_CLN_BITS(NFI_CNFG_REG16, CNFG_AHB);
    }

    snand_set_autoformat(auto_fmt);

    if (mtk_ecc)
    {
        SNAND_REG_SET_BITS(NFI_CNFG_REG16, CNFG_HW_ECC_EN);
        snand_ecc_encode_start();
    }
    else
    {
        SNAND_REG_CLN_BITS(NFI_CNFG_REG16, CNFG_HW_ECC_EN);
    }

#if 0
#if USE_AHB_MODE
    SNAND_REG_SET_BITS(NFI_CNFG_REG16, CNFG_AHB);
    SNAND_WriteReg(NFI_STRADDR_REG32, buf);
#else
    SNAND_REG_CLN_BITS(NFI_CNFG_REG16, CNFG_AHB);
#endif

    SNAND_REG_SET_BITS(NFI_CNFG_REG16, CNFG_HW_ECC_EN);

    snand_set_autoformat(1);

    snand_ecc_encode_start();
#endif

    bRet = 1;

    return bRet;
}

//-----------------------------------------------------------------------------
static bool snand_dma_read_data(u8 * pDataBuf, u32 num_sec)
{
    u32 timeout = 0xFFFF;

    SNAND_REG_CLN_BITS(NFI_CNFG_REG16, CNFG_BYTE_RW);

    // set dummy command to trigger NFI enter custom mode
    SNAND_WriteReg(NFI_CMD_REG16, NAND_CMD_DUMMYREAD);

    *(NFI_INTR_REG16);
    SNAND_WriteReg(NFI_INTR_EN_REG16, INTR_AHB_DONE_EN);

    SNAND_REG_SET_BITS(NFI_CON_REG16, CON_NFI_BRD);

    if (en_interrupt)
    {
        if(event_wait_timeout(&nand_int_event,100))
        {
            printf("[snand_dma_read_data]wait for AHB done timeout\n");
            snand_dump_reg();
            return 0;
        }

        timeout = 0xFFFF;

        while (num_sec > ((*(NFI_BYTELEN_REG16) & 0xf000) >> 12))
        {
            timeout--;

            if (0 == timeout)
            {
                return 0;       //4
            }
        }
    }
    else
    {
        while (!(*(NFI_INTR_REG16) & INTR_AHB_DONE))
        {
            timeout--;

            if (0 == timeout)
            {
                return 0;
            }
        }

        timeout = 0xFFFF;

        while (num_sec > ((*(NFI_BYTELEN_REG16) & 0xf000) >> 12))
        {
            timeout--;

            if (0 == timeout)
            {
                return 0;       //4
            }
        }
    }
    return 1;
}

static bool snand_mcu_read_data(u8 * pDataBuf, u32 length, bool full)
{
    u32 timeout = 0xFFFF;
    u32 i;
    u32 *pBuf32;
    u32 snf_len;

    // set SNF data length
    if (full)
    {
        snf_len = length + g_nand_chip.oobsize;
    }
    else
    {
        snf_len = length;
    }

    SNAND_WriteReg(RW_SNAND_MISC_CTL2, (snf_len | (snf_len << SNAND_PROGRAM_LOAD_BYTE_LEN_OFFSET)));

    // set dummy command to trigger NFI enter custom mode
    SNAND_WriteReg(NFI_CMD_REG16, NAND_CMD_DUMMYREAD);

    if (length % 4)
    {
        SNAND_REG_SET_BITS(NFI_CNFG_REG16, CNFG_BYTE_RW);
    }
    else
    {
        SNAND_REG_CLN_BITS(NFI_CNFG_REG16, CNFG_BYTE_RW);
    }

    SNAND_REG_SET_BITS(NFI_CON_REG16, CON_NFI_BRD);

    pBuf32 = (u32 *) pDataBuf;

    if (length % 4)
    {
        for (i = 0; (i < length) && (timeout > 0);)
        {
            WAIT_NFI_PIO_READY(timeout);
            *pDataBuf++ = *(NFI_DATAR_REG32);
            i++;

        }
    }
    else
    {
        WAIT_NFI_PIO_READY(timeout);
        for (i = 0; (i < (length >> 2)) && (timeout > 0);)
        {
            WAIT_NFI_PIO_READY(timeout);
            *pBuf32++ = *(NFI_DATAR_REG32);
            i++;
        }
    }
    return 1;
}

static bool snand_read_page_data(u8 * buf, u32 num_sec, u8 ahb_mode)
{
    //#if USE_AHB_MODE
    if (ahb_mode)
    {
        return snand_dma_read_data(buf, num_sec);
    }
    //#else
    else
    {
        return snand_mcu_read_data(buf, (num_sec * (NAND_SECTOR_SIZE + (g_nand_chip.oobsize / (g_nand_chip.page_size / NAND_SECTOR_SIZE)))), 1);
    }
    //#endif
}

static bool snand_dma_write_data(u8 * buf, u32 length)
{
    u32 timeout = 0xFFFF;

    SNAND_REG_CLN_BITS(NFI_CNFG_REG16, CNFG_BYTE_RW);

    // set dummy command to trigger NFI enter custom mode
    SNAND_WriteReg(NFI_CMD_REG16, NAND_CMD_DUMMYPROG);

    *(NFI_INTR_REG16);
    SNAND_WriteReg(NFI_INTR_EN_REG16, INTR_CUSTOM_PROG_DONE_INTR_EN);

    if ((unsigned int)buf % 16)
    {
        SNAND_REG_CLN_BITS(NFI_CNFG_REG16, CNFG_DMA_BURST_EN);
    }
    else
    {
        //SNAND_REG_SET_BITS(NFI_CNFG_REG16, CNFG_DMA_BURST_EN);
        SNAND_REG_CLN_BITS(NFI_CNFG_REG16, CNFG_DMA_BURST_EN); // Stanley Chu
    }

    SNAND_REG_SET_BITS(NFI_CON_REG16, CON_NFI_BWR);

    if (en_interrupt)
    {
        if (event_wait_timeout(&nand_int_event,100))
        {
            printf("[snand_dma_write_data]wait for AHB done timeout\n");
            snand_dump_reg();

            return 0;
        }
    }
    else
    {
        //while (!(*(NFI_INTR_REG16) & INTR_AHB_DONE))
        while (!(*(RW_SNAND_STA_CTL1) & SNAND_CUSTOM_PROGRAM))  // for custom program, wait RW_SNAND_STA_CTL1's SNAND_CUSTOM_PROGRAM done to ensure all data are loaded to device buffer
        {
            timeout--;

            if (0 == timeout)
            {
                printf("wait write AHB done timeout\n");
                snand_dump_reg();

                return 0;
            }
        }
    }

    return 1;
}

static bool snand_mcu_write_data(const u8 * buf, u32 length)
{
    u32 timeout = 0xFFFF;
    u32 i;
    u32 *pBuf32 = (u32 *) buf;

    SNAND_REG_CLN_BITS(NFI_CNFG_REG16, CNFG_BYTE_RW);

    // set dummy command to trigger NFI enter custom mode
    SNAND_WriteReg(NFI_CMD_REG16, NAND_CMD_DUMMYPROG);

    SNAND_REG_SET_BITS(NFI_CON_REG16, CON_NFI_BWR);

    if ((u32) buf % 4 || length % 4)
    {
        SNAND_REG_SET_BITS(NFI_CNFG_REG16, CNFG_BYTE_RW);
    }
    else
    {
        SNAND_REG_CLN_BITS(NFI_CNFG_REG16, CNFG_BYTE_RW);
    }

    if ((u32) buf % 4 || length % 4)
    {
        for (i = 0; (i < (length)) && (timeout > 0);)
        {
            if (*(NFI_PIO_DIRDY_REG16) & 1)
            {
                SNAND_WriteReg(NFI_DATAW_REG32, *buf++);
                i++;
            }
            else
            {
                timeout--;
            }
            if (0 == timeout)
            {
                printf("[%s] nand mcu write timeout\n", __FUNCTION__);
                snand_dump_reg();
                return 0;
            }
        }
    }
    else
    {
        for (i = 0; (i < (length >> 2)) && (timeout > 0);)
        {
            if (*(NFI_PIO_DIRDY_REG16) & 1)
            {
                SNAND_WriteReg(NFI_DATAW_REG32, *pBuf32++);
                i++;
            }
            else
            {
                timeout--;
            }
            if (0 == timeout)
            {
                printf("[%s] nand mcu write timeout\n", __FUNCTION__);
                snand_dump_reg();
                return 0;
            }
        }
    }

    return 1;
}

//-----------------------------------------------------------------------------
static bool snand_write_page_data(u8 * buf, u32 length, u8 ahb_mode)
{
    //#if USE_AHB_MODE
    if (ahb_mode)
    {
        return snand_dma_write_data(buf, length);
    }
    //#else
    else
    {
        return snand_mcu_write_data(buf, length);
    }
    //#endif
}

static u8 snand_read_fdm_data_devecc(u8 * dest, u8 * src, u32 u4SecNum)
{
    u32 i, j;
    u32 dest_idx, cur_dest_sec;

    if (0 != (SNAND_SPARE_FORMAT_1 & devinfo.spareformat)) // spare format 1
    {
        dest_idx = 0;
        cur_dest_sec = 0;

        for (i = 0; g_snand_lk_spare_format_1[i][0] != 0; i++)
        {
            for (j = 0; j < g_snand_lk_spare_format_1[i][1]; j++)
            {
                dest[dest_idx] = src[devinfo.pagesize + (g_snand_lk_spare_format_1[i][0] + j)];

                dest_idx++;

                if (SNAND_FDM_DATA_SIZE_PER_SECTOR == dest_idx)  // all spare bytes in this sector are copied, go to next sector
                {
                    cur_dest_sec++;

                    if (u4SecNum == cur_dest_sec)   // all sectors' spare are filled
                    {
                        return 1;
                    }
                    else
                    {
                        dest_idx = 0;
                    }
                }
            }
        }
    }
    else
    {
        return 0;
    }

    return 1;
}

static void snand_read_fdm_data(u8 * pDataBuf, u32 u4SecNum)
{
    u32 i;
    u32 *pBuf32 = (u32 *) pDataBuf;
    for (i = 0; i < u4SecNum; ++i)
    {
        *pBuf32++ = *(NFI_FDM0L_REG32 + (i << 1));
        *pBuf32++ = *(NFI_FDM0M_REG32 + (i << 1));
    }
}

static u8 snand_write_fdm_data_devecc(u8 * dest, u8 * src, u32 u4SecNum)
{
    u32 i, j, src_idx, cur_sec_index;

    if (0 != (SNAND_SPARE_FORMAT_1 & devinfo.spareformat)) // spare format 1
    {
        // convert from continuous spare data to DEVICE ECC's spare format

        memset(dest, 0xFF, devinfo.sparesize);

        src_idx = 0;
        cur_sec_index = 0;

        for (i = 0; g_snand_lk_spare_format_1[i][0] != 0; i++)
        {
            for (j = 0; j < g_snand_lk_spare_format_1[i][1]; j++)
            {
                dest[g_snand_lk_spare_format_1[i][0] + j] = src[(cur_sec_index * SNAND_FDM_DATA_SIZE_PER_SECTOR) + src_idx];

                src_idx++;

                if (SNAND_FDM_DATA_SIZE_PER_SECTOR == src_idx)
                {
                    cur_sec_index++;

                    if (cur_sec_index == u4SecNum)
                    {
                        return 1;
                    }
                }
            }
        }
    }
    else
    {
        return 0;
    }

    return 1;
}

static void snand_write_fdm_data(u8 * pDataBuf, u32 u4SecNum)
{
    u32 i;
    u32 *pBuf32 = (u32 *) pDataBuf;
    for (i = 0; i < u4SecNum; ++i)
    {
        SNAND_WriteReg(NFI_FDM0L_REG32 + (i << 1), *pBuf32++);
        SNAND_WriteReg(NFI_FDM0M_REG32 + (i << 1), *pBuf32++);
    }
}

static void snand_stop_read(void)
{
    //------ NFI Part

    SNAND_REG_CLN_BITS(NFI_CON_REG16, CON_NFI_BRD);

    //------ SNF Part

    // set 1 then set 0 to clear done flag
    SNAND_WriteReg(RW_SNAND_STA_CTL1, SNAND_CUSTOM_READ);
    SNAND_WriteReg(RW_SNAND_STA_CTL1, 0);

    // clear essential SNF setting
    SNAND_REG_CLN_BITS(RW_SNAND_MISC_CTL, SNAND_DATARD_CUSTOM_EN);

    snand_ecc_decode_end();
}

static void snand_stop_write(void)
{
    //------ NFI Part

    SNAND_REG_CLN_BITS(NFI_CON_REG16, CON_NFI_BWR);

    //------ SNF Part

    // set 1 then set 0 to clear done flag
    SNAND_WriteReg(RW_SNAND_STA_CTL1, SNAND_CUSTOM_PROGRAM);
    SNAND_WriteReg(RW_SNAND_STA_CTL1, 0);

    // clear essential SNF setting
    SNAND_REG_CLN_BITS(RW_SNAND_MISC_CTL, SNAND_PG_LOAD_CUSTOM_EN);

    snand_dev_enable_spiq(0);

    snand_ecc_encode_end();
}

static bool snand_check_dececc_done(u32 u4SecNum)
{
    u32 timeout, dec_mask;
    timeout = 0xffff;
    dec_mask = (1 << u4SecNum) - 1;

    while ((dec_mask != *(ECC_DECDONE_REG16)) && timeout > 0)
    {
        timeout--;

        if (timeout == 0)
        {
            MSG(ERR, "ECC_DECDONE: timeout\n");
            snand_dump_reg();

            return 0;
        }
    }

    return 1;
}

static bool snand_read_page_part2(u32 row_addr, u32 num_sec, u8 * buf)
{
    bool    bRet = 1;
    u32     reg;
    u32     col_part2, i, len;
    u32     spare_per_sector;
    P_U8    buf_part2;
    u32     timeout = 0xFFFF;
    u32     old_dec_mode = 0;

    spare_per_sector = g_nand_chip.oobsize / (g_nand_chip.page_size / NAND_SECTOR_SIZE);

    arch_clean_invalidate_cache_range((addr_t)buf, (size_t)(NAND_SECTOR_SIZE + spare_per_sector));

    for (i = 0; i < 2 ; i++)
    {
        snand_reset_con();

        if (0 == i)
        {
            col_part2 = (NAND_SECTOR_SIZE + spare_per_sector) * (num_sec - 1);

            buf_part2 = buf;

            len = 2112 - col_part2;
        }
        else
        {
            col_part2 = 2112;

            buf_part2 += len;   // append to first round

            len = (num_sec * (NAND_SECTOR_SIZE + spare_per_sector)) - 2112;
        }

        //------ SNF Part ------

        // set DATA READ address
        SNAND_WriteReg(RW_SNAND_RD_CTL3, (col_part2 & SNAND_DATA_READ_ADDRESS_MASK));

        // set RW_SNAND_MISC_CTL
        reg = *(RW_SNAND_MISC_CTL);

        reg |= SNAND_DATARD_CUSTOM_EN;

        reg &= ~SNAND_DATA_READ_MODE_MASK;
        reg |= ((SNAND_DATA_READ_MODE_X4 << SNAND_DATA_READ_MODE_OFFSET) & SNAND_DATA_READ_MODE_MASK);

        SNAND_WriteReg(RW_SNAND_MISC_CTL, reg);

        // set SNF data length
        reg = len | (len << SNAND_PROGRAM_LOAD_BYTE_LEN_OFFSET);

        SNAND_WriteReg(RW_SNAND_MISC_CTL2, reg);

        //------ NFI Part ------

        snand_set_mode(CNFG_OP_CUST);
        SNAND_REG_SET_BITS(NFI_CNFG_REG16, CNFG_READ_EN);
        SNAND_REG_SET_BITS(NFI_CNFG_REG16, CNFG_AHB);
        SNAND_REG_CLN_BITS(NFI_CNFG_REG16, CNFG_HW_ECC_EN);
        snand_set_autoformat(0);

        SNAND_WriteReg(NFI_CON_REG16, 1 << CON_NFI_SEC_SHIFT);  // fixed to sector number 1

        SNAND_WriteReg(NFI_STRADDR_REG32, (u32)buf_part2);

        SNAND_WriteReg(NFI_SPIDMA_REG32, SPIDMA_SEC_EN | (len & SPIDMA_SEC_SIZE_MASK));


        SNAND_REG_CLN_BITS(NFI_CNFG_REG16, CNFG_BYTE_RW);

        // set dummy command to trigger NFI enter custom mode
        SNAND_WriteReg(NFI_CMD_REG16, NAND_CMD_DUMMYREAD);

        *(NFI_INTR_REG16);  // read clear
        SNAND_WriteReg(NFI_INTR_EN_REG16, INTR_AHB_DONE_EN);

        SNAND_REG_SET_BITS(NFI_CON_REG16, CON_NFI_BRD);

        timeout = 0xFFFF;

        while (!(*(NFI_INTR_REG16) & INTR_AHB_DONE))    // for custom read, wait NFI's INTR_AHB_DONE done to ensure all data are transferred to buffer
        {
            timeout--;

            if (0 == timeout)
            {
                return 0;
            }
        }

        timeout = 0xFFFF;

        while (((*(NFI_BYTELEN_REG16) & 0xf000) >> 12) != 1)
        {
            timeout--;

            if (0 == timeout)
            {
                return 0;
            }
        }

        //------ NFI Part

        SNAND_REG_CLN_BITS(NFI_CON_REG16, CON_NFI_BRD);

        //------ SNF Part

        // set 1 then set 0 to clear done flag
        SNAND_WriteReg(RW_SNAND_STA_CTL1, SNAND_CUSTOM_READ);
        SNAND_WriteReg(RW_SNAND_STA_CTL1, 0);

        // clear essential SNF setting
        SNAND_REG_CLN_BITS(RW_SNAND_MISC_CTL, SNAND_DATARD_CUSTOM_EN);
    }

    /* configure ECC decoder && encoder */
    reg = *(ECC_DECCNFG_REG32);
    old_dec_mode = reg & DEC_CNFG_DEC_MODE_MASK;
    reg &= ~DEC_CNFG_DEC_MODE_MASK;
    reg |= DEC_CNFG_AHB;
    SNAND_WriteReg(ECC_DECCNFG_REG32, reg);

    SNAND_WriteReg(ECC_DECDIADDR_REG32, (u32)buf);

    SNAND_WriteReg(ECC_DECCON_REG16, DEC_DE);
    SNAND_WriteReg(ECC_DECCON_REG16, DEC_EN);

    while(!((*(ECC_DECDONE_REG16)) & (1 << 0)));

    reg = *(ECC_DECENUM0_REG32);

    if (0 != reg)
    {
        reg &= 0x1F;

        if (0x1F == reg)
        {
            bRet = 0;   // ECC-U
        }
    }

    // restore essential NFI and ECC registers
    SNAND_WriteReg(NFI_SPIDMA_REG32, 0);
    reg = *(ECC_DECCNFG_REG32);
    reg &= ~DEC_CNFG_DEC_MODE_MASK;
    reg |= old_dec_mode;
    SNAND_WriteReg(ECC_DECCNFG_REG32, reg);
    SNAND_WriteReg(ECC_DECCON_REG16, DEC_DE);
    SNAND_WriteReg(ECC_DECDIADDR_REG32, 0);

    return bRet;
}

void snand_dump_reg(void)
{
    MSG(INFO, "~~~~Dump NFI/SNF/GPIO Register in Kernel~~~~\n");
    MSG(INFO, "NFI_CNFG_REG16: 0x%x\n", *(NFI_CNFG_REG16));
    MSG(INFO, "NFI_PAGEFMT_REG16: 0x%x\n", *(NFI_PAGEFMT_REG16));
    MSG(INFO, "NFI_CON_REG16: 0x%x\n", *(NFI_CON_REG16));
    MSG(INFO, "NFI_ACCCON_REG32: 0x%x\n", *(NFI_ACCCON_REG32));
    MSG(INFO, "NFI_INTR_EN_REG16: 0x%x\n", *(NFI_INTR_EN_REG16));
    MSG(INFO, "NFI_INTR_REG16: 0x%x\n", *(NFI_INTR_REG16));
    MSG(INFO, "NFI_CMD_REG16: 0x%x\n", *(NFI_CMD_REG16));
    MSG(INFO, "NFI_ADDRNOB_REG16: 0x%x\n", *(NFI_ADDRNOB_REG16));
    MSG(INFO, "NFI_COLADDR_REG32: 0x%x\n", *(NFI_COLADDR_REG32));
    MSG(INFO, "NFI_ROWADDR_REG32: 0x%x\n", *(NFI_ROWADDR_REG32));
    MSG(INFO, "NFI_STRDATA_REG16: 0x%x\n", *(NFI_STRDATA_REG16));
    MSG(INFO, "NFI_DATAW_REG32: 0x%x\n", *(NFI_DATAW_REG32));
    MSG(INFO, "NFI_DATAR_REG32: 0x%x\n", *(NFI_DATAR_REG32));
    MSG(INFO, "NFI_PIO_DIRDY_REG16: 0x%x\n", *(NFI_PIO_DIRDY_REG16));
    MSG(INFO, "NFI_STA_REG32: 0x%x\n", *(NFI_STA_REG32));
    MSG(INFO, "NFI_FIFOSTA_REG16: 0x%x\n", *(NFI_FIFOSTA_REG16));
    MSG(INFO, "NFI_ADDRCNTR_REG16: 0x%x\n", *(NFI_ADDRCNTR_REG16));
    MSG(INFO, "NFI_STRADDR_REG32: 0x%x\n", *(NFI_STRADDR_REG32));
    MSG(INFO, "NFI_BYTELEN_REG16: 0x%x\n", *(NFI_BYTELEN_REG16));
    MSG(INFO, "NFI_CSEL_REG16: 0x%x\n", *(NFI_CSEL_REG16));
    MSG(INFO, "NFI_IOCON_REG16: 0x%x\n", *(NFI_IOCON_REG16));
    MSG(INFO, "NFI_FDM0L_REG32: 0x%x\n", *(NFI_FDM0L_REG32));
    MSG(INFO, "NFI_FDM0M_REG32: 0x%x\n", *(NFI_FDM0M_REG32));
    MSG(INFO, "NFI_LOCK_REG16: 0x%x\n", *(NFI_LOCK_REG16));
    MSG(INFO, "NFI_LOCKCON_REG32: 0x%x\n", *(NFI_LOCKCON_REG32));
    MSG(INFO, "NFI_LOCKANOB_REG16: 0x%x\n", *(NFI_LOCKANOB_REG16));
    MSG(INFO, "NFI_FIFODATA0_REG32: 0x%x\n", *(NFI_FIFODATA0_REG32));
    MSG(INFO, "NFI_FIFODATA1_REG32: 0x%x\n", *(NFI_FIFODATA1_REG32));
    MSG(INFO, "NFI_FIFODATA2_REG32: 0x%x\n", *(NFI_FIFODATA2_REG32));
    MSG(INFO, "NFI_FIFODATA3_REG32: 0x%x\n", *(NFI_FIFODATA3_REG32));
    MSG(INFO, "NFI_MASTERSTA_REG16: 0x%x\n", *(NFI_MASTERSTA_REG16));

    MSG(INFO, "ECC_DECCNFG_REG32: 0x%x\n", *(ECC_DECCNFG_REG32));
    MSG(INFO, "ECC_DECENUM0_REG32: 0x%x\n", *(ECC_DECENUM0_REG32));
    MSG(INFO, "ECC_DECENUM1_REG32: 0x%x\n", *(ECC_DECENUM1_REG32));
    MSG(INFO, "ECC_DECDONE_REG16: 0x%x\n", *(ECC_DECDONE_REG16));
    MSG(INFO, "ECC_ENCCNFG_REG32: 0x%x\n", *(ECC_ENCCNFG_REG32));

	MSG(INFO, "RW_SNAND_MAC_CTL: 0x%x\n", *(RW_SNAND_MAC_CTL));
	MSG(INFO, "RW_SNAND_MAC_OUTL: 0x%x\n", *(RW_SNAND_MAC_OUTL));
	MSG(INFO, "RW_SNAND_MAC_INL: 0x%x\n", *(RW_SNAND_MAC_INL));

	MSG(INFO, "RW_SNAND_RD_CTL1: 0x%x\n", *(RW_SNAND_RD_CTL1));
	MSG(INFO, "RW_SNAND_RD_CTL2: 0x%x\n", *(RW_SNAND_RD_CTL2));
	MSG(INFO, "RW_SNAND_RD_CTL3: 0x%x\n", *(RW_SNAND_RD_CTL3));

	MSG(INFO, "RW_SNAND_GF_CTL1: 0x%x\n", *(RW_SNAND_GF_CTL1));
	MSG(INFO, "RW_SNAND_GF_CTL3: 0x%x\n", *(RW_SNAND_GF_CTL3));

	MSG(INFO, "RW_SNAND_PG_CTL1: 0x%x\n", *(RW_SNAND_PG_CTL1));
	MSG(INFO, "RW_SNAND_PG_CTL2: 0x%x\n", *(RW_SNAND_PG_CTL2));
	MSG(INFO, "RW_SNAND_PG_CTL3: 0x%x\n", *(RW_SNAND_PG_CTL3));

	MSG(INFO, "RW_SNAND_ER_CTL: 0x%x\n", *(RW_SNAND_ER_CTL));
	MSG(INFO, "RW_SNAND_ER_CTL2: 0x%x\n", *(RW_SNAND_ER_CTL2));

	MSG(INFO, "RW_SNAND_MISC_CTL: 0x%x\n", *(RW_SNAND_MISC_CTL));
	MSG(INFO, "RW_SNAND_MISC_CTL2: 0x%x\n", *(RW_SNAND_MISC_CTL2));

	MSG(INFO, "RW_SNAND_DLY_CTL1: 0x%x\n", *(RW_SNAND_DLY_CTL1));
	MSG(INFO, "RW_SNAND_DLY_CTL2: 0x%x\n", *(RW_SNAND_DLY_CTL2));
	MSG(INFO, "RW_SNAND_DLY_CTL3: 0x%x\n", *(RW_SNAND_DLY_CTL3));
	MSG(INFO, "RW_SNAND_DLY_CTL4: 0x%x\n", *(RW_SNAND_DLY_CTL4));

	MSG(INFO, "RW_SNAND_STA_CTL1: 0x%x\n", *(RW_SNAND_STA_CTL1));

	MSG(INFO, "RW_SNAND_CNFG: 0x%x\n", *(RW_SNAND_CNFG));
}


int nand_exec_read_page_hw(struct nand_chip *nand, u32 u4RowAddr, u32 u4PageSize, u8 * pPageBuf, u8 * pFDMBuf)
{
    bool bRet = 1;
    u32 u4SecNum = u4PageSize >> 9;
    u32 i;
    u8  * p_buf_local;
    #ifdef _SNAND_DEBUG
    u32 err_pos;
    #endif

    snand_rs_lk_reconfig_nfiecc(u4RowAddr);

    if (snand_rs_lk_if_require_split())
    {
        u4SecNum--;
    }

    #ifdef _SNAND_DEBUG
    MSG(INFO, "[LK-SNAND][%s] r:%d, b:%d, s:1, part:%d, spare:%d\n", __FUNCTION__, u4RowAddr, u4RowAddr / 64, g_snand_rs_cur_part, g_snand_spare_per_sector);
    #endif

    if (0 != g_snand_rs_ecc_bit)
    {
        if (snand_ready_for_read(nand, u4RowAddr, 0, u4SecNum, pPageBuf, 1, 1, 1))   // bFull = 1 before
        {
            if (!snand_read_page_data(pPageBuf, u4SecNum, 1))
            {
                #ifdef _SNAND_DEBUG
                err_pos = 10;
                #endif

                bRet = 0;
            }

            if (!snand_check_dececc_done(u4SecNum))
            {
                #ifdef _SNAND_DEBUG
                err_pos = 11;
                #endif

                bRet = 0;
            }

            snand_read_fdm_data(pFDMBuf, u4SecNum);

            if (!snand_check_bch_error(pPageBuf, pFDMBuf, u4SecNum - 1, u4RowAddr))
            {
                #ifdef _SNAND_DEBUG
                err_pos = 12;
                #endif

                g_i4ErrNum++;
            }

            snand_stop_read();
        }
        else
        {
            bRet = 0;
        }
    }
    else    // use Device ECC
    {
        // use internal buffer for read with device ECC on, then transform the format of data and spare

        if (snand_ready_for_read(nand, u4RowAddr, 0, u4SecNum, g_snand_lk_temp, 0, 0, 1))   // MTK ECC off, AUTO FMT off, AHB on
        {
            if (!snand_read_page_data(g_snand_lk_temp, u4SecNum, 1))
            {
                #ifdef _SNAND_DEBUG
                err_pos = 13;
                #endif

                bRet = 0;
            }

            // copy data
            memcpy((void *)pPageBuf, (void *)g_snand_lk_temp, NAND_SECTOR_SIZE * u4SecNum);

            // copy spare
            if (!snand_read_fdm_data_devecc(pFDMBuf, g_snand_lk_temp, u4SecNum))
            {
                #ifdef _SNAND_DEBUG
                err_pos = 14;
                #endif

                bRet = 0;
            }

            snand_stop_read();
        }
        else
        {
            bRet = 0;
        }
    }

    if (bRet && snand_rs_lk_if_require_split())
    {
        // read part II

        u4SecNum++;

        // note: use local temp buffer to read part 2
        if (!snand_read_page_part2(u4RowAddr, u4SecNum, g_snand_lk_temp))
        {
            bRet = 0;
        }

        // copy data

        p_buf_local = pPageBuf + NAND_SECTOR_SIZE * (u4SecNum - 1);

        for (i = 0; i < NAND_SECTOR_SIZE / sizeof(u32); i++)
        {
            ((u32 *)p_buf_local)[i] = ((u32 *)g_snand_lk_temp)[i];
        }

        // copy FDM data

        p_buf_local = pFDMBuf + NAND_FDM_PER_SECTOR * (u4SecNum - 1);

        for (i = 0; i < NAND_FDM_PER_SECTOR / sizeof(u32); i++)
        {
            ((u32 *)p_buf_local)[i] = ((u32 *)g_snand_lk_temp)[i + (NAND_SECTOR_SIZE / sizeof(u32))];
        }
    }

    snand_dev_enable_spiq(0);

    #ifdef _SNAND_DEBUG
    if (0 == bRet)
    {
        MSG(INFO, "err_pos: %d\n", err_pos);
        snand_dump_reg();
        while (1);
    }
    #endif

    return bRet;
}

static bool snand_exec_read_page(struct nand_chip *nand, u32 u4RowAddr, u32 u4PageSize, u8 * pPageBuf, u8 * pFDMBuf)
{
    u32 page_per_block = 1 << (nand->phys_erase_shift - nand->page_shift);
    int block = u4RowAddr / page_per_block;
    int page_in_block = u4RowAddr % page_per_block;
    int mapped_block;
    int i, start, len, offset;
    struct nand_oobfree *free;
    u8 oob[0x80];

    mapped_block = get_mapping_block_index(block);

    if (!nand_exec_read_page_hw(nand, (mapped_block * page_per_block + page_in_block), u4PageSize, pPageBuf, oob))
    {
        return 0;
    }

    offset = 0;
    free = nand->ecclayout->oobfree;

    for (i = 0;  i < MTD_MAX_OOBFREE_ENTRIES&&free[i].length; i++)
    {
        start = free[i].offset;
        len = free[i].length;
        memcpy(pFDMBuf + offset, oob + start, len);
        offset += len;
    }

    return 0;
}

static bool snand_dev_program_execute(u32 page)
{
    u32 cmd;
    u8  reg8;
    bool bRet = 1;

    // 3. Program Execute

    cmd = snand_gen_c1a3(SNAND_CMD_PROGRAM_EXECUTE, page);

    snand_dev_command(cmd, 4);

    SNAND_WriteReg(RW_SNAND_GPRAM_DATA, (SNAND_CMD_GET_FEATURES | (SNAND_CMD_FEATURES_STATUS << 8)));
    SNAND_WriteReg(RW_SNAND_MAC_OUTL, 2);
    SNAND_WriteReg(RW_SNAND_MAC_INL , 1);

    while (1)
    {
        snand_dev_mac_op(SPI);

        reg8 = *(((P_U8)RW_SNAND_GPRAM_DATA + 2));

        if (0 == (reg8 & SNAND_STATUS_OIP)) // ready
        {
            if (0 != (reg8 & SNAND_STATUS_PROGRAM_FAIL)) // ready but having fail report from device
            {
                printf("[snand] snand_dev_program_execute: prog failed\n");	// Stanley Chu

                bRet = 0;
            }

            break;
        }
    }

    return bRet;
}

bool snand_is_vendor_reserved_blocks(u32 row_addr)
{
    u32 page_per_block = g_nand_chip.erasesize / g_nand_chip.page_size;
    u32 target_block = row_addr / page_per_block;

    if (devinfo.advancedmode & SNAND_ADV_VENDOR_RESERVED_BLOCKS)
    {
        if (target_block >= 2045 && target_block <= 2048)
        {
            return 1;
        }
    }

    return 0;
}

static bool snand_exec_write_page(struct nand_chip *nand, u32 u4RowAddr, u32 u4PageSize, u8 * pPageBuf, u8 * pFDMBuf)
{
    bool bRet = 1;
    u32 u4SecNum = u4PageSize >> 9;
    #ifdef _SNAND_DEBUG
    u32 err_pos = 0;
    #endif

    if (1 == snand_is_vendor_reserved_blocks(u4RowAddr))
    {
        return 0;
    }

    snand_rs_lk_reconfig_nfiecc(u4RowAddr);

    #ifdef _SNAND_DEBUG
    printf("[LK-SNAND][%s] p:%d, b:%d, s:4, part:%d, spare:%d, ecc:%d\n", __FUNCTION__, u4RowAddr, u4RowAddr / 64, g_snand_rs_cur_part, g_snand_spare_per_sector, g_snand_rs_ecc_bit);
    #endif

    if (0 != g_snand_rs_ecc_bit)
    {
        if (snand_ready_for_write(nand, u4RowAddr, pPageBuf, 1, 1, 1))
        {
            snand_write_fdm_data(pFDMBuf, u4SecNum);

            if (!snand_write_page_data(pPageBuf, u4PageSize, 1))
            {
                #ifdef _SNAND_DEBUG
                err_pos = 1;
                #endif

                bRet = 0;
            }

            if (!snand_check_RW_count(u4PageSize))
            {
                #ifdef _SNAND_DEBUG
                err_pos = 2;
                #endif

                bRet = 0;
            }

            snand_stop_write();

            snand_dev_program_execute(u4RowAddr);
        }
        else
        {
            #ifdef _SNAND_DEBUG
            err_pos = 3;
            #endif

            bRet = 0;
        }
    }
    else    // use Device ECC
    {
        // use internal buffer for program with device ECC on

        // prepare page data
        memcpy((void *)g_snand_lk_temp, (void *)pPageBuf, devinfo.pagesize);

        // prepare spare data
        snand_write_fdm_data_devecc(&(g_snand_lk_temp[devinfo.pagesize]), pFDMBuf, u4SecNum);

        if (snand_ready_for_write(nand, u4RowAddr, pPageBuf, 0, 0, 1))
        {
            if (!snand_write_page_data(pPageBuf, u4PageSize, 1))
            {
                #ifdef _SNAND_DEBUG
                err_pos = 4;
                #endif

                bRet = 0;
            }

            if (!snand_check_RW_count(u4PageSize))
            {
                #ifdef _SNAND_DEBUG
                err_pos = 5;
                #endif

                bRet = 0;
            }

            snand_stop_write();

            snand_dev_program_execute(u4RowAddr);
        }
        else
        {
            #ifdef _SNAND_DEBUG
            err_pos = 6;
            #endif

            bRet = 0;
        }
    }

    #ifdef _SNAND_DEBUG
    if (0 == bRet)
    {
        printf("[LK-SNAND] ERROR! err_pos: %d\n", err_pos);
        snand_dump_reg();
        while (1);
    }
    #endif

    return bRet;
}

static bool snand_read_oob_raw(struct nand_chip *chip, u32 page_addr, u32 length, u8 * buf)
{
    bool bRet = 1;
    u8 buf_int[SNAND_FDM_DATA_SIZE_PER_SECTOR];

    // this API is called by nand_block_bad_hw() only! nand_block_bad_hw() will only check spare[0], thus we only need to read the 1st sector

    if (length > 32 || length % OOB_AVAIL_PER_SECTOR || !buf)
    {
        #ifdef _SNAND_DEBUG
        printf("[LK-SNAND][%s] invalid parameter, length: %d, buf: %p\n", __FUNCTION__, length, buf);
        #endif

        return 0;
    }

    snand_rs_lk_reconfig_nfiecc(page_addr);

    #ifdef _SNAND_DEBUG
    MSG(INFO, "[LK-SNAND][%s] r:%d, b:%d, s:1, part:%d, spare:%d\n", __FUNCTION__, page_addr, page_addr / 64, g_snand_rs_cur_part, g_snand_spare_per_sector);
    #endif

    // read the 1st sector (including its spare area) with MTK ECC enabled

    if (0 != g_snand_rs_ecc_bit)
    {
        if (snand_ready_for_read(chip, page_addr, 0, 1, g_snand_lk_temp, 1, 1, 1)) // bFull = 1 before
        {
            if (!snand_read_page_data(g_snand_lk_temp, 1, 1))  // only read 1 sector
            {
                bRet = 0;
            }

            if (!snand_check_dececc_done(1))
            {
                bRet = 0;
            }

            snand_read_fdm_data(g_snand_lk_temp + NAND_SECTOR_SIZE, 1);

            if (!snand_check_bch_error(g_snand_lk_temp, g_snand_lk_temp + NAND_SECTOR_SIZE, 1 - 1, page_addr))
            {
                g_i4ErrNum++;
            }

            snand_stop_read();

            // copy spare[0] to oob buffer
            buf[0] = g_snand_lk_temp[NAND_SECTOR_SIZE];
        }
        else
        {
            bRet = 0;
        }
    }
    else    // use Device ECC
    {
        // use internal buffer for read with device ECC on, then transform the format of data and spare
        // TODO: we could read spare data only here

        if (snand_ready_for_read(chip, page_addr, 0, devinfo.pagesize / NAND_SECTOR_SIZE, g_snand_lk_temp, 0, 0, 1))   // MTK ECC off, AUTO FMT off, AHB on
        {
            if (!snand_read_page_data(g_snand_lk_temp, devinfo.pagesize / NAND_SECTOR_SIZE, 1))
            {
                bRet = 0;
            }

            snand_stop_read();

            // copy spare (read only 1 sector's spare is enough)
            if (!snand_read_fdm_data_devecc(buf_int, g_snand_lk_temp, 1))
            {
                bRet = 0;
            }

            buf[0] = buf_int[0];
        }
        else
        {
            bRet = 0;
        }
    }

#if 0   // old code. it use MCU read with ECC disabled! This method has potential data loss issues.
    while (length > 0)
    {
        col_addr = NAND_SECTOR_SIZE + sector * (NAND_SECTOR_SIZE + spare_per_sec);
        if (!snand_ready_for_read(chip, page_addr, col_addr, 0, NULL))
        {
            return 0;
        }
        if (!snand_mcu_read_data(buf, length, 0))
        {
            return 0;
        }
        SNAND_REG_CLN_BITS(NFI_CON_REG16, CON_NFI_BRD);
        sector++;
        length -= OOB_AVAIL_PER_SECTOR;
    }
#endif

    return bRet;
}

bool nand_block_bad_hw(struct nand_chip * nand, u64 offset)
{
    u32 page_per_block = nand->erasesize / nand->page_size;
    u32 page_addr = offset >> nand->page_shift;
    u8 oob_buf[OOB_AVAIL_PER_SECTOR];

    if (1 == snand_is_vendor_reserved_blocks(page_addr))
    {
        printf("Bad block detect at block 0x%x, vendor-reserved\n", page_addr / page_per_block);
        return 1;   // return bad block for reserved blocks
    }

    memset(oob_buf, 0, OOB_AVAIL_PER_SECTOR);

    page_addr &= ~(page_per_block - 1);

    if (!snand_read_oob_raw(nand, page_addr, OOB_AVAIL_PER_SECTOR, oob_buf))
    {
        printf("snand_read_oob_raw return fail\n");
    }

    if (oob_buf[0] != 0xff)
    {
        printf("Bad block detect at block 0x%x, oob_buf[0] is %x\n", page_addr / page_per_block, oob_buf[0]);
        return 1;
    }

    return 0;
}

static bool snand_block_bad(struct nand_chip *nand, u32 page_addr)
{
    u32 page_per_block = 1 << (nand->phys_erase_shift - nand->page_shift);
    int block = page_addr / page_per_block;
    int mapped_block = get_mapping_block_index(block);

    return nand_block_bad_hw(nand, (u64)mapped_block << nand->phys_erase_shift);
}

static bool snand_erase_hw_rowaddr(u32 row_addr)
{
    bool bRet = 1;
    u32  reg;
    u32  polling_times;

    if (1 == snand_is_vendor_reserved_blocks(row_addr))
    {
        return 0;
    }

    #ifdef _SNAND_DEBUG
    printf("[LK-SNAND][%s] p:%d, b:%d\n", __FUNCTION__, row_addr, row_addr / 64);
    #endif

    snand_reset_con();

    // erase address
    SNAND_WriteReg(RW_SNAND_ER_CTL2, row_addr);

    // set loop limit and polling cycles
    reg = (SNAND_LOOP_LIMIT_NO_LIMIT << SNAND_LOOP_LIMIT_OFFSET) | 0x20;
    SNAND_WriteReg(RW_SNAND_GF_CTL3, reg);

    // set latch latency & CS de-select latency (ignored)

    // set erase command
    reg = SNAND_CMD_BLOCK_ERASE << SNAND_ER_CMD_OFFSET;
    SNAND_WriteReg(RW_SNAND_ER_CTL, reg);

    // trigger interrupt waiting
    reg = *(NFI_INTR_EN_REG16);
    reg = INTR_AUTO_BLKER_INTR_EN;
    SNAND_WriteReg(NFI_INTR_EN_REG16, reg);

    // trigger auto erase
    reg = *(RW_SNAND_ER_CTL);
    reg |= SNAND_AUTO_ERASE_TRIGGER;
    SNAND_WriteReg(RW_SNAND_ER_CTL, reg);

    // wait for auto erase finish
    for (polling_times = 1;; polling_times++)
    {
        reg = *(RW_SNAND_STA_CTL1);

        if ((reg & SNAND_AUTO_BLKER) == 0)
        {
            reg = *(RW_SNAND_GF_CTL1);
            reg &= SNAND_GF_STATUS_MASK;

            continue;
        }
        else
        {
            // set 1 then set 0 to clear done flag
            SNAND_WriteReg(RW_SNAND_STA_CTL1, reg);
            reg = reg & ~SNAND_AUTO_BLKER;
            SNAND_WriteReg(RW_SNAND_STA_CTL1, reg);

            // clear trigger bit
            reg = *(RW_SNAND_ER_CTL);
            reg &= ~SNAND_AUTO_ERASE_TRIGGER;
            SNAND_WriteReg(RW_SNAND_ER_CTL, reg);

            reg = *(RW_SNAND_GF_CTL1);
            reg &= SNAND_GF_STATUS_MASK;

            break;
        }
    }


    // check get feature status
    reg = *RW_SNAND_GF_CTL1 & SNAND_GF_STATUS_MASK;

    if (0 != (reg & SNAND_STATUS_ERASE_FAIL))
    {
        bRet = 0;
    }

    return bRet;
}

//not support un-block-aligned write
static int snand_part_write(part_dev_t * dev, uchar * src, u64 dst, int size)
{
    struct nand_chip *nand = (struct nand_chip *)dev->blkdev;
    u8 res;
    u32 u4PageSize = 1 << nand->page_shift;
    u32 u4PageNumPerBlock = 1 << (nand->phys_erase_shift - nand->page_shift);
    u32 u4BlkEnd = (nand->chipsize >> nand->phys_erase_shift) << 1;
    u32 u4BlkAddr = (dst >> nand->phys_erase_shift) << 1;
    u32 u4ColAddr = dst & (u4PageSize - 1);
    u32 u4RowAddr = dst >> nand->page_shift;
    u32 u4RowEnd;
    u32 u4WriteLen = 0;
    int i4Len;
    u32 k = 0;

    for (k = 0; k < sizeof(g_kCMD.au1OOB); k++)
    {
        *(g_kCMD.au1OOB + k) = 0xFF;
    }

    while (((u32)size > u4WriteLen) && (u4BlkAddr < u4BlkEnd))
    {
        if (!u4ColAddr)
        {
            MSG(OPS, "Erase the block of 0x%08x\n", u4BlkAddr);

            snand_erase_hw_rowaddr(u4RowAddr);
        }

        res = snand_block_bad(nand, ((u4BlkAddr >> 1) * u4PageNumPerBlock));

        if (!res)
        {
            u4RowEnd = (u4RowAddr + u4PageNumPerBlock) & (~u4PageNumPerBlock + 1);
            for (; u4RowAddr < u4RowEnd; u4RowAddr++)
            {
                i4Len = min((u32)size - u4WriteLen, u4PageSize - u4ColAddr);
                if (0 >= i4Len)
                {
                    break;
                }
                if ((u4ColAddr == 0) && ((u32)i4Len == u4PageSize))
                {
                    snand_exec_write_page(nand, u4RowAddr, u4PageSize, src + u4WriteLen, g_kCMD.au1OOB);
                }
                else
                {
                    snand_exec_read_page(nand, u4RowAddr, u4PageSize, nand->buffers->databuf, g_kCMD.au1OOB);
                    memcpy(nand->buffers->databuf + u4ColAddr, src + u4WriteLen, i4Len);
                    snand_exec_write_page(nand, u4RowAddr, u4PageSize, nand->buffers->databuf, g_kCMD.au1OOB);
                }
                u4WriteLen += i4Len;
                u4ColAddr = (u4ColAddr + i4Len) & (u4PageSize - 1);
            }
        }
        else
        {
            printf("Detect bad block at block 0x%x\n", u4BlkAddr);
            u4RowAddr += u4PageNumPerBlock;
        }
        u4BlkAddr++;
    }

    return (int)u4WriteLen;

}

static int snand_part_read(part_dev_t * dev, u64 source, uchar * dst, int size)
{
    struct nand_chip *nand = (struct nand_chip *)dev->blkdev;
    uint8_t res;
    u32 u4PageSize = 1 << nand->page_shift;
    u32 u4PageNumPerBlock = 1 << (nand->phys_erase_shift - nand->page_shift);
    u32 u4BlkEnd = (nand->chipsize >> nand->phys_erase_shift);
    u32 u4BlkAddr = (source >> nand->phys_erase_shift);
    u32 u4ColAddr = source & (u4PageSize - 1);
    u32 u4RowAddr = source >> nand->page_shift;
    u32 u4RowEnd;
    u32 u4ReadLen = 0;
    int i4Len;

    while (((u32)size > u4ReadLen) && (u4BlkAddr < u4BlkEnd))
    {
        res = snand_block_bad(nand, (u4BlkAddr * u4PageNumPerBlock));

        if (!res)
        {
            u4RowEnd = (u4RowAddr + u4PageNumPerBlock) & (~u4PageNumPerBlock + 1);
            for (; u4RowAddr < u4RowEnd; u4RowAddr++)
            {
                i4Len = min((u32)size - u4ReadLen, u4PageSize - u4ColAddr);
                if (0 >= i4Len)
                {
                    break;
                }
                if ((u4ColAddr == 0) && ((u32)i4Len == u4PageSize))
                {
                    snand_exec_read_page(nand, u4RowAddr, u4PageSize, dst + u4ReadLen, g_kCMD.au1OOB);
                }
                else
                {
                    snand_exec_read_page(nand, u4RowAddr, u4PageSize, nand->buffers->databuf, g_kCMD.au1OOB);
                    memcpy(dst + u4ReadLen, nand->buffers->databuf + u4ColAddr, i4Len);
                }
                u4ReadLen += i4Len;
                u4ColAddr = (u4ColAddr + i4Len) & (u4PageSize - 1);
            }
        }
        else
        {
            printf("Detect bad block at block 0x%x\n", u4BlkAddr);
            u4RowAddr += u4PageNumPerBlock;
        }
        u4BlkAddr++;
    }
    return (int)u4ReadLen;
}

static void snand_dev_read_id(u8 id[])
{
    u8 cmd = SNAND_CMD_READ_ID;

    snand_dev_command_ext(SPI, &cmd, id, 1, SNAND_MAX_ID + 1);
}

static void snand_command_bp(struct nand_chip *nand_chip, unsigned command, int column, int page_addr)
{
    switch (command)
    {
        case NAND_CMD_RESET:

            snand_reset_con();
            break;

        case NAND_CMD_READ_ID:

            snand_reset_con();
            snand_reset_dev();

            snand_dev_read_id(g_snand_id_data);
            g_snand_read_byte_mode = SNAND_RB_READ_ID;
            g_snand_id_data_idx = 1;

            printf("[SNAND] snand_command_bp(NAND_CMD_READID), ID:%x,%x\n", g_snand_id_data[1], g_snand_id_data[2]);

            break;

        default:
            printf("[ERR] snand_command_bp : unknow command %d\n", command);
            break;
    }
}

static u_char snand_read_byte(void)
{
    /* Check the PIO bit is ready or not */
    unsigned int timeout = TIMEOUT_4;

    if (SNAND_RB_READ_ID == g_snand_read_byte_mode)
    {
        if (g_snand_id_data_idx > SNAND_MAX_ID)
        {
            return 0;
        }
        else
        {
            return g_snand_id_data[g_snand_id_data_idx++];
        }
    }
    else
    {
        WAIT_NFI_PIO_READY(timeout);

        return *(NFI_DATAR_REG32);
    }
}

void lk_nand_irq_handler(unsigned int irq)
{
    u32 inte,sts;

    mt_irq_ack(irq);
    inte = *(NFI_INTR_EN_REG16);
    sts = *(NFI_INTR_REG16);
    //MSG(INT, "[lk_nand_irq_handler]irq %x enable:%x %x\n",irq,inte,sts);
    if(sts & inte)
    {
        //	printf("[lk_nand_irq_handler]send event,\n");
        SNAND_WriteReg(NFI_INTR_EN_REG16, 0);
        SNAND_WriteReg(NFI_INTR_REG16,sts);
        event_signal(&nand_int_event,0);
    }
    return;
}

#define _GPIO_MODE9         ((P_U32)(GPIO_BASE + 0x0390))
#define _GPIO_MASK          (0x0000000F)
#define _SFWP_GPIO_REG      (_GPIO_MODE9)
#define _SFWP_GPIO_OFFSET   (0)
#define _SFWP_GPIO_MODE     (2)
#define _SFOUT_GPIO_REG     (_GPIO_MODE9)
#define _SFOUT_GPIO_OFFSET  (4)
#define _SFOUT_GPIO_MODE    (2)
#define _SFCS0_GPIO_REG     (_GPIO_MODE9)
#define _SFCS0_GPIO_OFFSET  (8)
#define _SFCS0_GPIO_MODE    (2)
#define _SFHOLD_GPIO_REG    (_GPIO_MODE9)
#define _SFHOLD_GPIO_OFFSET (12)
#define _SFHOLD_GPIO_MODE   (2)
#define _SFIN_GPIO_REG      (_GPIO_MODE9)
#define _SFIN_GPIO_OFFSET   (16)
#define _SFIN_GPIO_MODE     (2)
#define _SFCK_GPIO_REG      (_GPIO_MODE9)
#define _SFCK_GPIO_OFFSET   (20)
#define _SFCK_GPIO_MODE     (2)
#define _SFCS1_GPIO_REG     (_GPIO_MODE9)
#define _SFCS1_GPIO_OFFSET  (24)
#define _SFCS1_GPIO_MODE    (2)
#define SNAND_CNFG_ENABLE_SNAND       (1)

#define RW_DRV_CFG          ((volatile P_U32)(0x10015060))
#define DRV_CFG_MC_0_MASK   (0x00000FFF)

static void snand_gpio_init(void)
{
    /*
     * [Pin Map]
     * GPIO72    SFWP_B
     * GPIO73    SFOUT
     * GPIO74    SFCS0
     * GPIO75    SFHOLD
     * GPIO76    SFIN
     * GPIO77    SFCK
     * GPIO78    SFCS1
     *
     * [GPIO MODE 9]
     * MSB LSB   NAME
     *  3   0   GPIO72
     *  7   4   GPIO73
     *  11  8   GPIO74
     *  15  12  GPIO75
     *  19  16  GPIO76
     *  23  20  GPIO77
     *  27  24  GPIO78
     *  31  28  GPIO79
     */

    // Switch to SPI NAND

    *RW_SNAND_CNFG = SNAND_CNFG_ENABLE_SNAND;

    // Config GPIO
    /*
     *    Switch GPIO to SPI-NAND by specific order
     *
     *    If SFCK may change or have glitch, SFCS0 and SFCS1 must keep high. Thus we switch SFCS0 and SFCS1 before SFCK.
     */
    *_SFWP_GPIO_REG = (*_SFWP_GPIO_REG & ~(_GPIO_MASK << _SFWP_GPIO_OFFSET)) | (_SFWP_GPIO_MODE << _SFWP_GPIO_OFFSET);
    snand_wait_us(1);
    *_SFOUT_GPIO_REG = (*_SFOUT_GPIO_REG & ~(_GPIO_MASK << _SFOUT_GPIO_OFFSET)) | (_SFOUT_GPIO_MODE << _SFOUT_GPIO_OFFSET);
    snand_wait_us(1);
    *_SFCS0_GPIO_REG = (*_SFCS0_GPIO_REG & ~(_GPIO_MASK << _SFCS0_GPIO_OFFSET)) | (_SFCS0_GPIO_MODE << _SFCS0_GPIO_OFFSET);
    snand_wait_us(1);
    *_SFCS1_GPIO_REG = (*_SFCS1_GPIO_REG & ~(_GPIO_MASK << _SFCS1_GPIO_OFFSET)) | (_SFCS1_GPIO_MODE << _SFCS1_GPIO_OFFSET);
    snand_wait_us(1);
    *_SFHOLD_GPIO_REG = (*_SFHOLD_GPIO_REG & ~(_GPIO_MASK << _SFHOLD_GPIO_OFFSET)) | (_SFHOLD_GPIO_MODE << _SFHOLD_GPIO_OFFSET);
    snand_wait_us(1);
    *_SFIN_GPIO_REG = (*_SFIN_GPIO_REG & ~(_GPIO_MASK << _SFIN_GPIO_OFFSET)) | (_SFIN_GPIO_MODE << _SFIN_GPIO_OFFSET);
    snand_wait_us(1);
    *_SFCK_GPIO_REG = (*_SFCK_GPIO_REG & ~(_GPIO_MASK << _SFCK_GPIO_OFFSET)) | (_SFCK_GPIO_MODE << _SFCK_GPIO_OFFSET);
    snand_wait_us(1);
}

void snand_dev_unlock_all_blocks(void)
{
    u32 cmd;
    u8  lock;
    u8  lock_new;

    // read original block lock settings
    cmd = SNAND_CMD_GET_FEATURES | (SNAND_CMD_FEATURES_BLOCK_LOCK << 8);
    SNAND_WriteReg(RW_SNAND_GPRAM_DATA, cmd);
    SNAND_WriteReg(RW_SNAND_MAC_OUTL, 2);
    SNAND_WriteReg(RW_SNAND_MAC_INL , 1);

    snand_dev_mac_op(SPI);

    lock = DRV_Reg8(((P_U8)RW_SNAND_GPRAM_DATA + 2));

    lock_new = lock & ~SNAND_BLOCK_LOCK_BITS;

    if (lock != lock_new)
    {
        // write enable
        snand_dev_command(SNAND_CMD_WRITE_ENABLE, 1);

        // set features
        cmd = SNAND_CMD_SET_FEATURES | (SNAND_CMD_FEATURES_BLOCK_LOCK << 8) | (lock_new << 16);
        snand_dev_command(cmd, 3);
    }
}

void snand_dev_turn_off_bbi(void)
{
    u32 cmd;
    u8  reg;
    u8  reg_new;

    // read original block lock settings
    cmd = SNAND_CMD_GET_FEATURES | (SNAND_CMD_FEATURES_OTP << 8);
    SNAND_WriteReg(RW_SNAND_GPRAM_DATA, cmd);
    SNAND_WriteReg(RW_SNAND_MAC_OUTL, 2);
    SNAND_WriteReg(RW_SNAND_MAC_INL , 1);

    snand_dev_mac_op(SPI);

    reg = DRV_Reg8(((P_U8)RW_SNAND_GPRAM_DATA + 2));

    reg_new = reg & ~SNAND_OTP_BBI;

    if (reg != reg_new)
    {
        // write enable
        snand_dev_command(SNAND_CMD_WRITE_ENABLE, 1);

        // set features
        cmd = SNAND_CMD_SET_FEATURES | (SNAND_CMD_FEATURES_OTP << 8) | (reg_new << 16);
        snand_dev_command(cmd, 3);
    }
}

int nand_init_device(struct nand_chip *nand)
{
    int index;
    u8 id[SNAND_MAX_ID] = {0};
    u32 spare_bit;
    u32 spare_per_sec;
    u32 ecc_bit;

    memset(&devinfo, 0, sizeof(devinfo));
    g_bInitDone = 0;
    g_kCMD.u4OOBRowAddr = (u32) - 1;

    snand_gpio_init();

#if defined(CONFIG_EARLY_LINUX_PORTING)		// FPGA NAND is placed at CS1
    SNAND_WriteReg(NFI_CSEL_REG16, 1);
#else
    SNAND_WriteReg(NFI_CSEL_REG16, NFI_DEFAULT_CS);
#endif

    //SNAND_WriteReg(NFI_ACCCON_REG32, NFI_DEFAULT_ACCESS_TIMING);
    SNAND_WriteReg(NFI_CNFG_REG16, 0);
    SNAND_WriteReg(NFI_PAGEFMT_REG16, 0);
    snand_reset_con();

    nand->nand_ecc_mode = NAND_ECC_HW;
    snand_command_bp(&g_nand_chip, NAND_CMD_READ_ID, 0, 0);

    MSG(INFO, "NAND ID: ");

    for (index = 0; index < SNAND_MAX_ID; index++)
    {
        id[index] = snand_read_byte();
        MSG(INFO, " %x", id[index]);
    }

    MSG(INFO, "\n ");

    if (!snand_get_device_info(id, &devinfo))
    {
        MSG(ERR, "NAND unsupport\n");
        return -1;
    }

    nand->name = devinfo.devicename;
    nand->chipsize = ((u64)devinfo.totalsize) << 20;
    nand->erasesize = devinfo.blocksize << 10;
    nand->phys_erase_shift = uffs(nand->erasesize) - 1;
    nand->page_size = devinfo.pagesize;
    nand->writesize = devinfo.pagesize;
    nand->page_shift = uffs(nand->page_size) - 1;
    nand->oobblock = nand->page_size;
    nand->bus16 = IO_WIDTH_4;
    nand->id_length = devinfo.id_length;
    nand->sector_size = NAND_SECTOR_SIZE;
    nand->sector_shift = 9;

    for (index = 0; index < devinfo.id_length; index++)
    {
        nand->id[index] = id[index];
    }

    // configure SNF timing
    *RW_SNAND_DLY_CTL1 = devinfo.SNF_DLY_CTL1;
    *RW_SNAND_DLY_CTL2 = devinfo.SNF_DLY_CTL2;
    *RW_SNAND_DLY_CTL3 = devinfo.SNF_DLY_CTL3;
    *RW_SNAND_DLY_CTL4 = devinfo.SNF_DLY_CTL4;
    *RW_SNAND_MISC_CTL = devinfo.SNF_MISC_CTL;

    // set inverse clk & latch latency
    *RW_SNAND_MISC_CTL &= ~SNAND_CLK_INVERSE;       // disable inverse clock and 1 T delay
    *RW_SNAND_MISC_CTL &= ~SNAND_LATCH_LAT_MASK;    // set latency to 0T delay
    *RW_SNAND_MISC_CTL |= SNAND_SAMPLE_CLK_INVERSE; // enable sample clock inverse
    *RW_SNAND_MISC_CTL |= SNAND_4FIFO_EN;

    // configure driving
    *RW_DRV_CFG = (*RW_DRV_CFG & ~DRV_CFG_MC_0_MASK) | ((devinfo.SNF_DRIVING << 9) | (devinfo.SNF_DRIVING << 6) | (devinfo.SNF_DRIVING << 3) | devinfo.SNF_DRIVING);

    spare_per_sec = devinfo.sparesize >> (nand->page_shift - 9);

    //MSG(INFO, "devinfo.sparesize:%d, nand->page_shift:%d, spare_per_sec:%d", devinfo.sparesize, nand->page_shift, spare_per_sec);	// Stanley Chu

    if(spare_per_sec>=32)
    {
        spare_bit = PAGEFMT_SPARE_32;
        ecc_bit = 12;
        spare_per_sec = 32;
    }
    else if(spare_per_sec>=28)
    {
        spare_bit = PAGEFMT_SPARE_28;
        ecc_bit = 8;
        spare_per_sec = 28;
    }
    else if(spare_per_sec>=27)
    {
        spare_bit = PAGEFMT_SPARE_27;
        ecc_bit = 8;
        spare_per_sec = 27;
    }
    else if(spare_per_sec>=26)
    {
        spare_bit = PAGEFMT_SPARE_26;
        ecc_bit = 8;
        spare_per_sec = 26;
    }
    else if(spare_per_sec>=16)
    {
        spare_bit = PAGEFMT_SPARE_16;
        ecc_bit = 4;
        spare_per_sec = 16;
    }
    else
    {
        printf("[NAND]: NFI not support oobsize: %x\n", spare_per_sec);
        while(1);
        return -1;
    }

    g_snand_spare_per_sector = spare_per_sec;
    g_snand_rs_ecc_bit_second_part = ecc_bit;

    devinfo.sparesize = spare_per_sec<<(nand->page_shift-9);
    MSG(INFO, "[NAND]nand eccbit %d , sparesize %d\n",ecc_bit,devinfo.sparesize);

    nand->oobsize = devinfo.sparesize;

    nand->buffers = malloc(sizeof(struct nand_buffers));

    if (nand->oobblock == 4096)
    {
        SNAND_REG_SET_BITS(NFI_PAGEFMT_REG16, (spare_bit << PAGEFMT_SPARE_SHIFT) | PAGEFMT_4K);
        nand->ecclayout = &nand_oob_128;
    }
    else if (nand->oobblock == 2048)
    {
        SNAND_REG_SET_BITS(NFI_PAGEFMT_REG16, (spare_bit << PAGEFMT_SPARE_SHIFT) | PAGEFMT_2K);
        nand->ecclayout = &nand_oob_64;
    }
    else if (nand->oobblock == 512)
    {
        SNAND_REG_SET_BITS(NFI_PAGEFMT_REG16, (spare_bit << PAGEFMT_SPARE_SHIFT) | PAGEFMT_512);
        nand->ecclayout = &nand_oob_16;
    }

    SNAND_REG_SET_BITS(NFI_PAGEFMT_REG16, PAGEFMT_SEC_SEL_512);

    g_snand_rs_spare_per_sector_second_part_nfi = (spare_bit << PAGEFMT_SPARE_SHIFT);

    if (nand->nand_ecc_mode == NAND_ECC_HW)
    {
        SNAND_REG_SET_BITS(NFI_CNFG_REG16, CNFG_HW_ECC_EN);
        snand_ecc_config(ecc_bit);
        snand_configure_fdm(NAND_FDM_PER_SECTOR);
    }

    *(NFI_INTR_REG16);
    SNAND_WriteReg(NFI_INTR_EN_REG16, 0);

    if (en_interrupt)
    {
        event_init(&nand_int_event,0,EVENT_FLAG_AUTOUNSIGNAL);
        mt_irq_set_sens(MT_NFI_IRQ_ID, MT65xx_EDGE_SENSITIVE);
        mt_irq_set_polarity(MT_NFI_IRQ_ID, MT65xx_POLARITY_LOW);
        mt_irq_unmask(MT_NFI_IRQ_ID);
    }

    g_nand_size = nand->chipsize;
    nand->chipsize -= nand->erasesize * (BMT_POOL_SIZE);

    // init read split boundary
    //g_snand_rs_num_page = SNAND_RS_BOUNDARY_BLOCK * (nand->erasesize / nand->page_size);
    g_snand_rs_num_page = SNAND_RS_BOUNDARY_KB * 1024 / nand->page_size;

    snand_dev_unlock_all_blocks();  // for fastboot erase and write

    snand_dev_turn_off_bbi();       // for fastboot erase and write

    // config read empty threshold for MTK ECC (MT6571 only)
    *NFI_EMPTY_THRESHOLD = 1;

    g_bInitDone = 1;

    if (!g_bmt)
    {
        if (!(g_bmt = init_bmt(nand, BMT_POOL_SIZE)))
        {
            MSG(INIT, "Error: init bmt failed\n");
            return -1;
        }
    }

    return 0;

}

void nand_init(void)
{
    static part_dev_t dev;

    if (!nand_init_device(&g_nand_chip))
    {
        struct nand_chip *t_nand = &g_nand_chip;
        printf("NAND init done in LK\n");
        total_size = t_nand->chipsize - t_nand->erasesize * (PMT_POOL_SIZE);
        dev.id = 0;
        dev.init = 1;
        dev.blkdev = (block_dev_desc_t *) t_nand;
        dev.read = snand_part_read;
        dev.write = snand_part_write;
        mt_part_register_device(&dev);
        printf("NAND register done in LK\n");
        return;
    }
    else
    {
        printf("NAND init fail in LK\n");
    }

}

void nand_driver_test(void)
{
#ifdef NAND_LK_TEST
    u32 test_len=4096;
    long len;
    int fail=0;
    u32 index = 0;
    part_dev_t *dev = mt_part_get_device();
    part_t *part = mt_part_get_partition(PART_LOGO);
    unsigned long start_addr = part->startblk * BLK_SIZE;
    u8 *original = malloc(test_len);
    u8 *source = malloc(test_len);
    u8 *readback = malloc(test_len);

    for (index = 0; index < test_len; index++)
    {
        source[index] = index % 16;
    }
    memset(original, 0x0a, test_len);
    memset(readback, 0x0b, test_len);
    printf("~~~~~~~~~nand driver test in lk~~~~~~~~~~~~~~\n");
    len = dev->read(dev, start_addr, (uchar *) original, test_len);
    if (len != test_len)
    {
        printf("read original fail %d\n", len);
    }
    printf("oringinal data:");
    for (index = 0; index < 300; index++)
    {
        printf(" %x", original[index]);
    }
    printf("\n");
    len = dev->write(dev, (uchar *) source, start_addr, test_len);
    if (len != test_len)
    {
        printf("write source fail %d\n", len);
    }
    len = dev->read(dev, start_addr, (uchar *) readback,test_len);
    if (len != test_len)
    {
        printf("read back fail %d\n", len);
    }
    printf("readback data:");
    for (index = 0; index < 300; index++)
    {
        printf(" %x", readback[index]);
    }
    printf("\n");
    for (index = 0; index < test_len; index++)
    {
        if (source[index] != readback[index])
        {
            printf("compare fail %d\n", index);
            fail=1;
            break;
        }
    }
    if(fail==0)
    {
        printf("compare success!\n");
    }
    len = dev->write(dev, (uchar *) original, start_addr, test_len);
    if (len != test_len)
    {
        printf("write back fail %d\n", len);
    }
    else
    {
        printf("recovery success\n");
    }
    memset(original,0xd,test_len);
    len = dev->read(dev, start_addr, (uchar *) original, test_len);
    if (len != test_len)
    {
        printf("read original fail %d\n", len);
    }
    printf("read back oringinal data:");
    for (index = 0; index < 300; index++)
    {
        printf(" %x", original[index]);
    }
    printf("\n");
    printf("~~~~~~~~~nand driver test in lk~~~~~~~~~~~~~~\n");
    free(original);
    free(source);
    free(readback);
#endif
}

/******************** ***/
/*    support for fast boot    */
/***********************/
int nand_erase(u64 offset, u64 size)
{
    u32 img_size = (u32)size;
    u32 tblksz;
    u64 cur_offset;
    u32 block_size = g_nand_chip.erasesize;

    // do block alignment check
    if ((u32)offset % block_size != 0)
    {
        printf("offset must be block alignment (0x%x)\n", block_size);
        return -1;
    }

    // calculate block number of this image
    if ((img_size % block_size) == 0)
    {
        tblksz = img_size / block_size;
    }
    else
    {
        tblksz = (img_size / block_size) + 1;
    }

    printf ("[ERASE] image size = 0x%x\n", img_size);
    printf ("[ERASE] the number of nand block of this image = %d\n", tblksz);

    // erase nand block
    cur_offset = (u64)offset;
    while (tblksz != 0)
    {
        if (__nand_erase(cur_offset) == 0)
        {
            printf("[ERASE] erase 0x%x fail\n",cur_offset);
            mark_block_bad (cur_offset);

        }
        cur_offset += block_size;

        tblksz--;

        if (tblksz != 0 && cur_offset >= total_size)
        {
            printf("[ERASE] cur offset (0x%x) exceeds erase limit address (0x%x)\n", cur_offset, total_size);
            return 0;
        }
    }


    return 0;

}
bool __nand_erase (u64 logical_addr)
{
    int block = logical_addr / g_nand_chip.erasesize;
    int mapped_block = get_mapping_block_index(block);

    if (!nand_erase_hw(mapped_block * g_nand_chip.erasesize))
    {
        printf("erase block 0x%x failed\n", mapped_block);
        if(update_bmt((u64)mapped_block * g_nand_chip.erasesize, UPDATE_ERASE_FAIL, NULL, NULL))
        {
            printf("erase block fail and update bmt sucess\n");
            return 1;
        }
        else
        {
            printf("erase block 0x%x failed but update bmt fail\n",mapped_block);
            return 0;
        }
    }

    return 1;
}
static int erase_fail_test = 0;
bool nand_erase_hw (u64 offset)
{
    bool bRet = 1;
    u32 page_addr = offset / g_nand_chip.oobblock;

    if (nand_block_bad_hw(&g_nand_chip,offset))
    {
        return 0;
    }

    if (erase_fail_test)
    {
        erase_fail_test = 0;
        return 0;
    }

    bRet = snand_erase_hw_rowaddr(page_addr);

    return bRet;
}

bool mark_block_bad (u64 logical_addr)
{
    int block = logical_addr / g_nand_chip.erasesize;
    int mapped_block = get_mapping_block_index(block);

    return mark_block_bad_hw((u64)mapped_block * g_nand_chip.erasesize);
}

bool mark_block_bad_hw(u64 offset)
{
    u32 index;
    unsigned char buf[4096];
    unsigned char spare_buf[64];
    u32 page_addr = offset / g_nand_chip.oobblock;
    u32 u4SecNum = g_nand_chip.oobblock >> 9;
    u32 i;
    int page_num = (g_nand_chip.erasesize / g_nand_chip.oobblock);

    memset(buf,0xFF,4096);

    for (index = 0; index < 64; index++)
    {
        *(spare_buf + index) = 0xFF;
    }

    for (index = 8, i = 0; i < u4SecNum; i++)
    {
        spare_buf[i * index] = 0x0;
    }

    page_addr &= ~(page_num - 1);
    MSG (INFO, "Mark bad block at 0x%x\n", page_addr);

    return snand_exec_write_page(&g_nand_chip, page_addr, g_nand_chip.oobblock, (u8 *)buf,(u8 *)spare_buf);
}
int nand_write_page_hw(u32 page, u8 *dat, u8 *oob)
{
    int i, j, start, len;
    bool empty = 1;
    u8 oob_checksum = 0;
    for (i = 0; i < MTD_MAX_OOBFREE_ENTRIES && g_nand_chip.ecclayout->oobfree[i].length; i++)
    {
        /* Set the reserved bytes to 0xff */
        start = g_nand_chip.ecclayout->oobfree[i].offset;
        len = g_nand_chip.ecclayout->oobfree[i].length;
        for (j = 0; j < len; j++)
        {
            oob_checksum ^= oob[start + j];
            if (oob[start + j] != 0xFF)
            {
                empty = 0;
            }
        }
    }

    if (!empty)
    {
        oob[g_nand_chip.ecclayout->oobfree[i-1].offset + g_nand_chip.ecclayout->oobfree[i-1].length] = oob_checksum;
    }

    return snand_exec_write_page(&g_nand_chip, page, g_nand_chip.oobblock, (u8 *)dat,(u8 *)oob);

}
int nand_write_page_hwecc (u64 logical_addr, char *buf, char *oob_buf)
{
    u32 page_size = g_nand_chip.oobblock;
    u32 block_size = g_nand_chip.erasesize;
    u32 block = logical_addr / block_size;
    u32 mapped_block = get_mapping_block_index(block);
    u32 pages_per_blk = (block_size/page_size);
    u32 page_in_block = (logical_addr/page_size)%pages_per_blk;
    u32 i;
    int start, len, offset;

    for (i = 0; i < sizeof(g_spare_buf); i++)
    {
        *(g_spare_buf + i) = 0xFF;
    }

    offset = 0;

    if(oob_buf != NULL)
    {
        for (i = 0; i < MTD_MAX_OOBFREE_ENTRIES && g_nand_chip.ecclayout->oobfree[i].length; i++)
        {
            /* Set the reserved bytes to 0xff */
            start = g_nand_chip.ecclayout->oobfree[i].offset;
            len = g_nand_chip.ecclayout->oobfree[i].length;
            memcpy ((g_spare_buf + start), (oob_buf + offset), len);
            offset += len;
        }
    }

    // write bad index into oob
    if (mapped_block != block)
    {
        // MSG(INIT, "page: 0x%x\n", page_in_block);
        set_bad_index_to_oob(g_spare_buf, block);
    }
    else
    {
        set_bad_index_to_oob(g_spare_buf, FAKE_INDEX);
    }

    if (!nand_write_page_hw(page_in_block + mapped_block * pages_per_blk,
                            (u8 *)buf, g_spare_buf))
    {
        MSG(INIT, "write fail happened @ block 0x%x, page 0x%x\n", mapped_block, page_in_block);
        return update_bmt( (u64)(page_in_block + mapped_block * pages_per_blk) * g_nand_chip.oobblock,
                           UPDATE_WRITE_FAIL, (u8 *)buf, g_spare_buf);
    }

    return 1;
}

int nand_write_img(u32 addr, void *data, u32 img_sz,u32 partition_size,int img_type)
{
    unsigned int page_size = g_nand_chip.oobblock;
    unsigned int img_spare_size = 0,write_size;
    unsigned int block_size = g_nand_chip.erasesize;
    unsigned int partition_end = addr + partition_size;
    bool ret;
    unsigned int b_lastpage = 0;
    printf("[nand_wite_img]write to addr %x,img size %x\n",addr,img_sz);
    if(addr % block_size || partition_size % block_size)
    {
        printf("[nand_write_img]partition address or partition size is not block size alignment\n");
        return -1;
    }
    if(img_sz > partition_size)
    {
        printf("[nand_write_img]img size %x exceed partition size\n",img_sz);
        return -1;
    }
    if(page_size == 4096)
    {
        img_spare_size = 128;
    }
    else if(page_size == 2048)
    {
        img_spare_size = 64;
    }

    if(img_type == YFFS2_IMG)
    {
        write_size = page_size + img_spare_size;

        if(img_sz % write_size)
        {
            printf("[nand_write_img]img size is not w_size %d alignment\n",write_size);
            return -1;
        }
    }
    else
    {
        write_size = page_size;
        /*	if(img_sz % write_size){
        		printf("[nand_write_img]img size is not w_size %d alignment\n",write_size);
        		return -1;
        	}*/
    }

    while(img_sz>0)
    {

        if((addr+img_sz)>partition_end)
        {
            printf("[nand_wite_img]write to addr %x,img size %x exceed parition size,may be so many bad blocks\n",addr,img_sz);
            return -1;
        }

        /*1. need to erase before write*/
        if((addr % block_size)==0)
        {
            if (__nand_erase(addr) == 0)
            {
                printf("[ERASE] erase 0x%x fail\n",addr);
                mark_block_bad (addr);
                addr += block_size;
                continue;  //erase fail, skip this block
            }
        }
        /*2. write page*/
        if((img_sz < write_size))
        {
            b_lastpage = 1;
            memset(g_data_buf,0xff,write_size);
            memcpy(g_data_buf,data,img_sz);
            if(img_type == YFFS2_IMG)
            {
                ret = nand_write_page_hwecc(addr, (char *)g_data_buf, (char *)g_data_buf + page_size);
            }
            else
            {
                if((img_type == UBIFS_IMG)&& (check_data_empty((void *)g_data_buf,page_size)))
                {
                    printf("[nand_write_img]skip empty page\n");
                    ret = 1;
                }
                else
                {
                    ret = nand_write_page_hwecc(addr, (char *)g_data_buf, NULL);
                }
            }
        }
        else
        {
            if(img_type == YFFS2_IMG)
            {
                ret = nand_write_page_hwecc(addr,data,data+page_size);
            }
            else
            {
                if((img_type == UBIFS_IMG)&& (check_data_empty((void *)data,page_size)))
                {
                    printf("[nand_write_img]skip empty page\n");
                    ret = 1;
                }
                else
                {
                    ret = nand_write_page_hwecc(addr,data,NULL);
                }
            }
        }
        if(ret == 0)
        {
            printf("[nand_write_img]write fail at % 0x%x\n",addr);
            if (__nand_erase(addr) == 0)
            {
                printf("[ERASE] erase 0x%x fail\n",addr);
                mark_block_bad (addr);
            }
            data -= ((addr%block_size)/page_size)*write_size;
            img_sz += ((addr%block_size)/page_size)*write_size;
            addr += block_size;
            continue;  // write fail, try  to write the next block
        }
        if(b_lastpage)
        {
            data += img_sz;
            img_sz = 0 ;
            addr += page_size;
        }
        else
        {
            data += write_size;
            img_sz -= write_size;
            addr += page_size;
        }
    }
    /*3. erase any block remained in partition*/
    addr = ((addr+block_size-1)/block_size)*block_size;

    nand_erase((u64)addr,(u64)(partition_end - addr));

    return 0;
}

int nand_write_img_ex(u32 addr, void *data, u32 length,u32 total_size, u32 *next_offset, u32 partition_start,u32 partition_size, int img_type)
{
    unsigned int page_size = g_nand_chip.oobblock;
    unsigned int img_spare_size = 0,write_size;
    unsigned int block_size = g_nand_chip.erasesize;
    unsigned int partition_end = partition_start + partition_size;
    unsigned int last_chunk = 0;
    unsigned int left_size = 0;
    bool ret;
    u32 last_addr = addr;
    u32 dst_block = 0;
    printf("[nand_write_img_ex]write to addr %x,img size %x, img_type %d\n",addr,length,img_type);
    if(partition_start % block_size || partition_size % block_size)
    {
        printf("[nand_write_img_ex]partition address or partition size is not block size alignment\n");
        return -1;
    }
    if(length > partition_size)
    {
        printf("[nand_write_img_ex]img size %x exceed partition size\n",length);
        return -1;
    }


    if(page_size == 4096)
    {
        img_spare_size = 128;
    }
    else if(page_size == 2048)
    {
        img_spare_size = 64;
    }
    if(last_addr % page_size)
    {
        printf("[nand_write_img_ex]write addr is not page_size %d alignment\n",page_size);
        return -1;
    }
    if(img_type == YFFS2_IMG)
    {
        write_size = page_size + img_spare_size;
        if(total_size % write_size)
        {
            printf("[nand_write_img_ex]total image size %d is not w_size %d alignment\n",total_size,write_size);
            return -1;
        }
    }
    else
    {
        write_size = page_size;
    }
    if(addr == partition_start)
    {
        printf("[nand_write_img_ex]first chunk\n");
        download_size = 0;
        memset(g_data_buf,0xff,write_size);
    }
    if((length + download_size) >= total_size)
    {
        printf("[nand_write_img_ex]last chunk\n");
        last_chunk = 1;
    }

    left_size = (download_size % write_size);

    while(length>0)
    {

        if((addr+length)>partition_end)
        {
            printf("[nand_write_img_ex]write to addr %x,img size %x exceed parition size,may be so many bad blocks\n",addr,length);
            return -1;
        }

        /*1. need to erase before write*/
        if((addr % block_size)==0)
        {
            if (__nand_erase(addr) == 0)
            {
                printf("[ERASE] erase 0x%x fail\n",addr);
                mark_block_bad (addr);
                addr += block_size;
                continue;  //erase fail, skip this block
            }
        }
        if((length < write_size)&&(!left_size))
        {
            memset(g_data_buf,0xff,write_size);
            memcpy(g_data_buf,data,length);

            if(!last_chunk)
            {
                download_size += length;
                break;
            }
        }
        else if(left_size)
        {
            memcpy(&g_data_buf[left_size],data,write_size-left_size);

        }
        else
        {
            memcpy(g_data_buf,data,write_size);
        }

        /*2. write page*/

        if(img_type == YFFS2_IMG)
        {
            ret = nand_write_page_hwecc(addr, (char *)g_data_buf, (char *)(g_data_buf + page_size));
        }
        else
        {
            if((img_type == UBIFS_IMG)&& (check_data_empty((void *)g_data_buf,page_size)))
            {
                printf("[nand_write_img_ex]skip empty page\n");
                ret = 1;
            }
            else
            {
                ret = nand_write_page_hwecc(addr, (char *)g_data_buf, NULL);
            }
        }
        /*need to check?*/
        if(ret == 0)
        {
            printf("[nand_write_img_ex]write fail at % 0x%x\n",addr);
            while(1)
            {
                dst_block = find_next_good_block(addr/block_size);
                if(dst_block == 0)
                {
                    printf("[nand_write_img_ex]find next good block fail\n");
                    return -1;
                }
                ret = block_replace(addr/block_size,dst_block,addr/page_size);
                if(ret == 0)
                {
                    printf("[nand_write_img_ex]block replace fail,continue\n");
                    continue;
                }
                else
                {
                    printf("[nand_write_img_ex]block replace sucess %x--> %x\n",addr/block_size,dst_block);
                    break;
                }

            }
            addr = (addr%block_size) + (dst_block*block_size);
            /*	if (__nand_erase(addr) == 0)
            	{
                    printf("[ERASE] erase 0x%x fail\n",addr);
                    mark_block_bad (addr);
             	}
            	data -= ((addr%block_size)/page_size)*write_size;
            	length += ((addr%block_size)/page_size)*write_size;
            	addr += block_size;*/
            continue;  // write fail, try  to write the next block
        }
        if(left_size)
        {
            data += (write_size - left_size);
            length -= (write_size - left_size);
            addr += page_size;
            download_size += (write_size - left_size);
            left_size = 0;
        }
        else
        {
            data += write_size;
            length -= write_size;
            addr += page_size;
            download_size += write_size;
        }
    }
    *next_offset = addr - last_addr;
    if(last_chunk)
    {
        /*3. erase any block remained in partition*/
        addr = ((addr+block_size-1)/block_size)*block_size;

        nand_erase((u64)addr,(u64)(partition_end - addr));
    }
    return 0;
}

int check_data_empty(void *data, unsigned size)
{
    u32 i;
    u32 *tp = (u32 *)data;

    for (i = 0; i < size / 4; i++)
    {
        if(*(tp + i) != 0xffffffff)
        {
            return 0;
        }
    }
    return 1;
}

static u32 find_next_good_block(u32 start_block)
{
    u32 i;
    u32 dst_block = 0;

    for (i=start_block; i<(total_size/g_nand_chip.erasesize); i++)
    {
        if(!snand_block_bad(&g_nand_chip,i*(g_nand_chip.erasesize/g_nand_chip.page_size)))
        {
            dst_block = i;
            break;
        }
    }
    return dst_block;
}

static bool block_replace(u32 src_block, u32 dst_block, u32 error_page)
{
    bool ret;
    u32 block_size = g_nand_chip.erasesize;
    u32 page_size = g_nand_chip.page_size;
    u32 i;
    u8 *data_buf;
    u8 *spare_buf;
    ret = __nand_erase((u64)dst_block*block_size);
    if(ret == 0)
    {
        printf("[block_replace]%x-->%x erase fail\n",src_block,dst_block);
        mark_block_bad((u64)src_block*block_size);
        return ret;
    }
    data_buf = malloc(4096);
    spare_buf = malloc(256);
    if(!data_buf || !spare_buf)
    {
        printf("[block_replace]malloc mem fail\n");
        return -1;
    }

    memset(data_buf,0xff,4096);
    memset(spare_buf,0xff,256);
    for (i=0; i<error_page; i++)
    {
        snand_exec_read_page(&g_nand_chip,src_block*(block_size/page_size) + i,page_size, (u8 *)data_buf, (u8 *)spare_buf);
        ret = nand_write_page_hwecc((u64)dst_block*block_size + i*page_size,(char *)data_buf,(char *)spare_buf);
        if(ret == 0)
        {
            mark_block_bad((u64)dst_block*block_size);
        }
    }

    mark_block_bad((u64)src_block*block_size);
    free(data_buf);
    free(spare_buf);
    return ret;

}

u32 mtk_nand_erasesize(void)
{
    return g_nand_chip.erasesize;
}

#endif
