package com.op;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

import com.op.dm.ui.DetectActitvity;

public class MyReceiver extends BroadcastReceiver {

    @Override
    public void onReceive(Context context, Intent intent) {
        if (Intent.ACTION_BOOT_COMPLETED.equals(intent.getAction())) {
            Intent intent2 = new Intent(context, DetectActitvity.class);  // 要启动的Activity
            //1.如果自启动APP，参数为需要自动启动的应用包名
            //Intent intent = getPackageManager().getLaunchIntentForPackage(packageName);
            //下面这句话必须加上才能开机自动运行app的界面
            intent2.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            //2.如果自启动Activity
            //context.startActivity(intent);
            //3.如果自启动服务
            context.startActivity(intent2);
        }
    }
}
