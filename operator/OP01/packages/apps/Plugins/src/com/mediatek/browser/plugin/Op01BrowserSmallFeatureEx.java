package com.mediatek.browser.plugin;

import android.content.Context;

import android.content.Intent;
import android.content.SharedPreferences;
import android.content.res.Resources;
import android.database.sqlite.SQLiteDatabase;
import android.preference.Preference;
import android.preference.Preference.OnPreferenceChangeListener;
import android.view.Menu;
import android.view.MenuInflater;
import android.webkit.WebSettings;

import com.mediatek.browser.ext.BrowserSmallFeatureEx;
import com.mediatek.browser.ext.IBrowserProvider2Ex;
import com.mediatek.op01.plugin.R;
import com.mediatek.xlog.Xlog;

public class Op01BrowserSmallFeatureEx extends BrowserSmallFeatureEx {
    
    private static final String TAG = "BrowserPluginEx";
    private static final String FILEMANAGER_EXTRA_NAME = "download path";
    private static final int RESULT_CODE_START_FILEMANAGER = 1000;
    static final String PREF_SEARCH_ENGINE = "search_engine";//BrowserSettings
    public static final String DEFAULT_DOWNLOAD_DIRECTORY_OP01 = "/storage/sdcard0/MyFavorite";  //BrowserSettings
    public static final String DEFAULT_DOWNLOAD_SDCARD2_DIRECTORY_OP01 = "/storage/sdcard1/MyFavorite";//BrowserSettings
    public static final String PREF_FONT_FAMILY = "font_family";//BrowserSettings
    public static final String SERCH_ENGIN_BAIDU = "baidu";//BrowserSettings
    public static final String PREF_LANDSCAPEONLY = "landscape_only";

    public Op01BrowserSmallFeatureEx(Context context) {
        super(context);
    }

    @Override
    public String getCustomerHomepage() {
        Xlog.i(TAG, "Enter: " + "getCustomerHomepage" + " --OP01 implement");
        return /*context.*/getResources().getString(R.string.homepage_base_site_navigation);
        
    }

    //@Override
    public void updatePreferenceItem(Preference pref, String value) {
        Xlog.i(TAG, "Enter: " + "updatePreferenceItem" + " --OP01 implement");
        if (PREF_FONT_FAMILY.equals(pref.getKey())) {    
            pref.setSummary(value);
        }
    }
    
    //@Override
    public void updatePreferenceItemAndSetListener(Preference pref, String value,
            OnPreferenceChangeListener onPreferenceChangeListener) {
        Xlog.i(TAG, "Enter: " + "updatePreferenceItemAndSetListener" + " --OP01 implement");
        pref.setOnPreferenceChangeListener(onPreferenceChangeListener);
        pref.setSummary(value);
    }
    
    //@Override
    public boolean shouldDownloadPreference() {
        Xlog.i(TAG, "Enter: " + "shouldDownloadPreference" + " --OP01 implement");
        return true;
    }
    
    //@Override
    public boolean shouldProcessResultForFileManager() {
        Xlog.i(TAG, "Enter: " + "shouldProcessResultForFileManager" + " --OP01 implement");
        return true;
    }
    
    //@Override
    public boolean shouldCheckUrlLengthLimit() {
        Xlog.i(TAG, "Enter: " + "shouldCheckUrlLengthLimit" + " --OP01 implement");
        // TODO Auto-generated method stub
        return true;
    }
    
    //@Override
    public String checkAndTrimUrl(/*Context context, */String url) {
        Xlog.i(TAG, "Enter: " + "checkAndTrimUrl" + " --OP01 implement");
        final int nLimit = /*context.*/getResources()
                .getInteger(com.mediatek.internal.R.integer.max_input_browser_search_limit);
        if (url != null && url.length() >= nLimit) {
            return url.substring(0, nLimit);
        }
        return url;
    }

    //@Override
    public String getSearchEngine(SharedPreferences mPrefs) {
        Xlog.i(TAG, "Enter: " + "getSearchEngine" + " --OP01 implement");
        return mPrefs.getString(PREF_SEARCH_ENGINE, SERCH_ENGIN_BAIDU);
    }
    
    //@Override
    public void setStandardFontFamily(WebSettings settings, String fontFamily) {
        Xlog.i(TAG, "Enter: " + "setStandardFontFamily" + " --OP01 implement");
        settings.setStandardFontFamily(fontFamily);
    }
    
    //@Override
    public boolean setIntentSearchEngineExtra(Intent intent, String string) {
        Xlog.i(TAG, "Enter: " + "setIntentSearchEngineExtra" + " --OP01 implement");
        intent.putExtra(PREF_SEARCH_ENGINE, string);
        return true;
    }
    
    //@Override
    public CharSequence[] getPredefinedWebsites() {
        Xlog.i(TAG, "Enter: " + "getPredefinedWebsites" + " --OP01 implement");
        return /*context.*/getResources().getTextArray(R.array.predefined_websites_op01);
    }
    
    //@Override
    public boolean shouldSetNavigationBarTitle() {
        Xlog.i(TAG, "Enter: " + "shouldSetNavigationBarTitle" + " --OP01 implement");
        return true;
    }
    /*
    @Override
    public boolean shouldChangeSelectionString() {
        return true;
    }
    */
    //@Override
    public boolean shouldTransferToWapBrowser() {
        Xlog.i(TAG, "Enter: " + "shouldTransferToWapBrowser" + " --OP01 implement");
        return true;
    }
    
    //@Override
    public int addDefaultBookmarksForCustomer(IBrowserProvider2Ex mBrowserProvider2, SQLiteDatabase db, long id,
            int position) {
        Xlog.i(TAG, "Enter: " + "addDefaultBookmarksForCustomer" + " --OP01 implement");
        Resources res = /*context.*/getResources();
        final CharSequence[] bookmarks = res.getTextArray(
                R.array.bookmarks_for_op01);
        int size = bookmarks.length;
        
        return mBrowserProvider2.addDefaultBookmarksHost(db, id, bookmarks, 1, position);
    }
    
    //@Override
    public boolean shouldCreateHistoryPageOptionMenu(Menu menu, MenuInflater inflater) {
        Xlog.i(TAG, "Enter: " + "createHistoryPageOptionMenu" + " --OP01 implement");
        //MenuInflater inflater1 = getMenuInflater();
        //inflater.inflate(R.menu.browser_history, menu);
        return true;
    }
    
    //@Override
    public boolean shouldCreateBookmarksOptionMenu(Menu menu, MenuInflater inflater) {
        Xlog.i(TAG, "Enter: " + "createBookmarksOptionMenu" + " --OP01 implement");
        //MenuInflater inflater1 = getMenuInflater();
        //inflater.inflate(R.menu.browser_bookmark, menu);
        return true;
    }
    
    //@Override
    public boolean shouldConfigHistoryPageMenuItem(Menu menu, boolean isNull, boolean isEmpty) {
        Xlog.i(TAG, "Enter: " + "configHistoryPageMenuItem" + " --OP01 implement");
        if (!isNull) {
            return !isEmpty;
        } else {
            //menu.findItem(R.id.clear_history_menu_id).setEnabled(false);
            return false;
        }
    }

    //@Override
    public boolean shouldChangeBookmarkMenuManner() {
        Xlog.i(TAG, "Enter: " + "shouldChangeBookmarkMenuManner" + " --OP01 implement");
        return true;
    }
    
    public String getOperatorUA(String defaultUA) {
        Xlog.i(TAG, "Enter: " + "getOperatorUA" + " --OP01 implement");
        return "MT6582_TD/V1 Linux/3.4.5 Android/4.2.2 Release/03.26.2013 " + "Browser/AppleWebKit534.30 " 
        + "Mobile Safari/534.30 MBBMS/2.2 System/Android 4.2.2";
    }
    
    public boolean shouldOnlyLandscape(SharedPreferences mPrefs) {
        Xlog.i(TAG, "Enter: " + "shouldOnlyLandscape" + " --OP01 implement");
        return mPrefs.getBoolean(PREF_LANDSCAPEONLY, false);
    }    
    public boolean shouldLoadCustomerAdvancedXml() {
        Xlog.i(TAG, "Enter: " + "shouldLoadCustomerAdvancedXml" + " --OP01 implement");
        return true;
    }
    public boolean shouldOverrideFocusContent() {
        Xlog.i(TAG, "Enter: " + "shouldOverrideFocusContext" + " --OP01 implement");
        return true;
    }
}
