package com.mediatek.op.telephony;

public class PhoneNumberExtOP01 extends PhoneNumberExt {

    public boolean isCustomizedEmergencyNumber(String number, String plusNumber, String numberPlus) {
        // Customized ecc number
        final int eccNo = 7;
        String []emergencyNumList = {"000", "08", "110", "118", "119", "999", "120"};
        for (int i = 0; i < eccNo; i++) {
            numberPlus = emergencyNumList[i] + "+";
            if (emergencyNumList[i].equals(number)
                || numberPlus.equals(number)) {
                return true;
            }
        }
        return false;
    }

    public boolean isSpecialEmergencyNumber(String dialString) {
        /* These special emergency number will show ecc in MMI but sent to nw as normal call */
        return (dialString.equals("110") || dialString.equals("119")
                || dialString.equals("000") || dialString.equals("08")
                || dialString.equals("118") || dialString.equals("999")
                || dialString.equals("120"));
    }

    public boolean isCustomizedEmergencyNumberExt(String number, String plusNumber, String numberPlus) {
        // Customized ecc number when SIM card is not inserted
        String []emergencyNumList = {"112", "911", "000", "08", "110", "118", "119", "999", "120"};

        for (String emergencyNum : emergencyNumList) {
            numberPlus = emergencyNum + "+";
            if (emergencyNum.equals(number)
                 || numberPlus.equals(number)) {
                return true;
            }
        }

        return false;
    }

    public boolean isPauseOrWait(char c) {
        return (c == 'p' || c == 'P' || c == 'w' || c == 'W');
    }
}
