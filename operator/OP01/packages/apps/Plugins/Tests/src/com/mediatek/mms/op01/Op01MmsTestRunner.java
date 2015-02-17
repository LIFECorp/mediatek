package com.mediatek.op01.tests;

import junit.framework.TestSuite;
import android.test.InstrumentationTestRunner;

public class Op01MmsTestRunner extends InstrumentationTestRunner {
	
    @Override
    public TestSuite getTestSuite() {
        TestSuite suite = new TestSuite();
        suite.addTestSuite(Op01MmsComposeExtTest.class);
        suite.addTestSuite(Op01MmsConfigExtTest.class);
        suite.addTestSuite(Op01MmsSlideShowExtTest.class);
        suite.addTestSuite(Op01MmsConversationExtTest.class);
        suite.addTestSuite(Op01MmsTextSizeAdjustExtTest.class);
        //suite.addTestSuite(Op01MmsDialogNotifyExtTest.class);
        suite.addTestSuite(Op01MmsAttachmentEnhanceExtTest.class);
        suite.addTestSuite(Op01SmsReceiverExtTest.class);
        suite.addTestSuite(Op01MmsMultiForwardExtTest.class);
        suite.addTestSuite(Op01MmsTransactionExtTest.class);
        suite.addTestSuite(Op01HttpRequestRetryHandlerTest.class);
        suite.addTestSuite(Op01MmsSlideShowEditorExtTest.class);
        return suite;
    }
    
}