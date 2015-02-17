package com.mediatek.backuprestore;

import android.app.Activity;
import android.app.ListActivity;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.KeyEvent;
import android.view.View;
import android.widget.ListAdapter;
import android.widget.ListView;

import com.mediatek.backuprestore.utils.Constants;
import com.mediatek.backuprestore.utils.Constants.LogTag;
import com.mediatek.backuprestore.utils.MyLogger;

import java.util.ArrayList;

public class CheckedListActivity extends ListActivity {

    interface OnCheckedCountChangedListener {
        void onCheckedCountChanged();
    }
    
    interface OnUnCheckedChangedListener {
        void OnUnCheckedChanged();
    }

    private static final String TAG = "CheckListActivity/";
    private static final String SAVE_STATE_UNCHECKED_IDS = "CheckedListActivity/unchecked_ids";
    private ArrayList<Long> mUnCheckedIds;
    private ArrayList<OnCheckedCountChangedListener> mListeners;
    private ArrayList<OnUnCheckedChangedListener> mUncheckedChangedListener;
    private boolean mStatus = true;
    private int mRequestCode = 0;

    @Override
    protected void onCreate(final Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.backup);
        Log.i(LogTag.LOG_TAG, TAG + "onCreate");
        if (savedInstanceState != null) {
            restoreInstanceState(savedInstanceState);
        } else {
            restoreInstanceStateFromIntent();
        }
    }

    /**
     * When activated interface, updated to the last selected state
     */
    private void restoreInstanceStateFromIntent() {
        Intent intent = getIntent();
        Bundle mBundle = intent.getExtras();
        /*if( intent.getStringExtra(Constants.PERSON_DATA) != null) {
            mRequestCode = Constants.RESULT_PERSON_DATA;
        } else {
            mRequestCode = Constants.RESULT_APP_DATA;
        }*/
        if (mBundle != null) {
            long[] data = mBundle.getLongArray("data");
            if (data != null) {
                    if (mUnCheckedIds == null) {
                        Log.i(LogTag.LOG_TAG, TAG +" restoreInstanceStateFromIntent init mUnCheckedIds");
                        mUnCheckedIds = new ArrayList<Long>();
                    }
                for (int i = 0; i < data.length; i++) {
                    if (mUnCheckedIds != null && !mUnCheckedIds.contains(data[i])) {
                        mUnCheckedIds.add(data[i]);
                    }
                }
                Log.i(LogTag.LOG_TAG,
                        TAG + " restoreInstanceStateFromIntent mUnCheckedIds = " + mUnCheckedIds.toString()
                                + ", mUnCheckedIds size == " + mUnCheckedIds.size());
            }
        }
    }

    protected void setRequestCode (int requsetCode) {
	mRequestCode = requsetCode;
    }

    @Override
    protected void onStart() {
        super.onStart();
        Log.i(LogTag.LOG_TAG, TAG + "onStart");
    }

    @Override
    protected void onResume() {
        super.onResume();
        Log.i(LogTag.LOG_TAG, TAG + "onResume");
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        if(mUnCheckedIds != null) {
            mUnCheckedIds.clear();
            Log.i(LogTag.LOG_TAG, TAG + "onDestroy and mUnCheckedIds is clear ");
        }
        Log.i(LogTag.LOG_TAG, TAG + "onDestroy");
    }

    protected void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);
        if(mUnCheckedIds != null) {
            int size = mUnCheckedIds.size();
            Log.i(LogTag.LOG_TAG, TAG + "onSaveInstanceState : mUnCheckedIds size == "+mUnCheckedIds.size());
            long[] array = new long[size];
            for (int position = 0; position < size; position++) {
                array[position] = mUnCheckedIds.get(position);
                Log.i(LogTag.LOG_TAG, TAG + "onSaveInstanceState :array[position]  == "+array[position] );
                
            }
            outState.putLongArray(SAVE_STATE_UNCHECKED_IDS, array);
        }
    }

    private void restoreInstanceState(final Bundle savedInstanceState) {
        long array[] = savedInstanceState.getLongArray(SAVE_STATE_UNCHECKED_IDS);
        if (array != null) {
            Log.i(LogTag.LOG_TAG, TAG + "restoreInstanceState : array lengh == "+array.length);
            for (long item : array) {
                if(mUnCheckedIds == null) {
                    mUnCheckedIds = new ArrayList<Long>();
                } 
                mUnCheckedIds.add(item);
                Log.i(LogTag.LOG_TAG, TAG + "restoreInstanceState : array  item = "+ item);
            }
            // Log.i(LogTag.LOG_TAG, TAG + "restoreInstanceState : mUnCheckedIds size = "+mUnCheckedIds.toString());
        }
    }

    @Override
    protected void onListItemClick(ListView l, View v, int position, long id) {
        super.onListItemClick(l, v, position, id);
        revertItemCheckedByPosition(position);
    }

    protected void setButtonStatus(boolean status) {
        mStatus = status;
    }

    public int getCount() {
        int count = 0;
        ListAdapter adapter = getListAdapter();
        if (adapter != null) {
            count = adapter.getCount();
        }
        return count;
    }

    public int getCheckedCount() {
        return getCount() - getUnCheckedCount();
    }

    public int getUnCheckedCount() {
        return mUnCheckedIds.size();
    }

    /**
     * add for all click button
     * 
     * @return
     */
    public boolean getUncheckedDataCount() {
        boolean result = false;
        ListAdapter adapter = getListAdapter();
        for (int i = 0; i < mUnCheckedIds.size(); i++) {
            for (int position = 0; position < adapter.getCount(); position++) {
                if (mUnCheckedIds.get(i) == adapter.getItemId(position)) {
                    if (adapter.isEnabled(position)) {
                        result = true;
                        return result;
                    }
                }
            }
        }
        return result;
    }

    public boolean isItemCheckedById(long id) {
        boolean ret = true;
        if (mUnCheckedIds != null && mUnCheckedIds.contains(id)) {
            ret = false;
        }
        return ret;
    }

    public boolean isItemCheckedByPosition(int position) {
        boolean ret = true;
        ListAdapter adapter = getListAdapter();
        if (adapter != null) {
            long itemId = adapter.getItemId(position);
            ret = isItemCheckedById(itemId);
        }
        return ret;
    }

    public void setItemCheckedByPosition(int position, boolean checked) {
        ListAdapter adapter = getListAdapter();
        if (adapter != null && mUnCheckedIds != null) {
            long itemId = adapter.getItemId(position);
            if (checked) {
                mUnCheckedIds.remove(itemId);
            } else {
                if (!mUnCheckedIds.contains(itemId)) {
                    mUnCheckedIds.add(itemId);
                }
            }
            notifyItemCheckChanged();
        }
    }
    
    public void setItemUnCheckedByPosition(int position, boolean checked) {
        ListAdapter adapter = getListAdapter();
        if (adapter != null && mUnCheckedIds != null) {
            long itemId = adapter.getItemId(position);
            if (checked) {
                mUnCheckedIds.remove(itemId);
            } else {
                if (!mUnCheckedIds.contains(itemId)) {
                    mUnCheckedIds.add(itemId);
                }
            }
            notifyUnCheckedChanged();
        }

    }

    public void setItemCheckedById(long id, boolean checked) {
        ListAdapter adapter = getListAdapter();
        if (adapter != null) {
            if (checked) {
                mUnCheckedIds.remove(id);
            } else {
                if (!mUnCheckedIds.contains(id)) {
                    mUnCheckedIds.add(id);
                }
            }
            notifyItemCheckChanged();
        }
    }

    public void revertItemCheckedByPosition(int position) {
        if (mStatus) {
            Log.i(LogTag.LOG_TAG, TAG + "revertItemCheckedByPosition updata item status");
            boolean checked = isItemCheckedByPosition(position);
            setItemCheckedByPosition(position, !checked);
        }
    }

    /**
     * 
     * @param checked
     * @param notify
     *            is to notify
     */
    public void setAllChecked(boolean checked) {

        mUnCheckedIds.clear();
        if (!checked) {
            ListAdapter adapter = getListAdapter();
            if (adapter != null) {
                int count = adapter.getCount();
                for (int position = 0; position < count; position++) {
                    long itemId = adapter.getItemId(position);
                    mUnCheckedIds.add(itemId);
                }
            }
        } else {
                ListAdapter adapter = getListAdapter();
                if (adapter != null) {
                    int count = adapter.getCount();
                    for (int position = 0; position < count; position++) {
                        long itemId = adapter.getItemId(position);
                        mUnCheckedIds.remove(itemId);
                    }
                }
        }
        notifyItemCheckChanged();
    }

    public boolean isAllChecked(boolean checked) {

        boolean ret = true;
        if (checked) {
            // is it all checked
            if (getUnCheckedCount() > 0 ) {
                ret = false;
            }
        } else {
            // is it all unchecked
            if (getCheckedCount() > 0) {
                ret = false;
            }
        }
        return ret;
    }

    public Object getItemByPosition(int position) {
        ListAdapter adapter = getListAdapter();
        if (adapter == null) {
            MyLogger.logE(LogTag.LOG_TAG, TAG + "getItemByPosition: adapter is null, please check");
            return null;
        }
        return adapter.getItem(position);
    }

    /*
     * after data changed(item increase or decrease), must sync unchecked list
     */
    protected void syncUnCheckedItems() {
        ListAdapter adapter = getListAdapter();
        if (adapter == null) {
            mUnCheckedIds.clear();
        } else {
            ArrayList<Long> list = new ArrayList<Long>();
            int count = adapter.getCount();
            for (int position = 0; position < count; position++) {
                long itemId = adapter.getItemId(position);
                if (mUnCheckedIds.contains(itemId)) {
                    list.add(itemId);
                }
            }
            mUnCheckedIds.clear();
            mUnCheckedIds = list;
        }
    }

    protected void registerOnCheckedCountChangedListener(OnCheckedCountChangedListener listener) {
        if (mListeners == null) {
            mListeners = new ArrayList<OnCheckedCountChangedListener>();
        }
        mListeners.add(listener);
    }
    
    protected void registerOnUnCheckedChangedListener(OnUnCheckedChangedListener listener) {
        if(mUncheckedChangedListener == null) {
            mUncheckedChangedListener = new ArrayList<OnUnCheckedChangedListener>();
        }
        mUncheckedChangedListener.add(listener);
    }

    protected void unRegisterOnCheckedCountChangedListener(OnCheckedCountChangedListener listener) {
        if (mListeners != null) {
            mListeners.remove(listener);
        }
    }
    
    protected void unRegisterOnUnCheckedChangedListener(OnCheckedCountChangedListener listener) {
        if (mUncheckedChangedListener != null) {
            mUncheckedChangedListener.remove(listener);
        }
    }

    private void notifyItemCheckChanged() {
        if (mListeners != null) {
            for (OnCheckedCountChangedListener listener : mListeners) {
                listener.onCheckedCountChanged();
            }
        }
    }
    
    private void notifyUnCheckedChanged() {
        if (mUncheckedChangedListener != null) {
            for (OnUnCheckedChangedListener listener : mUncheckedChangedListener) {
                listener.OnUnCheckedChanged();
            }
        }
    }

    /**
     * The corresponding back button, and the currently selected state as a
     * result return
     */
    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        if (keyCode == KeyEvent.KEYCODE_BACK && event.getRepeatCount() == 0 && mRequestCode != 0) {
            if(mUnCheckedIds != null) {
                Log.i(LogTag.LOG_TAG, TAG + "click KEYCODE_BACK ");
                Intent data = new Intent();
                Bundle b = new Bundle();
                int num = mUnCheckedIds.size();
                Log.i(LogTag.LOG_TAG, TAG + " backKey mUnCheckedIds num = " + num);
                long[] mdata = new long[num];
                for (int i = 0; i < mdata.length; i++) {
                    mdata[i] = mUnCheckedIds.get(i);
                }
                b.putLongArray("data", mdata);
                b.putBooleanArray("contactType", sContactCheckTypes);
                data.putExtras(b);
                BackupTabFragment.setResultData(mRequestCode,data);
                
            } else {
                Log.i(LogTag.LOG_TAG, TAG + "click KEYCODE_BACK but mUnCheckedIds is null");
            }
        }
        
        return super.onKeyDown(keyCode, event);
    }

    static boolean[] sContactCheckTypes = null;

    public void setContactCheckTypes(boolean[] check) {
        sContactCheckTypes = new boolean[check.length];
        for (int i = 0; i < check.length; i++) {
            sContactCheckTypes[i] = check[i];
        }
    }

    /**
     * For CTA Test add fun and init unselectAll item
     * @param list
     * @param flag
     */
    protected void updateUnCheckedStates(ArrayList list, int flag) {
        if (mUnCheckedIds != null) {
            return;
        } else {
            mUnCheckedIds = new ArrayList<Long>();
            Log.i(LogTag.LOG_TAG, TAG +" updateUnCheckedStates init mUnCheckedIds");
        }
        if (list != null) {
            for (int i = 0; i < list.size(); i++) {
                if (flag == Constants.RESULT_PERSON_DATA) {
                    PersonalItemData data = (PersonalItemData) list.get(i);
                    if (data != null) {
                        mUnCheckedIds.add((long) data.getType());
                    }
                } else if (flag == Constants.RESULT_APP_DATA) {
                    AppSnippet data = (AppSnippet) list.get(i);
                    if (data != null) {
                        mUnCheckedIds.add((long) data.getPackageName().hashCode());
                    }
                } else if (flag == Constants.RESULT_RESTORE_APP) {
                    AppSnippet data = (AppSnippet) list.get(i);
                    if (data != null) {
                        mUnCheckedIds.add((long) data.getFileName().hashCode());
                    }
                }
            }
        }
        
        Log.i(LogTag.LOG_TAG, TAG +" updateUnCheckedStates mUnCheckedIds = "+ mUnCheckedIds.toString());
    }
}
