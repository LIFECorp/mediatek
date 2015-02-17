package com.mediatek.op01.tests;

import android.content.Context;
import android.net.Uri;
import android.test.InstrumentationTestCase;
import android.widget.EditText;

import com.mediatek.downloadmanager.ext.IDownloadProviderFeatureEx;
import com.mediatek.downloadmanager.plugin.Op01DownloadProviderFeatureEx;
import com.mediatek.pluginmanager.Plugin;
import com.mediatek.pluginmanager.PluginManager;
import com.mediatek.xlog.Xlog;

public class Op01DownloadProviderFeatureExTest extends InstrumentationTestCase
{
    private final String TAG = "Op01DownloadProviderFeatureExTest";
    private static Op01DownloadProviderFeatureEx mPlugin = null;
    private Context mContext;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mContext = this.getInstrumentation().getContext();
        Object plugin = PluginManager.createPluginObject(mContext,
            "com.mediatek.downloadmanager.ext.IDownloadProviderFeatureEx");
        if(plugin instanceof Op01DownloadProviderFeatureEx){
            mPlugin = (Op01DownloadProviderFeatureEx) plugin;
        }
    }
    
    @Override    
    protected void tearDown() throws Exception {
        super.tearDown();
        mPlugin = null;
    }


    public void test01_shouldSetDownloadPath() {
        if(mPlugin != null){
            boolean mode = mPlugin.shouldSetDownloadPath();
            assertTrue(mode);
        }
    }

    
}