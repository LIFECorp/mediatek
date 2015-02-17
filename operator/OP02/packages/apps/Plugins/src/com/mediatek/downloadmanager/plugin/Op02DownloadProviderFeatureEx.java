package com.mediatek.downloadmanager.plugin;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface.OnClickListener;
import android.content.Intent;
import android.media.MediaFile;
import android.os.Environment;

import com.mediatek.downloadmanager.ext.DownloadProviderFeatureEx;
import com.mediatek.xlog.Xlog;


public class Op02DownloadProviderFeatureEx extends DownloadProviderFeatureEx {
    
    private static final String TAG = "DownloadProviderPluginEx";
    
    public static final String SHOW_DIALOG_REASON = "ShowDialogReason"; //DownloadInfo
    
    public Op02DownloadProviderFeatureEx(Context context) {
        super(context);
    }
    
    @Override
    public int getShowDialogReasonInt(Intent intent) {
        Xlog.i(TAG, "Enter: " + "getShowDialogReasonInt" + " --OP02 implement");
        return intent.getExtras().getInt(SHOW_DIALOG_REASON);
    }

    @Override
    public boolean shouldSetContinueDownload() {
        //
        Xlog.i(TAG, "Enter: " + "shouldSetContinueDownload" + " --OP02 implement");
        return true;
    }
    
    /**
     * M: Add this function to support CU customization
     * If user click "Yes", continue download.
     * If click "Cancel", abort download.
     */
    
    @Override
    public boolean shouldNotifyFileAlreadyExist() {
        Xlog.i(TAG, "Enter: " + "shouldNotifyFileAlreadyExist" + " --OP02 implement");
        return true;
    }
    
    /**
     *  M: This function is used to support OP02 customization
     *  Get different directory for different mimeType
     */
    @Override
    public String getStorageDirectory(String mimeType) {
        Xlog.i(TAG, "Enter: " + "getStorageDirectory" + " --OP02 implement");
        
        int fileType = MediaFile.getFileTypeForMimeType(mimeType);
        String selectionStr = null;

        if (mimeType.startsWith("audio/") || MediaFile.isAudioFileType(fileType)) {
            //base = new File(root.getPath() + Constants.OP02_CUSTOMIZATION_AUDIO_DL_SUBDIR);
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
        
        Xlog.i(TAG, "mimeType is: " + mimeType
                + "MediaFileType is: " + fileType + 
                "folder is: " + selectionStr);
        return selectionStr;
    }
  
    public boolean shouldFinishThisActivity() {
        Xlog.i(TAG, "Enter: " + "shouldFinishThisActivity" + " --OP02 implement");
        return true;
    }
    
    public boolean shouldProcessWhenFileExist() {
        Xlog.i(TAG, "Enter: " + "shouldProcessWhenFileExist" + " --OP02 implement");
        return true;
    }

    public void showFileAlreadyExistDialog(AlertDialog.Builder builder, CharSequence appLable,
            CharSequence message, String positiveButtonString, String negativeButtonString, OnClickListener listener) {
        Xlog.i(TAG, "Enter: " + "showFileAlreadyExistDialog" + " --OP02 implement");

            builder.setTitle(appLable)
                    .setMessage(message)
                    .setPositiveButton(positiveButtonString, listener)
                    .setNegativeButton(negativeButtonString, listener);
    } 
}
