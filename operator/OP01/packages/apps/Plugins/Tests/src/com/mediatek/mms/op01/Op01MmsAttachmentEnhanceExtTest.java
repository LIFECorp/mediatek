package com.mediatek.op01.tests;

import android.content.Context;
import android.content.Intent;
import android.provider.MediaStore;
import android.test.InstrumentationTestCase;
import android.view.View;
import android.widget.VideoView;
import android.widget.ImageView;
import android.widget.TextView;

import com.mediatek.pluginmanager.PluginManager;
import com.mediatek.pluginmanager.Plugin;
import com.mediatek.mms.op01.Op01MmsSlideShowExt;

import com.mediatek.xlog.Xlog;
import com.mediatek.mms.op01.Op01MmsAttachmentEnhanceExt;
import com.mediatek.mms.ext.IMmsAttachmentEnhance;
import com.mediatek.op01.plugin.R;
import android.content.Intent;
import android.os.Bundle;

import android.util.TypedValue;
import android.content.res.ColorStateList;
import android.content.res.TypedArray;
import android.content.res.Resources;
import android.util.AttributeSet;



public class Op01MmsAttachmentEnhanceExtTest extends InstrumentationTestCase {
    private Context context;
    private static Op01MmsAttachmentEnhanceExt mMmsAttachmentPlugin = null;
    private static final String TAG = "Op01MmsAttachmentEnhanceExtTest";
    private static final String MMS_SAVE_MODE = "savecontent" ;

    private static final int MMS_SAVE_OTHER_ATTACHMENT = 0;
    private static final int MMS_SAVE_ALL_ATTACHMENT = 1;

	protected void setUp() throws Exception {
        super.setUp();
        context = this.getInstrumentation().getContext();
        mMmsAttachmentPlugin = (Op01MmsAttachmentEnhanceExt)PluginManager.createPluginObject(context, "com.mediatek.mms.ext.IMmsAttachmentEnhance");
    }

    protected void tearDown() throws Exception {
        super.tearDown();
        mMmsAttachmentPlugin = null;
    }

    public void testIsSupportAttachmentEnhance() {
        boolean res;
        assertNotNull(mMmsAttachmentPlugin);
        res = mMmsAttachmentPlugin.isSupportAttachmentEnhance();
        // assertEquals(true, res);
        assertTrue(res);
    }

    public void testSetAttachmentName() {
        TextView txt;
        assertNotNull(mMmsAttachmentPlugin);
        txt = new TextView(context);
        mMmsAttachmentPlugin.setAttachmentName(txt,2);

        if (txt != null) {
            //assertEquals(getString(R.string.multi_files), txt.getText());
            assertEquals("Multi-files",txt.getText());
        } else {
            Xlog.e(TAG, "Can not create Textview");
            assertTrue(false);
        }
    }

    public void testSetSaveAttachIntent() {
        assertNotNull(mMmsAttachmentPlugin);
        Intent i = new Intent();;
        mMmsAttachmentPlugin.setSaveAttachIntent(i,MMS_SAVE_OTHER_ATTACHMENT);

        int smode = -1;

        Bundle data = i.getExtras();
        if (data != null) {
            smode = data.getInt(MMS_SAVE_MODE);
        }
        assertEquals(smode,MMS_SAVE_OTHER_ATTACHMENT);
    }

    public void testGetSaveAttachMode() {
        assertNotNull(mMmsAttachmentPlugin);
        Intent i = new Intent();
        Bundle data = new Bundle();
        data.putInt(MMS_SAVE_MODE,MMS_SAVE_OTHER_ATTACHMENT);
        i.putExtras(data);
        assertEquals(MMS_SAVE_OTHER_ATTACHMENT, mMmsAttachmentPlugin.getSaveAttachMode(i));
    }
}
