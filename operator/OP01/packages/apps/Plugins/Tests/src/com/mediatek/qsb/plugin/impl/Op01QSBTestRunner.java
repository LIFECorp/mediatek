package com.mediatek.qsb.plugin.impl;

import junit.framework.TestSuite;
import android.app.Instrumentation;
import android.test.InstrumentationTestRunner;


public class Op01QSBTestRunner extends InstrumentationTestRunner {
	
    @Override
    public TestSuite getTestSuite() {
        TestSuite suite = new TestSuite();
        suite.addTestSuite(Op01QSBWebSearchHandlerTest.class);
        return suite;
    }
    
}
