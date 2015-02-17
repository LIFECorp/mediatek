/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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
package com.mediatek.settings.plugin;

import android.app.Fragment;
import android.content.Context;
import android.content.ContextWrapper;
import android.content.Intent;
import android.media.AudioManager;
import android.media.RingtoneManager;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.ImageView;
import android.widget.RadioButton;
import android.widget.TextView;

import com.mediatek.settings.ext.DefaultAudioProfileExt;
import com.mediatek.audioprofile.AudioProfileManager;
import com.mediatek.op01.plugin.R;

public class AudioProfileExt extends DefaultAudioProfileExt {
    private static final String TAG = "AudioProfileExt";

    Fragment mFragment;
    LayoutInflater mInflater;
    Context mContext;

    private TextView mTextView = null;
    private RadioButton mCheckboxButton = null;
    private ImageView mImageView = null;

    public AudioProfileExt(Context context) {
        super(context);
        mContext = getBaseContext();
        mInflater = (LayoutInflater) getBaseContext().getSystemService(
                Context.LAYOUT_INFLATER_SERVICE);
    }

    public boolean isPrefEditable() {
        return true;
    }

    public View createView(int defaultLayoutId) {
        View view = mInflater.inflate(R.layout.audio_profile_item_cmcc, null);
        mCheckboxButton = (RadioButton) view.findViewById(R.id.radiobutton);
        mTextView = (TextView) view.findViewById(R.id.profiles_text);
        return view;
    }

    public View getPreferenceTitle(int defaultTitleId) {
        return mTextView;
    }

    public View getPreferenceSummary(int defaultSummaryId) {
        return null;
    }

    public View getPrefRadioButton(int defaultRBId) {
        return mCheckboxButton;
    }

    public View getPrefImageView(int defaultImageViewId) {
        return mImageView;
    }
    
    public void setRingtonePickerParams(Intent intent) {
        intent.putExtra(RingtoneManager.EXTRA_RINGTONE_SHOW_MORE_RINGTONES,
                true);
    }

    public void setRingerVolume(AudioManager audiomanager, int volume) {
        audiomanager.setStreamVolume(AudioProfileManager.STREAM_RING, volume, 0);
        audiomanager.setStreamVolume(AudioProfileManager.STREAM_NOTIFICATION, volume, 0);
    }

    public void setVolume(AudioManager audiomanager, int streamType, int volume) {
        audiomanager.setStreamVolume(streamType, volume, 0);
    }
}
