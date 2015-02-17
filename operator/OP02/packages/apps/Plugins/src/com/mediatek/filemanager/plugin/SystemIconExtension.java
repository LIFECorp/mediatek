package com.mediatek.filemanager.plugin;

import android.content.Context;
import android.content.res.Resources;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Build;
import android.os.Handler;
import android.text.TextUtils;

import com.mediatek.op02.plugin.R;
import com.mediatek.filemanager.ext.DefaultIconExtension;

import java.io.File;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;


public class SystemIconExtension extends DefaultIconExtension {

    private HashMap<SystemFolderInfo, IconInfo> mSystemFolderIconMap = new HashMap<SystemFolderInfo, IconInfo>();
    private static final String DOCUMENT = "Document";
    private static final String DOWNLOAD = "Download";
    private static final String MUSIC = "Music";
    private static final String PHOTO = "Photo";
    private static final String RECEIVED = "Received File";
    private static final String VIDEO = "Video";
    private Resources mPluginResource;

    public SystemIconExtension(Context context) {
        if (context == null) {
            throw new IllegalArgumentException("Context is null");
        }
        mPluginResource = context.getResources();
    }

    private static class IconInfo {
        private Bitmap mIcon;
        private int mResourceId;

        public IconInfo(int resourceId, Bitmap icon) {
            mIcon = icon;
            mResourceId = resourceId;
        }

        public int getResourceId() {
            return mResourceId;
        }

        public Bitmap getIcon() {
            return mIcon;
        }

        public void setIcon(Bitmap icon) {
            this.mIcon = icon;
        }

        public void setResourceId(int resourceId) {
            this.mResourceId = resourceId;
        }
    }

    private static class SystemFolderInfo {
        private String mPath;

        public SystemFolderInfo(String path) {
            mPath = path;
        }

        public String getPath() {
            return mPath;
        }

        @Override
        public boolean equals(Object o) {
            if (super.equals(o)) {
                return true;
            } else {
                if (o instanceof SystemFolderInfo) {
                    if (((SystemFolderInfo) o).getPath().equalsIgnoreCase(mPath)) {
                        return true;
                    }
                }
                return false;
            }
        }

        @Override
        public int hashCode() {
            return mPath.toLowerCase().hashCode();
        }
    }

    @Override
    public void createSystemFolder(String defaultPath) {

        mSystemFolderIconMap.clear();

        mSystemFolderIconMap.put(new SystemFolderInfo(defaultPath + DOCUMENT), new IconInfo(
                R.drawable.fm_document_folder, null));
        mSystemFolderIconMap.put(new SystemFolderInfo(defaultPath + DOWNLOAD), new IconInfo(
                R.drawable.fm_download_folder, null));
        mSystemFolderIconMap.put(new SystemFolderInfo(defaultPath + MUSIC), new IconInfo(
                R.drawable.fm_music_folder, null));
        mSystemFolderIconMap.put(new SystemFolderInfo(defaultPath + PHOTO), new IconInfo(
                R.drawable.fm_photo_folder, null));
        mSystemFolderIconMap.put(new SystemFolderInfo(defaultPath + RECEIVED), new IconInfo(
                R.drawable.fm_received_folder, null));
        mSystemFolderIconMap.put(new SystemFolderInfo(defaultPath + VIDEO), new IconInfo(
                R.drawable.fm_video_folder, null));

        createSystemFolder();

    }

    private void createSystemFolder() {
        new Thread(new Runnable() {

            @Override
            public void run() {
                Iterator<SystemFolderInfo> iterator = mSystemFolderIconMap.keySet().iterator();
                while (iterator.hasNext()) {
                    SystemFolderInfo systemFolderInfo = iterator.next();
                    File dir = new File(systemFolderInfo.mPath);
                    if (!dir.exists()) {
                        boolean ret = dir.mkdirs();
                    }
                }
            }
        }).start();
    }

    @Override
    public boolean isSystemFolder(String path) {
        return mSystemFolderIconMap.containsKey(new SystemFolderInfo(path));
    }

    @Override
    public Bitmap getSystemFolderIcon(String path) {
        IconInfo iconInfo = null;
        Bitmap icon = null;
        SystemFolderInfo systemFolderInfo = new SystemFolderInfo(path);

        iconInfo = mSystemFolderIconMap.get(systemFolderInfo);
        if (iconInfo != null) {
            if (iconInfo.getIcon() == null) {
                icon = BitmapFactory.decodeResource(mPluginResource, iconInfo.getResourceId());
                iconInfo.setIcon(icon);
            } else {
                icon = iconInfo.getIcon();
            }
        }

        return icon;
    }
}
