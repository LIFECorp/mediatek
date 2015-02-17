package com.mediatek.settings.plugin;

import android.content.Context;
import android.preference.DialogPreference;
import android.preference.Preference;
import android.preference.PreferenceScreen;
import android.test.ActivityInstrumentationTestCase2;
import android.util.AttributeSet;

import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.pluginmanager.PluginManager;
import com.mediatek.settings.ext.IApnSettingsExt;
import com.mediatek.settings.ext.IDeviceInfoSettingsExt;
import com.mediatek.settings.ext.ISettingsMiscExt;
import com.mediatek.settings.ext.ISimManagementExt;
import com.mediatek.settings.ext.IStatusGeminiExt;

public class OP02SettingsPluginTest extends ActivityInstrumentationTestCase2<MockActivity> {

    private Context mContext;
    private MockActivity mActivity;

    private IApnSettingsExt mApnSettingsExt;
    private IDeviceInfoSettingsExt mDeviceInfoSettingsExt;
    private ISettingsMiscExt mSettingsMiscExt;
    private ISimManagementExt mSimManagementExt;
    private IStatusGeminiExt mIStatusGeminiExt;

    public OP02SettingsPluginTest() {
        super(MockActivity.class);
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mContext = getInstrumentation().getContext();
        mActivity = getActivity();
        mApnSettingsExt = (IApnSettingsExt)PluginManager.createPluginObject(
                getInstrumentation().getContext(), IApnSettingsExt.class.getName());
        mDeviceInfoSettingsExt = (IDeviceInfoSettingsExt)PluginManager.createPluginObject(
                getInstrumentation().getContext(), IDeviceInfoSettingsExt.class.getName());
        mSettingsMiscExt = (ISettingsMiscExt)PluginManager.createPluginObject(
                getInstrumentation().getContext(), ISettingsMiscExt.class.getName());
        mSimManagementExt = (ISimManagementExt)PluginManager.createPluginObject(
                getInstrumentation().getContext(), ISimManagementExt.class.getName());
        mIStatusGeminiExt = (IStatusGeminiExt)PluginManager.createPluginObject(
                getInstrumentation().getContext(), IStatusGeminiExt.class.getName());
    }

    // Should disallow user edit default APN
    public void test01APNisAllowEditPresetApn() {
        String type = null;
        String apn = null;
        String numeric = "46001";
        int sourcetype = 0;
        assertTrue(!mApnSettingsExt.isAllowEditPresetApn(type, apn, numeric, sourcetype));

        numeric = "46001";
        sourcetype = 1;
        assertTrue(mApnSettingsExt.isAllowEditPresetApn(type, apn, numeric, sourcetype));

        numeric = "46000";
        sourcetype = 0;
        assertTrue(mApnSettingsExt.isAllowEditPresetApn(type, apn, numeric, sourcetype));
    }

    // Should remove device info's stats pref summary
    public void test02DeviceinfoInitSummary() {
        String summaryString = "summary";
        Preference pref = new Preference(mContext);
        pref.setSummary(summaryString);
        assertTrue(pref.getSummary().equals(summaryString));
        mDeviceInfoSettingsExt.initSummary(pref);
        assertTrue(pref.getSummary().equals(""));
    }

    public void test03SettingMiscSetTimeoutPrefTitle() {
        final DialogPreference dialogPref = new CustomDialogPreference(mContext, null);
        getInstrumentation().runOnMainSync(new Runnable() {
            public void run() {
                mActivity.getMockPreferenceGroup().addPreference(dialogPref);
            }
        });
        assertNull(dialogPref.getTitle());
        assertNull(dialogPref.getDialogTitle());
        getInstrumentation().runOnMainSync(new Runnable() {
            public void run() {
                mSettingsMiscExt.setTimeoutPrefTitle(dialogPref);
            }
        });
        assertNotNull(dialogPref.getTitle());
        assertNotNull(dialogPref.getDialogTitle());
    }

    public void test04SimManagementUpdateSimManagementPref() {
        final PreferenceScreen prefScreen = new PreferenceScreen(mContext, null);
        prefScreen.setKey("3g_service_settings");
        getInstrumentation().runOnMainSync(new Runnable() {
            public void run() {
                mActivity.getMockPreferenceGroup().addPreference(prefScreen);
            }
        });
        assertTrue(mActivity.getMockPreferenceGroup().getPreferenceCount() == 1);
        getInstrumentation().runOnMainSync(new Runnable() {
            public void run() {
                mSimManagementExt.updateSimManagementPref(mActivity.getMockPreferenceGroup());
            }
        });
        if (!FeatureOption.MTK_GEMINI_3G_SWITCH) {
            assertTrue(mActivity.getMockPreferenceGroup().getPreferenceCount() == 0);
        } else {
            assertTrue(mActivity.getMockPreferenceGroup().getPreferenceCount() == 1);
        }
    }

    public void test05StatusGeminiInitUI() {
        final PreferenceScreen prefScreen = new PreferenceScreen(mContext, null);
        final Preference pref = new Preference(mContext);
        getInstrumentation().runOnMainSync(new Runnable() {
            public void run() {
                mActivity.getMockPreferenceGroup().addPreference(prefScreen);
                prefScreen.addPreference(pref);
            }
        });
        assertTrue(prefScreen.getPreferenceCount() == 1);
        getInstrumentation().runOnMainSync(new Runnable() {
            public void run() {
                mIStatusGeminiExt.initUI(prefScreen, pref);
            }
        });
        assertTrue(prefScreen.getPreferenceCount() == 0);
    }

    public static class CustomDialogPreference extends DialogPreference {
        public CustomDialogPreference(Context context, AttributeSet attrs) {
            super(context, attrs);
        }
    }
}
