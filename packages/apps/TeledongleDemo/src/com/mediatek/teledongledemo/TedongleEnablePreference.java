package com.mediatek.teledongledemo;

import android.content.Context;
import android.preference.Preference;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.LinearLayout;
import android.widget.Switch;
import android.widget.CheckBox;
import android.widget.ImageView;
import android.widget.RelativeLayout;
import android.widget.TextView;

public class TedongleEnablePreference extends Preference implements
        OnClickListener {

    //interface OnPreferenceClickCallback {

    //    void onPreferenceClick();
    //}

    private static final String TAG = "TedongleEnablePreference";
    private OnCheckedChangeListener mSwitchChangeListener;;
    private Context mContext;
    private boolean mRadioOn;
    //private OnPreferenceClickCallback mClickCallback;
    private boolean mDisableSwitch;
    private String mName;
    private String mNumber;
    private int mStatus;
    private int mColor;
    private int mDisplayNumberFormat;
    
    /**
     * 
     * @param context
     *            Context
     * @param name
     *            String
     * @param number
     *            String
     * @param simSlot
     *            int
     * @param status
     *            int
     * @param color
     *            int
     * @param displayNumberFormat
     *            int
     * @param key
     *            long
     * @param isAirModeOn
     *            boolean
     */
    public TedongleEnablePreference(Context context, int status, boolean isAirModeOn) {
    	super(context, null );
        mContext = context;
        mStatus = status;
        mRadioOn = true;

        mDisableSwitch = isAirModeOn;
        setLayoutResource(R.layout.preference_tedongle_info);
    }

    @Override
    public View getView(View convertView, ViewGroup parent) {
        // TODO Auto-generated method stub
        View view = super.getView(convertView, parent);
        /*
        ImageView imageStatus = (ImageView) view.findViewById(R.id.tedongleStatus);
        if (imageStatus != null) {
            int res = R.drawable.ic_default_user;
            if (res == -1) {
                imageStatus.setVisibility(View.GONE);
            } else {
                imageStatus.setImageResource(res);
            }
        }*/
        
        Switch ckRadioOn = (Switch) view.findViewById(R.id.Check_Enable);

        if (ckRadioOn != null) {
            if (mSwitchChangeListener != null) {
                ckRadioOn.setClickable(true);
                ckRadioOn.setEnabled(!mDisableSwitch);
                ckRadioOn.setOnCheckedChangeListener(mSwitchChangeListener);
            }
        }
        View siminfoLayout = view.findViewById(R.id.tedongle_info_layout);
        if ((siminfoLayout != null) && siminfoLayout instanceof LinearLayout) {
            siminfoLayout.setOnClickListener(this);
            // siminfoLayout.setFocusable(true);
        }

        return view;
    }
    
        
    @Override
    public void onClick(View v) {
        // TODO Auto-generated method stub

        if (v == null) {
            return;
        }

    }

    void setCheckBoxClickListener(OnCheckedChangeListener listerner) {
        mSwitchChangeListener = listerner;

    }

    //void setClickCallback(OnPreferenceClickCallback callBack) {
    //    mClickCallback = callBack;
    //}

    boolean isRadioOn() {
        return mRadioOn;
    }

    void setRadioOn(boolean radioOn) {
        mRadioOn = radioOn;

    }

}
