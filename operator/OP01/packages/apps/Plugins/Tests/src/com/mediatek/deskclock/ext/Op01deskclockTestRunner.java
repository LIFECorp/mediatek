package com.mediatek.deskclock.ext;

import junit.framework.TestSuite;
import android.app.Instrumentation;
import android.test.InstrumentationTestCase;
import android.test.InstrumentationTestRunner;


public class Op01deskclockTestRunner extends InstrumentationTestRunner {
	
    @Override
    public TestSuite getTestSuite() {
        TestSuite suite = new TestSuite();
        suite.addTestSuite(Op01deskclockRepeatRefTest.class);
        return suite;
    }
    
}
