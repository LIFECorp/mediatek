package com.mediatek.backuprestore.utils;

import android.content.Context;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageParser;
import android.content.res.AssetManager;
import android.content.res.Resources;
import android.graphics.drawable.Drawable;
import android.util.DisplayMetrics;

import com.mediatek.backuprestore.AppSnippet;
import com.mediatek.backuprestore.R;

import java.io.File;
import java.io.IOException;
import java.math.BigDecimal;
import java.util.ArrayList;

public class FileUtils {

    public static final String CLASS_TAG = MyLogger.LOG_TAG + "/FileUtils";

    public static String getDisplaySize(long bytes, Context context) {
        String displaySize = context.getString(R.string.unknown);
        long iKb = bytes2KB(bytes);
        if (iKb == 0 && bytes >= 0) {
            // display "less than 1KB"
            displaySize = context.getString(R.string.less_1K);
        } else if (iKb >= 1024) {
            // diplay MB
            double iMb = ((double) iKb) / 1024;
            iMb = round(iMb, 2, BigDecimal.ROUND_UP);
            StringBuilder builder = new StringBuilder(new Double(iMb).toString());
            builder.append("MB");
            displaySize = builder.toString();
        } else {
            // display KB
            StringBuilder builder = new StringBuilder(new Long(iKb).toString());
            builder.append("KB");
            displaySize = builder.toString();
        }
        return displaySize;
    }

    /*    *//**
     * create files
     * 
     * @param filePath
     * @return
     */
    /*
     * public static File createFile(String filePath) { File file = null; File
     * tmpFile = new File(filePath); if (createFileorFolder(tmpFile)) { file =
     * tmpFile; } return file; }
     */

    public static boolean isMtkOldSmsData(String path) {
        MyLogger.logD(CLASS_TAG, "isMtkOldSmsData() path:" + path);

        ArrayList<String> mXmlList = new ArrayList<String>();
        ArrayList<String> mFileNameList = new ArrayList<String>();
        byte[] content = null;
        try {

            mFileNameList = (ArrayList<String>) BackupZip.getFileList(path, true, true, "sms/sms[0-9]+");
            mXmlList = (ArrayList<String>) BackupZip.getFileList(path, true, true, "sms/msg_box.xml");
            // ZipFile ZipFile = BackupZip.getZipFileFromFileName(path);
            if (mFileNameList.size() > 0) {
                // ArrayList<String> mFileNameList =
                // (ArrayList<String>)BackupZip.GetFileList(mFile.getAbsolutePath(),
                // true, true, "sms/msg_box.xml");
                content = BackupZip.readFileContent(path, mFileNameList.get(0)); // readFile(ZipFile,
                                                                                 // mFileNameList.get(0));
            }
            MyLogger.logD(CLASS_TAG, "isMtkOldSmsData() mFileNameList.size:" + mFileNameList.size() + ", content:"
                    + content);
        } catch (IOException e) {
            MyLogger.logD(CLASS_TAG, "isMtkOldSmsData() IOException");
        }
        boolean result = false;
        if (content != null) {
            byte value = content[0];
            MyLogger.logD(CLASS_TAG, "content" + content[0]);
            if ((value == 0x01) || (value == 0x03) || (value == 0x05) || (value == 0x07)) {
                result = true;
            }
        }

        if (result || !(mXmlList.isEmpty())) {
            MyLogger.logD(CLASS_TAG, "the haixin sms data ");
            return false;
        }
        MyLogger.logD(CLASS_TAG, "mtk sms data ");
        return true;
    }

    /*    *//**
     * create the file
     * 
     * @param file
     * @return
     */
    /*
     * public static boolean createFileorFolder(File file) { boolean success =
     * true; if (file != null) { File dir = file.getParentFile(); if (dir !=
     * null && !dir.exists()) { dir.mkdirs(); }
     * 
     * try { if (file.isFile()) { success = file.createNewFile(); } else {
     * success = file.mkdirs(); } } catch (IOException e) { success = false;
     * Log.d(CLASS_TAG, "createFile() failed !cause:" + e.getMessage());
     * e.printStackTrace(); } } return success; }
     */

    /**
     * see if the file exsit
     * 
     * @param filePath
     * @return
     */
    // public static boolean isFileExist(String filePath) {
    // File file = new File(filePath);
    // return file.exists();
    // }

    /*
     * public static String getNameWithoutExt(String fileName) { String
     * nameWithoutExt = fileName; int iExtPoint = fileName.lastIndexOf("."); if
     * (iExtPoint != -1) { nameWithoutExt = fileName.substring(0, iExtPoint); }
     * return nameWithoutExt; }
     */
    /*
     * public static long bytes2MB(long bytes) { return bytes2KB(bytes) / 1024;
     * }
     */

    public static long bytes2KB(long bytes) {
        return bytes / 1024;
    }

    /**
     * return the filename's ext
     * 
     * @param file
     * @return
     */
    public static String getExt(File file) {
        if (file == null) {
            return null;
        }
        return getExt(file.getName());
    }

    /**
     * return the filename's ext
     * 
     * @param fileName
     * @return
     */
    public static String getExt(String fileName) {
        if (fileName == null) {
            return null;
        }
        String ext = null;

        int iLastOfPoint = fileName.lastIndexOf(".");
        if (iLastOfPoint != -1) {
            ext = fileName.substring(iLastOfPoint + 1, fileName.length());
        }
        return ext;
    }

    public static double round(double value, int scale, int roundingMode) {
        BigDecimal bd = new BigDecimal(value);
        bd = bd.setScale(scale, roundingMode);
        double d = bd.doubleValue();
        bd = null;
        return d;
    }

    public static long computeAllFileSizeInFolder(File folderFile) {
        long size = 0;
        if (folderFile != null) {
            if (folderFile.isFile()) {
                size = folderFile.length();
            } else if (folderFile.isDirectory()) {
                File[] files = folderFile.listFiles();
                if(files == null) {
                    return 0;
                }
                for (File file : files) {
                    if (file.isDirectory()) {
                        size += computeAllFileSizeInFolder(file);
                    } else if (file.isFile()) {
                        size += file.length();
                    }
                }
            }
        }
        return size;
    }

    public static boolean isEmptyFolder(File folderName) {
        boolean ret = true;

        if (folderName != null && folderName.exists()) {
            if (folderName.isFile()) {
                ret = false;
            } else {
                File[] files = folderName.listFiles();
                if (files != null) {
                    for (File file : files) {
                        if (!isEmptyFolder(file)) {
                            ret = false;
                            break;
                        }
                    }
                }
            }
        }
        return ret;
    }

    public static boolean deleteFileOrFolder(File file) {
        boolean result = true;
        if (file == null || !file.exists()) {
            return result;
        }
        if (file.isFile()) {
            return file.delete();
        } else if (file.isDirectory()) {
            File[] files = file.listFiles();
            if (files != null) {
                for (File f : files) {
                    if (!deleteFileOrFolder(f)) {
                        result = false;
                    }
                }
            }
            if (!file.delete()) {
                result = false;
            }
        }
        return result;
    }

    public static ArrayList<File> getAllApkFileInFolder(File file) {
        if (file == null) {
            return null;
        }
        if (!file.exists() || file.isFile()) {
            return null;
        }
        ArrayList<File> list = new ArrayList<File>();
        File[] files = file.listFiles();
        for (File f : files) {
            String ext = getExt(f);
            if (ext != null && ext.equalsIgnoreCase("apk")) {
                list.add(f);
            }
        }
        return list;
    }

    /**
     * 
     * @param context
     * @param archiveFilePath
     *            apk absolute path name
     * @return AppSnippet
     */
    public static AppSnippet getAppSnippet(Context context, String archiveFilePath) {

        PackageParser packageParser = new PackageParser(archiveFilePath);

        File sourceFile = new File(archiveFilePath);
        DisplayMetrics metrics = new DisplayMetrics();
        metrics.setToDefaults();
        PackageParser.Package pkg = packageParser.parsePackage(sourceFile, archiveFilePath, metrics, 0);
        if (pkg == null) {
            return null;
        }

        ApplicationInfo appInfo = pkg.applicationInfo;
        Resources pRes = context.getResources();

        AssetManager assmgr = new AssetManager();
        assmgr.addAssetPath(archiveFilePath);
        Resources res = new Resources(assmgr, pRes.getDisplayMetrics(), pRes.getConfiguration());
        CharSequence label = null;
        // Try to load the label from the package's resources. If an app has not
        // explicitly
        // specified any label, just use the package name.
        if (appInfo.labelRes != 0) {
            try {
                label = res.getText(appInfo.labelRes);
            } catch (Resources.NotFoundException e) {
                e.printStackTrace();
            }
        }
        if (label == null) {
            label = (appInfo.nonLocalizedLabel != null) ? appInfo.nonLocalizedLabel : appInfo.packageName;
        }
        Drawable icon = null;
        // Try to load the icon from the package's resources. If an app has not
        // explicitly
        // specified any resource, just use the default icon for now.
        if (appInfo.icon != 0) {
            try {
                icon = res.getDrawable(appInfo.icon);
            } catch (Resources.NotFoundException e) {
                e.printStackTrace();
            }
        }
        if (icon == null) {
            icon = context.getPackageManager().getDefaultActivityIcon();
        }
        AppSnippet snippet = new AppSnippet(icon, label, appInfo.packageName);
        snippet.setFileName(archiveFilePath);
        return snippet;
    }
    
    public static void fileProber(File file) {
        File parentFile = file.getParentFile();
        if (!parentFile.exists()) {
            fileProber(parentFile);
            parentFile.mkdir();
            android.os.FileUtils.setPermissions(parentFile.toString(), 0777, -1, -1);
        }
    }

    // public static AppSnippet getAppSnippet(Context context, String apkPath) {
    // String PATH_PackageParser = "android.content.pm.PackageParser";
    // String PATH_AssetManager = "android.content.res.AssetManager";
    // try {
    //
    // Class pkgParserCls = Class.forName(PATH_PackageParser);
    // Class[] typeArgs = new Class[1];
    // typeArgs[0] = String.class;
    // Constructor pkgParserCt = pkgParserCls.getConstructor(typeArgs);
    // Object[] valueArgs = new Object[1];
    // valueArgs[0] = apkPath;
    // Object pkgParser = pkgParserCt.newInstance(valueArgs);
    // Log.d("ANDROID_LAB", "pkgParser:" + pkgParser.toString());
    // DisplayMetrics metrics = new DisplayMetrics();
    // metrics.setToDefaults();
    // // PackageParser.Package mPkgInfo = packageParser.parsePackage(new
    // // File(apkPath), apkPath,
    // // metrics, 0);
    // typeArgs = new Class[4];
    // typeArgs[0] = File.class;
    // typeArgs[1] = String.class;
    // typeArgs[2] = DisplayMetrics.class;
    // typeArgs[3] = Integer.TYPE;
    // Method pkgParser_parsePackageMtd =
    // pkgParserCls.getDeclaredMethod("parsePackage",
    // typeArgs);
    // valueArgs = new Object[4];
    // valueArgs[0] = new File(apkPath);
    // valueArgs[1] = apkPath;
    // valueArgs[2] = metrics;
    // valueArgs[3] = 0;
    // Object pkgParserPkg = pkgParser_parsePackageMtd.invoke(pkgParser,
    // valueArgs);
    //
    // Field appInfoFld =
    // pkgParserPkg.getClass().getDeclaredField("applicationInfo");
    // ApplicationInfo appInfo = (ApplicationInfo) appInfoFld.get(pkgParserPkg);
    // Log.d("ANDROID_LAB", "pkg:" + appInfo.packageName + " uid=" +
    // appInfo.uid);
    // Class assetMagCls = Class.forName(PATH_AssetManager);
    // Constructor assetMagCt = assetMagCls.getConstructor((Class[]) null);
    // Object assetMag = assetMagCt.newInstance((Object[]) null);
    // typeArgs = new Class[1];
    // typeArgs[0] = String.class;
    // Method assetMag_addAssetPathMtd =
    // assetMagCls.getDeclaredMethod("addAssetPath",
    // typeArgs);
    // valueArgs = new Object[1];
    // valueArgs[0] = apkPath;
    // assetMag_addAssetPathMtd.invoke(assetMag, valueArgs);
    // Resources res = context.getResources();
    // typeArgs = new Class[3];
    // typeArgs[0] = assetMag.getClass();
    // typeArgs[1] = res.getDisplayMetrics().getClass();
    // typeArgs[2] = res.getConfiguration().getClass();
    // Constructor resCt = Resources.class.getConstructor(typeArgs);
    // valueArgs = new Object[3];
    // valueArgs[0] = assetMag;
    // valueArgs[1] = res.getDisplayMetrics();
    // valueArgs[2] = res.getConfiguration();
    // res = (Resources) resCt.newInstance(valueArgs);
    // CharSequence label = null;
    // // Try to load the label from the package's resources. If an app has not
    // explicitly
    // // specified any label, just use the package name.
    // if (appInfo.labelRes != 0) {
    // try {
    // label = res.getText(appInfo.labelRes);
    // } catch (Resources.NotFoundException e) {
    //
    // }
    // }
    // if (label == null) {
    // label = (appInfo.nonLocalizedLabel != null) ?
    // appInfo.nonLocalizedLabel : appInfo.packageName;
    // }
    // Drawable icon = null;
    // // Try to load the icon from the package's resources. If an app has not
    // explicitly
    // // specified any resource, just use the default icon for now.
    // if (appInfo.icon != 0) {
    // try {
    // icon = res.getDrawable(appInfo.icon);
    // } catch (Resources.NotFoundException e) {
    // }
    // }
    // if (icon == null) {
    // icon = context.getPackageManager().getDefaultActivityIcon();
    // }
    // return new AppSnippet(icon, label, appInfo.packageName);
    // } catch (Exception e) {
    // e.printStackTrace();
    // }
    // return null;
    // }
}
