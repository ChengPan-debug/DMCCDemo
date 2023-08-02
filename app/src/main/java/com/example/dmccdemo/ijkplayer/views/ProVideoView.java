package com.example.dmccdemo.ijkplayer.views;

import static tv.danmaku.ijk.media.player.IjkMediaPlayer.native_active_days;

import android.Manifest;
import android.content.Context;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.text.TextUtils;
import android.util.AttributeSet;


import java.io.File;
import java.text.SimpleDateFormat;
import java.util.Date;

import tv.danmaku.ijk.media.player.IjkMediaPlayer;
import tv.danmaku.ijk.media.widget.media.IjkVideoView;

/**
 * 播放器
 *
 * Created by apple on 2017/2/11.
 */
public class ProVideoView extends IjkVideoView implements VideoControllerView.FullScreenAbleMediaPlayerControl {

    private String mRecordPath;

    public ProVideoView(Context context) {
        super(context);
    }

    public ProVideoView(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    public ProVideoView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
    }

    public ProVideoView(Context context, AttributeSet attrs, int defStyleAttr, int defStyleRes) {
        super(context, attrs, defStyleAttr, defStyleRes);
    }

    public static void setKey(String key) {
        Player_KEY = key;
    }

    public static long getActiveDays(Context context, String key) {
        return native_active_days(context, key);
    }

    @Override
    public boolean isFullScreen() {
        return false;
    }

    @Override
    public void toggleFullScreen() {

    }

    @Override
    public boolean recordEnable() {
        Uri uri = mUri;

        if (uri == null)
            return false;

        if (uri.getScheme() == null)
            return false;

        return !uri.getScheme().equals("file");
    }

    @Override
    public boolean speedCtrlEnable() {
        Uri uri = mUri;

        if (uri == null)
            return false;

        if (uri.getScheme() == null)
            return true;

        return uri.getScheme().equals("file");
    }

    @Override
    public boolean isRecording() {
        if (mMediaPlayer == null) {
            return false;
        }

        return !TextUtils.isEmpty(mRecordPath);
    }

    @Override
    public void reStart() {
        super.reStart();
        if (mRecordPath != null){
            toggleRecord();
            toggleRecord();
        }
    }

    @Override
    public void toggleRecord() {
    }

    @Override
    public float getSpeed() {
        if (mMediaPlayer == null) {
            return 1.0f;
        }

        if (mMediaPlayer instanceof IjkMediaPlayer) {
            IjkMediaPlayer player = (IjkMediaPlayer) mMediaPlayer;
            return player.getSpeed();
        }

        return 1.0f;
    }

    @Override
    public void setSpeed(float speed) {
        if (mMediaPlayer == null ) {
            return ;
        }

        if (mMediaPlayer instanceof IjkMediaPlayer) {
            IjkMediaPlayer player = (IjkMediaPlayer) mMediaPlayer;
            player.setSpeed(speed);
        }
    }

    @Override
    public void takePicture() {

    }

    @Override
    public void toggleMode() {

    }

    @Override
    public boolean isCompleted() {
        if (mMediaPlayer instanceof IjkMediaPlayer) {
            IjkMediaPlayer player = (IjkMediaPlayer) mMediaPlayer;
            return player.isCompleted();
        }

        return false;
    }
}
