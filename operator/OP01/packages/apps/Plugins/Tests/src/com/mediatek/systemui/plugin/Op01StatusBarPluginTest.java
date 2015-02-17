package com.mediatek.systemui.plugin;

import android.content.Context;
import android.content.Intent;
import android.test.InstrumentationTestCase;

import com.mediatek.pluginmanager.PluginManager;
import com.mediatek.pluginmanager.Plugin;
import com.mediatek.systemui.plugin.Op01StatusBarPlugin;

import android.content.ContextWrapper;
import android.content.res.Resources;

import com.mediatek.op01.plugin.R;

import com.mediatek.systemui.ext.DataType;
import com.mediatek.systemui.ext.IStatusBarPlugin;
import com.mediatek.systemui.ext.NetworkType;

public class Op01StatusBarPluginTest extends InstrumentationTestCase {
    
    private static Op01StatusBarPlugin mStatusBarPlugin = null;
    private Context context;
    static final int[] DATA_ACTIVITY = {
        R.drawable.stat_sys_signal_not_inout,
        R.drawable.stat_sys_signal_in,
        R.drawable.stat_sys_signal_out,
        R.drawable.stat_sys_signal_inout
    };

    @Override
    protected void setUp() throws Exception {
        super.setUp();

        context = this.getInstrumentation().getContext();
        mStatusBarPlugin = (Op01StatusBarPlugin)PluginManager.createPluginObject(context, "com.mediatek.systemui.ext.IStatusBarPlugin");
    }
    
    @Override    
    protected void tearDown() throws Exception {
        super.tearDown();
	mStatusBarPlugin = null;
    }

    public void testgetSignalIndicatorIconGemini() {
 	int nSignalIndicatorIconGemini = mStatusBarPlugin.getSignalIndicatorIconGemini(0);
        assertEquals(-1,nSignalIndicatorIconGemini);
    }

    public void testgetDataTypeIconListGemini() {
 	int[] ngetDataTypeIconListGemini = mStatusBarPlugin.getDataTypeIconListGemini(true,DataType.Type_G);
        assertEquals(null,ngetDataTypeIconListGemini);
    }

    public void testisHspaDataDistinguishable() {
        boolean isisHspaDataDistinguishable = mStatusBarPlugin.isHspaDataDistinguishable();
        assertTrue(isisHspaDataDistinguishable);
    }

    public void testgetDataNetworkTypeIconGemini() {
 	int nDataNetworkTypeIconGemini = mStatusBarPlugin.getDataNetworkTypeIconGemini(null,0);
        assertEquals(-1,nDataNetworkTypeIconGemini);
    }

    public void testgetDataActivityIconList() {
 	int[] nDataActivityIconList = mStatusBarPlugin.getDataActivityIconList(0,false);
        assertEquals(DATA_ACTIVITY.length,nDataActivityIconList.length);
    }

    public void testsupportDataTypeAlwaysDisplayWhileOn() {
        boolean issupportDataTypeAlwaysDisplayWhileOn = mStatusBarPlugin.supportDataTypeAlwaysDisplayWhileOn();
        assertTrue(issupportDataTypeAlwaysDisplayWhileOn);
    }

    public void testsupportDisableWifiAtAirplaneMode() {
        boolean issupportDisableWifiAtAirplaneMode = mStatusBarPlugin.supportDisableWifiAtAirplaneMode();
        assertTrue(issupportDisableWifiAtAirplaneMode);
    }

    public void testget3gDisabledWarningString() {
 	String s3gDisabledWarningString = mStatusBarPlugin.get3gDisabledWarningString();
        assertEquals(null,s3gDisabledWarningString);
    }
}
