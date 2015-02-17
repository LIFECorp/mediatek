#ifndef __ISP_PATH_H__
#define __ISP_PATH_H__

#include "isp_function.h"
//


#define ISP_MAX_TPIPE_SIMPLE_CONF_SIZE  (128*sizeof(int))


#define DEBUG_STR_BEGIN "EEEEEEEEEEEEEEEEEEEEE"
#define DEBUG_STR_END   "XXXXXXXXXXXXXXXXXXXXX"


enum EPathFreeBufMode {
    ePathFreeBufMode_SINGLE         = 0x0000,   //
    ePathFreeBufMode_ALL            = 0x0001,   //
};

/*/////////////////////////////////////////////////////////////////////////////
    IspPath_B
  /////////////////////////////////////////////////////////////////////////////*/
class IspFunction_B; //pre declaration

class CamPath_B
{
public:
    ISP_TURNING_CTRL    ispTurningCtrl;
    ISP_TOP_CTRL        ispTopCtrl;
    ISP_BUF_CTRL        ispBufCtrl;
    int CQ;

    //common
    ISP_RAW_PIPE    ispRawPipe;
    ISP_RGB_PIPE    ispRgbPipe;
    ISP_YUV_PIPE    ispYuvPipe;
    CAM_CDP_PIPE    cdpPipe;
    CAM_TDRI_PIPE   tdriPipe;
    DMA_CQ          DMACQ;
    //for pass1
    DMA_IMGO        DMAImgo;
    DMA_IMG2O       DMAImg2o;
    //for pass2
    DMA_IMGI        DMAImgi;
    DMA_TDRI        DMATdri;
    DMA_VIPI        DMAVipi;
    DMA_VIP2I       DMAVip2i;
    DMA_IMGCI       DMAImgci;
    DMA_LSCI        DMALsci;
    DMA_LCEI        DMALcei;
//SL TEST_MDP removed. Should no used    DMA_DISPO   DMADispo; //SL
//SL TEST_MDP removed. Should no used        DMA_VIDO    DMAVido; //SL
    //

/*

    DMA_FLKI    if_DMAFlki;
    DMA_DISPO   if_DMADispo;
    DMA_VIDO    if_DMAVido;
*/

public:
    CamPath_B();


    virtual ~CamPath_B(){};
public:
    int             start( void* pParam );
    int             stop( void* pParam );


    inline int              waitIrq( int type, unsigned int irq ){return this->_waitIrq( type,irq );}
    inline int              writeReg( unsigned long offset, unsigned long value ){return ispTopCtrl.writeReg(offset,value);}
    inline unsigned long    readReg( unsigned long offset ){return ispTopCtrl.readReg(offset);}
    inline int              readIrq(ISP_DRV_READ_IRQ_STRUCT *pReadIrq){return ispTopCtrl.readIrq(pReadIrq);}
    inline int              checkIrq(ISP_DRV_CHECK_IRQ_STRUCT CheckIrq){return ispTopCtrl.checkIrq(CheckIrq);}
    inline int              clearIrq(ISP_DRV_CLEAR_IRQ_STRUCT ClearIrq){return ispTopCtrl.clearIrq(ClearIrq);}

    int             dumpRegister( void* pRaram );
    int             end( void* pParam );
protected:
    virtual IspFunction_B**  isp_function_list() = 0;
    virtual int             isp_function_count() = 0;
public:
    virtual const char* name_Str(){     return  "IspPath";  }
protected:
    virtual int _config( void* pParam );
    virtual int _start( void* pParam );
    virtual int _stop( void* pParam );
    virtual int _waitIrq( int type,unsigned int irq );
    virtual int _end( void* pParam );
    virtual int _setZoom( void* pParam );
public:
    virtual int flushCqDescriptor( MUINT32 cq );
    virtual int setDMACurrBuf( MUINT32 const dmaChannel );
    virtual int setDMANextBuf( MUINT32 const dmaChannel );
    virtual int enqueueBuf( MUINT32 const dmaChannel, stISP_BUF_INFO bufInfo );
    virtual int freePhyBuf( MUINT32 const mode, stISP_BUF_INFO bufInfo );
    virtual void setCamMode(MUINT32 mode);
};

/*/////////////////////////////////////////////////////////////////////////////
    IspPathPass1
  /////////////////////////////////////////////////////////////////////////////*/
struct CamPathPass1Parameter
{
    //scenario/sub_mode
    int scenario;
    int subMode;
    int CQ;
    int isIspOn;
    int isEn1C24StatusFixed;
    int isEn1C02StatusFixed;
    int isEn1CfaStatusFixed;
    int isEn1HrzStatusFixed;
    int isEn1MfbStatusFixed;
    int isEn2CdrzStatusFixed;
    int isEn2G2cStatusFixed;
    int isEn2C42StatusFixed;
    int isImg2oStatusFixed;
    int isAaoStatusFixed;
    int isEsfkoStatusFixed;
    int isFlkiStatusFixed;
    int isLcsoStatusFixed;
    int isShareDmaCtlByTurn;
    int isEn1AaaGropStatusFixed;
    int bypass_ispRawPipe;
    int bypass_ispRgbPipe;
    int bypass_ispYuvPipe;
    int bypass_ispCdpPipe;

    //Misc
    int     b_continuous;

    //enable table
    struct stIspTopEnTbl      en_Top;
    struct stIspTopINT        ctl_int;
    struct stIspTopFmtSel     fmt_sel;
    struct stIspTopSel        ctl_sel;
    struct stIspTopMuxSel     ctl_mux_sel;
    struct stIspTopMuxSel2    ctl_mux_sel2;
    struct stIspTopSramMuxCfg ctl_sram_mux_cfg;
    //update function mask
    struct stIspTopFmtSel fixed_mask_cdp_fmt;
    //
    int                       pix_id;
    //source -> from TG
    IspSize         src_img_size;
    IspRect         src_img_roi;
    IspColorFormat  src_color_format;
    //
    IspSize         cdrz_in_size;
    /*===DMA===*/
    IspDMACfg imgo;     //dst00
    IspDMACfg img2o;    //dst01
    IspDMACfg lcso;
    IspDMACfg aao;
    IspDMACfg nr3o;
    IspDMACfg esfko;
    IspDMACfg afo;
    IspDMACfg eiso;
    IspDMACfg imgci;
    IspDMACfg nr3i;
    IspDMACfg flki;
    IspDMACfg lsci;
    IspDMACfg lcei;
    //
};


class CamPathPass1:public CamPath_B
{
private:
    int             m_isp_function_count;
    IspFunction_B*   m_isp_function_list[ISP_FUNCTION_MAX_NUM];
    //
public:
    CamPathPass1() :
        m_isp_function_count(0)
        {};
    virtual ~CamPathPass1(){};
private:
    virtual int _waitIrq( int type,unsigned int irq );
protected:
    virtual IspFunction_B**  isp_function_list()  {   return m_isp_function_list; }
    virtual int             isp_function_count() {   return m_isp_function_count; }
public:
    virtual const char* name_Str(){     return  "CamPathPass1";  }
public:
    int     config( struct CamPathPass1Parameter* p_parameter );
    int     setCdrz( IspSize out_size );
    int     setDMAImgo( IspDMACfg const out_dma );
    int     dequeueBuf( MUINT32 dmaChannel ,stISP_FILLED_BUF_LIST& bufInfo );
    int     setCQTriggerMode(MINT32 cq, MINT32 mode, MINT32 trig_src);
private:
};


/*/////////////////////////////////////////////////////////////////////////////
    IspPathPass2
  /////////////////////////////////////////////////////////////////////////////*/
struct CamPathPass2Parameter
{
    //scenario/sub_mode    
    //int subMode;
    unsigned int scenario;
    int CQ;
    int tpipe;
    int tcm_en;
    int isIspOn;
    int isConcurrency;
    int isEn1C24StatusFixed;
    int isEn1C02StatusFixed;
    int isEn1CfaStatusFixed;
    int isEn1HrzStatusFixed;
    int isEn1MfbStatusFixed;
    int isEn2CdrzStatusFixed;
    int isEn2G2cStatusFixed;
    int isEn2C42StatusFixed;
    int isImg2oStatusFixed;
    int isImgoStatusFixed;
    int isAaoStatusFixed;
    int isEsfkoStatusFixed;
    int isFlkiStatusFixed;
    int isLcsoStatusFixed;
    int isEn1AaaGropStatusFixed;
    int isApplyTurn;
    int isShareDmaCtlByTurn;
    int bypass_ispRawPipe;
    int bypass_ispRgbPipe;
    int bypass_ispYuvPipe;
    int bypass_ispCdpPipe;

    //enable table
    struct stIspTopEnTbl    en_Top;
    struct stIspTopINT      ctl_int;
    struct stIspTopFmtSel   fmt_sel;
    struct stIspTopSel      ctl_sel;
    struct stIspTopMuxSel     ctl_mux_sel;
    struct stIspTopMuxSel2    ctl_mux_sel2;
    struct stIspTopSramMuxCfg ctl_sram_mux_cfg;
    //update function mask
    struct stIspTopFmtSel fixed_mask_cdp_fmt;
    //
    int                     pix_id;
    //source ->  mem. in
    IspSize         src_img_size;
    IspRect         src_img_roi;
    IspColorFormat  src_color_format;
    //
    //IspSize         cdrz_out;
    /*===DMA===*/
    IspDMACfg tdri;
    IspDMACfg cqi;
    IspDMACfg imgi;
    IspDMACfg vipi;
    IspDMACfg vip2i;
    IspDMACfg imgci;
    IspDMACfg lcei;
    IspDMACfg lsci;
    CdpRotDMACfg dispo;
    CdpRotDMACfg vido;
    //

    IspTdriUpdateCfg updateTdri;
    IspRingTdriCfg ringTdriCfg;
    IspCapTdriCfg capTdriCfg;
    IspBnrCfg bnr;
    IspLscCfg lsc;
    IspLceCfg lce;
    IspNbcCfg nbc;
    IspSeeeCfg seee;
    IspDMACfg imgo_dma;
    IspImgoCfg imgo;
    IspEsfkoCfg esfko;
    IspAaoCfg aao;
    IspLcsoCfg lcso;
    IspCdrzCfg cdrz;
    IspCurzCfg curz;
    IspFeCfg fe;
    IspImg2oCfg img2o;
    IspPrzCfg prz;
    IspMfbCfg mfb;
    IspFlkiCfg flki;
    IspCfaCfg cfa;
	IspSl2Cfg sl2;
/*    IspDMACfg imgo;
    IspDMACfg img2o;
    IspDMACfg lcso;
    IspDMACfg aao;
    IspDMACfg nr3o;
    IspDMACfg esfko;
    IspDMACfg nr3i;
    IspDMACfg flki;
    IspDMACfg lsci;
    IspDMACfg lcei;
*/    //
};


class CamPathPass2:public CamPath_B
{
private:
    int             m_isp_function_count;
    IspFunction_B*   m_isp_function_list[ISP_FUNCTION_MAX_NUM];

public:
    CamPathPass2() :
        m_isp_function_count(0)
        {};
    virtual ~CamPathPass2(){};
private:
    virtual int _waitIrq( int type,unsigned int irq );
protected:
    virtual IspFunction_B**  isp_function_list()  {   return m_isp_function_list; }
    virtual int             isp_function_count() {   return m_isp_function_count; }
public:
    virtual const char* name_Str(){     return  "CamPathPass2";  }
public:
    int     config( struct CamPathPass2Parameter* p_parameter );
    int     setZoom( MUINT32 zoomRatio );
    int     dequeueBuf( MUINT32 dmaChannel ,stISP_FILLED_BUF_LIST& bufInfo );
    int     configRingTpipe( struct CamPathPass2Parameter* p_parameter);
private:
    int     configTpipeData( struct CamPathPass2Parameter* p_parameter );
    int     getTpipePerform( struct CamPathPass2Parameter* p_parameter );
    int     getCdpMuxSetting(struct CamPathPass2Parameter pass2Parameter, MINT32 *pDispVidSel);
};

#endif







