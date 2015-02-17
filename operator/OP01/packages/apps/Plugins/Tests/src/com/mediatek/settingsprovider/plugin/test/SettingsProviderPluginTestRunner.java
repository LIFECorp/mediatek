package com.mediatek.settingsprovider.plugin.test;

import android.test.InstrumentationTestRunner;

import junit.framework.TestSuite;

public class SettingsProviderPluginTestRunner extends InstrumentationTestRunner {

    @Override
    public TestSuite getTestSuite() {
        TestSuite suite = new TestSuite();
        suite.addTestSuite(SettingsProviderPluginTest.class);
        return suite;
    }
    
}
