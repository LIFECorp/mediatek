package com.mediatek.configurecheck2;

import java.util.ArrayList;

import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;

/**
 * 
 * @author mtk80202
 *
 */

abstract class CheckItemBase {

    private static String TAG = "Base";

	public enum check_result {
        UNKNOWN, RIGHT, WRONG
	};
	public static final String NOTIFY_ACTIVITY_ACTION = "mediatek.configcheck.NOTIFY_ACTIVITY";
	
	private static final String PREF_SAVED_RESULT = "checkitem_save_result";
	
	protected final int PROPERTY_CLEAR      = 0x00000000;
	protected final int PROPERTY_AUTO_CHECK = 0x00000001;
	protected final int PROPERTY_AUTO_CONFG = 0x00000002;
    protected final int PROPERTY_AUTO_FWD   = 0x00000004;
	
	private int mProperty = PROPERTY_CLEAR;

/*
    +---------------------------+         +----------------+
    |  +-------+                | press   |   mNote .....  |
	|  |mResult|  mTitle ...    | item    |  +----------+  |
	|  |  V/X  |     mValue ... |  =====> |  |    OK    |  |
	|  +-------+                |         |  +----------+  |
	+---------------------------+         +----------------+ 
*/	
    private Context mContext = null;
    private String mKey = null;
    protected String mTitle = null;
    protected String mValue = null;
    protected String mNote = null;
    protected check_result mResult = check_result.UNKNOWN;
	
	/**
	 * @Prompt please be sure setting Title and note string in your CheckItem constructor !
	 */
    CheckItemBase(Context context, String key) {
        if (null == context || null == key || key.isEmpty()) {
            throw new IllegalArgumentException("context=" + context + ", key="
                    + key);
        }
        mContext = context;
        mKey = key;
		mTitle = new String();
		mNote  = new String();
	}
	
    CheckItemBase(Context context, String key, String title, String note) {
        if (null == context || null == key || key.isEmpty()) {
            throw new IllegalArgumentException("context=" + context + ", key="
                    + key);
        }
        mContext = context;
        mKey = key;
		mTitle = title;
        mNote = note;
    }

    protected Context getContext() {
        return mContext;
	}
	
	/*
	 * Unique identification for a check item.
	 */
	public String getKey() {
		return mKey;
	}
	
    protected void setTitle(int id) {
        CTSCLog.i(TAG, "Key=" + mKey + ", setTitle(id)=" + mContext.getString(id));
        mTitle = mContext.getString(id);
    }

	protected void setTitle(String title) {
        CTSCLog.i(TAG, "Key=" + mKey + ", setTitle=" + title);
		mTitle = title;
	}
	
	public String getTitle() {
		return mTitle;
	}
	
    protected void setNote(int id) {
        CTSCLog.i(TAG, "Key=" + mKey + ", setNote(id)=" + mContext.getString(id));
        mNote = mContext.getString(id);
    }
    
	protected void setNote(String note) {
        CTSCLog.i(TAG, "Key=" + mKey + ", setNote=" + note);
		mNote = note;
	}
	
	public String getNote() {
		return mNote;
	}
	
    public void setValue(int id) {
        CTSCLog.i(TAG, "Key=" + mKey + ", setValue(id)=" + mContext.getString(id));
        mValue = mContext.getString(id);
    }
    
    
	public void setValue(String value) {
        CTSCLog.i(TAG, "Key=" + mKey + ", setValue=" + value);
		mValue = value;
	}
	
	public String getValue() {
		return mValue;
	}
	
	protected void setProperty(int property) {
        CTSCLog.i(TAG, "Key=" + mKey + ", setProperty=" + property);
		mProperty = property;
	}
	
	protected int getProperty() {
		return mProperty;
	}
	
	public boolean isCheckable() {
		return (mProperty & PROPERTY_AUTO_CHECK) > 0 ? true : false;
	}
	
	public boolean isConfigurable() {
		return (mProperty & PROPERTY_AUTO_CONFG) > 0 ? true : false;
	}

    public boolean isForwardable() {
        return (mProperty & PROPERTY_AUTO_FWD) > 0 ? true : false;
    }
    
    public Intent getForwardIntent() {
        return null;
    }

	/**
	 * @prompt Implement your check function here.
	 *         Default behavior is get result form local preference file.
	 * @param  c [in] App's context
	 * @return check_result
	 * 		   RIGHT: auto check pass, match SOP.
	 * 		   WRONG: auto check fail, not match SOP.
	 * 		   UNKNOWN: can't auto check, need users check by themselves.
	 */
	public check_result getCheckResult() {
		
	    if (!isCheckable()) {
        SharedPreferences pref = mContext.getSharedPreferences(
                PREF_SAVED_RESULT, Context.MODE_PRIVATE);
		int ret = pref.getInt(mKey, check_result.UNKNOWN.ordinal());
        mResult = check_result.values()[ret];
	        CTSCLog.i(TAG, "Key=" + mKey + ", result in pref =" + mResult);
	    }

        CTSCLog.i(TAG, "Key=" + mKey + ", getCheckResult=" + mResult);
		return mResult;
	}
	
	/**
	 * 
     * @param c
     *            [in] App's context for create SharedPreferences.
     * @param result
     *            [in] check result
	 * @return set result
	 * 
	 */
    public boolean setCheckResult(check_result result) {
	    
		if (isCheckable()) {
            // Not allow set result for auto check item.
            return false;
		}
		
		mResult = result;
		
        SharedPreferences pref = mContext.getSharedPreferences(
                PREF_SAVED_RESULT, Context.MODE_PRIVATE);
        SharedPreferences.Editor editor = pref.edit();
		
		if (result.equals(check_result.UNKNOWN)) {
			editor.remove(mKey);
		} else {
			editor.putInt(mKey, result.ordinal());
		}
	
		return editor.commit();
	}
	
    public boolean onCheck() {
        CTSCLog.i(TAG, "Key=" + mKey + ", onCheck()");
        return true;
    }
    
	/**
     * @prompt Implement your set configure function here
	 * @return reset result.
	 */
    public boolean onReset() {
        if (!isConfigurable()) {
            throw new IllegalArgumentException("Not configurable check item !!!");
        }
        
        return true;
    }
	
    protected void sendBroadcast() {
	    Intent intent = new Intent();
	    intent.setAction(NOTIFY_ACTIVITY_ACTION);
        mContext.sendBroadcast(intent);
	}
}

public abstract class CheckItemProviderBase {
	
	protected ArrayList<CheckItemBase> mArrayItems;
	
	CheckItemProviderBase() {
		mArrayItems = new ArrayList<CheckItemBase>();
	}
	
	public int getItemCount() {
		return mArrayItems.size();
	}
	
    public CheckItemBase getCheckItem(int index) {
		return mArrayItems.get(index);
	}

}
