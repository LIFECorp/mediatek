
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
import com.mediatek.mms.op01.Op01MmsSlideshowEditorExt;

import com.mediatek.xlog.Xlog;
import android.util.TypedValue;
import android.content.res.ColorStateList;
import android.content.res.TypedArray;
import android.content.res.Resources;
import android.util.AttributeSet;


public class Op01MmsSlideShowEditorExtTest extends InstrumentationTestCase {
	private static final int PLAY_AS_PAUSE = 1; 
    private Context context;
	private static Op01MmsSlideshowEditorExt mMmsSlideshowEditorPlugin = null;
    private static final String TAG = "Op01MmsSlideShowEditorExtTest";
    
    protected void setUp() throws Exception {
        super.setUp();
        context = this.getInstrumentation().getContext();
        mMmsSlideshowEditorPlugin = (Op01MmsSlideshowEditorExt)PluginManager.createPluginObject(context, "com.mediatek.mms.ext.IMmsSlideshowEditor");
    }
       
    protected void tearDown() throws Exception {
        super.tearDown();
        mMmsSlideshowEditorPlugin = null;
    }
    
    public void testIsSupportAddTopSlide(){
    	boolean res;
    	res = mMmsSlideshowEditorPlugin.isSupportAddTopSlide();
    	assertTrue(res == true);
   	}
}