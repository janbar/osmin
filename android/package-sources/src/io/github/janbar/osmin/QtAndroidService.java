package io.github.janbar.osmin;

import android.content.Context;
import android.content.Intent;
import android.util.Log;
import org.qtproject.qt5.android.bindings.QtService;

import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.os.Build;

public class QtAndroidService extends QtService
{
    private static final String TAG = "qt_service";

    @Override
    public void onCreate() {
        super.onCreate();
        Log.i(TAG, "Creating Service");
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        Log.i(TAG, "Destroying Service");
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        int ret = super.onStartCommand(intent, flags, startId);
        Log.i(TAG, "Service started");

        createNotificationChannel(getResources().getString(R.string.svc_channel));
        Notification.Builder builder = new Notification.Builder(this);
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            builder.setChannelId(CHANNEL_ID);
        }
        builder.setContentText(getResources().getString(R.string.svc_running))
               .setSmallIcon(android.R.drawable.ic_menu_mylocation);

        startForeground(1, builder.build());

        return START_STICKY;
    }

    private static String CHANNEL_ID = "ForegroundServiceChannel";

    private void createNotificationChannel(String name) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            NotificationChannel serviceChannel = new NotificationChannel(
                    CHANNEL_ID, name,
                    NotificationManager.IMPORTANCE_DEFAULT
            );
            NotificationManager manager = getSystemService(NotificationManager.class);
            manager.createNotificationChannel(serviceChannel);
        }
    }

    public static void startQtAndroidService(Context context) {
        Log.i(TAG, "Starting Service");
        context.startService(new Intent(context, QtAndroidService.class));
    }

    public static void stopQtAndroidService(Context context) {
        Log.i(TAG, "Stopping Service");
        context.stopService(new Intent(context, QtAndroidService.class));
    }
}
