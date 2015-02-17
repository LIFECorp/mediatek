package com.mediatek.browser.plugin;

import android.app.DownloadManager.Request;
import android.media.MediaFile;
import android.net.Uri;
import android.os.Environment;

import com.mediatek.browser.ext.BrowserDownloadEx;
import com.mediatek.xlog.Xlog;

public class Op01BrowserDownloadEx extends BrowserDownloadEx {
    
    public static final String DEFAULT_DOWNLOAD_DIRECTORY_OP01 = "/storage/sdcard0/MyFavorite"; //BrowserSettings
    public static final String DEFAULT_DOWNLOAD_SDCARD2_DIRECTORY_OP01 = "/storage/sdcard1/MyFavorite";  //BrowserSettings
    private static final String TAG = "BrowserPluginEx";
    @Override
    public boolean setRequestDestinationDir(String downloadPath, Request mRequest, String filename, String mimeType) {
        
        Xlog.i(TAG, "Enter: " + "setRequestDestinationDir" + " --OP01 implement");
        
        String path = null;
        if (downloadPath.equalsIgnoreCase(DEFAULT_DOWNLOAD_DIRECTORY_OP01) ||
                downloadPath.equalsIgnoreCase(DEFAULT_DOWNLOAD_SDCARD2_DIRECTORY_OP01)) {
            String folder = getStorageDirectoryForOperator(mimeType);
            path = "file://" + downloadPath +
                "/" + folder + "/" + filename;
        } else {
            path = "file://" + downloadPath + "/"
                + filename;
        }
        
        Uri downloadUri = Uri.parse(path);
        Xlog.i(TAG, "For OP01: selected download full path is: " +
            path + " MimeType is: " + mimeType + " and Uri is: " + downloadUri);
        mRequest.setDestinationUri(downloadUri);
        
        return true;
    }
    
    /**
     *  M: This function is used to support OP02 customization
     *  Get different directory for different mimeType
     */
    
    public String getStorageDirectoryForOperator(String mimeType) {
        
        Xlog.i(TAG, "Enter: " + "getStorageDirectoryForOperator" + " --OP01 implement");
        // if mimeType is null, return the default download folder.
        if (mimeType == null) {
            return Environment.DIRECTORY_DOWNLOADS;
        }

        // This is for OP02
        int fileType = MediaFile.getFileTypeForMimeType(mimeType);
        String selectionStr = null;

        if (mimeType.startsWith("audio/") || MediaFile.isAudioFileType(fileType)) {
            selectionStr = "Ringtone";
        } else if (mimeType.startsWith("image/") || MediaFile.isImageFileType(fileType)) {
            selectionStr = "Photo";
        } else if (mimeType.startsWith("video/") || MediaFile.isVideoFileType(fileType)) {
            selectionStr = "Video";
        } else if (mimeType.startsWith("text/") || mimeType.equalsIgnoreCase("application/msword")
                || mimeType.equalsIgnoreCase("application/vnd.ms-powerpoint")
                || mimeType.equalsIgnoreCase("application/pdf")) {
            selectionStr = "Document";
        } else {
            selectionStr = Environment.DIRECTORY_DOWNLOADS;
        }
        
        Xlog.d(TAG, "mimeType is: " + mimeType
                + "MediaFileType is: " + fileType +
                "folder is: " + selectionStr);
        
        return selectionStr;
    }
    
    @Override
    public boolean shouldShowDownloadOrOpenContent() {
        Xlog.i(TAG, "Enter: " + "shouldShowDownloadOrOpenContent" + " --OP01 implement");
        return true;
    }
    
    @Override
    public boolean shouldShowToastWithFileSize() {
        Xlog.i(TAG, "Enter: " + "shouldShowToastWithFileSize" + " --OP01 implement");
        return true;
    }
}
