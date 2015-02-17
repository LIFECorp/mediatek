/**
 * 2010-4-20
 */
package com.mediatek.backuprestore.utils;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;

import org.apache.commons.compress.archivers.tar.TarArchiveEntry;
import org.apache.commons.compress.archivers.tar.TarArchiveInputStream;
import org.apache.commons.compress.archivers.tar.TarArchiveOutputStream;
import libcore.io.Libcore;
import libcore.io.StructStat;
import libcore.io.ErrnoException;

import android.util.Log;
/**
 * TAR工具
 * 
 * @since 1.0
 */
public abstract class TarToolUtils {
    private static final String CLASS_TAG = MyLogger.LOG_TAG + "/TarToolUtils";
    private static final String BASE_DIR = "";

    // 符号"/"用来作为目录标识判断符
    private static final String PATH = "/";
    private static final int BUFFER = 1024;

    private static final String EXT = ".tar";
    private static int mode = 0;

    private static String sFolderName;
    private static final String TARPATH = "data/data/";

    /**
     * tar
     * 
     * @param srcPath
     * @param destPath
     * @throws IOException
     */
    public static void archive(String srcPath, String destPath) throws IOException {
        File srcFile = new File(srcPath);
        archive(srcFile, destPath);
    }

    /**
     * tar
     * 
     * @param srcFile
     * @param destPath
     * @throws IOException
     */
    public static void archive(File srcFile, File destFile) throws IOException {
        TarArchiveOutputStream taos = new TarArchiveOutputStream(new FileOutputStream(destFile));
        // archive(srcFile, taos, BASE_DIR);
        sFolderName = srcFile.getName() + File.separator;
        MyLogger.logD(CLASS_TAG,"archiveDir 63 : sFolderName= " +sFolderName + ",srcFile.getName() = " +srcFile.getName());
        File[] listFile = srcFile.listFiles();
        for (File file : listFile) {
            archive(file, taos, BASE_DIR);
        }
        taos.flush();
        taos.close();
    }

    /*    *//**
     * tar
     * 
     * @param srcFile
     * @throws IOException
     */
    /*
     * public static void archive(File srcFile) throws IOException { String name
     * = srcFile.getName(); String basePath = srcFile.getParent(); String
     * destPath = basePath + File.separator + name + EXT;
     * MyLogger.logD(CLASS_TAG, "destPath === " + destPath + ", name === " +
     * name + ", basePath === " + basePath); archive(srcFile, destPath); }
     */
    /**
     * tar file
     * 
     * @param srcFile
     * @param destPath
     * @throws IOException
     */
    public static void archive(File srcFile, String destPath) throws IOException {
        File desFile = new File(destPath);
        archive(srcFile, desFile.getAbsoluteFile());
    }

    /*    *//**
     * tar
     * 
     * @param srcPath
     * @throws IOException
     */
    /*
     * public static void archive(String srcPath) throws IOException { File
     * srcFile = new File(srcPath); archive(srcFile); }
     */

    /**
     * 
     * @param srcFile
     * @param taos
     *            TarArchiveOutputStream
     * @param basePath
     * @throws IOException
     */
    private static void archive(File srcFile, TarArchiveOutputStream taos, String basePath) throws IOException {
        if (srcFile.isDirectory()) {
            archiveDir(srcFile, taos, basePath);
        } else {
            archiveFile(srcFile, taos, basePath);
        }
    }

    /**
     * tar path
     * 
     * @param dir
     * @param taos
     *            TarArchiveOutputStream
     * @param basePath
     * @throws IOException
     */
    private static void archiveDir(File dir, TarArchiveOutputStream taos, String basePath) throws IOException {
        File[] files = dir.listFiles();
        MyLogger.logD(CLASS_TAG, "archiveDir : dir getAbsolutePath === " + dir.getAbsolutePath() + ", files length = "
                + files.length);
        // /M : get File Permissions from 4.2 merge
        mode = getMode(dir.toString());
        MyLogger.logD(CLASS_TAG,"archiveDir : dir mode ==== " + mode + ", dir name = " + dir.getName());
        TarArchiveEntry entry = new TarArchiveEntry(TARPATH + sFolderName + dir.getName() + PATH);
        entry.setMode(mode);
        MyLogger.logD(CLASS_TAG, " name == " + entry.getName() + ", linkname == " + entry.getLinkName()
                + ",  groupname == " + entry.getGroupName());
        taos.setLongFileMode(TarArchiveOutputStream.LONGFILE_GNU);
        taos.putArchiveEntry(entry);
        taos.closeArchiveEntry();
        if (files != null) {
            for (File file : files) {
                // loop
                archive(file, taos, basePath + dir.getName() + PATH);
            }
        }
    }

    /**
     * 数据归档
     * 
     * @param data
     *            待归档数据
     * @param path
     *            归档数据的当前路径
     * @param name
     *            归档文件名
     * @param taos
     *            TarArchiveOutputStream
     * @throws IOException
     */
    private static void archiveFile(File file, TarArchiveOutputStream taos, String dir) throws IOException {
        TarArchiveEntry entry = new TarArchiveEntry(TARPATH + sFolderName + dir + file.getName());
        mode = getMode(file.getPath());
        // name too long
        entry.setSize(file.length());
        // save file mode
        entry.setMode(getMode(file.getPath()));
        MyLogger.logD(CLASS_TAG, "save entry mode === " + mode + ", filepath == " + file.getPath());
        taos.setLongFileMode(TarArchiveOutputStream.LONGFILE_GNU);
        taos.putArchiveEntry(entry);

        BufferedInputStream bis = new BufferedInputStream(new FileInputStream(file));
        int count;
        byte data[] = new byte[BUFFER];
        while ((count = bis.read(data, 0, BUFFER)) != -1) {
            taos.write(data, 0, count);
        }
        bis.close();
        taos.closeArchiveEntry();
    }

    /*    *//**
     * 解归档
     * 
     * @param srcFile
     * @throws IOException
     */
    /*
     * public static void dearchive(File srcFile) throws IOException { String
     * basePath = srcFile.getParent() + File.separator; dearchive(srcFile,
     * basePath); }
     */

    /**
     * 解归档
     * 
     * @param srcFile
     * @param destFile
     * @throws IOException
     */
    public static void dearchive(File srcFile, File destFile) throws IOException {
        TarArchiveInputStream tais = new TarArchiveInputStream(new FileInputStream(srcFile));
        dearchive(destFile, tais);
        tais.close();
    }

    /**
     * 解归档
     * 
     * @param srcFile
     * @param destPath
     * @throws IOException
     */
    public static void dearchive(File srcFile, String destPath) throws IOException {
        dearchive(srcFile, new File(destPath));

    }

    /**
     * 文件 解归档
     * 
     * @param destFile
     *            目标文件
     * @param tais
     * @throws IOException
     */
    private static void dearchive(File destFile, TarArchiveInputStream tais) throws IOException {
        TarArchiveEntry entry = null;
        while ((entry = tais.getNextTarEntry()) != null) {
            // 文件
            String dir = destFile.getPath() + File.separator + entry.getName();
            File dirFile = new File(dir);
            MyLogger.logD(CLASS_TAG, "dearchive File  : entry.getName()  == " + entry.getName() + ", entry mode === "
                    + entry.getMode() + ", file : " + dirFile.toString());
            fileProber(dirFile);
            if (entry.isDirectory()) {
                dirFile.mkdirs();
                android.os.FileUtils.setPermissions(dirFile.getAbsolutePath(), entry.getMode(), -1, -1);
            } else {
                dearchiveFile(dirFile, tais, entry.getMode());
            }

        }
    }

    /*    *//**
     * @param srcPath
     * @throws IOException
     */
    /*
     * public static void dearchive(String srcPath) throws IOException { String
     * path = srcPath + EXT; File srcFile = new File(path); dearchive(srcFile);
     * }
     */

    /**
     * @param srcPath
     * @param destPath
     * @throws IOException
     */
    public static void dearchive(String srcPath, String destPath) throws IOException {
        File srcFile = new File(srcPath);
        dearchive(srcFile, destPath);
    }

    /**
     * 文件解归档
     * 
     * @param destFile
     * @param tais
     *            TarArchiveInputStream
     * @throws IOException
     */
    private static void dearchiveFile(File destFile, TarArchiveInputStream tais, int mode) throws IOException {
        BufferedOutputStream bos = new BufferedOutputStream(new FileOutputStream(destFile));
        int count;
        byte data[] = new byte[BUFFER];
        while ((count = tais.read(data, 0, BUFFER)) != -1) {
            bos.write(data, 0, count);
        }

        bos.close();
        // set file mode
        android.os.FileUtils.setPermissions(destFile.toString(), mode, -1, -1);
        MyLogger.logD(CLASS_TAG," dearchiveFile >>> set File Permissions : file name is = " + destFile.getName()
                + ", file path is = " + destFile.getPath() + ", the file mode = "+ mode);

    }
    
    /**
     * 
     * @param filePath
     * @return
     */
    private static int getMode(String filePath){
        try {
            StructStat stat = Libcore.os.stat(filePath);
            return stat.st_mode;
        } catch (ErrnoException e) {
            Log.e(CLASS_TAG, "Couldn't stat path " + filePath, e);
            return 0;
        }
    }

    private static void fileProber(File file) {
        File parentFile = file.getParentFile();
        if (!parentFile.exists()) {
            fileProber(parentFile);
            parentFile.mkdir();
            android.os.FileUtils.setPermissions(parentFile.toString(), 0777, -1, -1);
        }
    }
}
