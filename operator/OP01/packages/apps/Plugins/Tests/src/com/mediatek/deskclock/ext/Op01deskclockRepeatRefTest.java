package com.mediatek.deskclock.ext;

import android.content.Context;
import android.content.Intent;
import android.provider.MediaStore;
import android.test.InstrumentationTestCase;

import com.mediatek.deskclock.plugin.Op01RepeatPreferenceExtension;
import com.mediatek.pluginmanager.PluginManager;
import com.mediatek.pluginmanager.Plugin;


public class Op01deskclockRepeatRefTest extends InstrumentationTestCase {
    
    private static Op01RepeatPreferenceExtension mDeskclockRepeatRef = null;
    private Context context;

    @Override
    protected void setUp() throws Exception {
        super.setUp();

        context = this.getInstrumentation().getContext();
        mDeskclockRepeatRef = (Op01RepeatPreferenceExtension)PluginManager.createPluginObject(context, "com.mediatek.deskclock.ext.IRepeatPreferenceExtension");
    }
    
    @Override    
    protected void tearDown() throws Exception {
        super.tearDown();
	mDeskclockRepeatRef = null;
    }

    public void testIsShouldUseMTKRepeatPref() {
        boolean isShouldUseMTKRepeatPref = mDeskclockRepeatRef.shouldUseMTKRepeatPref();
        assertTrue(isShouldUseMTKRepeatPref);
    }

}
