package com.mediatek.systemui.plugin;

import android.content.Context;
import android.content.Intent;
import android.test.InstrumentationTestCase;

import com.mediatek.pluginmanager.PluginManager;
import com.mediatek.pluginmanager.Plugin;
import com.mediatek.systemui.plugin.Op02StatusBarPlugin;

import android.content.ContextWrapper;
import android.content.res.Resources;

import com.mediatek.op02.plugin.R;

import com.mediatek.systemui.ext.DataType;
import com.mediatek.systemui.ext.IStatusBarPlugin;
import com.mediatek.systemui.ext.NetworkType;

public class Op02StatusBarPluginTest extends InstrumentationTestCase {
    
    private static Op02StatusBarPlugin mStatusBarPlugin = null;
    private Context context;

    @Override
    protected void setUp() throws Exception {
        super.setUp();

        context = this.getInstrumentation().getContext();
        mStatusBarPlugin = (Op02StatusBarPlugin)PluginManager.createPluginObject(context, "com.mediatek.systemui.ext.IStatusBarPlugin");
    }
    
    @Override    
    protected void tearDown() throws Exception {
        super.tearDown();
        mStatusBarPlugin = null;
    }

    public void testgetSignalStrengthNullIconGemini() {
        int nSignalStrengthNullIconGemini = mStatusBarPlugin.getSignalStrengthNullIconGemini(0);
        assertEquals(R.drawable.stat_sys_gemini_signal_null_sim1, nSignalStrengthNullIconGemini);
        
        nSignalStrengthNullIconGemini = mStatusBarPlugin.getSignalStrengthNullIconGemini(1);
        assertEquals(R.drawable.stat_sys_gemini_signal_null_sim2, nSignalStrengthNullIconGemini);
    }
    
    public void testgetSignalIndicatorIconGemini() {
        int nSignalIndicatorIconGemini = mStatusBarPlugin.getSignalIndicatorIconGemini(0);
        assertEquals(R.drawable.stat_sys_gemini_signal_indicator_sim1, nSignalIndicatorIconGemini);
        
        nSignalIndicatorIconGemini = mStatusBarPlugin.getSignalIndicatorIconGemini(1);
        assertEquals(R.drawable.stat_sys_gemini_signal_indicator_sim2, nSignalIndicatorIconGemini);
    }

    public void testgetDataTypeIconListGemini() {
        int[] ngetDataTypeIconListGemini = mStatusBarPlugin.getDataTypeIconListGemini(false, DataType.Type_G);
        assertEquals(null, ngetDataTypeIconListGemini);
        
        ngetDataTypeIconListGemini = mStatusBarPlugin.getDataTypeIconListGemini(true, DataType.Type_G);
        assertTrue(null != ngetDataTypeIconListGemini);
    }

    public void testgetDataNetworkTypeIconGemini() {
        int nDataNetworkTypeIconGemini = mStatusBarPlugin.getDataNetworkTypeIconGemini(NetworkType.Type_1X3G, -1);
        assertEquals(-1, nDataNetworkTypeIconGemini);
        
        nDataNetworkTypeIconGemini = mStatusBarPlugin.getDataNetworkTypeIconGemini(NetworkType.Type_1X3G, 1);
        assertTrue(-1 == nDataNetworkTypeIconGemini);
        
        nDataNetworkTypeIconGemini = mStatusBarPlugin.getDataNetworkTypeIconGemini(NetworkType.Type_3G, 1);
        assertTrue(-1 != nDataNetworkTypeIconGemini);
    }

    public void testget3gDisabledWarningString() {
        String s3gDisabledWarningString = mStatusBarPlugin.get3gDisabledWarningString();
        assertTrue(null != s3gDisabledWarningString);
    }
}
