package com.mediatek.phone.callrejection;

/**
 * Assisted class for transfer contact data
 * @author MTK80906
 */
public class CallRejectListItem {
    private String mId;
    private String mName;
    private String mPhoneNum;
    private boolean mIsChecked;
    
    public CallRejectListItem() {
        mName = "";
        mPhoneNum = "";
        mId = "";
        mIsChecked = false;
    }
    
    public CallRejectListItem(String name, String phoneNum, String id, boolean isChecked) {
        this.mName = name;
        this.mPhoneNum = phoneNum;
        this.mId = id;
        this.mIsChecked = isChecked;
    }
    public String getId() {
        return mId;
    }

    public void setId(String id) {
        this.mId = id;
    }

    public String getName() {
        return mName;
    }

    public void setName(String name) {
        this.mName = name;
    }

    public String getPhoneNum() {
        return mPhoneNum;
    }

    public void setPhoneNum(String phoneNum) {
        this.mPhoneNum = phoneNum;
    }

    public boolean getIsChecked() {
        return mIsChecked;
    }

    public void setIsChecked(boolean isChecked) {
        this.mIsChecked = isChecked;
    }
}
