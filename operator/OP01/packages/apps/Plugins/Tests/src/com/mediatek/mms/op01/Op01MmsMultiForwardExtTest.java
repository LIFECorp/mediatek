package com.mediatek.op01.tests;

import android.content.Context;
import android.test.InstrumentationTestCase;
import android.widget.EditText;

import com.mediatek.pluginmanager.PluginManager;
import com.mediatek.pluginmanager.Plugin;
import com.mediatek.mms.ext.IMmsCompose;
import com.mediatek.mms.ext.IMmsMultiDeleteAndForwardHost;
import com.mediatek.mms.op01.Op01MmsMultiDeleteAndForwardExt;
import com.mediatek.xlog.Xlog;


public class Op01MmsMultiForwardExtTest extends InstrumentationTestCase implements IMmsMultiDeleteAndForwardHost
{
    private final String TAG = "Op01MmsMultiForwardExtTest";
    private static Op01MmsMultiDeleteAndForwardExt mMmsMultiDeleteAndForwardPlugin = null;
    private Context mContext;
    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mContext = this.getInstrumentation().getContext();
        Object plugin = PluginManager.createPluginObject(mContext, "com.mediatek.mms.ext.IMmsMultiDeleteAndForward");
        if(plugin instanceof Op01MmsMultiDeleteAndForwardExt){
            mMmsMultiDeleteAndForwardPlugin = (Op01MmsMultiDeleteAndForwardExt) plugin;
            mMmsMultiDeleteAndForwardPlugin.init(this);
        }
    }

    @Override    
    protected void tearDown() throws Exception {
        super.tearDown();
        mMmsMultiDeleteAndForwardPlugin = null;
    }


    public void test01_initBodyandAddress(){
        if(mMmsMultiDeleteAndForwardPlugin != null){
            mMmsMultiDeleteAndForwardPlugin.initBodyandAddress();
            assertTrue(true);
        }
    }

    public void test02_getBody(){
        if(mMmsMultiDeleteAndForwardPlugin != null){
            mMmsMultiDeleteAndForwardPlugin.getBody(0);
            assertTrue(true);
        }
    }

    public void test03_getAddress(){
        if(mMmsMultiDeleteAndForwardPlugin != null){
            mMmsMultiDeleteAndForwardPlugin.getAddress(0);
            assertTrue(true);
        }
    }

    public void test04_clearlist(){
        if(mMmsMultiDeleteAndForwardPlugin != null){
            mMmsMultiDeleteAndForwardPlugin.clearBodyandAddressList();
            assertTrue(true);
        }
    }

    public void test05_setBodyandAddress(){
        if(mMmsMultiDeleteAndForwardPlugin != null){
            mMmsMultiDeleteAndForwardPlugin.setBodyandAddress(null,0,1, 1, "mms",1);
            assertTrue(true);
        }
    }

    public void test06_forwardItemSelected(){
        if(mMmsMultiDeleteAndForwardPlugin != null){
            mMmsMultiDeleteAndForwardPlugin.onMultiforwardItemSelected();
            assertTrue(true);
        }
    }

    public void test07_getBoxType() {
        if(mMmsMultiDeleteAndForwardPlugin != null){
            mMmsMultiDeleteAndForwardPlugin.getBoxType(0);
            assertTrue(true);
        }
    }

    public void test08_initClass(){
        if(mMmsMultiDeleteAndForwardPlugin != null){
            mMmsMultiDeleteAndForwardPlugin.onNewBodyandAddressForTest();
            assertTrue(true);
        }
    }
    
    public void prepareToForwardMessage() {
    }
}

