package com.mediatek.op.bootanim;

import android.util.Log;
import com.mediatek.op.bootanim.DefaultBootAnimExt;
/**
 * Interface that defines all methos which are implemented in ConnectivityService
 * {@hide} 
 */
public class BootAnimExtOP02 extends DefaultBootAnimExt {

    private static final String TAG = "BootAnimExt";

    public BootAnimExtOP02() {
        Log.d(TAG,"BootAnimExtOP02 Contrustor !");
    }

    @Override
    public int getScreenTurnOffTime() {
        // op01 7*1000
        // 0p02 5*1000
        return 5 * 1000;
    }


    @Override
    public boolean isCustBootAnim() {
        return true;
    }
}
