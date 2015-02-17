package com.mediatek.browser.plugin;

import android.app.Activity;

import com.mediatek.browser.ext.BrowserSmsHandlerEx;
import com.mediatek.browser.ext.SmsHandler;
import com.mediatek.xlog.Xlog;

public class Op02BrowserSmsHandlerEx extends BrowserSmsHandlerEx {
    
    private static final String TAG = "BrowserPluginEx";
    
    @Override
    public SmsHandler createSmsHandler(Activity mActivity) {
        Xlog.i(TAG, "Enter: " + "createSmsHandler" + " --OP02 implement");
        return new SmsHandler(mActivity); 
    }
}