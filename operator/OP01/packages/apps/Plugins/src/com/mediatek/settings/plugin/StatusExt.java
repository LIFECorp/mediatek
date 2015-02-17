package com.mediatek.settings.plugin;

import android.content.IntentFilter;
import android.preference.Preference;

import com.mediatek.settings.ext.DefaultStatusExt;

public class StatusExt extends DefaultStatusExt {
    
    public void updateOpNameFromRec(Preference p, String name) {
        p.setSummary(name);
    }
    
    public void updateServiceState(Preference p, String name) {
        
    }
    public void addAction(IntentFilter intent, String action) {
        intent.addAction(action);
    }
}
