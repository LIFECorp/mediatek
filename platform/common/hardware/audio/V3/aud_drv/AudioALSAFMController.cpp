#include "AudioALSAFMController.h"

#include <linux/ioctl.h>
#include <cutils/properties.h>

#include "WCNChipController.h"

#include "AudioLock.h"

#include "AudioALSAHardwareResourceManager.h"
#include "AudioALSAVolumeController.h"


#define LOG_TAG "AudioALSAFMController"

namespace android
{

/*==============================================================================
 *                     Property keys
 *============================================================================*/

const char PROPERTY_KEY_FM_FORCE_DIRECT_MODE_TYPE[PROPERTY_KEY_MAX]  = "af.fm.force_direct_mode_type";

/*==============================================================================
 *                     Const Value
 *============================================================================*/

/*==============================================================================
 *                     Enumerator
 *============================================================================*/

enum fm_force_direct_mode_t
{
    FM_FORCE_NONE           = 0x0,
    FM_FORCE_DIRECT_MODE    = 0x1,
    FM_FORCE_INDIRECT_MODE  = 0x2,
};

/*==============================================================================
 *                     Singleton Pattern
 *============================================================================*/

AudioALSAFMController *AudioALSAFMController::mAudioALSAFMController = NULL;

AudioALSAFMController *AudioALSAFMController::getInstance()
{
    static AudioLock mGetInstanceLock;
    AudioAutoTimeoutLock _l(mGetInstanceLock);

    if (mAudioALSAFMController == NULL)
    {
        mAudioALSAFMController = new AudioALSAFMController();
    }
    ASSERT(mAudioALSAFMController != NULL);
    return mAudioALSAFMController;
}

/*==============================================================================
 *                     Constructor / Destructor / Init / Deinit
 *============================================================================*/

AudioALSAFMController::AudioALSAFMController() :
    mFmDeviceCallback(NULL),
    mHardwareResourceManager(AudioALSAHardwareResourceManager::getInstance()),
    mAudioALSAVolumeController(AudioALSAVolumeController::getInstance()),
    mFmEnable(false),
    mIsFmDirectConnectionMode(true),
    mFmVolume(-1.0), // valid volume value: 0.0 ~ 1.0
    mPcm(NULL)
{
    ALOGD("%s()", __FUNCTION__);

    memset(&mConfig, 0, sizeof(mConfig));
    mConfig.channels = 2;
    mConfig.rate = 44100;
    mConfig.period_size = 2048;
    mConfig.period_count = 8;
    mConfig.format = PCM_FORMAT_S16_LE;
    mConfig.start_threshold = 0;
    mConfig.stop_threshold = 0;
    mConfig.silence_threshold = 0;
}

AudioALSAFMController::~AudioALSAFMController()
{
    ALOGD("%s()", __FUNCTION__);
}

/*==============================================================================
 *                     FM Control
 *============================================================================*/

bool AudioALSAFMController::checkFmNeedUseDirectConnectionMode(const audio_devices_t output_device)
{
    // FM force direct/indirect mode
    char property_value[PROPERTY_VALUE_MAX];
    property_get(PROPERTY_KEY_FM_FORCE_DIRECT_MODE_TYPE, property_value, "0"); //"0": default not force any mode
    fm_force_direct_mode_t fm_force_direct_mode = (fm_force_direct_mode_t)atoi(property_value);

    bool retval = false;
    switch (fm_force_direct_mode)
    {
        case FM_FORCE_NONE:   // Decided by whether earphone is inserted or not
        {
            retval = (output_device == AUDIO_DEVICE_OUT_WIRED_HEADSET ||
                      output_device == AUDIO_DEVICE_OUT_WIRED_HEADPHONE) ? true : false; // only earphone use direct mode, else, ex: earphone + wify, use indirect mode
            ALOGD("%s(), FM_FORCE_NONE, retval = %d", __FUNCTION__, retval);
            break;
        }
        case FM_FORCE_DIRECT_MODE:   // Force to direct mode
        {
            ALOGW("%s(), FM_FORCE_DIRECT_MODE", __FUNCTION__);
            retval = true;
            break;
        }
        case FM_FORCE_INDIRECT_MODE:   // Force to indirect mode
        {
            ALOGW("%s(), FM_FORCE_INDIRECT_MODE", __FUNCTION__);
            retval = false;
            break;
        }
        default:
        {
            WARNING("No such fm_force_direct_mode!!");
        }
    }

    return retval;
}


bool AudioALSAFMController::getFmEnable()
{
    AudioAutoTimeoutLock _l(mLock);
    ALOGV("%s(), mFmEnable = %d", __FUNCTION__, mFmEnable);
    return mFmEnable;
}

status_t AudioALSAFMController::setFmEnable(const bool enable, const audio_devices_t output_device)
{
    // Lock to Protect HW Registers & AudioMode
    // TODO(Harvey): get stream manager lock here?

    AudioAutoTimeoutLock _l(mLock);

    ALOGD("+%s(), mFmEnable = %d => enable = %d", __FUNCTION__, mFmEnable, enable);

    // Check Current Status
    if (enable == mFmEnable)
    {
        ALOGW("-%s(), enable == mFmEnable, return.", __FUNCTION__);
        return INVALID_OPERATION;
    }

    // Update Enable Status
    mFmEnable = enable;

    // get current device
    ALOGD("%s(), output_device = 0x%x", __FUNCTION__, output_device);

    if (mFmEnable == true) // Open
    {
        // Set FM chip initialization: Config GPIO


        // Set FM source module enable


        // Set Audio Digital/Analog HW Register
        if (checkFmNeedUseDirectConnectionMode(output_device) == true)
        {
            setFmDirectConnection(true, true);
            mHardwareResourceManager->startOutputDevice(output_device);
        }
        else
        {
            mIsFmDirectConnectionMode = false;
        }

        // Set Direct/Indirect Mode to FMAudioPlayer
        doDeviceChangeCallback();
    }
    else // Close
    {
        // Disable Audio Digital/Analog HW Register
        if (mIsFmDirectConnectionMode == true)
        {
            mHardwareResourceManager->stopOutputDevice();
        }
        setFmDirectConnection(false, true);

        // Set FM source module disable

    }

    ALOGD("-%s()", __FUNCTION__);
    return NO_ERROR;
}

status_t AudioALSAFMController::routing(const audio_devices_t pre_device, const audio_devices_t new_device)
{
    AudioAutoTimeoutLock _l(mLock);

    ASSERT(mFmEnable == true);

    ALOGD("+%s(), pre_device = 0x%x, new_device = 0x%x", __FUNCTION__, pre_device, new_device);

#if 0
    const uint32_t kAudioDeviceSpeakerAndHeadset   = AUDIO_DEVICE_OUT_SPEAKER | AUDIO_DEVICE_OUT_WIRED_HEADSET;
    const uint32_t kAudioDeviceSpeakerAndHeadphone = AUDIO_DEVICE_OUT_SPEAKER | AUDIO_DEVICE_OUT_WIRED_HEADPHONE;
    if (new_device == pre_device)
    {
        ALOGE("-%s(), pre_device = 0x%x, new_device = 0x%x", __FUNCTION__, pre_device, new_device);
        return INVALID_OPERATION;
    }
    else if (new_device == kAudioDeviceSpeakerAndHeadset || new_device == kAudioDeviceSpeakerAndHeadphone)
    {
        ALOGD("%s(), entering Warning Tone, only config analog part", __FUNCTION__);

        ALOGD("-%s()", __FUNCTION__);
        return NO_ERROR;
    }
    else if (pre_device == kAudioDeviceSpeakerAndHeadset || pre_device == kAudioDeviceSpeakerAndHeadphone)
    {
        ALOGD("%s(), leaving Warning Tone, only config analog part", __FUNCTION__);

        ALOGD("-%s()", __FUNCTION__);
        return NO_ERROR;
    }
#endif

    // Close
    if (mIsFmDirectConnectionMode == true) // Direct mode, close it directly
    {
        mHardwareResourceManager->stopOutputDevice();
    }

    // Open
    setFmDirectConnection(checkFmNeedUseDirectConnectionMode(new_device), false);

    // Set Direct/Indirect Mode for FM Chip
    doDeviceChangeCallback();

    // Enable PMIC Analog Part
    if (mIsFmDirectConnectionMode == true) // Direct mode, open it directly
    {
        mHardwareResourceManager->startOutputDevice(new_device);
    }

    ALOGD("-%s()", __FUNCTION__);
    return NO_ERROR;
}

void AudioALSAFMController::setFmDeviceCallback(const AUDIO_DEVICE_CHANGE_CALLBACK_STRUCT *callback_data)
{
    if (callback_data == NULL)
    {
        mFmDeviceCallback = NULL;
    }
    else
    {
        mFmDeviceCallback = callback_data->callback;
        ASSERT(mFmDeviceCallback != NULL);
    }
}

status_t AudioALSAFMController::doDeviceChangeCallback()
{
    ALOGD("+%s(), mIsFmDirectConnectionMode = %d, callback = %p", __FUNCTION__, mIsFmDirectConnectionMode, mFmDeviceCallback);

    ASSERT(mFmEnable == true);

    if (mFmDeviceCallback == NULL) // factory mode might not set mFmDeviceCallback
    {
        ALOGE("-%s(), mFmDeviceCallback == NULL", __FUNCTION__);
        return NO_INIT;
    }


    if (mIsFmDirectConnectionMode == true)
    {
        mFmDeviceCallback((void *)false); // Direct Mode, No need to create in/out stream
        ALOGD("-%s(), mFmDeviceCallback(false)", __FUNCTION__);
    }
    else
    {
        mFmDeviceCallback((void *)true);  // Indirect Mode, Need to create in/out stream
        ALOGD("-%s(), mFmDeviceCallback(true)", __FUNCTION__);
    }

    return NO_ERROR;
}

/*==============================================================================
 *                     Audio HW Control
 *============================================================================*/

uint32_t AudioALSAFMController::getFmUplinkSamplingRate() const
{
    return 44100; // TODO(Harvey): query it
}

uint32_t AudioALSAFMController::getFmDownlinkSamplingRate() const
{
    return 44100; // TODO(Harvey): query it
}

status_t AudioALSAFMController::setFmDirectConnection(const bool enable, const bool bforce)
{
    ALOGD("+%s(), enable = %d, bforce = %d", __FUNCTION__, enable, bforce);

    // Check Current Status
    if (mIsFmDirectConnectionMode == enable && bforce == false)
    {
        ALOGW("-%s(), enable = %d, bforce = %d", __FUNCTION__, enable, bforce);
        return INVALID_OPERATION;
    }

    // Apply
    if (bforce == true) // TODO(Harvey, Chipeng): workaround, remove it later
    {
        if (enable == true)
        {
            if (mPcm == NULL)
            {
                mPcm = pcm_open(0, 6 , PCM_OUT, &mConfig);
                ALOGD("%s(), pcm_open mPcm = %p", __FUNCTION__, mPcm);
            }
            if (mPcm == NULL || pcm_is_ready(mPcm) == false)
            {
                ALOGE("%s(), Unable to open mPcm device %u (%s)", __FUNCTION__, 6 , pcm_get_error(mPcm));
            }
            pcm_start(mPcm);
        }
        else
        {
            if (mPcm != NULL)
            {
                pcm_close(mPcm);
                mPcm = NULL;
            }
        }
    }
    else // TODO(Harvey, Chipeng): workaround, remove it later
    {
        if (mIsFmDirectConnectionMode == true)
        {
            setFmVolume(0);
        }
    }

    // Update Direct Mode Status
    mIsFmDirectConnectionMode = enable;

    // Update (HW_GAIN2) Volume for Direct Mode Only
    if (mIsFmDirectConnectionMode == true)
    {
        setFmVolume(mFmVolume);
    }


    ALOGD("-%s(), enable = %d, bforce = %d", __FUNCTION__, enable, bforce);
    return NO_ERROR;
}

status_t AudioALSAFMController::setFmVolume(const float fm_volume)
{
    ALOGD("+%s(), mFmVolume = %f => fm_volume = %f", __FUNCTION__, mFmVolume, fm_volume);

    const float kMaxFmVolume = 1.0;
    ASSERT(0 <= fm_volume && fm_volume <= kMaxFmVolume); // valid volume value: 0.0 ~ 1.0

    mFmVolume = fm_volume;

    // Set HW Gain for Direct Mode // TODO(Harvey): FM Volume
    if (mFmEnable == true && mIsFmDirectConnectionMode == true)
    {
        mAudioALSAVolumeController->setFmVolume(mFmVolume);
    }
    else
    {
        ALOGD("%s(), Do nothing. mFMEnable = %d, mIsFmDirectConnectionMode = %d", __FUNCTION__, mFmEnable, mIsFmDirectConnectionMode);
    }

    ALOGD("-%s(), mFmVolume = %f", __FUNCTION__, mFmVolume);
    return NO_ERROR;
}

/*==============================================================================
 *                     WCN FM Chip Control
 *============================================================================*/

bool AudioALSAFMController::getFmChipPowerInfo()
{
    return WCNChipController::GetInstance()->GetFmChipPowerInfo();
}

} // end of namespace android
