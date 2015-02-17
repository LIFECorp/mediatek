#ifndef _SPEECH_ANC_CONTROLLER_H_
#define _SPEECH_ANC_CONTROLLER_H_


#include <utils/Log.h>
#include <stdint.h>
#include <sys/types.h>
#include <utils/threads.h>
#include <utils/String8.h>

#include <hardware_legacy/AudioHardwareBase.h>
#include <hardware_legacy/AudioSystemLegacy.h>
#include <media/AudioSystem.h>
#include <utils/threads.h>
#include <utils/KeyedVector.h>
#include <utils/Vector.h>

//#define MTK_ACTIVE_NOISE_CANCELLATION_SUPPORT
/*****************************************************************************
*                E X T E R N A L   R E F E R E N C E S
******************************************************************************
*/
#include <linux/ioctl.h>

namespace android
{
/*****************************************************************************
*                         I O C T R L  M E S S A G E S
******************************************************************************
*/

static char const *const kANCDeviceName = "/dev/anc";


enum ANC_Control_Cmd
{
    ANCControlCmd_Init = 0,
    ANCControlCmd_SwapToANC,
    ANCControlCmd_SwapFromANC,
    ANCControlCmd_Enable,
    ANCControlCmd_Disable,
    ANCControlCmd_EnableLog,
    ANCControlCmd_DisableLog,
    ANCControlCmd_Close,
    ANCControlCmd_GetDumpAddr
#if ANC_UT
    ,
    ANCControlCmd_GetStatus,
    ANCControlCmd_GetLogStatus,
    ANCControlCmd_GetCurGroup
#endif
};

//below is control message
#define AUD_DRV_ANC_IOC_MAGIC 'C'
//ANC Control
#define SET_ANC_CONTROL          _IOW(AUD_DRV_ANC_IOC_MAGIC, 0x1, int)
#define SET_ANC_PARAMETER        _IOW(AUD_DRV_ANC_IOC_MAGIC, 0x2, int)
#define GET_ANC_PARAMETER        _IOW(AUD_DRV_ANC_IOC_MAGIC, 0x3, int)

/*****************************************************************************
*                         F U N C T I O N S
******************************************************************************
*/

class SpeechANCController
{
    public:
        static SpeechANCController *getInstance();
        bool GetANCSupport(void);
#if defined(MTK_ACTIVE_NOISE_CANCELLATION_SUPPORT)
        void Init();
        void SetApplyANC(bool apply);
        bool CloseANC(void);
        bool GetApplyANC(void);
        void SetEanbleANCLog(bool enable, bool downsample);
        bool GetEanbleANCLog(void);
        bool EanbleANC(bool enable);
        void SetCoefficients(void *buf);
        bool SwapANC(bool swap2anc);
        bool GetEanbleANCLogDownSample(void);
#endif

    private:
        SpeechANCController();
        ~SpeechANCController();

        static SpeechANCController *UniqueSpeechANCController;
        Mutex   mMutex;
        int     mFd;
        bool mEnabled;
        bool mLogEnable;
        bool mLogDownSample;
        bool mApply;
        bool mGroupANC;
};   //SpeechANCControl

}   //namespace android

#endif   //_SPEECH_ANC_CONTROL_H_
