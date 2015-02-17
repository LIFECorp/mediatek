package com.mediatek.backuprestore.utils;

import android.content.Context;
import android.os.Looper;
import android.os.RemoteException;
import android.os.storage.StorageManager;
import android.os.storage.StorageVolume;
import android.os.Environment;
import android.util.Log;

import com.mediatek.backuprestore.R;
import com.mediatek.backuprestore.utils.Constants.LogTag;
import com.mediatek.backuprestore.utils.Constants.ModulePath;

import java.io.File;
import java.io.IOException;

public class SDCardUtils {

    public static final int MINIMUM_SIZE = 512;
    private static String usbPath = Environment.DIRECTORY_USBOTG;

    public static String getStoragePath() {
        String storagePath = null;
        StorageManager storageManager = null;

        try {
            storageManager = new StorageManager(null,Looper.getMainLooper());
        } catch (RemoteException e) {
            e.printStackTrace();
            return null;
        }

        StorageVolume[] volumes = storageManager.getVolumeList();
        if (volumes != null) {
            for (StorageVolume volume : volumes) {
                if (volume.isRemovable()) {
                    String path = volume.getPath();
                    if (path != null && !path.matches(usbPath)) {
                        storagePath = path + File.separator + "backup";
                        break;
                    }
                }
            }
        }

        Log.d(LogTag.LOG_TAG, "getStoragePath: path is " + storagePath);
        if (storagePath == null) {
            return null;
        }
        File file = new File(storagePath);
        if (file != null) {
            if (file.exists() && file.isDirectory()) {
                File temp = new File(storagePath + File.separator + ".BackupRestoretemp");
                boolean ret;
                if (temp.exists()) {
                    ret = temp.delete();
                } else {
                    try {
                        ret = temp.createNewFile();
                    } catch (IOException e) {
                        e.printStackTrace();
                        Log.e(LogTag.LOG_TAG, "getStoragePath: " + e.getMessage());
                        ret = false;
                    } finally {
                        temp.delete();
                    }
                }
                if (ret) {
                    return storagePath;
                } else {
                    return null;
                }

            } else if (file.mkdir()) {
                return storagePath;
            }
        }
        return null;
    }

    public static String getPersonalDataBackupPath() {
        String path = getStoragePath();
        if (path != null) {
            return path + File.separator + ModulePath.FOLDER_DATA;
        }

        return path;
    }

    public static String getAppsBackupPath() {
        String path = getStoragePath();
        if (path != null) {
            return path + File.separator + ModulePath.FOLDER_APP;
        }
        return path;
    }

    public static boolean isSdCardAvailable() {
        return (getStoragePath() != null);
    }

    public static long getAvailableSize(String file) {
        android.os.StatFs stat = new android.os.StatFs(file);
        long count = stat.getAvailableBlocks();
        long size = stat.getBlockSize();
        long totalSize = count * size;
        Log.v(LogTag.LOG_TAG, "file remain size = " + totalSize);
        return totalSize;
    }
    
    public static String getSDStatueMessage(Context context){
        String message= context.getString(R.string.nosdcard_notice);
        String status = Environment.getExternalStorageState();
        if (status.equals(Environment.MEDIA_SHARED) ||
            status.equals(Environment.MEDIA_UNMOUNTED)) {
            message = context.getString(R.string.sdcard_busy_message);
        }
        return message;
    }
}
