package com.mediatek.configurecheck2;

import com.mediatek.configurecheck2.R;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.AlertDialog.Builder;
import android.content.Intent;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.AdapterView.OnItemClickListener;

import com.mediatek.common.featureoption.FeatureOption;

public class ManSelfTestActivity extends Activity {

    private static final String TAG = "CategoryListActivity";
    private String mCtscVerStr;
    private ListView mSelfTestCateList;
    private String[] mSelfTestCateArray;
    private ArrayAdapter<String> mSelfTestCateAdapter;
    private ListView mMTKSelfTestCateList;
    private String[] mMTKSelfTestCateArray;
    private ArrayAdapter<String> mMTKSelfTestCateAdapter;
    private ListView mOthersCateList;
    private String[] mOthersCateArray;
    private ArrayAdapter<String> mOthersCateAdapter;
    
    private static final int MENU_CTSC_VER = 1;
    
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        // TODO Auto-generated method stub
        CTSCLog.i(TAG, "onCreate()");
        super.onCreate(savedInstanceState);
        setContentView(R.layout.self_test);
        
        mCtscVerStr = getCTSCVersion();
        mCtscVerStr = String.format(getString(R.string.CTSC_version), mCtscVerStr);
        
        setupSelfTestListview();
        setupMTKSelfTestListview();
        setupOthersListview();
    }
    
    private void setupSelfTestListview() {
        mSelfTestCateList = (ListView) findViewById(R.id.manufacturer_self_test_list);
        mSelfTestCateArray = getResources().getStringArray(R.array.category_manufacturer_self_test);
        mSelfTestCateAdapter = new ArrayAdapter<String>(this,
                android.R.layout.simple_list_item_1, mSelfTestCateArray);
        mSelfTestCateList.setAdapter(mSelfTestCateAdapter);
        
        mSelfTestCateList.setOnItemClickListener(new OnItemClickListener() {

            @Override
            public void onItemClick(AdapterView<?> arg0, View arg1, int arg2,
                    long arg3) {
                // TODO Auto-generated method stub
                CTSCLog.i(TAG, "onItemClick(): category = " + mSelfTestCateArray[arg2]);
                
                Intent intent = new Intent(ManSelfTestActivity.this,
                        SubCategoryListActivity.class);
                intent.putExtra("Category", mSelfTestCateArray[arg2]);
                startActivity(intent);
            }
        });
    }
    
    private void setupMTKSelfTestListview() {
        mMTKSelfTestCateList = (ListView) findViewById(R.id.mtk_self_test_list);
        if (FeatureOption.MTK_CTSC_MTBF_INTERNAL_SUPPORT == false) {
             mMTKSelfTestCateList.setVisibility(View.GONE);
             TextView mTitle = (TextView) findViewById(R.id.mtk_self_test_title);
             View mView = (View) findViewById(R.id.single_line2);
             mTitle.setVisibility(View.GONE);
             mView.setVisibility(View.GONE);
        }
        mMTKSelfTestCateArray= getResources().getStringArray(R.array.category_MTK_self_test);
        mMTKSelfTestCateAdapter = new ArrayAdapter<String>(this,
                android.R.layout.simple_list_item_1, mMTKSelfTestCateArray);
        mMTKSelfTestCateList.setAdapter(mMTKSelfTestCateAdapter);
        
        mMTKSelfTestCateList.setOnItemClickListener(new OnItemClickListener() {

            @Override
            public void onItemClick(AdapterView<?> arg0, View arg1, int arg2,
                    long arg3) {
                // TODO Auto-generated method stub
                CTSCLog.i(TAG, "onItemClick(): category = " + mMTKSelfTestCateArray[arg2]);
                
                Intent intent = new Intent(ManSelfTestActivity.this,
                        CheckResultActivity.class);
                intent.putExtra("Category", mMTKSelfTestCateArray[arg2]);
                startActivity(intent);
            }
        });
    } 

    private void setupOthersListview() {
        mOthersCateList = (ListView) findViewById(R.id.others_list);
        mOthersCateArray = getResources().getStringArray(R.array.category_others);
        mOthersCateAdapter = new ArrayAdapter<String>(this,
                android.R.layout.simple_list_item_1, mOthersCateArray);
        mOthersCateList.setAdapter(mOthersCateAdapter);
        
        mOthersCateList.setOnItemClickListener(new OnItemClickListener() {

            @Override
            public void onItemClick(AdapterView<?> arg0, View arg1, int arg2,
                    long arg3) {
                // TODO Auto-generated method stub
                CTSCLog.i(TAG, "onItemClick(): category = " + mOthersCateArray[arg2]);
                Intent intent = new Intent(ManSelfTestActivity.this, HelpActivity.class);
                intent.putExtra("TestType", "ManSelfTest");
                startActivity(intent);
            }
        });
    }
    
    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // TODO Auto-generated method stub  
        menu.add(0, MENU_CTSC_VER, 0, getString(R.string.menu_CTSC_ver));
        
        return super.onCreateOptionsMenu(menu);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // TODO Auto-generated method stub        
        switch (item.getItemId()) {
        case MENU_CTSC_VER:
            showVerionDlg(mCtscVerStr);
            break;
        default:
            break;
        }
        
        return super.onOptionsItemSelected(item);
    }
    
    private void showVerionDlg(String text) {
        final Builder builder = new AlertDialog.Builder(this);
        builder.setMessage(text);
        builder.setPositiveButton("OK",  null);
        builder.create().show();
    }
    
    public String getCTSCVersion() {
        PackageManager pm = getPackageManager();
        PackageInfo pInfo = null;
        try {
            pInfo = pm.getPackageInfo(getPackageName(), 0);
        } catch (NameNotFoundException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
        return pInfo.versionName;
    }
}
