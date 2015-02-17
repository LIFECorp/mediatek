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

import android.app.ActivityManager;
import android.app.Fragment;
import android.content.ActivityNotFoundException;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.view.animation.AnimationUtils;
import android.widget.AbsListView.RecyclerListener;
import android.widget.AdapterView;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

import com.mediatek.xlog.Xlog;

import java.util.ArrayList;
import java.util.HashMap;

public class RunningProcessFragment extends Fragment
        implements AdapterView.OnItemClickListener, RecyclerListener, View.OnClickListener,
        RunningState.OnRefreshUiListener,TaskManagerPageActivity.PrimarySetListener {  
    static final String TAG = "RunningProcessFragment";

    final HashMap<View, ActiveItem> mActiveItems = new HashMap<View, ActiveItem>();
    
    static ActivityManager sAm;
    
    RunningState mState;
    
    static Runnable sDataAvail;

    StringBuilder mBuilder = new StringBuilder(128);
    
    RunningState.BaseItem mCurSelected;
    
    static ListView sListView;
    static ServiceListAdapter sAdapter;
    static TextView sRunningAppCount;
    ImageView mKillAll;

    public static class ActiveItem {
        View mRootView;
        RunningState.BaseItem mItem;
        ActivityManager.RunningServiceInfo mService;
        ViewHolder mHolder;
        long mFirstRunTime;
        boolean mSetBackground;
        
        void updateSize(Context context, StringBuilder builder) {
            TextView upSizeView = null;           
            if (mItem instanceof RunningState.ServiceItem) {
                // If we are displaying a service, then the service
                // uptime goes at the top.
                upSizeView = mHolder.size;
                
            } else {
                String size = mItem.mSizeStr != null ? mItem.mSizeStr : "";
                if (!size.equals(mItem.mCurSizeStr)) {
                    mItem.mCurSizeStr = size;
                    mHolder.size.setText(size);
                }               
             }
        }
    }
    
    public  class ViewHolder {
        public View rootView;
        public ImageView icon;
        public TextView name;
        public TextView size;
        public ImageView killIcon;

        public ViewHolder(View v) {
            rootView = v;
            icon = (ImageView)v.findViewById(R.id.icon);
            name = (TextView)v.findViewById(R.id.name);
            size = (TextView)v.findViewById(R.id.size);
            killIcon = (ImageView)v.findViewById(R.id.kill_icon);
            v.setTag(this);
        }
        
        public ActiveItem bind(RunningState state, RunningState.BaseItem item,
                StringBuilder builder,ArrayList<RunningState.MergedItem> mItems) {
            synchronized (state.mLock) {
                PackageManager pm = rootView.getContext().getPackageManager();
                if (item.mPackageInfo == null && item instanceof RunningState.MergedItem) {
                    // Items for background processes don't normally load
                    // their labels for performance reasons.  Do it now.
                    ((RunningState.MergedItem)item).mProcess.ensureLabel(pm);
                    item.mPackageInfo = ((RunningState.MergedItem)item).mProcess.mPackageInfo;
                    item.mDisplayLabel = ((RunningState.MergedItem)item).mProcess.mDisplayLabel;
                }
                name.setText(item.mDisplayLabel);
                ActiveItem ai = new ActiveItem();
                ai.mRootView = rootView;
                ai.mItem = item;
                ai.mHolder = this;
                ai.mFirstRunTime = item.mActiveSince;
                item.mCurSizeStr = null;
                if (item.mPackageInfo != null) {
                    icon.setImageDrawable(item.mPackageInfo.loadIcon(pm));
                }
                icon.setVisibility(View.VISIBLE);
                ai.updateSize(rootView.getContext(), builder);

                final RunningState.MergedItem mi = (RunningState.MergedItem)item;
                final String processName = mi.mProcess.mProcessName;
                final ArrayList<RunningState.MergedItem> mNewItems = mItems;
                killIcon.setOnClickListener(new OnClickListener() {
                    public void onClick(View v) {
                     // kill it
                     Xlog.d(TAG,"******onClick kill icon**********");
                     String packageName;
                     if (mi.mPackageInfo != null) {
                         packageName = mi.mPackageInfo.packageName;
                     } else {
                          packageName = processName;
                     }
                     killProcess(mi.mBackground, processName, packageName);
                     // remove this item from adapter ,let the user thinks it's killed at right.
                     mNewItems.remove(mi);
                     sAdapter.notifyDataSetChanged();
                     RunningProcessFragment.this.refreshUi(false);
                     }
                });
                 return ai;
            }
        }
    }
    
    
    class ServiceListAdapter extends BaseAdapter {
        final RunningState mState;
        final LayoutInflater mInflater;
        boolean mShowBackground;
        ArrayList<RunningState.MergedItem> mItems;
        String mCurProcessName = "com.mediatek.taskmanager";

        ServiceListAdapter(RunningState state) {
            mState = state;
            mInflater = (LayoutInflater)getActivity().getSystemService(
                    Context.LAYOUT_INFLATER_SERVICE);
            refreshItems();
        }

        void showAllProcess() {
             refreshItems();
             notifyDataSetChanged();            
        }

        // kill all the processes , set the current process at last.
        // because if the current process is killed , the function will
        // not execute continully , so the other processes will not be killed.
        void killAllProcess() {
          if (mItems == null) {
             Xlog.i(TAG,"items is null , when kill the all processes!");
             return;
          }
          int size = mItems.size();
          for (int i = 0 ; i < size ; i++) {
              RunningState.MergedItem mi = mItems.get(i);
              String processName = mi.mProcess.mProcessName;
              String packageName;
              if (mi.mPackageInfo != null) {
              	  packageName = mi.mPackageInfo.packageName;
              } else {
                  packageName = processName;
              }
              if (!processName.equals(mCurProcessName)) {
                Xlog.d(TAG,"i = " + i + " , current kill process: " + processName);
                killProcess(mi.mBackground, processName, packageName);
               }
          }
          killProcess(false, mCurProcessName, mCurProcessName);
        }

        void refreshItems() {
          ArrayList<RunningState.MergedItem> newItems = mState.getCurrentMergedItems();
          ArrayList<String> newProcessName = new ArrayList<String>();
          RunningState.MergedItem curProcItem = null;
          RunningState.MergedItem mmService = null;
          for (int i = 0 ; i < newItems.size() ; i++) {
              String pName = newItems.get(i).mProcess.mProcessName;
              newProcessName.add(pName);
              if (pName.equals(mCurProcessName)) {
                Xlog.d(TAG,"mCurProcessName: " + pName);
                curProcItem = newItems.get(i);
              } else if (pName.equals("com.aspire.mmservice")) {
                 mmService = newItems.get(i);
			  }
             }
            newItems.remove(curProcItem);
            newItems.remove(mmService);
			
            Xlog.d(TAG,"newItems1: " + newItems.size());
            ArrayList<RunningState.MergedItem> newBackgroundItems = mState.getCurrentBackgroundItems();
            for (int i = 0 ; i < newBackgroundItems.size() ; i++) {
                String name = newBackgroundItems.get(i).mProcess.mProcessName;
                if (!newProcessName.contains(name) && !name.equals("com.aspire.mmservice")) {
			   	
                    newItems.add(newBackgroundItems.get(i));
                    Xlog.d(TAG,"i: " + i + " newBackgroundItems.get(i).packageName:" 
                           + newBackgroundItems.get(i).mProcess.mProcessName);
                }
            }
            if (curProcItem != null) {
                newItems.add(curProcItem);
                Xlog.d(TAG,"curProcItem2: " + curProcItem.mProcess.mProcessName);
            } else {
                Xlog.i(TAG,"curProcItem == null ");
            }
            Xlog.d(TAG,"newItems2: " + newItems.size());
            if (mItems != newItems) {
                mItems = newItems;
                Xlog.d(TAG,"mItems: " + mItems.size());
            }
            if (mItems == null) {
                mItems = new ArrayList<RunningState.MergedItem>();
            }
        }
        
        public boolean hasStableIds() {
            return true;
        }
        
        public int getCount() {
            return mItems.size();
        }

        @Override
        public boolean isEmpty() {
            return mState.hasData() && mItems.size() == 0;
        }

        public Object getItem(int position) {
            /// For ALPS00934194 @{
            Xlog.i(TAG, "ServiceListAdapter::getItem position = " + position);
            if (!isVaidPosition(position)) {
                Xlog.i(TAG, "ServiceListAdapter::getItem return null");
                return null;
            }
            /// @}
            return mItems.get(position);
        }

        public long getItemId(int position) {
            /// For ALPS00934194 @{
            Xlog.i(TAG, "ServiceListAdapter::getItemId position = " + position);
            if (!isVaidPosition(position)) {
                Xlog.i(TAG, "ServiceListAdapter::getItemId return 0");
                return 0;
            }
            /// @}
            return mItems.get(position).hashCode();
        }

        public boolean areAllItemsEnabled() {
            return false;
        }

        public boolean isEnabled(int position) {
            /// For ALPS00934194 @{
            Xlog.i(TAG, "ServiceListAdapter::isEnabled position = " + position);
            if (!isVaidPosition(position)) {
                Xlog.i(TAG, "ServiceListAdapter::isEnabled return false");
                return false;
            }
            /// @}
            return !mItems.get(position).mIsProcess;
        }

        /// For ALPS00934194 @{
        private boolean isVaidPosition(int position) {
            Xlog.i(TAG, "ServiceListAdapter::isVaidPosition position = " + position);
            if (mItems == null) {
                Xlog.i(TAG, "ServiceListAdapter::isVaidPosition return false, mItem == null");
                return false;
            } else if (mItems.size() <= position) {
                Xlog.i(TAG, "ServiceListAdapter::isVaidPosition return false, size = " + mItems.size());
                return false;
            } else {
                Xlog.i(TAG, "ServiceListAdapter::isVaidPosition return true");
                return true;
            }
        }
        /// @}
        public View getView(int position, View convertView, ViewGroup parent) {
            View v;
            if (convertView == null) {
                v = newView(parent,position);
            } else {
                v = convertView;
            }
            bindView(v, position);
            return v;
        }
        
        public View newView(ViewGroup parent,int pos) {
            View v = mInflater.inflate(R.layout.running_processes_item, parent, false);
            new ViewHolder(v);
            return v;
        }
        
        public void bindView(View view, int position) {
            synchronized (mState.mLock) {
                if (position >= mItems.size()) {
                    // List must have changed since we last reported its
                    // size...  ignore here, we will be doing a data changed
                    // to refresh the entire list.
                    return;
                }
                ViewHolder vh = (ViewHolder) view.getTag();
                RunningState.MergedItem item = mItems.get(position);
                ActiveItem ai = vh.bind(mState, item, mBuilder,mItems);
                mActiveItems.put(view, ai);
            }
        }
    }

    
    void refreshUi(boolean dataChanged) {

        
        handleRunningProcessesAvail(mState.hasData());
        
        ServiceListAdapter adapter = (ServiceListAdapter)(sListView.getAdapter());
        if (dataChanged) {
            adapter.refreshItems();
            adapter.notifyDataSetChanged();
        }
        int count = adapter.mItems.size();
        sRunningAppCount.setText(String.valueOf(count));

    }
    
    public void onItemClick(AdapterView<?> parent, View v, int position, long id) {
       ListView l = (ListView)parent;
       final RunningState.MergedItem mi = (RunningState.MergedItem)l.getAdapter().getItem(position);
       final String processName;
       // just launch the process
       if(mi.mPackageInfo != null) {
           processName = mi.mPackageInfo.packageName;
       } else {
           processName = mi.mProcess.mProcessName;
       }
	   
       /* Dailer and contacts is in the same process. should start contacts */
       if (processName.equals("com.android.contacts")) {
           Intent intent = new Intent("android.intent.action.MAIN");
           intent.setClassName("com.android.contacts",
                   "com.android.contacts.activities.PeopleActivity");
           getActivity().startActivity(intent);
           return;
       }
       ArrayList<RunningState.MergedItem> runningItems = mState.getCurrentMergedItems();
       Intent i = getActivity().getPackageManager().getLaunchIntentForPackage(processName);
       try { 
            if (i != null/* && runningItems.contains(mi)*/) {
                 Xlog.d(TAG, "Click process name: " + processName);
                 //i.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TASK);
                 i.setPackage(null);
                 getActivity().startActivity(i); 
            } else {
                 Toast.makeText(getActivity(), R.string.switch_error, Toast.LENGTH_SHORT).show(); 
                 Xlog.d(TAG,"the launch intent for package " + processName + "is null");
           }
         } catch (ActivityNotFoundException e) { 
               Xlog.i(TAG,"ActivityNotFoundException ");
         }
    }

    public void onClick(View v) {
            if (v == mKillAll) {
            Xlog.d(TAG,"repsonse to the kill all action.");
            ServiceListAdapter adapter = (ServiceListAdapter)(sListView.getAdapter());
            adapter.killAllProcess();
            }
    }
    public static void  killProcess(boolean background, String processName, String packageName) {
               // CR: ALPS00246495
               if (processName == null) {
                 Xlog.d(TAG,"to kill processName is null.");
               }
               if (processName.equals("android.process.acore")
                       || processName.contains("launcher")
                       || processName.equals("com.android.deskclock")
                       || processName.equals("com.sohu.inputmethod.sogou")) {
                 Xlog.d(TAG,"not kill the process" + processName + " to avoid some trouble.");
                 return;
               }
               if (background) {
                   // Background process.  Just kill it.
                   Xlog.d(TAG,"kill the background processes" + processName);
                   sAm.killBackgroundProcesses(processName);
               } else {
               // make sure the proccess will be killed , We'll do a force-stop on it.
               // CR:ALPS00244890
                   Xlog.d(TAG," always forceStopPackage" + packageName);
                   sAm.forceStopPackage(packageName);
               }
    }

    public void onMovedToScrapHeap(View view) {
        mActiveItems.remove(view);
    }


    




    private View mContentView;
    private View mRealContent;
    private View mLoadingContainer;
    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
            Bundle savedInstanceState) {

        sAm = (ActivityManager)getActivity().getSystemService(Context.ACTIVITY_SERVICE);
        mState = RunningState.getInstance(getActivity());
        mContentView = inflater.inflate(R.layout.running_processes_view, null);
        mRealContent = mContentView.findViewById(R.id.content);
        mRealContent.setVisibility(View.GONE);
        mLoadingContainer = mContentView.findViewById(R.id.loading_container);
        mLoadingContainer.setVisibility(View.VISIBLE);
        sRunningAppCount = (TextView)mContentView.findViewById(R.id.count_running_app);
        mKillAll = (ImageView)mContentView.findViewById(R.id.kill_all);
        mKillAll.setOnClickListener(this);
        sListView = (ListView)mContentView.findViewById(android.R.id.list);
        View emptyView = mContentView.findViewById(com.android.internal.R.id.empty);
        if (emptyView != null) {
            sListView.setEmptyView(emptyView);
        }
        sListView.setOnItemClickListener(this);
        sListView.setRecyclerListener(this);
        sAdapter = new ServiceListAdapter(mState);
        sListView.setAdapter(sAdapter);
        sAdapter.showAllProcess();

        return mContentView;
    }

    @Override
    public void onResume() {
        Xlog.d(TAG,"onResume");
        super.onResume();
        if (mState.hasData()) {
            // If the state already has its data, then let's populate our
            // list right now to avoid flicker.
            refreshUi(true);
        }        

    }
    @Override
    public void onPause() {
        Xlog.d(TAG,"onPause");
        super.onPause();
    }    

    @Override
    public void onRefreshUi(int what) {
        Xlog.d(TAG,"onRefreshUi " + what);
        switch (what) {
            case REFRESH_DATA:
            case REFRESH_STRUCTURE:
                refreshUi(true);
                break;
            default:
                break;
        }
    }


    void handleRunningProcessesAvail(boolean avail) {
        Xlog.i(TAG,"handleRunningProcessesAvail " + avail);
        if (avail) {
            if (mRealContent.getVisibility() != View.VISIBLE) {
                mLoadingContainer.startAnimation(AnimationUtils.loadAnimation(
                        getActivity(), android.R.anim.fade_out));
                mRealContent.startAnimation(AnimationUtils.loadAnimation(
                        getActivity(), android.R.anim.fade_in));
                mRealContent.setVisibility(View.VISIBLE);
                mLoadingContainer.setVisibility(View.GONE);
            }
        } else {
            mRealContent.setVisibility(View.GONE);
            mLoadingContainer.setVisibility(View.VISIBLE);        
        }
    }    

    @Override
    public void onSetPrimary() {
        Xlog.d(TAG,"onSetPrimary");
        if (mState != null) {
            mState.pause();
            mState.resume(this);        
        }
    }
}
