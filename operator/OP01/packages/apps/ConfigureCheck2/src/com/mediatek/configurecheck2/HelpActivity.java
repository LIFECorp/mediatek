package com.mediatek.configurecheck2;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.webkit.WebView;

public class HelpActivity extends Activity {
    private static final String TAG = "HelpActivity";
    private WebView mWebView;
    
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        // TODO Auto-generated method stub
        CTSCLog.i(TAG, "onCreate()");
        super.onCreate(savedInstanceState);
        setContentView(R.layout.help);
        
        Intent intent = getIntent();
        String testTypeStr = intent.getStringExtra("TestType");
        
        mWebView = (WebView)findViewById(R.id.webview);
        if (testTypeStr.equals("CMCCSendTest")) {
            mWebView.loadUrl("file:///android_asset/SendTestHelp.html");
            
        } else if (testTypeStr.equals("ManSelfTest")){
            mWebView.loadUrl("file:///android_asset/SelfTestHelp.html");
            
        } else {
            throw new RuntimeException(TAG + "Assertion failed:" + "no such test type!");
        }
    }

}
