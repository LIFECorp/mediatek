#ifndef ANDROID_ALSA_AUDIO_FM_CONTROLLER_H
#define ANDROID_ALSA_AUDIO_FM_CONTROLLER_H

#include "AudioType.h"
#include "AudioLock.h"

#include <tinyalsa/asoundlib.h>


namespace android
{

// FMAudioPlayer.cpp also need this structure!!
typedef struct _AUDIO_DEVICE_CHANGE_CALLBACK_STRUCT
{
    void (*callback)(void *data);
} AUDIO_DEVICE_CHANGE_CALLBACK_STRUCT;


class AudioALSAHardwareResourceManager;
class AudioALSAVolumeController;


class AudioALSAFMController
{
    public:
        virtual ~AudioALSAFMController();

        static AudioALSAFMController *getInstance();

        virtual bool     getFmEnable();
        virtual status_t setFmEnable(const bool enable, const audio_devices_t output_device);

        virtual uint32_t getFmUplinkSamplingRate() const;
        virtual uint32_t getFmDownlinkSamplingRate() const;

        virtual status_t routing(const audio_devices_t pre_device, const audio_devices_t new_device);

        virtual status_t setFmVolume(const float fm_volume);

        virtual bool     getFmChipPowerInfo();
        virtual void     setFmDeviceCallback(const AUDIO_DEVICE_CHANGE_CALLBACK_STRUCT *callback_data);


    protected:
        AudioALSAFMController();

        virtual status_t setFmDirectConnection(const bool enable, const bool bforce);

        inline bool      checkFmNeedUseDirectConnectionMode(const audio_devices_t output_device);

        void (*mFmDeviceCallback)(void *data);
        virtual status_t doDeviceChangeCallback();

        AudioALSAHardwareResourceManager *mHardwareResourceManager;
        AudioALSAVolumeController        *mAudioALSAVolumeController;


        AudioLock mLock; // TODO(Harvey): could remove it later...

        bool mFmEnable;
        bool mIsFmDirectConnectionMode;

        float mFmVolume;

        struct pcm_config mConfig;
        struct pcm *mPcm;


    private:
        static AudioALSAFMController *mAudioALSAFMController; // singleton

};

} // end namespace android

#endif // end of ANDROID_ALSA_AUDIO_FM_CONTROLLER_H
