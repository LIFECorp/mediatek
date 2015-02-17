#ifndef AUDIO_SPEECH_ENHANCE_INFO_H
#define AUDIO_SPEECH_ENHANCE_INFO_H

//#include "AudioUtility.h"
#include <utils/threads.h>
#include <utils/SortedVector.h>
#include <utils/KeyedVector.h>
#include <utils/TypeHelpers.h>
#include <utils/Vector.h>
#include <utils/String16.h>

#include "SpeechType.h"
#include "AudioType.h"
#include "AudioLock.h"

namespace android
{

// speech enhancement function dynamic mask
// This is the dynamic switch to decided the enhancment output.
enum voip_sph_enh_dynamic_mask_t
{
    VOIP_SPH_ENH_DYNAMIC_MASK_DMNR      = (1 << 0), // for receiver
    VOIP_SPH_ENH_DYNAMIC_MASK_VCE       = (1 << 1),
    VOIP_SPH_ENH_DYNAMIC_MASK_BWE       = (1 << 2),
    VOIP_SPH_ENH_DYNAMIC_MASK_LSPK_DMNR = (1 << 5), // for loud speaker
    VOIP_SPH_ENH_DYNAMIC_MASK_ALL       = 0xFFFFFFFF
};

typedef struct
{
    uint32_t dynamic_func; // DMNR,VCE,BWE,
} voip_sph_enh_mask_struct_t;

enum TOOL_TUNING_MODE
{
    TUNING_MODE_NONE            = 0,
    NORMAL_MODE_DMNR            = 1,
    HANDSFREE_MODE_DMNR           = 2
};


class AudioSpeechEnhanceInfo
{
    public:

        static AudioSpeechEnhanceInfo *getInstance();
        AudioSpeechEnhanceInfo();
        ~AudioSpeechEnhanceInfo();

        //BesRecord Preprocess +++
        void SetBesRecScene(int32_t BesRecScene);
        int32_t GetBesRecScene(void);
        void ResetBesRecScene(void);
        //BesRecord Preprocess ---

        //get the MMI switch info
        bool GetDynamicSpeechEnhancementMaskOnOff(const voip_sph_enh_dynamic_mask_t dynamic_mask_type);
        void UpdateDynamicSpeechEnhancementMask(const voip_sph_enh_mask_struct_t &mask);
        status_t SetDynamicVoIPSpeechEnhancementMask(const voip_sph_enh_dynamic_mask_t dynamic_mask_type, const bool new_flag_on);
        voip_sph_enh_mask_struct_t GetDynamicVoIPSpeechEnhancementMask() const { return mVoIPSpeechEnhancementMask; }
        //----------------Audio tunning +++ --------------------------------
        //----------------for AP DMNR tunning --------------------------------
        void SetAPDMNRTuningEnable(bool bEnable);
        bool IsAPDMNRTuningEnable(void);
        bool SetAPTuningMode(const TOOL_TUNING_MODE mode);
        int GetAPTuningMode(void);
        //----------------for HDRec tunning --------------------------------
        void SetBesRecTuningEnable(bool bEnable);
        bool IsBesRecTuningEnable(void);

        status_t SetBesRecVMFileName(const char *fileName);
        void GetBesRecVMFileName(char *VMFileName);
        //----------------Audio tunning --- --------------------------------


    private:

        /**
          * singleton pattern
          */
        static AudioSpeechEnhanceInfo *mAudioSpeechEnhanceInfo;


        /**
         * AudioSpeechEnhanceInfo lock
         */
        AudioLock mLock;

        //BesRecord Preprocess +++
        int32_t mBesRecScene;
        //BesRecord Preprocess ---

        //for BesRec tuning
        bool mBesRecTuningEnable;
        char mVMFileName[VM_FILE_NAME_LEN_MAX];
        //for AP DMNR tunning
        bool mAPDMNRTuningEnable;
        int mAPTuningMode;

        //Tina todo
        //        KeyedVector<AudioALSAStreamIn *, SPELayer *> mSPELayerVector; // vector to save current recording client
        voip_sph_enh_mask_struct_t mVoIPSpeechEnhancementMask;


};

}

#endif
