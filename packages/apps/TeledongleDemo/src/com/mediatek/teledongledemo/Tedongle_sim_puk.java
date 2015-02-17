package com.mediatek.teledongledemo;

import android.app.Activity;
import android.app.Dialog;
import android.app.ProgressDialog;
import android.content.Context;
import android.content.Intent;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.text.Editable;
import android.text.InputType;
import android.text.TextWatcher;
import android.text.method.DigitsKeyListener;
import android.util.AttributeSet;
import android.view.HapticFeedbackConstants;
import android.view.KeyEvent;
import android.view.View;
import android.view.WindowManager;
import android.view.View.OnClickListener;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.TextView.OnEditorActionListener;

import android.tedongle.TedongleManager;

/**
 * Displays a PIN pad for entering a PUK (Pin Unlock Kode) provided by a carrier.
 */
public class Tedongle_sim_puk extends LinearLayout
    implements  OnEditorActionListener, TextWatcher {

    private ProgressDialog mSimUnlockProgressDialog = null;
    private volatile boolean mCheckInProgress;
    private String mPukText;
    private String mPinText;
    private StateMachine mStateMachine = new StateMachine();
    protected TextView mPasswordEntry;
    private boolean mEnableHaptics = true;
    private Context mContext = getContext();
    
    private TedongleManager mTedongleManager;
    private TextView mMessageArea;
	private static final String ACTION_SIM_PUK_OK = "tedongle.intent.action.SIM_PUK_OK";
	private static final String ACTION_SIM_PUK_WRONG = "tedongle.intent.action.SIM_PUK_WRONG";
	
    private class StateMachine {
        final int ENTER_PUK = 0;
        final int ENTER_PIN = 1;
        final int CONFIRM_PIN = 2;
        final int DONE = 3;
        private int state = ENTER_PUK;

        public void next() {
            int msg = 0;
            if (state == ENTER_PUK) {
                if (checkPuk()) {
                    state = ENTER_PIN;
                    msg = R.string.kg_puk_enter_pin_hint;
                } else {
                    msg = R.string.kg_invalid_sim_puk_hint;
                }
            } else if (state == ENTER_PIN) {
                if (checkPin()) {
                    state = CONFIRM_PIN;
                    msg = R.string.kg_enter_confirm_pin_hint;
                } else {
                    msg = R.string.kg_invalid_sim_pin_hint;
                }
            } else if (state == CONFIRM_PIN) {
                if (confirmPin()) {
                    state = DONE;
                    msg = R.string.lockscreen_sim_unlock_progress_dialog_message;
                    updateSim();
                } else {
                    state = ENTER_PIN; // try again?
                    msg = R.string.kg_invalid_confirm_pin_hint;
                }
            }
            mPasswordEntry.setText(null);
            if (msg != 0) {
            	mMessageArea.setText(msg);
            	
            }
        }

        void reset() {
            mPinText="";
            mPukText="";
            state = ENTER_PUK;
            mMessageArea.setText(R.string.kg_puk_enter_puk_hint);
            mPasswordEntry.requestFocus();
        }
    }

    public Tedongle_sim_puk(Context context) {
        this(context, null);
		mTedongleManager = TedongleManager.getDefault();
    }

    public Tedongle_sim_puk(Context context, AttributeSet attrs) {
        super(context, attrs);
		mTedongleManager = TedongleManager.getDefault();
    }

    public void resetState() {
        mStateMachine.reset();
        mPasswordEntry.setEnabled(true);
    }

    protected int getPasswordTextViewId() {
        return R.id.pinEntry;
    }

    public void doHapticKeyClick() {
        if (mEnableHaptics) {
            performHapticFeedback(HapticFeedbackConstants.VIRTUAL_KEY,
                    HapticFeedbackConstants.FLAG_IGNORE_VIEW_SETTING
                    | HapticFeedbackConstants.FLAG_IGNORE_GLOBAL_SETTING);
        }
    }
    
    @Override
    protected void onFinishInflate() {
        super.onFinishInflate();
        mPasswordEntry = (TextView) findViewById(getPasswordTextViewId());
        mPasswordEntry.setOnEditorActionListener(this);
        mPasswordEntry.addTextChangedListener(this);
        
        mMessageArea = (TextView)findViewById(R.id.input_title);
        mMessageArea.setText(R.string.tedongle_sim_puk_text);

        // Set selected property on so the view can send accessibility events.
        mPasswordEntry.setSelected(true);

        // Poke the wakelock any time the text is selected or modified
        mPasswordEntry.setOnClickListener(new OnClickListener() {
            public void onClick(View v) {

            }
        });

        mPasswordEntry.addTextChangedListener(new TextWatcher() {
            public void onTextChanged(CharSequence s, int start, int before, int count) {
            }

            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
            }

            public void afterTextChanged(Editable s) {
            }
        });
        
        final View ok = findViewById(R.id.key_enter);
        if (ok != null) {
            ok.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    doHapticKeyClick();
                    verifyPasswordAndUnlock();
                }
            });
        }

        // The delete button is of the PIN keyboard itself in some (e.g. tablet) layouts,
        // not a separate view
        View pinDelete = findViewById(R.id.delete_button);
        if (pinDelete != null) {
            pinDelete.setVisibility(View.VISIBLE);
            pinDelete.setOnClickListener(new OnClickListener() {
                public void onClick(View v) {
                    CharSequence str = mPasswordEntry.getText();
                    if (str.length() > 0) {
                        mPasswordEntry.setText(str.subSequence(0, str.length()-1));
                    }
                    doHapticKeyClick();
                }
            });
            pinDelete.setOnLongClickListener(new View.OnLongClickListener() {
                public boolean onLongClick(View v) {
                    mPasswordEntry.setText("");
                    doHapticKeyClick();
                    return true;
                }
            });
        }

        mPasswordEntry.setKeyListener(DigitsKeyListener.getInstance());
        mPasswordEntry.setInputType(InputType.TYPE_CLASS_NUMBER
                | InputType.TYPE_NUMBER_VARIATION_PASSWORD);

        mPasswordEntry.requestFocus();

        //mSecurityMessageDisplay.setTimeout(0); // don't show ownerinfo/charging status by default
    }

    public void showUsabilityHint() {
    }

    public void onPause() {
        // dismiss the dialog.
        if (mSimUnlockProgressDialog != null) {
            mSimUnlockProgressDialog.dismiss();
            mSimUnlockProgressDialog = null;
        }
    }

    /**
     * Since the IPC can block, we want to run the request in a separate thread
     * with a callback.
     */
    private abstract class CheckSimPuk extends Thread {

        private final String mPin, mPuk;

        protected CheckSimPuk(String puk, String pin) {
            mPuk = puk;
            mPin = pin;
        }

        abstract void onSimLockChangedResponse(boolean success);

        @Override
        public void run() {
            try {
            	final boolean result = mTedongleManager.supplyPuk(mPuk, mPin);
            	
                post(new Runnable() {
                    public void run() {
                        onSimLockChangedResponse(result);
                    }
                });
            } catch (Exception e) {
                post(new Runnable() {
                    public void run() {
                        onSimLockChangedResponse(false);
                    }
                });
            }
        }
    }

    private Dialog getSimUnlockProgressDialog() {
        if (mSimUnlockProgressDialog == null) {
            mSimUnlockProgressDialog = new ProgressDialog(mContext);
            mSimUnlockProgressDialog.setMessage(
                    mContext.getString(R.string.kg_sim_unlock_progress_dialog_message));
            mSimUnlockProgressDialog.setIndeterminate(true);
            mSimUnlockProgressDialog.setCancelable(false);
            if (!(mContext instanceof Activity)) {
                mSimUnlockProgressDialog.getWindow().setType(
                        WindowManager.LayoutParams.TYPE_KEYGUARD_DIALOG);
            }
        }
        return mSimUnlockProgressDialog;
    }

    private boolean checkPuk() {
        // make sure the puk is at least 8 digits long.
        if (mPasswordEntry.getText().length() >= 8) {
            mPukText = mPasswordEntry.getText().toString();
            return true;
        }
        return false;
    }

    private boolean checkPin() {
        // make sure the PIN is between 4 and 8 digits
        int length = mPasswordEntry.getText().length();
        if (length >= 4 && length <= 8) {
            mPinText = mPasswordEntry.getText().toString();
            return true;
        }
        return false;
    }

    public boolean confirmPin() {
        return mPinText.equals(mPasswordEntry.getText().toString());
    }

    private void updateSim() {
        getSimUnlockProgressDialog().show();

        if (!mCheckInProgress) {
            mCheckInProgress = true;
            new CheckSimPuk(mPukText, mPinText) {
                void onSimLockChangedResponse(final boolean success) {
                    post(new Runnable() {
                        public void run() {
                            if (mSimUnlockProgressDialog != null) {
                                mSimUnlockProgressDialog.hide();
                            }
                            if (success) {
                            	Intent intent = new Intent();
                            	intent.setAction(ACTION_SIM_PUK_OK);
                            	mContext.sendBroadcast(intent);
                            } else {
                                mStateMachine.reset();
                            	mMessageArea.setText(R.string.kg_invalid_puk);
                            }
                            mCheckInProgress = false;
                        }
                    });
                }
            }.start();
        }
    }

    protected void verifyPasswordAndUnlock() {
        mStateMachine.next();
    }

	@Override
	public void afterTextChanged(Editable s) {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void beforeTextChanged(CharSequence s, int start, int count,
			int after) {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void onTextChanged(CharSequence s, int start, int before, int count) {
		// TODO Auto-generated method stub
		
	}

	@Override
	public boolean onEditorAction(TextView v, int actionId, KeyEvent event) {
		// TODO Auto-generated method stub
		return false;
	}
	
}
