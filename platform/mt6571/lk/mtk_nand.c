#include <string.h>
#include <config.h>
#include <malloc.h>
#include <printf.h>
#include <platform/mt_typedefs.h>
#include <platform/mtk_nand.h>
#include <mt_partition.h>
#include <platform/bmt.h>
#include "partition_define.h"
#include "cust_nand.h"
#include <arch/ops.h>
#include "nand_device_list.h"
#include <kernel/event.h>
#include <platform/mt_irq.h>

//#define NAND_LK_TEST
#ifdef NAND_LK_TEST
#include "mt_partition.h"
#endif
#if defined(MTK_COMBO_NAND_SUPPORT)
	// BMT_POOL_SIZE is not used anymore
#else
	#ifndef PART_SIZE_BMTPOOL
	#define BMT_POOL_SIZE (80)
	#else
	#define BMT_POOL_SIZE (PART_SIZE_BMTPOOL)
	#endif
#endif

#define PMT_POOL_SIZE	(2)

#define STATUS_READY			(0x40)
#define STATUS_FAIL				(0x01)
#define STATUS_WR_ALLOW			(0x80)

#ifdef CONFIG_CMD_NAND
extern int mt_part_register_device(part_dev_t * dev);

struct nand_ecclayout nand_oob_16 = {
    .eccbytes = 8,
    .eccpos = {8, 9, 10, 11, 12, 13, 14, 15},
    .oobfree = {{1, 6}, {0, 0}}
};

struct nand_ecclayout nand_oob_64 = {
    .eccbytes = 32,
    .eccpos = {32, 33, 34, 35, 36, 37, 38, 39,
               40, 41, 42, 43, 44, 45, 46, 47,
               48, 49, 50, 51, 52, 53, 54, 55,
               56, 57, 58, 59, 60, 61, 62, 63},
    .oobfree = {{1, 7}, {9, 7}, {17, 7}, {25, 6}, {0, 0}}
};

struct nand_ecclayout nand_oob_128 = {
    .eccbytes = 64,
    .eccpos = {
               64, 65, 66, 67, 68, 69, 70, 71,
               72, 73, 74, 75, 76, 77, 78, 79,
               80, 81, 82, 83, 84, 85, 86, 86,
               88, 89, 90, 91, 92, 93, 94, 95,
               96, 97, 98, 99, 100, 101, 102, 103,
               104, 105, 106, 107, 108, 109, 110, 111,
               112, 113, 114, 115, 116, 117, 118, 119,
               120, 121, 122, 123, 124, 125, 126, 127},
    .oobfree = {{1, 7}, {9, 7}, {17, 7}, {25, 7}, {33, 7}, {41, 7}, {49, 7},
                {57, 6}}
};

static bmt_struct *g_bmt = NULL;
static struct nand_chip g_nand_chip;
static int en_interrupt = 0;
static event_t nand_int_event;

#define ERR_RTN_SUCCESS   1
#define ERR_RTN_FAIL      0
#define ERR_RTN_BCH_FAIL -1
u32 BLOCK_SIZE;
#define NFI_ISSUE_COMMAND(cmd, col_addr, row_addr, col_num, row_num) \
   do { \
      DRV_WriteReg(NFI_CMD_REG16,cmd);\
      while (DRV_Reg32(NFI_STA_REG32) & STA_CMD_STATE);\
      DRV_WriteReg32(NFI_COLADDR_REG32, col_addr);\
      DRV_WriteReg32(NFI_ROWADDR_REG32, row_addr);\
      DRV_WriteReg(NFI_ADDRNOB_REG16, col_num | (row_num<<ADDR_ROW_NOB_SHIFT));\
      while (DRV_Reg32(NFI_STA_REG32) & STA_ADDR_STATE);\
   }while(0);

flashdev_info devinfo;

#define CHIPVER_ECO_1 (0x8a00)
#define CHIPVER_ECO_2 (0x8a01)
#define RAND_TYPE_SAMSUNG 0
#define RAND_TYPE_TOSHIBA 1
#define RAND_TYPE_NONE 2
extern u64 part_get_startaddress(u64 byte_address, u32 *idx);
extern bool raw_partition(u32 index);
bool __nand_erase (u64 logical_addr);
bool mark_block_bad (u64 logical_addr);
bool nand_erase_hw (u64 offset);
int check_data_empty(void *data, unsigned size);
struct NAND_CMD g_kCMD;
static u32 g_i4ErrNum;
static bool g_bInitDone;
u64 total_size;
u64 g_nand_size = 0;

__attribute__ ((aligned(64))) static unsigned char g_data_buf[16384+1024];
__attribute__ ((aligned(64))) static struct nand_buffers nBuf;
__attribute__ ((aligned(64))) static unsigned char data_buf_temp[16384];
__attribute__ ((aligned(64))) static unsigned char oob_buf_temp[1024];
enum flashdev_vendor gVendor;
static unsigned char g_spare_buf[256];
static u32 download_size = 0;
static bool use_randomizer = FALSE;
u32 MICRON_TRANSFER(u32 pageNo);

typedef u32 (*GetLowPageNumber)(u32 pageNo);

GetLowPageNumber functArray[]=
{
	MICRON_TRANSFER,
};

u32 MICRON_TRANSFER(u32 pageNo)
{
	u32 temp;
	if(pageNo < 4)
		return pageNo;
	temp = (pageNo - 4) & 0xFFFFFFFE;
	if(pageNo<=130)
		return (pageNo+temp);
	else
		return (pageNo+temp-2);
}


static bool mtk_nand_read_status(void);

static inline unsigned int uffs(unsigned int x)
{
    unsigned int r = 1;

    if (!x)
        return 0;
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

void dump_nfi(void)
{
    printf("~~~~Dump NFI Register in LK~~~~\n");
    printf("NFI_CNFG_REG16: 0x%x\n", DRV_Reg16(NFI_CNFG_REG16));
    printf("NFI_PAGEFMT_REG16: 0x%x\n", DRV_Reg16(NFI_PAGEFMT_REG16));
    printf("NFI_CON_REG16: 0x%x\n", DRV_Reg16(NFI_CON_REG16));
    printf("NFI_ACCCON_REG32: 0x%x\n", DRV_Reg32(NFI_ACCCON_REG32));
    printf("NFI_INTR_EN_REG16: 0x%x\n", DRV_Reg16(NFI_INTR_EN_REG16));
    printf("NFI_INTR_REG16: 0x%x\n", DRV_Reg16(NFI_INTR_REG16));
    printf("NFI_CMD_REG16: 0x%x\n", DRV_Reg16(NFI_CMD_REG16));
    printf("NFI_ADDRNOB_REG16: 0x%x\n", DRV_Reg16(NFI_ADDRNOB_REG16));
    printf("NFI_COLADDR_REG32: 0x%x\n", DRV_Reg32(NFI_COLADDR_REG32));
    printf("NFI_ROWADDR_REG32: 0x%x\n", DRV_Reg32(NFI_ROWADDR_REG32));
    printf("NFI_STRDATA_REG16: 0x%x\n", DRV_Reg16(NFI_STRDATA_REG16));
    printf("NFI_DATAW_REG32: 0x%x\n", DRV_Reg32(NFI_DATAW_REG32));
    printf("NFI_DATAR_REG32: 0x%x\n", DRV_Reg32(NFI_DATAR_REG32));
    printf("NFI_PIO_DIRDY_REG16: 0x%x\n", DRV_Reg16(NFI_PIO_DIRDY_REG16));
    printf("NFI_STA_REG32: 0x%x\n", DRV_Reg32(NFI_STA_REG32));
    printf("NFI_FIFOSTA_REG16: 0x%x\n", DRV_Reg16(NFI_FIFOSTA_REG16));
//    printf("NFI_LOCKSTA_REG16: 0x%x\n", DRV_Reg16(NFI_LOCKSTA_REG16));
    printf("NFI_ADDRCNTR_REG16: 0x%x\n", DRV_Reg16(NFI_ADDRCNTR_REG16));
    printf("NFI_STRADDR_REG32: 0x%x\n", DRV_Reg32(NFI_STRADDR_REG32));
    printf("NFI_BYTELEN_REG16: 0x%x\n", DRV_Reg16(NFI_BYTELEN_REG16));
    printf("NFI_CSEL_REG16: 0x%x\n", DRV_Reg16(NFI_CSEL_REG16));
    printf("NFI_IOCON_REG16: 0x%x\n", DRV_Reg16(NFI_IOCON_REG16));
    printf("NFI_FDM0L_REG32: 0x%x\n", DRV_Reg32(NFI_FDM0L_REG32));
    printf("NFI_FDM0M_REG32: 0x%x\n", DRV_Reg32(NFI_FDM0M_REG32));
    printf("NFI_LOCK_REG16: 0x%x\n", DRV_Reg16(NFI_LOCK_REG16));
    printf("NFI_LOCKCON_REG32: 0x%x\n", DRV_Reg32(NFI_LOCKCON_REG32));
    printf("NFI_LOCKANOB_REG16: 0x%x\n", DRV_Reg16(NFI_LOCKANOB_REG16));
    printf("NFI_FIFODATA0_REG32: 0x%x\n", DRV_Reg32(NFI_FIFODATA0_REG32));
    printf("NFI_FIFODATA1_REG32: 0x%x\n", DRV_Reg32(NFI_FIFODATA1_REG32));
    printf("NFI_FIFODATA2_REG32: 0x%x\n", DRV_Reg32(NFI_FIFODATA2_REG32));
    printf("NFI_FIFODATA3_REG32: 0x%x\n", DRV_Reg32(NFI_FIFODATA3_REG32));
    printf("NFI_MASTERSTA_REG16: 0x%x\n", DRV_Reg16(NFI_MASTERSTA_REG16));
    printf("NFI clock register: 0x%x: %s\n",(PERI_CON_BASE+0x18), (DRV_Reg32((volatile u32 *)(PERI_CON_BASE+0x18)) & (0x1)) ? "Clock Disabled" : "Clock Enabled");
    printf("NFI clock SEL (MT65XX):0x%x: %s\n",(PERI_CON_BASE+0x5C), (DRV_Reg32((volatile u32 *)(PERI_CON_BASE+0x5C)) & (0x1)) ? "Half clock" : "Quarter clock");
}
u32 mtk_nand_page_transform(u64 logical_address, u32* blk, u32* map_blk)
{
	u64 start_address;
	u32 index;
    u32 block;
    u32 page_in_block;
    u32 mapped_block;
	index= 0;
	if(VEND_NONE != gVendor)
	{
		start_address = part_get_startaddress((u64)logical_address, &index);

		//MSG(ERR, "start_address(0x%x), logical_address(0x%x) index(%d)\n",start_address,logical_address,index);
		if((0xFFFFFFFF != index) && (raw_partition(index)))
		{
	//		if(start_address == 0xFFFFFFFF)
	//			while(1);
			//MSG(ERR, "raw_partition(%d)\n",index);
			block = (u32)((start_address/BLOCK_SIZE) + (logical_address-start_address) / g_nand_chip.erasesize);
			page_in_block = ((logical_address-start_address) /g_nand_chip.page_size)% ((1 << (g_nand_chip.phys_erase_shift-g_nand_chip.page_shift)));

			if(devinfo.vendor != VEND_NONE)
			{
//				page_in_block = devinfo.feature_set.PairPage[page_in_block];
				page_in_block = functArray[devinfo.feature_set.ptbl_idx](page_in_block);
			}

		    mapped_block = get_mapping_block_index(block);
			//MSG(ERR, "transform_address(0x%x)\n",mapped_block*(BLOCK_SIZE/(g_nand_chip.page_size))+page_in_block);
		}
		else
		{
			block = (u32)(logical_address/BLOCK_SIZE);
			mapped_block = get_mapping_block_index(block);
			page_in_block = ((logical_address/g_nand_chip.page_size) % (BLOCK_SIZE >> g_nand_chip.page_shift));
		}
		
	}
	else
	{
		block = (u32)(logical_address/BLOCK_SIZE);
		mapped_block = get_mapping_block_index(block);
		page_in_block = (logical_address/g_nand_chip.page_size) % (1 << (g_nand_chip.phys_erase_shift-g_nand_chip.page_shift));
	}
		*blk = block;
		*map_blk = mapped_block;
		return mapped_block*(BLOCK_SIZE/(g_nand_chip.page_size))+page_in_block;
}


bool get_device_info(u8*id, flashdev_info *devinfo)
{
    u32 i,m,n,mismatch;
    int target=-1;
    u8 target_id_len=0;
    for (i = 0; i<CHIP_CNT; i++){
		mismatch=0;
		for(m=0;m<gen_FlashTable[i].id_length;m++){
			if(id[m]!=gen_FlashTable[i].id[m]){
				mismatch=1;
				break;
			}
		}
		if(mismatch == 0 && gen_FlashTable[i].id_length > target_id_len){
				target=i;
				target_id_len=gen_FlashTable[i].id_length;
		}
    }

    if(target != -1){
		MSG(INIT, "Recognize NAND: ID [");
		for(n=0;n<gen_FlashTable[target].id_length;n++){
			devinfo->id[n] = gen_FlashTable[target].id[n];
			MSG(INIT, "%x ",devinfo->id[n]);
		}
		MSG(INIT, "], Device Name [%s], Page Size [%d]B Spare Size [%d]B Total Size [%d]MB\n",gen_FlashTable[target].devciename,gen_FlashTable[target].pagesize,gen_FlashTable[target].sparesize,gen_FlashTable[target].totalsize);
		devinfo->id_length=gen_FlashTable[i].id_length;
		devinfo->blocksize = gen_FlashTable[target].blocksize;
		devinfo->addr_cycle = gen_FlashTable[target].addr_cycle;
		devinfo->iowidth = gen_FlashTable[target].iowidth;
		devinfo->timmingsetting = gen_FlashTable[target].timmingsetting;
		devinfo->advancedmode = gen_FlashTable[target].advancedmode;
		devinfo->pagesize = gen_FlashTable[target].pagesize;
		devinfo->sparesize = gen_FlashTable[target].sparesize;
		devinfo->totalsize = gen_FlashTable[target].totalsize;
		devinfo->sectorsize = gen_FlashTable[target].sectorsize;
		devinfo->s_acccon= gen_FlashTable[target].s_acccon;
		devinfo->s_acccon1= gen_FlashTable[target].s_acccon1;
		devinfo->freq= gen_FlashTable[target].freq;
		devinfo->vendor = gen_FlashTable[target].vendor;
		gVendor = gen_FlashTable[target].vendor;
		memcpy((u8*)&devinfo->feature_set, (u8*)&gen_FlashTable[target].feature_set, sizeof(struct MLC_feature_set));
		memcpy(devinfo->devciename, gen_FlashTable[target].devciename, sizeof(devinfo->devciename));
    	return true;
	}else{
	    MSG(INIT, "Not Found NAND: ID [");
		for(n=0;n<NAND_MAX_ID;n++){
			MSG(INIT, "%x ",id[n]);
		}
		MSG(INIT, "]\n");
        return false;
	}
}

static int mtk_nand_randomizer_config(struct gRandConfig *conf)
{
    kal_uint16   nfi_cnfg = 0;
	kal_uint32   nfi_ran_cnfg = 0;
	kal_uint8 i;

    /* set up NFI_CNFG */
    nfi_cnfg = DRV_Reg(NFI_CNFG_REG16);
	nfi_ran_cnfg = DRV_Reg32(NFI_RANDOM_CNFG_REG32);
	if(conf->type == RAND_TYPE_SAMSUNG)
	{
		nfi_ran_cnfg &= ~SEED_MASK << EN_SEED_SHIFT;
		nfi_ran_cnfg &= ~SEED_MASK << DE_SEED_SHIFT;
		nfi_ran_cnfg |= conf->seed[0] << EN_SEED_SHIFT;
		nfi_ran_cnfg |= conf->seed[0] << DE_SEED_SHIFT;
		nfi_cnfg |= CNFG_RAN_SEC;
		nfi_cnfg &= ~CNFG_RAN_SEL;
		use_randomizer = TRUE;
		//nfi_ran_cnfg |= 0x00010001;
	}
	else if(conf->type == RAND_TYPE_TOSHIBA)
	{
		use_randomizer = TRUE;
		for(i = 0 ; i < 6 ; i++)
		{	
			DRV_WriteReg32(NFI_RANDOM_ENSEED01_TS_REG32+i, conf->seed[i]);
			DRV_WriteReg32(NFI_RANDOM_DESEED01_TS_REG32+i, conf->seed[i]);
		}
		nfi_cnfg |= CNFG_RAN_SEC;
		nfi_cnfg &= ~CNFG_RAN_SEL;
		//nfi_ran_cnfg |= 0x00010001;
	}
	else
	{
		nfi_ran_cnfg &= ~0x00010001;
		use_randomizer = FALSE;
		return 0;
	}
    
    DRV_WriteReg(NFI_CNFG_REG16, nfi_cnfg);
	DRV_WriteReg32(NFI_RANDOM_CNFG_REG32, nfi_ran_cnfg);
	return 0;
}	
static bool mtk_nand_israndomizeron()
{
	kal_uint32   nfi_ran_cnfg = 0;
	nfi_ran_cnfg = DRV_Reg32(NFI_RANDOM_CNFG_REG32);
	if(nfi_ran_cnfg&0x00010001)
		return TRUE;

	return FALSE;
}
static void mtk_nand_turn_on_randomizer()
{
		//struct gRandConfig *conf = &devinfo.feature_set.randConfig;
	kal_uint32   nfi_ran_cnfg = 0;
	mtk_nand_randomizer_config(&devinfo.feature_set.randConfig);
	nfi_ran_cnfg = DRV_Reg32(NFI_RANDOM_CNFG_REG32);
	nfi_ran_cnfg |= 0x00010001;
	DRV_WriteReg32(NFI_RANDOM_CNFG_REG32, nfi_ran_cnfg);
}

static void mtk_nand_turn_off_randomizer()
{
	kal_uint32   nfi_ran_cnfg = 0;
	nfi_ran_cnfg = DRV_Reg32(NFI_RANDOM_CNFG_REG32);
	nfi_ran_cnfg &= ~0x00010001;
	DRV_WriteReg32(NFI_RANDOM_CNFG_REG32, nfi_ran_cnfg);
}


static void ECC_Config(u32 ecc_level)
{
    u32 u4ENCODESize;
    u32 u4DECODESize;
    u32 ecc_bit_cfg = 0;
	u32 sector_size = NAND_SECTOR_SIZE;
	if(devinfo.sectorsize == 1024)
		sector_size = 1024;
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
	case 14:
  		ecc_bit_cfg = ECC_CNFG_ECC14;
  		break;
	case 16:
  		ecc_bit_cfg = ECC_CNFG_ECC16;
  		break;
	case 18:
  		ecc_bit_cfg = ECC_CNFG_ECC18;
  		break;
	case 20:
  		ecc_bit_cfg = ECC_CNFG_ECC20;
  		break;
	case 22:
  		ecc_bit_cfg = ECC_CNFG_ECC22;
  		break;
	case 24:
  		ecc_bit_cfg = ECC_CNFG_ECC24;
  		break;
	case 28:
  		ecc_bit_cfg = ECC_CNFG_ECC28;
  		break;
	case 32:
  		ecc_bit_cfg = ECC_CNFG_ECC32;
  		break;
	case 36:
  		ecc_bit_cfg = ECC_CNFG_ECC36;
  		break;
	case 40:
  		ecc_bit_cfg = ECC_CNFG_ECC40;
  		break;
	case 44:
  		ecc_bit_cfg = ECC_CNFG_ECC44;
  		break;
	case 48:
  		ecc_bit_cfg = ECC_CNFG_ECC48;
  		break;
	case 52:
  		ecc_bit_cfg = ECC_CNFG_ECC52;
  		break;
	case 56:
  		ecc_bit_cfg = ECC_CNFG_ECC56;
  		break;
	case 60:
  		ecc_bit_cfg = ECC_CNFG_ECC60;
  		break;
    default:
  		break;

    }
    DRV_WriteReg16(ECC_DECCON_REG16, DEC_DE);
    do
    {;
    }
    while (!DRV_Reg16(ECC_DECIDLE_REG16));

    DRV_WriteReg16(ECC_ENCCON_REG16, ENC_DE);
    do
    {;
    }
    while (!DRV_Reg32(ECC_ENCIDLE_REG32));

    /* setup FDM register base */
 //   DRV_WriteReg32(ECC_FDMADDR_REG32, NFI_FDM0L_REG32);

    u4ENCODESize = (sector_size + NAND_FDM_PER_SECTOR) << 3;
    u4DECODESize = ((sector_size + NAND_FDM_PER_SECTOR) << 3) + ecc_level * ECC_PARITY_BIT;

    /* configure ECC decoder && encoder */
    DRV_WriteReg32(ECC_DECCNFG_REG32, ecc_bit_cfg | DEC_CNFG_NFI | DEC_CNFG_EMPTY_EN | (u4DECODESize << DEC_CNFG_CODE_SHIFT));
    DRV_WriteReg32(ECC_ENCCNFG_REG32, ecc_bit_cfg | ENC_CNFG_NFI | (u4ENCODESize << ENC_CNFG_MSG_SHIFT));
#ifndef MANUAL_CORRECT
    NFI_SET_REG32(ECC_DECCNFG_REG32, DEC_CNFG_CORRECT);
#else
    NFI_SET_REG32(ECC_DECCNFG_REG32, DEC_CNFG_EL);
#endif
}


static void ECC_Decode_Start(void)
{
    /* wait for device returning idle */
    while (!(DRV_Reg16(ECC_DECIDLE_REG16) & DEC_IDLE)) ;
    DRV_WriteReg16(ECC_DECCON_REG16, DEC_EN);
}


static void ECC_Decode_End(void)
{
    /* wait for device returning idle */
    while (!(DRV_Reg16(ECC_DECIDLE_REG16) & DEC_IDLE)) ;
    DRV_WriteReg16(ECC_DECCON_REG16, DEC_DE);
}

//-------------------------------------------------------------------------------
static void ECC_Encode_Start(void)
{
    /* wait for device returning idle */
    while (!(DRV_Reg32(ECC_ENCIDLE_REG32) & ENC_IDLE)) ;
    DRV_WriteReg16(ECC_ENCCON_REG16, ENC_EN);
}

//-------------------------------------------------------------------------------
static void ECC_Encode_End(void)
{
    /* wait for device returning idle */
    while (!(DRV_Reg32(ECC_ENCIDLE_REG32) & ENC_IDLE)) ;
    DRV_WriteReg16(ECC_ENCCON_REG16, ENC_DE);
}

//-------------------------------------------------------------------------------
static bool nand_check_bch_error(u8 * pDataBuf, u32 u4SecIndex, u32 u4PageAddr)
{
    bool bRet = true;
    u16 u2SectorDoneMask = 1 << u4SecIndex;
    u32 u4ErrorNumDebug0, u4ErrorNumDebug1,i, u4ErrNum;
    u32 timeout = 0xFFFF;

#ifdef MANUAL_CORRECT
    u32 au4ErrBitLoc[6];
    u32 u4ErrByteLoc, u4BitOffset;
    u32 u4ErrBitLoc1th, u4ErrBitLoc2nd;
#endif

    while (0 == (u2SectorDoneMask & DRV_Reg16(ECC_DECDONE_REG16)))
    {
        timeout--;
        if (0 == timeout)
        {
            return false;
        }
    }

#ifndef MANUAL_CORRECT
    if(0 == (DRV_Reg32(NFI_STA_REG32) & STA_READ_EMPTY))
    {
        u4ErrorNumDebug0 = DRV_Reg32(ECC_DECENUM0_REG32);
        u4ErrorNumDebug1 = DRV_Reg32(ECC_DECENUM1_REG32);
	if (0 != (u4ErrorNumDebug0 & 0xFFFFFFFF) || 0 != (u4ErrorNumDebug1 & 0xFFFFFFFF))
        {
            for (i = 0; i <= u4SecIndex; ++i)
            {
                if (i < 4)
                {
	                u4ErrNum = DRV_Reg32(ECC_DECENUM0_REG32) >> (i * 8);
                } else
                {
	                u4ErrNum = DRV_Reg32(ECC_DECENUM1_REG32) >> ((i - 4) * 8);
                }
	        u4ErrNum &= ERR_NUM0;
	        if (ERR_NUM0 == u4ErrNum)
                {
                    MSG(ERR, "In LittleKernel UnCorrectable at PageAddr=%d, Sector=%d\n", u4PageAddr, i);
                    bRet = false;
                } else
                {
		    if (u4ErrNum)
                    {
                        MSG(ERR, " In LittleKernel Correct %d at PageAddr=%d, Sector=%d\n", u4ErrNum, u4PageAddr, i);
		    }
                }
            }
        }
    }
#else
    memset(au4ErrBitLoc, 0x0, sizeof(au4ErrBitLoc));
    u4ErrorNumDebug0 = DRV_Reg32(ECC_DECENUM_REG32);
    u4ErrNum = (DRV_Reg32((ECC_DECENUM_REG32+(u4SecIndex/4)))>>((u4SecIndex%4)*8))& ERR_NUM0;

    if (u4ErrNum)
    {
        if (ERR_NUM0 == u4ErrNum)
        {
            MSG(ERR, "UnCorrectable at PageAddr=%d\n", u4PageAddr);
            bRet = false;
        } else
        {
            for (i = 0; i < ((u4ErrNum + 1) >> 1); ++i)
            {
                au4ErrBitLoc[i] = DRV_Reg32(ECC_DECEL0_REG32 + i);
                u4ErrBitLoc1th = au4ErrBitLoc[i] & 0x3FFF;
                if (u4ErrBitLoc1th < 0x1000)
                {
                    u4ErrByteLoc = u4ErrBitLoc1th / 8;
                    u4BitOffset = u4ErrBitLoc1th % 8;
                    pDataBuf[u4ErrByteLoc] = pDataBuf[u4ErrByteLoc] ^ (1 << u4BitOffset);
                } else
                {
                    MSG(ERR, "UnCorrectable ErrLoc=%d\n", au4ErrBitLoc[i]);
                }

                u4ErrBitLoc2nd = (au4ErrBitLoc[i] >> 16) & 0x3FFF;
                if (0 != u4ErrBitLoc2nd)
                {
                    if (u4ErrBitLoc2nd < 0x1000)
                    {
                        u4ErrByteLoc = u4ErrBitLoc2nd / 8;
                        u4BitOffset = u4ErrBitLoc2nd % 8;
                        pDataBuf[u4ErrByteLoc] = pDataBuf[u4ErrByteLoc] ^ (1 << u4BitOffset);
                    } else
                    {
                        MSG(ERR, "UnCorrectable High ErrLoc=%d\n", au4ErrBitLoc[i]);
                    }
                }
            }
            bRet = true;
        }

        if (0 == (DRV_Reg16(ECC_DECFER_REG16) & (1 << u4SecIndex)))
        {
            bRet = false;
        }
    }
#endif

    return bRet;
}

#if 1
static bool nand_RFIFOValidSize(u16 u2Size)
{
    u32 timeout = 0xFFFF;
    while (FIFO_RD_REMAIN(DRV_Reg16(NFI_FIFOSTA_REG16)) < u2Size)
    {
        timeout--;
        if (0 == timeout)
        {
            return false;
        }
    }
    if (u2Size == 0)
    {
        while (FIFO_RD_REMAIN(DRV_Reg16(NFI_FIFOSTA_REG16)))
        {
            timeout--;
            if (0 == timeout)
            {
                printf("nand_RFIFOValidSize failed: 0x%x\n", u2Size);
                return false;
            }
        }
    }

    return true;
}

//-------------------------------------------------------------------------------
static bool nand_WFIFOValidSize(u16 u2Size)
{
    u32 timeout = 0xFFFF;
    while (FIFO_WR_REMAIN(DRV_Reg16(NFI_FIFOSTA_REG16)) > u2Size)
    {
        timeout--;
        if (0 == timeout)
        {
            return false;
        }
    }
    if (u2Size == 0)
    {
        while (FIFO_WR_REMAIN(DRV_Reg16(NFI_FIFOSTA_REG16)))
        {
            timeout--;
            if (0 == timeout)
            {
                printf("nand_RFIFOValidSize failed: 0x%x\n", u2Size);
                return false;
            }
        }
    }

    return true;
}
#endif


static bool nand_status_ready(u32 u4Status)
{
    u32 timeout = 0xFFFF;
    while ((DRV_Reg32(NFI_STA_REG32) & u4Status) != 0)
    {
        timeout--;
        if (0 == timeout)
        {
            return false;
        }
    }
    return true;
}

static bool nand_reset(void)
{
    int timeout = 0xFFFF;
	bool ret;
    if (DRV_Reg16(NFI_MASTERSTA_REG16) & 0xFFF) // master is busy
    {
        DRV_WriteReg32(NFI_CON_REG16, CON_FIFO_FLUSH | CON_NFI_RST);
        while (DRV_Reg16(NFI_MASTERSTA_REG16) & 0xFFF)
        {
            timeout--;
            if (!timeout)
            {
                MSG(FUC, "Wait for NFI_MASTERSTA timeout\n");
            }
        }
    }
    /* issue reset operation */
    DRV_WriteReg32(NFI_CON_REG16, CON_FIFO_FLUSH | CON_NFI_RST);

	ret= nand_status_ready(STA_NFI_FSM_MASK | STA_NAND_BUSY) && nand_RFIFOValidSize(0) && nand_WFIFOValidSize(0);
    return ret;
}

//-------------------------------------------------------------------------------
static void nand_set_mode(u16 u2OpMode)
{
    u16 u2Mode = DRV_Reg16(NFI_CNFG_REG16);
    u2Mode &= ~CNFG_OP_MODE_MASK;
    u2Mode |= u2OpMode;
    DRV_WriteReg16(NFI_CNFG_REG16, u2Mode);
}

//-------------------------------------------------------------------------------
static void nand_set_autoformat(bool bEnable)
{
    if (bEnable)
    {
        NFI_SET_REG16(NFI_CNFG_REG16, CNFG_AUTO_FMT_EN);
    } else
    {
        NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_AUTO_FMT_EN);
    }
}

//-------------------------------------------------------------------------------
static void nand_configure_fdm(u16 u2FDMSize)
{
    NFI_CLN_REG16(NFI_PAGEFMT_REG16, PAGEFMT_FDM_MASK | PAGEFMT_FDM_ECC_MASK);
    NFI_SET_REG16(NFI_PAGEFMT_REG16, u2FDMSize << PAGEFMT_FDM_SHIFT);
    NFI_SET_REG16(NFI_PAGEFMT_REG16, u2FDMSize << PAGEFMT_FDM_ECC_SHIFT);
}


//-------------------------------------------------------------------------------
static bool nand_set_command(u16 command)
{
    /* Write command to device */
    DRV_WriteReg16(NFI_CMD_REG16, command);
    return nand_status_ready(STA_CMD_STATE);
}

//-------------------------------------------------------------------------------
static bool nand_set_address(u32 u4ColAddr, u32 u4RowAddr, u16 u2ColNOB, u16 u2RowNOB)
{
    /* fill cycle addr */
    DRV_WriteReg32(NFI_COLADDR_REG32, u4ColAddr);
    DRV_WriteReg32(NFI_ROWADDR_REG32, u4RowAddr);
    DRV_WriteReg16(NFI_ADDRNOB_REG16, u2ColNOB | (u2RowNOB << ADDR_ROW_NOB_SHIFT));
    return nand_status_ready(STA_ADDR_STATE);
}

//-------------------------------------------------------------------------------
static bool nand_check_RW_count(struct nand_chip *nand, u16 u2WriteSize)
{
    u32 timeout = 0xFFFF;
    u16 u2SecNum = u2WriteSize >> nand->sector_shift;
    while (ADDRCNTR_CNTR(DRV_Reg16(NFI_ADDRCNTR_REG16)) < u2SecNum)
    {
        timeout--;
        if (0 == timeout)
        {
            return false;
        }
    }
    return true;
}

//-------------------------------------------------------------------------------
static bool nand_ready_for_read(struct nand_chip *nand, u32 u4RowAddr, u32 u4ColAddr, bool bFull, u8 * buf)
{
    /* Reset NFI HW internal state machine and flush NFI in/out FIFO */
    bool bRet = false;
    u16 sec_num = 1 << (nand->page_shift - nand->sector_shift);
    u32 col_addr = u4ColAddr;
    if (nand->options & NAND_BUSWIDTH_16)
        col_addr >>= 1;
    u32 colnob = 2, rownob = devinfo.addr_cycle - 2;

    if (!nand_reset())
    {
        goto cleanup;
    }

    NFI_SET_REG16(NFI_CNFG_REG16, CNFG_HW_ECC_EN);
    nand_set_mode(CNFG_OP_READ);
    NFI_SET_REG16(NFI_CNFG_REG16, CNFG_READ_EN);
    DRV_WriteReg32(NFI_CON_REG16, sec_num << CON_NFI_SEC_SHIFT);

    if (bFull)
    {
#if USE_AHB_MODE
        NFI_SET_REG16(NFI_CNFG_REG16, CNFG_AHB);
#else
        NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_AHB);
#endif
        DRV_WriteReg32(NFI_STRADDR_REG32, buf);
        NFI_SET_REG16(NFI_CNFG_REG16, CNFG_HW_ECC_EN);
    } else
    {
        NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_AHB);
        NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_HW_ECC_EN);
    }

    nand_set_autoformat(bFull);
    if (bFull)
        ECC_Decode_Start();

    if (!nand_set_command(NAND_CMD_READ_0))
    {
        goto cleanup;
    }
    if (!nand_set_address(col_addr, u4RowAddr, colnob, rownob))
    {
        goto cleanup;
    }

    if (!nand_set_command(NAND_CMD_READ_START))
    {
        goto cleanup;
    }

    if (!nand_status_ready(STA_NAND_BUSY))
    {
        goto cleanup;
    }

    bRet = true;

  cleanup:
    return bRet;
}

//-----------------------------------------------------------------------------
static bool nand_ready_for_write(struct nand_chip *nand, u32 u4RowAddr, u8 * buf)
{
    bool bRet = false;
    u16 sec_num = 1 << (nand->page_shift - nand->sector_shift);
    u32 colnob = 2, rownob = devinfo.addr_cycle - 2;
    if (!nand_reset())
    {
        return false;
    }

    nand_set_mode(CNFG_OP_PRGM);
    NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_READ_EN);
    DRV_WriteReg32(NFI_CON_REG16, sec_num << CON_NFI_SEC_SHIFT);
#if USE_AHB_MODE
    NFI_SET_REG16(NFI_CNFG_REG16, CNFG_AHB);
    DRV_WriteReg32(NFI_STRADDR_REG32, buf);
#else
    NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_AHB);
#endif

    NFI_SET_REG16(NFI_CNFG_REG16, CNFG_HW_ECC_EN);
    nand_set_autoformat(true);
    ECC_Encode_Start();
    if (!nand_set_command(NAND_CMD_SEQIN))
    {
        goto cleanup;
    }
    if (!nand_set_address(0, u4RowAddr, colnob, rownob))
    {
        goto cleanup;
    }

    if (!nand_status_ready(STA_NAND_BUSY))
    {
        goto cleanup;
    }

    bRet = true;
  cleanup:

    return bRet;
}

//-----------------------------------------------------------------------------
static bool nand_dma_read_data(u8 * pDataBuf, u32 u4Size)
{
    u32 timeout = 0xFFFF;

    arch_clean_invalidate_cache_range((addr_t)pDataBuf,(size_t)u4Size);

    NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_BYTE_RW);
    DRV_Reg16(NFI_INTR_REG16);
    DRV_WriteReg16(NFI_INTR_EN_REG16, INTR_AHB_DONE_EN);
    NFI_SET_REG32(NFI_CON_REG16, CON_NFI_BRD);

    if(en_interrupt){
	if(event_wait_timeout(&nand_int_event,100)){
		printf("[nand_dma_read_data]wait for AHB done timeout\n");
		dump_nfi();
		return false;
	}

	timeout = 0xFFFF;
	    while ((u4Size >> g_nand_chip.sector_shift) > ((DRV_Reg16(NFI_BYTELEN_REG16) & 0xf000) >> 12))
	    {
		timeout--;
		if (0 == timeout)
		{
		    return false;       //4
		}
	    }
    }else{
    while (!(DRV_Reg16(NFI_INTR_REG16) & INTR_AHB_DONE))
    {
        timeout--;
        if (0 == timeout)
        {
            return false;
        }
    }

    timeout = 0xFFFF;
    while ((u4Size >> g_nand_chip.sector_shift) > ((DRV_Reg16(NFI_BYTELEN_REG16) & 0xf000) >> 12))
    {
        timeout--;
        if (0 == timeout)
        {
            return false;       //4
        }
    }
    }
    return true;
}

static bool nand_mcu_read_data(u8 * pDataBuf, u32 length)
{
    u32 timeout = 0xFFFF;
    u32 i;
    u32 *pBuf32;
    if (length % 4){
        NFI_SET_REG16(NFI_CNFG_REG16, CNFG_BYTE_RW);
    }else{
	  NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_BYTE_RW);
    }

    NFI_SET_REG32(NFI_CON_REG16, CON_NFI_BRD);
    pBuf32 = (u32 *) pDataBuf;
    if (length % 4)
    {
        for (i = 0; (i < length) && (timeout > 0);)
        {
            WAIT_NFI_PIO_READY(timeout);
            *pDataBuf++ = DRV_Reg8(NFI_DATAR_REG32);
            i++;

        }
    } else
    {
        WAIT_NFI_PIO_READY(timeout);
        for (i = 0; (i < (length >> 2)) && (timeout > 0);)
        {
            WAIT_NFI_PIO_READY(timeout);
            *pBuf32++ = DRV_Reg32(NFI_DATAR_REG32);
            i++;
        }
    }
    return true;
}

static bool nand_read_page_data(u8 * buf, u32 length)
{
#if USE_AHB_MODE
    return nand_dma_read_data(buf, length);
#else
    return nand_mcu_read_data(buf, length);
#endif
}

static bool nand_dma_write_data(u8 * buf, u32 length)
{
    u32 timeout = 0xFFFF;
    arch_clean_invalidate_cache_range((addr_t)buf,(size_t)length);

    NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_BYTE_RW);
    DRV_Reg16(NFI_INTR_REG16);
    DRV_WriteReg16(NFI_INTR_EN_REG16, INTR_AHB_DONE_EN);

    if ((unsigned int)buf % 16)
    {
        //printf("Un-16-aligned address\n");
        NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_DMA_BURST_EN);
    } else
    {
        NFI_SET_REG16(NFI_CNFG_REG16, CNFG_DMA_BURST_EN);
    }

    NFI_SET_REG32(NFI_CON_REG16, CON_NFI_BWR);
    if(en_interrupt){
	if(event_wait_timeout(&nand_int_event,100)){
		printf("[nand_dma_write_data]wait for AHB done timeout\n");
		dump_nfi();
                return false;
	}
    }else{
    while (!(DRV_Reg16(NFI_INTR_REG16) & INTR_AHB_DONE))
    {
        timeout--;
        if (0 == timeout)
        {
            printf("wait write AHB done timeout\n");
            dump_nfi();
	    return FALSE;
		}
        }
    }
    return true;
}

static bool nand_mcu_write_data(const u8 * buf, u32 length)
{
    u32 timeout = 0xFFFF;
    u32 i;
    u32 *pBuf32 = (u32 *) buf;
    NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_BYTE_RW);
    NFI_SET_REG32(NFI_CON_REG16, CON_NFI_BWR);

    if ((u32) buf % 4 || length % 4)
        NFI_SET_REG16(NFI_CNFG_REG16, CNFG_BYTE_RW);
    else
        NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_BYTE_RW);

    if ((u32) buf % 4 || length % 4)
    {
        for (i = 0; (i < (length)) && (timeout > 0);)
        {
            if (DRV_Reg16(NFI_PIO_DIRDY_REG16) & 1)
            {
                DRV_WriteReg32(NFI_DATAW_REG32, *buf++);
                i++;
            } else
            {
                timeout--;
            }
            if (0 == timeout)
            {
                printf("[%s] nand mcu write timeout\n", __FUNCTION__);
                dump_nfi();
                return false;
            }
        }
    } else
    {
        for (i = 0; (i < (length >> 2)) && (timeout > 0);)
        {
            if (DRV_Reg16(NFI_PIO_DIRDY_REG16) & 1)
            {
                DRV_WriteReg32(NFI_DATAW_REG32, *pBuf32++);
                i++;
            } else
            {
                timeout--;
            }
            if (0 == timeout)
            {
                printf("[%s] nand mcu write timeout\n", __FUNCTION__);
                dump_nfi();
                return false;
            }
        }
    }

    return true;
}

//-----------------------------------------------------------------------------
static bool nand_write_page_data(u8 * buf, u32 length)
{
#if USE_AHB_MODE
    return nand_dma_write_data(buf, length);
#else
    return nand_mcu_write_data(buf, length);
#endif
}

static void nand_read_fdm_data(u8 * pDataBuf, u32 u4SecNum)
{
    u32 i;
    u32 *pBuf32 = (u32 *) pDataBuf;
    for (i = 0; i < u4SecNum; ++i)
    {
        *pBuf32++ = DRV_Reg32(NFI_FDM0L_REG32 + (i << 1));
        *pBuf32++ = DRV_Reg32(NFI_FDM0M_REG32 + (i << 1));
    }
}

static void nand_write_fdm_data(u8 * pDataBuf, u32 u4SecNum)
{
    u32 i;
    u32 *pBuf32 = (u32 *) pDataBuf;
    for (i = 0; i < u4SecNum; ++i)
    {
        DRV_WriteReg32(NFI_FDM0L_REG32 + (i << 1), *pBuf32++);
        DRV_WriteReg32(NFI_FDM0M_REG32 + (i << 1), *pBuf32++);
    }
}

static void nand_stop_read(void)
{
    NFI_CLN_REG32(NFI_CON_REG16, CON_NFI_BRD);
    ECC_Decode_End();
}

static void nand_stop_write(void)
{
    NFI_CLN_REG32(NFI_CON_REG16, CON_NFI_BWR);
    ECC_Encode_End();
}

static bool nand_check_dececc_done(u32 u4SecNum)
{
    u32 timeout, dec_mask;
    timeout = 0xffff;
    dec_mask = (1 << u4SecNum) - 1;
    while ((dec_mask != DRV_Reg(ECC_DECDONE_REG16)) && timeout > 0)
        timeout--;
    if (timeout == 0)
    {
        MSG(ERR, "ECC_DECDONE: timeout\n");
        dump_nfi();
        return false;
    }
    return true;
}

//---------------------------------------------------------------------------
static bool mtk_nand_read_status(void)
{
    int status = 0;//, i;
    unsigned int timeout;

    nand_reset();

    /* Disable HW ECC */
    NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_HW_ECC_EN);

    /* Disable 16-bit I/O */
    NFI_CLN_REG16(NFI_PAGEFMT_REG16, PAGEFMT_DBYTE_EN);
    NFI_SET_REG16(NFI_CNFG_REG16, CNFG_OP_SRD | CNFG_READ_EN | CNFG_BYTE_RW);

    DRV_WriteReg32(NFI_CON_REG16, CON_NFI_SRD | (1 << CON_NFI_NOB_SHIFT));

    DRV_WriteReg32(NFI_CON_REG16, 0x3);
    nand_set_mode(CNFG_OP_SRD);
    DRV_WriteReg16(NFI_CNFG_REG16, 0x2042);
   	nand_set_command(NAND_CMD_STATUS);
    DRV_WriteReg32(NFI_CON_REG16, 0x90);

    timeout = TIMEOUT_4;
    WAIT_NFI_PIO_READY(timeout);

    if (timeout)
    {
        status = (DRV_Reg16(NFI_DATAR_REG32));
    }
    //~  clear NOB
    DRV_WriteReg32(NFI_CON_REG16, 0);

    if (g_nand_chip.bus16 == IO_WIDTH_16)
    {
        NFI_SET_REG16(NFI_PAGEFMT_REG16, PAGEFMT_DBYTE_EN);
        NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_BYTE_RW);
    }
    // check READY/BUSY status first
    if (!(STATUS_READY & status))
    {
        MSG(ERR, "status is not ready\n");
    }
    // flash is ready now, check status code
    if (STATUS_FAIL & status)
    {
        if (!(STATUS_WR_ALLOW & status))
        {
            MSG(INIT, "status locked\n");
            return FALSE;
        } else
        {
            MSG(INIT, "status unknown\n");
            return FALSE;
        }
    } else
    {
        return TRUE;
    }
}

bool mtk_nand_SetFeature(u16 cmd, u32 addr, u8 *value,  u8 bytes)
{
	kal_uint16           reg_val     	 = 0;
	kal_uint8            write_count     = 0;
	kal_uint32           timeout=TIMEOUT_3;//0xffff;

	nand_reset();

	reg_val |= (CNFG_OP_CUST | CNFG_BYTE_RW);
	DRV_WriteReg(NFI_CNFG_REG16, reg_val);

	nand_set_command(cmd);
	nand_set_address(addr, 0, 1, 0);
	//NFI_ISSUE_COMMAND(cmd, addr, 0, 1, 0)

	//SAL_NFI_Config_Sector_Number(1);
	DRV_WriteReg32(NFI_CON_REG16, 1 << CON_NFI_SEC_SHIFT);
	NFI_SET_REG32(NFI_CON_REG16, CON_NFI_BWR);
	DRV_WriteReg(NFI_STRDATA_REG16, 0x1);
	//SAL_NFI_Start_Data_Transfer(KAL_FALSE, KAL_TRUE);
	while ( (write_count < bytes) && timeout )
    {
    	WAIT_NFI_PIO_READY(timeout)
        if(timeout == 0)
        {
            break;
        }
        DRV_WriteReg32(NFI_DATAW_REG32, *value++);
        write_count++;
        timeout = TIMEOUT_3;
    }
	while ( (*NFI_STA_REG32 & STA_NAND_BUSY) && (timeout) ){timeout--;}
	mtk_nand_read_status();
	if(timeout != 0)
		return TRUE;
	else
		return FALSE;
}

bool mtk_nand_GetFeature(u16 cmd, u32 addr, u8 *value,  u8 bytes)
{
	kal_uint16           reg_val     	 = 0;
	kal_uint8            read_count     = 0;
	kal_uint32           timeout=TIMEOUT_3;//0xffff;

	nand_reset();

	reg_val |= (CNFG_OP_CUST | CNFG_BYTE_RW | CNFG_READ_EN);
	DRV_WriteReg(NFI_CNFG_REG16, reg_val);

	nand_set_command(cmd);
	nand_set_address(addr, 0, 1, 0);

	//SAL_NFI_Config_Sector_Number(0);
	DRV_WriteReg32(NFI_CON_REG16, 0 << CON_NFI_SEC_SHIFT);
	reg_val = DRV_Reg32(NFI_CON_REG16);
    reg_val &= ~CON_NFI_NOB_MASK;
    reg_val |= ((4 << CON_NFI_NOB_SHIFT)|CON_NFI_SRD);
    DRV_WriteReg32(NFI_CON_REG16, reg_val);
	//NFI_SET_REG16(NFI_CON_REG16, CON_NFI_BWR);
	DRV_WriteReg(NFI_STRDATA_REG16, 0x1);
//	SAL_NFI_Start_Data_Transfer(KAL_TRUE, KAL_TRUE);
	while ( (read_count < bytes) && timeout )
    {
    	WAIT_NFI_PIO_READY(timeout)
        if(timeout == 0)
        {
            break;
        }
		*value++ = DRV_Reg32(NFI_DATAR_REG32);
        read_count++;
        timeout = TIMEOUT_3;
    }
	mtk_nand_read_status();
	if(timeout != 0)
		return TRUE;
	else
		return FALSE;

}


int nand_exec_read_page_hw(struct nand_chip *nand, u32 u4RowAddr, u32 u4PageSize, u8 * pPageBuf, u8 * pFDMBuf)
{
    int bRet;
    u32 u4SecNum = u4PageSize >> nand->sector_shift;
	bool retry = FALSE;
	bool readRetry = FALSE;
	int retryCount = 0;
	
	do{
		if(use_randomizer && u4RowAddr >= RAND_START_ADDR && retry == FALSE)
			mtk_nand_turn_on_randomizer();
	bRet = ERR_RTN_SUCCESS;
    if (nand_ready_for_read(nand, u4RowAddr, 0, true, pPageBuf))
    {
        if (!nand_read_page_data(pPageBuf, u4PageSize))
        {
            bRet = ERR_RTN_FAIL;
        }
        if (!nand_status_ready(STA_NAND_BUSY))
        {
            bRet = ERR_RTN_FAIL;
        }

        if (!nand_check_dececc_done(u4SecNum))
        {
            bRet = ERR_RTN_FAIL;
        }

        nand_read_fdm_data(pFDMBuf, u4SecNum);

        if (!nand_check_bch_error(pPageBuf, u4SecNum - 1, u4RowAddr))
        {	
        	bRet = ERR_RTN_BCH_FAIL;
				if(devinfo.vendor != VEND_NONE){
					readRetry = TRUE;
				}
            g_i4ErrNum++;
        }
		if(0 != (DRV_Reg32(NFI_STA_REG32) & STA_READ_EMPTY))
		{
			memset(pPageBuf, 0xFF, u4PageSize);
			memset(pFDMBuf, 0xFF, 8*u4SecNum);
		}
        nand_stop_read();
    }
		if(use_randomizer)
					mtk_nand_turn_off_randomizer();
		if (bRet == ERR_RTN_BCH_FAIL)
		{
			u32 feature = devinfo.feature_set.FeatureSet.readRetryStart+retryCount;
			if(retryCount < devinfo.feature_set.FeatureSet.readRetryCnt)
			{
				mtk_nand_SetFeature(devinfo.feature_set.FeatureSet.sfeatureCmd,\
								devinfo.feature_set.FeatureSet.readRetryAddress,\
								(u8*)&feature,4);
				retryCount++;
			}
			else
			{
				feature = devinfo.feature_set.FeatureSet.readRetryDefault;
				mtk_nand_SetFeature(devinfo.feature_set.FeatureSet.sfeatureCmd,\
								devinfo.feature_set.FeatureSet.readRetryAddress,\
								(u8*)&feature,4);
				readRetry = FALSE;
			}		
		}
		else
		{
			if(retryCount != 0)
			{
				u32 feature = devinfo.feature_set.FeatureSet.readRetryDefault;
				mtk_nand_SetFeature(devinfo.feature_set.FeatureSet.sfeatureCmd,\
								devinfo.feature_set.FeatureSet.readRetryAddress,\
								(u8*)&feature,4);
			}
			readRetry = FALSE;
		}
		if(TRUE == readRetry)
			bRet = ERR_RTN_SUCCESS;
	}while(readRetry);
	if(use_randomizer && u4RowAddr >= RAND_START_ADDR)
			mtk_nand_turn_off_randomizer();

    return bRet;
}

static bool nand_exec_read_page(struct nand_chip *nand, u32 u4RowAddr, u32 u4PageSize, u8 * pPageBuf, u8 * pFDMBuf)
{
	int bRet = ERR_RTN_SUCCESS;
//    u32 page_per_block = (BLOCK_SIZE/nand->page_size);
    u32 block;
    //int page_in_block = u4RowAddr % page_per_block;
    u32 page_addr;
    u32 mapped_block;
    int i, start, len, offset;
    struct nand_oobfree *free;
    u8 oob[0x80];

    //mapped_block = get_mapping_block_index(block);
    page_addr= mtk_nand_page_transform((u64)u4RowAddr << g_nand_chip.page_shift,&block,&mapped_block);

	//bRet = nand_exec_read_page_hw(nand, (mapped_block * page_per_block + page_in_block), u4PageSize, pPageBuf, oob);
	bRet = nand_exec_read_page_hw(nand, page_addr, u4PageSize, pPageBuf, oob);
    if (bRet == ERR_RTN_FAIL)
        return false;

    offset = 0;
    free = nand->ecclayout->oobfree;
    for (i = 0;  i < MTD_MAX_OOBFREE_ENTRIES&&free[i].length; i++)
    {
        start = free[i].offset;
        len = free[i].length;
        memcpy(pFDMBuf + offset, oob + start, len);
        offset += len;
    }

    return bRet;
}

static bool nand_exec_write_page(struct nand_chip *nand, u32 u4RowAddr, u32 u4PageSize, u8 * pPageBuf, u8 * pFDMBuf)
{
    bool bRet = true;
	//u32 page_per_block = 1 << (nand->phys_erase_shift - nand->page_shift);
	u32 block;
//	int page_in_block = u4RowAddr % page_per_block;
    u32 u4SecNum = u4PageSize >> nand->sector_shift;
	u32 page_addr;
    u32 mapped_block;\

    page_addr= mtk_nand_page_transform((u64)u4RowAddr << g_nand_chip.page_shift,&block,&mapped_block);

	if(use_randomizer && page_addr >= RAND_START_ADDR)
			mtk_nand_turn_on_randomizer();
    if (nand_ready_for_write(nand, page_addr, pPageBuf))
    {
        nand_write_fdm_data(pFDMBuf, u4SecNum);
        if (!nand_write_page_data(pPageBuf, u4PageSize))
        {
            bRet = false;
        }
        if (!nand_check_RW_count(nand, u4PageSize))
        {
            bRet = false;
        }
        nand_stop_write();
        nand_set_command(NAND_CMD_PAGE_PROG);
        while (DRV_Reg32(NFI_STA_REG32) & STA_NAND_BUSY) ;
    }
	if(use_randomizer && page_addr >= RAND_START_ADDR)
			mtk_nand_turn_off_randomizer();
    return bRet;
}

static bool nand_exec_write_page_raw(struct nand_chip *nand, u32 u4RowAddr, u32 u4PageSize, u8 * pPageBuf, u8 * pFDMBuf)
{
    bool bRet = true;
//	u32 page_per_block = 1 << (nand->phys_erase_shift - nand->page_shift);
//	int block = u4RowAddr / page_per_block;
//	int page_in_block = u4RowAddr % page_per_block;
    u32 u4SecNum = u4PageSize >> nand->sector_shift;

	if(use_randomizer && u4RowAddr >= RAND_START_ADDR)
			mtk_nand_turn_on_randomizer();
    if (nand_ready_for_write(nand, u4RowAddr, pPageBuf))
    {
        nand_write_fdm_data(pFDMBuf, u4SecNum);
        if (!nand_write_page_data(pPageBuf, u4PageSize))
        {
            bRet = false;
        }
        if (!nand_check_RW_count(nand, u4PageSize))
        {
            bRet = false;
        }
        nand_stop_write();
        nand_set_command(NAND_CMD_PAGE_PROG);
        while (DRV_Reg32(NFI_STA_REG32) & STA_NAND_BUSY) ;
    }
	if(use_randomizer && u4RowAddr >= RAND_START_ADDR)
			mtk_nand_turn_off_randomizer();
    return bRet;
}


static bool nand_read_oob_raw(struct nand_chip *chip, u32 page_addr, u32 length, u8 * buf)
{
    u32 sector = 0;
    u32 col_addr = 0;
    u32 spare_per_sec = devinfo.sparesize>>(chip->page_shift-chip->sector_shift);

    if (length > 32 || length % OOB_AVAIL_PER_SECTOR || !buf)
    {
        printf("[%s] invalid parameter, length: %d, buf: %p\n", __FUNCTION__, length, buf);
        return false;
    }

    while (length > 0)
    {
        col_addr = chip->sector_size+ sector * (chip->sector_size + spare_per_sec);
        if (!nand_ready_for_read(chip, page_addr, col_addr, false, NULL))
            return false;
        if (!nand_mcu_read_data(buf, length))
            return false;
        NFI_CLN_REG32(NFI_CON_REG16, CON_NFI_BRD);
        sector++;
        length -= OOB_AVAIL_PER_SECTOR;
    }

    return true;
}

bool nand_block_bad_hw(struct nand_chip * nand, u64 offset)
{
    u32 page_per_block = BLOCK_SIZE / nand->page_size;
    u32 page_addr;
	u32 block;
    u32 mapped_block;

    //mapped_block = get_mapping_block_index(block);
	page_addr = mtk_nand_page_transform(offset,&block,&mapped_block);

    memset(oob_buf_temp, 0,1024);

    page_addr &= ~(page_per_block - 1);

    if (FALSE == nand_exec_read_page_hw(nand, page_addr, nand->page_size, data_buf_temp , oob_buf_temp))
    {
        printf("nand_read_oob_raw return fail\n");
    }

    if (oob_buf_temp[0] != 0xff)
    {
        printf("Bad block detect at block 0x%x, oob_buf[0] is %x\n", page_addr / page_per_block, oob_buf_temp[0]);
        return true;
    }

    return false;
}

static bool nand_block_bad(struct nand_chip *nand, u32 page_addr)
{
    //u32 page_per_block = 1 << (nand->phys_erase_shift - nand->page_shift);
    //int block = page_addr / page_per_block;
    //int mapped_block = get_mapping_block_index(block);

    return nand_block_bad_hw(nand, (((u64)page_addr) << nand->page_shift));
}

//not support un-block-aligned write
static int nand_part_write(part_dev_t * dev, uchar * src, u64 dst, int size)
{
    struct nand_chip *nand = (struct nand_chip *)dev->blkdev;
    u8 res;
    u32 u4PageSize = 1 << nand->page_shift;
    u32 u4PageNumPerBlock = 1 << (nand->phys_erase_shift - nand->page_shift);
    u32 u4BlkEnd = (u32)(nand->chipsize >> nand->phys_erase_shift) << 1;
    u32 u4BlkAddr = (dst >> nand->phys_erase_shift) << 1;
    u32 u4ColAddr = dst & (u4PageSize - 1);
    u32 u4RowAddr = dst / nand->page_size;
    u32 u4RowEnd;
    u32 u4WriteLen = 0;
    u32 i4Len;
//	u32 mapped;
    u32 k = 0;
	//mtk_nand_page_transform((u64)dst,&u4BlkAddr,&mapped);
    for (k = 0; k < sizeof(g_kCMD.au1OOB); k++)
        *(g_kCMD.au1OOB + k) = 0xFF;

    while (((u32)size > u4WriteLen) && (u4BlkAddr < u4BlkEnd))
    {
        if (!u4ColAddr)
        {
            MSG(OPS, "Erase the block of 0x%08x\n", u4BlkAddr);
            nand_reset();
            nand_set_mode(CNFG_OP_ERASE);
            nand_set_command(NAND_CMD_ERASE_1);
            nand_set_address(0, u4RowAddr, 0, 3);
            nand_set_command(NAND_CMD_ERASE_2);
            while (DRV_Reg32(NFI_STA_REG32) & STA_NAND_BUSY) ;
        }

        res = nand_block_bad(nand, ((u4BlkAddr >> 1) * u4PageNumPerBlock));

        if (!res)
        {
            	u4RowEnd = (u4RowAddr + u4PageNumPerBlock) & (~u4PageNumPerBlock + 1);
            for (; u4RowAddr < u4RowEnd;u4RowAddr++)
				{
                i4Len = min(size - u4WriteLen, u4PageSize - u4ColAddr);
                if (0 >= i4Len)
                {
                    break;
                }
                if ((u4ColAddr == 0) && (i4Len == u4PageSize))
                {
                    nand_exec_write_page(nand, u4RowAddr, u4PageSize, src + u4WriteLen, g_kCMD.au1OOB);
                } else
                {
                    nand_exec_read_page(nand, u4RowAddr, u4PageSize, nand->buffers->databuf, g_kCMD.au1OOB);
                    memcpy(nand->buffers->databuf + u4ColAddr, src + u4WriteLen, i4Len);
                    nand_exec_write_page(nand, u4RowAddr, u4PageSize, nand->buffers->databuf, g_kCMD.au1OOB);
                }
                u4WriteLen += i4Len;
                u4ColAddr = (u4ColAddr + i4Len) & (u4PageSize - 1);
            }
        } else
        {
            printf("Detect bad block at block 0x%x\n", u4BlkAddr);
            u4RowAddr += u4PageNumPerBlock;
        }
	u4BlkAddr++;
    }

    return (int)u4WriteLen;

}

static int nand_part_read(part_dev_t * dev, u64 source, uchar * dst, int size)
{
    struct nand_chip *nand = (struct nand_chip *)dev->blkdev;
    uint8_t res;
    u32 u4PageSize = 1 << nand->page_shift;
    u32 u4PageNumPerBlock = BLOCK_SIZE/nand->page_size;
    u32 u4BlkEnd = (u32)(nand->chipsize / BLOCK_SIZE);
    u32 u4BlkAddr = (u32)(source / BLOCK_SIZE);
    u32 u4ColAddr = (u32)(source & (u4PageSize - 1));
    u32 u4RowAddr = (u32)(source/nand->page_size);
    u32 u4RowEnd;
//	u32 mapped;
    u32 u4ReadLen = 0;
    u32 i4Len;
    //mtk_nand_page_transform((u64)source,&u4BlkAddr,&mapped);
    while (((u32)size > u4ReadLen) && (u4BlkAddr < u4BlkEnd))
    {
        res = nand_block_bad(nand, (u4BlkAddr * u4PageNumPerBlock));

        if (!res)
        {
				u4RowEnd = (u4RowAddr + u4PageNumPerBlock) & (~u4PageNumPerBlock + 1);
            for (; u4RowAddr < u4RowEnd;u4RowAddr++)
            {
                i4Len = min(size - u4ReadLen, u4PageSize - u4ColAddr);
                if (0 >= i4Len)
                {
                    break;
                }
                if ((u4ColAddr == 0) && (i4Len == u4PageSize))
                {
                    nand_exec_read_page(nand, u4RowAddr, u4PageSize, dst + u4ReadLen, g_kCMD.au1OOB);
                } else
                {
                    nand_exec_read_page(nand, u4RowAddr, u4PageSize, nand->buffers->databuf, g_kCMD.au1OOB);
                    memcpy(dst + u4ReadLen, nand->buffers->databuf + u4ColAddr, i4Len);
                }
                u4ReadLen += i4Len;
                u4ColAddr = (u4ColAddr + i4Len) & (u4PageSize - 1);
            }
        } else
        {
            printf("Detect bad block at block 0x%x\n", u4BlkAddr);
            u4RowAddr += u4PageNumPerBlock;
        }
	u4BlkAddr++;
    }
    return (int)u4ReadLen;
}

static void nand_command_bp(struct nand_chip *nand_chip, unsigned command, int column, int page_addr)
{
    struct nand_chip *nand = nand_chip;
    u32 timeout;
    switch (command)
    {
      case NAND_CMD_SEQIN:
          if (g_kCMD.u4RowAddr != (u32)page_addr)
          {
              memset(g_kCMD.au1OOB, 0xFF, sizeof(g_kCMD.au1OOB));
              g_kCMD.pDataBuf = NULL;
          }
          g_kCMD.u4RowAddr = page_addr;
          g_kCMD.u4ColAddr = column;
          break;
      case NAND_CMD_PAGE_PROG:
          if (g_kCMD.pDataBuf || (0xFF != g_kCMD.au1OOB[0]))
          {
              u8 *pDataBuf = g_kCMD.pDataBuf ? g_kCMD.pDataBuf : nand->buffers->databuf;
              nand_exec_write_page(nand, g_kCMD.u4RowAddr, nand->writesize, pDataBuf, g_kCMD.au1OOB);
              g_kCMD.u4RowAddr = (u32) - 1;
              g_kCMD.u4OOBRowAddr = (u32) - 1;
          }
          break;

      case NAND_CMD_READ_OOB:
          g_kCMD.u4RowAddr = page_addr;
          g_kCMD.u4ColAddr = column + nand->writesize;
          g_i4ErrNum = 0;
          break;

      case NAND_CMD_READ_0:
          g_kCMD.u4RowAddr = page_addr;
          g_kCMD.u4ColAddr = column;
          g_i4ErrNum = 0;
          break;

      case NAND_CMD_ERASE_1:
          nand_reset();
          nand_set_mode(CNFG_OP_ERASE);
          nand_set_command(NAND_CMD_ERASE_1);
          nand_set_address(0, page_addr, 0, devinfo.addr_cycle - 2);
          break;

      case NAND_CMD_ERASE_2:
          nand_set_command(NAND_CMD_ERASE_2);
          while (DRV_Reg32(NFI_STA_REG32) & STA_NAND_BUSY) ;
          break;

      case NAND_CMD_STATUS:
          NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_BYTE_RW);
          nand_reset();
          nand_set_mode(CNFG_OP_SRD);
          nand_set_command(NAND_CMD_STATUS);
          NFI_CLN_REG32(NFI_CON_REG16, CON_NFI_NOB_MASK);
          DRV_WriteReg32(NFI_CON_REG16, CON_NFI_SRD | (1 << CON_NFI_NOB_SHIFT));
          break;

      case NAND_CMD_RESET:
          nand_reset();
          break;
      case NAND_CMD_READ_ID:
          NFI_ISSUE_COMMAND(NAND_CMD_RESET, 0, 0, 0, 0);
          timeout = TIMEOUT_4;
          while (timeout)
          {
              timeout--;
          }
          nand_reset();
          /* Disable HW ECC */
          NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_HW_ECC_EN);
          NFI_CLN_REG16(NFI_PAGEFMT_REG16, PAGEFMT_DBYTE_EN);
          NFI_SET_REG16(NFI_CNFG_REG16, CNFG_READ_EN | CNFG_BYTE_RW);
          nand_set_mode(CNFG_OP_SRD);
          nand_set_command(NAND_CMD_READ_ID);
          nand_set_address(0, 0, 1, 0);
          DRV_WriteReg32(NFI_CON_REG16, CON_NFI_SRD);
          while (DRV_Reg32(NFI_STA_REG32) & STA_DATAR_STATE) ;
          break;
      default:
          printf("[ERR] nand_command_bp : unknow command %d\n", command);
          break;
    }
}

static u_char nand_read_byte(void)
{
    /* Check the PIO bit is ready or not */
    unsigned int timeout = TIMEOUT_4;
    WAIT_NFI_PIO_READY(timeout);
    return DRV_Reg8(NFI_DATAR_REG32);
}

#if 0
static void nand_read_buf(struct nand_chip *nand, u_char * buf, int len)
{
    struct nand_chip *nand = nand;
    struct CMD *pkCMD = &g_kCMD;
    u32 u4ColAddr = pkCMD->u4ColAddr;
    u32 u4PageSize = nand->writesize;

    if (u4ColAddr < u4PageSize)
    {
        if ((u4ColAddr == 0) && (len >= u4PageSize))
        {
            nand_exec_read_page(nand, pkCMD->u4RowAddr, u4PageSize, buf, pkCMD->au1OOB);
            if (len > u4PageSize)
            {
                u32 u4Size = min(len - u4PageSize,
                                 sizeof(pkCMD->au1OOB));
                memcpy(buf + u4PageSize, pkCMD->au1OOB, u4Size);
            }
        } else
        {
            nand_exec_read_page(nand, pkCMD->u4RowAddr, u4PageSize, nand->buffers->databuf, pkCMD->au1OOB);
            memcpy(buf, nand->buffers->databuf + u4ColAddr, len);
        }
        pkCMD->u4OOBRowAddr = pkCMD->u4RowAddr;
    } else
    {
        u32 u4Offset = u4ColAddr - u4PageSize;
        u32 u4Size = min(len - u4PageSize - u4Offset, sizeof(pkCMD->au1OOB));
        if (pkCMD->u4OOBRowAddr != pkCMD->u4RowAddr)
        {
            nand_exec_read_page(nand, pkCMD->u4RowAddr, u4PageSize, nand->buffers->databuf, pkCMD->au1OOB);
            pkCMD->u4OOBRowAddr = pkCMD->u4RowAddr;
        }
        memcpy(buf, pkCMD->au1OOB + u4Offset, u4Size);
    }
    pkCMD->u4ColAddr += len;
}

static void nand_write_buf(struct nand_chip nand, const u_char * buf, int len)
{
    struct CMD *pkCMD = &g_kCMD;
    u32 u4ColAddr = pkCMD->u4ColAddr;
    u32 u4PageSize = nand->writesize;
    u32 i;

    if (u4ColAddr >= u4PageSize)
    {
        u8 *pOOB = pkCMD->au1OOB;
        u32 u4Size = min(len, sizeof(pkCMD->au1OOB));
        for (i = 0; i < u4Size; i++)
        {
            pOOB[i] &= buf[i];
        }
    } else
    {
        pkCMD->pDataBuf = (u8 *) buf;
    }
    pkCMD->u4ColAddr += len;
}
#endif
void lk_nand_irq_handler(unsigned int irq)
{
	u32 inte,sts;

	mt_irq_ack(irq);
	inte = DRV_Reg16(NFI_INTR_EN_REG16);
	sts = DRV_Reg16(NFI_INTR_REG16);
	//MSG(INT, "[lk_nand_irq_handler]irq %x enable:%x %x\n",irq,inte,sts);
	if(sts & inte){
	//	printf("[lk_nand_irq_handler]send event,\n");
		DRV_WriteReg16(NFI_INTR_EN_REG16, 0);
		DRV_WriteReg16(NFI_INTR_REG16,sts);
		event_signal(&nand_int_event,0);
	}
	return;
}

int nand_init_device(struct nand_chip *nand)
{
    int index;//j, busw,;
    u8 id[NAND_MAX_ID];
    u32 spare_bit;
    u32 spare_per_sec;
    u32 ecc_bit;

    memset(&devinfo, 0, sizeof(devinfo));
    g_bInitDone = FALSE;
    g_kCMD.u4OOBRowAddr = (u32) - 1;
#ifdef MACH_FPGA 		// FPGA NAND is placed at CS1
		DRV_WriteReg16(NFI_CSEL_REG16, 0);
#else
    DRV_WriteReg16(NFI_CSEL_REG16, NFI_DEFAULT_CS);
#endif
    DRV_WriteReg32(NFI_ACCCON_REG32, NFI_DEFAULT_ACCESS_TIMING);
    DRV_WriteReg16(NFI_CNFG_REG16, 0);
    DRV_WriteReg16(NFI_PAGEFMT_REG16, 4);
    nand_reset();

    nand->nand_ecc_mode = NAND_ECC_HW;
    nand_command_bp(&g_nand_chip, NAND_CMD_READ_ID, 0, 0);
    MSG(INFO, "NAND ID: ");
    for (index = 0; index < NAND_MAX_ID; index++)
    {
        id[index] = nand_read_byte();
        MSG(INFO, " %x", id[index]);
    }
    MSG(INFO, "\n ");
    if (!get_device_info(id, &devinfo))
    {
        MSG(ERR, "NAND unsupport\n");
        return -1;
    }

    nand->name = devinfo.devciename;
    nand->chipsize = ((u64)devinfo.totalsize) << 20;
	if(devinfo.sectorsize == 512)
    	nand->erasesize = devinfo.blocksize << 10;
	else
		nand->erasesize = (devinfo.blocksize << 10)/2;
	BLOCK_SIZE = devinfo.blocksize << 10;
    nand->phys_erase_shift = uffs(nand->erasesize) - 1;
    nand->page_size = devinfo.pagesize;
    nand->writesize = devinfo.pagesize;
    nand->page_shift = uffs(nand->page_size) - 1;
    nand->oobblock = nand->page_size;
    nand->bus16 = devinfo.iowidth;
    nand->id_length = devinfo.id_length;
	nand->sector_size = NAND_SECTOR_SIZE;
	nand->sector_shift = 9;
	if(devinfo.sectorsize == 1024)
	{
		nand->sector_size = 1024;
		nand->sector_shift = 10;
		NFI_CLN_REG32(NFI_PAGEFMT_REG16, PAGEFMT_SECTOR_SEL);
	}
    for (index = 0; index < devinfo.id_length; index++)
    {
        nand->id[index] = id[index];
    }
	#if 1
	if(devinfo.vendor != VEND_NONE)
	{
		if(devinfo.feature_set.FeatureSet.Async_timing.feature != 0xFF)
		{
		struct gFeatureSet *feature_set = &(devinfo.feature_set.FeatureSet);
			mtk_nand_SetFeature((u16) feature_set->sfeatureCmd, \
		feature_set->Async_timing.address, (u8*)&feature_set->Async_timing.feature,\
		sizeof(feature_set->Async_timing.feature));
		}
	}
	#endif
    DRV_WriteReg32(NFI_ACCCON_REG32, devinfo.timmingsetting);

	spare_per_sec = devinfo.sparesize>>(nand->page_shift-nand->sector_shift);

	switch(spare_per_sec)
	{
		case 16:
			spare_bit = PAGEFMT_SPARE_16;
			ecc_bit = 4;
			spare_per_sec = 16;
			break;
		case 26:
		case 27:
		case 28:
			spare_bit = PAGEFMT_SPARE_26;
			ecc_bit = 10;
			spare_per_sec = 26;
			break;
		case 32:
			ecc_bit = 12;
			if(devinfo.sectorsize == 1024)
				spare_bit = PAGEFMT_SPARE_32_1KS;
			else
				spare_bit = PAGEFMT_SPARE_32;
			spare_per_sec = 32;
			break;
		case 40:
			ecc_bit = 18;
			spare_bit = PAGEFMT_SPARE_40;
			spare_per_sec = 40;
			break;
		case 44:
			ecc_bit = 20;
			spare_bit = PAGEFMT_SPARE_44;
			spare_per_sec = 44;
			break;
		case 48:
		case 49:
			ecc_bit = 22;
			spare_bit = PAGEFMT_SPARE_48;
			spare_per_sec = 48;
			break;
		case 50:
		case 51:
			ecc_bit = 24;
			spare_bit = PAGEFMT_SPARE_50;
			spare_per_sec = 50;
			break;
		case 52:
		case 54:
		case 56:
			ecc_bit = 24;
			if(devinfo.sectorsize == 1024)
				spare_bit = PAGEFMT_SPARE_52_1KS;
			else
				spare_bit = PAGEFMT_SPARE_52;
			spare_per_sec = 32;
			break;
		case 62:
		case 63:
			ecc_bit = 28;
			spare_bit = PAGEFMT_SPARE_62;
			spare_per_sec = 62;
			break;
		case 64:
			ecc_bit = 32;
			if(devinfo.sectorsize == 1024)
				spare_bit = PAGEFMT_SPARE_64_1KS;
			else
				spare_bit = PAGEFMT_SPARE_64;
			spare_per_sec = 64;
			break;
		case 72:
			ecc_bit = 36;
			if(devinfo.sectorsize == 1024)
				spare_bit = PAGEFMT_SPARE_72_1KS;
			spare_per_sec = 72;
			break;
		case 80:
			ecc_bit = 40;
			if(devinfo.sectorsize == 1024)
				spare_bit = PAGEFMT_SPARE_80_1KS;
			spare_per_sec = 80;
			break;
		case 88:
			ecc_bit = 44;
			if(devinfo.sectorsize == 1024)
				spare_bit = PAGEFMT_SPARE_88_1KS;
			spare_per_sec = 88;
			break;
		case 96:
		case 98:
			ecc_bit = 48;
			if(devinfo.sectorsize == 1024)
				spare_bit = PAGEFMT_SPARE_96_1KS;
			spare_per_sec = 96;
			break;
		case 100:
		case 102:
		case 104:
			ecc_bit = 52;
			if(devinfo.sectorsize == 1024)
				spare_bit = PAGEFMT_SPARE_100_1KS;
			spare_per_sec = 100;
			break;
		case 124:
		case 126:
		case 128:
			ecc_bit = 60;
			if(devinfo.sectorsize == 1024)
				spare_bit = PAGEFMT_SPARE_124_1KS;
			spare_per_sec = 124;
			break;
		default:
			printf("[NAND]: NFI not support oobsize: %x\n", spare_per_sec);
			while(1);
			return -1;
	}

	devinfo.sparesize = spare_per_sec<<(nand->page_shift-nand->sector_shift);
	MSG(INFO, "[NAND]nand eccbit %d , sparesize %d\n",ecc_bit,devinfo.sparesize);
	if(!devinfo.sparesize){
    		nand->oobsize = (8 << ((id[3] >> 2) & 0x01)) * (nand->oobblock / nand->sector_size); //FIX ME ,kai
	}else{
		nand->oobsize = devinfo.sparesize;
	}
    nand->buffers = &nBuf;//malloc(sizeof(struct nand_buffers));
    if (nand->bus16 == IO_WIDTH_16)
    {
        NFI_SET_REG16(NFI_PAGEFMT_REG16, PAGEFMT_DBYTE_EN);
        nand->options |= NAND_BUSWIDTH_16;
    }

	if (16384 == nand->oobblock)
    {
        NFI_SET_REG16(NFI_PAGEFMT_REG16, (spare_bit << PAGEFMT_SPARE_SHIFT) | PAGEFMT_16K_1KS);
        nand->ecclayout = &nand_oob_128;
    } else if (8192 == nand->oobblock)
    {
        NFI_SET_REG16(NFI_PAGEFMT_REG16, (spare_bit << PAGEFMT_SPARE_SHIFT) | PAGEFMT_8K_1KS);
        nand->ecclayout = &nand_oob_128;
    } else if (4096 == nand->oobblock)
    {
        if(devinfo.sectorsize == 512)
	        NFI_SET_REG16(NFI_PAGEFMT_REG16, (spare_bit << PAGEFMT_SPARE_SHIFT) | PAGEFMT_4K);
	    else
			NFI_SET_REG16(NFI_PAGEFMT_REG16, (spare_bit << PAGEFMT_SPARE_SHIFT) | PAGEFMT_4K_1KS);
        nand->ecclayout = &nand_oob_128;
    } else if (2048 == nand->oobblock)
    {
     	if(devinfo.sectorsize == 512)
          	NFI_SET_REG16(NFI_PAGEFMT_REG16, (spare_bit << PAGEFMT_SPARE_SHIFT) | PAGEFMT_2K);
		else
			NFI_SET_REG16(NFI_PAGEFMT_REG16, (spare_bit << PAGEFMT_SPARE_SHIFT) | PAGEFMT_2K_1KS);
       nand->ecclayout = &nand_oob_64;
    }    

    if (nand->nand_ecc_mode == NAND_ECC_HW)
    {
        NFI_SET_REG32(NFI_CNFG_REG16, CNFG_HW_ECC_EN);
        ECC_Config(ecc_bit);
        nand_configure_fdm(NAND_FDM_PER_SECTOR);
    }
    DRV_Reg16(NFI_INTR_REG16);
    DRV_WriteReg16(NFI_INTR_EN_REG16, 0);
	if(en_interrupt){
		event_init(&nand_int_event,false,EVENT_FLAG_AUTOUNSIGNAL);
		mt_irq_set_sens(MT_NFI_IRQ_ID, MT65xx_EDGE_SENSITIVE);
		mt_irq_set_polarity(MT_NFI_IRQ_ID, MT65xx_POLARITY_LOW);
		mt_irq_unmask(MT_NFI_IRQ_ID);
	}
	if(devinfo.vendor != VEND_NONE)
	{
		mtk_nand_randomizer_config(&devinfo.feature_set.randConfig);
		DRV_WriteReg32(NFI_DLYCTRL_REG32,0x8001);
		DRV_WriteReg32(NFI_DQS_DELAY_CTRL,0x000F0000); //temp
		DRV_WriteReg32(NFI_DQS_DELAY_MUX,0x3); //temp
	}
	
    g_nand_size = nand->chipsize;
    #if defined(MTK_COMBO_NAND_SUPPORT)
    nand->chipsize -= (PART_SIZE_BMTPOOL);
    #else
    nand->chipsize -= BLOCK_SIZE * (BMT_POOL_SIZE);
    #endif
   // nand->chipsize -= nand->erasesize * (PMT_POOL_SIZE);
   
    g_bInitDone = true;
    if (!g_bmt)
    {
    		#if defined(MTK_COMBO_NAND_SUPPORT)
        if (!(g_bmt = init_bmt(nand, (PART_SIZE_BMTPOOL)/BLOCK_SIZE)))
        #else
        if (!(g_bmt = init_bmt(nand, BMT_POOL_SIZE)))
        #endif
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
	total_size = t_nand->chipsize - BLOCK_SIZE * (PMT_POOL_SIZE);
        dev.id = 0;
        dev.init = 1;
        dev.blkdev = (block_dev_desc_t *) t_nand;
        dev.read = nand_part_read;
        dev.write = nand_part_write;
        mt_part_register_device(&dev);
        printf("NAND register done in LK\n");
        return;
    } else
    {
        printf("NAND init fail in LK\n");
    }

}

void nand_driver_test(void){
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
    if(fail==0){
	printf("compare success!\n");
    }
    len = dev->write(dev, (uchar *) original, start_addr, test_len);
    if (len != test_len)
    {
        printf("write back fail %d\n", len);
    } else
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
//    u32 tpgsz;
    u32 tblksz;
    u64 cur_offset;
//    u32 i = 0;
	u32 index;
	u32 block_size;
	
    // do block alignment check
    
	///printf ("[ERASE] offset = 0x%x\n", (u32)offset);
	part_get_startaddress(offset, &index);
	//printf ("[ERASE] index = %d\n", index);
	if(raw_partition(index))
	{
		printf ("[ERASE] raw TRUE\n");
		block_size = BLOCK_SIZE/2;
	}
	else
	{
		block_size = BLOCK_SIZE;
	}
	if ((u32)(offset % block_size) != 0)
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
    	//printf ("[ERASE] cur_offset = 0x%x\n", cur_offset);
        if (__nand_erase(cur_offset) == FALSE)
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
    u32 block;
    u32 mapped_block;
	mtk_nand_page_transform(logical_addr,&block,&mapped_block);
    if (!nand_erase_hw(mapped_block * BLOCK_SIZE))
    {
        printf("erase block 0x%x failed\n", mapped_block);
        if(update_bmt((u64)mapped_block * BLOCK_SIZE, UPDATE_ERASE_FAIL, NULL, NULL)){
			printf("erase block fail and update bmt sucess\n");
			return TRUE;
		}else{
			printf("erase block 0x%x failed but update bmt fail\n",mapped_block);
			return FALSE;
		}
    }

    return TRUE;
}
static int erase_fail_test = 0;
bool nand_erase_hw (u64 offset)
{
    bool bRet = TRUE;
//    u32 timeout, u4SecNum = g_nand_chip.oobblock >> g_nand_chip.sector_shift;
    u32 rownob = devinfo.addr_cycle - 2;
    u32 page_addr = offset / g_nand_chip.oobblock;

    if (nand_block_bad_hw(&g_nand_chip,offset))
    {
        return FALSE;
    }
	if(erase_fail_test){
		erase_fail_test = 0;
		return FALSE;
	}
    nand_reset ();
    nand_set_mode (CNFG_OP_ERASE);
    nand_set_command (NAND_CMD_ERASE_1);
    nand_set_address (0, page_addr, 0, rownob);

    nand_set_command (NAND_CMD_ERASE_2);
    if (!nand_status_ready(STA_NAND_BUSY))
    {
        return FALSE;
    }
    return bRet;
}

bool mark_block_bad_hw(u64 offset)
{
    u32 index;
    u32 page_addr = offset / g_nand_chip.oobblock;
    u32 u4SecNum = g_nand_chip.oobblock >> g_nand_chip.sector_shift;
    u32 i, page_num = (BLOCK_SIZE / g_nand_chip.oobblock);

    memset(data_buf_temp,0xAA,4096);

    for (index = 0; index < 64; index++)
        *(oob_buf_temp+ index) = 0xFF;

    for (index = 8, i = 0; i < u4SecNum; i++)
        oob_buf_temp[i * index] = 0x0;

    page_addr &= ~(page_num - 1);
    MSG (INIT, "Mark bad block at 0x%x\n", page_addr);
    while (DRV_Reg32 (NFI_STA_REG32) & STA_NAND_BUSY);

	return nand_exec_write_page_raw(&g_nand_chip, page_addr, g_nand_chip.oobblock, data_buf_temp,oob_buf_temp);
}

bool mark_block_bad (u64 logical_addr)
{
    //u32 block;
    //u32 mapped_block;
	//mtk_nand_page_transform(logical_addr,&block,&mapped_block);
    return mark_block_bad_hw((u64)logical_addr);
}

int nand_write_page_hw(u32 page, u8 *dat, u8 *oob)
{
//    u32 pagesz = g_nand_chip.oobblock;
//    u32 u4SecNum = pagesz >> g_nand_chip.sector_shift;

    int i, j, start, len;
    bool empty = TRUE;
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
                empty = FALSE;
        }
    }

    if (!empty)
    {
        oob[g_nand_chip.ecclayout->oobfree[i-1].offset + g_nand_chip.ecclayout->oobfree[i-1].length] = oob_checksum;
    }

    while (DRV_Reg32 (NFI_STA_REG32) & STA_NAND_BUSY);
	return nand_exec_write_page_raw(&g_nand_chip, page, g_nand_chip.oobblock, (u8 *)dat,(u8 *)oob);

  }
int nand_write_page_hwecc (u64 logical_addr, char *buf, char *oob_buf)
{
//	u32 page_size = g_nand_chip.oobblock;
//	u32 block_size = BLOCK_SIZE;
	u32 block;
    u32 mapped_block;
//	u32 pages_per_blk = (block_size/page_size);
	u32 page_no;
    //u32 page_in_block = (logical_addr/page_size)%pages_per_blk;
    u32 i;
    int start, len, offset;
	page_no = mtk_nand_page_transform(logical_addr,&block,&mapped_block);
    for (i = 0; i < sizeof(g_spare_buf); i++)
        *(g_spare_buf + i) = 0xFF;

    offset = 0;

	if(oob_buf != NULL){
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

    if (!nand_write_page_hw(page_no,(u8*)buf, g_spare_buf))
    {
        MSG(INIT, "write fail happened @ block 0x%x, page 0x%x\n", mapped_block, page_no);
        return update_bmt( (u64)page_no * g_nand_chip.oobblock,
                UPDATE_WRITE_FAIL, (u8*)buf, g_spare_buf);
    }

    return TRUE;
}
#if defined(MTK_MLC_NAND_SUPPORT)
int nand_write_img(u64 addr, void *data, u32 img_sz,u64 partition_size,int img_type)
#else
int nand_write_img(u32 addr, void *data, u32 img_sz,u32 partition_size,int img_type)
#endif
{
	unsigned int page_size = g_nand_chip.oobblock;
	unsigned int img_spare_size,write_size;
	unsigned int block_size;
	u64 partition_end = (u64)addr + partition_size;
	bool ret;
	u32 index;
	unsigned int b_lastpage = 0;
	printf("[nand_wite_img]write to img size, %x addr %lx\n",img_sz,addr);

	part_get_startaddress((u64)addr, &index);
	printf ("[nand_write_img] index = %d\n", index);
	if(raw_partition(index))
	{
		block_size = BLOCK_SIZE/2;
	}
	else
	{
		block_size = BLOCK_SIZE;
	}
	
	if(addr % block_size || partition_size % block_size){
		printf("[nand_write_img]partition address or partition size is not block size alignment\n");
		return -1;
	}
	if(img_sz > partition_size){
		printf("[nand_write_img]img size %x exceed partition size\n",img_sz);
		return -1;
	}
	
	if(page_size == 4096){
		img_spare_size = 128;
	}else if(page_size == 2048){
		img_spare_size = 64;
	}

	if(img_type == YFFS2_IMG){
		write_size = page_size + img_spare_size;

		if(img_sz % write_size){
			printf("[nand_write_img]img size is not w_size %d alignment\n",write_size);
			return -1;
		}
	}else{
		write_size = page_size;
	/*	if(img_sz % write_size){
			printf("[nand_write_img]img size is not w_size %d alignment\n",write_size);
			return -1;
		}*/
	}

	while(img_sz>0){

		if((addr+img_sz)>partition_end){
			printf("[nand_wite_img]write to addr %x,img size %x exceed parition size,may be so many bad blocks\n",addr,img_sz);
			return -1;
		}

		/*1. need to erase before write*/
		if((addr % block_size)==0){
			if (__nand_erase((u64)addr) == FALSE)
     		{
	            printf("[ERASE] erase 0x%x fail\n",addr);
	            mark_block_bad ((u64)addr);
           		addr += block_size;
				continue;  //erase fail, skip this block
       	 	}
		}
		/*2. write page*/
		if((img_sz < write_size)){
			b_lastpage = 1;
			memset(g_data_buf,0xff,write_size);
			memcpy(g_data_buf,data,img_sz);
			if(img_type == YFFS2_IMG){
				ret = nand_write_page_hwecc((u64)addr,(char *)g_data_buf,(char *)g_data_buf+page_size);
			}else{
				if((img_type == UBIFS_IMG)&& (check_data_empty((void *)g_data_buf,page_size))){
					printf("[nand_write_img]skip empty page\n");
					ret = true;
				}else{
				ret = nand_write_page_hwecc((u64)addr,(char*)g_data_buf,NULL);
			}
			}
		}else{
			if(img_type == YFFS2_IMG){
				ret = nand_write_page_hwecc((u64)addr,data,data+page_size);
			}else{
				if((img_type == UBIFS_IMG)&& (check_data_empty((void *)data,page_size))){
					printf("[nand_write_img]skip empty page\n");
					ret = true;
				}else{
				ret = nand_write_page_hwecc((u64)addr,data,NULL);
			}
		}
		}
		if(ret == FALSE){
			printf("[nand_write_img]write fail at % 0x%x\n",addr);
			if (__nand_erase((u64)addr) == FALSE)
     		{
	            printf("[ERASE] erase 0x%x fail\n",addr);
	            mark_block_bad ((u64)addr);
       	 	}
			data -= ((addr%block_size)/page_size)*write_size;
			img_sz += ((addr%block_size)/page_size)*write_size;
			addr += block_size;
			continue;  // write fail, try  to write the next block
		}
		if(b_lastpage){
			data += img_sz;
			img_sz = 0 ;
			addr += page_size;
		}else{
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
#if defined(MTK_MLC_NAND_SUPPORT)
int nand_write_img_ex(u64 addr, void *data, u32 length,u64 total_size, u32 *next_offset, u64 partition_start,u64 partition_size, int img_type)
#else
int nand_write_img_ex(u32 addr, void *data, u32 length,u32 total_size, u32 *next_offset, u32 partition_start,u32 partition_size, int img_type)
#endif
{
	unsigned int page_size = g_nand_chip.oobblock;
	unsigned int img_spare_size = 0;
	unsigned int write_size;
	unsigned int block_size = BLOCK_SIZE;
	u64 partition_end = partition_start + partition_size;
//	unsigned int first_chunk = 0;
	unsigned int last_chunk = 0;
	unsigned int left_size = 0;
	bool ret;
	u32 index;
	u64 last_addr = (u64)addr;
	u32 dst_block = 0;
  part_get_startaddress((u64)addr, &index);
	printf("[nand_write_img_ex]write to img_type %d, addr %lx,img size %lx partition_start %lx\n",img_type,addr,length,partition_start);
	if(raw_partition(index))
	{
		block_size = BLOCK_SIZE/2;
	}
	else
	{
		block_size = BLOCK_SIZE;
	}
	if(partition_start % block_size || partition_size % block_size){
		printf("[nand_write_img_ex]partition address or partition size is not block size alignment %lx,%lx\n",partition_start, partition_size);
		return -1;
	}
	if(length > partition_size){
		printf("[nand_write_img_ex]img size %x exceed partition size\n",length);
		return -1;
	}


	if(page_size == 4096){
		img_spare_size = 128;
	}else if(page_size == 2048){
		img_spare_size = 64;
	}
    if(last_addr % page_size){
		printf("[nand_write_img_ex]write addr is not page_size %d alignment\n",page_size);
		return -1;
	}
	if(img_type == YFFS2_IMG){
		write_size = page_size + img_spare_size;
		if(total_size % write_size){
			printf("[nand_write_img_ex]total image size %d is not w_size %d alignment\n",total_size,write_size);
			return -1;
		}
	}else{
		write_size = page_size;
	}
	if(addr == partition_start){
		printf("[nand_write_img_ex]first chunk\n");
//		first_chunk = 1;
		download_size = 0;
		memset(g_data_buf,0xff,write_size);
	}
	if((length + download_size) >= total_size){
		printf("[nand_write_img_ex]last chunk\n");
		last_chunk = 1;
	}

	left_size = (download_size % write_size);

	while(length>0){

		if((addr+length)>partition_end){
			printf("[nand_write_img_ex]write to addr %x,img size %x exceed parition size,may be so many bad blocks\n",addr,length);
			return -1;
		}

		/*1. need to erase before write*/
		if((addr % block_size)==0){
			if (__nand_erase((u64)addr) == FALSE)
     		{
	            printf("[ERASE] erase 0x%x fail\n",addr);
	            mark_block_bad ((u64)addr);
           		addr += block_size;
				continue;  //erase fail, skip this block
       	 	}
		}
		if((length < write_size)&&(!left_size)){
			memset(g_data_buf,0xff,write_size);
			memcpy(g_data_buf,data,length);

			if(!last_chunk){
				download_size += length;
				break;
			}
		}else if(left_size){
			memcpy(&g_data_buf[left_size],data,write_size-left_size);

		}else{
			memcpy(g_data_buf,data,write_size);
		}

		/*2. write page*/

		if(img_type == YFFS2_IMG){
			ret = nand_write_page_hwecc((u64)addr,(char *)g_data_buf,(char *)g_data_buf + page_size);
		}else{
			if((img_type == UBIFS_IMG)&& (check_data_empty((void *)g_data_buf,page_size))){
				printf("[nand_write_img_ex]skip empty page\n");
				ret = true;
			}
			else{
					ret = nand_write_page_hwecc((u64)addr,(char *)g_data_buf,NULL);
			}
		}
		/*need to check?*/
		if(ret == FALSE){
			printf("[nand_write_img_ex]write fail at % 0x%x\n",addr);
			while(1){
				dst_block = find_next_good_block((u64)addr/block_size);
				if(dst_block == 0)
				{
					printf("[nand_write_img_ex]find next good block fail\n");
					return -1;
				}
				ret = block_replace((u64)addr/block_size,dst_block,(u64)addr/page_size);
				if(ret == FALSE){
					printf("[nand_write_img_ex]block replace fail,continue\n");
					continue;
				}else{
					printf("[nand_write_img_ex]block replace sucess %x--> %x\n",addr/block_size,dst_block);
					break;
				}

			}
			addr = (addr%block_size) + (dst_block*block_size);
		/*	if (__nand_erase(addr) == FALSE)
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
		else{
			data += write_size;
			length -= write_size;
			addr += page_size;
			download_size += write_size;
		}
	}
	*next_offset = addr - last_addr;
	if(last_chunk){
		/*3. erase any block remained in partition*/
		addr = ((addr+block_size-1)/block_size)*block_size;

		nand_erase((u64)addr,(u64)(partition_end - addr));
	}
	return 0;
}

int check_data_empty(void *data, unsigned size)
{
		unsigned int i;
		u32 *tp = (u32 *)data;

		for(i =0;i<size/4;i++){
			if(*(tp+i) != 0xffffffff){
				return 0;
			}
		}
		return 1;
}

static u32 find_next_good_block(u32 start_block)
{
	u32 i;
	u32 dst_block = 0;
	for(i=start_block;i<(total_size/BLOCK_SIZE);i++)
	{
		if(!nand_block_bad(&g_nand_chip,i*(BLOCK_SIZE/g_nand_chip.page_size))){
			dst_block = i;
			break;
		}
	}
	return dst_block;
}

static bool block_replace(u32 src_block, u32 dst_block, u32 error_page)
{
	bool ret;
	u32 block_size = BLOCK_SIZE;
	u32 page_size = g_nand_chip.page_size;
	u32 i;
	u8 *data_buf;
	u8 *spare_buf;
	ret = __nand_erase((u64)dst_block*block_size);
	if(ret == FALSE){
		printf("[block_replace]%x-->%x erase fail\n",src_block,dst_block);
		mark_block_bad((u64)src_block*block_size);
		return ret;
	}
	data_buf = malloc(16384);
	spare_buf = malloc(1024);
	if(!data_buf || !spare_buf){
		printf("[block_replace]malloc mem fail\n");
		return -1;
	}

	memset(data_buf,0xff,16384);
	memset(spare_buf,0xff,1024);
	for(i=0;i<error_page;i++)
	{
		nand_exec_read_page(&g_nand_chip,src_block*(block_size/page_size) + i,page_size,data_buf,spare_buf);
		ret = nand_write_page_hwecc((u64)dst_block*block_size + i*page_size,(char*)data_buf,(char*)spare_buf);
		if(ret == FALSE)
			mark_block_bad((u64)dst_block*block_size);
	}

	mark_block_bad((u64)src_block*block_size);
	free(data_buf);
	free(spare_buf);
	return ret;

}
// Add for Get DL information
#define PRE_SCAN_BLOCK_NUM 20
/*Support to check format/download status, 2013/01/19 {*/
/* Max Number of Load Sections */
#define MAX_LOAD_SECTIONS		40
#define DL_MAGIC "DOWNLOAD INFORMATION!!"
#define DL_INFO_VER_V1  "V1.0"

#define DL_MAGIC_NUM_COUNT 32
#define DL_MAGIC_OFFSET 24
#define DL_IMG_NAME_LENGTH 16
#define DL_CUSTOM_INFO_SIZE (128)

/*download status v1 and old version for emmc*/
#define FORMAT_START    "FORMAT_START"
#define FORMAT_DONE      "FORMAT_DONE"
#define BL_START             "BL_START"
#define BL_DONE              "BL_DONE"
#define DL_START            "DL_START"
#define DL_DONE             "DL_DONE"
#define DL_ERROR           "DL_ERROR"
#define DL_CK_DONE       "DL_CK_DONE"
#define DL_CK_ERROR      "DL_CK_ERROR"

/*v1 and old version for emmc*/
#define CHECKSUM_PASS "PASS"
#define CHECKSUM_FAIL "FAIL"

typedef enum {
	DL_INFO_VERSION_V0 = 0,
	DL_INFO_VERSION_V1 = 1,
	DL_INFO_VERSION_UNKOWN = 0xFF,
} DLInfoVersion;

/*version v1.0 {*/
typedef struct {
    char image_name[DL_IMG_NAME_LENGTH];
} IMG_DL_INFO;

typedef struct {
    unsigned int image_index;
    unsigned int pc_checksum;
    unsigned int da_checksum;
    char   checksum_status[8];
} CHECKSUM_INFO_V1;

typedef struct {
    char          magic_num[DL_MAGIC_OFFSET];
    char		  version[DL_MAGIC_NUM_COUNT-DL_MAGIC_OFFSET];
    CHECKSUM_INFO_V1 part_info[MAX_LOAD_SECTIONS];
    char          ram_checksum[16];
    char          download_status[16];
    IMG_DL_INFO   img_dl_info[MAX_LOAD_SECTIONS];
} DL_STATUS_V1;
/*version v1.0 }*/
/*Support to check format/download status, 2013/01/19 {*/

#define DL_NOT_FOUND 2
#define DL_PASS 0
#define DL_FAIL 1

int nand_get_dl_info(void)
{
	DL_STATUS_V1 download_info;
	u8 *data_buf;
	u8 *spare_buf;
    int ret;
	u32 block_size = BLOCK_SIZE;
	u32 page_size = g_nand_chip.page_size;
    u32 pages_per_block = block_size/page_size;
    u32 total_blocks = g_nand_size/BLOCK_SIZE;
    u32 i,block_i,page_i;
    u32 block_addr;
    u32 dl_info_blkAddr = 0xFFFFFFFF;
    u32 page_index[4];

	data_buf = malloc(16384);
	spare_buf = malloc(1024);
	if(!data_buf || !spare_buf){
		printf("[nand_get_dl_info]malloc mem fail\n");
		ret = -1;
		return ret;
	}


  // DL information block should program to good block instead of always at last block.
  page_index[0] = 0;
  page_index[1] = 1;
  page_index[2] = pages_per_block-3;
  page_index[3] = pages_per_block-1;

  block_i = 1;
  do{
    block_addr = pages_per_block * (total_blocks-block_i);
    for(page_i=0; page_i<4; page_i++)
    {
        nand_exec_read_page(&g_nand_chip,block_addr+page_index[page_i],page_size,data_buf,spare_buf);
        ret = memcmp((void*)data_buf,DL_MAGIC, sizeof(DL_MAGIC));
        if(!ret){
          dl_info_blkAddr = block_addr;
          break;
        }
    }
    if(dl_info_blkAddr!=0xFFFFFFFF)
    {
      break;
    }
    block_i++;
  }while(block_i<=PRE_SCAN_BLOCK_NUM);
  if(dl_info_blkAddr==0xFFFFFFFF)
  {
    printf("DL INFO NOT FOUND\n");
    ret = DL_NOT_FOUND;
  }
  else
  {
    printf("get dl info from 0x%x\n",dl_info_blkAddr);

  	memcpy(&download_info,data_buf,sizeof(download_info));
  	if(!memcmp(download_info.download_status,DL_DONE,sizeof(DL_DONE))||!memcmp(download_info.download_status,DL_CK_DONE,sizeof(DL_CK_DONE)))
  	{
  		printf("dl done. status = %s\n",download_info.download_status);
  		printf("dram checksum : %s\n",download_info.ram_checksum);
  		for(i=0;i<PART_MAX_COUNT;i++)
  		{
  			if(download_info.part_info[i].image_index!=0){
  				printf("image_index:%d, checksum: %s\n",download_info.part_info[i].image_index,download_info.part_info[i].checksum_status);
  			}
  		}
  		ret = DL_PASS;
  	}else
  	{
  		printf("dl error. status = %s\n",download_info.download_status);
  		printf("dram checksum : %s\n",download_info.ram_checksum);
  		for(i=0;i<PART_MAX_COUNT;i++)
  		{
  			if(download_info.part_info[i].image_index!=0){
  				printf("image_index:%d, checksum: %s\n",download_info.part_info[i].image_index,download_info.part_info[i].checksum_status);
  			}
  		}
  		ret = DL_FAIL;
  	}
  }
  free(data_buf);
  free(spare_buf);

  return ret;
}

u32 mtk_nand_erasesize(void)
{
	return g_nand_chip.erasesize;
}

#endif
