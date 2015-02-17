package com.mediatek.soundrecorder.plugin;

import com.mediatek.soundrecorder.ext.DefaultQualityLevel;

public class Op01QualityLevel extends DefaultQualityLevel {

    private static final int VOICE_QUALITY_LEVEL_NUMBER = 3;

    @Override
    public int getLevelNumber() {
        return VOICE_QUALITY_LEVEL_NUMBER;
    }
}
