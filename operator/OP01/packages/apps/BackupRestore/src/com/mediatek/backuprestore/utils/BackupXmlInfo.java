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
package com.mediatek.backuprestore.utils;

import java.util.Date;

public class BackupXmlInfo {
    // private Date backupdate = null;
    private String mBackupdate = null;
    private String mDevicetype = null;
    private String mSystem = null;
    private int mContactNum = 0;
    private int mSmsNum = 0;
    private int mMmsNum = 0;
    private int mCalendarNum = 0;
    private int mAppNum = 0;
    private int mPictureNum = 0;
    private int mMusicNum = 0;
    private int mNotebookNum = 0;

    public void setBackupDate(Date date) {

    }

    public void setBackupDate(String dateString) {
        mBackupdate = dateString;
    }

    public String getBackupDateString() {
        return mBackupdate;
    }

    public void setDevicetype(String type) {
        mDevicetype = type;
    }

    public String getDevicetype() {
        return mDevicetype;
    }

    public void setSystem(String sys) {
        mSystem = sys;
    }

    public String getSystem() {
        return mSystem;
    }

    public void setContactNum(int num) {
        mContactNum = num;
    }

    public int getContactNum() {
        return mContactNum;
    }

    public void setSmsNum(int num) {
        mSmsNum = num;
    }

    public int getSmsNum() {
        return mSmsNum;
    }

    public void setMmsNum(int num) {
        mMmsNum = num;
    }

    public int getMmsNum() {
        return mMmsNum;
    }

    public void setCalendarNum(int num) {
        mCalendarNum = num;
    }

    public int getCalendarNum() {
        return mCalendarNum;
    }

    public void setAppNum(int num) {
        mAppNum = num;
    }

    public int getAppNum() {
        return mAppNum;
    }

    public void setPictureNum(int num) {
        mPictureNum = num;
    }

    public int getPictureNum() {
        return mPictureNum;
    }

    public void setMusicNum(int num) {
        mMusicNum = num;
    }

    public int getMusicNum() {
        return mMusicNum;
    }

    public void setNoteBookNum(int num) {
        mNotebookNum = num;
    }

    public int getNoteBookNum() {
        return mNotebookNum;
    }

    public int getTotalNum() {
        return mContactNum + mSmsNum + mMmsNum + mCalendarNum + mAppNum + mPictureNum + mMusicNum + mNotebookNum;
    }
}
