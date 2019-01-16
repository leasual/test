package com.op.dm;

import android.content.Context;
import android.content.res.AssetManager;
import android.util.Log;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

/**
 * Created by chris on 12/10/18.
 */

public class Utils {
    public  static void addModeles(Context context){
        try {
            String [] files = context.getAssets().list("");
            String storePathRoot =  context.getExternalFilesDir(null).getAbsolutePath() == null? context.getFilesDir().getAbsolutePath() : context.getExternalFilesDir(null).getAbsolutePath();
            for (String dir :
                    files) {
                if("images".equals(dir) || "webkit".equals(dir))
                    continue;
                copyFilesFassets(context,dir,storePathRoot + File.separator + dir);
            }
            File feature = new File(storePathRoot + File.separator + "feature");
            if(!feature.exists()){
                feature.mkdir();
            }
        } catch (Exception e) {
            e.printStackTrace();
        }

    }
    /**
     *  从assets目录中复制整个文件夹内容
     *  @param  context  Context 使用CopyFiles类的Activity
     *  @param  path  String  原文件路径  如：/aa
     *  @param  storagePath  String  复制后路径  如：xx:/bb/cc
     */
    public static void copyFilesFassets(Context context,String path,String storagePath) {
        try {
            String fileNames[] = context.getAssets().list(path);//获取assets目录下的所有文件及目录名
            if (fileNames.length > 0) {//如果是目录
                File file = new File(storagePath );
                file.mkdirs();//如果文件夹不存在，则递归
                for (String fileName : fileNames) {
                    copyFilesFassets(context,path + "/" + fileName,storagePath+"/"+fileName);
                }
            } else {//如果是文件
                File temp = new File(storagePath);
                if(temp.exists()){
                    Log.e("tag",  temp.getAbsolutePath() + " is exist --");
                    return;
                }
                InputStream is = context.getAssets().open(path);
                FileOutputStream fos = new FileOutputStream(temp);
                byte[] buffer = new byte[1024];
                int byteCount=0;
                while((byteCount=is.read(buffer))!=-1) {
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


    private void CopyAssets(String assetDir, String dir,Context context) {
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
                        CopyAssets(fileName, dir + fileName + "/",context);
                    } else {
                        CopyAssets(assetDir + "/" + fileName, dir + "/"
                                + fileName + "/",context);
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
            }

            catch (IOException e) {
                e.printStackTrace();
            }
        }

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
            fileOrDir(context,outFile, assetManager,filename);
        }
    }

    private static void fileOrDir(Context context, File outFile,AssetManager assetManager,String fileName) {
        if(outFile.isFile()){
            Log.e("tag",  outFile.getAbsolutePath() + " is File --");
            createFile(context,outFile,assetManager);
        }else if(outFile.isDirectory()){
            Log.e("tag", outFile.getAbsolutePath() + "  is DIR --");
//            String[] names = outFile.list();
            String[] names = new String[0];
            try {
                names = assetManager.list(fileName);
                for (String subFIle :
                        names) {
//                    File parent = new File(outFile,fileName);
                    File subFile = new File(outFile, subFIle);
                    fileOrDir(context,subFile,assetManager,"");
                }
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }


    private static void createFile(Context context, File outFile, AssetManager assetManager) {
        if (outFile.exists()){
            Log.e("tag", "exist :-- " + outFile.getAbsoluteFile().getName());
            return;
        }

        InputStream in = null;
        OutputStream out = null;
        String filename = outFile.getName();
        if(filename.contains("image")){
            Log.e("tag", ": " + filename);

        }
        try {
            Log.e("tag", " creat -- " + context.getExternalFilesDir(null)+"/" + outFile.getName());

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

        if(in != null){
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
        if(out!= null){
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
