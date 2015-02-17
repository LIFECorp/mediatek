package com.hissage.pushinfo;

public class NmsPushInfo implements Comparable<NmsPushInfo> {

    public static final int PUSH_INFO_STATUS_DEFAULT = 0;
    public static final int PUSH_INFO_STATUS_CLICKED = 1;
    public static final int PUSH_INFO_STATUS_SHOW = 2;
    public static final int PUSH_INFO_STATUS_CANCEL = 3;
    public static final int PUSH_INFO_STATUS_DIABLE = 4;
    public static final int PUSH_INFO_STATUS_ERROR = 5;

    public int _id;
    public String pushInfoID;
    public String pushInfoContent;
    public String pushTitle;
    public String pushPicUrl;
    public String pushInfoUrl;
    public String pushShowTime;
    public String pushExpireTime;
    public int pushInteractionType = -1;
    public int pushShowType = -1;
    public int pushMsgType = -1;
    public int pushInfoPriority = -1;
    public int pushInfoStatus = -1;
    public int randomRange = 0;
    public int infoEnable = -1;
    public String infoOpratertime;
    public int operaterType;
    
    public String infoOpShowTime;
    public String infoReceiveTime;
    public String infoTimeZone;
    public String timeZoneID;

    @Override
    public int compareTo(NmsPushInfo pushInfo) {
        // TODO Auto-generated method stub
        if (this.pushInfoPriority < pushInfo.pushInfoPriority)
            return -1;
        else if (this.pushInfoPriority > pushInfo.pushInfoPriority)
            return 1;
        return 0;

    }
}
