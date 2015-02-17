#include "SpeechANCController.h"
#include "AudioALSAStreamManager.h"
#include "audio_custom_exp.h"
#include "AudioCustParam.h"
#include "AudioType.h"
#include "AudioALSAHardware.h"

#define LOG_TAG "SpeechANCController"
#define param_anc_add
namespace android
{

/*==============================================================================
 *                     Singleton Pattern
 *============================================================================*/

SpeechANCController *SpeechANCController::UniqueSpeechANCController = NULL;


SpeechANCController *SpeechANCController::getInstance()
{
    static Mutex mGetInstanceLock;
    Mutex::Autolock _l(mGetInstanceLock);
    ALOGD("%s()", __FUNCTION__);

    if (UniqueSpeechANCController == NULL)
    {
        UniqueSpeechANCController = new SpeechANCController();
    }
    ASSERT(UniqueSpeechANCController != NULL);
    return UniqueSpeechANCController;
}
/*==============================================================================
 *                     Constructor / Destructor / Init / Deinit
 *============================================================================*/

SpeechANCController::SpeechANCController()
{
    ALOGD("%s()", __FUNCTION__);
    mEnabled       = false;
    mGroupANC      = false;
#if defined(MTK_ACTIVE_NOISE_CANCELLATION_SUPPORT)
    Init();
#endif
}

SpeechANCController::~SpeechANCController()
{
    ALOGD("%s()", __FUNCTION__);
#if defined(MTK_ACTIVE_NOISE_CANCELLATION_SUPPORT)
    if (mFd)
    {
        ::close(mFd);
        mFd = 0;
    }
#endif
}

/*==============================================================================
 *                     AudioANCControl Imeplementation
 *============================================================================*/
    bool SpeechANCController::GetANCSupport(void)
    {
        ALOGD("%s(), GetANCSupport:%d", __FUNCTION__, mApply);
        //TODO(Tina): return by project config
    
#if defined(MTK_ACTIVE_NOISE_CANCELLATION_SUPPORT)
        return true;
#else
        return false;
#endif
    
    }

#if defined(MTK_ACTIVE_NOISE_CANCELLATION_SUPPORT)

void SpeechANCController::Init()
{
    ALOGD("%s()", __FUNCTION__);
    mFd            = ::open(kANCDeviceName, O_RDWR);
    if (mFd < 0)
    {
        ALOGE("%s() fail to open %s", __FUNCTION__, kANCDeviceName);
    }
    else
    {
        ALOGD("%s() open %s success!", __FUNCTION__, kANCDeviceName);

        ::ioctl(mFd, SET_ANC_CONTROL, ANCControlCmd_Init);
    }
#ifdef param_anc_add
    AUDIO_ANC_CUSTOM_PARAM_STRUCT pSphParamAnc;
    Mutex::Autolock _l(mMutex);
    GetANCSpeechParamFromNVRam(&pSphParamAnc);
    mLogEnable     = pSphParamAnc.ANC_log;
    mLogDownSample = pSphParamAnc.ANC_log_downsample;
    mApply         = pSphParamAnc.ANC_apply;

    SetCoefficients(pSphParamAnc.ANC_para);
#else
    mLogEnable     = false;
    mLogDownSample = false;
    mApply         = false;

#endif
}


void SpeechANCController::SetCoefficients(void *buf)
{
    ALOGD("%s(), SetCoefficients:%d", __FUNCTION__);
    ::ioctl(mFd, SET_ANC_PARAMETER, buf);
}

void SpeechANCController::SetApplyANC(bool apply)
{
    //if mmi selected, set flag and enable/disable anc
    ALOGD("%s(), SetApply:%d", __FUNCTION__, apply);

    if (apply ^ mApply)
    {
#ifdef param_anc_add
        AUDIO_ANC_CUSTOM_PARAM_STRUCT pSphParamAnc;
        Mutex::Autolock _l(mMutex);
        GetANCSpeechParamFromNVRam(&pSphParamAnc);
        pSphParamAnc.ANC_apply = apply;
        SetANCSpeechParamToNVRam(&pSphParamAnc);
#endif
        mApply = apply;

    }

}


bool SpeechANCController::GetApplyANC(void)
{
    //get compile option and return
    ALOGD("%s(), mApply:%d", __FUNCTION__, mApply);
    return mApply;
}

void SpeechANCController::SetEanbleANCLog(bool enable, bool downsample)
{
    ALOGD("%s(), enable:%d, downsample(%d)", __FUNCTION__, enable, downsample);
    if (enable ^ mLogEnable || mLogDownSample ^ downsample)
    {
#ifdef param_anc_add
        AUDIO_ANC_CUSTOM_PARAM_STRUCT pSphParamAnc;
        Mutex::Autolock _l(mMutex);
        GetANCSpeechParamFromNVRam(&pSphParamAnc);
        pSphParamAnc.ANC_log = enable;
        pSphParamAnc.ANC_log_downsample = downsample;
        SetANCSpeechParamToNVRam(&pSphParamAnc);
#endif
        mLogEnable = enable;
        mLogDownSample = downsample;
        if (enable)
        {
            ::ioctl(mFd, SET_ANC_CONTROL, ANCControlCmd_EnableLog);
        }
        else
        {
            ::ioctl(mFd, SET_ANC_CONTROL, ANCControlCmd_DisableLog);
        }
    }
}

bool SpeechANCController::GetEanbleANCLog(void)
{
    ALOGD("%s(), mLogEnable:%d", __FUNCTION__, mLogEnable);
    return mLogEnable;
}

bool SpeechANCController::GetEanbleANCLogDownSample(void)
{
    ALOGD("%s(), mLogDownSample:%d", __FUNCTION__, mLogDownSample);
    return mLogDownSample;
}

bool SpeechANCController::EanbleANC(bool enable)
{
    int ret;

    ALOGD("%s(), mEnabled(%d), enable(%d)", __FUNCTION__, mEnabled, enable);


    if (!mGroupANC)
    {
        ALOGD("%s(), EnableError, Not ANC group", __FUNCTION__);
        return false;
    }
    //only in call mode do enable/disable
    if (AudioALSAStreamManager::getInstance()->isModeInPhoneCall() == true)
    {
        if (enable ^ mEnabled)
        {
            Mutex::Autolock _l(mMutex);
            if (enable)
            {
                ret = ::ioctl(mFd, SET_ANC_CONTROL, ANCControlCmd_Enable);
            }
            else
            {
                ret = ::ioctl(mFd, SET_ANC_CONTROL, ANCControlCmd_Disable);
            }
            if (ret == -1)
            {
                ALOGD("%s(), EnableFail:%d", __FUNCTION__, ret);
                return false;
            }
            mEnabled = enable;
        }
    }
    return true;
}

bool SpeechANCController::CloseANC(void)
{
    int ret;
    ALOGD("%s()", __FUNCTION__);
    if (!mGroupANC)
    {
        ALOGD("%s(), CloseError, Not ANC group", __FUNCTION__);
        return false;
    }
    Mutex::Autolock _l(mMutex);
    ret = ::ioctl(mFd, SET_ANC_CONTROL, ANCControlCmd_Close);
    if (ret == -1)
    {
        ALOGD("%s(), EnableFail:%d", __FUNCTION__, ret);
        return false;
    }
    mEnabled = false;
    return true;
}

bool SpeechANCController::SwapANC(bool swap2anc)
{
    int ret;
    ALOGD("%s(), mGroupANC(%d), swap2anc(%d)", __FUNCTION__, mGroupANC, swap2anc);
    if (mGroupANC ^ swap2anc)
    {
        if (swap2anc)
        {
            ret = ::ioctl(mFd, SET_ANC_CONTROL, ANCControlCmd_SwapToANC);
        }
        else
        {
            ret = ::ioctl(mFd, SET_ANC_CONTROL, ANCControlCmd_SwapFromANC);
        }
        
#if 0 //tina test       
        mGroupANC = swap2anc;
#endif

        if (ret == -1)
        {
            ALOGD("%s(), SWAPFail:%d", __FUNCTION__, ret);
            return false;
        }
        mGroupANC = swap2anc;
    }
    return true;
}

#endif

}   //namespace android
