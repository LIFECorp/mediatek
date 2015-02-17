package com.mediatek.configurecheck2;

import java.util.ArrayList;
import com.mediatek.configurecheck2.R;
import com.mediatek.configurecheck2.CheckItemBase.check_result;

import android.app.AlertDialog;
import android.app.TabActivity;
import android.app.AlertDialog.Builder;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.view.GestureDetector;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.RelativeLayout;
import android.widget.TabHost;
import android.widget.TextView;

public class CheckResultActivity extends TabActivity implements TabHost.OnTabChangeListener
            , View.OnClickListener {

	private static final String TAG = "CheckResultActivity";
	
	//data
	private String mCheckCategory;
	private CheckItemProviderBase mCheckItemProvider;
	private static ArrayList<ListViewItem> mAutoItemArray;
	private static ArrayList<ListViewItem> mManualItemArray;
	
	//UI
	private ListView mAutoResultListView;
	private ListView mManualResultListView;
	private CheckResultAdapter mAutoAdapter;
	private CheckResultAdapter mManualAdapter;
	private TextView mChooseConfigText;
	private TextView mChooseConfirmText;
	private Button mAutoConfigBtn;
	private Button mRightConfirmBtn;
	private Button mWrongConfirmBtn;
	private Button mUnknownConfirmBtn;
	private RelativeLayout mConfirmBtns;
    private boolean mInitialized = false;

    private GestureDetector mDetector;
    private static final int FLING_X_MIN_DISTANCE = 100;
    private static final int FLING_Y_MAX_DISTANCE = 50;
    private CtstReceiver mCtstReceiver;
    private int mValueDefaultColor;

    private static final int MENU_GROUP = 0;
    private static final int MENU_CHECK_ALL = 1;
    private static final int MENU_UNCHECK_ALL = 2;
    private static final int MENU_CHECK_REPORT = 3;
	
    private static final int TAB_AUTO = 0;
    private static final int TAB_MANUAL = 1;
    private int mTabIndex;
    
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		CTSCLog.i(TAG, "onCreate()");
		super.onCreate(savedInstanceState);

        setTitle(getIntent().getStringExtra("Category"));
		
		TabHost tabHost = getTabHost();
		LayoutInflater.from(this).inflate(R.layout.check_result, tabHost.getTabContentView(), true);
	    
	    IntentFilter intentFilter = new IntentFilter();
	    intentFilter.addAction(CheckItemBase.NOTIFY_ACTIVITY_ACTION);
	    mCtstReceiver = new CtstReceiver();
	    registerReceiver(mCtstReceiver, intentFilter);
	    
	    mDetector = new GestureDetector(this, new MyGestureListener());
	}

	@Override
	protected void onResume() {
		// TODO Auto-generated method stub
	    CTSCLog.d(TAG, "onResume()" + " mInitialized = " + mInitialized);
		super.onResume();
		
		if (!mInitialized) {
	        constructListViewData();
	        setupButtons();
	        setupListView();
	        setupTabs();
	        if (mCheckCategory.equals(getString(R.string.one_button_check))) {
	            AlertUserToReset();
          }
	        mInitialized = true;
	        CTSCLog.d(TAG, "Initialize done");
        } else {
            //scenario: CheckResultActivity has been resume, then user switch to do 
            //some setting after which user turn back to CheckResultActivity again.
            //so here need to do checking again and call refresh() to refresh again
            doCheck();
            refresh(true, true);
        }
	}
	
	//alert user to restore factory setting
	private void AlertUserToReset() {
        Builder builder = new AlertDialog.Builder(this);
        View titleView = LayoutInflater.from(CheckResultActivity.this)
                .inflate(R.layout.custom_alert_dialog, null);
        TextView title = (TextView)titleView.findViewById(R.id.title);
        title.setText(getString(R.string.alert_restore_title));
        builder.setCustomTitle(titleView);
        builder.setMessage(getString(R.string.alert_restore_note));
        builder.setPositiveButton(R.string.alert_dialog_ok,  null);
        builder.create().show();
    }
	
	@Override
    protected void onDestroy() {
        // TODO Auto-generated method stub
        super.onDestroy();
        unregisterReceiver(mCtstReceiver);
    }

    private void doCheck() {
        int count = mCheckItemProvider.getItemCount();
        for (int i = 0; i < count; i++) {
            CheckItemBase item = mCheckItemProvider.getCheckItem(i);
            item.onCheck();
        }
    }
	
	private void setupButtons() {
	    mAutoConfigBtn = (Button)findViewById(R.id.auto_configure_btn);
        mAutoConfigBtn.setOnClickListener(this);
        mRightConfirmBtn = (Button)findViewById(R.id.right_confirm_btn);
        mRightConfirmBtn.setOnClickListener(this);
        mWrongConfirmBtn = (Button)findViewById(R.id.wrong_confirm_btn);
        mWrongConfirmBtn.setOnClickListener(this);
        mUnknownConfirmBtn = (Button)findViewById(R.id.unknown_confirm_btn);
        mUnknownConfirmBtn.setOnClickListener(this);
        
        mChooseConfigText = (TextView)findViewById(R.id.choose_to_config_text);
        mChooseConfirmText = (TextView)findViewById(R.id.choose_to_confirm_text);
        mConfirmBtns = (RelativeLayout)findViewById(R.id.confirm_btns);
        
        refreshButtons();
    }
	
	private boolean isShowConfigBtn() {
	    if (mAutoItemArray.isEmpty()) {
            return false;
        }
        int count = mAutoItemArray.size();
        for (int i = 0; i < count; i++) {
            ListViewItem item = mAutoItemArray.get(i);
            if (item.mNeedCheckBox) {
                return true;
            }
        }
        return false;
    }
	
	private boolean isShowConfirmBtns() {
	    if (mManualItemArray.isEmpty()) {
            return false;
        }
        int count = mManualItemArray.size();
        for (int i = 0; i < count; i++) {
            ListViewItem item = mManualItemArray.get(i);
            if (item.mNeedCheckBox) {
                return true;
            }
        }
        return false;
	}
	
	private void constructListViewData() {
		
		mAutoItemArray = new ArrayList<ListViewItem>();
		mManualItemArray = new ArrayList<ListViewItem>();
		
		//get category provider
		Intent intent = getIntent();
	    mCheckCategory = intent.getStringExtra("Category");
		mCheckItemProvider = ProviderFactory.getProvider(this, mCheckCategory);
		
		//construct ListViewItem array
        int count = mCheckItemProvider.getItemCount();
        for (int i = 0; i < count; i++) {
            ListViewItem item = generateListViewItem(mCheckItemProvider.getCheckItem(i));
            addListView(item);
        }
	}
	
	private void addListView(ListViewItem item) {
        if (item.mCheckItem.isCheckable()) {
            mAutoItemArray.add(item);
        } else {
            mManualItemArray.add(item);
        }
    }
	
    private ListViewItem generateListViewItem(CheckItemBase checkItem) {
        
        checkItem.onCheck();
        CheckItemBase.check_result resultType = checkItem.getCheckResult();
        int resultIconId = getResultIconId(resultType);
        boolean needCheckBox = itemNeedCheckBox(checkItem, resultType);
        //by default, auto check item set check box checked, while
        //manual check item set unchecked
        boolean defaultChecked = checkItem.isCheckable() ? true : false;
        ListViewItem listItem = new ListViewItem(checkItem, resultIconId,
                needCheckBox, defaultChecked);
        return listItem;
    }
	
	private boolean hintNeedNotice(CheckItemBase item) {
		
		boolean needNotice = false;
		if (item.isCheckable() 
				&& item.getCheckResult()==check_result.WRONG
				&& !item.isConfigurable()) {
			needNotice = true;
		}
    	return needNotice;
	}
    
	private boolean itemNeedCheckBox(CheckItemBase item, check_result resultType) {
		
		boolean needCheckBox = false;
		//items that is not checkable is put in manual tab, they always need check box
		//if ((resultType==check_result.WRONG && item.isConfigurable())
		//		|| !item.isCheckable()) {
		//	needCheckBox = true;
		//}
		if (((resultType==check_result.WRONG || resultType==check_result.UNKNOWN) && item.isConfigurable())
				|| !item.isCheckable()) { //resultType==check_result.UNKNOWN is added for MTBF-CheckShortcut
			needCheckBox = true;
		}
    	return needCheckBox;
	}
	
	private int getResultIconId(check_result resultType) {
    	
    	int icon = 0;
    	switch (resultType) {
    	case RIGHT:
    		icon = R.drawable.ic_right;
    		break;
    		
    	case WRONG:
    		icon = R.drawable.ic_wrong;
    		break;
    	
    	case UNKNOWN:
    		icon = R.drawable.ic_unknown;
    		break;
    		
    	default:
    		myAssert(false, "No such check type!");
    	}
    	return icon;
    }

	@Override
	public void onTabChanged(String tabId) {
		// TODO Auto-generated method stub
        CTSCLog.d(TAG, "onTabChanged() tabId = " + tabId);  
        if (tabId.equals("auto")) {
            mTabIndex = TAB_AUTO;
        } else {
            mTabIndex = TAB_MANUAL;
        }
	}
	
	@Override
	public void onClick(View v) {
		// TODO Auto-generated method stub
		CTSCLog.d(TAG, "onClick View = " + v);
		switch (v.getId()) {
		case R.id.auto_configure_btn:
			autoConfigure_confirm();
			break;
        case R.id.right_confirm_btn:
			confirm(check_result.RIGHT);
			break;
        case R.id.wrong_confirm_btn:
        	confirm(check_result.WRONG);
			break;
        case R.id.unknown_confirm_btn:
        	confirm(check_result.UNKNOWN);
			break;

		default:
			break;
		}
	}
	
	private void confirm(check_result resultType) {
		myAssert(TAB_MANUAL == mTabIndex, "confirm_btns should be in TAB_MANUAL!");
		boolean doRefresh = false;
        int count = mManualItemArray.size();
        for (int i = 0; i < count; i++) {
            ListViewItem item = mManualItemArray.get(i);
            if (item.mNeedCheckBox && item.mIsChecked) {
                if (item.mCheckItem.setCheckResult(resultType)) {
                    doRefresh = true;      
                } else {
                    //XYM_Q:here may show up an alert dialog to user?
                }
            }
        }
        
        if (doRefresh) {
            refresh(true, false);
        }
	}

    private void autoConfigure_confirm() {
        String nextLine = System.getProperty("line.separator");
        StringBuilder messageBuilder = new StringBuilder(getString(R.string.alert_modify_message));
        messageBuilder.append(nextLine);
        
        int count = mAutoItemArray.size();
        for (int i = 0; i < count; i++) {
            ListViewItem item = mAutoItemArray.get(i);
            if (item.mNeedCheckBox && item.mIsChecked) {
                messageBuilder.append("- ");
                messageBuilder.append(item.mCheckItem.getTitle());
                messageBuilder.append(nextLine);
            }
        }

        AlertDialog alert = new AlertDialog.Builder(this)
                                           .setTitle(getString(R.string.alert_modify_title))
                                           .setMessage(messageBuilder.toString())
                                           .setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener(){
                                                   public void onClick(DialogInterface dialog, int which) {
                                                       autoConfigure();
                                                   }
                                               })
                                           .setNegativeButton(android.R.string.cancel, null)
                                           .create();
        alert.show();
    }
	private void autoConfigure() {
		myAssert(TAB_AUTO == mTabIndex, "auto_configure_btn should be in TAB_AUTO!");
		boolean doRefresh = false;
		int count = mAutoItemArray.size();
		for (int i = 0; i < count; i++) {
            ListViewItem item = mAutoItemArray.get(i);
            if (item.mNeedCheckBox && item.mIsChecked) {
                if (item.mCheckItem.onReset()) {
                    //need to do check again
                    item.mCheckItem.onCheck();
                    doRefresh = true;      
                } else {
                    //XYM_Q:here may show up an alert dialog to user?
                }
            }
        }
		
		if (doRefresh) {
			refresh(true, false);
		}
	}
	
	private void showConfigBtn(boolean show) {
	    if (show) {
	        mChooseConfigText.setVisibility(View.VISIBLE);
            mAutoConfigBtn.setVisibility(View.VISIBLE);
        } else {
            mChooseConfigText.setVisibility(View.GONE);
            mAutoConfigBtn.setVisibility(View.GONE);
        }
    }
	
	private void showConfirmBtns(boolean show) {
	    if (show) {
	        mChooseConfirmText.setVisibility(View.VISIBLE);
            mConfirmBtns.setVisibility(View.VISIBLE);
        } else {
            mChooseConfirmText.setVisibility(View.GONE);
            mConfirmBtns.setVisibility(View.GONE);
        }
    }
	
	private void refreshConfigBtn() {
	    if (isShowConfigBtn()) {
            showConfigBtn(true);
            setConfigBtnState();
        } else {
            showConfigBtn(false);
        }
    }
	
	private void setConfigBtnState() {
	    if (isEnableConfigBtn()) {
            mAutoConfigBtn.setEnabled(true);
        } else {
            mAutoConfigBtn.setEnabled(false);
        }
    }
	
	//if has no check box be checked, set button disabled
	private boolean isEnableConfigBtn() {
        int count = mAutoItemArray.size();
        for (int i = 0; i < count; i++) {
            ListViewItem item = mAutoItemArray.get(i);
            if (item.mNeedCheckBox && item.mIsChecked) {
                return true;
            }
        }
        return false;
    }
	
	private void refreshConfirmBtns() {
	    if (isShowConfirmBtns()) {
            showConfirmBtns(true);
            setConfirmBtnsState();
        } else {
            showConfirmBtns(false);
        }
    }
	
	private void setConfirmBtnsState() {
	    if (isEnableConfirmBtns()) {
            mRightConfirmBtn.setEnabled(true);
            mWrongConfirmBtn.setEnabled(true);
            mUnknownConfirmBtn.setEnabled(true);
        } else {
            mRightConfirmBtn.setEnabled(false);
            mWrongConfirmBtn.setEnabled(false);
            mUnknownConfirmBtn.setEnabled(false);
        }
    }
	
	//if has no check box be checked, set button disabled
	private boolean isEnableConfirmBtns() {
        int count = mManualItemArray.size();
        for (int i = 0; i < count; i++) {
            ListViewItem item = mManualItemArray.get(i);
            if (item.mNeedCheckBox && item.mIsChecked) {
                return true;
            }
        }
        return false;
    }
	
	private void refreshButtons() {
	    refreshConfigBtn();
	    refreshConfirmBtns();
	}
	
	//isItemValueUpdated = true:
	//the value of CheckItemBase has been changed,
	//so we need to call CheckItemBase::getCheckResult() again.
	private void refresh(boolean isItemValueChanged, boolean updateAll) {
	    CTSCLog.d(TAG, "refresh()" + " isItemValueChanged = " + isItemValueChanged
	            + " updateAll = " + updateAll);
		if (isItemValueChanged) {
		    updateListViewData(updateAll);
        }
		refreshUI(updateAll);
	}
	
	//updateAll=true: update auto Tab and manual Tab
    //updateAll=false: just update current Tab
	private void refreshUI(boolean updateAll) {
        if (updateAll) {
            refreshAutoTab();
            refreshManualTab();
        } else {
            if (TAB_AUTO == mTabIndex) {
                refreshAutoTab();
            } else if (TAB_MANUAL == mTabIndex) {
                refreshManualTab();
            } else {
                myAssert(false, "No such tab!");
            }
        }
    }
	
	private void refreshAutoTab() {
        if (mAutoResultListView != null) {
            mAutoAdapter.notifyDataSetChanged();
        }
        refreshConfigBtn();
    }
	
	private void refreshManualTab() {
        if (mManualResultListView != null) {
            mManualAdapter.notifyDataSetChanged();
        }
        refreshConfirmBtns();
    }
	
	//updateAll=true: update all items
	//updateAll=false: just update current Tab's items
	private void updateListViewData(boolean updateAll) {
	    if (updateAll) {
	        updateListViewItem(mAutoItemArray);
	        updateListViewItem(mManualItemArray);
        } else {
            if (TAB_AUTO == mTabIndex) {
                updateListViewItem(mAutoItemArray);
            } else if (TAB_MANUAL == mTabIndex) {
                updateListViewItem(mManualItemArray);
            } else {
                myAssert(false, "No such tab!");
            }
        }
    }
	
	private void updateListViewItem(ArrayList<ListViewItem> itemArray) {
	    //call CheckItemBase::getCheckResult() again
        for (int i = 0; i < itemArray.size(); i++) {
            ListViewItem item = itemArray.get(i);
            CheckItemBase.check_result resultType = item.mCheckItem.getCheckResult();
            int resultIconId = getResultIconId(resultType);
            boolean needCheckBox = itemNeedCheckBox(item.mCheckItem, resultType);
            item.mResultIconId = resultIconId;
            item.mNeedCheckBox = needCheckBox;
            item.mIsChecked = item.mCheckItem.isCheckable() ? true : false;
        }
	}
	
	private boolean isShowCheckAllInMenu() {
	    boolean show = false;
	    switch (mTabIndex) {
        case TAB_AUTO:
            show = isShowConfigBtn();
            break;
        case TAB_MANUAL:
            show = isShowConfirmBtns();
            break;
        default:
            myAssert(false, "No such tab!");
            break;
        }
	    return show;
    }
	
	private boolean isShowReportInMenu() {
	    return mCheckItemProvider.getItemCount()!=0 ? true : false;
	}
	
    @Override
    public boolean onPrepareOptionsMenu(Menu menu) {
        // TODO Auto-generated method stub
        boolean showCheckAll = isShowCheckAllInMenu();
        menu.getItem(MENU_CHECK_ALL-1).setVisible(showCheckAll);
        menu.getItem(MENU_UNCHECK_ALL-1).setVisible(showCheckAll);
        
        boolean showReport = isShowReportInMenu();
        menu.getItem(MENU_CHECK_REPORT-1).setVisible(showReport);
        
        return super.onPrepareOptionsMenu(menu);
    }

    @Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// TODO Auto-generated method stub
		menu.add(MENU_GROUP, MENU_CHECK_ALL, 0, getString(R.string.menu_check_all));
		menu.add(MENU_GROUP, MENU_UNCHECK_ALL, 0, getString(R.string.menu_uncheck_all));
		menu.add(MENU_GROUP, MENU_CHECK_REPORT, 0, getString(R.string.menu_check_report));
		return super.onCreateOptionsMenu(menu);
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		// TODO Auto-generated method stub
	    CTSCLog.i(TAG, "onOptionsItemSelected(), item Id = " + item.getItemId());
		switch (item.getItemId()) {
		case MENU_CHECK_ALL:
			setCheckAll(true);
			break;
        case MENU_UNCHECK_ALL:
        	setCheckAll(false);
			break;
        case MENU_CHECK_REPORT:
        	Intent intent = new Intent(CheckResultActivity.this, CheckReportActivity.class);
        	intent.putExtra("Category", mCheckCategory);
        	startActivity(intent);
			break;
        default:
			break;
		}
		return super.onOptionsItemSelected(item);
	}
	
	private void setCheckAll(boolean checkAll) {
	    ArrayList<ListViewItem> itemArray = null;
	    if (TAB_AUTO == mTabIndex) {
	        itemArray = mAutoItemArray;
        } else if (TAB_MANUAL == mTabIndex) {
            itemArray = mManualItemArray;
        } else {
            myAssert(false, "No such tab!");
        }
	    
	    for (int i = 0; i < itemArray.size(); i++) {
	        ListViewItem item = itemArray.get(i);
	        if (item.mNeedCheckBox) {
	            item.mIsChecked = checkAll;
            }
        }
	    
		refresh(false, false);
	}

	private void myAssert(boolean noException, String log) {
        if (!noException) {
            throw new RuntimeException(TAG + "Assertion failed:" + log);
        }
    }
	
	private void setupAlertDlg(CheckItemBase item) {
	    Builder builder = new AlertDialog.Builder(this);
        
        String hintString = constructHint(item);
        View titleView = LayoutInflater.from(CheckResultActivity.this)
                .inflate(R.layout.custom_alert_dialog, null);
        builder.setCustomTitle(titleView);
        builder.setMessage(hintString);
        builder.setPositiveButton(R.string.alert_dialog_ok,  null);
        
        if (item.isForwardable()) {
            final CheckItemBase checkItem = item; 
            builder.setNeutralButton(R.string.alert_dialog_forward,
                    new DialogInterface.OnClickListener() {
                
                @Override
                public void onClick(DialogInterface dialog, int which) {
                    // TODO Auto-generated method stub
                    Intent intent = checkItem.getForwardIntent();
                    myAssert(intent != null, "Forward intent is null! Check item = " + checkItem.getTitle());
                    startActivity(intent);
                }
            });
        }
        
        builder.create().show();
    }
	
    private void setupListView() {
    	AdapterView.OnItemClickListener listener = new AdapterView.OnItemClickListener() {

			@Override
			public void onItemClick(AdapterView<?> arg0, View arg1, int arg2,
					long arg3) {
				// TODO Auto-generated method stub
				CheckItemBase item = null;
				if (TAB_AUTO == mTabIndex) {
					item = mAutoItemArray.get(arg2).mCheckItem;
				} else if (TAB_MANUAL == mTabIndex) {
					item = mManualItemArray.get(arg2).mCheckItem;
				} else {
					myAssert(false, "No such tab!");
				}
				setupAlertDlg(item);
			}
		};
    	mAutoResultListView = (ListView)findViewById(R.id.auto_check_result_list);
    	mAutoAdapter = new CheckResultAdapter(this, true);
    	mAutoResultListView.setAdapter(mAutoAdapter);
    	mAutoResultListView.setOnItemClickListener(listener);
    	
    	mManualResultListView = (ListView)findViewById(R.id.manual_check_result_list);
    	mManualAdapter = new CheckResultAdapter(this, false);
    	mManualResultListView.setAdapter(mManualAdapter);
    	mManualResultListView.setOnItemClickListener(listener);
    }
    
    private String constructHint(CheckItemBase item) {      
        String noticeTitle = getString(R.string.hint_notice_title);
        String notice = getString(R.string.hint_notice);
        String nextLine = System.getProperty("line.separator");
        
        StringBuilder sb = new StringBuilder("");
        if (hintNeedNotice(item)) {
            sb.append(noticeTitle).append(notice).append(nextLine);
        }
        if (!item.getNote().isEmpty()) {
            sb.append(item.getNote()); 
        }
  
        return sb.toString();
    }
	
	private void setupTabs() {
		TabHost tabHost = getTabHost();
		tabHost.setOnTabChangedListener(this);
        
		TabHost.TabSpec spec;
        spec = tabHost.newTabSpec("auto")
            .setIndicator(getString(R.string.tab_label_auto))
            .setContent(R.id.auto_check_result_tab);        
        tabHost.addTab(spec);
        
        spec = tabHost.newTabSpec("manual")
        .setIndicator(getString(R.string.tab_label_manual))
        .setContent(R.id.manual_check_result_tab);        
        tabHost.addTab(spec);
        
        mTabIndex = TAB_AUTO;
        if (mAutoItemArray.isEmpty() && !mManualItemArray.isEmpty()) {
            mTabIndex = TAB_MANUAL;
        }
        tabHost.setCurrentTab(mTabIndex);
    }
	
	public class ListViewItem {
        CheckItemBase mCheckItem;
	    int mResultIconId;
	    boolean mNeedCheckBox;
	    boolean mIsChecked; //is check box checked
	    //by default, auto check item set check box checked, while
        //manual check item set unchecked
	    
	    private ListViewItem(CheckItemBase item, int resultIconId,
	            boolean needCheckBox, boolean defaultChecked) {
	        mCheckItem = item;
	        mResultIconId = resultIconId;
	        mNeedCheckBox = needCheckBox;
	        mIsChecked = defaultChecked;
	    }
	}

    private class CheckResultAdapter extends BaseAdapter {
        
        private LayoutInflater mInflater;
        private boolean mIsAutoAdapter;

        class ViewHolder {
        	private ImageView mItemResultImage;
        	private TextView mItemName;
            private TextView mItemValueOrInfo;
            private CheckBox mItemCheckBox;
        }
        
        class CheckBoxListener implements OnCheckedChangeListener {
            private ListViewItem mItem;
            public CheckBoxListener(ListViewItem item) {
            	mItem = item;
            }
        	
			@Override
			public void onCheckedChanged(CompoundButton buttonView,
					boolean isChecked) { //notice: only be callback when checked changed!!!
				// TODO Auto-generated method stub
				mItem.mIsChecked = isChecked;
				
				if (TAB_AUTO == mTabIndex) {
		            setConfigBtnState();
		        } else if (TAB_MANUAL == mTabIndex) {
		            setConfirmBtnsState();
		        } else {
		            myAssert(false, "No such tab!");
		        }
			}
        	
        }
        
        private CheckResultAdapter(Context c, boolean isAuto) {
            mInflater = LayoutInflater.from(c);
            mIsAutoAdapter = isAuto;
        }        

        public int getCount() {
            int size;
            if (mIsAutoAdapter) {
                size = mAutoItemArray.size();
            } else {
                size = mManualItemArray.size();
            } 
            CTSCLog.d(TAG, "getCount() size = " + size);
            return size;
        }

        public Object getItem(int position) {
            return position;
        }

        public long getItemId(int position) {
            return 0;
        }
        
        public View getView(int position, View convertView, ViewGroup parent) {
            CTSCLog.d(TAG, "getView() position = " + position + " convertView = " + convertView);            
            ViewHolder holder;
            if (convertView == null) {
                holder = new ViewHolder();
                convertView = mInflater.inflate(R.layout.check_result_item, null);
                
                holder.mItemResultImage = (ImageView)convertView.findViewById(R.id.check_result_icon);   
                holder.mItemName = (TextView)convertView.findViewById(R.id.check_item_name);
                holder.mItemValueOrInfo = (TextView)convertView.findViewById(R.id.check_item_value_or_info);                  
                holder.mItemCheckBox = (CheckBox)convertView.findViewById(R.id.check_item_checkbox);
                
                mValueDefaultColor = holder.mItemValueOrInfo.getCurrentTextColor();
                convertView.setTag(holder);//to associate(store) the holder as a tag in convertView
            } else {
                holder = (ViewHolder)convertView.getTag();               
            }
            
            ListViewItem item;
            if (mIsAutoAdapter) {
                item = mAutoItemArray.get(position);
            } else {
                item = mManualItemArray.get(position);
            }
            setContentToHolder(holder, item);
            
            return convertView;
        }
        
        private void setContentToHolder(ViewHolder holder, ListViewItem item) {
            holder.mItemResultImage.setImageDrawable(getResources().getDrawable(item.mResultIconId));
            holder.mItemName.setText(item.mCheckItem.getTitle());
            
            String valueOrInfo;
            boolean isValue = false;
            if (item.mCheckItem.getValue()!=null && !item.mCheckItem.getValue().isEmpty()) {
            	valueOrInfo = item.mCheckItem.getValue();
            	isValue = true;
			} else {
				valueOrInfo = getString(R.string.click_to_know_info);
			}
            holder.mItemValueOrInfo.setText(valueOrInfo);
            boolean isWrongResult = item.mResultIconId==R.drawable.ic_wrong ? true : false;
            if (isWrongResult && isValue) {
                holder.mItemValueOrInfo.setTextColor(getResources().getColor(R.color.red));
            } else {
                holder.mItemValueOrInfo.setTextColor(mValueDefaultColor);
            }
            
            if (item.mNeedCheckBox) {
            	holder.mItemCheckBox.setVisibility(View.VISIBLE);
            	holder.mItemCheckBox.setOnCheckedChangeListener(new CheckBoxListener(item));
                holder.mItemCheckBox.setChecked(item.mIsChecked);
			} else {
				holder.mItemCheckBox.setVisibility(View.INVISIBLE);
			}
		}
    }
    
    private class CtstReceiver extends BroadcastReceiver {

        @Override
        public void onReceive(Context arg0, Intent intent) {
            // TODO Auto-generated method stub
            CTSCLog.d(TAG, "onReceive():" + "intent action = " + intent.getAction());
            if (intent.getAction().equals(CheckItemBase.NOTIFY_ACTIVITY_ACTION)) {
                refresh(true, true);                 
            }                       
        }        
    }
    
    public static ArrayList<ListViewItem> getAutoItemArray() {
        return mAutoItemArray;
    }
    
    public static ArrayList<ListViewItem> getManualItemArray() {
        return mManualItemArray;
    }
    
    @Override
    public boolean dispatchTouchEvent(MotionEvent ev) {
        // TODO Auto-generated method stub
        //mDetector.onTouchEvent(ev) will trigger MyGestureListener.onFling()
        if (mDetector.onTouchEvent(ev)) {
            ev.setAction(MotionEvent.ACTION_CANCEL);
        }
        
        return super.dispatchTouchEvent(ev);
    }
    
    class MyGestureListener extends GestureDetector.SimpleOnGestureListener {

        @Override
        public boolean onFling(MotionEvent e1, MotionEvent e2, float velocityX,
                float velocityY) {
            // TODO Auto-generated method stub
            CTSCLog.i(TAG, "onFling()");
            TabHost tabHost = getTabHost();
            if (Math.abs(e1.getY()-e2.getY()) > FLING_Y_MAX_DISTANCE) {
                //if dy is too big, just ignore
                return false;
            }
            if (e1.getX()-e2.getX() < (-FLING_X_MIN_DISTANCE)) {
                //left to right: switch to TAB_AUTO
                if (TAB_MANUAL == mTabIndex) {
                    tabHost.setCurrentTab(TAB_AUTO);
                    return true;
                }
            } else if (e1.getX()-e2.getX() > FLING_X_MIN_DISTANCE){
                //right to left: switch to TAB_MANUAL
                if (TAB_AUTO == mTabIndex) {
                    tabHost.setCurrentTab(TAB_MANUAL);
                    return true;
                }
            }
            
            return false;
        }
    }
}
