package com.mediatek.op01.tests;

import android.app.DownloadManager;
import android.app.DownloadManager.Request;
import android.content.Context;
import android.net.Uri;
import android.test.InstrumentationTestCase;
import android.widget.EditText;

import com.mediatek.browser.ext.IBrowserDownloadEx;
import com.mediatek.browser.plugin.Op01BrowserDownloadEx;
import com.mediatek.pluginmanager.Plugin;
import com.mediatek.pluginmanager.PluginManager;
import com.mediatek.xlog.Xlog;

public class Op01BrowserDownloadExTest extends InstrumentationTestCase
{
    private final String TAG = "Op01BrowserDownloadExTest";
    private static Op01BrowserDownloadEx mPlugin = null;
    private Context mContext;
    public static final String DEFAULT_DOWNLOAD_DIRECTORY_OP01 = "/storage/sdcard0/MyFavorite"; //BrowserSettings

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mContext = this.getInstrumentation().getContext();
        Object plugin = PluginManager.createPluginObject(mContext, "com.mediatek.browser.ext.IBrowserDownloadEx");
        if(plugin instanceof Op01BrowserDownloadEx){
            mPlugin = (Op01BrowserDownloadEx) plugin;
        }
    }
    
    @Override    
    protected void tearDown() throws Exception {
        super.tearDown();
        mPlugin = null;
    }


    public void test01_setRequestDestinationDir(){
        if(mPlugin != null){
            DownloadManager.Request mRequest = new DownloadManager.Request(Uri.parse(DEFAULT_DOWNLOAD_DIRECTORY_OP01));
            boolean mode = mPlugin.setRequestDestinationDir(DEFAULT_DOWNLOAD_DIRECTORY_OP01,
                    mRequest, "testFileName", "image/png");
            assertTrue(mode);
        }
    }
    
    public void test02_shouldShowDownloadOrOpenContent(){
        if(mPlugin != null){
            boolean mode = mPlugin.shouldShowDownloadOrOpenContent();
            assertTrue(mode);
        }
    }
    
    public void test03_shouldShowToastWithFileSize(){
        if(mPlugin != null){
            boolean mode = mPlugin.shouldShowToastWithFileSize();
            assertTrue(mode);
        }
    }
    
}