package com.mediatek.qsb.plugin.impl;

import android.content.Context;
import android.content.Intent;
import android.test.InstrumentationTestCase;

import com.mediatek.pluginmanager.PluginManager;
import com.mediatek.pluginmanager.Plugin;
import com.mediatek.qsb.plugin.OP01WebSearchHandler;

public class Op01QSBWebSearchHandlerTest extends InstrumentationTestCase {
    
    private static OP01WebSearchHandler mQSBWebSearchHandler = null;
    private Context context;

    private static final String SEARCH_URL_OP01 = "http://s.139.com/search.do?q={searchTerms}&amp;category=downloadable|web|browseable&amp;tid=2123,2124,2125,2126&amp;fr=portalcustom2";
    private static final String SEARCH_ENGINE_NAME = "s.139.com";

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        context = this.getInstrumentation().getContext();
        mQSBWebSearchHandler = (OP01WebSearchHandler)PluginManager.createPluginObject(context, "com.mediatek.qsb.ext.IWebSearchHandler");
    }
    
    @Override    
    protected void tearDown() throws Exception {
        super.tearDown();
	mQSBWebSearchHandler = null;
    }

    public void testHandleSearchInternal() {
        boolean isHandleSearchInternal = mQSBWebSearchHandler.handleSearchInternal(context,SEARCH_ENGINE_NAME,SEARCH_URL_OP01);
        assertTrue(isHandleSearchInternal);
    }

    public void testHandleSearchInternalNull() {
        boolean isHandleSearchInternalNull = mQSBWebSearchHandler.handleSearchInternal(context,null,SEARCH_URL_OP01);
        assertFalse(isHandleSearchInternalNull);
    }
}
