package com.mediatek.op01.tests;

import junit.framework.TestSuite;
import android.test.InstrumentationTestRunner;

public class Op01DownloadManagerTestRunner extends InstrumentationTestRunner {
    
    @Override
    public TestSuite getTestSuite() {
        TestSuite suite = new TestSuite();
        suite.addTestSuite(Op01DownloadProviderFeatureExTest.class);
        return suite;
    }
    
}