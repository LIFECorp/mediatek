package com.mediatek.calendar.plugin;


import android.content.Context;
import android.content.res.Resources;
import android.util.Log;

import com.mediatek.calendar.ext.DefaultLunarExtension;

import com.mediatek.op02.plugin.R;

import java.util.Locale;

public class TCLunarPlugin extends DefaultLunarExtension {

    private static final String TAG = "TCLunarPlugin";

    private static String[] sSolarTermNames;
    private static String sLunarFestChunjie;
    private static String sLunarFestDuanwu;
    private static String sLunarFestZhongqiu;
    /// M: word "閏".
    private static String sLunarTextLeap;
    /// M: an index refer to word "閏".
    private static final int LUNAR_WORD_RUN = 1;

    /**
     * mContext will hold the Plugin's Context
     */
    private Context mContext;

    /**
     * M: PluginManager will instantiate this object with the context
     * of com.mediatek.op02.plugin
     * it's not an Application, so the context.getApplicationContext will
     * be null, we don't need it. we need context to access the resources
     * of com.mediatek.op02.plugin
     * @param context
     */
    public TCLunarPlugin(Context context) {
        Log.d(TAG, "in constructor");
        mContext = context;
        loadResources();
        Log.d(TAG, "load resources done");
    }

    @Override
    public String getSolarTermNameByIndex(int index) {
        if (canShowTCLunar()) {
            if (index < 1 || index > sSolarTermNames.length) {
                Log.e(TAG, "SolarTerm should between [1, 24]");
                return null;
            }
            return sSolarTermNames[index - 1];
        }

        return super.getSolarTermNameByIndex(index);
    }

    @Override
    public String getLunarFestival(int lunarMonth, int lunarDay) {
        if (canShowTCLunar()) {
            if ((lunarMonth == 1) && (lunarDay == 1)) {
                return sLunarFestChunjie;
            } else if ((lunarMonth == 5) && (lunarDay == 5)) {
                return sLunarFestDuanwu;
            } else if ((lunarMonth == 8) && (lunarDay == 15)) {
                return sLunarFestZhongqiu;
            }
        }
        return super.getLunarFestival(lunarMonth, lunarDay);
    }

    /**
     * M: Get the special word in TC mode.
     * @param index refer to the special word.
     * @return the word needed,like: LUNAR_WORD_RUN refer to "閏" in TC.
     */
    @Override
    public String getSpecialWord(int index) {
        if (canShowTCLunar()) {
            if (index == LUNAR_WORD_RUN) {
                return sLunarTextLeap;
            }
        }
        return super.getSpecialWord(index);
    }

    @Override
    public String getGregFestival(int gregorianMonth, int gregorianDay) {
        if (canShowTCLunar()) {
            return null;
        }
        return super.getGregFestival(gregorianMonth, gregorianDay);
    }

    @Override
    public boolean canShowLunarCalendar() {
        return canShowTCLunar() ? true : super.canShowLunarCalendar();
    }

    /**
     * M: whether in current env can TC Lunar be shown
     * @return traditional chinese can show TC Lunar, return true
     */
    private boolean canShowTCLunar() {
        return Locale.TRADITIONAL_CHINESE.equals(Locale.getDefault());
    }

    /**
     * M: load the Traditional Chinese resources for displaying
     */
    private void loadResources() {
        final Resources res = mContext.getResources();
        sLunarFestChunjie = res.getString(R.string.lunar_fest_chunjie);
        sSolarTermNames = res.getStringArray(R.array.sc_solar_terms);
        sLunarFestDuanwu = res.getString(R.string.lunar_fest_duanwu);
        sLunarFestZhongqiu = res.getString(R.string.lunar_fest_zhongqiu);
        sLunarTextLeap = res.getString(R.string.lunar_leap);
    }
}
