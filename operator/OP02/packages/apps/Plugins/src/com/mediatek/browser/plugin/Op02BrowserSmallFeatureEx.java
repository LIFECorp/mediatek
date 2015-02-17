package com.mediatek.browser.plugin;

import android.content.Context;
import android.content.res.Resources;
import android.database.sqlite.SQLiteDatabase;
import android.os.Build;
import android.preference.ListPreference;
import android.preference.Preference;
import android.preference.Preference.OnPreferenceChangeListener;

import com.mediatek.browser.ext.BrowserSmallFeatureEx;
import com.mediatek.browser.ext.IBrowserProvider2Ex;
import com.mediatek.op02.plugin.R;
import com.mediatek.xlog.Xlog;

import java.text.SimpleDateFormat;
import java.util.Date;

public class Op02BrowserSmallFeatureEx extends BrowserSmallFeatureEx {
    
    private static final String TAG = "BrowserPluginEx";
    
    
    public Op02BrowserSmallFeatureEx(Context context) {
        super(context);
    }
    
    @Override
    public void updatePreferenceItem(Preference pref, String value) {
        Xlog.i(TAG, "Enter: " + "updatePreferenceItem" + " --OP02 implement");
        return;
    }
    
    @Override
    public void updatePreferenceItemAndSetListener(Preference pref, String value,
            OnPreferenceChangeListener onPreferenceChangeListener) {
        Xlog.i(TAG, "Enter: " + "updatePreferenceItemAndSetListener" + " --OP02 implement");
        return;
    }

    @Override
    public void setTextEncodingChoices(ListPreference e) {
        Xlog.i(TAG, "Enter: " + "setTextEncodingChoices" + " --OP02 implement");
        e.setEntries(getResources().getTextArray(R.array.pref_op02_text_encoding_choices));
        e.setEntryValues(getResources().getTextArray(R.array.pref_op02_text_encoding_values));
        
    }
    
    @Override
    public String getCustomerHomepage() {
        Xlog.i(TAG, "Enter: " + "getCustomerHomepage" + " --OP02 implement");
        return /*context.*/getResources().getString(R.string.homepage_for_op02);
        
    }
    
    //@Override
    public int addDefaultBookmarksForCustomer(IBrowserProvider2Ex mBrowserProvider2, SQLiteDatabase db, long id,
            int position) {
        Xlog.i(TAG, "Enter: " + "addDefaultBookmarksForCustomer" + " --OP02 implement");
        Resources res = /*context.*/getResources();
        final CharSequence[] bookmarks = res.getTextArray(
                R.array.bookmarks_for_op02);
        int size = bookmarks.length;
        //TypedArray preloads = res.obtainTypedArray(R.array.bookmark_preloads_for_op02);
        return mBrowserProvider2.addDefaultBookmarksHost(db, id, bookmarks, 2, position);
    }
    
    public String getOperatorUA(String defaultUA) {
        Xlog.i(TAG, "Enter: " + "getOperatorUA" + " --OP02 implement");

        Date date = new Date(Build.TIME);
        String strTime = new SimpleDateFormat("MM.dd.yyyy").format(date);
        String model = "MT6582/V1";

        String op02UA = model + " Linux/3.4.67 Android/" + Build.VERSION.RELEASE
                + " Release/" + strTime + " Browser/AppleWebKit537.36 Profile/MIDP-2.0 Configuration/CLDC-1.1"
                + " Chrome/30.0.0.0 Mobile Safari/537.36 System/Android " + Build.VERSION.RELEASE + ";";
        return op02UA;
    }
}