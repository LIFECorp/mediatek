/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

package com.mediatek.settings.plugin;

import android.app.ActionBar;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.ContentQueryMap;
import android.content.ContentResolver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;

import android.database.Cursor;
import android.location.LocationManager;
import android.os.Bundle;
import android.os.SystemProperties;
import android.preference.CheckBoxPreference;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceScreen;
import android.preference.PreferenceFragment;

import android.provider.Settings;
import android.view.Gravity;
import android.widget.CompoundButton;
import android.widget.Switch;

import com.mediatek.op01.plugin.R;

import com.mediatek.common.agps.MtkAgpsManager;
import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.xlog.Xlog;

import java.util.Observable;
import java.util.Observer;

public class AgpsSettingEnter extends PreferenceActivity implements
        CompoundButton.OnCheckedChangeListener {

    private static final String TAG = "AgpsEpoSettingEnter";


    // / M: Agps enable and agps settings preference key
    private static final String KEY_AGPS_ENABLER = "agps_enabler";
    private static final String KEY_AGPS_SETTINGS = "agps_settings";

    // / M: Agps enable and agps settings preference
    private MtkAgpsManager mAgpsMgr;
    private CheckBoxPreference mAgpsCB;
    private Preference mAgpsPref;
    private Switch mEnabledSwitch;

    private static final int CONFIRM_AGPS_DIALOG_ID = 1;

    // These provide support for receiving notification when Location Manager
    // settings change.
    // This is necessary because the Network Location Provider can change
    // settings
    // if the user does not confirm enabling the provider.
    private ContentQueryMap mContentQueryMap;

    private Observer mSettingsObserver;
    private Cursor mSettingsCursor;

    private ContentResolver mContentResolver;

    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        mContentResolver = getContentResolver();
        // Make sure we reload the preference hierarchy since some of these
        // settings
        // depend on others...
        createPreferenceHierarchy();
        if(!(("tablet".equals(SystemProperties.get("ro.build.characteristics"))) &&
	   (getResources().getBoolean(com.android.internal.R.bool.preferences_prefer_dual_pane))))
        {
            getActionBar().setTitle(R.string.gps_settings_title);
        }
        //from fragment onActivityCreated
        /// M: for using MTK style Switch , text is I/O , not On/Off {@
        //mEnabledSwitch= (Switch)getLayoutInflater()
        //                               .inflate(com.mediatek.internal.R.layout.imageswitch_layout,null);
        /// @}

        boolean gpsEnabled = Settings.Secure.isLocationProviderEnabled(
                mContentResolver, LocationManager.GPS_PROVIDER);
        //mEnabledSwitch.setChecked(gpsEnabled);

        final int padding = getResources().getDimensionPixelSize(
                R.dimen.action_bar_switch_padding);
        //mEnabledSwitch.setPaddingRelative(0, 0, padding, 0);
        //mEnabledSwitch.setOnCheckedChangeListener(this);
    }
/*
    @Override
    public void onActivityCreated(Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);


        /// M: for using MTK style Switch , text is I/O , not On/Off {@
        mEnabledSwitch= (Switch)getLayoutInflater()
                                       .inflate(com.mediatek.internal.R.layout.imageswitch_layout,null);
        /// @}

        boolean gpsEnabled = Settings.Secure.isLocationProviderEnabled(
                mContentResolver, LocationManager.GPS_PROVIDER);
        mEnabledSwitch.setChecked(gpsEnabled);

        final int padding = getResources().getDimensionPixelSize(
                R.dimen.action_bar_switch_padding);
        mEnabledSwitch.setPaddingRelative(0, 0, padding, 0);
        mEnabledSwitch.setOnCheckedChangeListener(this);
    }
*/
    @Override
    public void onStart() {
        super.onStart();

        getActionBar().setDisplayOptions(
                ActionBar.DISPLAY_SHOW_CUSTOM, ActionBar.DISPLAY_SHOW_CUSTOM);
        //getActionBar().setCustomView(
        //        mEnabledSwitch,
        //        new ActionBar.LayoutParams(ActionBar.LayoutParams.WRAP_CONTENT,
        //                ActionBar.LayoutParams.WRAP_CONTENT,
        //                 Gravity.CENTER_VERTICAL | Gravity.END));

        // listen for Location Manager settings changes
        mSettingsCursor = mContentResolver.query(
                Settings.Secure.CONTENT_URI, null, "(" + Settings.System.NAME
                        + "=?)",
                new String[] { Settings.Secure.LOCATION_PROVIDERS_ALLOWED },
                null);
        mContentQueryMap = new ContentQueryMap(mSettingsCursor,
                Settings.System.NAME, true, null);

    }

    @Override
    public void onStop() {
        super.onStop();
        getActionBar().setDisplayOptions(0, ActionBar.DISPLAY_SHOW_CUSTOM);
        getActionBar().setCustomView(null);

        if (mSettingsObserver != null) {
            mContentQueryMap.deleteObserver(mSettingsObserver);
        }
        if (mSettingsCursor != null) {
            mSettingsCursor.close();
        }
    }

    private void createPreferenceHierarchy() {

        addPreferencesFromResource(R.xml.agps_setting);

        // / M: If not support Agps, remove Agps related preference @{
        if (FeatureOption.MTK_AGPS_APP
                && (FeatureOption.MTK_GPS_SUPPORT || FeatureOption.MTK_EMULATOR_SUPPORT)) {
            mAgpsMgr = (MtkAgpsManager) getSystemService(Context.MTK_AGPS_SERVICE);
        }
        mAgpsPref = findPreference(KEY_AGPS_SETTINGS);
        mAgpsCB = (CheckBoxPreference) findPreference(KEY_AGPS_ENABLER);

        if ((!FeatureOption.MTK_GPS_SUPPORT && !FeatureOption.MTK_EMULATOR_SUPPORT)
                || !FeatureOption.MTK_AGPS_APP) {
            if (mAgpsPref != null) {
                getPreferenceScreen().removePreference(mAgpsPref);
            }
            if (mAgpsCB != null) {
                getPreferenceScreen().removePreference(mAgpsCB);
            }
        }
    }

    @Override
    public void onResume() {
        super.onResume();

        updateLocationToggles();

        if (mSettingsObserver == null) {
            mSettingsObserver = new Observer() {
                public void update(Observable o, Object arg) {
                    updateLocationToggles();
                }
            };
        }

        mContentQueryMap.addObserver(mSettingsObserver);
    }

    @Override
    public boolean onPreferenceTreeClick(PreferenceScreen preferenceScreen,
            Preference preference) {

        if (preference == mAgpsCB) {
            if (mAgpsCB.isChecked()) {
                mAgpsCB.setChecked(false);
                showDialog(CONFIRM_AGPS_DIALOG_ID);
            } else {
                mAgpsMgr.disable();
            }
        } else if (preference == mAgpsPref) {
            startActivity(new Intent(this, AgpsSettings.class));
        } else {
            // If we didn't handle it, let preferences handle it.
            return super.onPreferenceTreeClick(preferenceScreen, preference);
        }

        return true;
    }

    @Override
    public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
        Xlog.d(TAG,
                "[f] onCheckedChanged " + isChecked + "; "
                        + buttonView.isChecked());
        //if (buttonView == mEnabledSwitch) {
        //    Xlog.d(TAG, "[f] onCheckedChanged buttonview is mEnableSwitch");
        //    Settings.Secure.setLocationProviderEnabled(mContentResolver,
        //            LocationManager.GPS_PROVIDER, isChecked);

        //}
    }

    /*
     * Creates toggles for each available location provider
     */
    private void updateLocationToggles() {

        boolean gpsEnabled = Settings.Secure.isLocationProviderEnabled(
                mContentResolver, LocationManager.GPS_PROVIDER);
        //mEnabledSwitch.setChecked(gpsEnabled);

        if (mAgpsCB != null && mAgpsMgr != null) {
            Xlog.d(TAG, "[agps_sky] agpsSettingsEnter screen check Agps status = " + mAgpsMgr.getStatus());
            mAgpsCB.setChecked(mAgpsMgr.getStatus());
            mAgpsCB.setEnabled(true);
        }

    }

    public Dialog onCreateDialog(int id) {

        Dialog dialog = null;
        switch (id) {
        case CONFIRM_AGPS_DIALOG_ID:
            dialog = new AlertDialog.Builder(this)
                    .setTitle(R.string.agps_enable_confirm_title)
                    .setMessage(R.string.agps_enable_confirm)
                    .setIcon(com.android.internal.R.drawable.ic_dialog_alert)
                    .setPositiveButton(R.string.agps_enable_confirm_allow,
                            new DialogInterface.OnClickListener() {
                                public void onClick(
                                        DialogInterface dialoginterface, int i) {
                                    mAgpsCB.setChecked(true);
                                    mAgpsMgr.enable();
                                }
                            })
                    .setNegativeButton(R.string.agps_enable_confirm_deny,
                            new DialogInterface.OnClickListener() {
                                public void onClick(
                                        DialogInterface dialoginterface, int i) {
                                    // / M: see cr [alps00339992] enabe a-gps
                                    // should be disabeld when you tap cancel
                                    Xlog.d(TAG, "-->mAgpsMgr.getStatus()"
                                            + mAgpsMgr.getStatus());
                                    if (!mAgpsMgr.getStatus()) {
                                        mAgpsCB.setChecked(false);
                                    }
                                    Xlog.i(TAG, "DenyDenyDeny");
                                }
                            }).create();
            break;
        default:
            break;
        }
        return dialog;
    }

}
