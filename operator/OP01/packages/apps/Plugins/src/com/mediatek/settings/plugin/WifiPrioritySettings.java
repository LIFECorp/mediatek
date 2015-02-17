package com.mediatek.op01.plugin;

import android.content.Context;
import android.net.wifi.WifiConfiguration;
import android.net.wifi.WifiManager;
import android.os.Bundle;
import android.preference.ListPreference;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceCategory;
import android.preference.PreferenceFragment;
import android.preference.Preference.OnPreferenceChangeListener;
import android.widget.Toast;

import com.mediatek.op01.plugin.R;
import com.mediatek.xlog.Xlog;

import java.util.List;

public class WifiPrioritySettings  extends PreferenceActivity implements OnPreferenceChangeListener {
    private static final String TAG = "WifiPrioritySettings";
    static final String CMCC_SSID = "CMCC";
    static final String CMCC_AUTO_SSID = "CMCC-AUTO";
    private WifiManager mWifiManager;
    PreferenceCategory mConfiguredAps;
    private int[] mPriorityOrder;
    private List<WifiConfiguration> mConfigs;
    private int configuredApCount;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        
        addPreferencesFromResource(R.layout.wifi_priority_settings);
        mConfiguredAps = (PreferenceCategory)findPreference("configured_ap_list");
        
        mWifiManager = (WifiManager) getSystemService(Context.WIFI_SERVICE);
        initPage();
    }
    
    /**
     * init page, give each access point a right priority order
     */
    public void initPage() {
        if (mWifiManager==null) {
            Xlog.e(TAG, "Fail to get Wifi Manager service");
            return;
        }
        
        mConfigs = mWifiManager.getConfiguredNetworks();
        if (mConfigs != null && mConfiguredAps!=null) {
            mConfiguredAps.removeAll();
            
            configuredApCount = mConfigs.size();
            String[] priorityEntries = new String[configuredApCount];
//            String[] priorityValues = new String[configuredApCount];
            for (int i = 0;i < configuredApCount;i++) {
                priorityEntries[i]=String.valueOf(i+1);
//                priorityValues[i]=String.valueOf(configuredApCount-i);
            }
            
            for (int i = 0;i < configuredApCount;i++) {
                Xlog.e(TAG, "Before sorting: priority array=" + mConfigs.get(i).priority);
            }
            //get the correct priority order for each ap
            mPriorityOrder = calculateInitPriority(mConfigs);
            
            String summaryPreStr = getResources().getString(R.string.wifi_priority_label) + ": ";
            //initiate page with list preference
            for (int i = 0;i < configuredApCount;i++) {
                Xlog.e(TAG, "After sorting: order array=" + mPriorityOrder[i]);
                WifiConfiguration config = mConfigs.get(i);
                if (config.priority != configuredApCount-mPriorityOrder[i] + 1) {
                    config.priority=configuredApCount-mPriorityOrder[i] + 1;
                    updateConfig(config);
                }
                
                //add a list Preference to this page
                //String ssidStr = removeDoubleQuotes(config.SSID);
                String ssidStr = (config.SSID == null ? "" : removeDoubleQuotes(config.SSID));
                ListPreference pref = new ListPreference(this);
                pref.setOnPreferenceChangeListener(this);
                pref.setTitle(ssidStr);
                pref.setDialogTitle(ssidStr);
                pref.setSummary(summaryPreStr + mPriorityOrder[i]);
                pref.setEntries(priorityEntries);
                pref.setEntryValues(priorityEntries);
                pref.setValueIndex(mPriorityOrder[i]-1);
                mConfiguredAps.addPreference(pref);
            }
            mWifiManager.saveConfiguration();
        }
    }
    
    /**
     * calculate priority order of input ap list, each ap's right order is stored in a int array
     * @param configs
     * @return
     */
    public int[] calculateInitPriority(List<WifiConfiguration> configs) {
        if (configs == null) {
            return null;
        }
        for (WifiConfiguration config:configs) {
            if (config==null) {
                config = new WifiConfiguration();
                config.SSID="ERROR";
                config.priority=0;
            }
        }
        
        int totalSize = configs.size();
        int[] result = new int[totalSize];
        for (int i = 0;i < totalSize;i++) {
            int biggestPoz = 0;
            for (int j = 1;j < totalSize;j++) {
                if (!formerHasHigherPriority(configs.get(biggestPoz),configs.get(j))) {
                    biggestPoz=j;
                }
            }
            //this is the [i+1] biggest one, so give such order to it
            result[biggestPoz]=i + 1;
            configs.get(biggestPoz).priority = -1;//don't count this one in any more
        }
        return result;
    }
    
    /**
     * compare priority of two AP
     * @param former
     * @param backer
     * @return true if former one has higher priority, otherwise return false
     */
    private boolean formerHasHigherPriority(WifiConfiguration former, WifiConfiguration backer) {
        if (former==null) {
            return false;
        } else if (backer==null) {
            return true;
        } else {
            if (former.priority>backer.priority) {
                return true;
            } else if (former.priority<backer.priority) {
                return false;
            } else {//have the same priority, so default trusted AP go first
                String formerSSID = (former.SSID == null ? "" : removeDoubleQuotes(former.SSID));
                String backerSSID = (backer.SSID == null ? "" : removeDoubleQuotes(backer.SSID));
                /*when same priority,CMCC_AUTO > CMCC > CMCC_EDU > other*/
                
                if(CMCC_AUTO_SSID.equals(formerSSID))
                {
                    Xlog.d(TAG, "WifiSettingsExt formerHasHigherPriority() same true");
                    return true;
                }
                else if(CMCC_SSID.equals(formerSSID))
                {
                    if(!CMCC_AUTO_SSID.equals(backerSSID))
                    {
                        Xlog.d(TAG, "WifiSettingsExt formerHasHigherPriority() same true");
                        return true;
                    }
                    else
                    {
                        Xlog.d(TAG, "WifiSettingsExt formerHasHigherPriority() same false");
                        return false;
                    }
                    
                }
                else
                {
                    if(!CMCC_SSID.equals(backerSSID)
                            && !CMCC_AUTO_SSID.equals(backerSSID))
                    {
                        return formerSSID.compareTo(backerSSID) <= 0;
                    }
                    else
                    {
                        Xlog.d(TAG, "WifiSettingsExt formerHasHigherPriority() same false");
                        return false;
                    }
                }


                /*
                if (!CMCC_SSID.equals(backerSSID) && !CMCC_EDU_SSID.equals(backerSSID)){
                    return true;
                } else {
                    if (!CMCC_SSID.equals(formerSSID) && !CMCC_EDU_SSID.equals(formerSSID)){
                        return false;
                    } else {
                        return formerSSID.compareTo(backerSSID)<=0;
                    }
                }
                */
            }
        }
    }
    

    public boolean onPreferenceChange(Preference preference, Object newValue) {
        if (preference instanceof ListPreference) {
            ListPreference pref = (ListPreference)preference;
            int oldOrder=0;
            int newOrder=0;
            try {
                oldOrder = Integer.parseInt(pref.getValue());
                newOrder = Integer.parseInt((String)newValue);
            } catch (NumberFormatException e) {
                Xlog.e(TAG, "Error happens when modify priority manually");
                e.printStackTrace();
            }
            Xlog.e(TAG, "Priority old value=" + oldOrder + ", new value=" + newOrder);
            Toast.makeText(this, getString(R.string.wifi_priority_changed,pref.getValue(),(String)newValue), Toast.LENGTH_SHORT).show();
            
            //this is priority order, bigger order, smaller priority
            if (oldOrder!=newOrder && mPriorityOrder!=null) {
                if (oldOrder>newOrder){
                    //selected AP will have a higher priority, but smaller order
                    for (int i = 0;i < mPriorityOrder.length;i++) {
                        WifiConfiguration config = mConfigs.get(i);
                        if (mPriorityOrder[i] >= newOrder && mPriorityOrder[i] < oldOrder) {
                            mPriorityOrder[i]++;
                            config.priority = configuredApCount-mPriorityOrder[i] + 1;
                            updateConfig(config);
                        } else if (mPriorityOrder[i] == oldOrder) {
                            mPriorityOrder[i] = newOrder;
                            config.priority = configuredApCount - newOrder + 1;
                            updateConfig(config);
                        }
                    }
                } else {
                  //selected AP will have a lower priority, but bigger order
                    for (int i = 0;i < mPriorityOrder.length;i++) {
                        WifiConfiguration config = mConfigs.get(i);
                        if (mPriorityOrder[i] <= newOrder && mPriorityOrder[i] > oldOrder) {
                            mPriorityOrder[i]--;
                            config.priority = configuredApCount-mPriorityOrder[i] + 1;
                            updateConfig(config);
                        }else if (mPriorityOrder[i] == oldOrder) {
                            mPriorityOrder[i] = newOrder;
                            config.priority = configuredApCount - newOrder + 1; 
                            updateConfig(config);
                        }
                    }
                }
                mWifiManager.saveConfiguration();
                
                updateUI();
            }
        }
        return true;
    }
    
    /**
     * update each list view according to configure order array
     */
    public void updateUI() {
        for (int i = 0;i < mPriorityOrder.length;i++) {
            Preference pref = mConfiguredAps.getPreference(i);
            if (pref!=null){
                String summaryPreStr = getResources().getString(R.string.wifi_priority_label) + ": ";
                pref.setSummary(summaryPreStr + mPriorityOrder[i]);
            }
            if (pref instanceof ListPreference) {
                ((ListPreference)pref).setValue(String.valueOf(mPriorityOrder[i]));
            }
            
        }
    }
    
    private void updateConfig(WifiConfiguration config) {
        Xlog.e(TAG, "updateConfig()");
        if (config==null) {
            return;
        }
        WifiConfiguration newConfig = new WifiConfiguration();
        newConfig.networkId = config.networkId;
        newConfig.priority = config.priority;
        mWifiManager.updateNetwork(newConfig);
    }
    private String removeDoubleQuotes(String string) {
        int length = string.length();
        if ((length > 1) && (string.charAt(0) == '"')
                && (string.charAt(length - 1) == '"')) {
            return string.substring(1, length - 1);
        }
        return string;
    }
}
