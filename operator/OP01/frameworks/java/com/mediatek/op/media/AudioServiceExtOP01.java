package com.mediatek.op.media;

import com.mediatek.xlog.Xlog;
import com.mediatek.common.media.IAudioServiceExt;

public class AudioServiceExtOP01 implements IAudioServiceExt {
    private static final String TAG = "AudioServiceExtOP01";
    private static final boolean LOG = true;
    private static final boolean cameraSoundForce = true;

    /**
     * Gets cameraSoundForce default mode.
     * @return cameraSoundForce default mode.
     */
    public boolean isCameraSoundForced() {
        return cameraSoundForce;
    }
}
