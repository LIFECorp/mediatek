package com.mediatek.op.audioprofile;

import android.content.ContentResolver;
import android.content.Context;
import android.media.AudioManager;
import android.net.Uri;
import android.provider.Settings;
import android.util.Log;

import com.mediatek.audioprofile.AudioProfileManager;
import com.mediatek.audioprofile.AudioProfileService;
import com.mediatek.audioprofile.AudioProfileState;
import com.mediatek.audioprofile.AudioProfileManager.ProfileSettings;
import com.mediatek.audioprofile.AudioProfileManager.Scenario;
import com.mediatek.common.audioprofile.IAudioProfileService;

public class AudioProfileExtensionOP01 extends DefaultAudioProfileExtension {

    @Override
    public boolean shouldCheckDefaultProfiles() {
        return IS_SUPPORT_OUTDOOR_EDITABLE;
    }

    @Override
    public boolean shouldSyncGeneralRingtoneToOutdoor() {
        return false;
    }
}
