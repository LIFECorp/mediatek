package com.mediatek.backuprestore;

import android.content.Context;
import android.preference.Preference;
import android.util.AttributeSet;
import android.view.View;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;

import com.mediatek.backuprestore.utils.MyLogger;

import java.io.File;

public class RestoreCheckBoxPreference extends Preference {

    private static final String CLASS_TAG = MyLogger.LOG_TAG + "/RestoreCheckBoxPreference";

    private CheckBox mCheckBox;
    private boolean mVisibility = false;
    private boolean mChecked = false;
    private File mAccociateFile;

    public RestoreCheckBoxPreference(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);

    }

    public RestoreCheckBoxPreference(Context context, AttributeSet attrs) {
        super(context, attrs);
        setWidgetLayoutResource(R.layout.prefrence_checkbox);
    }

    public RestoreCheckBoxPreference(Context context) {
        super(context);
        setWidgetLayoutResource(R.layout.prefrence_checkbox);
    }

    public File getAccociateFile() {
        return mAccociateFile;
    }

    public void setAccociateFile(File accociateFile) {
        this.mAccociateFile = accociateFile;
    }

    @Override
    protected void onBindView(View view) {
        mCheckBox = (CheckBox) view.findViewById(R.id.prefrence_checkbox_id);

        showCheckbox(mVisibility);
        if (mCheckBox != null) {
            mCheckBox.setOnCheckedChangeListener(new OnCheckedChangeListener() {

                @Override
                public void onCheckedChanged(CompoundButton arg0, boolean arg1) {
                    mChecked = arg1;
                }
            });
        }
        setChecked(mChecked);
        super.onBindView(view);
    }

    public void showCheckbox(boolean bshow) {
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
    }

    public boolean isChecked() {
        return mChecked;
    }

    public void setChecked(boolean bChecked) {
        mChecked = bChecked;
        if (mCheckBox != null) {
            mCheckBox.setChecked(bChecked);
        }
    }

    public String getKey() {
        if (mAccociateFile != null) {
            return mAccociateFile.getAbsolutePath();
        }
        return "";
    }
}
