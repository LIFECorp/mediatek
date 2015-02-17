package com.mediatek.teledongledemo;

import android.tedongle.TedongleManager;

import android.content.Context;
import android.content.Intent;
import android.app.Activity;
import android.app.Dialog;
import android.app.ProgressDialog;
import android.os.RemoteException;
//import android.os.ServiceManager;
import android.text.Editable;
import android.text.InputType;
import android.text.TextWatcher;
import android.text.method.DigitsKeyListener;
import android.util.AttributeSet;
import android.view.HapticFeedbackConstants;
import android.view.KeyEvent;
import android.view.View;
import android.view.WindowManager;
import android.widget.TextView.OnEditorActionListener;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.Toast;

/**
 * Displays a PIN pad for unlocking.
 */
public class Tedongle_sim_pin extends LinearLayout
        implements  OnEditorActionListener, TextWatcher {

    private ProgressDialog mSimUnlockProgressDialog = null;
    private volatile boolean mSimCheckInProgress;
    protected TextView mPasswordEntry;
    private boolean mEnableHaptics = true;
    private Context mContext = getContext();
    private TedongleManager mTedongleManager;
    private TextView mMessageArea;
	private static final String ACTION_SIM_PIN_OK = "tedongle.intent.action.SIM_PIN_OK";
	private static final String ACTION_SIM_PIN_WRONG = "tedongle.intent.action.SIM_PIN_WRONG";

    public Tedongle_sim_pin(Context context) {
        this(context, null);
		mTedongleManager = TedongleManager.getDefault();
    }

    public Tedongle_sim_pin(Context context, AttributeSet attrs) {
        super(context, attrs);
		mTedongleManager = TedongleManager.getDefault();
    }

    public void resetState() {
    	mMessageArea.setText(R.string.kg_sim_pin_instructions);
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
        mMessageArea.setText(R.string.tedongle_sim_pin_text);

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
    private abstract class CheckSimPin extends Thread {
        private final String mPin;

        protected CheckSimPin(String pin) {
            mPin = pin;
        }

        abstract void onSimCheckResponse(boolean success);

        @Override
        public void run() {
            try {
            	final boolean result = mTedongleManager.supplyPin(mPin);
                post(new Runnable() {
                    public void run() {
                        onSimCheckResponse(result);
                    }
                });
            } catch (Exception e) {
                post(new Runnable() {
                    public void run() {
                        onSimCheckResponse(false);
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


    protected void verifyPasswordAndUnlock() {
        String entry = mPasswordEntry.getText().toString();
        
        if (entry.length() < 4) {
            // otherwise, display a message to the user, and don't submit.
        	mMessageArea.setText(R.string.kg_invalid_sim_pin_hint);
            mPasswordEntry.setText("");
            return;
        }

        getSimUnlockProgressDialog().show();

        if (!mSimCheckInProgress) {
            mSimCheckInProgress = true; // there should be only one
            new CheckSimPin(mPasswordEntry.getText().toString()) {
                void onSimCheckResponse(final boolean success) {
                    post(new Runnable() {
                        public void run() {
                            if (mSimUnlockProgressDialog != null) {
                                mSimUnlockProgressDialog.hide();
                            }
							if (success) {
							// before closing the screen, report back that the sim is unlocked
							// so it knows right away.
							//using broadcast???
								Intent intent = new Intent();
								intent.setAction(ACTION_SIM_PIN_OK);
								mContext.sendBroadcast(intent);
							} else {
								mMessageArea.setText(R.string.kg_password_wrong_pin_code);
								mPasswordEntry.setText("");
								Intent intent = new Intent();
								intent.setAction(ACTION_SIM_PIN_WRONG);
								mContext.sendBroadcast(intent);
							}

                            mSimCheckInProgress = false;
                        }
                    });
                }
            }.start();
        }
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
