package com.mediatek.mms.op01;

import android.app.AlertDialog;
import android.content.ActivityNotFoundException;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.net.Uri;
import android.os.Parcel;
import android.provider.Browser;
import android.text.style.URLSpan;
import android.util.Log;
import android.view.View;
import android.widget.TextView;

//import com.mediatek.encapsulation.com.mediatek.internal.EncapsulatedR;
import com.mediatek.internal.R;

public class ExtendURLSpan extends URLSpan {
    private static final String TAG = "ExtendURLSpan";

    public ExtendURLSpan(String url) {
        super(url);
    }

    public ExtendURLSpan(Parcel src) {
        super(src);
        // TODO Auto-generated constructor stub
    }

    @Override
    public void onClick(View widget) {
        Log.d(TAG, "onClick, widget = " + widget);
        Uri uri = Uri.parse(getURL());
        Log.d(TAG, "onClick, uri = " + uri);
        /// M: If the operator is CMCC and the getURL() is a web URL, show a chooser dialog to user. Otherwise start the activity directly. CR: ALPS00116591 @{
        final Context context = widget.getContext();
        final Intent intent = new Intent(Intent.ACTION_VIEW, uri);
        intent.putExtra(Browser.EXTRA_APPLICATION_ID, context.getPackageName());
        try {
            if (widget instanceof TextView) {
                Log.d(TAG, "it is a TextView");
                
                TextView textView = (TextView)widget;
                boolean isWebURL = false;
                String scheme = uri.getScheme();
                if (scheme != null) {
                    isWebURL = scheme.equalsIgnoreCase("http") || scheme.equalsIgnoreCase("https")
                            || scheme.equalsIgnoreCase("rtsp");
                }
                
                if (isWebURL) {
                    Log.d(TAG, "Yes it is a url");
                    AlertDialog.Builder builder = new AlertDialog.Builder(context);
                    builder.setTitle(R.string.url_dialog_choice_title);
                    builder.setMessage(R.string.url_dialog_choice_message);
                    builder.setCancelable(true);
                    
                    builder.setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
                        public final void onClick(DialogInterface dialog, int which) {
                            Log.d(TAG, "Press Yes");
                            context.startActivity(intent);
                            dialog.dismiss();
                        }
                    });
                    
                    builder.setNegativeButton(android.R.string.cancel, new DialogInterface.OnClickListener() {
                        public final void onClick(DialogInterface dialog, int which) {
                            Log.d(TAG, "Press no");
                            dialog.dismiss();
                        }
                    });
                    
                    builder.show();
                } else {
                    context.startActivity(intent);
                }
            } else {
                Log.d(TAG, "it is not a TextView");
                context.startActivity(intent);
            }
        } catch (ActivityNotFoundException e) {
            Intent chooserIntent = Intent.createChooser(intent, null);
            context.startActivity(chooserIntent);
        }
        /// @} 
    }
}
