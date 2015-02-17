package com.mediatek.vtdialer;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;


public class OP01VTDialerActivity extends Activity{

    @Override
    protected void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        
        Intent intent = new Intent(Intent.ACTION_DIAL);
        startActivity(intent);

        finish();
    }
}
