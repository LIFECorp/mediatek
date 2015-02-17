package com.mediatek.contacts.plugin;

import android.provider.ContactsContract.CommonDataKinds.Phone;
import android.util.Log;

import com.mediatek.contacts.ext.ContactPluginDefault;
import com.mediatek.contacts.ext.QuickContactExtension;

public class OP01QuickContactExtension extends QuickContactExtension {
    private static final String TAG = "OP01QuickContactExtension";
    
    @Override
    public boolean collapseListPhone(final String mimeType, String commd) {
        if (! ContactPluginDefault.COMMD_FOR_OP01.equals(commd)) {
            return false;
        }
        Log.i(TAG, "[collapseListPhone()]"); 
        if (Phone.CONTENT_ITEM_TYPE.equals(mimeType)) {
            return false;
        }
        return true;
    }
}
