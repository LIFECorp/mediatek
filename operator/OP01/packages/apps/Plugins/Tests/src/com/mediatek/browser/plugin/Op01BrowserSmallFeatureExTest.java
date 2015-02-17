package com.mediatek.op01.tests;

import android.app.DownloadManager;
import android.app.DownloadManager.Request;
import android.content.Context;
import android.net.Uri;
import android.test.InstrumentationTestCase;
import android.widget.EditText;

import com.mediatek.browser.ext.IBrowserSmallFeatureEx;
import com.mediatek.browser.plugin.Op01BrowserSmallFeatureEx;
import com.mediatek.pluginmanager.Plugin;
import com.mediatek.pluginmanager.PluginManager;
import com.mediatek.xlog.Xlog;

public class Op01BrowserSmallFeatureExTest extends InstrumentationTestCase
{
    private final String TAG = "Op01BrowserDownloadExTest";
    private static Op01BrowserSmallFeatureEx mPlugin = null;
    private Context mContext;
    
    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mContext = this.getInstrumentation().getContext();
        Object plugin = PluginManager.createPluginObject(mContext, "com.mediatek.browser.ext.IBrowserSmallFeatureEx");
        if(plugin instanceof Op01BrowserSmallFeatureEx) {
            mPlugin = (Op01BrowserSmallFeatureEx) plugin;
        }
    }

    @Override    
    protected void tearDown() throws Exception {
        super.tearDown();
        mPlugin = null;
    }

    public void test01_shouldDownloadPreference(){
        if(mPlugin != null){
            boolean mode = mPlugin.shouldDownloadPreference();
            assertTrue(mode);
        }
    }

    public void test02_shouldProcessResultForFileManager(){
        if(mPlugin != null){
            boolean mode = mPlugin.shouldProcessResultForFileManager();
            assertTrue(mode);
        }
    }

    public void test02_shouldCheckUrlLengthLimit(){
        if(mPlugin != null){
            boolean mode = mPlugin.shouldCheckUrlLengthLimit();
            assertTrue(mode);
        }
    }

    public void test03_shouldSetNavigationBarTitle(){
        if(mPlugin != null){
            boolean mode = mPlugin.shouldSetNavigationBarTitle();
            assertTrue(mode);
        }
    }

    public void test04_shouldTransferToWapBrowser(){
        if(mPlugin != null){
            boolean mode = mPlugin.shouldTransferToWapBrowser();
            assertTrue(mode);
        }
    }

    public void test05_shouldChangeBookmarkMenuManner(){
        if(mPlugin != null){
            boolean mode = mPlugin.shouldChangeBookmarkMenuManner();
            assertTrue(mode);
        }
    }

    public void test06_shouldLoadCustomerAdvancedXml(){
        if(mPlugin != null){
            boolean mode = mPlugin.shouldLoadCustomerAdvancedXml();
            assertTrue(mode);
        }
    }

    public void test07_shouldLoadCustomerAdvancedXml(){
        if(mPlugin != null){
            String ret = mPlugin.getOperatorUA("testString");
            assertEquals(ret, "Athens15_TD/V2 Linux/3.0.13 Android/4.0 Release/02.15.2012 "
                    + "Browser/AppleWebKit534.30 "
                    + "Mobile Safari/534.30 MBBMS/2.2 System/Android 4.0.1;");
        }
    }
}