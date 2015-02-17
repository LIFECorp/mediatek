package com.mediatek.mms.op01;

import android.app.Activity;
import android.content.Context;
import android.util.FloatMath;
import android.util.Log;
import android.view.MotionEvent;

import com.mediatek.xlog.Xlog;

/**
 * only first and second finger can pinch.
 * if finger num > 2, the gesture stop. if only one finger stay, you can start with another finger down.
 * @author mtk80999
 *
 */
public class ScaleDetector {
    
    private static final String LOGTAG = "ScaleDetector";
    
    /**
     * This value is the threshold ratio between our previous combined pressure
     * and the current combined pressure. We will only fire an onScale event if
     * the computed ratio between the current and previous event pressures is
     * greater than this value. When pressure decreases rapidly between events
     * the position values can often be imprecise, as it usually indicates
     * that the user is in the process of lifting a pointer off of the device.
     * Its value was tuned experimentally.
     */
    private static final float PRESSURE_THRESHOLD = 0.67f;
    
    // Pointer IDs currently responsible for the two fingers controlling the gesture
    private int mActiveId0;
    private int mActiveId1;
    
    private boolean mGestureInProgress;
    
    //when points > 3 and the first/second finger up, the gesture is invalid.
    private boolean mInvalidGesture;
    
    private MotionEvent mPrevEvent;
    private MotionEvent mCurrEvent;
    private float mPrevFingerDiffX;
    private float mPrevFingerDiffY;
    private float mCurrFingerDiffX;
    private float mCurrFingerDiffY;
    private float mCurrLen;
    private float mPrevLen;
    private float mScaleFactor;
    private float mCurrPressure;
    private float mPrevPressure;
    
    private  OnScaleListener mListener;
    private  Activity  mActivity;
    
    public interface OnScaleListener {
        
        /**
         * when the ScaleGesture start, onScaleStart will be called.
         * if return false, the onScale()and onScaleEnd will not be called
         * @return boolean
         */
        boolean onScaleStart(ScaleDetector detector);        
        
        /**
         * 
         */
        void onScaleEnd(ScaleDetector detector);
        
        /**
         * when two touch point the length changed, 
         * @param factor is the rate that the length of current two touch point and length of two touch point when MotionDown
         */
        boolean onScale(ScaleDetector detector);
    }
    
    
    
    private void log(String msg) {
        Log.e(LOGTAG, msg);
    }
    
//    public ScaleDetector(){
//        this(null, null);
//    }
//    
//    public ScaleDetector(OnScaleListener listener){
//        this(null, listener);
//    }

    public ScaleDetector(OnScaleListener listener) {
        this(null, listener);
    }

    
    public ScaleDetector(Activity activity, OnScaleListener listener) {
        mActivity = activity;
        mListener = listener;
        reset();
    }

    public void setActivity(Activity activity) {
        mActivity = activity;
    }
    
    
    
    public boolean  onTouchEvent(MotionEvent event) {
        
        boolean ret = false;
        
        
        final int action = event.getActionMasked();
        
        if (action == MotionEvent.ACTION_DOWN) {
            reset(); // Start fresh
        }
        
       
        switch (action) {
        
        case MotionEvent.ACTION_DOWN:
            mActiveId0 = event.getPointerId(0);
            log("ACTION_DOWN: count = " + event.getPointerCount());
            break;
        
        
        case MotionEvent.ACTION_POINTER_DOWN:
            int count = event.getPointerCount();
            int index = event.getActionIndex();
            int id = event.getPointerId(index);
            log("ACTION_POINTER_DOWN: count = " + count + ", actionId = " + id);
            
            if (count == 2) {
                mActiveId0 = event.getPointerId(0);
                mActiveId1 = event.getPointerId(1);                
                
                mPrevEvent = MotionEvent.obtain(event);
                setContext(event);
                
                if (mListener != null) {
                    mGestureInProgress = mListener.onScaleStart(this);
                    if (mGestureInProgress) {
                      //send ACTION_CANCEL to cancel previous actions before ACTION_POINTER_DOWN
                        MotionEvent cancle = MotionEvent.obtain(0, 0, MotionEvent.ACTION_CANCEL, 0, 0, 0);
                        if (mActivity != null) {
                            mActivity.getWindow().superDispatchTouchEvent(cancle);
                        }
                    }
                }
                mInvalidGesture = false;
            }
            
            if (count > 2 && !mInvalidGesture) {
                mInvalidGesture = true;
                setContext(event);
                if (mGestureInProgress && mListener != null) {
                    mListener.onScaleEnd(this);
                }
            }
            break;
        
        case MotionEvent.ACTION_MOVE:
          if (mGestureInProgress && !mInvalidGesture) {
              setContext(event);
              
              // Only accept the event if our relative pressure is within
              // a certain limit - this can help filter shaky data as a
              // finger is lifted.
              if (mCurrPressure / mPrevPressure > PRESSURE_THRESHOLD) {
                  final boolean updatePrevious = mListener.onScale(this);

                  if (updatePrevious) {
                      mPrevEvent.recycle();
                      mPrevEvent = MotionEvent.obtain(event);
                  }
              }
          }
          break;
          
        case MotionEvent.ACTION_POINTER_UP:
            int count2 = event.getPointerCount();
            int index2 = event.getActionIndex();
            int id2 = event.getPointerId(index2);
            log("ACTION_POINTER_UP, count = " + count2 + ", ActionId = " + id2);
            
            if (mGestureInProgress && count2 == 2 && !mInvalidGesture) {                
                setContext(event);
                if (mListener != null) {                    
                    mListener.onScaleEnd(this);
                }
                mInvalidGesture = true;
            }
            break;
        
        case MotionEvent.ACTION_UP:
            log("ACTION_UP");
            reset();
            break;
            
        case MotionEvent.ACTION_CANCEL:
            log("ACTION_CANCEL");
            reset();
            break;  
        default:
            break;
        }
        
        if (!mGestureInProgress) {
            log("return value is false, action = " + event.getActionMasked());
        }
        
//        return true;
        return mGestureInProgress;
    }
    
    
    private void reset() {        
        
        if (mPrevEvent != null) {
            mPrevEvent.recycle();
            mPrevEvent = null;
        }
        if (mCurrEvent != null) {
            mCurrEvent.recycle();
            mCurrEvent = null;
        }
        
        
        mActiveId0 = -1;
        mActiveId1 = -1;
        mGestureInProgress = false;
        mInvalidGesture = false;
    }
    
    private void setContext(MotionEvent curr) {
        if (mCurrEvent != null) {
            mCurrEvent.recycle();
        }
        mCurrEvent = MotionEvent.obtain(curr);

        mCurrLen = -1;
        mPrevLen = -1;
        mScaleFactor = -1;

        final MotionEvent prev = mPrevEvent;

        final int prevIndex0 = prev.findPointerIndex(mActiveId0);
        final int prevIndex1 = prev.findPointerIndex(mActiveId1);
        final int currIndex0 = curr.findPointerIndex(mActiveId0);
        final int currIndex1 = curr.findPointerIndex(mActiveId1);

        if (prevIndex0 < 0 || prevIndex1 < 0 || currIndex0 < 0 || currIndex1 < 0) {
            mInvalidGesture = true;
            if (mGestureInProgress) {
                mListener.onScaleEnd(this);
            }
            return;
        }

        final float px0 = prev.getX(prevIndex0);
        final float py0 = prev.getY(prevIndex0);
        final float px1 = prev.getX(prevIndex1);
        final float py1 = prev.getY(prevIndex1);
        final float cx0 = curr.getX(currIndex0);
        final float cy0 = curr.getY(currIndex0);
        final float cx1 = curr.getX(currIndex1);
        final float cy1 = curr.getY(currIndex1);

        final float pvx = px1 - px0;
        final float pvy = py1 - py0;
        final float cvx = cx1 - cx0;
        final float cvy = cy1 - cy0;
        mPrevFingerDiffX = pvx;
        mPrevFingerDiffY = pvy;
        mCurrFingerDiffX = cvx;
        mCurrFingerDiffY = cvy;

        mCurrPressure = curr.getPressure(currIndex0) + curr.getPressure(currIndex1);
        mPrevPressure = prev.getPressure(prevIndex0) + prev.getPressure(prevIndex1);
    }
    
    
    
    /**
     * Return the current distance between the two pointers forming the
     * gesture in progress.
     *
     * @return Distance between pointers in pixels.
     */
    public float getCurrentSpan() {
        if (mCurrLen == -1) {
            final float cvx = mCurrFingerDiffX;
            final float cvy = mCurrFingerDiffY;
            mCurrLen = FloatMath.sqrt(cvx * cvx + cvy * cvy);
        }
        return mCurrLen;
    }


    /**
     * Return the current x distance between the two pointers forming the
     * gesture in progress.
     *
     * @return Distance between pointers in pixels.
     */
    public float getCurrentSpanX() {
        return mCurrFingerDiffX;
    }

    /**
     * Return the current y distance between the two pointers forming the
     * gesture in progress.
     *
     * @return Distance between pointers in pixels.
     */
    public float getCurrentSpanY() {
        return mCurrFingerDiffY;
    }

    /**
     * Return the previous distance between the two pointers forming the
     * gesture in progress.
     *
     * @return Previous distance between pointers in pixels.
     */
    public float getPreviousSpan() {
        if (mPrevLen == -1) {
            final float pvx = mPrevFingerDiffX;
            final float pvy = mPrevFingerDiffY;
            mPrevLen = FloatMath.sqrt(pvx * pvx + pvy * pvy);
        }
        return mPrevLen;
    }

    /**
     * Return the previous x distance between the two pointers forming the
     * gesture in progress.
     *
     * @return Previous distance between pointers in pixels.
     */
    public float getPreviousSpanX() {
        return mPrevFingerDiffX;
    }

    /**
     * Return the previous y distance between the two pointers forming the
     * gesture in progress.
     *
     * @return Previous distance between pointers in pixels.
     */
    public float getPreviousSpanY() {
        return mPrevFingerDiffY;
    }

    /**
     * Return the scaling factor from the previous scale event to the current
     * event. This value is defined as
     * ({@link #getCurrentSpan()} / {@link #getPreviousSpan()}).
     *
     * @return The current scaling factor.
     */
    public float getScaleFactor() {
        if (mScaleFactor == -1) {
            mScaleFactor = getCurrentSpan() / getPreviousSpan();
        }
        return mScaleFactor;
    }
    
    public abstract static class SimpleOnScaleListener implements OnScaleListener {
        private static final String TAG = "SimpleOnScaleListener";
        private static final int DEFAULT_TEXT_SIZE = 18;
        private static final int MIN_TEXT_SIZE = 10;
        private static final int MAX_TEXT_SIZE = 32;
//        private ScaleDetector mScaleDetector;
        private float mTextSize = DEFAULT_TEXT_SIZE;
        private static final float MIN_ADJUST_TEXT_SIZE = 0.2f;
        
        private Context mContext;
        
        
        
        public interface OnTextSizeChanged {
            void onChanged(float size);
        }
        
        
        protected abstract void performChangeText(float size);
        

        public SimpleOnScaleListener() {
            this(null, 0.0f);
        }
      
        public SimpleOnScaleListener(Context context, float initTextSize) {
            mContext = context;
            mTextSize = initTextSize;
        }

        public void setContext(Context context) {
            mContext = context;
        }

        public void setTextSize(float size) {
            mTextSize = size;
        }

        
        public void onScaleEnd(ScaleDetector detector) {
            Xlog.i(TAG, "onScaleEnd -> mTextSize = " + mTextSize);

            //save current value to preference
            Op01MmsUtils.setTextSize(mContext, mTextSize);
        }
        
        public boolean onScale(ScaleDetector detector) {
            float size = mTextSize * detector.getScaleFactor();

            if (Math.abs(size - mTextSize) < MIN_ADJUST_TEXT_SIZE) {
                return false;
            }
            if (size < MIN_TEXT_SIZE) {
                size = MIN_TEXT_SIZE;
            }
            if (size > MAX_TEXT_SIZE) {
                size = MAX_TEXT_SIZE;
            }
            if ((size - mTextSize > 0.0000001) || (size - mTextSize < -0.0000001)) {
                mTextSize = size;
                performChangeText(size);
            }
            return true;
        }        

        public boolean onScaleStart(ScaleDetector detector) {
            Xlog.i(TAG, "onScaleStart -> mTextSize = " + mTextSize);
            return true;
        }
    }
}
