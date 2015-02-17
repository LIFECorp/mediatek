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

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.zip.ZipEntry;
import java.util.zip.ZipException;
import java.util.zip.ZipFile;
import java.util.zip.ZipInputStream;
import java.util.zip.ZipOutputStream;

public class BackupZip {
    private static final String CLASS_TAG = MyLogger.LOG_TAG + "/BackupZip";

    private String mZipFile;

    public BackupZip(String zipfile) throws IOException {
        createZipFile(zipfile);
        mZipFile = zipfile;
    }

    public static List<String> getFileList(String zipFileString, boolean bContainFolder, boolean bContainFile,
            String tmpString) throws IOException {

        MyLogger.logI(CLASS_TAG, "GetFileList");
        List<String> fileList = new ArrayList<String>();
        String szName = "";

        try {
            final ZipFile zipFile = new ZipFile(zipFileString);
            MyLogger.logI(CLASS_TAG, "privateZip.size()=" + zipFile.size());

            for (final ZipEntry zipEntry : Collections.list(zipFile.entries())) {
                szName = zipEntry.getName();
                MyLogger.logD(CLASS_TAG, szName);
                if (zipEntry.isDirectory()) {
                    // get the folder name of the widget
                    szName = szName.substring(0, szName.length() - 1);
                    if (bContainFolder) {
                        if (tmpString == null) {
                            fileList.add(szName);
                        } else if (szName.matches(tmpString)) {
                            fileList.add(szName);
                        }
                    }

                } else {
                    if (bContainFile) {
                        if (tmpString == null) {
                            fileList.add(szName);
                        } else if (szName.matches(tmpString)) {
                            fileList.add(szName);
                        }
                    }
                }
            }

            zipFile.close();
        } catch (ZipException e) {
            e.printStackTrace();
            MyLogger.logI(CLASS_TAG, "ZipException ,The file not a zip file or damaged");
            return null;
        }

        if (fileList.size() > 0) {
            Collections.sort(fileList);
            Collections.reverse(fileList);
        }
        return fileList;
    }

    public static String readFile(String zipFileString, String fileString) {
        MyLogger.logI(CLASS_TAG, "getFile");
        ByteArrayOutputStream baos = null;
        String content = null;
        try {
            ZipFile zipFile = new ZipFile(zipFileString);
            ZipEntry zipEntry = zipFile.getEntry(fileString);
            if (zipEntry != null) {
                InputStream is = zipFile.getInputStream(zipEntry);
                baos = new ByteArrayOutputStream();
                int len = -1;
                byte[] buffer = new byte[512];
                while ((len = is.read(buffer, 0, 512)) != -1) {
                    baos.write(buffer, 0, len);
                }
                content = baos.toString();
                is.close();
            }

            zipFile.close();
        } catch (IOException e) {
            e.printStackTrace();
            return null;
        }
        return content;
    }

    public static byte[] readFileContent(String zipFileString, String fileString) {
        MyLogger.logI(CLASS_TAG, "getFile");
        ByteArrayOutputStream baos = null;
        try {
            ZipFile zipFile = new ZipFile(zipFileString);
            ZipEntry zipEntry = zipFile.getEntry(fileString);
            if (zipEntry != null) {
                InputStream is = zipFile.getInputStream(zipEntry);
                baos = new ByteArrayOutputStream();
                int len = -1;
                byte[] buffer = new byte[512];
                while ((len = is.read(buffer, 0, 512)) != -1) {
                    baos.write(buffer, 0, len);
                }
                is.close();
            }

            zipFile.close();
        } catch (IOException e) {
            e.printStackTrace();
            return null;
        }

        return baos.toByteArray();
    }

    public static ZipFile getZipFileFromFileName(String zipFileName) throws IOException {
        return new ZipFile(zipFileName);
    }

    public static String readFile(ZipFile zipFile, String fileString) {
        MyLogger.logI(CLASS_TAG, "getFile");
        ByteArrayOutputStream baos = null;
        String content = null;
        try {
            ZipEntry zipEntry = zipFile.getEntry(fileString);
            if (zipEntry != null) {
                InputStream is = zipFile.getInputStream(zipEntry);
                baos = new ByteArrayOutputStream();
                int len = -1;
                byte[] buffer = new byte[512];
                while ((len = is.read(buffer, 0, 512)) != -1) {
                    baos.write(buffer, 0, len);
                }
                content = baos.toString();
                is.close();
            }

        } catch (IOException e) {
            e.printStackTrace();
            return null;
        }
        return content;
    }

    public static byte[] readFileContent(ZipFile zipFile, String fileString) {
        MyLogger.logI(CLASS_TAG, "getFile");
        ByteArrayOutputStream baos = null;
        try {
            ZipEntry zipEntry = zipFile.getEntry(fileString);
            if (zipEntry != null) {
                InputStream is = zipFile.getInputStream(zipEntry);
                baos = new ByteArrayOutputStream();
                int len = -1;
                byte[] buffer = new byte[512];
                while ((len = is.read(buffer, 0, 512)) != -1) {
                    baos.write(buffer, 0, len);
                }
                is.close();
            }
        } catch (IOException e) {
            e.printStackTrace();
            return null;
        }

        return baos.toByteArray();
    }

    public static void unZipFile(String zipFileName, String srcFileName, String destFileName) throws IOException {
        File destFile = new File(destFileName);
        ZipFile zipFile = null;
        InputStream is = null;
        if (!destFile.exists()) {
            File tmpDir = destFile.getParentFile();
            if (!tmpDir.exists()) {
                tmpDir.mkdirs();
            }

            destFile.createNewFile();
        }

        FileOutputStream out = new FileOutputStream(destFile);
        try {
            zipFile = new ZipFile(zipFileName);
            ZipEntry zipEntry = zipFile.getEntry(srcFileName);
            if (zipEntry != null) {
                is = zipFile.getInputStream(zipEntry);
                int len = -1;
                byte[] buffer = new byte[512];
                while ((len = is.read(buffer, 0, 512)) != -1) {
                    out.write(buffer, 0, len);
                    out.flush();
                }

                is.close();
            }
            zipFile.close();
        } catch (IOException e) {
            // e.printStackTrace();
            throw e;
        } finally {
            out.close();
            is.close();
            zipFile.close();
        }
    }

    ZipOutputStream mOutZip;

    public void createZipFile(String zipFileString) throws IOException {
        mOutZip = new ZipOutputStream(new FileOutputStream(zipFileString));
    }

    public void addFileByFileName(String srcFileName, String desFileName) throws IOException {
        MyLogger.logD("BACKUP", "addFileByFileName:" + "srcFile:" + srcFileName + ",desFile:" + desFileName);

        ZipEntry zipEntry = new ZipEntry(desFileName);
        File file = new File(srcFileName);
        FileInputStream inputStream = new FileInputStream(file);
        mOutZip.putNextEntry(zipEntry);

        byte[] buffer = new byte[1024];
        int len = 0;
        while ((len = inputStream.read(buffer)) != -1) {
            mOutZip.write(buffer, 0, len);
        }

        inputStream.close();
    }

    public void finish() throws IOException {
        mOutZip.finish();
        mOutZip.close();
    }
}