package com.mediatek.cmcc.tests;

import android.test.InstrumentationTestRunner;

import junit.framework.TestSuite;

public class CustomerServiceTestRunner extends InstrumentationTestRunner {

    public TestSuite getAllTests() {
        TestSuite suite = new TestSuite();
        suite.addTestSuite(CustomerServiceTest.class);
       
        return suite;
    }
}
