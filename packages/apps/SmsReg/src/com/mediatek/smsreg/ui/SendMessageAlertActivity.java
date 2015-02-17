package com.mediatek.smsreg.ui;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.AlertDialog.Builder;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.DialogInterface.OnClickListener;
import android.os.Bundle;
import android.provider.Settings;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;
import android.widget.CheckBox;
import android.widget.TextView;

import com.android.internal.app.AlertActivity;
import com.android.internal.app.AlertController;
import com.mediatek.smsreg.R;
import com.mediatek.smsreg.SmsRegConst;
import com.mediatek.smsreg.SmsRegReceiver;
import com.mediatek.smsreg.SmsRegService;
import com.mediatek.xlog.Xlog;

public class SendMessageAlertActivity extends Activity {

    private static final String TAG = "SmsReg/SendMessageAlertActivity";
    private static final String ON_SECOND_DIALOG = "on_second_dialog";

    private NotificationManager mNotificationManager = null;
    private Notification mNotification = null;
    private Dialog mDialog = null;
    private boolean mIsOnSecondDiag = false;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        Xlog.d(TAG, "onCreate." );
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        mNotificationManager = (NotificationManager) this.getSystemService(Context.NOTIFICATION_SERVICE);

        mDialog = buildConfirmDialog();
        mDialog.show();
        sendNotification();
    }

    private Dialog buildConfirmDialog() {
        Log.d(TAG, "buildConfirmDialog.");

        LayoutInflater inflater = LayoutInflater.from(this);
        View layout = inflater.inflate(R.layout.notify_dialog_customview, null);

         return new AlertDialog.Builder(this)
            .setView(layout)
            .setCancelable(false)
            .setTitle(R.string.send_message_dlg_title)
            .setPositiveButton(R.string.alert_dlg_ok_button, new OnClickListener() {
                @Override
                public void onClick(DialogInterface dialog, int which) {
                    Xlog.d(TAG, "Click positive button." );
                    clear();
                    dialog.dismiss();

                    responseSendMsg(true);
                }
            })
            .setNegativeButton(R.string.alert_dlg_cancel_button, new OnClickListener() {
                @Override
                public void onClick(DialogInterface dialog, int which) {
                    Xlog.d(TAG, "Click negative button." );
                    dialog.dismiss();
                    showSecondDialog();
                }
            })
            .create();
    }

    private void showSecondDialog() {
        Xlog.i(TAG, "Current is on second dialog.");
        mIsOnSecondDiag = true;

        Dialog dialog = new AlertDialog.Builder(SendMessageAlertActivity.this)
            .setCancelable(false)
            .setTitle(R.string.confirm_dlg_title)
            .setMessage(R.string.confirm_dlg__msg)
            .setPositiveButton(R.string.alert_dlg_ok_button, new OnClickListener() {
                @Override 
                public void onClick(DialogInterface dialog, int which) {
                    Xlog.d(TAG, "Click positive button." );

                    dialog.dismiss();
                    clear();

                    responseSendMsg(false);
                }
            })
            .setNegativeButton(R.string.alert_dlg_cancel_button, new OnClickListener() {  
                @Override 
                public void onClick(DialogInterface dialog, int which) {  
                    Xlog.i(TAG, "Click negative button." );

                    dialog.dismiss();
                    clear();

                    mIsOnSecondDiag = false;

                    Intent intent = getIntent();
                    intent.setAction(SmsRegConst.ACTION_CONFIRM_DIALOG_START);
                    startActivity(intent);
                }
            })
            .create();
        dialog.show();
    }

    private void clear() {
        this.finish();
        mNotificationManager.cancel(SmsRegConst.ID_NOTIFICATION_SEND_MSG_DIALOG);
    }

    private void sendNotification() {
        mNotification = new Notification();
        mNotification.icon = R.drawable.perm_sent_mms;
        mNotification.tickerText = getResources().getString(R.string.send_message_notification_tickerText);
        mNotification.flags = Notification.FLAG_NO_CLEAR;
        mNotification.audioStreamType= android.media.AudioManager.ADJUST_LOWER;  
        Intent intent = getIntent();
        intent.setClass(this, SendMessageAlertActivity.class); 
        PendingIntent pendingIntent = PendingIntent.getActivity(this, 0, intent, PendingIntent.FLAG_UPDATE_CURRENT);  
        mNotification.setLatestEventInfo(this, 
                getResources().getString(R.string.send_message_notification_title),
                getResources().getString(R.string.send_message_notification_msg), 
                pendingIntent);
        mNotificationManager.notify(SmsRegConst.ID_NOTIFICATION_SEND_MSG_DIALOG, mNotification);
    }

    private void responseSendMsg(boolean result) {
        Xlog.d(TAG, "responseSendMsg with " + result);
        Intent intent = getIntent();
        intent.putExtra(SmsRegConst.EXTRA_IS_NEED_SEND_MSG, result);
        int slotid = intent.getIntExtra(SmsRegConst.EXTRA_SLOT_ID, -1);
        Xlog.d(TAG, " slot id is :"  + slotid);
        intent.setAction(SmsRegConst.ACTION_CONFIRM_DIALOG_END);
        intent.setClass(this, SmsRegReceiver.class);
        sendBroadcast(intent);
    }

    @Override
    protected void onRestoreInstanceState(Bundle savedInstanceState) {
        Log.i(TAG, "onRestoreInstanceState");

        if (savedInstanceState != null) {
            Log.i(TAG, "mIsOnSecondDiag is " + savedInstanceState.getBoolean(ON_SECOND_DIALOG));

            if (savedInstanceState.getBoolean(ON_SECOND_DIALOG)) {
                mDialog.dismiss();
                showSecondDialog();
            }
        }
    }

    @Override
    protected void onSaveInstanceState(Bundle outState) {
        Log.i(TAG, "onSaveInstanceState mIsOnSecondDiag is " + mIsOnSecondDiag);
        outState.putBoolean(ON_SECOND_DIALOG, mIsOnSecondDiag);
    }
}
