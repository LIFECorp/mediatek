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

import android.util.Log;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;
import org.xmlpull.v1.XmlPullParserFactory;

import java.io.IOException;
import java.io.StringReader;

public class BackupXmlParser {
    static final String LOGTAG = "Backupxmlparser";

    static final int CONTACT = 0X01;
    static final int SMS = 0X02;
    static final int MMS = 0X04;
    static final int CALENDAR = 0X08;
    static final int APP = 0X10;
    static final int PICTURE = 0X20;
    static final int MUSIC = 0X80;
    static final int NOTEBOOK = 0X100;

    public static BackupXmlInfo parse(String backString) {
        Log.i(LOGTAG, "TomorrowWeatherPullParse.parse");
        int curValue = 0;
        BackupXmlInfo info = new BackupXmlInfo();

        try {
            XmlPullParserFactory factory = XmlPullParserFactory.newInstance();
            XmlPullParser parser = factory.newPullParser();
            parser.setInput(new StringReader(backString));

            int eventType = parser.getEventType();

            while (eventType != XmlPullParser.END_DOCUMENT) {
                switch (eventType) {
                case XmlPullParser.START_DOCUMENT:
                    break;

                case XmlPullParser.START_TAG:
                    String tagName = parser.getName();

                    if (tagName.equals(BackupXml.BACKUPDATE)) {
                        String backupdate = parser.nextText();
                        info.setBackupDate(backupdate);
                        Log.i(LOGTAG, "backupdate = " + backupdate);
                    } else if (tagName.equals(BackupXml.COMPONENT)) {
                        int attrNum = parser.getAttributeCount();
                        Log.i(LOGTAG, "attributeCount = " + attrNum);
                        for (int i = 0; i < attrNum; i++) {
                            String name = parser.getAttributeName(i);
                            String value = parser.getAttributeValue(i);
                            if (name.equals(BackupXml.ID)) {
                                if (value.equals(BackupXml.CONTACTS)) {
                                    curValue = CONTACT;
                                } else if (value.equals(BackupXml.SMS)) {
                                    curValue = SMS;
                                } else if (value.equals(BackupXml.MMS)) {
                                    curValue = MMS;
                                } else if (value.equals(BackupXml.CALENDAR)) {
                                    curValue = CALENDAR;
                                } else if (value.equals(BackupXml.APP)) {
                                    curValue = APP;
                                } else if (value.equals(BackupXml.PICTURE)) {
                                    curValue = PICTURE;
                                } else if (value.equals(BackupXml.MUSIC)) {
                                    curValue = MUSIC;
                                } else if (value.equals(BackupXml.NOTEBOOK)) {
                                    curValue = NOTEBOOK;
                                }
                            }

                            Log.i(LOGTAG, "attributeName" + i + " = " + parser.getAttributeName(i));
                            Log.i(LOGTAG, "attributeValue" + i + " = " + parser.getAttributeValue(i));
                        }
                    } else if (tagName.equals(BackupXml.COUNT)) {
                        int count = Integer.parseInt(parser.nextText());
                        Log.i(LOGTAG, "count = " + count);
                        switch (curValue) {
                        case CONTACT:
                            info.setContactNum(count);
                            break;

                        case SMS:
                            info.setSmsNum(count);
                            break;

                        case MMS:
                            info.setMmsNum(count);
                            break;

                        case CALENDAR:
                            info.setCalendarNum(count);
                            break;

                        case APP:
                            info.setAppNum(count);
                            break;

                        case PICTURE:
                            info.setPictureNum(count);
                            break;

                        case MUSIC:
                            info.setMusicNum(count);
                            break;

                        case NOTEBOOK:
                            info.setNoteBookNum(count);
                            break;
                        default:
                            break;
                        }

                        curValue = 0;
                    }
                    break;

                case XmlPullParser.END_TAG:
                    break;

                case XmlPullParser.END_DOCUMENT:
                    break;
                default:
                    break;
                }

                // use next to process next event
                eventType = parser.next();
            }

        } catch (XmlPullParserException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        }

        return info;
    }
}
