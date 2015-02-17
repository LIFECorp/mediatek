#include <linux/kernel.h>
#include <asm/io.h>

#if 1
#include "mt_reg_base.h"
#include "mt_emi_bm.h"
#include "sync_write.h"
#include "mt_typedefs.h"
#include "mt_mci.h"
#else
#include <mach/mt_reg_base.h>
#include <mach/mt_emi_bm.h>
#include <mach/sync_write.h>
#include <mach/mt_typedefs.h>
#include <mach/mt_mci.h>
#endif
static unsigned char g_cBWL;

/* return 1X emi freq : unit Khz */
// TODO: re-check
//unsigned int MET_BM_get_emi_freq(void)
//{
//        unsigned int emi_freq = 0;
//	unsigned int uplmt_10, dnlmt_10;
//
//	mt_fh_query_SSC_boundary(MAINPLL_ID, &uplmt_10, &dnlmt_10);   //10* %
//	emi_freq = mt_get_emi_freq();  //Unit Khz for 2x EMI clock
//
//	emi_freq = emi_freq * (1000 - (dnlmt_10>>1));
//        emi_freq = ((emi_freq + 500)/1000) >> 1;
//
//        return emi_freq;
//}

void MET_BM_Init(void)
{
    g_cBWL = 0;

    /*
    * make sure BW limiter counts consumed Soft-mode bandwidth of each master
    */
    if (readl(EMI_ARBA) & 0x00008000) {
        g_cBWL |= 1 << 0;
        writel(readl(EMI_ARBA) & ~0x00008000, EMI_ARBA);
    }

    if (readl(EMI_ARBB) & 0x00008000) {
        g_cBWL |= 1 << 1;
        writel(readl(EMI_ARBB) & ~0x00008000, EMI_ARBB);
    }

    if (readl(EMI_ARBC) & 0x00008000) {
        g_cBWL |= 1 << 2;
        writel(readl(EMI_ARBC) & ~0x00008000, EMI_ARBC);
    }

    if (readl(EMI_ARBD) & 0x00008000) {
        g_cBWL |= 1 << 3;
        writel(readl(EMI_ARBD) & ~0x00008000, EMI_ARBD);
    }

    if (readl(EMI_ARBE) & 0x00008000) {
        g_cBWL |= 1 << 4;
        writel(readl(EMI_ARBE) & ~0x00008000, EMI_ARBE);
    }

    if (readl(EMI_ARBF) & 0x00008000) {
        g_cBWL |= 1 << 5;
        writel(readl(EMI_ARBF) & ~0x00008000, EMI_ARBF);
    }
    /* Select EMI_CLOCK_CNT for EMI_TSCT , Enable bus monitor clock*/
    writel(readl(EMI_BMEN) |0x00000880, EMI_BMEN);
}

void MET_BM_DeInit(void)
{
    if (g_cBWL & (1 << 0)) {
        g_cBWL &= ~(1 << 0);
        writel(readl(EMI_ARBA) | 0x00008000, EMI_ARBA);
    }

    if (g_cBWL & (1 << 1)) {
        g_cBWL &= ~(1 << 1);
        writel(readl(EMI_ARBB) | 0x00008000, EMI_ARBB);
    }

    if (g_cBWL & (1 << 2)) {
        g_cBWL &= ~(1 << 2);
        writel(readl(EMI_ARBC) | 0x00008000, EMI_ARBC);
    }

    if (g_cBWL & (1 << 3)) {
        g_cBWL &= ~(1 << 3);
        writel(readl(EMI_ARBD) | 0x00008000, EMI_ARBD);
    }

    if (g_cBWL & (1 << 4)) {
        g_cBWL &= ~(1 << 4);
        writel(readl(EMI_ARBE) | 0x00008000, EMI_ARBE);
    }

    if (g_cBWL & (1 << 5)) {
        g_cBWL &= ~(1 << 5);
        writel(readl(EMI_ARBF) | 0x00008000, EMI_ARBF);
    }

    /* Disable bus monitor clock*/
    writel(readl(EMI_BMEN) & (~0x00000080), EMI_BMEN);
}

void MET_BM_Enable(const unsigned int enable)
{
    const unsigned int value = readl(EMI_BMEN);

    mt65xx_reg_sync_writel((value & ~(BUS_MON_PAUSE | BUS_MON_EN)) | (enable ? BUS_MON_EN : 0), EMI_BMEN);
}

/*
void MET_BM_Disable(void)
{
    const unsigned int value = readl(EMI_BMEN);

    mt65xx_reg_sync_writel(value & (~BUS_MON_EN), EMI_BMEN);
}
*/

void MET_BM_Pause(void)
{
    const unsigned int value = readl(EMI_BMEN);

    mt65xx_reg_sync_writel(value | BUS_MON_PAUSE, EMI_BMEN);
}

void MET_BM_Continue(void)
{
    const unsigned int value = readl(EMI_BMEN);

    mt65xx_reg_sync_writel(value & (~BUS_MON_PAUSE), EMI_BMEN);
}

unsigned int MET_BM_IsOverrun(void)
{
    /*
    * return !0 if EMI_BCNT(bus cycle counts) or EMI_WACT(total word counts) is overrun,
    * otherwise non-overrun return an 0 value
    */
    const unsigned int value = readl(EMI_BMEN);

    return (value & BC_OVERRUN);
}

void MET_BM_SetReadWriteType(const unsigned int ReadWriteType)
{
    const unsigned int value = readl(EMI_BMEN);

    /*
    * ReadWriteType: 00/11 --> both R/W
    *                   01 --> only R
    *                   10 --> only W
    */
    mt65xx_reg_sync_writel((value & 0xFFFFFFCF) | (ReadWriteType << 4), EMI_BMEN);
}

int MET_BM_GetBusCycCount(void)
{
    return MET_BM_IsOverrun() ? BM_ERR_OVERRUN : readl(EMI_BCNT);
}

unsigned int MET_BM_GetTransAllCount(void)
{
    return readl(EMI_TACT);
}

int MET_BM_GetTransCount(const unsigned int counter_num)
{
    unsigned int iCount;

    switch (counter_num) {
    case 1:
        iCount = readl(EMI_TSCT);
        break;

    case 2:
        iCount = readl(EMI_TSCT2);
        break;

    case 3:
        iCount = readl(EMI_TSCT3);
        break;

    case 4:
        iCount = readl(EMI_TSCT4);
        break;

    case 5:
        iCount = readl(EMI_TSCT5);
        break;

    default:
        return BM_ERR_WRONG_REQ;
    }

    return iCount;
}

unsigned int MET_BM_GetEMIClockCount(void)
{
	return readl(EMI_TSCT);
}

int MET_BM_GetWordAllCount(void)
{
    return MET_BM_IsOverrun() ? BM_ERR_OVERRUN : readl(EMI_WACT);
}

int MET_BM_GetWordCount(const unsigned int counter_num)
{
    unsigned int iCount;

    switch (counter_num) {

    case 1:
        iCount = readl(EMI_WSCT);
        break;

    case 2:
        iCount = readl(EMI_WSCT2);
        break;

    case 3:
        iCount = readl(EMI_WSCT3);
        break;

    case 4:
        iCount = readl(EMI_WSCT4);
        break;

    case 5:
        iCount = readl(EMI_WSCT5);
        break;

    default:
        return BM_ERR_WRONG_REQ;
    }

    return iCount;
}

unsigned int MET_BM_GetBandwidthWordCount(void)    // CM [20121101] wait for implement
{
    return readl(EMI_BACT);
}

unsigned int MET_BM_GetOverheadWordCount(void)   // CM [20121101] wait for implement
{
    return readl(EMI_BSCT);
}

unsigned int MET_BM_GetBusBusyAllCount(void)
{
    return readl(EMI_BACT);
}

unsigned int MET_BM_GetBusBusyCount(const unsigned int counter_num)
{
    unsigned int iCount;

    switch (counter_num) {
    case 1:
        iCount = readl(EMI_BSCT);
        break;

    case 2:
        iCount = readl(EMI_BSCT2);
        break;

    case 3:
        iCount = readl(EMI_BSCT3);
        break;

    case 4:
        iCount = readl(EMI_BSCT4);
        break;

    case 5:
        iCount = readl(EMI_BSCT5);
        break;

    default:
        return BM_ERR_WRONG_REQ;
    }

    return iCount;
}

int MET_BM_GetTransTypeCount(const unsigned int counter_num)
{
    return (counter_num < 1 || counter_num > BM_COUNTER_MAX) ? BM_ERR_WRONG_REQ : readl(EMI_TTYPE1 + (counter_num - 1) * 8);
}

int MET_BM_SetMonitorCounter(const unsigned int counter_num, const unsigned int master, const unsigned int trans_type)
{
    unsigned int value, addr;
    const unsigned int iMask = 0xFFFF;

    if (counter_num < 1 || counter_num > BM_COUNTER_MAX) {
        return BM_ERR_WRONG_REQ;
    }

    if (counter_num == 1) {
        addr = EMI_BMEN;
        value = (readl(addr) & ~(iMask << 16)) | ((trans_type & 0xFF) << 24) | ((master & 0xFF) << 16);
    } else {
        addr = (counter_num <= 3) ? EMI_MSEL : (EMI_MSEL2 + (counter_num / 2 - 2) * 8);

        // clear master and transaction type fields
        value = readl(addr) & ~(iMask << ((counter_num % 2) * 16));

        // set master and transaction type fields
        value |= (((trans_type & 0xFF) << 8) | (master & 0xFF)) << ((counter_num % 2) * 16);
    }

    mt65xx_reg_sync_writel(value, addr);

    return BM_REQ_OK;
}

int MET_BM_SetMaster(const unsigned int counter_num, const unsigned int master)
{
    unsigned int value, addr;
    const unsigned int iMask = 0xFF;

    if (counter_num < 1 || counter_num > BM_COUNTER_MAX) {
        return BM_ERR_WRONG_REQ;
    }

    if (counter_num == 1) {
        addr = EMI_BMEN;
        value = (readl(addr) & ~(iMask << 16)) | ((master & iMask) << 16);
    }else {
        addr = (counter_num <= 3) ? EMI_MSEL : (EMI_MSEL2 + (counter_num / 2 - 2) * 8);

        // clear master and transaction type fields
        value = readl(addr) & ~(iMask << ((counter_num % 2) * 16));

        // set master and transaction type fields
        value |= ((master & iMask) << ((counter_num % 2) * 16));
    }

    mt65xx_reg_sync_writel(value, addr);

    return BM_REQ_OK;
}

int MET_BM_SetIDSelect(const unsigned int counter_num, const unsigned int id, const unsigned int enable)
{
    unsigned int value, addr, shift_num;

    if ((counter_num < 1 || counter_num > BM_COUNTER_MAX)
        || (id > 0xFF)
        || (enable > 1)) {
        return BM_ERR_WRONG_REQ;
    }

    addr = EMI_BMID0 + (counter_num - 1) / 4 * 8;

    // field's offset in the target EMI_BMIDx register
    shift_num = ((counter_num - 1) % 4) * 8;

    // clear SELx_ID field
    value = readl(addr) & ~(0xFF << shift_num);

    // set SELx_ID field
    value |= id << shift_num;

    mt65xx_reg_sync_writel(value, addr);

    value = (readl(EMI_BMID5) & ~(1 << (counter_num - 1 + 7))) | (enable << (counter_num - 1 + 7));
    mt65xx_reg_sync_writel(value, EMI_BMID5);

    return BM_REQ_OK;
}

int MET_BM_SetUltraHighFilter(const unsigned int counter_num, const unsigned int enable) // CM [20121102] wait for implement
{
    unsigned int value;

    if ((counter_num < 1 || counter_num > BM_COUNTER_MAX)
        || (enable > 1)) {
        return BM_ERR_WRONG_REQ;
    }

    value = (readl(EMI_BMID5) & ~(0x11 << 30)) |(enable ? (0x11 << 30) : 0 );

    mt65xx_reg_sync_writel(value, EMI_BMID5);

    return BM_REQ_OK;
}

int MET_BM_SetLatencyCounter(void)
{
   return BM_REQ_OK;
}

int MET_BM_GetLatencyCycle(const unsigned int counter_num)
{
  return 0;
}

unsigned int MET_DRAMC_GetPageHitCount(DRAMC_Cnt_Type CountType)  // CM [20121102] wait for implement
{
    unsigned int iCount;

    switch (CountType) {      //MT6572 not suppot by R2R�BR2W�BW2R�BW2W to record
    case DRAMC_R2R:
        //iCount = readl(DRAMC_R2R_PAGE_HIT);
        //break;

    case DRAMC_R2W:
        //iCount = readl(DRAMC_R2W_PAGE_HIT);
        //break;

    case DRAMC_W2R:
        //iCount = readl(DRAMC_W2R_PAGE_HIT);
        //break;

    case DRAMC_W2W:
        //iCount = readl(DRAMC_W2W_PAGE_HIT);
        //break;
    case DRAMC_ALL:
        iCount = readl(DRAMC_PAGE_HIT);
        break;
    default:
        return BM_ERR_WRONG_REQ;
    }

    return iCount;
}

unsigned int MET_DRAMC_GetPageMissCount(DRAMC_Cnt_Type CountType)  // CM [20121102] wait for implement
{
    unsigned int iCount;

    switch (CountType) {      //MT6572 not suppot by R2R�BR2W�BW2R�BW2W to record
    case DRAMC_R2R:
        //iCount = readl(DRAMC_R2R_PAGE_MISS);
       // break;

    case DRAMC_R2W:
        //iCount = readl(DRAMC_R2W_PAGE_MISS);
        //break;

    case DRAMC_W2R:
        //iCount = readl(DRAMC_W2R_PAGE_MISS);
        //break;

    case DRAMC_W2W:
       // iCount = readl(DRAMC_W2W_PAGE_MISS);
        //break;
    case DRAMC_ALL:
        iCount = readl(DRAMC_PAGE_MISS);
        break;
    default:
        return BM_ERR_WRONG_REQ;
    }

    return iCount;
}

unsigned int MET_DRAMC_GetInterbankCount(DRAMC_Cnt_Type CountType)  // CM [20121102] wait for implement
{
    unsigned int iCount;

    switch (CountType) {       //MT6572 not suppot by R2R�BR2W�BW2R�BW2W to record
    case DRAMC_R2R:
        //iCount = readl(DRAMC_R2R_INTERBANK);
        //break;

    case DRAMC_R2W:
       // iCount = readl(DRAMC_R2W_INTERBANK);
       // break;

    case DRAMC_W2R:
        //iCount = readl(DRAMC_W2R_INTERBANK);
        //break;

    case DRAMC_W2W:
        //iCount = readl(DRAMC_W2W_INTERBANK);
        //break;
    case DRAMC_ALL:
        iCount = readl(DRAMC_INTERBANK);
        break;
    default:
        return BM_ERR_WRONG_REQ;
    }

    return iCount;
}

unsigned int MET_DRAMC_GetIdleCount(void)  // CM [20121102] wait for implement
{
    return 0;  //readl(DRAMC_IDLE_COUNT);
}

void MET_MCI_Mon_Enable(void)    //MT6572 no support MCI
{
   return;
}

void MET_MCI_Mon_Disable(void)   //MT6572 no support MCI
{
  return;
}

void MET_MCI_Event_Set(unsigned int evt0, unsigned int evt1) //MT6572 no support MCI
{
  return;
}

void MET_MCI_Event_Read()   //MT6572 no support MCI
{
   return;
}

unsigned int MET_MCI_GetEventCount(int evt_counter) //MT6572 no support MCI
{
  return 0;
}

void MET_LM_Enable(const unsigned int enable)
{
    const unsigned int value = readl(EMI_LMEN);

    mt65xx_reg_sync_writel((value & ~(Lat_MON_EN)) | (enable ? Lat_MON_EN : 0), EMI_LMEN);
}

unsigned int MET_LM_IsOverrun(void)
{
    /*
    * return !0 if EMI_BCNT(bus cycle counts) or EMI_WACT(total word counts) is overrun,
    * otherwise non-overrun return an 0 value
    */
    const unsigned int value = readl(EMI_LMEN);

    return (value & Lat_OVERRUN);
}

int MET_LM_SetMaster(const unsigned int master)
{
    unsigned int value, addr;
    const unsigned int iMask = 0x7;


    addr = EMI_LMEN;
    value = (readl(addr) & ~(iMask << 8)) | ((master & iMask) << 8);

    mt65xx_reg_sync_writel(value, addr);

    return BM_REQ_OK;
}

int MET_LM_GetWTransCount(void)
{
    return readl(EMI_WTCT);
}

int MET_LM_GetWLatCount(void)
{
    return readl(EMI_WLCT);
}

int MET_LM_GetRTransCount(void)
{
    return readl(EMI_RTCT);
}

int MET_LM_GetRLatCount(void)
{
    return readl(EMI_RLCT);
}

