package com.mediatek.configurecheck2;

import android.content.Context;

class MTKMTBFProvider extends CheckItemProviderBase {
    MTKMTBFProvider(Context c) {
         mArrayItems.add(new CheckBJTime(c, CheckItemKeySet.CI_BJ_DATA_TIME));
         mArrayItems.add(new CheckScreenOn(c, CheckItemKeySet.CI_SCREEN_ON_SLEEP));
         mArrayItems.add(new CheckScreenOn(c, CheckItemKeySet.CI_SCREEN_ON_UNLOCK));
         mArrayItems.add(new CheckScreenRotate(c, CheckItemKeySet.CI_SCREEN_ROTATE));
         mArrayItems.add(new CheckDataConnect(c, CheckItemKeySet.CI_DATA_CONNECT_ON));
         mArrayItems.add(new CheckCurAPN(c, CheckItemKeySet.CI_CMWAP));
         mArrayItems.add(new CheckWIFIControl(c, CheckItemKeySet.CI_WIFI_NEVERKEEP));
         mArrayItems.add(new CheckUrl(c, CheckItemKeySet.CI_BROWSER_URL));
         mArrayItems.add(new CheckShortcut(c, CheckItemKeySet.CI_SHORTCUT));
        // mArrayItems.add(new CheckDefaultStorage(c, CheckItemKeySet.CI_DEFAULTSTORAGE));
         mArrayItems.add(new CheckDefaultIME(c, CheckItemKeySet.CI_DEFAULTIME));
         mArrayItems.add(new CheckDefaultIME(c, CheckItemKeySet.CI_MANUL_CHECK));
         mArrayItems.add(new CheckDualSIMAsk(c, CheckItemKeySet.CI_MANUL_CHECK));
         mArrayItems.add(new CheckWebFont(c, CheckItemKeySet.CI_MANUL_CHECK));
         mArrayItems.add(new CheckLogger(c, CheckItemKeySet.CI_MANUL_CHECK));
         //mArrayItems.add(new CheckRedScreenOff(c, CheckItemKeySet.CI_MANUL_CHECK));
         mArrayItems.add(new CheckMMSSetting(c, CheckItemKeySet.CI_MANUL_CHECK));
         mArrayItems.add(new CheckEmailSetting(c, CheckItemKeySet.CI_MANUL_CHECK));
         mArrayItems.add(new CheckDefaultStorageSetting(c, CheckItemKeySet.CI_MANUL_CHECK));         
         mArrayItems.add(new CheckRedScreen(c, CheckItemKeySet.CI_DISABLED_RED_SCREEN));
         mArrayItems.add(new CheckAddCallLog(c, CheckItemKeySet.CI_ADD_Mo_CALL_LOG));
    }
}