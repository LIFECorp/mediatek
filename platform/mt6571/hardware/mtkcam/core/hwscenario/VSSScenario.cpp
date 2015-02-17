#define LOG_TAG "MtkCam/VSSScen"
//
#include <vector>
using namespace std;
//
#include <utils/Vector.h>
#include <mtkcam/common.h>
#include <mtkcam/imageio/IPipe.h>
#include <mtkcam/imageio/ICamIOPipe.h>
#include <mtkcam/imageio/IPostProcPipe.h>
//
#include <mtkcam/campipe/IPipe.h>
#include <mtkcam/campipe/IXdpPipe.h>
//
#include <mtkcam/drv/imem_drv.h>
#include <mtkcam/imageio/ispio_stddef.h>
#include <mtkcam/drv/isp_drv.h>
#include <mtkcam/hal/sensor_hal.h>
using namespace NSImageio;
using namespace NSIspio;
//
#include <mtkcam/v1/hwscenario/IhwScenarioType.h>
using namespace NSHwScenario;
#include <mtkcam/v1/hwscenario/IhwScenario.h>
#include "hwUtility.h"
#include "VSSScenario.h"
//
#include <cutils/atomic.h>
#include <mtkcam/v1/camutils/CamInfo.h>
//
#if defined(HAVE_AEE_FEATURE)
#include <aee.h>
#endif
//
#define ROUND_TO_2X(x) ((x) & (~0x1))
//
/*******************************************************************************
*
********************************************************************************/
#include <mtkcam/Log.h>
#define MY_LOGV(fmt, arg...)    CAM_LOGV("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)    CAM_LOGD("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)    CAM_LOGI("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)    CAM_LOGW("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)    CAM_LOGE("[%s] "fmt, __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, arg...)    if (cond) { MY_LOGV(arg); }
#define MY_LOGD_IF(cond, arg...)    if (cond) { MY_LOGD(arg); }
#define MY_LOGI_IF(cond, arg...)    if (cond) { MY_LOGI(arg); }
#define MY_LOGW_IF(cond, arg...)    if (cond) { MY_LOGW(arg); }
#define MY_LOGE_IF(cond, arg...)    if (cond) { MY_LOGE(arg); }
//
#define FUNCTION_LOG_START      MY_LOGD("+");
#define FUNCTION_LOG_END        MY_LOGD("-");
#define ERROR_LOG               MY_LOGE("Error");
//
#define _PASS1_CQ_CONTINUOUS_MODE_
#define ENABLE_LOG_PER_FRAME    (0)
//
#if defined(HAVE_AEE_FEATURE)
    #define AEE_ASSERT(String) \
        do { \
            aee_system_exception( \
                "VSSScenario", \
                NULL, \
                DB_OPT_DEFAULT, \
                String); \
        } while(0)
#else
    #define AEE_ASSERT(String)
#endif
//
/*******************************************************************************
*
********************************************************************************/
VSSScenario*
VSSScenario::createInstance(EScenarioFmt rSensorType, halSensorDev_e const &dev, ERawPxlID const &bitorder)
{
    return new VSSScenario(rSensorType, dev, bitorder);
}


/*******************************************************************************
*
********************************************************************************/
MVOID
VSSScenario::destroyInstance()
{
    //
    delete this;
}


/*******************************************************************************
*
********************************************************************************/
VSSScenario::VSSScenario(EScenarioFmt rSensorType, halSensorDev_e const &dev, ERawPxlID const &bitorder)
            : mpCamIOPipe(NULL)
            , mpPostProcPipe(NULL)
            , mpXdpPipe(NULL)
            , mpIMemDrv(NULL)
            , mSensorType(rSensorType)
            , mSensorDev(dev)
            , mSensorBitOrder(bitorder)
            , mModuleMtx()
            , mbP1Out(MFALSE)
            , mbP1DispOut(MFALSE)
            , mpExtImgProcHw(NULL)
            , mbTwoRunPass2(MFALSE)
            , mbTwoRunPass2Dispo(MTRUE)
{
    mSensorId = dev == SENSOR_DEV_MAIN ? 0 : 1;
    MINT32 wantedOri = android::MtkCamUtils::DevMetaInfo::queryDeviceWantedOrientation(mSensorId);
    MINT32 realOri = android::MtkCamUtils::DevMetaInfo::queryDeviceSetupOrientation(mSensorId);
    if(wantedOri == realOri)
    {
        mbTwoRunRot = MFALSE;
    }
    else
    {
        mbTwoRunRot = MTRUE;
    }
    MY_LOGD("mSensorId(%d),wantedOri(%d),realOri(%d),mbTwoRunRot(%d)",
            mSensorId,
            wantedOri,
            realOri,
            mbTwoRunRot);
    //
    MY_LOGD("mSensorBitOrder(%d),this(%p),sizeof(%d)",
            mSensorBitOrder,
            this,
            sizeof(VSSScenario));
}


/*******************************************************************************
*
********************************************************************************/
VSSScenario::~VSSScenario()
{
    MY_LOGD("");
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
VSSScenario::
init()
{
    FUNCTION_LOG_START;
    //(1)
    mpCamIOPipe = ICamIOPipe::createInstance(eScenarioID_VSS, mSensorType);
    if ( ! mpCamIOPipe || ! mpCamIOPipe->init())
    {
        MY_LOGE("ICamIOPipe init error");
        return MFALSE;
    }
    //(2)
    mpPostProcPipe = IPostProcPipe::createInstance(eScenarioID_VSS, mSensorType);
    if ( ! mpPostProcPipe || ! mpPostProcPipe->init())
    {
        MY_LOGE("IPostProcPipe init error");
        return MFALSE;
    }
    //(3)
    mpExtImgProcHw = ExtImgProcHw::createInstance();
    if(!mpExtImgProcHw || !mpExtImgProcHw->init())
    {
        MY_LOGE("IExtImgProc init error");
        return MFALSE;
    }
    //
    mpXdpPipe = NSCamPipe::IXdpPipe::createInstance(NSCamPipe::eSWScenarioID_VSS, NSCamPipe::eScenarioFmt_YUV);
    if (!mpXdpPipe || !mpXdpPipe->init())
    {
        MY_LOGE("IXdpPipe init error");
        return MFALSE;
    }
    //
    mpIMemDrv = IMemDrv::createInstance();
    if(!mpIMemDrv || !mpIMemDrv->init())
    {
        MY_LOGE("IMemDrv init error");
        return MFALSE;
    }
    //
    mpCamIOPipe->sendCommand(EPIPECmd_SET_CQ_CHANNEL,(MINT32)EPIPE_PASS1_CQ0, 0, 0);

    mpCamIOPipe->sendCommand(EPIPECmd_SET_CQ_TRIGGER_MODE,
                            (MINT32)EPIPE_PASS1_CQ0,
                            (MINT32)EPIPECQ_TRIGGER_SINGLE_IMMEDIATE,
                            (MINT32)EPIPECQ_TRIG_BY_START);

    mpPostProcPipe->sendCommand(EPIPECmd_SET_CQ_CHANNEL,
                               (MINT32)EPIPE_PASS2_CQ1, 0, 0);
    mpCamIOPipe->sendCommand(EPIPECmd_SET_CONFIG_STAGE,(MINT32)eConfigSettingStage_Init, 0, 0);
    mpPostProcPipe->sendCommand(EPIPECmd_SET_CONFIG_STAGE,(MINT32)eConfigSettingStage_Init, 0, 0);
    //
    msPass1OutFmt = "UNKNOWN";
    msPass2DispoOutFmt = "UNKNOWN";
    msPass2VidoOutFmt = "UNKNOWN";
    //
    mTwoRunPass2Info.tempBuf.memID = -1;
    mTwoRunPass2Info.tempBuf.virtAddr = 0;
    mTwoRunPass2Info.tempBuf.phyAddr = 0;
    mTwoRunPass2Info.tempBuf.size = 0;
    //
    FUNCTION_LOG_END;
    //
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
VSSScenario::uninit()
{
    FUNCTION_LOG_START;
    //
    MBOOL ret = MTRUE;
    //(1)
    if ( mpCamIOPipe )
    {
        if ( ! mpCamIOPipe->uninit())
        {
            MY_LOGE("mpCamIOPipe uninit fail");
            ret = MFALSE;
        }
        mpCamIOPipe->destroyInstance();
        mpCamIOPipe = NULL;
    }
    //(2)
    if ( mpPostProcPipe )
    {
        if ( ! mpPostProcPipe->uninit())
        {
            MY_LOGE("mpPostProcPipe uninit fail");
            ret = MFALSE;
        }
        mpPostProcPipe->destroyInstance();
        mpPostProcPipe = NULL;
    }
    //
    if(mpExtImgProcHw != NULL)
    {
        mpExtImgProcHw->uninit();
        mpExtImgProcHw->destroyInstance();
        mpExtImgProcHw = NULL;
    }
    //
    if(mpXdpPipe != NULL)
    {
        if (!mpXdpPipe->uninit())
        {
            MY_LOGE("mpXdpPipe uninit fail");
            ret = MFALSE;
        }
        mpXdpPipe->destroyInstance();
        mpXdpPipe = NULL;
    }
    //
    freeTwoRunPass2TempBuf();
    //
    if(mpIMemDrv != NULL)
    {
        mpIMemDrv->uninit();
        mpIMemDrv->destroyInstance();
        mpIMemDrv = NULL;
    }
    //
    FUNCTION_LOG_END;
    //
    return ret;
}


/*******************************************************************************
* wait hardware interrupt
********************************************************************************/
MVOID
VSSScenario::wait(EWaitType rType)
{
    switch(rType)
    {
        case eIRQ_VS:
            mpCamIOPipe->irq(EPipePass_PASS1_TG1, EPIPEIRQ_VSYNC);
        break;
        default:
        break;
    }
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
VSSScenario::start()
{
    FUNCTION_LOG_START;
    // (1) start CQ
    mpCamIOPipe->startCQ0();
#if defined(_PASS1_CQ_CONTINUOUS_MODE_)
    mpCamIOPipe->sendCommand(EPIPECmd_SET_CQ_TRIGGER_MODE,
                             (MINT32)EPIPE_PASS1_CQ0,
                             (MINT32)EPIPECQ_TRIGGER_CONTINUOUS_EVENT,
                             (MINT32)EPIPECQ_TRIG_BY_PASS1_DONE);
#else
    mpCamIOPipe->sendCommand(EPIPECmd_SET_CQ_CHANNEL,(MINT32)EPIPE_CQ_NONE, 0, 0);
#endif
    // (2) pass1 start
    if ( ! mpCamIOPipe->start())
    {
        MY_LOGE("mpCamIOPipe->start() fail");
        return MFALSE;
    }
    // align to Vsync
    //mpCamIOPipe->irq(EPipePass_PASS1_TG1, EPIPEIRQ_VSYNC);
    //MY_LOGD("- wait IRQ: ISP_DRV_IRQ_INT_STATUS_VS1_ST");
    //
    FUNCTION_LOG_END;
    //
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
VSSScenario::stop()
{
    FUNCTION_LOG_START;
    //
    //try to remove this
#if 0
    PortID rPortID;
    mapPortCfg(eID_Pass1Out, rPortID);
    PortQTBufInfo dummy(eID_Pass1Out);
    mpCamIOPipe->dequeOutBuf(rPortID, dummy.bufInfo);
#endif
    //
    if ( ! mpCamIOPipe->stop())
    {
        MY_LOGE("mpCamIOPipe->stop() fail");
        return MFALSE;
    }
    //
    FUNCTION_LOG_END;
    //
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
VSSScenario::
setConfig(vector<PortImgInfo> *pImgIn)
{
    if ( ! pImgIn )
    {
        MY_LOGE("pImgIn==NULL");
        return MFALSE;
    }

    bool isPass1 = false;
    MUINT32 dispoImgSize = 0, vidoImgSize = 0;

    vector<PortImgInfo>::const_iterator rSrc;
    for (rSrc = pImgIn->begin(); rSrc != pImgIn->end(); ++rSrc )
    {
        //
        // Pass 1 config will be fixed. Pass 2 config can be updated later.
        //
#define SetCommonSetting(src, port)                                              \
        do{                                                                      \
            mapFormat(src->sFormat, port.eImgFmt);                               \
            port.u4ImgWidth                  = src->u4Width;                     \
            port.u4ImgHeight                 = src->u4Height;                    \
            port.u4Stride[ESTRIDE_1ST_PLANE] = src->u4Stride[ESTRIDE_1ST_PLANE]; \
            port.u4Stride[ESTRIDE_2ND_PLANE] = src->u4Stride[ESTRIDE_2ND_PLANE]; \
            port.u4Stride[ESTRIDE_3RD_PLANE] = src->u4Stride[ESTRIDE_3RD_PLANE]; \
            port.crop.x                      = src->crop.x;                      \
            port.crop.y                      = src->crop.y;                      \
            port.crop.floatX                 = src->crop.floatX;                 \
            port.crop.floatY                 = src->crop.floatY;                 \
            port.crop.w                      = src->crop.w;                      \
            port.crop.h                      = src->crop.h;                      \
        }while(0)

        if (rSrc->ePortIdx == eID_Pass1In)
        {
            isPass1 = true;
            mbP1Out = MFALSE;
            mbP1DispOut = MFALSE;
            defaultSetting();
            SetCommonSetting(rSrc, mSettingPorts.tgi);
        }
        else if (rSrc->ePortIdx == eID_Pass1Out)
        {
            mbP1Out = MTRUE;
            msPass1OutFmt = rSrc->sFormat;
            SetCommonSetting(rSrc, mSettingPorts.imgo);
        }
        else if (rSrc->ePortIdx == eID_Pass1DispOut)
        {
            //2nd raw out
            mbP1DispOut = MTRUE;
            msPass1OutFmt = rSrc->sFormat;
            SetCommonSetting(rSrc, mSettingPorts.img2o);
        }
        else if (rSrc->ePortIdx == eID_Pass2In)
        {
            SetCommonSetting(rSrc, mSettingPorts.imgi);
            //
            if( mbTwoRunRot &&
                calRotation() != eImgRot_180)
            {
                calCrop(
                    mSettingPorts.imgi.u4ImgWidth,
                    mSettingPorts.imgi.u4ImgHeight,
                    mSettingPorts.imgi.crop.w,
                    mSettingPorts.imgi.crop.h,
                    mSettingPorts.imgi.crop.x,
                    mSettingPorts.imgi.crop.y);
            }
        }
        else if (rSrc->ePortIdx == eID_Pass2DISPO)
        {
            msPass2DispoOutFmt = rSrc->sFormat;
            SetCommonSetting(rSrc, mSettingPorts.dispo);
            mSettingPorts.dispo.eImgRot = calRotation();
            //
            if(mbTwoRunRot)
            {
                mSettingPorts.dispo.eImgRot = calRotation();
            }
#if (PLATFORM_VERSION_MAJOR == 2) 
            mSettingPorts.dispo.eImgRot = rSrc->eRotate;
#endif
        }
        else if (rSrc->ePortIdx == eID_Pass2VIDO)
        {
            msPass2VidoOutFmt = rSrc->sFormat;
            SetCommonSetting(rSrc, mSettingPorts.vido);
            //
            if(mbTwoRunRot)
            {
                mSettingPorts.vido.eImgRot = calRotation(rSrc->eRotate);
            }
            else
            {
                mSettingPorts.vido.eImgRot = rSrc->eRotate;
            }
        }
        else
        {
            MY_LOGE("Not done yet!!");
        }
#undef SetCommonSetting
    }

    //mSettingPorts.dump();
    
    if (isPass1)
    {
        // Note:: must to config cameraio pipe before irq
        //        since cameio pipe won't be changed later, do it here
        vector<PortInfo const*> vCamIOInPorts;
        vector<PortInfo const*> vCamIOOutPorts;
        vCamIOInPorts.push_back(&mSettingPorts.tgi);
        if( mbP1Out )
        {
            vCamIOOutPorts.push_back(&mSettingPorts.imgo);
        }
        if( mbP1DispOut )
        {
            vCamIOOutPorts.push_back(&mSettingPorts.img2o);
        }
        mpCamIOPipe->configPipe(vCamIOInPorts, vCamIOOutPorts);
    }
    else
    {
        if(dispoImgSize >= vidoImgSize)
        {
            mbTwoRunPass2Dispo = MTRUE;
        }
        else
        {
            mbTwoRunPass2Dispo = MFALSE;
        }
    }
    //
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MVOID
VSSScenario::sDefaultSetting_Ports::
dump()
{
    MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "[TG]:F(%d),W(%d),H(%d),Str(%d)",
        tgi.eImgFmt,
        tgi.u4ImgWidth,
        tgi.u4ImgHeight,
        tgi.u4Stride[ESTRIDE_1ST_PLANE]);
    MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "[IMGO]:F(%d),W(%d),H(%d),Str(%d,%d,%d)",
        imgo.eImgFmt,
        imgo.u4ImgWidth,
        imgo.u4ImgHeight,
        imgo.u4Stride[ESTRIDE_1ST_PLANE],
        imgo.u4Stride[ESTRIDE_2ND_PLANE],
        imgo.u4Stride[ESTRIDE_3RD_PLANE]);
    MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "[IMG2O]:F(%d),W(%d),H(%d),Str(%d,%d,%d)",
        img2o.eImgFmt,
        img2o.u4ImgWidth,
        img2o.u4ImgHeight,
        img2o.u4Stride[ESTRIDE_1ST_PLANE],
        img2o.u4Stride[ESTRIDE_2ND_PLANE],
        img2o.u4Stride[ESTRIDE_3RD_PLANE]);
    MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "[IMGI]:F(%d),W(%d),H(%d),Str(%d,%d,%d)",
        imgi.eImgFmt,
        imgi.u4ImgWidth,
        imgi.u4ImgHeight,
        imgi.u4Stride[ESTRIDE_1ST_PLANE],
        imgi.u4Stride[ESTRIDE_2ND_PLANE],
        imgi.u4Stride[ESTRIDE_3RD_PLANE]);
    MY_LOGD_IF(ENABLE_LOG_PER_FRAME,"[DISPO]:F(%d),W(%d),H(%d),Str(%d,%d,%d)",
        dispo.eImgFmt,
        dispo.u4ImgWidth,
        dispo.u4ImgHeight,
        dispo.u4Stride[ESTRIDE_1ST_PLANE],
        dispo.u4Stride[ESTRIDE_2ND_PLANE],
        dispo.u4Stride[ESTRIDE_3RD_PLANE]);
    MY_LOGD_IF(ENABLE_LOG_PER_FRAME,"[VIDO]:F(%d),W(%d),H(%d),Str(%d,%d,%d),Rot(%d)",
        vido.eImgFmt,
        vido.u4ImgWidth,
        vido.u4ImgHeight,
        vido.u4Stride[ESTRIDE_1ST_PLANE],
        vido.u4Stride[ESTRIDE_2ND_PLANE],
        vido.u4Stride[ESTRIDE_3RD_PLANE],
        vido.eImgRot);
}

/*******************************************************************************
*
********************************************************************************/
MVOID
VSSScenario::
defaultSetting()
{

    ////////////////////////////////////////////////////////////////////
    //      Pass 1 setting (default)                                  //
    ////////////////////////////////////////////////////////////////////

    // (1.1) tgi
    PortInfo &tgi = mSettingPorts.tgi;
    tgi.eRawPxlID = mSensorBitOrder; //only raw looks this
    tgi.type = EPortType_Sensor;
    tgi.inout  = EPortDirection_In;
    tgi.index = EPortIndex_TG1I; 
    tgi.u4Stride[ESTRIDE_1ST_PLANE] = 0; 
    tgi.u4Stride[ESTRIDE_2ND_PLANE] = 0; 
    tgi.u4Stride[ESTRIDE_3RD_PLANE] = 0; 
    // (1.2) imgo
    PortInfo &imgo = mSettingPorts.imgo;
    imgo.type = EPortType_Memory;
    imgo.index = EPortIndex_IMGO;
    imgo.inout  = EPortDirection_Out;
    imgo.u4Stride[ESTRIDE_1ST_PLANE] = 0; 
    imgo.u4Stride[ESTRIDE_2ND_PLANE] = 0; 
    imgo.u4Stride[ESTRIDE_3RD_PLANE] = 0; 

    // (1.3) img2o
    PortInfo &img2o = mSettingPorts.img2o;
    img2o.type = EPortType_Memory;
    img2o.index = EPortIndex_IMG2O;
    img2o.inout  = EPortDirection_Out;
    img2o.u4Stride[ESTRIDE_1ST_PLANE] = 0; 
    img2o.u4Stride[ESTRIDE_2ND_PLANE] = 0; 
    img2o.u4Stride[ESTRIDE_3RD_PLANE] = 0; 

    ////////////////////////////////////////////////////////////////////
    //Pass 2 setting (default)
    ////////////////////////////////////////////////////////////////////

    //(2.1)
    PortInfo &imgi = mSettingPorts.imgi;
    imgi.eImgFmt = eImgFmt_UNKNOWN;
    imgi.type = EPortType_Memory;
    imgi.index = EPortIndex_IMGI;
    imgi.inout = EPortDirection_In;
    imgi.pipePass = EPipePass_PASS2;
    imgi.u4Offset = 0;
    imgi.u4Stride[ESTRIDE_1ST_PLANE] = 0;
    imgi.u4Stride[ESTRIDE_2ND_PLANE] = 0;
    imgi.u4Stride[ESTRIDE_3RD_PLANE] = 0;

    //(2.2)
    PortInfo &dispo = mSettingPorts.dispo;
    dispo.eImgFmt = eImgFmt_UNKNOWN;
    dispo.eImgRot = eImgRot_0;                  //dispo NOT support rotation
    dispo.eImgFlip = eImgFlip_OFF;              //dispo NOT support flip
    dispo.type = EPortType_DISP_RDMA;           //EPortType
    dispo.index = EPortIndex_DISPO;
    dispo.inout  = EPortDirection_Out;
    dispo.u4Offset = 0;
    dispo.u4Stride[ESTRIDE_1ST_PLANE] = 0;
    dispo.u4Stride[ESTRIDE_2ND_PLANE] = 0;
    dispo.u4Stride[ESTRIDE_3RD_PLANE] = 0;

    //(2.3)
    PortInfo &vido = mSettingPorts.vido;
    vido.eImgFmt = eImgFmt_UNKNOWN;
    vido.eImgRot = eImgRot_0;
    vido.eImgFlip = eImgFlip_OFF;
    vido.type = EPortType_VID_RDMA;
    vido.index = EPortIndex_VIDO;
    vido.inout  = EPortDirection_Out;
    vido.u4Offset = 0;
    vido.u4Stride[ESTRIDE_1ST_PLANE] = 0;
    vido.u4Stride[ESTRIDE_2ND_PLANE] = 0;
    vido.u4Stride[ESTRIDE_3RD_PLANE] = 0;
}


/*******************************************************************************
*  enque:
********************************************************************************/
MBOOL
VSSScenario::
enque(vector<IhwScenario::PortQTBufInfo> const &in)
{
    //
    if(in.size() == 0)
    {
        MY_LOGE("Size is 0");
        return MFALSE;
    }
    //
    //Enque pass1 buffer: PortQTBufInfo -> PortBufInfo
    //mutilple buffers: may happens
    MY_LOGD_IF(in.size() > 1, "enque in.size(): %d", in.size() );
    //buffer with multiple planes: pass1 won't happen
    //MY_LOGW_IF(in.at(0).bufInfo.vBufInfo.size() > 1, "in.at(0).bufInfo.vBufInfo.size() > 1");

    vector<IhwScenario::PortBufInfo> vEnBufPass1Out;

    //loop over buffers
    vector<PortQTBufInfo>::const_iterator iter;
    for (iter = in.begin(); iter != in.end(); ++iter )
    {
        //single-plane buffer
        IhwScenario::PortBufInfo one( iter->ePortIndex,
                                      iter->bufInfo.vBufInfo.at(0).u4BufVA,
                                      iter->bufInfo.vBufInfo.at(0).u4BufPA,
                                      iter->bufInfo.vBufInfo.at(0).u4BufSize,
                                      iter->bufInfo.vBufInfo.at(0).memID,
                                      iter->bufInfo.vBufInfo.at(0).bufSecu,
                                      iter->bufInfo.vBufInfo.at(0).bufCohe);

        vEnBufPass1Out.push_back(one);
    };

    enque(NULL, &vEnBufPass1Out);

    return MTRUE;
}


/*******************************************************************************
*  enque:
********************************************************************************/
MBOOL
VSSScenario::
enque( vector<PortBufInfo> *pBufIn, vector<PortBufInfo> *pBufOut)
{
    if ( !pBufIn ) // pass 1
    {
        enquePass1(pBufOut);
    }
    else  // pass 2
    {
        if(mbTwoRunRot)
        {
            enquePass2TwoRunRot(pBufIn, pBufOut);
        }
        else
        {
            if(mbTwoRunPass2)
            {
                enquePass2TwoRunPass2(pBufIn, pBufOut);
        }
        else
        {
            enquePass2(pBufIn,pBufOut);
        }
    }
    }
    return MTRUE;
}


/*******************************************************************************
*  enque Pass 1:
********************************************************************************/
MBOOL
VSSScenario::
enquePass1(vector<PortBufInfo> *pBufOut)
{
    // pBufOut: single-plane buffers
    // Note:: can't update config, but address
   
    vector<PortBufInfo>::const_iterator iter;
    for (iter = pBufOut->begin(); iter != pBufOut->end(); ++iter )
    {
        PortID rPortID;
        QBufInfo rQbufInfo;
        mapConfig( *iter, rPortID, rQbufInfo);
        mpCamIOPipe->enqueOutBuf(rPortID, rQbufInfo);
        //
        MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "P1(0x%x %d/0x%08X)",
            iter->ePortIndex,
            iter->memID,
            iter->virtAddr);
    }

    return MTRUE;
}


/*******************************************************************************
*  enque Pass
********************************************************************************/
MBOOL
VSSScenario::
enquePass2(
    vector<PortBufInfo> *pBufIn,
    vector<PortBufInfo> *pBufOut)
{
    //(1)
    MUINT32 size = 0;
    // [pass 2 In]
    vector<PortInfo const*> vPostProcInPorts;
    vPostProcInPorts.push_back(&mSettingPorts.imgi);
    // [pass 2 Out]
    vector<PortInfo const*> vPostProcOutPorts;
    vector<PortID> vPortID;
    vector<QBufInfo> vQbufInfo;
    //
    size = pBufOut->size();
    for (MUINT32 i = 0; i < size; i++)
    {
        PortID rPortID;
        QBufInfo rQbufInfo;
        mapConfig(pBufOut->at(i), rPortID, rQbufInfo);
        vPortID.push_back(rPortID);
        vQbufInfo.push_back(rQbufInfo);
        //
        if (rPortID.index == EPortIndex_DISPO)
        {
            vPostProcOutPorts.push_back(&mSettingPorts.dispo);
        }
        else if (rPortID.index == EPortIndex_VIDO)
        {
            vPostProcOutPorts.push_back(&mSettingPorts.vido);
        }
    }
    //
    mpPostProcPipe->configPipe(vPostProcInPorts, vPostProcOutPorts);
    // (2)
    size = pBufIn->size();
    for (MUINT32 i = 0; i < size; i++)
    {
        PortID rPortID;
        QBufInfo rQbufInfo;
        mapConfig(pBufIn->at(i), rPortID, rQbufInfo);
        mpPostProcPipe->enqueInBuf(rPortID, rQbufInfo);
        //
        if(size > 1)
        {
            MY_LOGD_IF(0, "P2I(%d-%d/0x%08X)",
                        i,
                        rQbufInfo.vBufInfo.at(0).memID,
                        rQbufInfo.vBufInfo.at(0).u4BufVA);
        }
        else
        {
            MY_LOGD_IF(0, "P2I(%d/0x%08X)",
                        rQbufInfo.vBufInfo.at(0).memID,
                        rQbufInfo.vBufInfo.at(0).u4BufVA);
        }
    }
    //
    size = vPortID.size();
    for (MUINT32 i = 0; i < size; i++)
    {
        mpPostProcPipe->enqueOutBuf(vPortID.at(i), vQbufInfo.at(i));
        //
        if(size > 1)
        {
            MY_LOGD_IF(0, "P2O(%d-%d/0x%08X)",
                        i,
                        vQbufInfo.at(i).vBufInfo.at(0).memID,
                        vQbufInfo.at(i).vBufInfo.at(0).u4BufVA);
        }
        else
        {
            MY_LOGD_IF(0, "P2O(%d/0x%08X)",
                        vQbufInfo.at(i).vBufInfo.at(0).memID,
                        vQbufInfo.at(i).vBufInfo.at(0).u4BufVA);
        }
    }
    // revise config to "update" mode after the first configPipe
    mpPostProcPipe->sendCommand(EPIPECmd_SET_CONFIG_STAGE, (MINT32)eConfigSettingStage_UpdateTrigger, 0, 0);
    //
    return MTRUE;
}


/*******************************************************************************
*  enque Pass 2 Two Run Rot:
********************************************************************************/
MBOOL
VSSScenario::
enquePass2TwoRunRot(
    vector<PortBufInfo> *pBufIn,
    vector<PortBufInfo> *pBufOut)
{
    MUINT32 size;
    // [pass 2 In]
    vector<PortInfo const*> vPostProcInPorts;
    vPostProcInPorts.push_back(&mSettingPorts.imgi);
    // [pass 2 Out]
    vector<PortInfo const*> vPostProcOutPorts;
    vector<PortID> vPortID;
    vector<QBufInfo> vQbufInfo;
    //
    size = pBufOut->size();      
    if (size == 2)
     {
        MY_LOGD("DISPO&VIDO");
        //
        PortID rPortID;
        // (.1) keep dispo to be used later
        mTwoRunRotInfo.outBuf.vBufInfo.clear();
        //
        int dispo_idx = 0;
        pBufOut->at(0).ePortIndex == eID_Pass2DISPO ? dispo_idx = 0 : 1;
        //
        mapConfig(pBufOut->at(dispo_idx), rPortID, mTwoRunRotInfo.outBuf);
        mapPortCfg(eID_Pass2VIDO, rPortID);
        //
        mTwoRunRotInfo.outPort = mSettingPorts.dispo;
        mTwoRunRotInfo.outPort.type = EPortType_VID_RDMA;
        mTwoRunRotInfo.outPort.index = EPortIndex_VIDO;
        // (.2) use vido right now
        QBufInfo rQbufInfo;
        mapConfig(pBufOut->at(1-dispo_idx), rPortID, rQbufInfo);
        vPortID.push_back(rPortID);
        vQbufInfo.push_back(rQbufInfo);
        //
        vPostProcOutPorts.push_back(&mSettingPorts.vido);
     }
     else
     if (size == 1)
     {            
        PortID rPortID;
        QBufInfo rQbufInfo;
        mapConfig(pBufOut->at(0), rPortID, rQbufInfo);
        // case (2): DISPO 
        if (rPortID.index == EPortIndex_DISPO)            
        { 
            MY_LOGD("DISPO");
            // (.1) revise its port index to vido 
            mapPortCfg(eID_Pass2VIDO, rPortID);
            vPortID.push_back(rPortID);
            vQbufInfo.push_back(rQbufInfo);
            // (.2) revise its port index to vido 
            mTwoRunRotInfo.outPort = mSettingPorts.dispo;
            mTwoRunRotInfo.outPort.type = EPortType_VID_RDMA;
            mTwoRunRotInfo.outPort.index = EPortIndex_VIDO;
            // (.3)                
            vPostProcOutPorts.push_back(&mTwoRunRotInfo.outPort); 
        }
        // case (3): VIDO
        else
        if (rPortID.index == EPortIndex_VIDO)
        {
            MY_LOGD("VIDO");
            //
            vPortID.push_back(rPortID);
            vQbufInfo.push_back(rQbufInfo);
            //
            vPostProcOutPorts.push_back(&mSettingPorts.vido);             
        }
     }
    //
    mpPostProcPipe->configPipe(vPostProcInPorts, vPostProcOutPorts);
    //enque pass 2 in
    size = pBufIn->size();
    for (MUINT32 i = 0; i < size; i++)
    {
        PortID rPortID;
        QBufInfo rQbufInfo;
        mapConfig(pBufIn->at(i), rPortID, rQbufInfo);
        mpPostProcPipe->enqueInBuf(rPortID, rQbufInfo);
        //
        if(size > 1)
        {
            MY_LOGD_IF(1, "P2I(%d-%d/0x%08X)",
                i,
                rQbufInfo.vBufInfo.at(0).memID,
                rQbufInfo.vBufInfo.at(0).u4BufVA);
        }
        else
        {
            MY_LOGD_IF(1, "P2I(%d/0x%08X)",
                rQbufInfo.vBufInfo.at(0).memID,
                rQbufInfo.vBufInfo.at(0).u4BufVA);
        }
        //
        mTwoRunRotInfo.inBuf = rQbufInfo;
    }
    //enque pass 2 out
    mpPostProcPipe->enqueOutBuf(vPortID.at(0), vQbufInfo.at(0));
    // revise config to "update" mode after the first configPipe
    mpPostProcPipe->sendCommand(EPIPECmd_SET_CONFIG_STAGE, (MINT32)eConfigSettingStage_UpdateTrigger, 0, 0);
    //
    return MTRUE;    
}


/*******************************************************************************
*  enque Pass 2 Two Run Rot:
********************************************************************************/
MBOOL
VSSScenario::
enquePass2TwoRunPass2(
    vector<PortBufInfo> *pBufIn,
    vector<PortBufInfo> *pBufOut)
{
    MUINT32 i,size;
    // [pass 2 In]
    vector<PortInfo const*> vPostProcInPorts;
    vPostProcInPorts.push_back(&mSettingPorts.imgi);
    // [pass 2 Out]
    vector<PortInfo const*> vPostProcOutPorts;
    vector<PortID> vPortID;
    vector<QBufInfo> vQbufInfo;
    //
    mTwoRunPass2Info.dispo.bufInfo.vBufInfo.clear();
    mTwoRunPass2Info.vido.bufInfo.vBufInfo.clear();
    //
    for(i=0; i<pBufOut->size(); i++)
    {
        if(pBufOut->at(i).ePortIndex == eID_Pass2DISPO)
        {
            mapConfig(pBufOut->at(i), mTwoRunPass2Info.dispo.portId, mTwoRunPass2Info.dispo.bufInfo);
            mTwoRunPass2Info.dispo.portInfo = mSettingPorts.dispo;
        }
        else
        if(pBufOut->at(i).ePortIndex == eID_Pass2VIDO)
        {
            mapConfig(pBufOut->at(i), mTwoRunPass2Info.vido.portId, mTwoRunPass2Info.vido.bufInfo);
            mTwoRunPass2Info.vido.portInfo = mSettingPorts.vido;
        }
    }
    //
    if(mbTwoRunPass2Dispo)
    {
        vPortID.push_back(mTwoRunPass2Info.dispo.portId);
        vQbufInfo.push_back(mTwoRunPass2Info.dispo.bufInfo);
        mTwoRunPass2Info.tempPort.portInfo = mSettingPorts.dispo;
        size = mTwoRunPass2Info.dispo.bufInfo.vBufInfo[0].u4BufSize;
    }
    else
    {
        vPortID.push_back(mTwoRunPass2Info.vido.portId);
        vQbufInfo.push_back(mTwoRunPass2Info.vido.bufInfo);
        mTwoRunPass2Info.tempPort.portInfo= mSettingPorts.vido;
        size = mTwoRunPass2Info.vido.bufInfo.vBufInfo[0].u4BufSize;
    }
    //
    allocTwoRunPass2TempBuf(size);
    //
    vQbufInfo[0].vBufInfo[0].memID = mTwoRunPass2Info.tempBuf.memID;
    vQbufInfo[0].vBufInfo[0].u4BufVA = mTwoRunPass2Info.tempBuf.virtAddr;
    vQbufInfo[0].vBufInfo[0].u4BufPA = mTwoRunPass2Info.tempBuf.phyAddr;
    //
    mTwoRunPass2Info.tempPort.portId = vPortID[0];
    mTwoRunPass2Info.tempPort.bufInfo = vQbufInfo[0];
    mTwoRunPass2Info.tempPort.portInfo.eImgRot = eImgRot_0;
    vPostProcOutPorts.push_back(&mTwoRunPass2Info.tempPort.portInfo);
    //
    mpPostProcPipe->configPipe(vPostProcInPorts, vPostProcOutPorts);
    //enque pass 2 in
    size = pBufIn->size();
    for (MUINT32 i = 0; i < size; i++)
    {
        PortID rPortID;
        QBufInfo rQbufInfo;
        mapConfig(pBufIn->at(i), rPortID, rQbufInfo);
        mpPostProcPipe->enqueInBuf(rPortID, rQbufInfo);
        //
        if(size > 1)
        {
            MY_LOGD_IF(1, "P2I(%d-%d/0x%08X)",
                i,
                rQbufInfo.vBufInfo.at(0).memID,
                rQbufInfo.vBufInfo.at(0).u4BufVA);
        }
        else
        {
            MY_LOGD_IF(1, "P2I(%d/0x%08X)",
                rQbufInfo.vBufInfo.at(0).memID,
                rQbufInfo.vBufInfo.at(0).u4BufVA);
        }
        //
        mTwoRunRotInfo.inBuf = rQbufInfo;
    }
    //enque pass 2 out
    mpPostProcPipe->enqueOutBuf(vPortID.at(0), vQbufInfo.at(0));
    // revise config to "update" mode after the first configPipe
    mpPostProcPipe->sendCommand(EPIPECmd_SET_CONFIG_STAGE, (MINT32)eConfigSettingStage_UpdateTrigger, 0, 0);
    //
    return MTRUE;    
}



/*******************************************************************************
*  deque:
********************************************************************************/
MBOOL
VSSScenario::
deque(
    MUINT32 port,
    vector<PortQTBufInfo> *pBufIn)
{
    MBOOL result = MTRUE;
    if ( ! pBufIn )
    {
        MY_LOGE("pBufIn==NULL");
        return MFALSE;
    }

    if ( port == eID_Unknown )
    {
        MY_LOGE("port == eID_Unknown");
        return MFALSE;
    }

    MY_LOGD_IF(0, "+ port(0x%X)",port);

    //(1.1) wait pass 1 done
    if (port & (eID_Pass1Out|eID_Pass1DispOut))
    {
        result = dequePass1(port, pBufIn);
    }
    //(1.2) wait pass 2 done
    if ((port & eID_Pass2DISPO) || (port & eID_Pass2VIDO))
    {
        if(mbTwoRunRot)
        {
            result = dequePass2TwoRunRot(port, pBufIn);
        }
        else
        {
            if(mbTwoRunPass2)
            {
                result = dequePass2TwoRunPass2(port, pBufIn);
        }
        else
        {
            result = dequePass2(port, pBufIn);
        }
    }
    }
    MY_LOGD_IF(0, "- port(0x%X)",port);
    //
    return result;
}


/*******************************************************************************
*  deque:
********************************************************************************/
MBOOL
VSSScenario::
dequePass1(
    MUINT32 port,
    vector<PortQTBufInfo> *pBufIn)
{
    //MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "DQP1 (0x%x)", port);

#define DEQUEPass1( flag, eID_port, queue )                                     \
    do {                                                                        \
        if( flag & eID_port )                                                   \
        {                                                                       \
            PortID rPortID;                                                     \
            mapPortCfg(eID_port, rPortID);                                      \
            PortQTBufInfo one(eID_port);                                        \
            if ( ! mpCamIOPipe->dequeOutBuf(rPortID, one.bufInfo))              \
            {                                                                   \
                MY_LOGE("mpCamIOPipe->dequeOutBuf %s fail", #eID_port);         \
                AEE_ASSERT("ISP deque fail:sensor may not output enough data!");\
                return MFALSE;                                                  \
            }                                                                   \
            if ( one.bufInfo.vBufInfo.size() )                                  \
            {                                                                   \
                queue->push_back(one);                                          \
            }                                                                   \
            else                                                                \
            {                                                                   \
                MY_LOGE("Pass 1 deque %s without buffer", #eID_port);           \
            }                                                                   \
        }                                                                       \
    }while(0)
    
    DEQUEPass1( port, eID_Pass1Out, pBufIn );
    DEQUEPass1( port, eID_Pass1DispOut, pBufIn );

#undef DEQUEPass1
    
    vector<PortQTBufInfo>::const_iterator iter;
    for (iter = pBufIn->begin(); iter != pBufIn->end(); ++iter )
    {
        // Pass1 buffers are all single-planed
        MY_LOGD_IF(ENABLE_LOG_PER_FRAME && iter->bufInfo.vBufInfo.size(),
                    "P1(0x%x %d/0x%08X/0x%08X/%d/%d.%06d)",
                    iter->ePortIndex,
                    iter->bufInfo.vBufInfo.at(0).memID,
                    iter->bufInfo.vBufInfo.at(0).u4BufVA,
                    iter->bufInfo.vBufInfo.at(0).u4BufPA,
                    iter->bufInfo.vBufInfo.at(0).u4BufSize,
                    iter->bufInfo.vBufInfo.at(0).i4TimeStamp_sec,
                    iter->bufInfo.vBufInfo.at(0).i4TimeStamp_us);
        //
        if(mbP1DispOut)
        {
            if(iter->ePortIndex == eID_Pass1DispOut)
            {
                if(mpExtImgProcHw != NULL)
                {
                    if(mpExtImgProcHw->getImgMask() & ExtImgProcHw::BufType_ISP_VSS_P1_Out)
                    {
                        IExtImgProc::ImgInfo img;
                        //
                        img.bufType     = ExtImgProcHw::BufType_ISP_VSS_P1_Out;
                        img.format      = msPass1OutFmt;
                        img.width       = mSettingPorts.img2o.u4ImgWidth;
                        img.height      = mSettingPorts.img2o.u4ImgHeight;
                        img.stride[0]   = mSettingPorts.img2o.u4Stride[ESTRIDE_1ST_PLANE];
                        img.stride[1]   = mSettingPorts.img2o.u4Stride[ESTRIDE_2ND_PLANE];
                        img.stride[2]   = mSettingPorts.img2o.u4Stride[ESTRIDE_3RD_PLANE];
                        img.virtAddr    = iter->bufInfo.vBufInfo.at(0).u4BufVA;
                        img.bufSize     = iter->bufInfo.vBufInfo.at(0).u4BufSize;
                        //
                        mpExtImgProcHw->doImgProc(img);
                    }
                }
            }
        }
        else
        {
            if(iter->ePortIndex == eID_Pass1Out)
            {
                if(mpExtImgProcHw != NULL)
                {
                    if(mpExtImgProcHw->getImgMask() & ExtImgProcHw::BufType_ISP_VSS_P1_Out)
                    {
                        IExtImgProc::ImgInfo img;
                        //
                        img.bufType     = ExtImgProcHw::BufType_ISP_VSS_P1_Out;
                        img.format      = msPass1OutFmt;
                        img.width       = mSettingPorts.imgo.u4ImgWidth;
                        img.height      = mSettingPorts.imgo.u4ImgHeight;
                        img.stride[0]   = mSettingPorts.imgo.u4Stride[ESTRIDE_1ST_PLANE];
                        img.stride[1]   = mSettingPorts.imgo.u4Stride[ESTRIDE_2ND_PLANE];
                        img.stride[2]   = mSettingPorts.imgo.u4Stride[ESTRIDE_3RD_PLANE];
                        img.virtAddr    = iter->bufInfo.vBufInfo.at(0).u4BufVA;
                        img.bufSize     = iter->bufInfo.vBufInfo.at(0).u4BufSize;
                        //
                        mpExtImgProcHw->doImgProc(img);
                    }
                }
            }
        }
    }
    //
    return MTRUE;     
}


/*******************************************************************************
*  deque Pass 2 Simple:
********************************************************************************/
MBOOL
VSSScenario::
dequePass2(
    MUINT32 port,
    vector<PortQTBufInfo> *pBufIn)
{
    MY_LOGD_IF(0, "SCB");
    //
    mpPostProcPipe->sendCommand((MINT32)EPIPECmd_SET_CURRENT_BUFFER, (MINT32)EPortIndex_IMGI,0,0);
    //
    if (port & eID_Pass2DISPO)
    {
        mpPostProcPipe->sendCommand((MINT32)EPIPECmd_SET_CURRENT_BUFFER, (MINT32)EPortIndex_DISPO,0,0);
    }
    if (port & eID_Pass2VIDO)
    {
        mpPostProcPipe->sendCommand((MINT32)EPIPECmd_SET_CURRENT_BUFFER, (MINT32)EPortIndex_VIDO,0,0);
    }
    //
    MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "SR");
    mpPostProcPipe->start();
    MY_LOGD_IF(0, "IRQ");
    mpPostProcPipe->irq(EPipePass_PASS2,EPIPEIRQ_PATH_DONE);
    //
    if (port & eID_Pass2DISPO)
    {
        PortID rPortID;
        mapPortCfg(eID_Pass2DISPO, rPortID);
        PortQTBufInfo one(eID_Pass2DISPO);
        MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "DQD");
        if(!mpPostProcPipe->dequeOutBuf(rPortID, one.bufInfo))
        {
            MY_LOGE("MDP deque DISPO fail");
            AEE_ASSERT("MDP deque DISPO fail");
            goto EXIT;
        }
        pBufIn->push_back(one);
        //
        MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "DISPO(%d/0x%08X)",
            one.bufInfo.vBufInfo.at(0).memID,
            one.bufInfo.vBufInfo.at(0).u4BufVA);
    }
    if (port & eID_Pass2VIDO)
    {
        PortID rPortID;
        mapPortCfg(eID_Pass2VIDO, rPortID);
        PortQTBufInfo one(eID_Pass2VIDO);
        MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "DQV");
        if(!mpPostProcPipe->dequeOutBuf(rPortID, one.bufInfo))
        {
            MY_LOGE("MDP deque VIDO fail");
            AEE_ASSERT("MDP deque VIDO fail");
            goto EXIT;
        }
        pBufIn->push_back(one);
        //
        MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "VIDO(%d/0x%08X)",
            one.bufInfo.vBufInfo.at(0).memID,
            one.bufInfo.vBufInfo.at(0).u4BufVA);            
    }
    // deque out pass2 in buffer
    {
        PortID rPortID;
        mapPortCfg(eID_Pass2In, rPortID);
        QTimeStampBufInfo dummy;
        MY_LOGD_IF(0, "DQI");
        mpPostProcPipe->dequeInBuf(rPortID, dummy);
    }
    //
    EXIT:
    MY_LOGD_IF(0, "SP");
    mpPostProcPipe->stop();
    //
    return MTRUE;    
}


/*******************************************************************************
*  deque Pass2 Two Run Rot:
********************************************************************************/
MBOOL
VSSScenario::
dequePass2TwoRunRot(
    MUINT32 port,
    vector<PortQTBufInfo> *pBufIn)
{
    MBOOL bTwoRun = MFALSE;
    PortQTBufInfo one(eID_Pass2VIDO);
    //
    MY_LOGD_IF(0, "SCB");
    //
    mpPostProcPipe->sendCommand((MINT32)EPIPECmd_SET_CURRENT_BUFFER, (MINT32)EPortIndex_IMGI,0,0);
    mpPostProcPipe->sendCommand((MINT32)EPIPECmd_SET_CURRENT_BUFFER, (MINT32)EPortIndex_VIDO,0,0);
    //
    MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "SR");
    mpPostProcPipe->start();
    MY_LOGD_IF(0, "IRQ");
    mpPostProcPipe->irq(EPipePass_PASS2,EPIPEIRQ_PATH_DONE);
    // case (1): DISPO, VIDO  
    if ( (port & eID_Pass2DISPO) && (port & eID_Pass2VIDO))
    {
        MY_LOGD_IF(0, "DISPO&VIDO");
        PortID rPortID;
        // (.1) deque VIDO 
        MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "DQD");
        mapPortCfg(eID_Pass2VIDO, rPortID);
        if(!mpPostProcPipe->dequeOutBuf(rPortID, one.bufInfo))
        {
            MY_LOGE("MDP deque DISPO fail");
            AEE_ASSERT("MDP deque DISPO fail");
            goto EXIT;
        }
        pBufIn->push_back(one);
        // (.2) deque pass2 in buffer
        mapPortCfg(eID_Pass2In, rPortID);
        QTimeStampBufInfo pass2InBuf;
        mpPostProcPipe->dequeInBuf(rPortID, pass2InBuf);
        // (.3) pass 2nd run: pass2-in  --> DISPO buffer (use VIDO port)
        //gInterInfo.inBuf.vBufInfo = pass2InBuf.vBufInfo;
        bTwoRun = MTRUE;
    }
    // case (2): DISPO
    // case (3): VIDO        
    else
    {
        MY_LOGD_IF(0, "VIDO");
        // (.1) no matter case 2 or 3, take from vido.
        PortID rPortID;
        MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "DQV");
        mapPortCfg(eID_Pass2VIDO, rPortID);
        if(!mpPostProcPipe->dequeOutBuf(rPortID, one.bufInfo))
        {
            MY_LOGE("MDP deque VIDO fail");
            AEE_ASSERT("MDP deque VIDO fail");
            goto EXIT;
        }
        pBufIn->push_back(one);
        // (.2) deque pass2 in 
        mapPortCfg(eID_Pass2In, rPortID);
        QTimeStampBufInfo dummy;
        mpPostProcPipe->dequeInBuf(rPortID, dummy);
    }
    //
    MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "VIDO(%d/0x%08X)",
        one.bufInfo.vBufInfo.at(0).memID,
        one.bufInfo.vBufInfo.at(0).u4BufVA);  
    // --[diff] 
    EXIT:
    mpPostProcPipe->stop();
    //
    if(bTwoRun)
    {
        PortID rPortID;
        QTimeStampBufInfo dummy;
        vector<PortInfo const*> vPostProcInPorts;
        vector<PortInfo const*> vPostProcOutPorts;
        //
        vPostProcInPorts.push_back(&mSettingPorts.imgi);
        vPostProcOutPorts.push_back(&mTwoRunRotInfo.outPort);
        //
        MY_LOGD_IF(0, "TR:configPipe");
        mpPostProcPipe->configPipe(vPostProcInPorts, vPostProcOutPorts);
        //
        MY_LOGD_IF(0, "TR:enqueInBuf");
        mapPortCfg(eID_Pass2In, rPortID);    
        mpPostProcPipe->enqueInBuf(rPortID, mTwoRunRotInfo.inBuf);
        MY_LOGD_IF(1, "TR:In(%d/0x%08X)",
            mTwoRunRotInfo.inBuf.vBufInfo.at(0).memID,
            mTwoRunRotInfo.inBuf.vBufInfo.at(0).u4BufVA);
        //
        MY_LOGD_IF(0, "TR:enqueOutBuf");
        mapPortCfg(eID_Pass2VIDO, rPortID);    
        mpPostProcPipe->enqueOutBuf(rPortID, mTwoRunRotInfo.outBuf);
        MY_LOGD_IF(1, "TR:Out(%d/0x%08X)",
            mTwoRunRotInfo.outBuf.vBufInfo.at(0).memID,
            mTwoRunRotInfo.outBuf.vBufInfo.at(0).u4BufVA);
        //
        mpPostProcPipe->sendCommand(EPIPECmd_SET_CONFIG_STAGE, (MINT32)eConfigSettingStage_UpdateTrigger, 0, 0);
        mpPostProcPipe->sendCommand((MINT32)EPIPECmd_SET_CURRENT_BUFFER, (MINT32)EPortIndex_IMGI,0,0);
        mpPostProcPipe->sendCommand((MINT32)EPIPECmd_SET_CURRENT_BUFFER, (MINT32)EPortIndex_VIDO,0,0);
        //
        MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "TR:SR");
        mpPostProcPipe->start();
        MY_LOGD_IF(0, "TR:IRQ");
        mpPostProcPipe->irq(EPipePass_PASS2, EPIPEIRQ_PATH_DONE);
        MY_LOGD_IF(0, "TR:DONE");
        //
        mapPortCfg(eID_Pass2VIDO, rPortID);
        PortQTBufInfo out(eID_Pass2VIDO);
        MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "TR:DQV");
        if(!mpPostProcPipe->dequeOutBuf(rPortID, out.bufInfo))
        {
            MY_LOGE("MDP deque VIDO fail");
            AEE_ASSERT("MDP deque VIDO fail");
            goto EXIT2;
        }
        //
        mapPortCfg(eID_Pass2In, rPortID);
        MY_LOGD_IF(0, "TR:DQI");
        mpPostProcPipe->dequeInBuf(rPortID, dummy);    
        //
        pBufIn->push_back(out);
        //
        MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "TR:VIDO(%d/0x%08X)",
            out.bufInfo.vBufInfo.at(0).memID,
            out.bufInfo.vBufInfo.at(0).u4BufVA); 
        //
        EXIT2:
        MY_LOGD_IF(0, "TR:SP");
        mpPostProcPipe->stop();
    }
    //
    return MTRUE;    
}



/*******************************************************************************
*  deque Pass2 Two Run Pass2:
********************************************************************************/
MBOOL
VSSScenario::
dequePass2TwoRunPass2(
    MUINT32 port,
    vector<PortQTBufInfo> *pBufIn)
{
    PortID rPortID;
    vector<NSCamPipe::PortInfo const*> vInPorts; 
    vector<NSCamPipe::PortInfo const*> vOutPorts;
    //
    MY_LOGD_IF(0, "SCB");
    //
    mpPostProcPipe->sendCommand((MINT32)EPIPECmd_SET_CURRENT_BUFFER, (MINT32)EPortIndex_IMGI,0,0);
    //
    if(mbTwoRunPass2Dispo)
    {
        mpPostProcPipe->sendCommand((MINT32)EPIPECmd_SET_CURRENT_BUFFER, (MINT32)EPortIndex_DISPO,0,0);
    }
    else
    {
        mpPostProcPipe->sendCommand((MINT32)EPIPECmd_SET_CURRENT_BUFFER, (MINT32)EPortIndex_VIDO,0,0);
    }
    //
    MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "SR");
    mpPostProcPipe->start();
    MY_LOGD_IF(0, "IRQ");
    mpPostProcPipe->irq(EPipePass_PASS2,EPIPEIRQ_PATH_DONE);
    //
    if(mbTwoRunPass2Dispo)
    {
        mapPortCfg(eID_Pass2DISPO, rPortID);
        PortQTBufInfo dispoBuf(eID_Pass2DISPO);
        MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "DQT");
        if(!mpPostProcPipe->dequeOutBuf(rPortID, dispoBuf.bufInfo))
        {
            MY_LOGE("MDP deque DISPO fail");
            AEE_ASSERT("MDP deque DISPO fail");
            mpPostProcPipe->stop();
            return MFALSE;
        }
        MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "Temp(%d/0x%08X)",
            dispoBuf.bufInfo.vBufInfo.at(0).memID,
            dispoBuf.bufInfo.vBufInfo.at(0).u4BufVA);
    }
    else
    {
        mapPortCfg(eID_Pass2VIDO, rPortID);
        PortQTBufInfo vidoBuf(eID_Pass2VIDO);
        MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "DQT");
        if(!mpPostProcPipe->dequeOutBuf(rPortID, vidoBuf.bufInfo))
        {
            MY_LOGE("MDP deque VIDO fail");
            AEE_ASSERT("MDP deque VIDO fail");
            mpPostProcPipe->stop();
            return MFALSE;
        }
        MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "Temp(%d/0x%08X)",
            vidoBuf.bufInfo.vBufInfo.at(0).memID,
            vidoBuf.bufInfo.vBufInfo.at(0).u4BufVA);
    }
    //
    MY_LOGD_IF(0, "SP");
    mpPostProcPipe->stop();
    //
    if(mpExtImgProcHw != NULL)
    {
        if(mpExtImgProcHw->getImgMask() & ExtImgProcHw::BufType_ISP_VSS_P2_TwoRun_In)
        {
            IExtImgProc::ImgInfo img;
            //
            img.bufType     = ExtImgProcHw::BufType_ISP_VSS_P2_TwoRun_In;
            if(mbTwoRunPass2Dispo)
            {
                img.format      = msPass2DispoOutFmt;
            }
            else
            {
                img.format      = msPass2VidoOutFmt;
            }
            img.width       = mTwoRunPass2Info.tempPort.portInfo.u4ImgWidth;
            img.height      = mTwoRunPass2Info.tempPort.portInfo.u4ImgHeight;
            img.stride[0]   = mTwoRunPass2Info.tempPort.portInfo.u4Stride[ESTRIDE_1ST_PLANE];
            img.stride[1]   = mTwoRunPass2Info.tempPort.portInfo.u4Stride[ESTRIDE_2ND_PLANE];
            img.stride[2]   = mTwoRunPass2Info.tempPort.portInfo.u4Stride[ESTRIDE_3RD_PLANE];
            img.virtAddr    = mTwoRunPass2Info.tempPort.bufInfo.vBufInfo[0].u4BufVA;
            img.bufSize     = mTwoRunPass2Info.tempPort.bufInfo.vBufInfo[0].u4BufSize;
            //
            mpExtImgProcHw->doImgProc(img);
        }
    }
    //
    MY_LOGD_IF(0, "2nd:SCB");
    mpXdpPipe->setCallbacks(NULL, NULL, NULL); 
    //IMGI
    NSCamPipe::MemoryInPortInfo rMemInPort(
                                    NSCamHW::ImgInfo(
                                        mTwoRunPass2Info.tempPort.portInfo.eImgFmt,
                                        mTwoRunPass2Info.tempPort.portInfo.u4ImgWidth,
                                        mTwoRunPass2Info.tempPort.portInfo.u4ImgHeight),
                                    0,
                                    mTwoRunPass2Info.tempPort.portInfo.u4Stride,
                                    Rect(
                                        mTwoRunPass2Info.tempPort.portInfo.crop.x,
                                        mTwoRunPass2Info.tempPort.portInfo.crop.y,
                                        mTwoRunPass2Info.tempPort.portInfo.crop.w,
                                        mTwoRunPass2Info.tempPort.portInfo.crop.h));
    rMemInPort.u4Stride[0] = mTwoRunPass2Info.tempPort.portInfo.u4Stride[0];
    rMemInPort.u4Stride[1] = mTwoRunPass2Info.tempPort.portInfo.u4Stride[1];
    rMemInPort.u4Stride[2] = mTwoRunPass2Info.tempPort.portInfo.u4Stride[2];
    vInPorts.push_back(&rMemInPort);
    //DISPO
    NSCamPipe::MemoryOutPortInfo rDispPort(
                                    NSCamHW::ImgInfo(
                                        mTwoRunPass2Info.dispo.portInfo.eImgFmt,
                                        mTwoRunPass2Info.dispo.portInfo.u4ImgWidth,
                                        mTwoRunPass2Info.dispo.portInfo.u4ImgHeight),
                                    mTwoRunPass2Info.dispo.portInfo.u4Stride,
                                    0,
                                    0);
    rDispPort.u4Stride[0] = mTwoRunPass2Info.dispo.portInfo.u4Stride[0];
    rDispPort.u4Stride[1] = mTwoRunPass2Info.dispo.portInfo.u4Stride[1];
    rDispPort.u4Stride[2] = mTwoRunPass2Info.dispo.portInfo.u4Stride[2];
    rDispPort.index = 0;
    if(port & eID_Pass2DISPO)
    {
        vOutPorts.push_back(&rDispPort);
    }
    //VIDO
    NSCamPipe::MemoryOutPortInfo rVdoPort(
                                    NSCamHW::ImgInfo(
                                        mTwoRunPass2Info.vido.portInfo.eImgFmt,
                                        mTwoRunPass2Info.vido.portInfo.u4ImgWidth,
                                        mTwoRunPass2Info.vido.portInfo.u4ImgHeight),
                                    mTwoRunPass2Info.vido.portInfo.u4Stride,
                                    mTwoRunPass2Info.vido.portInfo.eImgRot,
                                    0);
    rVdoPort.u4Stride[0] = mTwoRunPass2Info.vido.portInfo.u4Stride[0];
    rVdoPort.u4Stride[1] = mTwoRunPass2Info.vido.portInfo.u4Stride[1];
    rVdoPort.u4Stride[2] = mTwoRunPass2Info.vido.portInfo.u4Stride[2];
    rVdoPort.index = 1;
    if(port & eID_Pass2VIDO)
    {
        vOutPorts.push_back(&rVdoPort);
    }
    //
    mpXdpPipe->configPipe(
                    vInPorts,
                    vOutPorts); 
    //IMGI
    MY_LOGD_IF(0,"2nd:EQI");
    NSCamPipe::QBufInfo rInQBuf;
    NSCamHW::BufInfo rInBufInfo(
                        mTwoRunPass2Info.tempPort.bufInfo.vBufInfo[0].u4BufSize,
                        mTwoRunPass2Info.tempPort.bufInfo.vBufInfo[0].u4BufVA,
                        mTwoRunPass2Info.tempPort.bufInfo.vBufInfo[0].u4BufPA,
                        mTwoRunPass2Info.tempPort.bufInfo.vBufInfo[0].memID);  
    rInQBuf.vBufInfo.push_back(rInBufInfo);
    mpXdpPipe->enqueBuf(
                NSCamPipe::PortID(
                    NSCamPipe::EPortType_MemoryIn,
                    0,
                    0),
                rInQBuf); 
    //DISPO
    NSCamPipe::QBufInfo rDispQBuf; 
    NSCamHW::BufInfo rDispBufInfo;
    if(port & eID_Pass2DISPO)
    {
        rDispBufInfo.u4BufSize  = mTwoRunPass2Info.dispo.bufInfo.vBufInfo[0].u4BufSize;
        rDispBufInfo.u4BufVA    = mTwoRunPass2Info.dispo.bufInfo.vBufInfo[0].u4BufVA;
        rDispBufInfo.u4BufPA    = mTwoRunPass2Info.dispo.bufInfo.vBufInfo[0].u4BufPA;
        rDispBufInfo.i4MemID    = mTwoRunPass2Info.dispo.bufInfo.vBufInfo[0].memID;
        rDispQBuf.vBufInfo.push_back(rDispBufInfo);
        MY_LOGD_IF(0,"2nd:EQD");
        mpXdpPipe->enqueBuf(
                    NSCamPipe::PortID(
                        NSCamPipe::EPortType_MemoryOut,
                        0,
                        1),
                    rDispQBuf);
    }
    //VIDO
    NSCamPipe::QBufInfo rVdoQBuf; 
    NSCamHW::BufInfo rVdoBufInfo;
    if(port & eID_Pass2VIDO)
    {
        rVdoBufInfo.u4BufSize   = mTwoRunPass2Info.vido.bufInfo.vBufInfo[0].u4BufSize;
        rVdoBufInfo.u4BufVA     = mTwoRunPass2Info.vido.bufInfo.vBufInfo[0].u4BufVA;
        rVdoBufInfo.u4BufPA     = mTwoRunPass2Info.vido.bufInfo.vBufInfo[0].u4BufPA;
        rVdoBufInfo.i4MemID     = mTwoRunPass2Info.vido.bufInfo.vBufInfo[0].memID;
        rVdoQBuf.vBufInfo.push_back(rVdoBufInfo); 
        MY_LOGD_IF(0,"2nd:EQV");
        mpXdpPipe->enqueBuf(
                    NSCamPipe::PortID(
                        NSCamPipe::EPortType_MemoryOut,
                        1,
                        1),
                    rVdoQBuf);
    }
    //
    MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "2nd:SR");
    mpXdpPipe->start();
    //
    if(port & eID_Pass2DISPO)
    {
        MY_LOGD_IF(ENABLE_LOG_PER_FRAME,"2nd:DQD");
        NSCamPipe::QTimeStampBufInfo rQDispOutBuf; 
        if(mpXdpPipe->dequeBuf(NSCamPipe::PortID(NSCamPipe::EPortType_MemoryOut,0,1),rQDispOutBuf))
        {
            PortQTBufInfo one(eID_Pass2DISPO);
            NSIspio::BufInfo dispoBufInfo;
            //
            dispoBufInfo.u4BufSize          = rQDispOutBuf.vBufInfo[0].u4BufSize;
            dispoBufInfo.u4BufVA            = rQDispOutBuf.vBufInfo[0].u4BufVA;
            dispoBufInfo.u4BufPA            = rQDispOutBuf.vBufInfo[0].u4BufPA;
            dispoBufInfo.memID              = rQDispOutBuf.vBufInfo[0].i4MemID;
            dispoBufInfo.bufSecu            = rQDispOutBuf.vBufInfo[0].i4BufSecu;
            dispoBufInfo.bufCohe            = rQDispOutBuf.vBufInfo[0].i4BufCohe;
            dispoBufInfo.i4TimeStamp_sec    = 0;
            dispoBufInfo.i4TimeStamp_us     = 0;
            //
            one.bufInfo.i4TimeStamp_sec = rQDispOutBuf.i4TimeStamp_sec;
            one.bufInfo.i4TimeStamp_us  = rQDispOutBuf.i4TimeStamp_us;
            one.bufInfo.u4User          = rQDispOutBuf.u4User;
            one.bufInfo.u4Reserved      = rQDispOutBuf.u4Reserved;
            one.bufInfo.u4BufIndex      = 0;
            one.bufInfo.vBufInfo.push_back(dispoBufInfo);
            //
            pBufIn->push_back(one);
            MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "DISPO(%d/0x%08X)",
                one.bufInfo.vBufInfo.at(0).memID,
                one.bufInfo.vBufInfo.at(0).u4BufVA);
        }
        else
        {
            MY_LOGE("MDP deque DISPO fail");
            AEE_ASSERT("MDP deque DISPO fail");
            mpXdpPipe->stop();
            return MFALSE;
        }
    }
    //
    if(port & eID_Pass2VIDO)
    {
        MY_LOGD_IF(ENABLE_LOG_PER_FRAME,"2nd:DQV");
        NSCamPipe::QTimeStampBufInfo rQVdoOutBuf; 
        if(mpXdpPipe->dequeBuf(NSCamPipe::PortID(NSCamPipe::EPortType_MemoryOut,1,1),rQVdoOutBuf))
        {
            PortQTBufInfo one(eID_Pass2VIDO);
            NSIspio::BufInfo vidoBufInfo;
            //
            vidoBufInfo.u4BufSize       = rQVdoOutBuf.vBufInfo[0].u4BufSize;
            vidoBufInfo.u4BufVA         = rQVdoOutBuf.vBufInfo[0].u4BufVA;
            vidoBufInfo.u4BufPA         = rQVdoOutBuf.vBufInfo[0].u4BufPA;
            vidoBufInfo.memID           = rQVdoOutBuf.vBufInfo[0].i4MemID;
            vidoBufInfo.bufSecu         = rQVdoOutBuf.vBufInfo[0].i4BufSecu;
            vidoBufInfo.bufCohe         = rQVdoOutBuf.vBufInfo[0].i4BufCohe;
            vidoBufInfo.i4TimeStamp_sec = 0;
            vidoBufInfo.i4TimeStamp_us  = 0;
            //
            one.bufInfo.i4TimeStamp_sec = rQVdoOutBuf.i4TimeStamp_sec;
            one.bufInfo.i4TimeStamp_us  = rQVdoOutBuf.i4TimeStamp_us;
            one.bufInfo.u4User          = rQVdoOutBuf.u4User;
            one.bufInfo.u4Reserved      = rQVdoOutBuf.u4Reserved;
            one.bufInfo.u4BufIndex      = 0;
            one.bufInfo.vBufInfo.push_back(vidoBufInfo);
            //
            pBufIn->push_back(one);
            MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "VIDO(%d/0x%08X)",
                one.bufInfo.vBufInfo.at(0).memID,
                one.bufInfo.vBufInfo.at(0).u4BufVA);
        }
        else
        {
            MY_LOGE("MDP deque VIDO fail");
            AEE_ASSERT("MDP deque VIDO fail");
            mpXdpPipe->stop();
            return MFALSE;
        }
    }
    //
    MY_LOGD_IF(0,"2nd:DQI");
    NSCamPipe::QTimeStampBufInfo rQInBuf;
    mpXdpPipe->dequeBuf(
                    NSCamPipe::PortID(
                        NSCamPipe::EPortType_MemoryIn,
                        0,
                        0),
                    rQInBuf);
    //
    MY_LOGD_IF(0,"2nd:SP");
    mpXdpPipe->stop();
    //
    return MTRUE;    
}



/*******************************************************************************
*  replaceQue:
********************************************************************************/
MBOOL
VSSScenario::
replaceQue(vector<PortBufInfo> *pBufOld, vector<PortBufInfo> *pBufNew)
{
    //
    vector<PortID> vPort;
    QBufInfo rQbufInfo;

    MY_LOGE_IF( pBufOld->size() != pBufNew->size(), "replace ques with different size");

    vector<PortBufInfo>::const_iterator pOld, pNew;
    pOld = pBufOld->begin();
    pNew = pBufNew->begin();
    while( pOld != pBufOld->end() && pNew != pBufNew->end() )
    {
        PortID rPortID;
        mapConfig(*pOld, rPortID, rQbufInfo);
        mapConfig(*pNew, rPortID, rQbufInfo);
        vPort.push_back(rPortID);
    //
    MY_LOGD("P1:Old(%d/0x%08X),New(%d/0x%08X)",
                pOld->memID,
                pOld->virtAddr,
                pNew->memID,
                pNew->virtAddr);
        pOld++;
        pNew++;
    }

    while( vPort.size() )
    {
        mpCamIOPipe->enqueOutBuf(vPort.front(), rQbufInfo);
        // clear one port's buf
        vPort.erase( vPort.begin() );
        rQbufInfo.vBufInfo.erase( rQbufInfo.vBufInfo.begin(),
                                  rQbufInfo.vBufInfo.begin() + 2 );
    }

    //
    return MTRUE;
}


/******************************************************************************
* This is used to check whether width or height exceed limitations of HW.
*******************************************************************************/
#define VSS_HW_LIMIT_LINE_BUF      (3264)
#define VSS_HW_LIMIT_FRAME_PIXEL   (3264*1836)
#define VSS_HW_VR_HRZ_WIDTH        (1280)
//
#define VSS_HW_LIMIT_HRZ_WIDTH      (0.5)
#define VSS_HW_LIMIT_HRZ_THRESHOLD  (125000000)
//
MVOID
VSSScenario::
getHwValidSize(MUINT32 id, MUINT32 &width, MUINT32 &height, MUINT32 fps, MBOOL tryResize)
{
    bool changed = MFALSE;
    MY_LOGD("Port 0x%x In:W(%u),H(%u) fps(%u)", id, width, height, fps);
    //
    switch(id)
    {
        case eID_Pass1Out:  
            if( !tryResize )
            { // for not affect other adapter
#if defined(VSS_HW_LIMIT_LINE_BUF)
                if(width > VSS_HW_LIMIT_LINE_BUF)
                {
                    width = VSS_HW_LIMIT_LINE_BUF;
                    changed = MTRUE;
                }
#endif
#if defined(VSS_HW_LIMIT_FRAME_PIXEL)
                // for video-recording, 16:9 is enough.
                if((width * height) > VSS_HW_LIMIT_FRAME_PIXEL)
                {
                    MY_LOGW("Frame pixel(%u x %u = %u) is larger than limitation(%u)",
                            width,
                            height,
                            (width * height),
                            VSS_HW_LIMIT_FRAME_PIXEL);
                    height = (width*9/16)&(~0x1);
                    changed = MTRUE;
                }
#endif
            }
            else
            { // for VR BW
                if(width > VSS_HW_VR_HRZ_WIDTH)
                {
                    MY_LOGD("w(%d) > limit(%d)", width, VSS_HW_VR_HRZ_WIDTH);
                    width = VSS_HW_VR_HRZ_WIDTH;        
                    changed = MTRUE;
                }
            }
            break;
        case eID_Pass1DispOut:
            changed = MTRUE;
            if(width * height * (fps/10) > VSS_HW_LIMIT_HRZ_THRESHOLD)
            {
                width *= VSS_HW_LIMIT_HRZ_WIDTH;
                width &= (~0x1);
            }
            else
            {
                width = 0;
                height = 0;
            }
            break;
        default:
            //do nothing
            break;
    }
    MY_LOGD( "changed(%d),Out:W(%u),H(%u)", changed, width, height);
}


/*******************************************************************************
*  calRotation:
*  need to crop frame buffer from 4:3 rectangle to be 3:4
********************************************************************************/
MVOID
VSSScenario::
calCrop(
    MUINT32     srcW,
    MUINT32     SrcH,
    MUINT32&    rCropW,
    MUINT32&    rCropH, 
    MUINT32&    rCropX,
    MUINT32&    rCropY)
{
    MUINT32 cropW,cropH;
    MUINT32 cropX,cropY;
    MUINT32 ratio_w = 100 * (float)srcW/rCropW;
    MUINT32 ratio_h = 100 * (float)SrcH/rCropH;
    MUINT32 ratio_h_w = 100 * (float)rCropH/rCropW;
    MUINT32 ratio_zoom = ratio_w < ratio_h ? ratio_w : ratio_h; 
    //
    MY_LOGD_IF(1,"In:Src(%d/%d),Crop(%d,%d,%d,%d)",
        srcW,
        SrcH,
        rCropX,
        rCropY,
        rCropW,
        rCropH);
    MY_LOGD_IF(0,"R(%d/%d),H/W(%d),RZ(%d)",
        ratio_w,
        ratio_h,
        ratio_h_w,
        ratio_zoom);
    //
    cropH = SrcH;
    cropW = cropH * ratio_h_w;
    //
    cropH = ROUND_TO_2X(100 * cropH/ratio_zoom);
    cropW = ROUND_TO_2X(cropW/ratio_zoom);
    //
    cropX = (srcW - cropW) / 2;
    cropY = (SrcH - cropH) / 2;
    //
    rCropW = cropW;
    rCropH = cropH;
    rCropX = cropX;
    rCropY = cropY;
    //
    MY_LOGD_IF(1,"Out:Crop(%d,%d,%d,%d)",
        rCropW,
        rCropH,
        rCropX,
        rCropY);
}


/*******************************************************************************
*  calRotation:
********************************************************************************/
EImageRotation
VSSScenario::
calRotation(EImageRotation rot)
{
    MUINT32 rotation  = rot == eImgRot_0 ? 0
                        : rot == eImgRot_90 ? 270
                        : rot == eImgRot_180 ? 180 : 90;
    // no matter facing
    MUINT32 diff = (android::MtkCamUtils::DevMetaInfo::queryDeviceWantedOrientation(mSensorId)-
                android::MtkCamUtils::DevMetaInfo::queryDeviceSetupOrientation(mSensorId)+ rotation + 360 ) % 360;
    //
    switch (diff)
    {
        case 270:
            return eImgRot_90;
        case 180:
            return eImgRot_180;
        case 90:
            return eImgRot_270;
        default:
            return eImgRot_0;
    }
    //
    return eImgRot_0;
}


/*******************************************************************************
*  enable2RunPass2:
********************************************************************************/
MVOID
VSSScenario::
enableTwoRunPass2(MBOOL en)
{
    MY_LOGD("en(%d)",en);
    mbTwoRunPass2 = en;
}



/*******************************************************************************
*  allocTwoRunPass2TempBuf:
********************************************************************************/
MBOOL
VSSScenario::
allocTwoRunPass2TempBuf(MUINT32 bufSize)
{
    MBOOL result = MTRUE;
    //
    if(bufSize == 0)
    {
        MY_LOGE("bufSize is 0");
        return MFALSE;
    }
    //
    if(mTwoRunPass2Info.tempBuf.size == bufSize)
    {
        //MY_LOGD("Buffer is allcoted already!");
        return MFALSE;
    }
    else
    if(mTwoRunPass2Info.tempBuf.size != 0)
    {
        MY_LOGW("re-allcote %d --> %d",mTwoRunPass2Info.tempBuf.size,bufSize);
        freeTwoRunPass2TempBuf();
    }
    //
    mTwoRunPass2Info.tempBuf.size = bufSize;
    //
    if(mpIMemDrv->allocVirtBuf(&mTwoRunPass2Info.tempBuf) < 0)
    {
        MY_LOGE("mpIMemDrv->allocVirtBuf() error");
        result = MFALSE;
        goto EXIT;
    }
    if(mpIMemDrv->mapPhyAddr(&mTwoRunPass2Info.tempBuf) < 0)
    {
        MY_LOGE("mpIMemDrv->mapPhyAddr() error");
        result = MFALSE;
        goto EXIT;
    }
    //
    MY_LOGD("Temp(%d,0x%08X/0x%08X,%d)",
            mTwoRunPass2Info.tempBuf.memID,
            mTwoRunPass2Info.tempBuf.virtAddr,
            mTwoRunPass2Info.tempBuf.phyAddr,
            mTwoRunPass2Info.tempBuf.size);
    //
    EXIT:
    return result;
}


/*******************************************************************************
*  freeTwoRunPass2TempBuf:
********************************************************************************/
MBOOL
VSSScenario::
freeTwoRunPass2TempBuf()
{
    MBOOL result = MTRUE;
    //
    if(mTwoRunPass2Info.tempBuf.size == 0)
    {
        MY_LOGW("Buffer is free already!");
        return MFALSE;
    }
    //
    MY_LOGD("Temp(%d,0x%08X/0x%08X,%d)",
            mTwoRunPass2Info.tempBuf.memID,
            mTwoRunPass2Info.tempBuf.virtAddr,
            mTwoRunPass2Info.tempBuf.phyAddr,
            mTwoRunPass2Info.tempBuf.size);
    //
    if(mpIMemDrv->unmapPhyAddr(&mTwoRunPass2Info.tempBuf) < 0)
    {
        MY_LOGE("mpIMemDrv->unmapPhyAddr() error");
        result = MFALSE;
        goto EXIT;
    }
    if(mpIMemDrv->freeVirtBuf(&mTwoRunPass2Info.tempBuf) < 0)
    {
        MY_LOGE("mpIMemDrv->freeVirtBuf() error");
        result = MFALSE;
        goto EXIT;
    }
    //
    mTwoRunPass2Info.tempBuf.size = 0;
    //
    EXIT:
    return result;
}


/*******************************************************************************
*  set AppCamMode:
********************************************************************************/
MVOID
VSSScenario::           
setCamMode(MUINT32 mode)
{    
    MY_LOGD(" +");
    
    if (mpCamIOPipe)
    {    
        mpCamIOPipe->sendCommand(EPIPECmd_SET_CAM_MODE,(MINT32)mode, 0, 0);
    }
    else
    {
        MY_LOGW("mpCamIOPipe is NULL");
    }
    
    if (mpPostProcPipe)
    {    
        mpPostProcPipe->sendCommand(EPIPECmd_SET_CAM_MODE,(MINT32)mode, 0, 0);
    }
    else
    {
        MY_LOGW("mpCamIOPipe is NULL");
    } 
}

