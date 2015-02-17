package com.mediatek.calendar.plugin;

import com.mediatek.calendar.ext.ILunarExtension;
import com.mediatek.calendar.plugin.TCLunarPlugin;
import com.mediatek.pluginmanager.PluginManager;

import android.test.InstrumentationTestCase;
import android.test.suitebuilder.annotation.SmallTest;

import java.util.Locale;

public class TCLunarPluginTest extends InstrumentationTestCase {

    TCLunarPlugin mTCPlugin;
    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mTCPlugin = (TCLunarPlugin)PluginManager.createPluginObject(
                getInstrumentation().getContext(), ILunarExtension.class.getName());
    }

    @SmallTest
    public void testTCLunarPluginInTC() {
        if (!Locale.TRADITIONAL_CHINESE.equals(Locale.getDefault())) {
            return;
        }

        assertTrue(mTCPlugin.canShowLunarCalendar());
        assertEquals("春節", mTCPlugin.getLunarFestival(1, 1));
        assertNull(mTCPlugin.getGregFestival(1, 1));
        assertEquals("驚蟄", mTCPlugin.getSolarTermNameByIndex(5));
        assertNull(mTCPlugin.getSolarTermNameByIndex(-1));
        assertEquals("端午節", mTCPlugin.getLunarFestival(5, 5));
        assertEquals("中秋節", mTCPlugin.getLunarFestival(8, 15));
        assertNull(mTCPlugin.getLunarFestival(3, 15));
    }

    @SmallTest
    public void testTCLunarPluginInSC() {
        if (!Locale.SIMPLIFIED_CHINESE.equals(Locale.getDefault())) {
            return;
        }

        assertFalse(mTCPlugin.canShowLunarCalendar());
        assertNull(mTCPlugin.getLunarFestival(1, 1));
        assertNull(mTCPlugin.getGregFestival(1, 1));
        assertNull(mTCPlugin.getSolarTermNameByIndex(5));
        assertNull(mTCPlugin.getSolarTermNameByIndex(-1));
    }

    @SmallTest
    public void testTCLunarPluginInOther() {
        if (Locale.SIMPLIFIED_CHINESE.equals(Locale.getDefault())
                || Locale.TRADITIONAL_CHINESE.equals(Locale.getDefault())) {
            return;
        }

        assertFalse(mTCPlugin.canShowLunarCalendar());
        assertNull(mTCPlugin.getGregFestival(1, 1));
        assertNull(mTCPlugin.getLunarFestival(1, 1));
        assertNull(mTCPlugin.getSolarTermNameByIndex(5));
        assertNull(mTCPlugin.getSolarTermNameByIndex(-1));
    }
}
