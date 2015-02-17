package com.mediatek.launcher.ext;

import junit.framework.TestSuite;
import android.app.Instrumentation;
import android.test.InstrumentationTestRunner;


public class Op01LauncherTestRunner extends InstrumentationTestRunner {
	
    @Override
    public TestSuite getTestSuite() {
        TestSuite suite = new TestSuite();
        suite.addTestSuite(Op01LauncherTest.class);
        return suite;
    }
    
}
