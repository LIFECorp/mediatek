package com.mediatek.settings.plugin;

import android.content.ContentUris;
import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.net.Uri;
import android.provider.Telephony;

import com.mediatek.settings.ext.DefaultReplaceApnProfile;

import com.mediatek.xlog.Xlog;

public class OP02ReplaceApnProfileExt extends DefaultReplaceApnProfile {
    private static final String TAG = "OP02ReplaceApnProfileExt";

    // -1 stands for the apn inserted fail
    private static final long APN_NO_UPDATE = -1;

    private static final String CU_NUMERIC = "46001";

    /**
     * Check if the same name(apn item)exists, if it exists, replace it
     * @param uri to access database
     * @param apn profile apn
     * @param apnId profile apn id
     * * @param name profile carrier name
     * @param values new profile values to update
     * @param numeric selected numeric
     * returns the replaced profile id
     */
    @Override
    public long replaceApn(Context context, Uri uri, String apn, String apnId, String name,
            ContentValues values, String numeric) {
        long numReplaced = APN_NO_UPDATE;
        Xlog.d(TAG,"params: apn = " + apn + " numeric = " + numeric + " apnId = " + apnId);
        if (CU_NUMERIC.equals(numeric)) {
            String where = "numeric=\"" + numeric + "\"";
            Cursor cursor = null;
            try {
                cursor = context.getContentResolver().query(uri,
                        new String[] {  Telephony.Carriers._ID, Telephony.Carriers.APN }, where, null,
                        Telephony.Carriers.DEFAULT_SORT_ORDER);
                if (cursor == null || cursor.getCount() == 0) {
                    Xlog.d(TAG, "cu card ,cursor is null ,return");
                    return APN_NO_UPDATE;
                }
                cursor.moveToFirst();
                while (!cursor.isAfterLast()) {
                    Xlog.d(TAG, "apn = " + apn + " getApn = " + cursor.getString(1));
                    if (apn.equals(cursor.getString(1))) {
                        numReplaced = Integer.parseInt(cursor.getString(0));
                        Uri newUri = ContentUris.withAppendedId(uri,numReplaced);
                        context.getContentResolver().update(newUri, values, null, null);
                        break;
                    }
                    cursor.moveToNext();
                }
            } finally {
                if (cursor != null) {
                    cursor.close();
                }
            }
        } else {
            numReplaced = super.replaceApn(context, uri, apn, apnId, name, values,
                    numeric);
        }
        return numReplaced;
    }
}
