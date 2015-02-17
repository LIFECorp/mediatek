package com.mediatek.op01.tests;

import android.content.Context;
import android.content.Intent;
import android.net.http.AndroidHttpClient;
import android.provider.MediaStore;
import android.test.InstrumentationTestCase;

import com.mediatek.mms.op01.Op01MmsTransactionExt;
import com.mediatek.pluginmanager.PluginManager;

import org.apache.http.params.HttpConnectionParams;
import org.apache.http.params.HttpParams;

import android.app.Service;
import org.apache.http.impl.client.DefaultHttpRequestRetryHandler;
import android.os.IBinder;

import com.mediatek.common.featureoption.FeatureOption;

public class Op01MmsTransactionExtTest extends InstrumentationTestCase {
    private static Op01MmsTransactionExt sMmsTransactionPlugin = null;
    private Context mContext;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mContext = this.getInstrumentation().getContext();
        sMmsTransactionPlugin = (Op01MmsTransactionExt)PluginManager.createPluginObject(mContext, "com.mediatek.mms.ext.IMmsTransaction");
    }
    
    @Override    
    protected void tearDown() throws Exception {
        super.tearDown();
        sMmsTransactionPlugin = null;
    }

    public void testServerFailCount() {
        sMmsTransactionPlugin.setMmsServerStatusCode(400);
        int count = sMmsTransactionPlugin.getMmsServerFailCount();
        assertEquals(count, 1);
    }

    public void testUpdateConnection() {
        sMmsTransactionPlugin.setMmsServerStatusCode(400);
        sMmsTransactionPlugin.setMmsServerStatusCode(400);
        boolean ret = sMmsTransactionPlugin.updateConnection();
        assertTrue(ret == false);
    }

    public void testUpdateConnection1() {
        sMmsTransactionPlugin.setMmsServerStatusCode(400);
        sMmsTransactionPlugin.setMmsServerStatusCode(400);
        sMmsTransactionPlugin.setMmsServerStatusCode(400);
        boolean ret = sMmsTransactionPlugin.updateConnection();
        assertTrue(true);
    }

    public void testIsSyncStartPdpEnabled() {
        boolean ret = sMmsTransactionPlugin.isSyncStartPdpEnabled();
        assertTrue(ret);
    }

    public void testGetHttpRequestRetryHandler() {
        DefaultHttpRequestRetryHandler handler = sMmsTransactionPlugin.getHttpRequestRetryHandler();
        assertTrue(handler != null);
    }

    public void testForegroundNotification() {
        sMmsTransactionPlugin.startServiceForeground(null);
        sMmsTransactionPlugin.stopServiceForeground(null);
        assertTrue(true);
    }

    public void testIsRestartPendingsEnabled() {
        boolean ret = sMmsTransactionPlugin.isRestartPendingsEnabled();
        assertTrue(ret);
    }

    public void testSetSoSendTimeoutProperty() {
        sMmsTransactionPlugin.setSoSendTimeoutProperty();
        assertTrue(true);
    }

    public void testIsGminiMultiTransactionEnabled() {
        boolean ret = sMmsTransactionPlugin.isGminiMultiTransactionEnabled();
        if (FeatureOption.MTK_GEMINI_SUPPORT) {
            assertTrue(ret == true);
        } else {
            assertTrue(ret == false);
        }
    }
    
    class TestService extends Service {
        public IBinder onBind(Intent intent) {
            return null;
        }
    }
}



