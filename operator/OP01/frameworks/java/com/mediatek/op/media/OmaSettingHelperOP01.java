package com.mediatek.op.media;

import com.mediatek.xlog.Xlog;

public class OmaSettingHelperOP01 extends DefaultOmaSettingHelper {
    private static final String TAG = "OmaSettingHelperOP01";
    private static final boolean LOG = true;
    private static final int DEFAULT_RTSP_BUFFER_SIZE = 6;//seconds

    /**
     * Whether oma is supported or not.
     * @return true enabled, otherwise false.
     */
    protected boolean isOMAEnabled() {
        if (LOG) Xlog.v(TAG, "isOMAEnabled: enabled=true.");
        return true;
    }

    /**
     * Gets RTSP default buffer size.
     * @return RTSP default buffer size.
     */
    protected int getRtspDefaultBufferSize() {
        return DEFAULT_RTSP_BUFFER_SIZE;
    }
}
