package com.op.dm.widget

import android.app.Activity
import android.content.Context
import android.text.Editable
import android.text.InputType
import android.text.TextWatcher
import android.text.method.DigitsKeyListener
import android.util.AttributeSet
import android.view.Gravity
import android.widget.EditText
import android.widget.LinearLayout
import android.widget.TextView
import com.ut.sdk.R

class SettingView @JvmOverloads constructor(context: Context, attrs: AttributeSet? = null, defStyleAttr: Int = 0) : LinearLayout(context, attrs, defStyleAttr) {
    var tag1: Array<TextView>? = null
    var tagText = arrayOf("视频录制时间: ","拍照张数: ","拍照间隔: ")
    val defalt = arrayOf("5", "3", "2")
    var title:TextView? = null



    fun setTitle(titlet:String){
        title?.text = titlet
    }

    fun setValue( key:String, value:String){
        var sp = (context as Activity).getPreferences(Context.MODE_PRIVATE)
        var edit = sp.edit()
        edit.putString(key,value)
    }

    fun getMemoValue( key:String, defaltV:String):String{
        var sp = (context as Activity).getPreferences(Context.MODE_PRIVATE)
        return sp.getString(key,defaltV)
    }

    init {
        orientation = VERTICAL
        var arry = context.obtainStyledAttributes(attrs, R.styleable.SettingView)
        var param = LayoutParams(LayoutParams.MATCH_PARENT, LayoutParams.WRAP_CONTENT)
        param.bottomMargin = 15
        title = TextView(context)
        title?.textSize = 17F
        var titless = arry.getString(R.styleable.SettingView_android_text)
        title?.text = titless
        title?.layoutParams = param
        addView(title)
        tagText.forEachIndexed { index, s ->
            var text = TextView(context)
            text.text = s
            var edit = EditText(context)
            val key = titless+s
            var value = getMemoValue(key,defalt[index])
            edit.setText(value)
            edit.inputType = InputType.TYPE_CLASS_NUMBER;
            edit.inputType = InputType.TYPE_NUMBER_FLAG_DECIMAL;
            edit.keyListener = DigitsKeyListener.getInstance(false,false)

            edit.addTextChangedListener(object:TextWatcher{
                override fun beforeTextChanged(s: CharSequence?, start: Int, count: Int, after: Int) {
                }

                override fun onTextChanged(s: CharSequence?, start: Int, before: Int, count: Int) {
                }

                override fun afterTextChanged(s: Editable?) {
                    s?.toString()?.let {
                        if(!it.isEmpty())
                            setValue(key, it)
                    }
                }
            })

            var lly = LinearLayout(context)
            var paramTextView = LayoutParams(LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT)
            paramTextView.gravity = Gravity.CENTER_VERTICAL
            paramTextView.leftMargin = 10
            text.layoutParams = paramTextView
            edit.layoutParams = paramTextView
            lly.addView(text)
            lly.addView(edit)
//            param.bottomMargin = 0
//            lly.layoutParams = param
            addView(lly)
        }
    }

}