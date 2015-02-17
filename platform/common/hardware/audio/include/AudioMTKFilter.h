#ifndef AUDIO_MTK_FILTER_PROCESS_H
#define AUDIO_MTK_FILTER_PROCESS_H

#include "AudioCompensationFilter.h"
#include "MtkAudioLoud.h"
#include "MtkAudioBitConverter.h"
//#if defined(MTK_VIBSPK_SUPPORT)
#include "AudioCompFltCustParam.h"
#include "AudioVIBSPKControl.h"
//#endif

#ifndef uint32_t
typedef unsigned int        uint32;
#endif


namespace android
{

class AudioMTKFilter
{
    public:
        AudioMTKFilter(AudioCompFltType_t type,
                       AudioComFltMode_t mode,
                       uint32_t sampleRate,
                       uint32_t channel,
                       uint32_t format,
                       size_t bufferSize);
        ~AudioMTKFilter();
        void start();
        void stop();
        void pause();
        void resume();
        bool isStart();
        bool isActive();
        void setParameter(void *param);
        uint32_t  process(void *inBuffer, uint32_t bytes, void *outBuffer, uint32_t outBytes=0);
        void setParameter2Sub(void *param);
    private:
        AudioMTKFilter(const AudioMTKFilter &);
        AudioMTKFilter &operator=(const AudioMTKFilter &);
        status_t init();
        AudioCompFltType_t      mType;
        AudioComFltMode_t       mMode;
        uint32_t                mSampleTate;
        uint32_t                mChannel;
        uint32_t                mFormat;
        size_t                  mBufferSize;
        MtkAudioLoud *mFilter;
        bool                    mStart;
        bool                    mActive;
        mutable Mutex           mLock;
};

class AudioMTKFilter_VibSpkAddTone
{
    public:
        AudioMTKFilter_VibSpkAddTone(uint32_t sampleRate,
                              uint32_t channel,
                              uint32_t format,
                              size_t bufferSize);
        ~AudioMTKFilter_VibSpkAddTone();    
        size_t  DoVibSignal2DLProcess(void *outbuffer, void *src2DLbuffer, size_t bytes);
    private:    
        MtkAudioBitConverter *mShifter_to_1_31_VIBSPK;        
        AudioVIBSPKControl *mVibSpk;
        uint32_t mVibSpkFreq;
        bool  mVibSpkEnable;        
        uint8_t *mVIBSPKToneBuffer;//always 16bit
        uint8_t *mVIBSPKToneWorkBuffer;    
        uint32_t mChannels;
        

};

class AudioMTKFilterManager
{
    public:
        AudioMTKFilterManager(uint32_t sampleRate,
                              uint32_t channel,
                              uint32_t format,
                              size_t bufferSize);
        ~AudioMTKFilterManager();
        void start();
        void stop();
        bool isParamFixed();
        void setDevice(uint32_t devices);
        void setParamFixed(bool flag);
        void setParameter(uint32_t type, void *param);
        bool isFilterStart(uint32_t type);
        uint32_t  process(void *inBuffer, uint32_t bytes, void *outBuffer, uint32_t outBytes=0);
    private:
        AudioMTKFilterManager(const AudioMTKFilterManager &);
        AudioMTKFilterManager &operator=(const AudioMTKFilterManager &);
        bool init();
        uint32_t        mSamplerate;
        uint32_t        mChannel;
        uint32_t        mFormat;
        size_t          mBufferSize;
        bool            mFixedParam;
        AudioMTKFilter *mSpeakerFilter;
        AudioMTKFilter *mHeadphoneFilter;
        AudioMTKFilter *mEnhanceFilter;
//#if defined(MTK_VIBSPK_SUPPORT)        
        AudioMTKFilter *mVIBSPKFilter;
        AudioMTKFilter_VibSpkAddTone* mVibspkAddToneFilter;
//#endif
        uint8_t        *mBuffer;
        uint32_t        mDevices;
};

}

#endif

