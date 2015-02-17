package com.mediatek.op01.tests;

import android.content.Context;
import android.content.Intent;
import android.net.http.AndroidHttpClient;
import android.provider.MediaStore;
import android.test.InstrumentationTestCase;

import java.io.IOException;
import com.mediatek.mms.op01.Op01HttpRequestRetryHandler;
import org.apache.http.protocol.HttpContext;

public class Op01HttpRequestRetryHandlerTest extends InstrumentationTestCase {
    private Context mContext;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mContext = this.getInstrumentation().getContext();
    }
    
    @Override    
    protected void tearDown() throws Exception {
        super.tearDown();
    }

    public void testNullContext() {
        Op01HttpRequestRetryHandler handler = new Op01HttpRequestRetryHandler(mContext, 1, false);
        try {
            IOException e = new IOException("test");
            handler.retryRequest(e, 1, null);
            assertTrue(false);
        } catch(IllegalArgumentException ex) {
            assertTrue(true);
        }
    }

    public void testNullException() {
        Op01HttpRequestRetryHandler handler = new Op01HttpRequestRetryHandler(mContext, 1, false);
        try {
            IOException e = new IOException("test");
            TestHttpContext context = new TestHttpContext();
            
            handler.retryRequest(null, 1, context);
            assertTrue(false);
        } catch(IllegalArgumentException ex) {
            assertTrue(true);
        }
    }

    public void testNoConnection() {
        Op01HttpRequestRetryHandler handler = new Op01HttpRequestRetryHandler(mContext, 1, false);
        try {
            IOException e = new IOException("test");
            TestHttpContext context = new TestHttpContext();
            
            boolean ret = handler.retryRequest(e, 1, context);
            assertTrue(ret == false);
        } catch(IllegalArgumentException ex) {
            assertTrue(false);
        }
    }
    
    class TestHttpContext implements HttpContext {
        TestHttpContext() {
        }
        
        public Object getAttribute(String id) {
            return null;
        }

        public void setAttribute(String id, Object obj) {
        }

        public Object removeAttribute(String id) {
            return null;
        }
    }
}

