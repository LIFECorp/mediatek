package com.mediatek.op01.tests;

import android.content.Context;
import android.net.Uri;
import android.test.InstrumentationTestCase;
import android.widget.EditText;

import com.mediatek.browser.ext.IBrowserProcessNetworkEx;
import com.mediatek.browser.plugin.Op01BrowserProcessNetworkEx;
import com.mediatek.pluginmanager.Plugin;
import com.mediatek.pluginmanager.PluginManager;
import com.mediatek.xlog.Xlog;

public class Op01BrowserProcessNetworkExTest extends InstrumentationTestCase
{
    private final String TAG = "Op01BrowserProcessNetworkExTest";
    private static Op01BrowserProcessNetworkEx mPlugin = null;
    private Context mContext;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mContext = this.getInstrumentation().getContext();
        Object plugin = PluginManager.createPluginObject(mContext,
                "com.mediatek.browser.ext.IBrowserProcessNetworkEx");
        if(plugin instanceof Op01BrowserProcessNetworkEx){
            mPlugin = (Op01BrowserProcessNetworkEx) plugin;
        }
    }
    
    @Override    
    protected void tearDown() throws Exception {
        super.tearDown();
        mPlugin = null;
    }


    public void test01_shouldProcessNetworkNotify() {
        if(mPlugin != null){
            boolean mode = mPlugin.shouldProcessNetworkNotify(true);
            assertTrue(mode);
        }
    }

    
}