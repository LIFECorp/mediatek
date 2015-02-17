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
import android.widget.AdapterView.OnItemClickListener;

public class CMCCSendTestActivity extends Activity {

    private static final String TAG = "CMCCSendTestActivity";
    private String mCtscVerStr;
    private ListView mSendTestCateList;
    private String[] mSendTestCateArray;
    private ArrayAdapter<String> mSendTestCateAdapter;
    private ListView mOthersCateList;
    private String[] mOthersCateArray;
    private ArrayAdapter<String> mOthersCateAdapter;
    
    private static final int MENU_CTSC_VER = 1;
    
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        // TODO Auto-generated method stub
        CTSCLog.i(TAG, "onCreate()");
        super.onCreate(savedInstanceState);
        setContentView(R.layout.send_test);
        
        mCtscVerStr = getCTSCVersion();
        mCtscVerStr = String.format(getString(R.string.CTSC_version), mCtscVerStr);
        
        setupSendTestListview();
        setupOthersListview();
    }
    
    private void setupSendTestListview() {
        mSendTestCateList = (ListView) findViewById(R.id.CMCC_send_test_list);
        mSendTestCateArray = getResources().getStringArray(R.array.category_CMCC_send_test);
        mSendTestCateAdapter = new ArrayAdapter<String>(this,
                android.R.layout.simple_list_item_1, mSendTestCateArray);
        mSendTestCateList.setAdapter(mSendTestCateAdapter);
        
        mSendTestCateList.setOnItemClickListener(new OnItemClickListener() {

            @Override
            public void onItemClick(AdapterView<?> arg0, View arg1, int arg2,
                    long arg3) {
                // TODO Auto-generated method stub
                CTSCLog.i(TAG, "onItemClick(): category = " + mSendTestCateArray[arg2]);
                
                Intent intent = new Intent(CMCCSendTestActivity.this,
                        CheckResultActivity.class);
                intent.putExtra("Category", mSendTestCateArray[arg2]);
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
                Intent intent = new Intent(CMCCSendTestActivity.this, HelpActivity.class);
                intent.putExtra("TestType", "CMCCSendTest");
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
