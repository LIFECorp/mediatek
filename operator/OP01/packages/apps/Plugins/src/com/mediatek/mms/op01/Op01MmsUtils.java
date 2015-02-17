package com.mediatek.mms.op01;

import android.content.Context;
import android.content.SharedPreferences;
import android.preference.PreferenceManager;

import com.mediatek.xlog.Xlog;


public class Op01MmsUtils {
    private static final String TAG = "Op01MmsUtils";
    private static final String TEXT_SIZE = "message_font_size";    
    private static final float DEFAULT_TEXT_SIZE = 18;
    private static final float MIN_TEXT_SIZE = 10;
    private static final float MAX_TEXT_SIZE = 32;
    
    
    public static float getTextSize(Context context) {
        SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(context);
        float size = sp.getFloat(TEXT_SIZE, DEFAULT_TEXT_SIZE);
        Xlog.v(TAG, "getTextSize = " + size);
        if (size < MIN_TEXT_SIZE) {
            size = MIN_TEXT_SIZE;
        } else if (size > MAX_TEXT_SIZE) {
            size = MAX_TEXT_SIZE;
        }
        return size;
    }
    
    public static void setTextSize(Context context, float size) {
        float textSize;
        
        Xlog.v(TAG, "setTextSize = " + size);

        if (size < MIN_TEXT_SIZE) {
            textSize = MIN_TEXT_SIZE;
        } else if (size > MAX_TEXT_SIZE) {
            textSize = MAX_TEXT_SIZE;
        } else {
            textSize = size;
        }
        SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(context);
        SharedPreferences.Editor editor = sp.edit();
        editor.putFloat(TEXT_SIZE, textSize);
        editor.commit();
    }

}
