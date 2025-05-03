package io.github.janbar.osmin;

import android.os.Build;
import android.os.Bundle;
import android.util.Log;
import android.view.KeyEvent;
import android.window.OnBackInvokedDispatcher;
import android.content.ComponentCallbacks2;

import org.qtproject.qt.android.QtActivityBase;

public class QtAndroidActivity extends QtActivityBase
{
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);

        // handle back pressed from android-33
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
            getOnBackInvokedDispatcher()
                    .registerOnBackInvokedCallback(
                            OnBackInvokedDispatcher.PRIORITY_DEFAULT,
                            () -> {
                                Log.d("osmin", "Handle BACK PRESSED");
                                super.onKeyDown(
                                        KeyEvent.KEYCODE_PAGE_UP,
                                        new KeyEvent(KeyEvent.ACTION_DOWN, KeyEvent.KEYCODE_PAGE_UP));
                            });
        }
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event)
    {
        // handle back pressed up to android-32
        if ((keyCode == KeyEvent.KEYCODE_BACK)) {
            Log.d("osmin", "Handle BACK PRESSED LEGACY");
            return super.onKeyDown(
                    KeyEvent.KEYCODE_PAGE_UP,
                    new KeyEvent(KeyEvent.ACTION_DOWN, KeyEvent.KEYCODE_PAGE_UP));
        }
        return super.onKeyDown(keyCode, event);
    }

    @Override
    public void onTrimMemory(int level)
    {
        if (level >= ComponentCallbacks2.TRIM_MEMORY_RUNNING_CRITICAL) {
            Log.w("osmin", "Memory advise: RUNNING_CRITICAL");
            JNI.onTrimMemory(5); // keep 5sec
        } else if (level >= ComponentCallbacks2.TRIM_MEMORY_RUNNING_LOW) {
            Log.w("osmin", "Memory advise: RUNNING_LOW");
            JNI.onTrimMemory(30); // keep 30sec
        } else if (level >= ComponentCallbacks2.TRIM_MEMORY_RUNNING_MODERATE) {
            Log.w("osmin", "Memory advise: RUNNING_MODERATE");
        }
        super.onTrimMemory(level);
        // adb shell am send-trim-memory io.github.janbar.osmin RUNNING_LOW
    }
}

