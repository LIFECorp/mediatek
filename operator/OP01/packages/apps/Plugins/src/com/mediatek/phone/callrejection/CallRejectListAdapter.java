package com.mediatek.phone.callrejection;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.TextView;

import com.mediatek.op01.plugin.R;

import java.util.ArrayList;

public class CallRejectListAdapter extends BaseAdapter {
    //private static final String TAG = "CallRejectListAdapter";
    
    private LayoutInflater mInflater;
    private ArrayList<CallRejectListItem> mDataList;
    private Context mContext;
    private CheckSelectCallBack mCheckSelectCallBack = null;
    
    public CallRejectListAdapter(Context context, ArrayList<CallRejectListItem> data) {
        mContext = context;
        mDataList = data;
        mInflater = (LayoutInflater)mContext.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
    }
    

    @Override
    public View getView(final int position, View convertView, ViewGroup parent) {
        final ItemViewHolder holder;
        if (convertView == null) {
            holder = new ItemViewHolder();
            convertView = mInflater.inflate(R.layout.call_reject_list_item, null);

            holder.mName = (TextView)convertView.findViewById(R.id.call_reject_contact_name);
            holder.mCheckBox = (CheckBox)convertView.findViewById(R.id.call_reject_contact_check_btn);
            holder.mId = position;
            holder.mCheckBox.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
                @Override
                public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                    mDataList.get(holder.mId).setIsChecked(isChecked);
                    mCheckSelectCallBack.setChecked(isChecked);
                }
            });
            holder.mPhoneNum = (TextView)convertView.findViewById(R.id.call_reject_contact_phone_num);

            convertView.setTag(holder);
        } else {
            holder = (ItemViewHolder)convertView.getTag();
            holder.mId = position;
        }
        
        if (holder.mName != null) {
            holder.mName.setText(mDataList.get(position).getName());
        }
        if (holder.mCheckBox != null) {
            holder.mCheckBox.setChecked(mDataList.get(position).getIsChecked());
        }
        if (holder.mPhoneNum != null) {
            holder.mPhoneNum.setText(mDataList.get(position).getPhoneNum());
        }
        return convertView;
    }

    @Override
    public int getCount() {
        return mDataList != null ? mDataList.size() : 0;
    }

    @Override
    public Object getItem(int position) {
        return mDataList != null ? mDataList.get(position) : null;
    }
    
    @Override
    public long getItemId(int position) {
        return 0;
    }

    public void setCheckSelectCallBack(CheckSelectCallBack callBack) {
        mCheckSelectCallBack = callBack;
    }

    static class ItemViewHolder {
        TextView mName;
        TextView mPhoneNum;
        CheckBox mCheckBox;
        int mId;
    }

    public interface CheckSelectCallBack {
        void setChecked(boolean isChecked);
    }
}
