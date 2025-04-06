package io.github.janbar.osmin;

import android.util.Log;
import android.content.ComponentCallbacks2;
import org.qtproject.qt5.android.bindings.QtActivity;

public class QtAndroidActivity extends QtActivity
{
    @Override
    public void onTrimMemory(int level) {
        if (level >= ComponentCallbacks2.TRIM_MEMORY_RUNNING_CRITICAL) {
            Log.w("osmin", "Memory advise: RUNNING_CRITICAL");
        } else if (level >= ComponentCallbacks2.TRIM_MEMORY_RUNNING_LOW) {
            Log.w("osmin", "Memory advise: RUNNING_LOW");
        } else if (level >= ComponentCallbacks2.TRIM_MEMORY_RUNNING_MODERATE) {
            Log.w("osmin", "Memory advise: RUNNING_MODERATE");
        }
        super.onTrimMemory(level);
        // adb shell am send-trim-memory io.github.janbar.osmin RUNNING_LOW
    }
}

