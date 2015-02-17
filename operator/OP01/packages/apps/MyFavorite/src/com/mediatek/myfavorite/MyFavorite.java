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

package com.mediatek.myfavorite;

import android.app.Activity;
import android.content.ActivityNotFoundException;
import android.content.ComponentName;
import android.content.Intent;
import android.database.Cursor;
import android.net.Uri;
import android.os.Bundle;
import android.os.RemoteException;
//import android.os.storage.StorageManager;
import android.provider.MediaStore;
//import android.provider.MediaStore.Images;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ListView;
import android.widget.SimpleAdapter;
import android.widget.Toast;

import com.mediatek.storage.StorageManagerEx;
import com.mediatek.xlog.Xlog;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class MyFavorite extends Activity implements OnItemClickListener {
    private static final String TAG = "MyFavorite";

    private static final String PICTURE_MIMETYPE = "vnd.android.cursor.dir/image";

    private static final String VIDEO_MIMETYPE = "vnd.android.cursor.dir/video";

    private static final String DEFAULT_PHOTO_FULL_PATH = "/mnt/sdcard/DCIM/Camera";
    
    private static final String DEFAULT_PHOTO_PATH = "/DCIM/Camera";

    //private StorageManager mStorageManager = null;

    private static final int PHOTO_TYPE = 1;

    private static final int MUSIC_ITEM = 0;

    private static final int VIDEO_ITEM = 1;

    //private static final int PHOTO_ITEM = 2;

    private static final int PICTURE_ITEM = 2;

    private static final int ALL_FILES_ITEM = 3;

    private static final int[] FOLDER_ICON_ID = new int[] {
            R.drawable.music, R.drawable.video, R.drawable.picture,
            R.drawable.all_files
    };

    private ListView mListView;

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.myfavorite);
        initUI();
    }

    private void initUI() {
        mListView = (ListView) findViewById(R.id.listfolder);
        SimpleAdapter sa = new SimpleAdapter(this, myfavoriteItems(), R.layout.myfavorite_row,
                new String[] {
                        "folderlogo", "text", "rightlogo"
                }, new int[] {
                        R.id.folderlogo, R.id.foldername
                });
        mListView.setAdapter(sa);
        mListView.setOnItemClickListener(this);
    }

    private List<Map<String, Object>> myfavoriteItems() {
        List<Map<String, Object>> data = new ArrayList<Map<String, Object>>();
        Map<String, Object> map = null;
        String[] folderNames = getResources().getStringArray(R.array.foldername);
        for (int i = 0; i < folderNames.length; i++) {
            map = new HashMap<String, Object>();
            map.put("folderlogo", FOLDER_ICON_ID[i]);
            map.put("text", folderNames[i]);
            data.add(map);
        }

        return data;
    }

    public void onItemClick(AdapterView<?> arg0, View arg1, int arg2, long arg3) {
        switch (arg2) {
            case MUSIC_ITEM:
                launchMusic();
                break;

            case VIDEO_ITEM:
                launchVideo();
                break;

            //case PHOTO_ITEM:
                //launchGallery(MediaStore.Images.Media.EXTERNAL_CONTENT_URI, null);
                //break;

            case PICTURE_ITEM:
                launchGallery(MediaStore.Images.Media.EXTERNAL_CONTENT_URI, PICTURE_MIMETYPE);
                break;

            case ALL_FILES_ITEM:
                launchFileManager();
                break;

            default:
                break;
        }
    }

    private void launchFileManager() {
        Intent intent = new Intent();
        try {
            intent.setComponent(new ComponentName("com.mediatek.filemanager",
                    "com.mediatek.filemanager.FileManagerOperationActivity"));
            this.startActivity(intent);
        } catch (ActivityNotFoundException e) {
            Toast.makeText(this, R.string.allfilesuninstalled, Toast.LENGTH_LONG).show();
        }
    }

    private void launchMusic() {
        Intent intent = new Intent();
        try {
            intent.setComponent(new ComponentName("com.android.music", "com.android.music.MusicBrowserActivity"));
            this.startActivity(intent);
        } catch (ActivityNotFoundException e) {
            Toast.makeText(this, R.string.musicuninstalled, Toast.LENGTH_LONG).show();
        }
    }

    private void launchVideo() {
        Intent intent = new Intent();
        try {
            intent.setComponent(new ComponentName("com.mediatek.videoplayer", "com.mediatek.videoplayer.MovieListActivity"));
            this.startActivity(intent);
        } catch (ActivityNotFoundException e1) {
            launchGallery(MediaStore.Video.Media.EXTERNAL_CONTENT_URI, VIDEO_MIMETYPE);
        }
    }

    private void launchGallery(Uri uri, String mimeType) {
        try {
            if (null == mimeType) {
                String photoPath = getPhotoPath();
                Xlog.i(TAG, "launchGallery(): photo path is " + photoPath);
                if (photoPath == null) {
                    photoPath = getDefaultPhotoPath();
                    Xlog.i(TAG, "launchGallery(): default photo path is " + photoPath);
                }
                uri = uri.buildUpon().appendQueryParameter("bucketId", getHashcode(photoPath))
                        .build();
                Intent intent = new Intent(Intent.ACTION_VIEW, uri);
                intent.putExtra("windowTitle", getString(R.string.photo));
                intent.putExtra("mediaTypes", PHOTO_TYPE);
                startActivity(intent);
            } else {
                Intent intent = new Intent(Intent.ACTION_VIEW);
                intent.setType(mimeType);
                this.startActivity(intent);
            }
        } catch (ActivityNotFoundException e) {
            Toast.makeText(this, R.string.galleryuninstalled, Toast.LENGTH_LONG).show();
        }
    }

    private String getHashcode(String path) {
        return String.valueOf(path.toLowerCase().hashCode());
    }

    private String getPhotoPath() {
        String path = null;
        // Environment.getExternalStoragePublicDirectory("DCIM").getAbsolutePath();
        String[] projection = new String[] { "id", "category", "path" }; 
        Uri allCategory = Uri.parse("content://com.mediatek.camera.provider");
        Cursor cur = managedQuery(allCategory, projection, "photo", null, null); 
        if (cur != null && cur.moveToFirst()) {
            int pathCol = cur.getColumnIndex("path");
            do {
                path = cur.getString(pathCol);
            } while (cur.moveToNext());
        }
        return path;
    }

    private String getDefaultPhotoPath() {
        String path = null;
        //if (mStorageManager == null) {
            //try {
                //mStorageManager = new StorageManager(this.getMainLooper());
            //} catch (RemoteException e) {
                //Xlog.e(TAG, "getDefaultPhotoPath(): failed to get StorageManager");
            //}
        //}
        //if (mStorageManager != null) {
        String defaultStoragePath = StorageManagerEx.getDefaultPath();
        Xlog.i(TAG, "getDefaultPhotoPath(): defaultStoragePath=" + defaultStoragePath);

        path = defaultStoragePath + DEFAULT_PHOTO_PATH;
        //}
        
        return path;
    }
}
