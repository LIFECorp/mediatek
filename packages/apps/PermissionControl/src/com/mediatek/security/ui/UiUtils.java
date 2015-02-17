package com.mediatek.security.ui;

import java.util.ArrayList;
import java.util.List;

import android.content.ContentResolver;
import android.content.Context;
import android.database.ContentObserver;
import android.net.Uri;
import android.os.Handler;
import android.provider.Settings;

import com.mediatek.common.mom.IMobileManager;
import com.mediatek.common.mom.PermissionRecord;
import com.mediatek.common.mom.SubPermissions;
import com.mediatek.hotknot.HotKnotAdapter;
import com.mediatek.security.service.PermControlUtils;
import com.mediatek.xlog.Xlog;

public class UiUtils {
    private static final String TAG = "UiUtils";
    public static final String ACTION_SWITCH_OFF_CONTROL_FROM_SECURITY =
                          "com.mediatek.security.SWITCH_OFF_CONTROL_FROM_SECURITY";

    public static final String ACTION_SWITCH_OFF_CONTROL_FROM_APP_PERM = 
                          "com.mediatek.security.SWITCH_OFF_CONTROL_FROM_APP_PERM";

    public static final int RESULT_START_ACTIVITY = 0;
    public static final int RESULT_FINISH_ITSELF = 1;

    /**
     * This array stands for the permission status code,
     * the sort is defined by planner
     */
    public static final int[] PERM_STATUS_ARRAY = new int[] {
        IMobileManager.PERMISSION_STATUS_GRANTED,
        IMobileManager.PERMISSION_STATUS_CHECK,
        IMobileManager.PERMISSION_STATUS_DENIED
      };

    /*
     * get select index according to the permission status for dialog show
     * @param status int , permission status
     * @return the PERM_STATUS_ARRAY array index
     * */
    public static int getSelectIndex(int status) {
        int index = 1;
        for (int i = 0; i < PERM_STATUS_ARRAY.length; i++) {
            if (PERM_STATUS_ARRAY[i] == status) {
                index = i;
                break;
            }
        }
        return index;
    }

    /**
     * add content obsever to connect with other permission management app
     * 
     */
    public static abstract class SwitchContentObserver extends ContentObserver {

        public SwitchContentObserver(Handler handler) {
            super(handler);
        }

        public void register(ContentResolver contentResolver) {
            contentResolver.registerContentObserver(Settings.System
                    .getUriFor(PermControlUtils.PERMISSION_CONTROL_ATTACH),
                    false, this);
        }

        public void unregister(ContentResolver contentResolver) {
            contentResolver.unregisterContentObserver(this);
        }

        @Override
        public abstract void onChange(boolean selfChange, Uri uri);
    }
    
    
    /**
     * Get the filtered  permission record list
     * @param context, Context
     * @param originPermList, the list need to filter
     * @return the list of filtered permission record
     */
    public static List<PermissionRecord> filterPermList(Context context,List<PermissionRecord> originPermList) { 
        if (originPermList != null) {
       	    List<PermissionRecord> list = new ArrayList<PermissionRecord>();
        	for (PermissionRecord permRecord : originPermList) {
        		if (permRecord.mPermissionName.equals(SubPermissions.HOTKNOT_BIND)) {
            		if (HotKnotAdapter.getDefaultAdapter(context) != null) {
            			Xlog.d(TAG,"add HotKnot permission");
            			list.add(permRecord);
            		}
        		}  else {
        			list.add(permRecord);
        		}
        	}
            return list;
        } else {
            return null;
        }
    }
}
