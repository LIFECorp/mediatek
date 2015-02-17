package com.mediatek.downloadmanager.plugin;

import android.content.ContentUris;
import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.provider.Downloads;

import com.mediatek.downloadmanager.ext.DownloadProviderFeatureEx;
import com.mediatek.op01.plugin.R;
import com.mediatek.xlog.Xlog;

import org.apache.http.params.HttpConnectionParams;
import org.apache.http.params.HttpParams;

import java.util.HashSet;

public class Op01DownloadProviderFeatureEx extends DownloadProviderFeatureEx {
    
    private static final String TAG = "DownloadProviderPluginEx";
    
    public static final String BROWSER_NOTIFICATION_PACKAGE = 
        "com.android.browser";        //Constants
    
    public static final String OMADL_NOTIFICATION_PACKAGE =
        "com.android.providers.downloads.ui";        //Constants
    
    public Op01DownloadProviderFeatureEx(Context context) {
        super(context);
    }
    
    @Override
    public void addAppReadableColumnsSet(
            HashSet<String> sAppReadableColumnsSet, String column) {
        Xlog.i(TAG, "Enter: " + "addAppReadableColumnsSet" + " --OP01 implement");
        // TODO Auto-generated method stub
        sAppReadableColumnsSet.add(column);
    }
    
    @Override
    public String getNotificationText(String packageName,
            String mimeType, String fullFileName) {
        //
        Xlog.i(TAG, "Enter: " + "getNotificationText" + " --OP01 implement");
        // TODO Auto-generated method stub
        String  caption = "";
        if ((packageName.equals(BROWSER_NOTIFICATION_PACKAGE)
                || packageName.equals(OMADL_NOTIFICATION_PACKAGE))
                && !mimeType.equalsIgnoreCase("application/vnd.oma.drm.message")
                && !mimeType.equalsIgnoreCase("application/vnd.oma.drm.content")
                && !mimeType.equalsIgnoreCase("application/vnd.oma.drm.rights+wbxml")
                && !mimeType.equalsIgnoreCase("application/vnd.oma.drm.rights+xml")) {
          caption = getResources().getString(R.string.notification_download_complete_op01);
          caption = caption + fullFileName;
          return caption;
        } else {
            caption = getResources().getString(R.string.notification_download_complete);
            return caption;
        }
    }
    
    @Override
    public boolean shouldSetDownloadPath() {
        //
        Xlog.i(TAG, "Enter: " + "shouldSetDownloadPath" + " --OP01 implement");
        return true;
    }
    
    @Override
    public void setHttpSocketTimeOut(HttpParams params, int timeout) {
        //
        Xlog.i(TAG, "Enter: " + "setHttpSocketTimeOut" + " --OP01 implement");
        HttpConnectionParams.setSoTimeout(params, 60 * 1000);
    }
    
    @Override
    public boolean shouldSetDownloadPathSelectFileMager() {
        Xlog.i(TAG, "Enter: " + "SetDownloadPathSelectFileMager" + " --OP01 implement");
        return true;
    }
    
    @Override
    public void copyContentValues(String key, ContentValues from, ContentValues to) {
        //
        Xlog.i(TAG, "Enter: " + "copyContentValues" + " --OP01 implement");
        String s = from.getAsString(key);
        if (s != null) {
            to.put(key, s);
        }
    }
}
