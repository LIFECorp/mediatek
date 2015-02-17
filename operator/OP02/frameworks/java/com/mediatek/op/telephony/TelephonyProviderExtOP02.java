package com.mediatek.op.telephony;

import android.util.Log;
import android.content.Context;
import android.content.ContentValues;
import android.provider.Telephony;
import com.mediatek.common.telephony.ITelephonyProviderExt;
import com.mediatek.internal.R;

public class TelephonyProviderExtOP02 extends TelephonyProviderExt {
    private Context mContext;
    private final String cuNum = "46001";
    private final String cuApnNet = "3gnet";
    private final String cuApnWap = "3gwap";

    public TelephonyProviderExtOP02(Context context) {
        mContext = context;
    }

    public int onLoadApns(ContentValues row) {
        if (row != null) {
            if ((row.get(Telephony.Carriers.NUMERIC) != null) &&
                    row.get(Telephony.Carriers.NUMERIC).equals(cuNum) &&
                    row.get(Telephony.Carriers.APN) != null) {

                if (row.get(Telephony.Carriers.APN).equals(cuApnNet)) {
                    row.put(Telephony.Carriers.NAME, mContext.getResources()
                            .getString(R.string.cu_3gnet_name));					
                    return 1;
                } else if (row.get(Telephony.Carriers.APN).equals(cuApnWap)) {
                    if (!((row.containsKey(Telephony.Carriers.TYPE) == true) &&
                            (row.get(Telephony.Carriers.TYPE) != null) &&
                            (row.get(Telephony.Carriers.TYPE).equals("mms")))) {
                        row.put(Telephony.Carriers.NAME, mContext
                                .getResources().getString(R.string.cu_3gwap_name));
                        return 1;
                    }
                }

            }
        }
        return 0;
    }
}
