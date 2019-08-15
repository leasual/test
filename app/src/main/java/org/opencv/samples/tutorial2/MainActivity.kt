package org.opencv.samples.tutorial2

import android.app.Activity
import android.content.Intent
import android.os.Bundle
import com.ut.sdk2.R
import kotlinx.android.synthetic.main.activity_main.*
import kotlinx.android.synthetic.main.activity_main.view.*

class MainActivity :Activity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        b640.setOnClickListener {
            var intent = Intent(this,DetectActitvity::class.java)
            intent.putExtra("px",640)
            startActivity(intent)
        }
        b720.setOnClickListener {
            var intent = Intent(this,DetectActitvity::class.java)
            intent.putExtra("px",720)
            startActivity(intent)
        }
        back.setOnClickListener {
            finish()
        }
    }

}