package com.mediatek.backuprestore;

import android.content.Context;
import android.preference.Preference;
import android.util.AttributeSet;
import android.view.View;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.RadioButton;

import com.mediatek.backuprestore.utils.MyLogger;

import java.io.File;

public class AppBackupRadioButtonPreference extends Preference {

    private static final String CLASS_TAG = MyLogger.LOG_TAG + "/AppBackupRadioButtonPreference";

    private RadioButton mRadioButton;
    private boolean mChecked = false;

    public AppBackupRadioButtonPreference(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);

    }

    public AppBackupRadioButtonPreference(Context context, AttributeSet attrs) {
        super(context, attrs);
        setWidgetLayoutResource(R.layout.prefrence_radiobutton);
    }

    public AppBackupRadioButtonPreference(Context context) {
        super(context);
        setWidgetLayoutResource(R.layout.prefrence_radiobutton);
    }


    @Override
    protected void onBindView(View view) {
        mRadioButton = (RadioButton) view.findViewById(R.id.prefrence_radioButton_id);

        //showCheckbox(mVisibility);
        if (mRadioButton != null) {
            mRadioButton.setOnCheckedChangeListener(new OnCheckedChangeListener() {

                @Override
                public void onCheckedChanged(CompoundButton arg0, boolean arg1) {
                    mChecked = arg1;
                }
            });
        }
        setChecked(mChecked);
        super.onBindView(view);
    }

    /*public void showCheckbox(boolean bshow) {
        MyLogger.logD(CLASS_TAG, "showCheckbox: " + bshow);
        mVisibility = bshow;
        if (mCheckBox == null) {
            return;
        }
        if (bshow) {
            mCheckBox.setVisibility(View.VISIBLE);
        } else {
            mCheckBox.setVisibility(View.GONE);
        }
    }*/

    public boolean isChecked() {
        return mChecked;
    }

    public void setChecked(boolean bChecked) {
        mChecked = bChecked;
        if (mRadioButton != null) {
            mRadioButton.setChecked(bChecked);
        }
    }

}
