package com.mediatek.systemui.plugin;

import junit.framework.TestSuite;
import android.app.Instrumentation;
import android.test.InstrumentationTestRunner;


public class Op01StatusBarPluginTestRunner extends InstrumentationTestRunner {
	
    @Override
    public TestSuite getTestSuite() {
        TestSuite suite = new TestSuite();
        suite.addTestSuite(Op01StatusBarPluginTest.class);
        return suite;
    }
    
}
