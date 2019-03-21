package com.op.dm.widget;

import android.content.Context;
import android.support.annotation.Nullable;
import android.util.AttributeSet;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.TextView;

import org.w3c.dom.Text;

public class SettngView extends LinearLayout {
    public SettngView(Context context) {
        this(context,null);
    }

    public SettngView(Context context, @Nullable AttributeSet attrs) {
        this(context, attrs,0);
    }

    public SettngView(Context context, @Nullable AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        EditText tt = new EditText(context);
        String ss = "";
        tt.setText(ss);
    }
}
