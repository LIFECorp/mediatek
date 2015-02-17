package com.hissage.pushinfo;

import java.text.SimpleDateFormat;
import java.util.Date;

import com.hissage.db.NmsDBOpenHelper;
import com.hissage.db.NmsDBUtils;
import com.hissage.message.ip.NmsIpMessageConsts;
import com.hissage.service.NmsService;
import com.hissage.upgrade.NmsUpgradeManager;
import com.hissage.util.log.NmsLog;

import android.app.Service;
import android.content.ContentValues;
import android.content.Intent;
import android.net.Uri;
import android.os.IBinder;
import android.util.Log;

/*****
 * 
 * @author hst00105
 * 
 */
public class NmsPushInfoService extends Service {

    @Override
    public void onCreate() {
        // TODO Auto-generated method stub
        super.onCreate();
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {

        NmsLog.trace(NmsUpgradeManager.TAGPUSH, " NmsPushInfoService  onStartCommand intent:"
                + intent);
        if (intent == null) {
            return super.onStartCommand(intent, flags, startId);
        }
        Intent intentAction = intent.getParcelableExtra("intentAction");

        String pushInfoId = intentAction.getStringExtra("pushInfoId");
        int type = intentAction.getIntExtra("pushInfoType", -1);

        int pushMsgType = intentAction.getIntExtra("pushMsgType", -1);
        NmsLog.trace(NmsUpgradeManager.TAGPUSH, " NmsPushInfoService pushMsgType:." + pushMsgType
                + " pushInfoId" + pushInfoId);

        if (pushMsgType == 0) {
            Intent intentUrl = new Intent();
            intentUrl.setAction(NmsIpMessageConsts.ACTION_TERM);
            intentUrl.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            startActivity(intentUrl);

        } else if (pushMsgType == 1) {
            String url = intentAction.getStringExtra("url");
            NmsLog.trace(NmsUpgradeManager.TAGPUSH, " NmsPushInfoBroadcast  url " + url);
            if (url == null) {
                return super.onStartCommand(intent, flags, startId);
            }
            Intent intentUrl = new Intent();
            intentUrl.setAction("android.intent.action.VIEW");
            intentUrl.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            Uri content_url = Uri.parse(url);
            intentUrl.setData(content_url);
            startActivity(intentUrl);

        } else {
            return -1;
        }

        ContentValues values = new ContentValues();
        SimpleDateFormat dateformat = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
        values.put(NmsDBOpenHelper.PUSH_INFO_OPRATERTIME,
                dateformat.format(new Date(System.currentTimeMillis())).toString());
        values.put(NmsDBOpenHelper.PUSH_INFO_STATUS, NmsPushInfo.PUSH_INFO_STATUS_CLICKED);

        NmsDBUtils.getDataBaseInstance(NmsService.getInstance()).updatePushInfoToDB(values,
                pushInfoId);

        return super.onStartCommand(intent, flags, startId);
    }

    @Override
    public IBinder onBind(Intent intent) {
        // TODO Auto-generated method stub
        return null;
    }

}
