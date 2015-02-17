/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * MediaTek Inc. (C) 2010. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.backuprestore.modules;

public class SmsXmlInfo {
    public static class SmsXml {
        public static final String RECORD = "record";

        public static final String CATEGORY = "category";
        public static final String ID = "_id";
        public static final String ISREAD = "isread";
        public static final String LOCALDATE = "local_date";
        public static final String ST = "st";
        public static final String MSGBOX = "msg_box";
        public static final String DATE = "date";
        public static final String SIZE = "m_size";
        public static final String SEEN = "seen";
        public static final String SIMID = "sim_id";
    }

    private String mCategory;
    private String mId;
    private String mIsRead;
    private String mLocalDate;
    private String mST;
    private String mMsgBox;
    private String mDate;
    private String mSize;
    private String mSeen;
    private String mSimID;

    public void setCategory(String category) {
        mCategory = category;
    }

    public String getCategory() {
        return (mCategory == null) ? "0" : mCategory;
    }

    public void setID(String id) {
        mId = id;
    }

    public String getID() {
        return (mId == null) ? "" : mId;
    }

    public void setIsRead(String isread) {
        mIsRead = isread;
    }

    public String getIsRead() {
        return (mIsRead == null) ? "0" : mIsRead;
    }

    public void setLocalDate(String date) {
        mLocalDate = date;
    }

    public String getLocalDate() {
        return (mLocalDate == null) ? "" : mLocalDate;
    }

    public void setST(String st) {
        mST = st;
    }

    public String getST() {
        return (mST == null) ? "0" : mST;
    }

    public void setMsgBox(String msgBox) {
        mMsgBox = msgBox;
    }

    public String getMsgBox() {
        if (mMsgBox != null && Integer.parseInt(mMsgBox) > 4) {
            return null;
        } else {
            return (mMsgBox == null) ? "1" : mMsgBox;
        }
    }

    public void setDate(String date) {
        mDate = date;
    }

    public String getDate() {
        return (mDate == null) ? "" : mDate;
    }

    public void setSize(String size) {
        mSize = size;
    }

    public String getSize() {
        return (mSize == null) ? "0" : mSize;
    }

    public void setSeen(String seen) {
        mSeen = seen;
    }

    public String getSeen() {
        return (mSeen == null) ? "0" : mSeen;
    }

    public void setSimID(String simId) {
        mSimID = simId;
    }

    public String getSimID() {
        return (mSimID == null) ? "-1" : mSimID;
    }
}
