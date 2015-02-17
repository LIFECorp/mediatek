package com.mediatek.teledongledemo;

import java.net.InetAddress;
import java.util.ArrayList;
import java.util.Collection;

//import org.apache.http.conn.routing.RouteInfo;

import android.app.Activity;
import android.app.ProgressDialog;
import android.content.Context;
import android.net.ConnectivityManager;
import android.os.Bundle;
//import andorid.tedongle.TedongleManager;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.content.BroadcastReceiver;
import android.util.Log;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Handler;
import android.text.TextUtils;
import android.content.res.Resources;
import android.net.LinkAddress;
import android.net.LinkProperties;
import android.net.RouteInfo;

import android.tedongle.TedongleManager;
import com.android.internal.tedongle.ITedongleStateListener;

public class TedongleInfo extends PreferenceActivity {
	private final String LOG_TAG = "3GD-APK-TedongleInfo";
    //private TeledongleManager mTeledongleManager;
	private boolean mIsAirplneMode = false;
    private static Resources sRes;
    private ConnectivityManager mConnService;
    private boolean mRadioEnable = false;
    private LinkProperties mLinkProperties;
    private TedongleManager mTedongleManager;
    
    private Preference mIp = findPreference(KEY_IP);
    private Preference mInterfaceName = findPreference(KEY_INTERFACENAME);
    private Preference mType = findPreference(KEY_TYPE);
    private Preference mDnses = findPreference(KEY_DNSES);
    private Preference mRoute = findPreference(KEY_ROUTE);
	
    String mIfaceName;
    private Collection<LinkAddress> mLinkAddresses = new ArrayList<LinkAddress>();
    private Collection<InetAddress> mDnsesAddr = new ArrayList<InetAddress>();
    private Collection<RouteInfo> mRoutes = new ArrayList<RouteInfo>();
    
    private static final String KEY_IP = "ip";
    private static final String KEY_INTERFACENAME = "ifname";
    private static final String KEY_TYPE = "type";
    private static final String KEY_DNSES = "dnses";
    private static final String KEY_ROUTE = "route";
    
    public static final int TYPE_TEDONGLE = 30;
    
    public static final int DATA_UNKNOWN        = -1;
    /** Data connection state: Disconnected. IP traffic not available. */
    public static final int DATA_DISCONNECTED   = 0;
    /** Data connection state: Currently setting up a data connection. */
    public static final int DATA_CONNECTING     = 1;
    /** Data connection state: Connected. IP traffic should be available. */
    public static final int DATA_CONNECTED      = 2;
    /** Data connection state: Suspended. The connection is up, but IP
     * traffic is temporarily unavailable. For example, in a 2G network,
     * data activity may be suspended when a voice call arrives. */
    public static final int DATA_SUSPENDED      = 3;
	
     private BroadcastReceiver mTedongleReceiver = new BroadcastReceiver() {
    	@Override
        public void onReceive(Context context, Intent intent) {
   
            String action = intent.getAction();
            Log.d(LOG_TAG, "TedongleReceiver -----action=" + action);
            
            if (action.equals(Intent.ACTION_AIRPLANE_MODE_CHANGED)) {
                
                boolean enabled = intent.getBooleanExtra("state", false);
                Log.d(LOG_TAG, "TedongleReceiver ------ AIRPLANEMODE enabled=" + enabled);
                mIsAirplneMode = enabled;
                //mTeledongleManager.setRadio(!enabled);
            }
        }
    };
    
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		//mTedongleManager = new TedongleManager(this);
		mTedongleManager = TedongleManager.getDefault();
		
		addPreferencesFromResource(R.xml.dongle_network_info);
        sRes = getResources();
        ///M: initilize connectivity manager for consistent_UI
        mConnService = (ConnectivityManager) getSystemService(Context.CONNECTIVITY_SERVICE);
        mRadioEnable = (boolean)getIntent().getBooleanExtra("radioState", false);
        initPreference();
        Log.d(LOG_TAG, "oncreate end...");
	}
	
	public void initPreference() {
		//according the value of mRadioEnable
        Log.d(LOG_TAG, "initPreference mRadioEnable:" + mRadioEnable);
		if (mRadioEnable == false) {
	        String display = sRes.getString(R.string.unknown);
	    	setSummaryText(KEY_INTERFACENAME,display);
	    	setSummaryText(KEY_IP,display);
	    	setSummaryText(KEY_DNSES,display);
	    	setSummaryText(KEY_ROUTE, display);
	    	setSummaryText(KEY_TYPE, display);
		} else {
			updateNetworkInfo();
		}
	}
	
    @Override
    protected void onDestroy() {
        super.onDestroy();
    }

    @Override
    protected void onResume() {
        super.onResume();
        IntentFilter intentFilter = new IntentFilter(
                Intent.ACTION_AIRPLANE_MODE_CHANGED);
        registerReceiver(mTedongleReceiver, intentFilter);
        updateNetworkInfo();
        
    }

    private void setSummaryText(String preference, String text) {
        if (TextUtils.isEmpty(text)) {
            text = this.getResources().getString(R.string.device_info_default);
        }
        // some preferences may be missing
        Preference p = findPreference(preference);
        if (p != null) {
            p.setSummary(text);
        }
    }
    
    @Override
    public void onPause() {
        super.onPause();
        unregisterReceiver(mTedongleReceiver);
    }

    private void updateNetworkInfo() {
    	//mLinkProperties = mConnService.getActiveLinkProperties();
    	mLinkProperties = mConnService.getLinkProperties(TYPE_TEDONGLE);
    	if (mLinkProperties != null) {
    	//interface name
        Log.d(LOG_TAG, "updateNetworkInfo12 " );
    	mIfaceName = mLinkProperties.getInterfaceName();
    	String ifaceName = (mIfaceName == null ? "not available" : mIfaceName );
    	setSummaryText(KEY_INTERFACENAME,ifaceName);
    	//ip
    	mLinkAddresses = mLinkProperties.getLinkAddresses();
        String linkAddresses = "";
        for (LinkAddress addr : mLinkAddresses) linkAddresses += addr.toString() + ",";
    	setSummaryText(KEY_IP,linkAddresses);
    	//dns
    	mDnsesAddr = mLinkProperties.getDnses();
    	String dns = "";
    	for (InetAddress addr : mDnsesAddr) dns += addr.getHostAddress() + ",";
    	setSummaryText(KEY_DNSES,linkAddresses);
    	//gateway
    	mRoutes = mLinkProperties.getRoutes();
    	String routes = "";
    	for (RouteInfo route : mRoutes) routes += route.toString() + ",";
    	setSummaryText(KEY_ROUTE, routes);
    	//network type
    	String mNetworkType = mTedongleManager.getNetworkTypeName();
    	setSummaryText(KEY_TYPE, mNetworkType);

    } else {
        Log.d(LOG_TAG, "updateNetworkInfo1 " );
        String display = sRes.getString(R.string.unknown);
    	setSummaryText(KEY_INTERFACENAME,display);
    	setSummaryText(KEY_IP,display);
    	setSummaryText(KEY_DNSES,display);
    	setSummaryText(KEY_ROUTE, display);
    	setSummaryText(KEY_TYPE, display);
    }
    }
    
    
}
