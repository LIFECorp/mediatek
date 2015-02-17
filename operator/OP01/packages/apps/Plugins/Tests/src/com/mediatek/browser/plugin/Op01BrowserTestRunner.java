package com.mediatek.op01.tests;

import junit.framework.TestSuite;
import android.test.InstrumentationTestRunner;

public class Op01BrowserTestRunner extends InstrumentationTestRunner {
    
    @Override
    public TestSuite getTestSuite() {
        TestSuite suite = new TestSuite();
        suite.addTestSuite(Op01BrowserDownloadExTest.class);
        suite.addTestSuite(Op01BrowserProcessNetworkExTest.class);
        suite.addTestSuite(Op01BrowserSmallFeatureExTest.class);

        return suite;
    }
    
}