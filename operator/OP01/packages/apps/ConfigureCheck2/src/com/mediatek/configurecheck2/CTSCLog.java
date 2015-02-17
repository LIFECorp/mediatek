package com.mediatek.configurecheck2;

import android.util.Log;


public class CTSCLog {
    private static final String TAG_PREFIX = "[CTSC ";
    private static final String TAG_SUFFIX = "]";
    
    
    public static int v(String tag, String msg) {
        return Log.v(TAG_PREFIX + tag + TAG_SUFFIX, msg);
    }

    public static int d(String tag, String msg) {        
        return Log.d(TAG_PREFIX + tag + TAG_SUFFIX, msg);
    }
    public static int i(String tag, String msg) {
        return Log.i(TAG_PREFIX + tag + TAG_SUFFIX, msg);
    }
    public static int w(String tag, String msg) {
        return Log.w(TAG_PREFIX + tag + TAG_SUFFIX, msg);
    }
    public static int e(String tag, String msg) {
        return Log.e(TAG_PREFIX + tag + TAG_SUFFIX, msg);
    }
}
