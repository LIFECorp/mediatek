/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * MediaTek Inc. (C) 2010. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.mediatek.taskmanager;

import android.app.ActionBar;
import android.app.ActionBar.Tab;
import android.app.FragmentTransaction;
import android.content.Context;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.preference.PreferenceManager;

import com.mediatek.xlog.Xlog;

/**
 * Adapter for the action bar at the top of the Contacts activity.
 */
public class ActionBarAdapter  {

    public interface Listener {
        void onSelectedTabChanged();
    }

    private static final String EXTRA_KEY_SEARCH_MODE = "navBar.searchMode";
    private static final String EXTRA_KEY_QUERY = "navBar.query";
    private static final String EXTRA_KEY_SELECTED_TAB = "navBar.selectedTab";

    private static final String PERSISTENT_LAST_TAB = "actionBarAdapter.lastTab";

    private String mQueryString;


    private final Context mContext;
    private final SharedPreferences mPrefs;

    private Listener mListener;

    private final ActionBar mActionBar;
    private final MyTabListener mTabListener = new MyTabListener();

    private boolean mShowHomeIcon;
    private boolean mShowTabsAsText;

    public enum TabState {
        RUNNING,
        INSTALLED,
        SYSTEM_INFO;

        public static TabState fromInt(int value) {
            if (RUNNING.ordinal() == value) {
                return RUNNING;
            }
            if (INSTALLED.ordinal() == value) {
                return INSTALLED;
            }
            if (SYSTEM_INFO.ordinal() == value) {
                return SYSTEM_INFO;
            }
            throw new IllegalArgumentException("Invalid value: " + value);
        }
    }

    private static final TabState DEFAULT_TAB = TabState.RUNNING;
    private TabState mCurrentTab = DEFAULT_TAB;

    public ActionBarAdapter(Context context, Listener listener, ActionBar actionBar,
            boolean isUsingTwoPanes) {
        mContext = context;
        mListener = listener;
        mActionBar = actionBar;
        mPrefs = PreferenceManager.getDefaultSharedPreferences(mContext);

        mShowHomeIcon = true;
        // On wide screens, show the tabs as text (instead of icons)
        mShowTabsAsText = isUsingTwoPanes;


        // Set up tabs
        addTab(TabState.RUNNING,  R.string.running);
        addTab(TabState.INSTALLED,  R.string.installed);
        addTab(TabState.SYSTEM_INFO, R.string.memory_info);
    }

    public void initialize(Bundle savedState) {
        if (savedState == null) {
            mCurrentTab = loadLastTabPreference();
            Xlog.d("TaskManager" ,"init tab saved null " + mCurrentTab);
        } else {
            mCurrentTab = TabState.fromInt(savedState.getInt(EXTRA_KEY_SELECTED_TAB));
            Xlog.d("TaskManager" ,"init tab saved  " + mCurrentTab);                        
        }
        update();
    }

    public void setListener(Listener listener) {
        mListener = listener;
    }

    private void addTab(TabState tabState,  int description) {
        final Tab tab = mActionBar.newTab();
        tab.setTag(tabState);
        tab.setTabListener(mTabListener);
        tab.setText(description);
        mActionBar.addTab(tab);
    }

    private class MyTabListener implements ActionBar.TabListener {
        /**
         * If true, it won't call {@link #setCurrentTab} in {@link #onTabSelected}.
         * This flag is used when we want to programmatically update the current tab without
         * {@link #onTabSelected} getting called.
         */
        public boolean mIgnoreTabSelected;

        @Override public void onTabReselected(Tab tab, FragmentTransaction ft) { }
        @Override public void onTabUnselected(Tab tab, FragmentTransaction ft) { }

        @Override public void onTabSelected(Tab tab, FragmentTransaction ft) {
                setCurrentTab((TabState)tab.getTag());

        }
    }

    /**
     * Change the current tab, and notify the listener.
     */
    public void setCurrentTab(TabState tab) {
        setCurrentTab(tab, true);
    }

    /**
     * Change the current tab
     */
    public void setCurrentTab(TabState tab, boolean notifyListener) {
        if (tab == null) {
            throw new NullPointerException();
        }
        if (tab == mCurrentTab) {
            return;
        }
        mCurrentTab = tab;

        int index = mCurrentTab.ordinal();
        if ((mActionBar.getNavigationMode() == ActionBar.NAVIGATION_MODE_TABS)
                && (index != mActionBar.getSelectedNavigationIndex())) {
            mActionBar.setSelectedNavigationItem(index);
        }

        if (notifyListener && mListener != null) {
            mListener.onSelectedTabChanged();
        }
        saveLastTabPreference(mCurrentTab);
    }

    public TabState getCurrentTab() {
        return mCurrentTab;
    }


    private void updateDisplayOptions() {
        // All the flags we may change in this method.
        final int mask = ActionBar.DISPLAY_SHOW_TITLE | ActionBar.DISPLAY_SHOW_HOME
                | ActionBar.DISPLAY_HOME_AS_UP | ActionBar.DISPLAY_SHOW_CUSTOM;

        // The current flags set to the action bar.  (only the ones that we may change here)
        final int current = mActionBar.getDisplayOptions() & mask;

        // Build the new flags...
        int newFlags = 0;
        newFlags |= ActionBar.DISPLAY_SHOW_TITLE;
        if (mShowHomeIcon) {
            newFlags |= ActionBar.DISPLAY_SHOW_HOME;
        }
        mActionBar.setHomeButtonEnabled(true);

        if (current != newFlags) {
            // Pass the mask here to preserve other flags that we're not interested here.
            Xlog.d("TaskManager" ,"set action bar flag " + newFlags);
            mActionBar.setDisplayOptions(newFlags, mask);
        }
    }

    private void update() {

        if (mActionBar.getNavigationMode() != ActionBar.NAVIGATION_MODE_TABS) {
            // setNavigationMode will trigger onTabSelected() with the tab which was previously
            // selected.
            // The issue is that when we're first switching to the tab navigation mode after
            // screen orientation changes, onTabSelected() will get called with the first tab
            // (i.e. favorite), which would results in mCurrentTab getting set to FAVORITES and
            // we'd lose restored tab.
            // So let's just disable the callback here temporarily.  We'll notify the listener
            // after this anyway.
            mTabListener.mIgnoreTabSelected = true;
            mActionBar.setNavigationMode(ActionBar.NAVIGATION_MODE_TABS);
            mActionBar.setSelectedNavigationItem(mCurrentTab.ordinal());
            mTabListener.mIgnoreTabSelected = false;
        }
        //mActionBar.setTitle(null);
        // Since we have the {@link SearchView} in a custom action bar, we must manually handle
        // collapsing the {@link SearchView} when search mode is exited.
        
        //updateDisplayOptions();
    }

   

    public void onSaveInstanceState(Bundle outState) {
        outState.putInt(EXTRA_KEY_SELECTED_TAB, mCurrentTab.ordinal());
    }

  
    private void saveLastTabPreference(TabState tab) {
        Xlog.d("TaskManager" ,"saveLastTabPreference " + tab);        
        mPrefs.edit().putInt(PERSISTENT_LAST_TAB, tab.ordinal()).apply();
    }

    private TabState loadLastTabPreference() {
        try {
            return TabState.fromInt(mPrefs.getInt(PERSISTENT_LAST_TAB, DEFAULT_TAB.ordinal()));
        } catch (IllegalArgumentException e) {
            // Preference is corrupt?
            return DEFAULT_TAB;
        }
    }
}

