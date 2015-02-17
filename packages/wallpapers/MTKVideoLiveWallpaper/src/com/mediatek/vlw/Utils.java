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

package com.mediatek.vlw;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.content.DialogInterface;
import android.database.Cursor;
import android.net.Uri;
import android.provider.MediaStore;
import android.provider.MediaStore.Video;
import android.view.Display;
import android.view.WindowManager;
import android.widget.Toast;

import com.mediatek.media.MediaPlayerEx;
import com.mediatek.xlog.Xlog;

import java.io.File;
import java.text.Collator;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.Random;

public final class Utils {
    public static final String VIDEO_LIVE_WALLPAPER_PACKAGE = "com.mediatek.vlw";
    public static final String VIDEO_LIVE_WALLPAPER_CLASS = "com.mediatek.vlw.VideoLiveWallpaper";
    
    private static final String EQUALS = "=";
    private static final String EXTERNALSD = "/sdcard1/";
    private static final String INTERNALSD = "/sdcard0/";
    private static Random sRandom = new Random();

    /**
     * Need not to instantiate this class object.
     */
    private Utils() {
    }
    
    /**
     * This method used to create a toast or a AlertDialog.
     * 
     * @param context
     * @param id  Mark the text will be shown.
     * @param silent Mark show a toast or a AlertDialog.
     * 
     */
    public static void showInfo(Context context, int id, boolean silent) {
        if (silent) {
            Toast.makeText(context, id, Toast.LENGTH_LONG).show();
        } else if (context instanceof Activity) {
            new AlertDialog.Builder(context).setTitle(
                    R.string.VideoEditor_error_title).setMessage(id)
                    .setPositiveButton(R.string.VideoEditor_error_button,
                            new DialogInterface.OnClickListener() {
                                /**
                                 * If we get here, there is no onError listener,
                                 * so at least inform them that the video is
                                 * over.
                                 */
                                /* TODO: */
                                public void onClick(DialogInterface dialog,
                                        int whichButton) {
                                }
                            }).setCancelable(false).show();
        }
    }
    
    /**
     * This method used to check if the video is the default video according to
     * its Uri.
     * 
     * @param uri  The Uri of the video will be checked.
     * 
     */
    public static boolean isDefaultVideo(Uri uri) {
        if (uri == null) {
            return true;
        }
        String scheme = uri.getScheme();
        if (ContentResolver.SCHEME_ANDROID_RESOURCE.equals(scheme)) {
            return true;
        } else if (ContentResolver.SCHEME_CONTENT.equals(scheme)
                || ContentResolver.SCHEME_FILE.equals(scheme)) {
            return false;
        }
        return false;
    }
    
    /// M: for s3d video
    public static ArrayList<Uri> queryUrisFromBucketId(Context context, String bucketId) {
        if (context == null || bucketId == null) {
            return null;
        }
        ArrayList<Uri> uris = null;
        uris = query2DVideos(context, bucketId);
        Collections.sort(uris, new Comparator<Uri>() {
            final Collator mCollator;

            {
                mCollator = Collator.getInstance();
            }

            public int compare(Uri uri1, Uri uri2) {
                return mCollator.compare(uri1.getPath(), uri2.getPath());
            }
        });
        return uris;
    }
    
    /**
     * This method used to get all normal 2D videos in a folder.
     * 
     * @param context
     * @param bucketId The bucketId of the folder will be checked.
     */
    public static ArrayList<Uri> query2DVideos(Context context, String bucketId) {
        if (context == null || bucketId == null) {
            return null;
        }
        ArrayList<Uri> uris = new ArrayList<Uri>();
        Cursor cursor = null;
        try {
            // Add 4k video filter FeatureOption.MTK_VIDEO_4KH264_SUPPORT
            String where = Video.VideoColumns.BUCKET_ID + "=" + bucketId
                    + " AND ((" + Video.VideoColumns.WIDTH + " <= 1920"
                    + " AND " + Video.VideoColumns.HEIGHT + " <= 1920)"
                    + " OR (" + Video.VideoColumns.WIDTH + " IS NULL" + " OR "
                    + Video.VideoColumns.HEIGHT + " IS NULL))";

            ContentResolver cr = context.getContentResolver();
            String[] projection = { Video.Media.DATA };
            cursor = cr.query(Video.Media.EXTERNAL_CONTENT_URI,
                    projection, where, null, null);
            //Xlog.d("VideoLiveWallpaperUtils", "cursor count: " + cursor.getCount());  
            if (cursor != null && cursor.moveToFirst()) {
                String uriString = null;
                do {
                    uriString = cursor.getString(0);
                    uris.add(Uri.fromFile(new File(uriString)));
                    //Xlog.d("VideoLiveWallpaperUtils", "get URI: " + uriString);  
                } while (cursor.moveToNext());
            }
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }

        Collections.sort(uris, new Comparator<Uri>() {
            final Collator mCollator;

            {
                mCollator = Collator.getInstance();
            }

            public int compare(Uri uri1, Uri uri2) {
                return mCollator.compare(uri1.getPath(), uri2.getPath());
            }
        });

        return uris;
    }
    
    /**
     * This method used to get the folder information according to the bucketId.
     * 
     * @param context
     * @param bucketId The bucketId of the folder will be checked.
     */
    public static String queryFolderInfo(Context context, String bucketId) {
        if (context == null || bucketId == null) {
            return null;
        }
        String info = null;
            Cursor cursor = null;
            try {
                String where = Video.VideoColumns.BUCKET_ID + "="
                        + bucketId;
                ContentResolver cr = context.getContentResolver();
                String[] projection = { Video.Media.BUCKET_DISPLAY_NAME };
                cursor = cr.query(Video.Media.EXTERNAL_CONTENT_URI,
                        projection, where, null, null);
                if (cursor != null && cursor.moveToFirst()) {
                    info = cursor.getString(0);
                }
            } finally {
                if (cursor != null) {
                    cursor.close();
                }
            }
        return info;
    }
    
    /**
     * This method used to check whether the file exist or not.
     * 
     * @param path The file's path.
     */
    public static boolean isExternalFileExists(String path) {
        if (path == null) {
            return false;
        }
        File externalFile = new File(path);
        return externalFile.exists();
    }
    
    /**
     * This method used to check whether the file exist or not.
     * 
     * @param context
     * @param uri The Uri of the file will be checked.
     */
    public static boolean isExternalFileExists(Context context, Uri uri) {
        if (context == null || uri == null) {
            return false;
        }
        boolean result = false;
        Cursor cursor = null;
        try {
            // video from SDCARD
            ContentResolver cr = context.getContentResolver();
            String[] proj = { Video.Media.DATA };
            cursor = cr.query(uri, proj, null, null, null);
            if (cursor != null && cursor.moveToFirst()) {
                int dataIndex = cursor
                        .getColumnIndexOrThrow(Video.Media.DATA);
                String path = cursor.getString(dataIndex);
                File externalFile = new File(path);
                if (externalFile.exists()) {
                    result = true;
                }                    
            }
        } finally {
            /*
             * Actually, the Cursor from managedQury() will take care of its own
             * life
             */
            if (cursor != null) {
                cursor.close();
            }
        }

        return result;
    }

    /**
     * Return next index according to loop mode
     * 
     * @param mode
     * @param curPos
     * @param len
     * @return > 0 next index; -1 len is 0, there is nothing
     */
    public static int getLoopIndex(LoopMode mode, int curPos, ArrayList<Uri> uris, ArrayList<Uri> invalid) {
        final int bound = uris.size() - 1;

        int position = (curPos < 0 || curPos > bound) ? 0 : curPos;
        // return -1 if len <= 0 or all videos are invalid
        if (bound < 0 || (invalid != null && (invalid.size() == uris.size()))) {
            return -1;
        }
        // Now at least one valid video exists
        switch (mode) {
        case SINGLE:
            if (invalid != null && invalid.contains(uris.get(position))) {
                position = -1;
            } else {
                position = curPos;
            }

            break;
        case ALL:
            position = ++curPos;
            if (position > bound) {
                position = 0;
            }                
            while (invalid != null && invalid.contains(uris.get(position))) {
                if (++position > bound) {
                    position = 0;
                }                   
            }              

            break;
        case RANDOM:
        default:
            position = sRandom.nextInt(bound);
            while (invalid != null && invalid.contains(uris.get(position))) {
                position = sRandom.nextInt(bound);
            }

            break;
        }

        return position;
    }
    
    /**
     * This method used to query the resolution of the display. 
     * 
     */
    public static float queryResolutionRatio(Context context) {
        WindowManager wm = (WindowManager) context
                .getSystemService(Context.WINDOW_SERVICE);
        Display disp = wm.getDefaultDisplay();
        return (float) disp.getWidth() * 2 / disp.getHeight();
    }

    /**
     * Check if this video resides in resources, if not return it's storage path
     * 
     * @param uri
     * @return null default video from resources not null, external storage path
     */
    public static String getVideoPath(Context context, Uri uri) {
        if (context == null || uri == null) {
            return null;
        }
        String path = null;
        String scheme = uri.getScheme();
        if (ContentResolver.SCHEME_CONTENT.equals(scheme)
                || ContentResolver.SCHEME_FILE.equals(scheme)) {
            Cursor cursor = null;
            try {
                // Video from SDCARD
                ContentResolver cr = context.getContentResolver();
                String[] proj = { Video.Media.DATA };
                cursor = cr.query(uri, proj, null, null, null);
                if (cursor != null && cursor.moveToFirst()) {
                    int dataIndex = cursor.getColumnIndexOrThrow(Video.Media.DATA);
                    path = cursor.getString(dataIndex);
                }
            } finally {
                // Actually, the Cursor from managedQury() will take care of
                // its own life
                if (cursor != null) {
                    cursor.close();
                }
            }
        }
        return path;
    }
    
    /**
     * This method used to swap the SDCard Uri.
     * 
     * @param uri The Uri of the file will be swaped.
     */
    public static Uri swapSdcardUri(Uri uri) {
        if (uri == null) {
            return uri;
        }
        Uri swapUri = null;
        String path = uri.getPath();
        if (path.contains(INTERNALSD)) {
            String swapPath = path.replace(INTERNALSD, EXTERNALSD);
            swapUri = Uri.fromFile(new File(swapPath));
        } else if (path.contains(EXTERNALSD)) {
            String swapPath = path.replace(EXTERNALSD, INTERNALSD);
            swapUri = Uri.fromFile(new File(swapPath));
        }

        return swapUri;
    }

    /**
     * Query bucketId from media database for given video absolute path.
     * 
     * @param context
     * @param videoPath.
     */
    public static String queryBucketId(Context context, String videoPath) {
        if (context == null || videoPath == null) {
            return null;
        }
        String bucketId = null;
        Cursor cursor = null;
        try {
            // Must use "'" enclose videoPath, or SQLite will throw syntax error
            String where = Video.VideoColumns.DATA + "=" 
                    + "'" + videoPath + "'";
            ContentResolver cr = context.getContentResolver();
            String[] projection = { Video.VideoColumns.BUCKET_ID };
            cursor = cr.query(Video.Media.EXTERNAL_CONTENT_URI,
                    projection, where, null, null);
            if (cursor != null && cursor.moveToFirst()) {
                bucketId = cursor.getString(0);
            }
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
        return bucketId;
    }
    
    public enum LoopMode {
        // loop mode
        RANDOM(0), SINGLE(1), ALL(2);

        LoopMode(int mode) {
            // TODO Auto-generated constructor stub
            this.mMode = mode;
        }

        private final int mMode;

        /**
         * This method used to get the value of mMode.
         * 
         */
        public int getValue() {
            return mMode;
        }
    }
}
