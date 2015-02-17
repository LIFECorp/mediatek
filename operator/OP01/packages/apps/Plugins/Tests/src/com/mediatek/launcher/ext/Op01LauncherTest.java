package com.mediatek.launcher.ext;

import android.content.Context;
import android.content.Intent;
import android.provider.MediaStore;
import android.test.InstrumentationTestCase;

import com.mediatek.pluginmanager.PluginManager;
import com.mediatek.pluginmanager.Plugin;
import com.mediatek.launcher2.plugin.Op01AllAppsListExt;

public class Op01LauncherTest extends InstrumentationTestCase {
    
    private static Op01AllAppsListExt mAllAppsListEx = null;
    private Context context;

    @Override
    protected void setUp() throws Exception {
        super.setUp();

        context = this.getInstrumentation().getContext();
        mAllAppsListEx = (Op01AllAppsListExt)PluginManager.createPluginObject(context, "com.mediatek.launcher2.ext.IAllAppsListExt");
    }
    
    @Override    
    protected void tearDown() throws Exception {
        super.tearDown();
        mAllAppsListEx = null;
    }

    public void testIsNeedToRemoveWifiSettings() {

        boolean isShowWifiSettings = mAllAppsListEx.isShowWifiSettings();
        assertTrue(isShowWifiSettings);
    }

}
