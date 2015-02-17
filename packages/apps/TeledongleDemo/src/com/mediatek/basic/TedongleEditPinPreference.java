package com.mediatek.basic;

import android.app.Dialog;
import android.content.Context;
import android.preference.EditTextPreference;
import android.text.InputType;
import android.util.AttributeSet;
import android.view.View;
import android.widget.EditText;

/**
 * TODO: Add a soft dialpad for PIN entry.
 */
public class TedongleEditPinPreference extends EditTextPreference {

    public interface OnPinEnteredListener {
        public void onPinEntered(TedongleEditPinPreference preference, boolean positiveResult);
    }
    
    private OnPinEnteredListener mPinListener;
    
    public TedongleEditPinPreference(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    public TedongleEditPinPreference(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
    }
    
    public void setOnPinEnteredListener(OnPinEnteredListener listener) {
        mPinListener = listener;
    }

    @Override
    protected void onBindDialogView(View view) {
        super.onBindDialogView(view);

        final EditText editText = getEditText();

        if (editText != null) {
            editText.setInputType(InputType.TYPE_CLASS_NUMBER |
                InputType.TYPE_NUMBER_VARIATION_PASSWORD);
        }
    }

    public boolean isDialogOpen() {
        Dialog dialog = getDialog();
        return dialog != null && dialog.isShowing();
    }

    @Override
    protected void onDialogClosed(boolean positiveResult) {
        super.onDialogClosed(positiveResult);
        if (mPinListener != null) {
            mPinListener.onPinEntered(this, positiveResult);
        }
    }

    public void showPinDialog() {
        Dialog dialog = getDialog();
        if (dialog == null || !dialog.isShowing()) {
            showDialog(null);
        }
    }
}

