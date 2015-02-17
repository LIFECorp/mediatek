package com.mediatek.op.telephony;

import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.net.Uri;
import android.provider.Telephony;
import android.util.Log;
import com.android.internal.telephony.dataconnection.ApnSetting;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.mediatek.common.telephony.IGsmDCTExt;
import com.mediatek.common.telephony.IApnSetting;
import com.mediatek.common.featureoption.FeatureOption;


public class GsmDCTExtOP02 extends GsmDCTExt {
    private Context mContext;

    public GsmDCTExtOP02(Context context) {
        mContext = context;
    }

    private boolean isCuImsi(String imsi){

        if(imsi != null){
            int mcc = Integer.parseInt(imsi.substring(0,3));
            int mnc = Integer.parseInt(imsi.substring(3,5));
                  
            if(mcc == 460 && mnc == 01){
               return true;
            }
            
            if (mcc == 001) {
                return true;
            }
       }

       return false;
    }

    public Cursor getOptPreferredApn(String imsi, String operator, int simID) {
        log("getOptPreferredApn imsi:" + imsi + ", operator: " + operator);
        if (!isCuImsi(imsi)) {
            return null;
        }
        Uri baseUri = Telephony.Carriers.CONTENT_URI;
        if (FeatureOption.MTK_GEMINI_SUPPORT) {
            if (simID == PhoneConstants.GEMINI_SIM_2) {
                baseUri = Telephony.Carriers.SIM2Carriers.CONTENT_URI;
            } else {
                baseUri = Telephony.Carriers.SIM1Carriers.CONTENT_URI;
            } 
        }
        ContentResolver resolver = mContext.getContentResolver();
        String selection = Telephony.Carriers.APN + "=" + "'3gnet'" + " and " +
            Telephony.Carriers.TYPE + "!='mms'" +" and " + Telephony.Carriers.NUMERIC + " = '"+ operator + "'" + " and " +
            Telephony.Carriers.CARRIER_ENABLED + " = 1";
        Cursor cursor = resolver.query(baseUri, null, selection, null, null);
        if (cursor == null || !cursor.moveToFirst()) {
            if (cursor != null) cursor.close();
            return null;
        }

        Uri uri = Uri.withAppendedPath(baseUri, "preferapn_no_update");
        ContentValues values = new ContentValues();
        long id = cursor.getLong(cursor.getColumnIndexOrThrow(Telephony.Carriers._ID));
        log("set prefer apn for cu " + id);
        values.put("apn_id", id);
        resolver.insert(uri, values);

        return cursor;
    }
}
