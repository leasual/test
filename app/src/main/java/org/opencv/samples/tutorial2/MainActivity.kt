package org.opencv.samples.tutorial2

import android.app.Activity
import android.content.Intent
import android.os.Bundle
import com.ut.sdk2.R
import kotlinx.android.synthetic.main.activity_main.*
import kotlinx.android.synthetic.main.activity_main.view.*

class MainActivity :Activity() {
    var checked = R.id.internal
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        b640.setOnClickListener {
            var intent = Intent(this,DetectActitvity::class.java)
            intent.putExtra("px",640)
            intent.putExtra("store",checked)
            startActivity(intent)
        }
        b720.setOnClickListener {
            var intent = Intent(this,DetectActitvity::class.java)
            intent.putExtra("px",720)
            intent.putExtra("store",checked)
            startActivity(intent)
        }
        back.setOnClickListener {
            finish()
        }
        radio_g.setOnCheckedChangeListener { group, checkedId ->
            checked = checkedId
        }
    }

}