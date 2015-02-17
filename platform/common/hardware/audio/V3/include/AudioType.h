#ifndef __AUDIO_TYPE_H__
#define __AUDIO_TYPE_H__

#include <stdint.h>
#include <sys/types.h>

#include <utils/Errors.h>
#include <utils/Vector.h>
#include <utils/String16.h>
#include <utils/String8.h>
#include <hardware_legacy/AudioSystemLegacy.h>
#include <hardware/audio_effect.h>

#include "AudioAssert.h"

using android::status_t;

#define VM_FILE_NAME_LEN_MAX 128
#define MAX_PREPROCESSORS 3 /* maximum one AGC + one NS + one AEC per input stream */

// when call I2S start , need parameters for I2STYPE
typedef enum
{
    MATV,                         //I2S Input For ATV
    FMRX,                         //I2S Input For FMRX
    FMRX_32K,                 //I2S Input For FMRX_32K
    FMRX_48K,                 //I2S Input For FMRX_48K
    I2S0OUTPUT,             //   I2S0 output
    I2S1OUTPUT,             //   I2S1 output
    HOA_SAMPLERATE,   //   use for HQA support
    NUM_OF_I2S
} I2STYPE;

#define AUDIO_LOCK_TIMEOUT_VALUE_MS (5000)  //The same with ANR

#if 1 //HP switch
//#define HIFIDAC_SWITCH
//#define SWITCH_BEFORE_HPAMP
//#define HIFI_SWITCH_BY_AUDENH

//#define EXTDAC_PMIC_MUTE
//#define RINGTONE_USE_PMIC
#endif

// TODO(Harvey): move it to somewhere else
/**
 * Playback handler types
 */
enum playback_handler_t
{
    PLAYBACK_HANDLER_BASE = -1,
    PLAYBACK_HANDLER_NORMAL,
    PLAYBACK_HANDLER_VOICE,
    PLAYBACK_HANDLER_FM,
    PLAYBACK_HANDLER_MATV,
};

/**
 * Capture handler types
 */
enum capture_handler_t
{
    CAPTURE_HANDLER_BASE = -1,
    CAPTURE_HANDLER_NORMAL,
    CAPTURE_HANDLER_VOICE,
    CAPTURE_HANDLER_FM_RADIO,
    CAPTURE_HANDLER_SPK_FEED,
};


/**
 * Capture Data Provider types
 */
enum capture_provider_t
{
    CAPTURE_PROVIDER_BASE = -1,
    CAPTURE_PROVIDER_NORMAL,
    CAPTURE_PROVIDER_VOICE,
    CAPTURE_PROVIDER_FM_RADIO,
    CAPTURE_PROVIDER_SPK_FEED,
    CAPTURE_PROVIDER_MAX
};

typedef struct
{
    bool    besrecord_enable;
    int32_t besrecord_scene;
    //for besrecord tuning
    bool    besrecord_tuningEnable;
    char    besrecord_VMFileName[VM_FILE_NAME_LEN_MAX];
    //for AP DMNR tunning
    bool    besrecord_dmnr_tuningEnable;
    int32_t besrecord_dmnr_tuningMode;
} besrecord_info_struct_t;

typedef struct
{
    bool    PreProcessEffect_Update;
    int PreProcessEffect_Count;
    effect_handle_t PreProcessEffect_Record[MAX_PREPROCESSORS];

} native_preprocess_info_struct_t;


struct stream_attribute_t
{
    audio_format_t       audio_format;
    audio_channel_mask_t audio_channel_mask;
    audio_output_flags_t audio_output_flags;

    audio_devices_t      output_devices;

    audio_devices_t      input_device;
    audio_source_t       input_source;

    uint32_t             num_channels;
    uint32_t             sample_rate;

    uint32_t             buffer_size;
    uint32_t             latency;
    uint32_t             interrupt_samples;

    audio_in_acoustics_t acoustics_mask;

    bool                 digital_mic_flag; //or use mIsDigitalMIC??

    audio_mode_t         audio_mode;

    uint32_t             mStreamOutIndex;  // AudioALSAStreamOut pass to AudioALSAStreamManager

    besrecord_info_struct_t BesRecord_Info;

    native_preprocess_info_struct_t NativePreprocess_Info;
};


enum sgen_mode_t
{
    SGEN_MODE_I00_I01           = 0,
    SGEN_MODE_I02               = 1,
    SGEN_MODE_I03_I04           = 2,
    SGEN_MODE_I05_I06           = 3,
    SGEN_MODE_I07_I08           = 4,
    SGEN_MODE_I09               = 5,
    SGEN_MODE_I10_I11           = 6,
    SGEN_MODE_I12_I13           = 7,
    SGEN_MODE_I14               = 8,
    SGEN_MODE_I15_I16           = 9,
    SGEN_MODE_I17_I18           = 10,
    SGEN_MODE_I19_I20           = 11,
    SGEN_MODE_I21_I22           = 12,
    SGEN_MODE_O00_O01           = 13,
    SGEN_MODE_O02               = 14,
    SGEN_MODE_O03_O04           = 15,
    SGEN_MODE_O05_O06           = 16,
    SGEN_MODE_O07_O08           = 17,
    SGEN_MODE_O09_O10           = 18,
    SGEN_MODE_O11               = 19,
    SGEN_MODE_O12               = 20,
    SGEN_MODE_O13_O14           = 21,
    SGEN_MODE_O15_O16           = 22,
    SGEN_MODE_O17_O18           = 23,
    SGEN_MODE_O19_O20           = 24,
    SGEN_MODE_O21_O22           = 25,
    SGEN_MODE_O23_O24           = 26, // TODO: O25 ??
    SGEN_MODE_DISABLE           = 27
};


#endif
