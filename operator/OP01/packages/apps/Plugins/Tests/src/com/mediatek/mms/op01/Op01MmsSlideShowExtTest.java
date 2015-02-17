
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
import android.util.TypedValue;
import android.content.res.ColorStateList;
import android.content.res.TypedArray;
import android.content.res.Resources;
import android.util.AttributeSet;


public class Op01MmsSlideShowExtTest extends InstrumentationTestCase {
	private static final int PLAY_AS_PAUSE = 1; 
    private final static float DEFAULT_TEXT_SIZE = 18;
	private Context context;
	private static Op01MmsSlideShowExt mMmsSlideShowPlugin = null;
    private static final String TAG = "Op01MmsSlideShowExtTest";
    
    protected void setUp() throws Exception {
        super.setUp();
        context = this.getInstrumentation().getContext();
        mMmsSlideShowPlugin = (Op01MmsSlideShowExt)PluginManager.createPluginObject(context, "com.mediatek.mms.ext.IMmsSlideShow");
    }
       
    protected void tearDown() throws Exception {
        super.tearDown();
        mMmsSlideShowPlugin = null;
    }
    
    public void testGetInitialPlayState(){
    	int res;
    	assertNotNull(mMmsSlideShowPlugin);
    	res = mMmsSlideShowPlugin.getInitialPlayState();
    	assertEquals(0, res);
   	}
   	
    public void testConfigTextView() {   		   	
    	TextView view;
    	assertNotNull(mMmsSlideShowPlugin);
    	view = new TextView(context);
        if (view != null){
            mMmsSlideShowPlugin.configTextView(context,view);
        	
        	int valDefault = (int)TypedValue.applyDimension(TypedValue.COMPLEX_UNIT_SP,
                DEFAULT_TEXT_SIZE, context.getResources().getDisplayMetrics());

            assertEquals(valDefault, (int)view.getTextSize());
            Xlog.d(TAG, "view size = " + view.getTextSize());
            Xlog.d(TAG, "view valDefault = " + valDefault);
        }else{                
            Xlog.e(TAG, "can not create Textview");
            assertTrue(false);
        }
    }
   	
    public void testConfigVideoView(){
    	myVideoView v;
    	assertNotNull(mMmsSlideShowPlugin);
    	v = new myVideoView(context);
    	mMmsSlideShowPlugin.configVideoView(v);
    	assertEquals(1, v.getSeekTo());
    }
    
    private class myVideoView extends VideoView
    {
    	int index = 0;
    	public void seekTo(int i){
    		index = i;
    	}
    	
    	public int getSeekTo(){
    		return index;	
    	}

        public myVideoView(Context context) {
            super(context);
        }
    } 
}