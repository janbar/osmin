package io.github.janbar.osmin;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.os.Build;
import android.util.Log;
import android.view.Window;
import android.view.WindowManager;

import java.io.File;

import androidx.core.content.FileProvider;

public class QtAndroidHelper {

    private static final String TAG = "helper";

    public static int platformVersion() {
        return Build.VERSION.SDK_INT;
    }

    public static void preventBlanking(Context context, boolean on) {
        if (context instanceof Activity) {
            Log.i(TAG, "FLAG_KEEP_SCREEN: " + (on ? "ON" : "OFF"));
            Window window = ((Activity) context).getWindow();
            if (on)
                window.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
            else
                window.clearFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        }
    }

    public static boolean shareContent(Context context, String text, String path, String mimeType) {
        if (context instanceof Activity) {
            File file = new File(path);
            Uri uri = FileProvider.getUriForFile(context, context.getApplicationContext().getPackageName() + ".qtprovider", file);
            Intent shareIntent = new Intent();
            shareIntent.setAction(Intent.ACTION_SEND);
            shareIntent.putExtra(Intent.EXTRA_SUBJECT, text);
            shareIntent.putExtra(Intent.EXTRA_STREAM, uri);
            shareIntent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
            shareIntent.setType(mimeType);
            // Verify that the intent will resolve to an activity
            if (shareIntent.resolveActivity(((Activity) context).getPackageManager()) != null) {
                ((Activity) context).startActivity(shareIntent);
                return true;
            } else {
                Log.d("shareContent", "Intent not resolved");
            }
        }
        return false;
    }

    public static boolean shareData(Context context, String text, String data, String mimeType) {
        if (context instanceof Activity) {
            Intent shareIntent = new Intent();
            shareIntent.setAction(Intent.ACTION_SEND);
            shareIntent.putExtra(Intent.EXTRA_SUBJECT, text);
            shareIntent.putExtra(Intent.EXTRA_TEXT, data);
            shareIntent.setType(mimeType);
            // Verify that the intent will resolve to an activity
            if (shareIntent.resolveActivity(((Activity) context).getPackageManager()) != null) {
                ((Activity) context).startActivity(shareIntent);
                return true;
            } else {
                Log.d("shareData", "Intent not resolved");
            }
        }
        return false;
    }
}

