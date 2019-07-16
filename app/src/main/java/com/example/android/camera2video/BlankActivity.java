package com.example.android.camera2video;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;

import com.ut.sdk.R;

public class BlankActivity extends Activity {

    @Override
    protected void onCreate( Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.fragment_camera2_video);
        Intent intent = new Intent(this,CamerSer.class);
        startService(intent);
    }
}
