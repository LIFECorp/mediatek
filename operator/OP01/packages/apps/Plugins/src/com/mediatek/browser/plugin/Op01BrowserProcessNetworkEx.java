package com.mediatek.browser.plugin;

import android.app.ActivityManagerNative;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.net.ConnectivityManager;
import android.net.wifi.ScanResult;
import android.net.wifi.WifiManager;
import android.webkit.WebView;

import com.mediatek.browser.ext.BrowserProcessNetworkEx;
import com.mediatek.browser.ext.IBrowserControllerEx;
import com.mediatek.xlog.Xlog;

import java.util.List;

public class Op01BrowserProcessNetworkEx extends BrowserProcessNetworkEx {
    
    private static final String TAG = "BrowserPluginEx";
    private static boolean sOp01NoNetworkShouldNotify = true;
    
    @Override
    public boolean shouldProcessNetworkNotify(boolean isNetworkUp) {
        Xlog.i(TAG, "Enter: " + "shouldProcessNetworkNotify: " + isNetworkUp + " --OP01 implement");
        return sOp01NoNetworkShouldNotify;
    }
    
    @Override
    public void processNetworkNotify(WebView view, Context context, boolean isNetworkUp,
            IBrowserControllerEx mBrowserControllerEx) {
        Xlog.i(TAG, "Enter: " + "handleOp01NoNetworkNotify" + " --OP01 implement");
        
        ConnectivityManager conMgr = (ConnectivityManager)context.getSystemService(Context.CONNECTIVITY_SERVICE);
        WifiManager wifiMgr = (WifiManager) context.getSystemService(Context.WIFI_SERVICE);
        Xlog.d(TAG, "wifiMgr.isWifiEnabled(): " + wifiMgr.isWifiEnabled());
        if (wifiMgr.isWifiEnabled()) {
            //WiFi is not connected
            if (conMgr.getActiveNetworkInfo() == null || 
                    (conMgr.getActiveNetworkInfo() != null && conMgr.getActiveNetworkInfo().getType()
                            != ConnectivityManager.TYPE_WIFI)) {
                //For OP01 wlan test case 7.3.4
                List<ScanResult> list = wifiMgr.getScanResults();
                if (list != null && list.size() == 0) {
                    Xlog.d(TAG, "handleOp01NoNetworkNotify(): For OP01 wlan test case 7.3.4.");
                    SharedPreferences sh = context.getSharedPreferences("data_connection", context.MODE_WORLD_READABLE);
                    boolean needShow = !sh.getBoolean("pref_not_remind", false);
                    if (needShow) {
                        mBrowserControllerEx.showNetworkUnavilableDialog();
                        //Intent dlg = new Intent(context, DataConnectionDialog.class);
                        //context.startActivity(dlg);
                    }
                } else {
                    //For OP01 wlan test case 7.3.2
                    if (ActivityManagerNative.isSystemReady()) {
                        Xlog.d(TAG, "handleOp01NoNetworkNotify(): For OP01 wlan test case 7.3.2.");
                        Intent intent = new Intent("android.net.wifi.PICK_WIFI_NETWORK_AND_GPRS");
                        intent.putExtra("access_points_and_gprs", true);
                        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                        context.startActivity(intent);
                    } else {
                        Xlog.e(TAG, "handleOp01NoNetworkNotify(): Error, ActivityManagerNative.isSystemReady return false.");
                    }
                }
                sOp01NoNetworkShouldNotify = false;
            }
        } else {
            if (!isNetworkUp) {
                view.setNetworkAvailable(false);
                Xlog.v(TAG, "handleOp01NoNetworkNotify(): WiFi is not enabled.");
            }
        }
    }
}