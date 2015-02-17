package com.mediatek.teledongledemo;
import android.content.Context;  
import android.content.res.TypedArray;  
import android.graphics.drawable.Drawable;  
import android.preference.Preference;  
import android.util.AttributeSet;  
import android.util.Log;  
import android.view.View;  
import android.widget.ImageView;
import android.view.LayoutInflater;
import android.view.ViewGroup;

public class IconPreferenceScreen extends Preference {  
      
    private static final String TAG = "IconPreferenceScreen";  
    private Drawable mIcon;  
    private Context mContext;
    public IconPreferenceScreen(Context context, AttributeSet attrs) {  
        this(context, attrs, 0);
        mContext = context;
    }  
    public IconPreferenceScreen(Context context, AttributeSet attrs, int defStyle) {  
        super(context, attrs, defStyle);  
        mContext = context;
        TypedArray a = mContext.obtainStyledAttributes(attrs,  
               R.styleable.IconPreferenceScreen, defStyle, 0);  
        mIcon = a.getDrawable(R.styleable.IconPreferenceScreen_icon);  
    }  
    protected View onCreateView(ViewGroup parent) {
        final LayoutInflater layoutInflater =
            (LayoutInflater) mContext.getSystemService(Context.LAYOUT_INFLATER_SERVICE);       
        final View layout = layoutInflater.inflate(R.layout.preference_icon, parent, false);
        return layout;

    }
    @Override  
    public void onBindView(View view) {  
        super.onBindView(view);  
        Log.i(TAG, "[onBindView](0)");  
        ImageView imageView = (ImageView) view.findViewById(R.id.icon);  
        //if (imageView != null && mIcon != null) {  
        imageView.setImageDrawable(mIcon); 
            Log.i(TAG, "[onBindView](1)");  
        //}  
        Log.i(TAG, "[onBindView](2)");  
    }  
    public void setIcon(Drawable icon){
    	mIcon = icon;
    	notifyChanged();
    }
}  
