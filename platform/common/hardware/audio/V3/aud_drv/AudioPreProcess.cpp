/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

/*
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sched.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <time.h>
#include <sys/time.h>

#include <utils/Log.h>
#include <utils/String8.h>
#include <cutils/properties.h>
#include "AudioPreProcess.h"



#define LOG_TAG "AudioPreProcess"

namespace android
{

AudioPreProcess::AudioPreProcess(const stream_attribute_t *streamIn_attribute) :
    mStreamInAttribute(streamIn_attribute)
{
    ALOGD("%s()+", __FUNCTION__);
    //Mutex::Autolock lock(mLock);
    AudioAutoTimeoutLock _l(mLock);

    num_preprocessors = 0;;
    need_echo_reference = false;

    proc_buf_in = NULL;
    proc_buf_out = NULL;
    proc_buf_size = 0;
    proc_buf_frames = 0;

    ref_buf = NULL;
    ref_buf_size = 0;
    ref_buf_frames = 0;

    echo_reference = NULL;

    for (int i = 0; i < MAX_PREPROCESSORS; i++)
        preprocessors[i] = {0};

    mInChn = 0;
    mInSampleRate = 16000;

    mAudioSpeechEnhanceInfoInstance = AudioSpeechEnhanceInfo::getInstance();
    if (!mAudioSpeechEnhanceInfoInstance)
    {
        ALOGE("%s() mAudioSpeechEnhanceInfoInstance get fail", __FUNCTION__);
    }

    ALOGD("%s()-", __FUNCTION__);
}


AudioPreProcess::~AudioPreProcess()
{
    ALOGD("%s()+", __FUNCTION__);
    MutexLock();
    //AudioAutoTimeoutLock _l(mLock);

    if (proc_buf_in)
    {
        free(proc_buf_in);
        proc_buf_in = NULL;
    }
    /*
            if(proc_buf_out)
                free(proc_buf_out);
    */
    if (ref_buf)
    {
        free(ref_buf);
        ref_buf = NULL;
    }

    MutexUnlock();
    if (echo_reference != NULL)
    {
        stop_echo_reference(echo_reference);
    }

    ALOGD("%s()-", __FUNCTION__);
}

status_t AudioPreProcess::CheckNativeEffect(void)
{
    if (mStreamInAttribute->NativePreprocess_Info.PreProcessEffect_Update == true)
    {
        ALOGD("%s()+ %d using", __FUNCTION__, num_preprocessors);
        bool bRemoveEffectMatch = false;
        bool bAddEffectMatch = false;
        //check if  effect need remove
        do
        {
            bRemoveEffectMatch = false;
            if (mStreamInAttribute->NativePreprocess_Info.PreProcessEffect_Count == 0)  //no effect is enabled
            {
                //remove all effect
                ALOGD("%s(), remove all effect %d", __FUNCTION__, num_preprocessors);
                for (int i = 0; i < num_preprocessors; i++)
                {
                    removeAudioEffect(preprocessors[i].effect_itfe);
                    bRemoveEffectMatch = true;
                    break;
                }
            }
            else
            {
                for (int i = 0; i < num_preprocessors; i++)
                {
                    for (int j = 0; j < mStreamInAttribute->NativePreprocess_Info.PreProcessEffect_Count; j++)
                    {
                        if (preprocessors[i].effect_itfe == mStreamInAttribute->NativePreprocess_Info.PreProcessEffect_Record[j])
                        {
                            //find match effect which still needed
                            break;
                        }
                        if (j == (mStreamInAttribute->NativePreprocess_Info.PreProcessEffect_Count - 1)) //no match effect find, need to remove this effect
                        {
                            //remove effect in i
                            ALOGD("%s(), find effect need remove", __FUNCTION__);
                            removeAudioEffect(preprocessors[i].effect_itfe);
                            bRemoveEffectMatch = true;
                        }
                    }
                    if (bRemoveEffectMatch)
                    {
                        break;
                    }
                }
            }
        }
        while (bRemoveEffectMatch);


        //add effect
        do
        {
            bAddEffectMatch = false;
            if (num_preprocessors == 0) //if there is no effect active, all new effect need added
            {
                for (int i = 0; i < mStreamInAttribute->NativePreprocess_Info.PreProcessEffect_Count; i++)
                {
                    //add effect in i
                    addAudioEffect(mStreamInAttribute->NativePreprocess_Info.PreProcessEffect_Record[i]);
                }
            }
            else
            {
                for (int i = 0; i < mStreamInAttribute->NativePreprocess_Info.PreProcessEffect_Count; i++)
                {
                    for (int j = 0; j < num_preprocessors; j++)
                    {
                        if (preprocessors[j].effect_itfe == mStreamInAttribute->NativePreprocess_Info.PreProcessEffect_Record[i])
                        {
                            //find match effect which already added
                            break;
                        }
                        if (j == (num_preprocessors - 1)) //no match effect find, need to add this effect
                        {
                            //add effect in
                            ALOGD("%s(), find effect need add", __FUNCTION__);
                            addAudioEffect(mStreamInAttribute->NativePreprocess_Info.PreProcessEffect_Record[i]);
                            bAddEffectMatch = true;
                        }
                    }
                    if (bAddEffectMatch)
                    {
                        break;
                    }
                }
            }
        }
        while (bAddEffectMatch);

        ALOGD("%s()-", __FUNCTION__);
    }
    return NO_ERROR;
}

status_t AudioPreProcess::addAudioEffect(effect_handle_t effect)
{
    ALOGD("%s()+ %p", __FUNCTION__, effect);

    AudioAutoTimeoutLock _l(mLock);

    status_t RetStatus = -EINVAL;
    effect_descriptor_t desc;
    int i;

    if (num_preprocessors >= MAX_PREPROCESSORS)
    {
        RetStatus = -ENOSYS;
        ALOGD("%s(), exceed the uplimit", __FUNCTION__);
        goto exit;
    }

    RetStatus = (*effect)->get_descriptor(effect, &desc);
    if (RetStatus != 0)
    {
        goto exit;
    }

    for (i = 0; i < num_preprocessors; i++)
    {
        if (preprocessors[i].effect_itfe == effect)
        {
            ALOGD("%s() already found %s at index %d", __FUNCTION__, desc.name, i);
            RetStatus = -ENOSYS;
            goto exit;
        }
    }

    preprocessors[num_preprocessors].effect_itfe = effect;

    /* add the supported channel of the effect in the channel_configs */
    //in_read_audio_effect_channel_configs(&preprocessors[num_preprocessors]);

    num_preprocessors++;

    ALOGD("%s(), effect type: %08x, effect name:%s", __FUNCTION__, desc.type.timeLow, desc.name);

    ALOGD("%s(), StreamInAttributeinfo num_channels=%d, audio_channel_mask=%x", __FUNCTION__, mStreamInAttribute->num_channels, mStreamInAttribute->audio_channel_mask);

exit:
    ALOGD("%s()-, RetStatus=%d", __FUNCTION__, RetStatus);
    return RetStatus;
}

status_t AudioPreProcess::removeAudioEffect(effect_handle_t effect)
{
    ALOGD("%s()+ %p", __FUNCTION__, effect);

    AudioAutoTimeoutLock _l(mLock);

    int i;
    status_t RetStatus = -EINVAL;
    effect_descriptor_t desc;

    if (num_preprocessors <= 0)
    {
        RetStatus = -ENOSYS;
        ALOGD("%s(), num_preprocessors wrong", __FUNCTION__);
        goto exit;
    }

    for (i = 0; i < num_preprocessors; i++)
    {
        if (RetStatus == 0)   /* status == 0 means an effect was removed from a previous slot */
        {
            preprocessors[i - 1].effect_itfe = preprocessors[i].effect_itfe;
            preprocessors[i - 1].channel_configs = preprocessors[i].channel_configs;
            preprocessors[i - 1].num_channel_configs = preprocessors[i].num_channel_configs;
            ALOGD("%s() moving fx from %d to %d", __FUNCTION__, i, i - 1);
            continue;
        }
        if (preprocessors[i].effect_itfe == effect)
        {
            ALOGD("%s() found fx at index %d", __FUNCTION__, i);
            //            free(preprocessors[i].channel_configs);
            RetStatus = 0;
        }
    }

    if (RetStatus != 0)
    {
        goto exit;
    }

    num_preprocessors--;
    /* if we remove one effect, at least the last preproc should be reset */
    preprocessors[num_preprocessors].num_channel_configs = 0;
    preprocessors[num_preprocessors].effect_itfe = NULL;
    preprocessors[num_preprocessors].channel_configs = NULL;


    RetStatus = (*effect)->get_descriptor(effect, &desc);
    if (RetStatus != 0)
    {
        goto exit;
    }

    ALOGD("%s(), effect type: %08x, effect name:%s", __FUNCTION__, desc.type.timeLow, desc.name);

exit:


    ALOGD("%s()-, RetStatus=%d", __FUNCTION__, RetStatus);
    return RetStatus;
}


uint32_t AudioPreProcess::NativePreprocess(void *buffer , uint32_t bytes)
{
    ALOGD("%s()+", __FUNCTION__);
    if (num_preprocessors == 0)
    {
        return bytes;
    }
    else
    {
        ssize_t frames_wr = 0;
        audio_buffer_t in_buf;
        audio_buffer_t out_buf;
        const uint8_t num_channel = mStreamInAttribute->num_channels;
        ssize_t frames = bytes / sizeof(int16_t) / num_channel;
        ssize_t needframes = frames + proc_buf_frames;
        int i;

        ALOGD("%s: %d bytes, %d frames, proc_buf_frames=%d, mAPPS->num_preprocessors=%d,num_channel=%d", __FUNCTION__, bytes, frames, proc_buf_frames, num_preprocessors, num_channel);
        proc_buf_out = (int16_t *)buffer;

        if ((proc_buf_size < (size_t)needframes) || (proc_buf_in == NULL))
        {
            proc_buf_size = (size_t)needframes;
            proc_buf_in = (int16_t *)realloc(proc_buf_in, proc_buf_size * num_channel * sizeof(int16_t));
            //mpPreProcessIn->proc_buf_out = (int16_t *)realloc(mpPreProcessIn->proc_buf_out, mpPreProcessIn->proc_buf_size*mChNum*sizeof(int16_t));

            if (proc_buf_in == NULL)
            {
                ALOGW("%s(), proc_buf_in realloc fail", __FUNCTION__);
                return bytes;
            }
            ALOGD("%s: proc_buf_in %p extended to %d bytes", __FUNCTION__, proc_buf_in, proc_buf_size * sizeof(int16_t)*num_channel);
        }

        memcpy(proc_buf_in + proc_buf_frames * num_channel, buffer, bytes);

        proc_buf_frames += frames;

        while (frames_wr < frames)
        {
            //AEC
#if 0
            if (mEcho_Reference != NULL)
            {
                push_echo_reference(proc_buf_frames);
            }
            else
            {
                //prevent start_echo_reference fail previously due to output not enable
                if (need_echo_reference)
                {
                    ALOGD("try start_echo_reference");
                    mEcho_Reference = start_echo_reference(
                                          AUDIO_FORMAT_PCM_16_BIT,
                                          num_channel,
                                          sampleRate());
                }
            }
#endif
            /* in_buf.frameCount and out_buf.frameCount indicate respectively
                  * the maximum number of frames to be consumed and produced by process() */
            in_buf.frameCount = proc_buf_frames;
            in_buf.s16 = proc_buf_in;
            out_buf.frameCount = frames - frames_wr;
            out_buf.s16 = (int16_t *)proc_buf_out + frames_wr * num_channel;

            /* FIXME: this works because of current pre processing library implementation that
                 * does the actual process only when the last enabled effect process is called.
                 * The generic solution is to have an output buffer for each effect and pass it as
                 * input to the next.
                 */

            for (i = 0; i < num_preprocessors; i++)
            {
                (*preprocessors[i].effect_itfe)->process(preprocessors[i].effect_itfe,
                                                         &in_buf,
                                                         &out_buf);
            }

            /* process() has updated the number of frames consumed and produced in
                  * in_buf.frameCount and out_buf.frameCount respectively
                 * move remaining frames to the beginning of in->proc_buf_in */
            proc_buf_frames -= in_buf.frameCount;

            if (proc_buf_frames)
            {
                memcpy(proc_buf_in,
                       proc_buf_in + in_buf.frameCount * num_channel,
                       proc_buf_frames * num_channel * sizeof(int16_t));
            }

            /* if not enough frames were passed to process(), read more and retry. */
            if (out_buf.frameCount == 0)
            {
                ALOGV("%s, No frames produced by preproc", __FUNCTION__);
                break;
            }

            if ((frames_wr + (ssize_t)out_buf.frameCount) <= frames)
            {
                frames_wr += out_buf.frameCount;
                ALOGV("%s, out_buf.frameCount=%d,frames_wr=%d", __FUNCTION__, out_buf.frameCount, frames_wr);
            }
            else
            {
                /* The effect does not comply to the API. In theory, we should never end up here! */
                ALOGE("%s, preprocessing produced too many frames: %d + %d  > %d !", __FUNCTION__,
                      (unsigned int)frames_wr, out_buf.frameCount, (unsigned int)frames);
                frames_wr = frames;
            }
        }
        //        ALOGD("frames_wr=%d, bytes=%d",frames_wr,frames_wr*num_channel*sizeof(int16_t));
        return frames_wr * num_channel * sizeof(int16_t);
    }
    ALOGD("%s()-", __FUNCTION__);
}

void AudioPreProcess::stop_echo_reference(struct echo_reference_itfe *reference)
{
    ALOGD("%s()+", __FUNCTION__);
    //Mutex::Autolock lock(mLock);
    AudioAutoTimeoutLock _l(mLock);

    /* stop reading from echo reference */
    if (echo_reference != NULL && echo_reference == reference)
    {
        echo_reference->read(reference, NULL);
        clear_echo_reference(reference);
    }
    ALOGD("%s()-", __FUNCTION__);
}

void AudioPreProcess::clear_echo_reference(struct echo_reference_itfe *reference)
{
    ALOGD("%s()+ %p", __FUNCTION__, reference);
    //if ((mHw->mStreamHandler->echo_reference!= NULL) && (mHw->mStreamHandler->echo_reference == reference) &&
    if ((reference == echo_reference) && (reference != NULL))
    {
        //        if (mAudioSpeechEnhanceInfoInstance->IsOutputRunning())
        remove_echo_reference(reference);

        release_echo_reference(reference);
        echo_reference = NULL;
        //mHw->mStreamHandler->echo_reference = NULL;
    }
    ALOGD("%s()-", __FUNCTION__);
}

void AudioPreProcess::remove_echo_reference(struct echo_reference_itfe *reference)
{
    ALOGD("%s()+ %p", __FUNCTION__, reference);
    //TODO:Sam
    //mAudioSpeechEnhanceInfoInstance->remove_echo_reference(reference);
    ALOGD("%s()-", __FUNCTION__);
}

struct echo_reference_itfe *AudioPreProcess::start_echo_reference(audio_format_t format,
                                                                  uint32_t channel_count, uint32_t sampling_rate)
{
    //Mutex::Autolock lock(mLock);
    ALOGD("%s()+ channel_count=%d,sampling_rate=%d,echo_reference=%p", __FUNCTION__, channel_count, sampling_rate, echo_reference);
    AudioAutoTimeoutLock _l(mLock);

    clear_echo_reference(echo_reference);
    /* echo reference is taken from the low latency output stream used
         * for voice use cases */
    //TODO:Sam
    //if (mAudioSpeechEnhanceInfoInstance->IsOutputRunning())
    {
#ifdef EXTCODEC_ECHO_REFERENCE_SUPPORT
        //use the same setting with MTK VoIP echo reference data format
        uint32_t wr_channel_count = 2;  //android limit in Echo_reference.c, must be 2!!
        uint32_t wr_sampling_rate = 16000;
#else
        //TODO:Sam
        uint32_t wr_channel_count = 2;
        uint32_t wr_sampling_rate = 44100;
        //uint32_t wr_channel_count = mAudioSpeechEnhanceInfoInstance->GetOutputChannelInfo();
        //uint32_t wr_sampling_rate = mAudioSpeechEnhanceInfoInstance->GetOutputSampleRateInfo();
#endif
        mInChn = channel_count;
        mInSampleRate = sampling_rate;
        ALOGD("start_echo_reference,wr_channel_count=%d,wr_sampling_rate=%d", wr_channel_count, wr_sampling_rate);
        ALOGD("%s(),wr_channel_count=%d,wr_sampling_rate=%d", __FUNCTION__, wr_channel_count, wr_sampling_rate);
        int status = create_echo_reference(AUDIO_FORMAT_PCM_16_BIT,
                                           channel_count,
                                           sampling_rate,
                                           AUDIO_FORMAT_PCM_16_BIT,
                                           wr_channel_count,
                                           wr_sampling_rate,
                                           &echo_reference);
        if (status == 0)
        {
            //mHw->mStreamHandler->echo_reference = echo_reference;
            add_echo_reference(echo_reference);
        }
        else
        {
            ALOGW("%s() fail", __FUNCTION__);
        }
    }
    ALOGD("%s()-", __FUNCTION__);
    return echo_reference;
}

void AudioPreProcess::add_echo_reference(struct echo_reference_itfe *reference)
{
    ALOGD("%s()+, reference=%p", __FUNCTION__, reference);
    //TODO:Sam
    //mAudioSpeechEnhanceInfoInstance->add_echo_reference(reference);
    ALOGD("%s()-", __FUNCTION__);
}

void AudioPreProcess::push_echo_reference(size_t frames)
{

    //Mutex::Autolock lock(mLock);
    AudioAutoTimeoutLock _l(mLock);
    /* read frames from echo reference buffer and update echo delay
     * in->ref_buf_frames is updated with frames available in in->ref_buf */
    int32_t delay_us = update_echo_reference(frames) / 1000;
    int i;
    audio_buffer_t buf;

    if (ref_buf_frames < frames)
    {
        frames = ref_buf_frames;
    }

    buf.frameCount = frames;
    buf.raw = ref_buf;

    for (i = 0; i < num_preprocessors; i++)
    {
        if ((*preprocessors[i].effect_itfe)->process_reverse == NULL)
        {
            continue;
        }

        (*preprocessors[i].effect_itfe)->process_reverse(preprocessors[i].effect_itfe,
                                                         &buf,
                                                         NULL);
        set_preprocessor_echo_delay(preprocessors[i].effect_itfe, delay_us);
    }

    ref_buf_frames -= buf.frameCount;
    if (ref_buf_frames)
    {
        //        ALOGV("push_echo_reference,ref_buf_frames=%d",ref_buf_frames);
        memcpy(ref_buf,
               ref_buf + buf.frameCount * mInChn,
               ref_buf_frames * mInChn * sizeof(int16_t));
    }
}

int32_t AudioPreProcess::update_echo_reference(size_t frames)
{

    struct echo_reference_buffer b;
    b.delay_ns = 0;

    //    ALOGD("update_echo_reference, frames = [%d], ref_buf_frames = [%d],  "
    //          "b.frame_count = [%d]", frames, ref_buf_frames, frames - ref_buf_frames);
    if (ref_buf_frames < frames)
    {
        if (ref_buf_size < frames)
        {
            ref_buf_size = frames;
            ref_buf = (int16_t *)realloc(ref_buf, frames * sizeof(int16_t) * mInChn);
            ALOG_ASSERT((ref_buf != NULL),
                        "update_echo_reference() failed to reallocate ref_buf");

            ALOGD("%s(), ref_buf %p extended to %d bytes", __FUNCTION__, ref_buf, frames * sizeof(int16_t)*mInChn);
        }
        b.frame_count = frames - ref_buf_frames;
        //fixme? raw address?
        b.raw = (void *)(ref_buf + ref_buf_frames * mInChn);

        get_capture_delay(frames, &b);

        if (echo_reference->read(echo_reference, &b) == 0)
        {
            ref_buf_frames += b.frame_count;
            /*            ALOGD("update_echo_reference(): ref_buf_frames:[%d], "
                                "ref_buf_size:[%d], frames:[%d], b.frame_count:[%d]",
                             ref_buf_frames, ref_buf_size, frames, b.frame_count);*/
        }
    }
    else
    {
        ALOGV("%s(): NOT enough frames to read ref buffer", __FUNCTION__);
    }
    return b.delay_ns;
}

int AudioPreProcess::set_preprocessor_echo_delay(effect_handle_t handle, int32_t delay_us)
{
    uint32_t buf[sizeof(effect_param_t) / sizeof(uint32_t) + 2];
    effect_param_t *param = (effect_param_t *)buf;

    param->psize = sizeof(uint32_t);
    param->vsize = sizeof(uint32_t);
    *(uint32_t *)param->data = AEC_PARAM_ECHO_DELAY;
    *((int32_t *)param->data + 1) = delay_us;

    return set_preprocessor_param(handle, param);
}

int AudioPreProcess::set_preprocessor_param(effect_handle_t handle, effect_param_t *param)
{
    uint32_t size = sizeof(int);
    uint32_t psize = ((param->psize - 1) / sizeof(int) + 1) * sizeof(int) +
                     param->vsize;

    int status = (*handle)->command(handle,
                                    EFFECT_CMD_SET_PARAM,
                                    sizeof(effect_param_t) + psize,
                                    param,
                                    &size,
                                    &param->status);
    if (status == 0)
    {
        status = param->status;
    }

    return status;
}

void AudioPreProcess::get_capture_delay(size_t frames, struct echo_reference_buffer *buffer)
{
    //FIXME:: calculate for more precise time delay
    struct timespec tstamp;
    long buf_delay = 0;
    long rsmp_delay = 0;
    long kernel_delay = 0;
    long delay_ns = 0;


    int rc = clock_gettime(CLOCK_MONOTONIC, &tstamp);
    if (rc != 0)
    {
        buffer->time_stamp.tv_sec  = 0;
        buffer->time_stamp.tv_nsec = 0;
        buffer->delay_ns           = 0;
        ALOGD("%s(): clock_gettime error", __FUNCTION__);
        return;
    }


    buf_delay = (long)(((int64_t)(proc_buf_frames) * 1000000000) / mInSampleRate);

    delay_ns = kernel_delay + buf_delay + rsmp_delay;

    buffer->time_stamp = tstamp;
    buffer->delay_ns   = delay_ns;
    //    ALOGV("get_capture_delay time_stamp = [%ld].[%ld], delay_ns: [%d]",
    //         buffer->time_stamp.tv_sec , buffer->time_stamp.tv_nsec, buffer->delay_ns);
}

bool AudioPreProcess::MutexLock(void)
{
    mLock.lock();
    return true;
}
bool AudioPreProcess::MutexUnlock(void)
{
    mLock.unlock();
    return true;
}


#define GET_COMMAND_STATUS(status, fct_status, cmd_status) \
    do {                                           \
        if (fct_status != 0)                       \
            status = fct_status;                   \
        else if (cmd_status != 0)                  \
            status = cmd_status;                   \
    } while(0)

int AudioPreProcess::in_configure_reverse(uint32_t channel_count, uint32_t sampling_rate)
{
    ALOGD("%s()+", __FUNCTION__);
    //Mutex::Autolock lock(mLock);
    AudioAutoTimeoutLock _l(mLock);

    int32_t cmd_status;
    uint32_t size = sizeof(int);
    effect_config_t config;
    int32_t status = 0;
    int32_t fct_status = 0;
    int i;

    //fixme:: currently AEC stereo channel has problem, force using mono while AEC enable
    if (num_preprocessors > 0)
    {
        config.inputCfg.channels = 1;//mChNum;channel_count
        config.outputCfg.channels = 1;//mHw->mStreamHandler->mOutput[0]->GetChannelInfo();
        config.inputCfg.format = AUDIO_FORMAT_PCM_16_BIT;
        config.outputCfg.format = AUDIO_FORMAT_PCM_16_BIT;
        config.inputCfg.samplingRate = sampling_rate;
        //TODO:Sam
        //config.outputCfg.samplingRate = mAudioSpeechEnhanceInfoInstance->GetOutputSampleRateInfo();
        config.outputCfg.samplingRate = 44100;

        config.inputCfg.mask =
            (EFFECT_CONFIG_SMP_RATE | EFFECT_CONFIG_CHANNELS | EFFECT_CONFIG_FORMAT);
        config.outputCfg.mask =
            (EFFECT_CONFIG_SMP_RATE | EFFECT_CONFIG_CHANNELS | EFFECT_CONFIG_FORMAT);

        for (i = 0; i < num_preprocessors; i++)
        {
            if ((*preprocessors[i].effect_itfe)->process_reverse == NULL)
            {
                continue;
            }
            fct_status = (*(preprocessors[i].effect_itfe))->command(
                             preprocessors[i].effect_itfe,
                             EFFECT_CMD_SET_CONFIG_REVERSE,
                             sizeof(effect_config_t),
                             &config,
                             &size,
                             &cmd_status);

            GET_COMMAND_STATUS(status, fct_status, cmd_status);
        }
    }
    ALOGD("%s()-, status=%d", __FUNCTION__, status);
    return status;
}

// ----------------------------------------------------------------------------
}

