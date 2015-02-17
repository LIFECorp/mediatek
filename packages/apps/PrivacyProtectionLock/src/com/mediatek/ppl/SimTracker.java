package com.mediatek.ppl;

import android.content.Context;

import com.mediatek.telephony.TelephonyManagerEx;

import java.util.Arrays;

/**
 * Utility class to track SIM states.
 */
public class SimTracker {
    private static final String TAG = "PPL/SimTracker";

    public final int slotNumber;
    private Context mContext;
    boolean inserted[];
    String serialNumbers[];
    int states[];

    public SimTracker(int number, Context context) {
        slotNumber = number;
        mContext = context;
        inserted = new boolean[slotNumber];
        serialNumbers = new String[slotNumber];
        states = new int[slotNumber];
    }

    /**
     * Take a snapshot of the current SIM information in system and store it in this object.
     */
    public synchronized void takeSnapshot() {
        if (!PlatformManager.isTelephonyReady(mContext)) {
            return;
        }

        TelephonyManagerEx telephonyManagerEx = new TelephonyManagerEx(mContext);
        for (int i = 0; i < slotNumber; ++i) {
            if (telephonyManagerEx.hasIccCard(i)) {
                inserted[i] = true;
                serialNumbers[i] = telephonyManagerEx.getSimSerialNumber(i);
                if ("".equals(serialNumbers[i])) {
                    serialNumbers[i] = null;
                }
                states[i] = telephonyManagerEx.getSimState(i);
            } else {
                inserted[i] = false;
            }
        }
    }

    /**
     * Get a list of the IDs of current inserted SIM cards.
     * 
     * @return
     */
    public synchronized int[] getInsertedSim() {
        int[] result = new int[inserted.length];
        int count = 0;
        for (int i = 0; i < inserted.length; ++i) {
            if (inserted[i]) {
                result[count++] = i;
            }
        }
        return Arrays.copyOf(result, count);
    }

    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder("SimTracker: ");
        for (int i = 0; i < slotNumber; ++i) {
            sb.append("{")
              .append(inserted[i])
              .append(", ")
              .append(serialNumbers[i])
              .append(", ")
              .append(states[i])
              .append("}, ");
        }
        return sb.toString();
    }
}
