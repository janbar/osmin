package io.github.janbar.osmin;

import android.app.Activity;
import android.content.Context;
import android.util.Log;
import android.view.Window;
import android.view.WindowManager;

public class QtAndroidHelper {

    private static final String TAG = "helper";

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
}
