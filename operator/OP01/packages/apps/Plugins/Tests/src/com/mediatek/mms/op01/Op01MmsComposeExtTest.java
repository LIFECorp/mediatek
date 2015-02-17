package com.mediatek.op01.tests;

import android.content.Context;
import android.test.InstrumentationTestCase;
import android.widget.EditText;

import com.mediatek.pluginmanager.PluginManager;
import com.mediatek.pluginmanager.Plugin;
import com.mediatek.mms.ext.IMmsCompose;
import com.mediatek.mms.op01.Op01MmsComposeExt;
import com.mediatek.xlog.Xlog;

public class Op01MmsComposeExtTest extends InstrumentationTestCase
{
    private final String TAG = "Op01MmsComposeExtTest";
    private static Op01MmsComposeExt mMmsComposePlugin = null;
    private Context mContext;
    

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mContext = this.getInstrumentation().getContext();
        Object plugin = PluginManager.createPluginObject(mContext, "com.mediatek.mms.ext.IMmsCompose");
        if(plugin instanceof Op01MmsComposeExt){
            mMmsComposePlugin = (Op01MmsComposeExt) plugin;
        }
    }
    
    @Override    
    protected void tearDown() throws Exception {
        super.tearDown();
        mMmsComposePlugin = null;
    }


    public void test01_GetCapturePicMode(){
        if(mMmsComposePlugin != null){
            int mode = mMmsComposePlugin.getCapturePicMode();
            assertEquals( mode, IMmsCompose.CAPTURE_PIC_AT_TEMP_FILE);
        }
    }
	
    public void test02_IsAppendSender(){
        if(mMmsComposePlugin != null){
            boolean ret = mMmsComposePlugin.isAppendSender();
            assertTrue(ret);
        }
    }

    public void test03_IsAddMmsUrlToBookMark(){
        if(mMmsComposePlugin != null){
            boolean ret = mMmsComposePlugin.isAddMmsUrlToBookMark();
            assertTrue(ret);
        }
    }

    public void test04_ConfigSubjectEditor(){
        if(mMmsComposePlugin != null){
            EditText editor = new EditText(mContext);
            if(editor != null){
                String string1 = "12345678901234567890";
                String string2 = "12345678901234567890123456789012345678901";
                mMmsComposePlugin.configSubjectEditor(editor);
                editor.setText(string1);
                String string3 = editor.getText().toString();
                assertEquals(string1, string3);
                editor.setText(string2);
                String string4 = editor.getText().toString();
                int length = string4.length();
                assertEquals(length, 40);
            }else{
                Xlog.e(TAG, "can not create EditText");
                assertTrue(false);
            }
        }
    }
}