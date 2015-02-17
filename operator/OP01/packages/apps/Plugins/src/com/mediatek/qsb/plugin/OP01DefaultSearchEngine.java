package com.mediatek.qsb.plugin;

import android.content.Context;
import android.content.res.Resources;

import com.mediatek.common.search.SearchEngineInfo;
import com.mediatek.common.search.ISearchEngineManager;
import com.mediatek.qsb.ext.IPreferenceSetting;

public class OP01DefaultSearchEngine implements IPreferenceSetting {
    private static final String TAG = "OP01DefaultSearchEngine";

    public static final String DEFAULT_SEARCH_ENGINE = "baidu";
    
    /// M: Return the default search engine info. @{
    public SearchEngineInfo getDefaultSearchEngine(Context context) {
        ISearchEngineManager searchEngineManager = (ISearchEngineManager) context
                .getSystemService(Context.SEARCH_ENGINE_SERVICE);
        return searchEngineManager.getSearchEngineByName(DEFAULT_SEARCH_ENGINE);
    }
    /// @}
}
