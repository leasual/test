package com.op.dm.ui

import android.app.Activity
import android.content.Context
import android.os.Bundle
import android.util.AttributeSet
import android.view.View
import android.widget.AdapterView
import android.widget.ArrayAdapter
import android.widget.LinearLayout
import android.widget.TextView
import com.ut.sdk.R
import kotlinx.android.synthetic.main.setting_lly.*

class SettingActivity :Activity(){

    internal var aa = arrayListOf("352×288", "704×288","704×576","640×480","1280×720","1920×1080")

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.setting_lly)
        spinner_photo.adapter = ArrayAdapter<String>(this,android.R.layout.simple_list_item_1,aa)
        spinner_photo.onItemSelectedListener
        back.setOnClickListener { finish() }

    }

    internal inner class Lis : AdapterView.OnItemSelectedListener {

        override fun onItemSelected(parent: AdapterView<*>, view: View, position: Int, id: Long) {
            val text = aa[position]
            val numbers = text.split("×")


        }

        override fun onNothingSelected(parent: AdapterView<*>) {

        }
    }
}

class SettngView @JvmOverloads constructor(context: Context, attrs: AttributeSet? = null, defStyleAttr: Int = 0) : LinearLayout(context, attrs, defStyleAttr) {

    init {
        val tt = TextView(context)
    }
}