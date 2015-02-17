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

import android.util.Xml;

import org.xmlpull.v1.XmlSerializer;

import com.mediatek.backuprestore.utils.Constants.ModulePath;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStreamWriter;
import java.io.StringWriter;
import java.io.Writer;

/**
 * Describe class NoteBookXmlComposer here.
 * 
 * @author
 * @version 1.0
 */
public class NoteBookXmlComposer {
    private XmlSerializer mSerializer = null;
    private Writer mWriter = null;
    private File mFile;

    /**
     * Creates a new <code>NoteBookXmlComposer</code> instance.
     * 
     */
    public NoteBookXmlComposer() {
    }

    public boolean startCompose(File path) throws IOException {
        boolean result = false;
        mSerializer = Xml.newSerializer();
        mFile = new File(path + File.separator + ModulePath.NOTEBOOK_XML);
        try {
            mWriter = new BufferedWriter(new OutputStreamWriter(new FileOutputStream(mFile)));
            mSerializer.setOutput(mWriter);
            // serializer.startDocument("UTF-8", null);
            mSerializer.startDocument(null, false);
            mSerializer.startTag("", NoteBookXmlInfo.ROOT);
            result = true;
        } catch (IOException e) {
            e.printStackTrace();
            throw e;
        } catch (IllegalArgumentException e) {
            e.printStackTrace();
        } catch (IllegalStateException e) {
            e.printStackTrace();
        }
        return result;
    }

    public boolean endCompose() throws IOException {
        boolean result = false;
        try {
            mSerializer.endTag("", NoteBookXmlInfo.ROOT);
            mSerializer.endDocument();
            result = true;
        } catch (IllegalArgumentException e) {
            e.printStackTrace();
        } catch (IllegalStateException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
            throw e;
        } finally {
            if (mWriter != null) {
                mWriter.close();
            }
        }

        return result;
    }

    public boolean addOneMmsRecord(NoteBookXmlInfo record) throws IOException {
        boolean result = false;
        try {
            mSerializer.startTag("", NoteBookXmlInfo.RECORD);
            if (record.getTitle() != null) {
                mSerializer.attribute("", NoteBookXmlInfo.TITLE, record.getTitle());
            }

            if (record.getNote() != null) {
                mSerializer.attribute("", NoteBookXmlInfo.NOTE, record.getNote());
            }

            if (record.getCreated() != null) {
                mSerializer.attribute("", NoteBookXmlInfo.CREATED, record.getCreated());
            }

            if (record.getModified() != null) {
                mSerializer.attribute("", NoteBookXmlInfo.MODIFIED, record.getModified());
            }

            if (record.getNoteGroup() != null) {
                mSerializer.attribute("", NoteBookXmlInfo.NOTEGROUP, record.getNoteGroup());
            }

            mSerializer.endTag("", NoteBookXmlInfo.RECORD);

            result = true;
        } catch (IllegalArgumentException e) {
            e.printStackTrace();
        } catch (IllegalStateException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
            throw e;
        }

        return result;
    }
}
