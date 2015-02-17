package com.mediatek.phone.callrejection;

import android.app.ActionBar;
import android.app.Activity;
import android.app.Dialog;
import android.app.ProgressDialog;
import android.content.ContentUris;
import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.graphics.Color;
import android.net.Uri;
import android.os.AsyncTask;  
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.AdapterView;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.ImageButton;
import android.widget.ListView;
import android.widget.PopupMenu.OnMenuItemClickListener;

import com.mediatek.op01.plugin.R;
import com.mediatek.phone.callrejection.CallRejectDropMenu.DropDownMenu;

import java.util.ArrayList;

public class CallRejectListModify extends Activity implements CallRejectListAdapter.CheckSelectCallBack {
    private static final String TAG = "MY::CallRejectListModify";

    private static final Uri URI = Uri.parse("content://reject/list");
    private static final int ID_INDEX = 0;
    private static final int NUMBER_INDEX = 1;
    private static final int TYPE_INDEX = 2;
    private static final int NAME_INDEX = 3;

    private static final int CALL_LIST_DIALOG_WAIT = 2;

    private ListView mListView;
    private CallRejectListAdapter mCallRejectListAdapter;

    private String mType;

    private static final int MENU_ID_SELECT_ALL = Menu.FIRST;
    private static final int MENU_ID_UNSELECT_ALL = Menu.FIRST + 1;
    private static final int MENU_ID_TRUSH = Menu.FIRST + 2;

    private ArrayList<CallRejectListItem> mCRLItemArray = new ArrayList<CallRejectListItem>();
    private ArrayList<CallRejectListItem> mCRLItemArrayTemp = new ArrayList<CallRejectListItem>();
    private DelContactsTask mDelContactsTask = null;
    private ReadContactsTask mReadContactsTask = null;
    private boolean mIsGetover = false;
    /// New feature: For ALPS00670111 @{
    private DropDownMenu mSelectionMenu;
    /// @}

    class ReadContactsTask extends AsyncTask<Integer, Integer, String> {  

        @Override  
        protected void onPreExecute() {  
            showDialog(CALL_LIST_DIALOG_WAIT);
            super.onPreExecute();  
        }  
          
        @Override  
        protected String doInBackground(Integer... params) {  
            getCallRejectListItems();
            return "";  
        }  
  
        @Override  
        protected void onProgressUpdate(Integer... progress) {  
            super.onProgressUpdate(progress);  
        }  
  
        @Override  
        protected void onPostExecute(String result) {  
            if (!this.isCancelled()) {
                mCRLItemArray.clear();
                mCRLItemArray.addAll(mCRLItemArrayTemp);
                for (CallRejectListItem callrejectitem : mCRLItemArrayTemp) {
                    Log.v(TAG, "ReadContactsTask:onPostExecute: name=" + callrejectitem.getName()+ " number=" + callrejectitem.getPhoneNum());
                }
                mCallRejectListAdapter.notifyDataSetChanged();
                mIsGetover = true;
                dismissDialog(CALL_LIST_DIALOG_WAIT);
            }
            super.onPostExecute(result);  
        }  

    }

    class DelContactsTask extends AsyncTask<Integer, String, Integer> {  
        @Override  
        protected void onPreExecute() {
            showDialog(CALL_LIST_DIALOG_WAIT);
            super.onPreExecute();  
        }  
          
        @Override  
        protected Integer doInBackground(Integer... params) {  
            int index = params[0];
            int size = params[1];
            while ((index < size) && !isCancelled()) {
                //CallRejectListItem callrejectitem = mCRLItemArray.get(index);
                CallRejectListItem callrejectitem = mCRLItemArrayTemp.get(index);
                if (callrejectitem.getIsChecked()) {
                    String id = callrejectitem.getId();
                    //mCRLItemArray.remove(index);
                    mCRLItemArrayTemp.remove(index);
                    Log.v(TAG, "doInBackground---------------");
                    //publishProgress("fire");
                    if (isCurTypeVtAndVoice(id)) {
                        updateRowById(id);
                    } else {
                        deleteRowById(id);
                    }
                    size--;
                } else {
                    index++;    
                }
            }
            return size;  
        }  
  
        @Override  
        protected void onProgressUpdate(String... id) {  
            Log.v(TAG, "onProgressUpdate---------------");
            super.onProgressUpdate(id);  
            //mCallRejectListAdapter.notifyDataSetChanged();
        }  
  
        @Override  
        protected void onPostExecute(Integer size) {  
            if (!this.isCancelled()) {
                mCRLItemArray.clear();
                mCRLItemArray.addAll(mCRLItemArrayTemp);
                mCallRejectListAdapter.notifyDataSetChanged();
                dismissDialog(CALL_LIST_DIALOG_WAIT);
                mListView.invalidateViews();
                if (size == 0) {
                    CallRejectListModify.this.finish();
                }
                updateSelectedItemsView(getString(R.string.selected_item_count, 0));
                /// JE: For ALPS00688770 @{
                updateOkButton(null);
                /// @}
            }
            super.onPostExecute(size);  
        }  
    }  

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.call_reject_list_modify);

        mType = getIntent().getStringExtra("type");

        //getCallRejectListItems();
        mListView = (ListView)findViewById(android.R.id.list);
        mCallRejectListAdapter = new CallRejectListAdapter(this, mCRLItemArray);
        if (mListView != null) {
            mListView.setAdapter(mCallRejectListAdapter);
            mListView.setOnItemClickListener(new AdapterView.OnItemClickListener() {
                @Override
                public void onItemClick(AdapterView<?> parent, View view, int position, long id) {

                    CheckBox checkboxView = (CheckBox)view.findViewById(R.id.call_reject_contact_check_btn);
                    if (checkboxView != null) {
                        checkboxView.setChecked(!checkboxView.isChecked());
                    }
                }    
            });
        }

        mCallRejectListAdapter.setCheckSelectCallBack(this);
        mType = getIntent().getStringExtra("type");
        configureActionBar();
        updateSelectedItemsView(getString(R.string.selected_item_count, 0));
    }

    @Override
    protected Dialog onCreateDialog(int id) {
        if (id == CALL_LIST_DIALOG_WAIT) {
            ProgressDialog dialog = new ProgressDialog(this);
            dialog.setMessage(getResources().getString(R.string.call_reject_please_wait));
            dialog.setCancelable(false);
            dialog.setIndeterminate(true);
            return dialog;
        }
        return null;
    }

    /// New feature: For ALPS0000670111 @{
    /*
    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        menu.add(Menu.NONE, MENU_ID_SELECT_ALL, 0, R.string.select_all)
                .setIcon(R.drawable.ic_menu_contact_select_all)
                .setShowAsAction(MenuItem.SHOW_AS_ACTION_IF_ROOM);
        menu.add(Menu.NONE, MENU_ID_UNSELECT_ALL, 0, R.string.select_clear)
                .setIcon(R.drawable.ic_menu_contact_clear_select)
                .setShowAsAction(MenuItem.SHOW_AS_ACTION_IF_ROOM);
        menu.add(Menu.NONE, MENU_ID_TRUSH, 0, R.string.select_trash)
                .setIcon(R.drawable.ic_menu_contact_trash)
                .setShowAsAction(MenuItem.SHOW_AS_ACTION_IF_ROOM);
        return super.onCreateOptionsMenu(menu);
    } 

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
        case MENU_ID_SELECT_ALL:
            selectAll();
            break;
        case MENU_ID_UNSELECT_ALL:
            unSelectAll();
            break;
        case MENU_ID_TRUSH:
            deleteSelection();
            break;
        case android.R.id.home:
            finish();
            break;
        default:
            break;
        }
        return super.onOptionsItemSelected(item);
    }
    */
    /// @}

    @Override
    public void onResume() {
        super.onResume();
        if ((mDelContactsTask != null) && (mDelContactsTask.getStatus() == AsyncTask.Status.RUNNING)) {
            Log.v(TAG, "onResume-------no update again--------");  
        } else if (!mIsGetover) {
            mCRLItemArray.clear();
            mCRLItemArrayTemp.clear();
            mReadContactsTask = new ReadContactsTask();
            mReadContactsTask.execute(0, 0);
            Log.v(TAG, "onResume---------------");  
        }
    }

    @Override
    public void onDestroy() {
        super.onDestroy();    
        if (mDelContactsTask != null) {
            mDelContactsTask.cancel(true);
        }
        if (mReadContactsTask != null) {
            mReadContactsTask.cancel(true);
        }
    }

    private void getCallRejectListItems() {
        Cursor cursor = null;
        try {
            cursor = getContentResolver().query(URI, new String[] {
                "_id", "Number", "type", "Name"}, null, null, null);

            if (cursor == null) {
                return;
            }
            cursor.moveToFirst();
            mCRLItemArrayTemp.clear();
            while (!cursor.isAfterLast()) {
                String id = cursor.getString(ID_INDEX);
                String numberDB = cursor.getString(NUMBER_INDEX);
                String type = cursor.getString(TYPE_INDEX);
                String name = cursor.getString(NAME_INDEX);
                Log.d(TAG, "id=" + id);
                Log.d(TAG, "numberDB=" + numberDB);
                Log.d(TAG, "type=" + type);
                Log.d(TAG, "name=" + name);
                if ("3".equals(type)
                    || ("2".equals(type) && "video".equals(mType))
                    || ("1".equals(type) && "voice".equals(mType))) {
                    CallRejectListItem crli = new CallRejectListItem(name, numberDB, id, false);
                    //mCRLItemArray.add(crli);
                    mCRLItemArrayTemp.add(crli);
                }
                cursor.moveToNext();
            }
        } finally {
            cursor.close();
        }
    }
    
    private void selectAll() {
        for (CallRejectListItem callrejectitem : mCRLItemArrayTemp) {
            callrejectitem.setIsChecked(true);    
        }
        updateSelectedItemsView(getString(R.string.selected_item_count, mCRLItemArrayTemp.size()));
        mListView.invalidateViews();
    }

    private void unSelectAll() {
        for (CallRejectListItem callrejectitem : mCRLItemArrayTemp) {
            callrejectitem.setIsChecked(false);    
        }
        updateSelectedItemsView(getString(R.string.selected_item_count, 0));
        mListView.invalidateViews();
    }

    private void deleteSelection() {
        Log.i(TAG, "Enter deleteSecection Function");
        boolean isSelected = false;
        for (CallRejectListItem callrejectitem : mCRLItemArrayTemp) {
            isSelected |= callrejectitem.getIsChecked();
        }
        if (isSelected) {
            mDelContactsTask  = new DelContactsTask();
            mCRLItemArrayTemp.clear();
            mCRLItemArrayTemp.addAll(mCRLItemArray);
            mDelContactsTask.execute(0, mCRLItemArray.size());
        }
    }

    private void updateRowById(String id) {
        ContentValues contentValues = new ContentValues();
        if ("video".equals(mType)) {
            contentValues.put("Type", "1");
        } else {
            contentValues.put("Type", "2");
        }

        try {
            Uri existNumberURI = ContentUris.withAppendedId(URI, Integer.parseInt(id));
            int result = getContentResolver().update(existNumberURI, contentValues, null, null);
            Log.i(TAG, "result is " + result);
        } catch (NumberFormatException e) {
            Log.e(TAG, "parseInt failed, the index is " + id);
        }
    }

    private void deleteRowById(String id) {
        try {
            Uri existNumberURI = ContentUris.withAppendedId(URI, Integer.parseInt(id));
            Log.i(TAG, "existNumberURI is " + existNumberURI);
            int result = getContentResolver().delete(existNumberURI, null, null);
            Log.i(TAG, "result is " + result);
        } catch (NumberFormatException e) {
            Log.e(TAG, "parseInt failed, the index is " + id);
        }
    }

    private boolean isCurTypeVtAndVoice(String id) {
        Uri existNumberURI = null;
        try {
            existNumberURI = ContentUris.withAppendedId(URI, Integer.parseInt(id));
        } catch (NumberFormatException e) {
                Log.e(TAG, "parseInt failed, the index is " + id);
        }

        Cursor cursor = getContentResolver().query(existNumberURI, new String[] {
                "_id", "Number", "Type", "Name"}, null, null, null);
        if (cursor == null) {
            return false;
        }
        cursor.moveToFirst();

        while (!cursor.isAfterLast()) {
            String type = cursor.getString(TYPE_INDEX);
            if ("3".equals(type)) {
                cursor.close();
                return true;
            }
            cursor.moveToNext();
        }
        cursor.close(); 
        return false;
    }

    /// New feature: For ALPS0000670111 @{
    protected OnClickListener getClickListenerOfActionBarOKButton() {
        return new OnClickListener() {
            @Override
            public void onClick(View v) {
                deleteSelection();
                Log.v(TAG, "configureActionBar, delete");
                return;
            }
        };
    }
    /**
     * add dropDown menu on the selectItems.The menu is "Select all" or "Deselect all"
     * @param customActionBarView
     */
    public void updateSelectionMenu(View customActionBarView) {
        CallRejectDropMenu dropMenu = new CallRejectDropMenu(this);
        // new and add a menu.
        mSelectionMenu = dropMenu.addDropDownMenu((Button) customActionBarView
                .findViewById(R.id.select_items), R.menu.selection);
        // new and add a menu.
        Button selectView = (Button) customActionBarView
                .findViewById(R.id.select_items);
        //selectView.setBackgroundDrawable(getResources().getDrawable(R.drawable.dropdown_normal_holo_dark));
        selectView.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                View parent = (View) v.getParent();
                updateSelectionMenu(parent);
                mSelectionMenu.show();
                return;
            }
        });
        MenuItem item = mSelectionMenu.findItem(R.id.action_select_all);
        int count = 0;
        for (CallRejectListItem callrejectitem : mCRLItemArrayTemp) {
            if (callrejectitem.getIsChecked()) {
            count++;
            }
        }
        boolean mIsSelectedAll = mCRLItemArrayTemp.size() == count;
        // if select all items, the menu is "Deselect all"; else the menu is "Select all".
        if (mIsSelectedAll) {
            Log.e(TAG, "mIsSelectedAll:" + mIsSelectedAll + "select none string:" + getString(R.string.menu_select_none));
            item.setChecked(true);
            item.setTitle(R.string.menu_select_none);
            // click the menu, deselect all items
            dropMenu.setOnMenuItemClickListener(new OnMenuItemClickListener() {
                public boolean onMenuItemClick(MenuItem item) {
                    Log.e(TAG, "select none click");
                    unSelectAll();
                    return false;
                }
            });
        } else {
            Log.e(TAG, "mIsSelectedAll:" + mIsSelectedAll + "select all string:" + getString(R.string.menu_select_all));
            item.setChecked(false);
            item.setTitle(R.string.menu_select_all);
            //click the menu, select all items.
            dropMenu.setOnMenuItemClickListener(new OnMenuItemClickListener() {
                public boolean onMenuItemClick(MenuItem item) {
                    Log.e(TAG, "select all click");
                    selectAll();
                    return false;
                }
            });
        }
        return;
    }
    /**
     * when the selected item changed, update the show count"
     * return the count
     */
    public int updateOkButton(Button okButton) {
        Log.e(TAG, "updateOkButton");
        int count = 0;
        for (CallRejectListItem callrejectitem : mCRLItemArrayTemp) {
            if (callrejectitem.getIsChecked()) {
                count++;
            }
        }
        /// JE: For ALPS00688770 @{
        Button deleteView = null;
        if (okButton == null) {
            Log.v(TAG, "updateOkButton, okButton is null, reload again");
            deleteView = (Button) getActionBar().getCustomView().findViewById(R.id.delete);
        } else {
            Log.v(TAG, "updateOkButton, okButton is not null");
            deleteView = okButton;
        }
        /// @}
        Log.v(TAG, "updateOkButton, checked count= " + count);
        if (count == 0) {
            // if there is no item selected, the "OK" button is disable.
            deleteView.setEnabled(false);
            deleteView.setTextColor(Color.GRAY);
        } else {
            deleteView.setEnabled(true);
            deleteView.setTextColor(Color.BLACK);
        }
        return count;
    }
    /// @}
    private void updateSelectedItemsView(String checkedItemsCount) {
        Button selectedItemsView = (Button) getActionBar().getCustomView().findViewById(R.id.select_items);
        if (selectedItemsView == null) {
            return;
        }
        selectedItemsView.setText(checkedItemsCount);
    }

    private void configureActionBar() {
        Log.v(TAG, "configureActionBar()");
        LayoutInflater inflater = (LayoutInflater) getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        View customActionBarView = inflater.inflate(R.layout.call_reject_list_modify_action_bar, null);
        ImageButton doneMenuItem = (ImageButton) customActionBarView.findViewById(R.id.done_menu_item);
        doneMenuItem.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                finish();
            }
        });
        /// New feature: For ALPS0000670111 @{
        //dispaly the "select_items" .
        Button selectView = (Button) customActionBarView
                .findViewById(R.id.select_items);
        //selectView.setBackgroundDrawable(getResources().getDrawable(R.drawable.dropdown_normal_holo_dark));
        selectView.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                View parent = (View) v.getParent();
                updateSelectionMenu(parent);
                mSelectionMenu.show();
                Log.v(TAG, "configureActionBar, tophome");
                return;
            }
        });

        //dispaly the "CANCEL" button.
        Button cancelView = (Button) customActionBarView
                .findViewById(R.id.cancel);
        String cancelText = cancelView.getText().toString();
        if ("Cancel".equalsIgnoreCase(cancelText)) {
            cancelText = cancelText.toUpperCase();
            cancelView.setText(cancelText);
        }
        cancelView.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                finish();
                Log.v(TAG, "configureActionBar, cancel");
            }
        });
        //dispaly the "OK" button.
        Button deleteView = (Button) customActionBarView
                .findViewById(R.id.delete);
        updateOkButton(deleteView);
        deleteView.setOnClickListener(getClickListenerOfActionBarOKButton());
        /// @}
        ActionBar actionBar = getActionBar();
        if (actionBar != null) {
            actionBar.setDisplayOptions(ActionBar.DISPLAY_SHOW_CUSTOM,
            ActionBar.DISPLAY_SHOW_CUSTOM /*| ActionBar.DISPLAY_SHOW_HOME
                /*| ActionBar.DISPLAY_SHOW_TITLE*/);
            actionBar.setCustomView(customActionBarView);
            actionBar.setDisplayShowHomeEnabled(false);
            //actionBar.setDisplayHomeAsUpEnabled(true);
        }
    }

    @Override
    public void setChecked(boolean isChecked) {
        int count = 0;
        /// New feature: For ALPS0000670111 @{
        count = updateOkButton(null);
        /// @}
        updateSelectedItemsView(getString(R.string.selected_item_count, count));
    }
}
