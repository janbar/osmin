package io.github.janbar.osmin;

import android.os.Build;
import android.os.Bundle;
import android.util.Log;
import android.view.KeyEvent;
import android.window.OnBackInvokedDispatcher;

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
}

