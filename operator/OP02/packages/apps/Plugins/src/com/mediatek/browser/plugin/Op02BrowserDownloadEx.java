package com.mediatek.browser.plugin;

import android.app.DownloadManager.Request;
import android.media.MediaFile;
import android.net.Uri;
import android.os.Environment;

import com.mediatek.browser.ext.BrowserDownloadEx;
import com.mediatek.xlog.Xlog;

import java.io.File;

public class Op02BrowserDownloadEx extends BrowserDownloadEx {
    
    private static final String TAG = "BrowserPluginEx";
    
    @Override
    public boolean setRequestDestinationDir(String downloadPath, Request mRequest, String filename, String mimeType) {
        Xlog.i(TAG, "Enter: " + "setRequestDestinationDir" + " --OP02 implement");
        String op02Folder = getStorageDirectoryForOperator(mimeType);

        String path = "file://" + downloadPath.substring(0, downloadPath.lastIndexOf("/")) + File.separator
                                        + op02Folder + File.separator + filename;
        Uri pathUri = Uri.parse(path);
        mRequest.setDestinationUri(pathUri);
        Xlog.i(TAG, "For OP02: selected download full path is: " +
                path + " MimeType is: " + mimeType + " and Uri is: " + pathUri);
        
        return true;
    }
    
    /**
     *  M: This function is used to support OP02 customization
     *  Get different directory for different mimeType
     */
    
    public String getStorageDirectoryForOperator(String mimeType) {
        
        Xlog.i(TAG, "Enter: " + "getStorageDirectoryForOperator" + " --OP02 implement");
        
        // if mimeType is null, return the default download folder.
        if (mimeType == null) {
            return Environment.DIRECTORY_DOWNLOADS;
        }

        // This is for OP02
        int fileType = MediaFile.getFileTypeForMimeType(mimeType);
        String selectionStr = null;

        if (mimeType.startsWith("audio/") || MediaFile.isAudioFileType(fileType)) {
            selectionStr = "Music";

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
        
        Xlog.i(TAG, "Enter: " + "shouldShowDownloadOrOpenContent" + " --OP02 implement");
        
        return true;
    }

    @Override
    public boolean shouldShowToastWithFileSize() {
        Xlog.i(TAG, "Enter: " + "shouldShowToastWithFileSize" + " --OP02 implement");
        return true;
    }
    
}
