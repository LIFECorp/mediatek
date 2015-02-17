package com.mediatek.backuprestore.modules;

import android.app.Service;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.IBinder;
import android.os.RemoteException;
import android.util.Log;

public class AidlService extends Service {
    private final String CLASS_TAG = "BackupRestore/AppRestoreComposer";

    private AidlBinder mAidlBinder;

    // Context mContext = this;

    public class AidlBinder extends AppService.Stub {
        public boolean disableApp(java.lang.String appName) throws android.os.RemoteException {
            Log.d(CLASS_TAG, "disableApp:" + appName);
            try {
                PackageManager packageManager = AidlService.this.getPackageManager();
                packageManager.setApplicationEnabledSetting(appName,
                        android.content.pm.PackageManager.COMPONENT_ENABLED_STATE_DISABLED, 0);
                Log.d(CLASS_TAG, "getPackageState:" + packageManager.getApplicationEnabledSetting(appName));
            } catch (Exception e) {
                return false;
            }

            return true;
        }

        public boolean enableApp(java.lang.String appName) throws android.os.RemoteException {
            Log.d(CLASS_TAG, "enableApp:" + appName);
            try {
                PackageManager packageManager = AidlService.this.getPackageManager();
                // packageManager.setApplicationEnabledSetting(appName,
                // android.content.pm.PackageManager.COMPONENT_ENABLED_STATE_DEFAULT,
                // 0);
                packageManager.setApplicationEnabledSetting(appName,
                        android.content.pm.PackageManager.COMPONENT_ENABLED_STATE_ENABLED, 0);
                Log.d(CLASS_TAG, "getPackageState:" + packageManager.getApplicationEnabledSetting(appName));
            } catch (Exception e) {
                return false;
            }

            return true;
        }
    }

    public final void onCreate() {
        super.onCreate();
        mAidlBinder = new AidlBinder();
    }

    public final IBinder onBind(final Intent intent) {
        return mAidlBinder;
    }

    public final void onDestroy() {
    }

}