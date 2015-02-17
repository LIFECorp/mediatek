package com.mediatek.configurecheck2;

import com.mediatek.configurecheck2.R;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.app.AlertDialog.Builder;

public class SubCategoryListActivity extends Activity {

	private static final String TAG = "SubCategoryListActivity";
	private ListView mSubCategoryListView;
	private String[] mSubCategoryArray;
	private ArrayAdapter<String> mSubCategoryAdapter;
	private String mRequiredCategory;
	private Boolean mIsServiceTest;
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
        CTSCLog.i(TAG, "onCreate()");
		super.onCreate(savedInstanceState);
		setContentView(R.layout.sub_category_list);
		
		setTitle(getIntent().getStringExtra("Category"));
		
		mSubCategoryListView = (ListView)findViewById(R.id.sub_category_list);
		mIsServiceTest = isServiceTestCategory();
		CTSCLog.i(TAG, "mIsServiceTest = " + mIsServiceTest);
		
		if (mIsServiceTest) {
			mSubCategoryArray = getResources().getStringArray(
					R.array.service_test_sub_category);
		} else {
			mSubCategoryArray = getResources().getStringArray(
	    			R.array.protocol_conformance_sub_category);
		}
        mSubCategoryAdapter = new ArrayAdapter<String>(this,
                android.R.layout.simple_list_item_1, mSubCategoryArray);
        mSubCategoryListView.setAdapter(mSubCategoryAdapter);
        
        mSubCategoryListView.setOnItemClickListener(new OnItemClickListener() {

            @Override
            public void onItemClick(AdapterView<?> arg0, View arg1, int arg2,
                long arg3) {
                // TODO Auto-generated method stub
                CTSCLog.i(TAG, "onItemClick(): category = " + mSubCategoryArray[arg2]);

                if (false == checkProviderIsEmpty(mSubCategoryArray[arg2])) {
                    Intent intent = new Intent(SubCategoryListActivity.this, CheckResultActivity.class);
                    intent.putExtra("Category", mSubCategoryArray[arg2]);
                    startActivity(intent);
                } else {
                    AlertDialog builder = new AlertDialog.Builder(SubCategoryListActivity.this)
                                                         .setMessage(getString(R.string.str_noitem_message))
                                                         .setPositiveButton(android.R.string.ok, null)
                                                         .create();
                    builder.show();
                }
            }
        });
    }

    private boolean checkProviderIsEmpty(String catogory) {
        CheckItemProviderBase provider = ProviderFactory.getProvider(this, catogory);

        int count = provider.getItemCount();

        if (0 == count) {
            return true;
        }

        return false;
    }

    public Boolean isServiceTestCategory() {

        Intent intent = getIntent();
        mRequiredCategory = intent.getStringExtra("Category");
        
        if (mRequiredCategory.equals(
        		getString(R.string.protocol_conformance_test))) {
        	return false;
		} else if (mRequiredCategory.equals(
				getString(R.string.service_test))){
			return true;
		} else {
			throw new RuntimeException("No such category!");
		}
	}
	
}
