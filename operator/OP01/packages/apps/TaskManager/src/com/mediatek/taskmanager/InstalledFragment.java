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

import android.app.Fragment;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.provider.Settings;
import android.text.format.Formatter;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.animation.AnimationUtils;
import android.widget.AdapterView;
import android.widget.BaseAdapter;
import android.widget.Filter;
import android.widget.Filterable;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.TextView;

import com.mediatek.taskmanager.ApplicationsState.AppEntry;
import com.mediatek.xlog.Xlog;

import java.util.ArrayList;
import java.util.Comparator;

public class InstalledFragment extends Fragment
        implements AdapterView.OnItemClickListener ,TaskManagerPageActivity.PrimarySetListener {  
    static final String TAG = "InstalledFragment";


    // constant value that can be used to check return code from sub activity.
    private static final int INSTALLED_APP_DETAILS = 1;

    private ApplicationsState mApplicationsState;
    private ApplicationsAdapter mApplicationsAdapter;



   
    private boolean mShouldShowSdCard = true;
    
    // mSize resource used for packages whose size computation failed for some reason
    private CharSequence mInvalidSizeStr;
    private CharSequence mComputingSizeStr;
    
    // layout inflater object used to inflate views
    private LayoutInflater mInflater;
    
    private String mCurrentPkgName;
    
    private View mLoadingContainer;

    private View mListContainer;

    // ListView used to display list
    private ListView mListView;
     

    // sort order
    private int mSortOrder = TaskManagerPageActivity.SORT_ORDER_ALPHA;
    // Filter value
    private int mFilterApps = TaskManagerPageActivity.FILTER_APPS_ALL;

    // View Holder used when displaying views
    static class AppViewHolder {
        ApplicationsState.AppEntry mEntry;
        TextView mAppName;
        ImageView mAppIcon;
        TextView mAppSize;

        void updateSizeText(InstalledFragment ma) {
            Xlog.d(TAG, "updateSizeText of " + "label: " + mEntry.mLabel + " mEntry: " + mEntry
                    + " mEntry.mSizeStr: " + mEntry.mSizeStr + " mEntry.mSize: " + mEntry.mSize);
            if (mEntry.mSizeStr != null) {
                mAppSize.setText(Formatter.formatFileSize(ma.getActivity(), mEntry.mSize));
            } else if (mEntry.mSize == ApplicationsState.SIZE_INVALID) {
                mAppSize.setText(ma.mInvalidSizeStr);
            }
        }
    }    
    /*
     * Custom adapter implementation for the ListView
     * This adapter maintains a map for each displayed application and its properties
     * An index value on each AppInfo object indicates the correct position or index
     * in the list. If the list gets updated dynamically when the user is viewing the list of
     * applications, we need to return the correct index of position. This is done by mapping
     * the getId methods via the package name into the internal maps and indices.
     * The order of applications in the list is mirrored in mAppLocalList
     */
    class ApplicationsAdapter extends BaseAdapter implements Filterable,
            ApplicationsState.Callbacks {
        private final ApplicationsState mState;
        private final ArrayList<View> mActive = new ArrayList<View>();
        private ArrayList<ApplicationsState.AppEntry> mBaseEntries;
        private ArrayList<ApplicationsState.AppEntry> mEntries;
        private boolean mResumed;
        private int mLastFilterMode = -1;
        private int mLastSortMode = -1;
        private boolean mWaitingForData;
        CharSequence mCurFilterPrefix;

        private Filter mFilter = new Filter() {
            @Override
            protected FilterResults performFiltering(CharSequence constraint) {
                ArrayList<ApplicationsState.AppEntry> entries
                        = applyPrefixFilter(constraint, mBaseEntries);
                FilterResults fr = new FilterResults();
                fr.values = entries;
                fr.count = entries.size();
                return fr;
            }

            @Override
            protected void publishResults(CharSequence constraint, FilterResults results) {
                mCurFilterPrefix = constraint;
                mEntries = (ArrayList<ApplicationsState.AppEntry>)results.values;
                notifyDataSetChanged();
            }
        };

        public ApplicationsAdapter(ApplicationsState state) {
            mState = state;
        }

        public void resume(int filter, int sort) {
            Xlog.i(TAG, "Resume!  mResumed=" + mResumed);
            if (!mResumed) {
                mResumed = true;
                mState.resume(this);
                mLastFilterMode = filter;
                mLastSortMode = sort;
                rebuild(true);
            } else {
                rebuild(filter, sort);
            }
        }

        public void pause() {
            if (mResumed) {
                mResumed = false;
                mState.pause();
            }
        }

        public void rebuild(int filter, int sort) {
            if (filter == mLastFilterMode && sort == mLastSortMode) {
                return;
            }
            mLastFilterMode = filter;
            mLastSortMode = sort;
            rebuild(true);
        }
        
        public void rebuild(boolean eraseold) {
            Xlog.d(TAG, "Rebuilding app list...");
            ApplicationsState.AppFilter filterObj = null;
            Comparator<AppEntry> comparatorObj;
            switch (mLastSortMode) {
                case TaskManagerPageActivity.SORT_ORDER_SIZE:
                    comparatorObj = ApplicationsState.SIZE_COMPARATOR;
                    break;
                default:
                    comparatorObj = ApplicationsState.ALPHA_COMPARATOR;
                    break;
            }
            ArrayList<ApplicationsState.AppEntry> entries
                    = mState.rebuild(filterObj, comparatorObj);
            if (entries == null && !eraseold) {
                // Don't have new list yet, but can continue using the old one.
                return;
            }
            mBaseEntries = entries;
            if (mBaseEntries != null) {
                mEntries = applyPrefixFilter(mCurFilterPrefix, mBaseEntries);
            } else {
                mEntries = null;
            }
            notifyDataSetChanged();

            if (entries == null) {
                mWaitingForData = true;
                mListContainer.setVisibility(View.INVISIBLE);
                mLoadingContainer.setVisibility(View.VISIBLE);
            } else {
                mListContainer.setVisibility(View.VISIBLE);
                mLoadingContainer.setVisibility(View.GONE);
            }
        }

        ArrayList<ApplicationsState.AppEntry> applyPrefixFilter(CharSequence prefix,
                ArrayList<ApplicationsState.AppEntry> origEntries) {
            if (prefix == null || prefix.length() == 0) {
                return origEntries;
            } else {
                String prefixStr = ApplicationsState.normalize(prefix.toString());
                final String spacePrefixStr = " " + prefixStr;
                ArrayList<ApplicationsState.AppEntry> newEntries
                        = new ArrayList<ApplicationsState.AppEntry>();
                for (int i = 0; i < origEntries.size(); i++) {
                    ApplicationsState.AppEntry entry = origEntries.get(i);
                    String nlabel = entry.getNormalizedLabel();
                    if (nlabel.startsWith(prefixStr) || nlabel.indexOf(spacePrefixStr) != -1) {
                        newEntries.add(entry);
                    }
                }
                return newEntries;
            }
        }

        @Override
        public void onRunningStateChanged(boolean running) {
            //setProgressBarIndeterminateVisibility(running);
        }

        @Override
        public void onRebuildComplete(ArrayList<AppEntry> apps) {
            if (mLoadingContainer.getVisibility() == View.VISIBLE) {
                mLoadingContainer.startAnimation(AnimationUtils.loadAnimation(
                        getActivity(), android.R.anim.fade_out));
                mListContainer.startAnimation(AnimationUtils.loadAnimation(
                        getActivity(), android.R.anim.fade_in));
            }
            mListContainer.setVisibility(View.VISIBLE);
            mLoadingContainer.setVisibility(View.GONE);
            mWaitingForData = false;
            mBaseEntries = apps;
            mEntries = applyPrefixFilter(mCurFilterPrefix, mBaseEntries);
            notifyDataSetChanged();
        }

        @Override
        public void onPackageListChanged() {
            rebuild(false);
        }

        @Override
        public void onPackageIconChanged() {
            // We ensure icons are loaded when their item is displayed, so
            // don't care about icons loaded in the background.
        }

        @Override
        public void onPackageSizeChanged(String packageName) {
            for (int i = 0; i < mActive.size(); i++) {
                AppViewHolder holder = (AppViewHolder)mActive.get(i).getTag();
                if (holder.mEntry.mInfo.packageName.equals(packageName)) {
                    synchronized (holder.mEntry) {
                        holder.updateSizeText(InstalledFragment.this);
                    }
                    if (holder.mEntry.mInfo.packageName.equals(mCurrentPkgName)
                            && mLastSortMode == TaskManagerPageActivity.SORT_ORDER_SIZE) {
                        // We got the size information for the last app the
                        // user viewed, and are sorting by size...  they may
                        // have cleared data, so we immediately want to resort
                        // the list with the new size to reflect it to the user.
                        rebuild(false);
                    }
                    return;
                }
            }
        }

        @Override
        public void onAllSizesComputed() {
            if (mLastSortMode == TaskManagerPageActivity.SORT_ORDER_SIZE) {
                rebuild(false);
            }
        }
        
        public int getCount() {
            return mEntries != null ? mEntries.size() : 0;
        }
        
        public Object getItem(int position) {
            return mEntries.get(position);
        }
        
        public ApplicationsState.AppEntry getAppEntry(int position) {
            return mEntries.get(position);
        }

        public long getItemId(int position) {
            return mEntries.get(position).mId;
        }

        public View getView(int position, View convertView, ViewGroup parent) {
            // A ViewHolder keeps references to children views to avoid unnecessary calls
            // to findViewById() on each row.
            AppViewHolder holder;

            // When convertView is not null, we can reuse it directly, there is no need
            // to reinflate it. We only inflate a new View when the convertView supplied
            // by ListView is null.
            if (convertView == null) {
                convertView = mInflater.inflate(R.layout.manage_applications_item, null);

                // Creates a ViewHolder and store references to the two children views
                // we want to bind data to.
                holder = new AppViewHolder();
                holder.mAppName = (TextView) convertView.findViewById(R.id.app_name);
                holder.mAppIcon = (ImageView) convertView.findViewById(R.id.app_icon);
                holder.mAppSize = (TextView) convertView.findViewById(R.id.app_size);
                convertView.setTag(holder);
            } else {
                // Get the ViewHolder back to get fast access to the TextView
                // and the ImageView.
                holder = (AppViewHolder) convertView.getTag();
            }

            // Bind the data efficiently with the holder
            ApplicationsState.AppEntry entry = mEntries.get(position);
            synchronized (entry) {
                holder.mEntry = entry;
                if (entry.mLabel != null) {
                    holder.mAppName.setText(entry.mLabel);
                    holder.mAppName.setTextColor(getResources().getColorStateList(
                            entry.mInfo.enabled ? android.R.color.primary_text_dark
                                    : android.R.color.secondary_text_dark));
                }
                mState.ensureIcon(entry);
                if (entry.mIcon != null) {
                    holder.mAppIcon.setImageDrawable(entry.mIcon);
                }
                holder.updateSizeText(InstalledFragment.this);
            }
            mActive.remove(convertView);
            mActive.add(convertView);
            return convertView;
        }

        @Override
        public Filter getFilter() {
            return mFilter;
        }

        //@Override
        public void onMovedToScrapHeap(View view) {
            mActive.remove(view);
        }
    }
    

    
    public void onItemClick(AdapterView<?> parent, View view, int position,
            long id) {
        ApplicationsState.AppEntry entry = mApplicationsAdapter.getAppEntry(position);
        mCurrentPkgName = entry.mInfo.packageName;
        startApplicationDetailsActivity();
    }




    @Override
    public void onActivityResult(int requestCode, int resultCode,
            Intent data) {
        if (requestCode == INSTALLED_APP_DETAILS && mCurrentPkgName != null) {
            mApplicationsState.requestSize(mCurrentPkgName);
        }
    }
    
    // utility method used to start sub activity
    private void startApplicationDetailsActivity() {
        // Create intent to start new activity
        Intent intent = new Intent(Settings.ACTION_APPLICATION_DETAILS_SETTINGS,
                Uri.fromParts("package", mCurrentPkgName, null));
        // start new activity to display extended information
        startActivityForResult(intent, INSTALLED_APP_DETAILS);
    }

    




    private View mContentView;
    
    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
            Bundle savedInstanceState) {

        mInflater = inflater;
        mApplicationsState = ApplicationsState.getInstance(getActivity().getApplication());
        mApplicationsAdapter = new ApplicationsAdapter(mApplicationsState);

        mContentView = inflater.inflate(R.layout.installed_app, null);
        mListView = (ListView)mContentView.findViewById(android.R.id.list);
        View emptyView = mContentView.findViewById(com.android.internal.R.id.empty);
        mLoadingContainer = mContentView.findViewById(R.id.loading_container);        
        mListContainer = mContentView.findViewById(R.id.list_container);        
        
        if (emptyView != null) {
            mListView.setEmptyView(emptyView);
        }

        mListView.setOnItemClickListener(this);
        //mListView.setRecyclerListener(mApplicationsAdapter);
        mListView.setAdapter(mApplicationsAdapter);




        return mContentView;
    }

    @Override
    public void onResume() {
        super.onResume();
        mApplicationsAdapter.resume(mFilterApps, mSortOrder);


    }
    @Override
    public void onPause() {
        super.onPause();
        if (mApplicationsAdapter != null) {
          mApplicationsAdapter.pause();
        }
    }    

    public void setSortOrder(int order) {
        mSortOrder = order;
        mApplicationsAdapter.rebuild(mFilterApps, mSortOrder);
        
    }

    @Override
    public void onSetPrimary() {
        Xlog.d(TAG,"onSetPrimary");
    }
}

