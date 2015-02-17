#include "AudioFtmBase.h"

#include <utils/threads.h>
#include <utils/Log.h>

#include "AudioAssert.h"

#include "AudioFtm.h"

#define LOG_TAG "AudioFtmBase"

namespace android
{

AudioFtmBase *AudioFtmBase::createAudioFtmInstance()
{
    static Mutex mGetInstanceLock;
    Mutex::Autolock _l(mGetInstanceLock);

    return AudioFtm::getInstance();
}

AudioFtmBase::AudioFtmBase()
{
    ALOGD("%s()", __FUNCTION__);
}

AudioFtmBase::~AudioFtmBase()
{
    ALOGD("%s()", __FUNCTION__);
}


/// Codec
void AudioFtmBase::Audio_Set_Speaker_Vol(int level)
{
    ALOGW("%s()", __FUNCTION__);
}
void AudioFtmBase::Audio_Set_Speaker_On(int Channel)
{
    ALOGW("%s()", __FUNCTION__);
}
void AudioFtmBase::Audio_Set_Speaker_Off(int Channel)
{
    ALOGW("%s()", __FUNCTION__);
}
void AudioFtmBase::Audio_Set_HeadPhone_On(int Channel)
{
    ALOGW("%s()", __FUNCTION__);
}
void AudioFtmBase::Audio_Set_HeadPhone_Off(int Channel)
{
    ALOGW("%s()", __FUNCTION__);
}
void AudioFtmBase::Audio_Set_Earpiece_On()
{
    ALOGW("%s()", __FUNCTION__);
}
void AudioFtmBase::Audio_Set_Earpiece_Off()
{
    ALOGW("%s()", __FUNCTION__);
}


/// for factory mode & Meta mode (Analog part)
void AudioFtmBase::FTM_AnaLpk_on(void)
{
    ALOGW("%s()", __FUNCTION__);
}

void AudioFtmBase::FTM_AnaLpk_off(void)
{
    ALOGW("%s()", __FUNCTION__);
}


/// Output device test
int AudioFtmBase::RecieverTest(char receiver_test)
{
    ALOGW("%s()", __FUNCTION__);
    return true;
}

int AudioFtmBase::LouderSPKTest(char left_channel, char right_channel)
{
    ALOGW("%s()", __FUNCTION__);
    return true;
}
int AudioFtmBase::EarphoneTest(char bEnable)
{
    ALOGW("%s()", __FUNCTION__);
    return true;
}
int AudioFtmBase::EarphoneTestLR(char bLR)
{
    ALOGW("%s()", __FUNCTION__);
    return true;
}


/// Speaker over current test
int AudioFtmBase::Audio_READ_SPK_OC_STA(void)
{
    ALOGW("%s()", __FUNCTION__);
    return true;
}
int AudioFtmBase::LouderSPKOCTest(char left_channel, char right_channel)
{
    ALOGW("%s()", __FUNCTION__);
    return true;
}


/// Loopback
int AudioFtmBase::PhoneMic_Receiver_Loopback(char echoflag)
{
    ALOGW("%s()", __FUNCTION__);
    return true;
}
int AudioFtmBase::PhoneMic_EarphoneLR_Loopback(char echoflag)
{
    ALOGW("%s()", __FUNCTION__);
    return true;
}
int AudioFtmBase::PhoneMic_SpkLR_Loopback(char echoflag)
{
    ALOGW("%s()", __FUNCTION__);
    return true;
}
int AudioFtmBase::HeadsetMic_EarphoneLR_Loopback(char bEnable, char bHeadsetMic)
{
    ALOGW("%s()", __FUNCTION__);
    return true;
}
int AudioFtmBase::HeadsetMic_SpkLR_Loopback(char echoflag)
{
    ALOGW("%s()", __FUNCTION__);
    return true;
}

int AudioFtmBase::PhoneMic_Receiver_Acoustic_Loopback(int Acoustic_Type, int *Acoustic_Status_Flag, int bHeadset_Output)
{
    ALOGW("%s()", __FUNCTION__);
    return true;
}


/// FM / mATV
int AudioFtmBase::FMLoopbackTest(char bEnable)
{
    ALOGW("%s()", __FUNCTION__);
    return true;
}

int AudioFtmBase::Audio_FM_I2S_Play(char bEnable)
{
    ALOGW("%s()", __FUNCTION__);
    return true;
}
int AudioFtmBase::Audio_MATV_I2S_Play(int enable_flag)
{
    ALOGW("%s()", __FUNCTION__);
    return true;
}
int AudioFtmBase::Audio_FMTX_Play(bool Enable, unsigned int Freq)
{
    ALOGW("%s()", __FUNCTION__);
    return true;
}

int AudioFtmBase::ATV_AudPlay_On(void)
{
    ALOGW("%s()", __FUNCTION__);
    return true;
}
int AudioFtmBase::ATV_AudPlay_Off(void)
{
    ALOGW("%s()", __FUNCTION__);
    return true;
}
unsigned int AudioFtmBase::ATV_AudioWrite(void *buffer, unsigned int bytes)
{
    ALOGW("%s()", __FUNCTION__);
    return true;
}


/// HDMI
int AudioFtmBase::Audio_HDMI_Play(bool Enable, unsigned int Freq)
{
    ALOGW("%s()", __FUNCTION__);
    return 0;
}

int AudioFtmBase::HDMI_SineGenPlayback(bool bEnable, int dSamplingRate) 
{
    ALOGE("%s() is not supported!!", __FUNCTION__);
    return false;
}


/// Vibration Speaker
int AudioFtmBase::SetVibSpkCalibrationParam(void *cali_param)
{
    ALOGW("%s()", __FUNCTION__);
    return 0;
}

uint32_t AudioFtmBase::GetVibSpkCalibrationStatus()
{
    ALOGW("%s()", __FUNCTION__);
    return 0;
}

void AudioFtmBase::SetVibSpkEnable(bool enable, uint32_t freq)
{
    ALOGW("%s()", __FUNCTION__);
}

void AudioFtmBase::SetVibSpkRampControl(uint8_t rampcontrol)
{
    ALOGW("%s()", __FUNCTION__);
}

bool AudioFtmBase::ReadAuxadcData(int channel, int *value)
{
    ALOGW("%s()", __FUNCTION__);
    return false;
}


} // end of namespace android
