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

/*
 * Copyright (C) 2006 The Android Open Source Project
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

import android.app.Activity;
import android.app.Fragment;
import android.app.FragmentManager;
import android.app.FragmentTransaction;
import android.content.DialogInterface;
import android.os.Bundle;
import android.os.Parcelable;
import android.support.v4.view.PagerAdapter;
import android.support.v4.view.ViewPager;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;

import com.mediatek.taskmanager.ActionBarAdapter.TabState;




/**
 * Activity to pick an application that will be used to display installation information and
 * options to uninstall/delete user data for system applications. This activity
 * can be launched through Settings or via the ACTION_MANAGE_PACKAGE_STORAGE
 * intent.
 */
public class TaskManagerPageActivity extends Activity implements
        DialogInterface.OnCancelListener,ActionBarAdapter.Listener {
    static final String TAG = "TaskManagerPageActivity";
    static final boolean DEBUG = false;
    
    // attributes used as keys when passing values to InstalledAppDetails activity
    public static final String APP_CHG = "chg";
    
    // constant value that can be used to check return code from sub activity.
    private static final int INSTALLED_APP_DETAILS = 1;

    // sort order that can be changed through the menu can be sorted alphabetically
    // or size(descending)
    private static final int MENU_OPTIONS_BASE = 0;
    // Filter options used for displayed list of applications
    public static final int FILTER_APPS_ALL = MENU_OPTIONS_BASE + 0;
    public static final int FILTER_APPS_THIRD_PARTY = MENU_OPTIONS_BASE + 1;
    public static final int FILTER_APPS_SDCARD = MENU_OPTIONS_BASE + 2;

    public static final int SORT_ORDER_ALPHA = MENU_OPTIONS_BASE + 4;
    public static final int SORT_ORDER_SIZE = MENU_OPTIONS_BASE + 5;


    // sort order
    private int mSortOrder = SORT_ORDER_ALPHA;
    // Filter value
    private int mFilterApps = FILTER_APPS_THIRD_PARTY;
    
    private boolean mShouldShowSdCard = true;
 
  

    RunningState mState;     

    private TabPagerAdapter mPageAdapter ;
    private ViewPager mPager;

    
    private static final int RUNNING = 0;
    private static final int INSTALLED = 1;
    private static final int SYSTEM_INFO = 2;    
    private static final int NUM_TABS = 3;


    private ViewPager mTabPager;
    private TabPagerAdapter mTabPagerAdapter;
    private final TabPagerListener mTabPagerListener = new TabPagerListener();
    private ActionBarAdapter mActionBarAdapter;    

    final String mRunningTag = "tab-pager-running";
    final String mInstalledTag = "tab-pager-install";
    final String mSystemInfoTag = "tab-pager-system";


    private class TabPagerAdapter extends PagerAdapter {
        private final FragmentManager mFragmentManager;
        private FragmentTransaction mCurTransaction = null;

        private Fragment mCurrentPrimaryItem;


        public TabPagerAdapter() {
            mFragmentManager = getFragmentManager();
        }

        @Override
        public int getCount() {
            return NUM_TABS;
        }

        /** Gets called when the number of items changes. */
        @Override
        public int getItemPosition(Object object) {

            if (object == mInstalledFragment) {
                return INSTALLED;
            }
            if (object == mRunningFragment) {
                return RUNNING;
            }
            if (object == mSystemInfoFragment) {
                return SYSTEM_INFO;
            }
            return POSITION_NONE;
        }

        @Override
        public void startUpdate(View container) {
        }

        private Fragment getFragment(int position) {
            if (position == INSTALLED) {
                return mInstalledFragment;
            } else if (position == RUNNING) {
                return mRunningFragment;
            } else if (position == SYSTEM_INFO) {
                return mSystemInfoFragment;
            }

            throw new IllegalArgumentException("position: " + position);
        }

        @Override
        public Object instantiateItem(View container, int position) {
            if (mCurTransaction == null) {
                mCurTransaction = mFragmentManager.beginTransaction();
            }
            Fragment f = getFragment(position);
            mCurTransaction.show(f);

            // Non primary pages are not visible.
            f.setUserVisibleHint(f == mCurrentPrimaryItem);
            return f;
        }

        @Override
        public void destroyItem(View container, int position, Object object) {
            if (mCurTransaction == null) {
                mCurTransaction = mFragmentManager.beginTransaction();
            }
            mCurTransaction.hide((Fragment) object);
        }

        @Override
        public void finishUpdate(View container) {
            if (mCurTransaction != null) {
                mCurTransaction.commitAllowingStateLoss();
                mCurTransaction = null;
                mFragmentManager.executePendingTransactions();
            }
        }

        @Override
        public boolean isViewFromObject(View view, Object object) {
            return ((Fragment) object).getView() == view;
        }

        @Override
        public void setPrimaryItem(View container, int position, Object object) {
            Fragment fragment = (Fragment) object;
            if (mCurrentPrimaryItem != fragment) {
                if (mCurrentPrimaryItem != null) {
                    mCurrentPrimaryItem.setUserVisibleHint(false);
                }
                if (fragment != null) {
                    fragment.setUserVisibleHint(true);
            PrimarySetListener listener = (PrimarySetListener) object;
            listener.onSetPrimary();
                }
                mCurrentPrimaryItem = fragment;
                
            }
        }

        @Override
        public Parcelable saveState() {
            return null;
        }

        @Override
        public void restoreState(Parcelable state, ClassLoader loader) {
        }
    }

    interface PrimarySetListener {
        void onSetPrimary();
    }
    private class TabPagerListener implements ViewPager.OnPageChangeListener {
        @Override
        public void onPageScrollStateChanged(int state) {
        }

        @Override
        public void onPageScrolled(int position, float positionOffset, int positionOffsetPixels) {
        }

        @Override
        public void onPageSelected(int position) {
                // Make sure not in the search mode, in which case position != TabState.ordinal().
                TabState selectedTab = TabState.fromInt(position);
                mActionBarAdapter.setCurrentTab(selectedTab, false);
                TaskManagerPageActivity.this.updateFragmentsVisibility();
        }
    }

    private InstalledFragment mInstalledFragment;
    private RunningProcessFragment mRunningFragment;
    private SystemInfoFragment mSystemInfoFragment;
    
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.task_manager_pages);

        final FragmentManager fragmentManager = getFragmentManager();

        // Hide all tabs (the current tab will later be reshown once a tab is selected)
        final FragmentTransaction transaction = fragmentManager.beginTransaction();

        mTabPager = (ViewPager)findViewById(R.id.tab_pager);
    mTabPagerAdapter = new TabPagerAdapter();
    mTabPager.setAdapter(mTabPagerAdapter);
    mTabPager.setOnPageChangeListener(mTabPagerListener);

        // Create the fragments and add as children of the view pager.
    // The pager adapter will only change the visibility; it'll never create/destroy
    // fragments. However, if it's after screen rotation, the fragments have been re-created by
    // the fragment manager, so first see if there're already the target fragments existing.
        mInstalledFragment = (InstalledFragment)fragmentManager.findFragmentByTag(mInstalledTag);
        mRunningFragment = (RunningProcessFragment) fragmentManager.findFragmentByTag(mRunningTag);
        mSystemInfoFragment = (SystemInfoFragment) fragmentManager.findFragmentByTag(mSystemInfoTag);     

        if (mInstalledFragment == null) {
            mInstalledFragment = new InstalledFragment();
            mRunningFragment = new RunningProcessFragment();
            mSystemInfoFragment = new SystemInfoFragment();            
            transaction.add(R.id.tab_pager, mInstalledFragment, mInstalledTag);
            transaction.add(R.id.tab_pager, mRunningFragment, mRunningTag);
            transaction.add(R.id.tab_pager, mSystemInfoFragment, mSystemInfoTag);
        }
              


          // Hide all fragments for now.  We adjust visibility when we get onSelectedTabChanged()
         // from ActionBarAdapter.
         transaction.hide(mInstalledFragment);
         transaction.hide(mRunningFragment);
         transaction.hide(mSystemInfoFragment);

         // Configure action bar
         mActionBarAdapter = new ActionBarAdapter(this, this, getActionBar(), false);
         mActionBarAdapter.initialize(savedInstanceState);

         transaction.commitAllowingStateLoss();
         fragmentManager.executePendingTransactions();
                 mState = RunningState.getInstance(this);         

     }
    
   
    @Override
    protected void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);
        outState.putInt("sortOrder", mSortOrder);
    }

    
    @Override
    protected void onPause() {
        super.onPause();
    mState.pause();
    }


    @Override 
    protected void onResume() {
        super.onResume();
        if (mActionBarAdapter.getCurrentTab() == TabState.RUNNING) {        
                  mState.resume(mRunningFragment);
            } else if (mActionBarAdapter.getCurrentTab() == TabState.SYSTEM_INFO) {        
                  mState.resume(mSystemInfoFragment);
        }
    }
    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        menu.add(0, SORT_ORDER_ALPHA, 1, R.string.sort_order_alpha);
        menu.add(0, SORT_ORDER_SIZE, 2, R.string.sort_order_size);
        return true;
    }
    
    @Override
    public boolean onPrepareOptionsMenu(Menu menu) {
        /*
         *  update the menu when in different tab , set some menu disable 
         *  becasue in some case doesn't make any sense.
         */
       if (mActionBarAdapter.getCurrentTab() == TabState.INSTALLED) {
           menu.findItem(SORT_ORDER_ALPHA)
                   .setVisible(mSortOrder != SORT_ORDER_ALPHA)
                   .setShowAsAction(MenuItem.SHOW_AS_ACTION_IF_ROOM);
           menu.findItem(SORT_ORDER_SIZE)
                   .setVisible(mSortOrder != SORT_ORDER_SIZE)
                   .setShowAsAction(MenuItem.SHOW_AS_ACTION_IF_ROOM);
        } else {
            menu.findItem(SORT_ORDER_ALPHA).setVisible(false);
            menu.findItem(SORT_ORDER_SIZE).setVisible(false);
        }
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        int menuId = item.getItemId();
        if ((menuId == SORT_ORDER_ALPHA) || (menuId == SORT_ORDER_SIZE)) {
            mSortOrder = menuId;
            if (mActionBarAdapter.getCurrentTab() == TabState.INSTALLED) {
                mInstalledFragment.setSortOrder(mSortOrder);
            }
        } 
        invalidateOptionsMenu();
        return true;
    }
    
    
    // Finish the activity if the user presses the back button to cancel the activity
    public void onCancel(DialogInterface dialog) {
        finish();
    }

    @Override
    protected void onDestroy() {
        // Some of variables will be null if this Activity redirects Intent.
        // See also onCreate() or other methods called during the Activity's initialization.
        if (mActionBarAdapter != null) {
            mActionBarAdapter.setListener(null);
        }
        super.onDestroy();
    }


    @Override
    public void onSelectedTabChanged() {
        updateFragmentsVisibility();
    }

    private void updateFragmentsVisibility() {
       TabState tab = mActionBarAdapter.getCurrentTab();

       int tabIndex = tab.ordinal();
       if (mTabPager.getCurrentItem() != tabIndex) {
           mTabPager.setCurrentItem(tabIndex);
       }
  
       invalidateOptionsMenu();
 

   }
    
    
}
