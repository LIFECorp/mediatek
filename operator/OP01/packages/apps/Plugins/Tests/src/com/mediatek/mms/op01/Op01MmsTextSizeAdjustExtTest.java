package com.mediatek.op01.tests;

import android.app.Instrumentation;
import android.content.Context;
import android.os.SystemClock;
import android.test.InstrumentationTestCase;
import android.view.MotionEvent;
import android.view.MotionEvent.PointerCoords;
import android.widget.EditText;

import com.mediatek.pluginmanager.PluginManager;
import com.mediatek.pluginmanager.Plugin;
import com.mediatek.mms.ext.IMmsTextSizeAdjust;
import com.mediatek.mms.ext.IMmsTextSizeAdjustHost;
import com.mediatek.mms.op01.Op01MmsTextSizeAdjustExt;
import com.mediatek.mms.op01.Op01MmsUtils;
import com.mediatek.xlog.Xlog;

public class Op01MmsTextSizeAdjustExtTest extends InstrumentationTestCase
                                                        implements IMmsTextSizeAdjustHost{
    private final String TAG = "Op01MmsTextSizeAdjustExtTest";
    private static Op01MmsTextSizeAdjustExt mPlugin = null;
    private Context mContext;
    private float mTextSize = 18;
    

    @Override
    protected void setUp() throws Exception {
        super.setUp();
//        mContext = this.getInstrumentation().getContext();
        mContext = this.getInstrumentation().getTargetContext();
        Object plugin = PluginManager.createPluginObject(mContext, "com.mediatek.mms.ext.IMmsTextSizeAdjust");
        if(plugin instanceof Op01MmsTextSizeAdjustExt){
            mPlugin = (Op01MmsTextSizeAdjustExt) plugin;
        }
    }
    
    @Override
    protected void tearDown() throws Exception {
        super.tearDown();
        mPlugin = null;
    }

    @Override
    public void setTextSize(float size){
        Xlog.v(TAG, "setTextSize: size = " + size);
        mTextSize = size;
    }


    public void test01_initAndRefresh(){
        Xlog.v(TAG, "test01_initAndRefresh...");
        if(mPlugin != null){
            mPlugin.init(this, null);
            mPlugin.refresh();
            assertTrue(mTextSize == 18);
            
            float size = 20;
            mPlugin.setHostContext(mContext);
            Op01MmsUtils.setTextSize(mContext, size);
            mPlugin.refresh();
            assertTrue(mTextSize == size);
        }
    }
    
    public void test02_notScale(){
        Xlog.v(TAG, "test02_notScale...");
        if(mPlugin != null){
            mPlugin.init(this, null);
            mPlugin.setHostContext(mContext);
            mTextSize = 18;
            Op01MmsUtils.setTextSize(mContext, mTextSize);
            
            long downTime = SystemClock.uptimeMillis();
            long eventTime = SystemClock.uptimeMillis();
            MotionEvent event = MotionEvent.obtain(downTime, eventTime, MotionEvent.ACTION_DOWN,
                    50, 50, 1);
            mPlugin.dispatchTouchEvent(event);
            event.recycle();
            
            event = MotionEvent.obtain(downTime, eventTime, MotionEvent.ACTION_MOVE,
                    50, 100, 1);
            mPlugin.dispatchTouchEvent(event);
            event.recycle();
            
            event = MotionEvent.obtain(downTime, eventTime, MotionEvent.ACTION_DOWN,
                    50, 100, 1);
            mPlugin.dispatchTouchEvent(event);
            event.recycle();
            float curSize = Op01MmsUtils.getTextSize(mContext);
            assertTrue(mTextSize == curSize);
            assertTrue(mTextSize == 18);
        }
    }
    
    public void test03_ScaleLarge()throws Exception{
        Xlog.v(TAG, "test03_ScaleLarge...");
        if(mPlugin != null){
            mTextSize = 18;
            Op01MmsUtils.setTextSize(mContext, mTextSize);
            mPlugin.init(this, null);
            mPlugin.setHostContext(mContext);
            
            float beginSize = mTextSize;
            Xlog.v(TAG, "test03: beginSize = " + beginSize );
            
            float x = 100;
            float y2 = 200;
            float y1_1 = 100;
            float y1_2 = 90;
            float y1_3 = 80;
            float y1_4 = 70;
            float y1_5 = 60;
            
            int[] ids1= new int[]{1,2};
            int[] ids2= new int[]{2,1};
            
            long downTime = SystemClock.uptimeMillis();
            long eventTime = SystemClock.uptimeMillis();
            MotionEvent event;

            PointerCoords p1 = new PointerCoords();
            PointerCoords p2 = new PointerCoords();
            p1.x = x;
            p1.y = y1_1;
            p1.pressure = 1;
            p2.x = x;
            p2.y = y2;
            p2.pressure = 1;
            PointerCoords[] pointers1 = new PointerCoords[] {p1, p2};
            PointerCoords[] pointers2 = new PointerCoords[] {p2, p1};

            event = MotionEvent.obtain(downTime, eventTime, MotionEvent.ACTION_DOWN, 1,
                ids1, pointers1, 0, 1, 1, 0,0,0,0);
            mPlugin.dispatchTouchEvent(event);
            event.recycle();

            event = MotionEvent.obtain(downTime, eventTime, MotionEvent.ACTION_POINTER_DOWN, 2,
                    ids2, pointers2, 0, 0, 0, 0,0,0,0);
            mPlugin.dispatchTouchEvent(event);
            event.recycle();
            
            p1.y = y1_2;
            event = MotionEvent.obtain(downTime, eventTime, MotionEvent.ACTION_MOVE, 2,
                    ids1, pointers1, 1, 1, 0, 0,0,0,0);
            mPlugin.dispatchTouchEvent(event);
            Thread.sleep(50);
            event.recycle();

            p1.y = y1_3;
            event = MotionEvent.obtain(downTime, eventTime, MotionEvent.ACTION_MOVE, 2,
                    ids1, pointers1, 1, 1, 0, 0,0,0,0);
            mPlugin.dispatchTouchEvent(event);
            Thread.sleep(50);
            event.recycle();

            p1.y = y1_4;
            event = MotionEvent.obtain(downTime, eventTime, MotionEvent.ACTION_MOVE, 2,
                    ids1, pointers1, 1, 1, 0, 0,0,0,0);
            mPlugin.dispatchTouchEvent(event);
            Thread.sleep(50);
            event.recycle();

            p1.y = y1_5;
            event = MotionEvent.obtain(downTime, eventTime, MotionEvent.ACTION_MOVE, 2,
                    ids1, pointers1, 1, 1, 0, 0,0,0,0);
            mPlugin.dispatchTouchEvent(event);
            Thread.sleep(50);
            event.recycle();

            event = MotionEvent.obtain(downTime, eventTime, MotionEvent.ACTION_POINTER_UP, 2,
                    ids1, pointers1, 1, 1, 0, 0,0,0,0);
            mPlugin.dispatchTouchEvent(event);
            event.recycle();
            
            event = MotionEvent.obtain(downTime, eventTime, MotionEvent.ACTION_UP, 1,
                    ids1, pointers1, 1, 1, 0, 0,0,0,0);
            mPlugin.dispatchTouchEvent(event);
            Thread.sleep(100);
            event.recycle();
            
            float endSize = mTextSize;
            Xlog.v(TAG, "end text size = " + endSize);
            
            assertTrue(beginSize < endSize);
            assertTrue(beginSize > 10);
            assertTrue(beginSize <= 32);
        }
    }
    
    public void test04_ScaleSmall() throws Exception{
        Xlog.v(TAG, "test03_ScaleLarge...");
        if(mPlugin != null){
            mTextSize = 30;
            Op01MmsUtils.setTextSize(mContext, mTextSize);
            mPlugin.init(this, null);
            mPlugin.setHostContext(mContext);
            
            float beginSize = mTextSize;
            Xlog.v(TAG, "test03: beginSize = " + beginSize );
            
            float x = 100;
            float y2 = 200;
            float y1_1 = 50;
            float y1_2 = 60;
            float y1_3 = 70;
            float y1_4 = 80;
            float y1_5 = 90;
            
            int[] ids1= new int[]{1,2};
            int[] ids2= new int[]{2,1};
            
            long downTime = SystemClock.uptimeMillis();
            long eventTime = SystemClock.uptimeMillis();
            MotionEvent event;

            PointerCoords p1 = new PointerCoords();
            PointerCoords p2 = new PointerCoords();
            p1.x = x;
            p1.y = y1_1;
            p1.pressure = 1;
            p2.x = x;
            p2.y = y2;
            p2.pressure = 1;
            PointerCoords[] pointers1 = new PointerCoords[] {p1, p2};
            PointerCoords[] pointers2 = new PointerCoords[] {p2, p1};

            event = MotionEvent.obtain(downTime, eventTime, MotionEvent.ACTION_DOWN, 1,
                ids1, pointers1, 0, 1, 1, 0,0,0,0);
            mPlugin.dispatchTouchEvent(event);
            event.recycle();

            event = MotionEvent.obtain(downTime, eventTime, MotionEvent.ACTION_POINTER_DOWN, 2,
                    ids2, pointers2, 0, 0, 0, 0,0,0,0);
            mPlugin.dispatchTouchEvent(event);
            event.recycle();
            
            p1.y = y1_2;
            event = MotionEvent.obtain(downTime, eventTime, MotionEvent.ACTION_MOVE, 2,
                    ids1, pointers1, 1, 1, 0, 0,0,0,0);
            mPlugin.dispatchTouchEvent(event);
            Thread.sleep(50);
            event.recycle();

            p1.y = y1_3;
            event = MotionEvent.obtain(downTime, eventTime, MotionEvent.ACTION_MOVE, 2,
                    ids1, pointers1, 1, 1, 0, 0,0,0,0);
            mPlugin.dispatchTouchEvent(event);
            Thread.sleep(50);
            event.recycle();

            p1.y = y1_4;
            event = MotionEvent.obtain(downTime, eventTime, MotionEvent.ACTION_MOVE, 2,
                    ids1, pointers1, 1, 1, 0, 0,0,0,0);
            mPlugin.dispatchTouchEvent(event);
            Thread.sleep(50);
            event.recycle();

            p1.y = y1_5;
            event = MotionEvent.obtain(downTime, eventTime, MotionEvent.ACTION_MOVE, 2,
                    ids1, pointers1, 1, 1, 0, 0,0,0,0);
            mPlugin.dispatchTouchEvent(event);
            Thread.sleep(50);
            event.recycle();

            event = MotionEvent.obtain(downTime, eventTime, MotionEvent.ACTION_POINTER_UP, 2,
                    ids1, pointers1, 1, 1, 0, 0,0,0,0);
            mPlugin.dispatchTouchEvent(event);
            event.recycle();
            
            event = MotionEvent.obtain(downTime, eventTime, MotionEvent.ACTION_UP, 1,
                    ids1, pointers1, 1, 1, 0, 0,0,0,0);
            mPlugin.dispatchTouchEvent(event);
            Thread.sleep(100);
            event.recycle();
            
            float endSize = mTextSize;
            Xlog.v(TAG, "end text size = " + endSize);
            
            assertTrue(beginSize > endSize);
            assertTrue(beginSize >= 10);
            assertTrue(beginSize < 32);
        }
    }
}