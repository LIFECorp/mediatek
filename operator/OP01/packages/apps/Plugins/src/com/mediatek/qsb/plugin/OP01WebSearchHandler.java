package com.mediatek.qsb.plugin;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.content.pm.PackageManager.NameNotFoundException;
import android.net.Uri;
import android.provider.Browser;
import android.util.Log;

import com.mediatek.qsb.ext.IWebSearchHandler;

public class OP01WebSearchHandler implements IWebSearchHandler {
    private static final String TAG = "OP01WebSearchHandlerExtension";

    public boolean handleSearchInternal(Context context, String searchEngineName, String searchUri) {
        Log.i(TAG, "handleSearchInternal in Op01 plugin");

        String cmccSearchEngineKeyWord = context.getResources().getString(
                         com.mediatek.internal.R.string.wap_browser_keyword);
        if (cmccSearchEngineKeyWord == null || !cmccSearchEngineKeyWord.equals(searchEngineName)) {
            return false;
        }
        String wapBrowserPackage = context.getResources().getString(com.mediatek.internal.R.string.wap_browser_pakcage);
        String wapBrowserComponent = context.getResources().getString(com.mediatek.internal.R.string.wap_browser_component);
        String wapBrowserUrlFlag = context.getResources().getString(com.mediatek.internal.R.string.wap_browser_url_flag);
        ComponentName componentName = new ComponentName(wapBrowserPackage, wapBrowserComponent); 
        ActivityInfo activityInfo;
        try {
            activityInfo = context.getPackageManager().getActivityInfo(componentName, 0);
        } catch (NameNotFoundException e) {
            activityInfo = null;
        }
        if (activityInfo == null) {
            Intent launchUriIntent = new Intent(Intent.ACTION_VIEW, Uri.parse(searchUri));
            launchUriIntent.putExtra(Browser.EXTRA_APPLICATION_ID, context.getPackageName());
            launchUriIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            context.startActivity(launchUriIntent);
        } else {
            Intent launchUriIntent = new Intent(Intent.ACTION_MAIN);
            launchUriIntent.setComponent(componentName);
            launchUriIntent.putExtra(wapBrowserUrlFlag, searchUri);
            context.startActivity(launchUriIntent);
        }
        return true;
    }
}
