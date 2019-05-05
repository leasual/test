package org.opencv.samples.tutorial2;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.support.annotation.Nullable;
import android.util.Log;

public class Mainactivity extends Activity {
    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        startActivityForResult(new Intent(this,DetectActitvity.class),100);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        System.exit(0);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        Log.e("onActivityResult ", "" + resultCode);
        Log.e("onActivityResult ", "" + requestCode);
        if(resultCode == -2)
            finish();
        if(requestCode == 100 && resultCode == RESULT_OK){
            Handler h = new Handler();
            h.postDelayed(new Runnable() {
                @Override
                public void run() {
                    startActivityForResult(new Intent(Mainactivity.this,DetectActitvity.class),100);
                }
            },1000);

        }
    }
}
