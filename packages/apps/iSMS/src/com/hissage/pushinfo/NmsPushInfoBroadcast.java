package com.hissage.pushinfo;

import com.hissage.upgrade.NmsUpgradeManager;
import com.hissage.util.log.NmsLog;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.util.Log;

public class NmsPushInfoBroadcast extends BroadcastReceiver {

    @Override
    public void onReceive(Context context, Intent intent) {
        // TODO Auto-generated method stub
        Intent realIntent = new Intent();
        realIntent.setAction("com.hissage.pushinfo.NmsPushInfoService");
        realIntent.putExtra("intentAction", intent);
        context.startService(realIntent);
    }
}
