package com.mediatek.filemanager.plugin;

import android.graphics.Bitmap;
import android.test.InstrumentationTestCase;
import android.util.Log;

import com.mediatek.filemanager.ext.DefaultIconExtension;
import com.mediatek.filemanager.ext.IIconExtension;
import com.mediatek.storage.StorageManagerEx;
import com.mediatek.filemanager.plugin.SystemIconExtension;
import com.mediatek.pluginmanager.PluginManager;

import java.io.File;
import java.io.IOException;


public class FileManagerPluginTest extends InstrumentationTestCase {

    private final static String TAG = "FileManagerPluginTest";
    private SystemIconExtension mSystemIconExtension = null;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mSystemIconExtension = (SystemIconExtension) PluginManager
                .createPluginObject(getInstrumentation().getTargetContext(), IIconExtension.class.getName());
    }

    @Override
    protected void tearDown() throws Exception {
        if (mSystemIconExtension != null) {
            mSystemIconExtension = null;
        }
        super.tearDown();
    }

    public void testCreateSystemFolder() {
        String defaultPath = StorageManagerEx.getDefaultPath() + "/testCreateSystemFolder";
        Log.d(TAG, "defaultPath = " + defaultPath);
        File loadFile = new File(defaultPath);
        if (loadFile.exists()) {
            deleteFile(loadFile);
        }
        createDirectory(loadFile);
        defaultPath = defaultPath + "/";
        mSystemIconExtension.createSystemFolder(defaultPath);
        try {
            Thread.sleep(5000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }

        File file = new File(defaultPath + "Document");
        assertTrue(file.exists());
        file = new File(defaultPath + "Download");
        assertTrue(file.exists());
        file = new File(defaultPath + "Music");
        assertTrue(file.exists());
        file = new File(defaultPath + "Photo");
        assertTrue(file.exists());
        file = new File(defaultPath + "Received File");
        assertTrue(file.exists());
        file = new File(defaultPath + "Video");
        assertTrue(file.exists());

        defaultPath = defaultPath + "Document";
        if (mSystemIconExtension.isSystemFolder(defaultPath)) {
            Bitmap icon = null;
            icon = mSystemIconExtension.getSystemFolderIcon(defaultPath);
            boolean flag = false;
            if (icon != null) {
                flag = true;
            }
            assertTrue(flag);
        }
    }

    /**
     * This method will create a directory if file is not exist or is not directory
     * 
     * @param file
     */
    public static void createDirectory(File file) {
        if (file.exists() && file.isDirectory()) {
            return;
        }
        boolean flag = file.mkdirs();
        if (!flag) {
            Log.e(TAG, "file.mkdirs() failed in createDirectory()");
        }
    }

    /**
     * This method will delete curFile itself and all of its contains on FileSystem.
     * 
     * @param curFile
     */
    public static void deleteFile(File curFile) {
        if (!curFile.exists()) {
            return;
        }
        if (curFile.isDirectory()) {
            File[] files = curFile.listFiles();
            for (File file : files) {
                deleteFile(file);
            }
            curFile.delete();
        } else {
            curFile.delete();
        }
    }
}
