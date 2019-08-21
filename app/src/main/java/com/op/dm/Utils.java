package com.op.dm;

import android.Manifest;
import android.app.Activity;
import android.content.Context;
import android.content.res.AssetManager;
import android.os.StatFs;
import android.os.storage.StorageManager;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.text.TextUtils;
import android.util.Log;
import android.view.inputmethod.InputMethodManager;

import org.opencv.samples.tutorial2.DetectActitvity;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.DataOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.RandomAccessFile;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.zip.ZipEntry;
import java.util.zip.ZipOutputStream;

import static android.content.ContentValues.TAG;

/**
 * Created by chris on 12/10/18.
 */

public class Utils {

    public static void writeFlag(String text){
        String root = "/data/local/tmp";
//        String root = "/sdcard";
        File f = new File(root);
        if(!f.exists())
            f.mkdirs();
        String path = root + "/LIGHT_SWITCH.txt";

        try {
            BufferedWriter writer = new BufferedWriter(new FileWriter(path));
            writer.write(text);
            writer.close();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }



    public static class Volume {
        protected String path;
        protected boolean removable;
        protected String state;

        public String getPath() {
            return path;
        }

        public void setPath(String path) {
            this.path = path;
        }

        public boolean isRemovable() {
            return removable;
        }

        public void setRemovable(boolean removable) {
            this.removable = removable;
        }

        public String getState() {
            return state;
        }

        public void setState(String state) {
            this.state = state;
        }
    }


    public static ArrayList<Volume> getVolume(Context context) {
        ArrayList<Volume> list_storagevolume = new ArrayList<Volume>();

        StorageManager storageManager = (StorageManager) context.getSystemService(Context.STORAGE_SERVICE);

        try {
            Method method_volumeList = StorageManager.class.getMethod("getVolumeList");

            method_volumeList.setAccessible(true);

            Object[] volumeList = (Object[]) method_volumeList.invoke(storageManager);
            if (volumeList != null) {
                Volume volume;
                for (int i = 0; i < volumeList.length; i++) {
                    try {
                        volume = new Volume();
                        volume.setPath((String) volumeList[i].getClass().getMethod("getPath").invoke(volumeList[i]));
                        volume.setRemovable((boolean) volumeList[i].getClass().getMethod("isRemovable").invoke(volumeList[i]));
                        volume.setState((String) volumeList[i].getClass().getMethod("getState").invoke(volumeList[i]));
                        list_storagevolume.add(volume);
                    } catch (IllegalAccessException e) {
                        e.printStackTrace();
                    } catch (InvocationTargetException e) {
                        e.printStackTrace();
                    } catch (NoSuchMethodException e) {
                        e.printStackTrace();
                    }

                }
            } else {
                Log.e("null", "null-------------------------------------");
            }
        } catch (Exception e1) {
            e1.printStackTrace();
        }

        return list_storagevolume;
    }

    public static void rumCmd(final int arg1, final String arg2) {
        try {
            getForegroundApp();

            String[] cmdline = {"cd /system/bin", "./testcameralight.sh 0 -i"};
//            String[] cmdline = { "/system/bin/testcameralight 0 -r" };

            ShellUtil.CommandResult s = ShellUtil.execCommand(cmdline, false, true);
            Log.e("shell 1: ", s.result + " - " + s.errorMsg + " -" + s.successMsg);

            String[] cmdline2 = {"cd /sdcard/Android/data/com.ut.sdk2/files", "testcameralight 0 -i"};

            ShellUtil.CommandResult s2 = ShellUtil.execCommand(cmdline2, false, true);
            Log.e("shell 2: ", s2.result + " - " + s2.errorMsg + " -" + s2.successMsg);

        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private static String getForegroundApp() {
        String packageName = "";
        try {
            String command[] = new String[]{"sh", "-c",
                    "dumpsys activity | grep \"ACTIVITY MANAGER RECENT TASKS\" -A 10 |grep \"Recent tasks:\" -A 1"};
            //"dumpsys window | grep mCurrentFocus"
            Process process = Runtime.getRuntime().exec(command);
            int code = process.waitFor();
            Log.e(TAG, "code ： " + code);
            StringBuilder successMsg = new StringBuilder();
            StringBuilder errorMsg = new StringBuilder();
            BufferedReader successResult = new BufferedReader(new InputStreamReader(process.getInputStream()));
            BufferedReader errorResult = new BufferedReader(new InputStreamReader(process.getErrorStream()));
            String s;
            while ((s = successResult.readLine()) != null) {
                successMsg.append(s);
            }
            while ((s = errorResult.readLine()) != null) {
                errorMsg.append(s);
            }
            Log.e(TAG, "successMsg ： " + successMsg);
            Log.e(TAG, "errorMsg ： " + errorMsg);
            String content = successMsg.toString();
            if (content.contains("A") && content.contains("U")) {
                int indexA = content.lastIndexOf("A");
                int indexU = content.lastIndexOf("U");
                packageName = content.substring(indexA + 2, indexU - 1);
                Log.e(TAG, "content ： " + packageName);
                return packageName.trim();
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
        return packageName.trim();
    }

    public void  a(Activity context){
        ActivityCompat.requestPermissions(context,
                new String[]{Manifest.permission.READ_CONTACTS}, 1);
    }

    public static void addModeles(Context context) {
        try {

//            long size = getSDAvailableSize(context);
//            if(size!= 0){
//                File images = new File("/storage/sdcard1/images");
//                if(!images.exists()){
//                    images.mkdir();
//                }
//            }
            String[] files = context.getAssets().list("");
            int [] a = new int[2];
//            String storePathRoot = context.getExternalFilesDir(null).getAbsolutePath() == null ? context.getFilesDir().getAbsolutePath() : context.getExternalFilesDir(null).getAbsolutePath();
            String storePathRoot = "/storage/usb1";
            for (String dir :
                    files) {
                if ("images".equals(dir) || "webkit".equals(dir))
                    continue;
                copyFilesFassets(context, dir, storePathRoot + File.separator + dir);
            }
            File feature = new File(storePathRoot + File.separator + "feature");
            if (!feature.exists()) {
                feature.mkdir();
            }
        } catch (Exception e) {
            e.printStackTrace();
        }

    }

    public static long getSDAvailableSize(Context context) {
        ArrayList<Volume> list = getVolume(context);
        for (Volume v : list) {
            if (v.path.contains("sdcard1") && !v.state.contains("mounted")) {
                return 0;
            }
            Log.e("存储 ------ ", v.path);
            Log.e("state ------ ", v.state);
            Log.e("remove ------ ", v.removable+ "");
        }
//        File file = new File("/storage/sdcard1");
//        if(!file.exists()){
//            Log.e("File  storage/sdcard1) ", " not exist " );
//            return 0;
//        }

        StatFs stat = new StatFs("/storage/sdcard1");
        long blockSize = stat.getBlockSize();
        long availableBlocks = (stat.getAvailableBlocks() * blockSize) / (1024 * 1024);


        Log.e(" path size  ", " <> " + availableBlocks);
        return availableBlocks;
    }

    /**
     * 从assets目录中复制整个文件夹内容
     *
     * @param context     Context 使用CopyFiles类的Activity
     * @param path        String  原文件路径  如：/aa
     * @param storagePath String  复制后路径  如：xx:/bb/cc
     */
    public static void copyFilesFassets(Context context, String path, String storagePath) {
        try {
            String fileNames[] = context.getAssets().list(path);//获取assets目录下的所有文件及目录名
            if (fileNames.length > 0) {//如果是目录
                File file = new File(storagePath);
                file.mkdirs();//如果文件夹不存在，则递归
                for (String fileName : fileNames) {
                    copyFilesFassets(context, path + "/" + fileName, storagePath + "/" + fileName);
                }
            } else {//如果是文件
                File temp = new File(storagePath);
                if (temp.exists()) {
                    Log.e("tag", temp.getAbsolutePath() + " is exist --");
                    return;
                }
                InputStream is = context.getAssets().open(path);
                FileOutputStream fos = new FileOutputStream(temp);
                byte[] buffer = new byte[1024];
                int byteCount = 0;
                while ((byteCount = is.read(buffer)) != -1) {
                    fos.write(buffer, 0, byteCount);
                }
                fos.flush();
                is.close();
                fos.close();
            }
        } catch (Exception e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
    }


    private void CopyAssets(String assetDir, String dir, Context context) {
        String[] files;
        try {
            // 获得Assets一共有几多文件
            files = context.getResources().getAssets().list(assetDir);
        } catch (IOException e1) {
            return;
        }
//        File mWorkingPath = new File(dir);
        File mWorkingPath = context.getExternalFilesDir("");
        // 如果文件路径不存在
        if (!mWorkingPath.exists()) {
            // 创建文件夹
            if (!mWorkingPath.mkdirs()) {
                // 文件夹创建不成功时调用
            }
        }

        for (int i = 0; i < files.length; i++) {
            try {
                // 获得每个文件的名字
                String fileName = files[i];
                // 根据路径判断是文件夹还是文件
                if (!fileName.contains(".")) {
                    if (0 == assetDir.length()) {
                        CopyAssets(fileName, dir + fileName + "/", context);
                    } else {
                        CopyAssets(assetDir + "/" + fileName, dir + "/"
                                + fileName + "/", context);
                    }
                    continue;
                }
                File outFile = new File(mWorkingPath, fileName);
                if (outFile.exists())
                    outFile.delete();
                InputStream in = null;
                if (0 != assetDir.length())
                    in = context.getAssets().open(assetDir + "/" + fileName);
                else
                    in = context.getAssets().open(fileName);
                OutputStream out = new FileOutputStream(outFile);

                // Transfer bytes from in to out
                byte[] buf = new byte[1024];
                int len;
                while ((len = in.read(buf)) > 0) {
                    out.write(buf, 0, len);
                }
                in.close();
                out.close();
            } catch (FileNotFoundException e) {
                e.printStackTrace();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }

    }

    public static String time(){
        long time=System.currentTimeMillis();
        SimpleDateFormat format=new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
        Date d1 = new Date(time);
        return format.format(d1);
    }

    public static String time2(){
        long time=System.currentTimeMillis();
        SimpleDateFormat format=new SimpleDateFormat("MM-dd-HH:mm:ss");
        Date d1 = new Date(time);
        Log.e("时间戳 ：" , format.format(d1));
        return format.format(d1);
    }


    public static void copyAssets(Context context) {
        AssetManager assetManager = context.getAssets();
        String[] files = null;
        try {
            files = assetManager.list("");
        } catch (IOException e) {
            Log.e("tag", "Failed to get asset file list.", e);
        }
        if (files != null) for (String filename : files) {
            File outFile = new File(context.getExternalFilesDir(null), filename);
            fileOrDir(context, outFile, assetManager, filename);
        }
    }

    private static void randomfile(){
        RandomAccessFile f ;
        
    }

    private static void fileOrDir(Context context, File outFile, AssetManager assetManager, String fileName) {
        if (outFile.isFile()) {
            Log.e("tag", outFile.getAbsolutePath() + " is File --");
            createFile(context, outFile, assetManager);
        } else if (outFile.isDirectory()) {
            Log.e("tag", outFile.getAbsolutePath() + "  is DIR --");
//            String[] names = outFile.list();
            String[] names = new String[0];
            try {
                names = assetManager.list(fileName);
                for (String subFIle :
                        names) {
//                    File parent = new File(outFile,fileName);
                    File subFile = new File(outFile, subFIle);
                    fileOrDir(context, subFile, assetManager, "");
                }
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }


    private static void createFile(Context context, File outFile, AssetManager assetManager) {
        if (outFile.exists()) {
            Log.e("tag", "exist :-- " + outFile.getAbsoluteFile().getName());
            return;
        }

        InputStream in = null;
        OutputStream out = null;
        String filename = outFile.getName();
        if (filename.contains("image")) {
            Log.e("tag", ": " + filename);

        }
        try {
            Log.e("tag", " creat -- " + context.getExternalFilesDir(null) + "/" + outFile.getName());

            in = assetManager.open(filename);
        } catch (IOException e) {
            Log.e("tag", "Failed to copy asset file: " + filename, e);
            if (in != null) {
                try {
                    in.close();
                    in = null;
                } catch (IOException e1) {
                    e1.printStackTrace();
                    in = null;

                }
            }
        }

        if (in != null) {
            try {
                out = new FileOutputStream(outFile);

            } catch (FileNotFoundException e) {
                e.printStackTrace();
                if (out != null) {
                    try {
                        out.close();
                        out = null;
                    } catch (IOException e1) {
                        e1.printStackTrace();
                        out = null;
                    }

                }
            }

        }
        if (out != null) {
            try {
                copyFile(in, out);
            } catch (IOException e) {
                e.printStackTrace();
            }

        }
    }

    private static void copyFile(InputStream in, OutputStream out) throws IOException {
        byte[] buffer = new byte[1024];
        int read;
        while ((read = in.read(buffer)) != -1) {
            out.write(buffer, 0, read);
        }
    }
}
