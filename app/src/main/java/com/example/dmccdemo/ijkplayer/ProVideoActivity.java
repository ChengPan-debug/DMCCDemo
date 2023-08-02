package com.example.dmccdemo.ijkplayer;

/*
 * Copyright (C) 2015 Zhang Rui <bbcallen@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import android.annotation.SuppressLint;
import android.content.ContentResolver;
import android.content.Intent;
import android.graphics.Color;
import android.media.MediaScannerConnection;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.text.TextUtils;
import android.util.Log;
import android.view.GestureDetector;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.widget.RelativeLayout;

import androidx.annotation.RequiresApi;
import androidx.appcompat.app.AppCompatActivity;
import androidx.databinding.DataBindingUtil;

import com.example.dmccdemo.R;
import com.example.dmccdemo.ijkplayer.views.ProVideoView;
import com.example.dmccdemo.ijkplayer.views.VideoControllerView;


import java.io.File;

import tv.danmaku.ijk.media.player.IMediaPlayer;
import tv.danmaku.ijk.media.player.IjkMediaPlayer;

public class ProVideoActivity extends AppCompatActivity implements View.OnTouchListener {
    private static final String TAG = "ProVideoActivity";

    public static final int REQUEST_WRITE_STORAGE = 111;

    private String mVideoPath;
    private Uri mVideoUri;

    private ProVideoView videoView,videoView2,videoView3,videoView4;        // 播放器View

    private GestureDetector detector;

    private VideoControllerView mediaController,mediaController2,mediaController3,mediaController4;
    private MediaScannerConnection mScanner;

    private Runnable mSpeedCalcTask;

    private int mMode;
    private int mStartX;
    private int mStartY;
    private int mEndX;
    private int mEndY;// 画面模式
    private RelativeLayout line1;

    @RequiresApi(api = Build.VERSION_CODES.CUPCAKE)
    @SuppressLint("ClickableViewAccessibility")
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        ProVideoView.setKey("EasyPlayer is free!");

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
            View decorView = getWindow().getDecorView();
            decorView.setSystemUiVisibility(View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN | View.SYSTEM_UI_FLAG_LAYOUT_STABLE);
            getWindow().setStatusBarColor(Color.TRANSPARENT);
        }

        setContentView(R.layout.activity_main_pro);
        line1 = findViewById(R.id.line1);
        videoView = findViewById(R.id.video_view);
        videoView2 = findViewById(R.id.video_view2);
        videoView3 = findViewById(R.id.video_view3);
        videoView4 = findViewById(R.id.video_view4);

        // handle arguments
        mVideoPath = "rtsp://192.168.20.222:8554/10002/20230802134750";

        Intent intent = getIntent();
        String intentAction = intent.getAction();
        if (!TextUtils.isEmpty(intentAction)) {
            if (intentAction.equals(Intent.ACTION_VIEW)) {
                mVideoPath = intent.getDataString();
            } else if (intentAction.equals(Intent.ACTION_SEND)) {
                mVideoUri = intent.getParcelableExtra(Intent.EXTRA_STREAM);

                if (Build.VERSION.SDK_INT < Build.VERSION_CODES.ICE_CREAM_SANDWICH) {
                    String scheme = mVideoUri.getScheme();

                    if (TextUtils.isEmpty(scheme)) {
                        Log.e(TAG, "Null unknown scheme\n");
                        finish();
                        return;
                    }

                    if (scheme.equals(ContentResolver.SCHEME_ANDROID_RESOURCE)) {
                        mVideoPath = mVideoUri.getPath();
                    } else if (scheme.equals(ContentResolver.SCHEME_CONTENT)) {
                        Log.e(TAG, "Can not resolve content below Android-ICS\n");
                        finish();
                        return;
                    } else {
                        Log.e(TAG, "Unknown scheme " + scheme + "\n");
                        finish();
                        return;
                    }
                }
            }
        }

        mediaController = new VideoControllerView(this);
        mediaController.setMediaPlayer(videoView);
        mediaController2 = new VideoControllerView(this);
        mediaController2.setMediaPlayer(videoView2);
        mediaController3 = new VideoControllerView(this);
        mediaController3.setMediaPlayer(videoView3);
        mediaController4 = new VideoControllerView(this);
        mediaController4.setMediaPlayer(videoView4);

        videoView .setOnInfoListener((iMediaPlayer, arg1, arg2) -> {
            switch (arg1) {
                case IMediaPlayer.MEDIA_INFO_VIDEO_TRACK_LAGGING:
                    Log.i(TAG, "MEDIA_INFO_VIDEO_TRACK_LAGGING");
                    break;
                case IMediaPlayer.MEDIA_INFO_VIDEO_RENDERING_START:
                    Log.i(TAG, "MEDIA_INFO_VIDEO_RENDERING_START");
                    videoView.setVisibility(View.VISIBLE);
                    videoView2.setVisibility(View.VISIBLE);

                    break;
                case IMediaPlayer.MEDIA_INFO_BUFFERING_START:
                    Log.i(TAG, "MEDIA_INFO_BUFFERING_START");
                    break;
                case IMediaPlayer.MEDIA_INFO_BUFFERING_END:
                    Log.i(TAG, "MEDIA_INFO_BUFFERING_END");
                    break;
                case IMediaPlayer.MEDIA_INFO_NETWORK_BANDWIDTH:
                    Log.i(TAG, "MEDIA_INFO_NETWORK_BANDWIDTH");
                    break;
                case IMediaPlayer.MEDIA_INFO_BAD_INTERLEAVING:
                    Log.i(TAG, "MEDIA_INFO_BAD_INTERLEAVING");
                    break;
                case IMediaPlayer.MEDIA_INFO_NOT_SEEKABLE:
                    Log.i(TAG, "MEDIA_INFO_NOT_SEEKABLE");
                    break;
                case IMediaPlayer.MEDIA_INFO_METADATA_UPDATE:
                    Log.i(TAG, "MEDIA_INFO_METADATA_UPDATE");
                    break;
                case IMediaPlayer.MEDIA_INFO_UNSUPPORTED_SUBTITLE:
                    Log.i(TAG, "MEDIA_INFO_UNSUPPORTED_SUBTITLE");
                    break;
                case IMediaPlayer.MEDIA_INFO_SUBTITLE_TIMED_OUT:
                    Log.i(TAG, "MEDIA_INFO_SUBTITLE_TIMED_OUT");
                    break;
                case IMediaPlayer.MEDIA_INFO_VIDEO_ROTATION_CHANGED:
                    Log.i(TAG, "MEDIA_INFO_VIDEO_ROTATION_CHANGED");
                    break;
                case IMediaPlayer.MEDIA_INFO_AUDIO_RENDERING_START:
                    Log.i(TAG, "MEDIA_INFO_AUDIO_RENDERING_START");
                    break;
            }

            return false;
        });

        if (mVideoPath != null) {
            videoView.setVideoPath(mVideoPath);
            videoView2.setVideoPath(mVideoPath);
            videoView3.setVideoPath(mVideoPath);
            videoView4.setVideoPath(mVideoPath);
        } else if (mVideoUri != null) {
            videoView.setVideoURI(mVideoUri);
            videoView2.setVideoURI(mVideoUri);
            videoView3.setVideoURI(mVideoUri);
            videoView4.setVideoURI(mVideoUri);
        } else {
            Log.e(TAG, "Null Data Source\n");
            finish();
            return;
        }

        videoView.start();

        videoView2.start();

        videoView3.start();

        videoView4.start();

        GestureDetector.SimpleOnGestureListener listener = new GestureDetector.SimpleOnGestureListener() {
            @Override
            public boolean onSingleTapConfirmed(MotionEvent e) {
                if (videoView.isInPlaybackState()) {
                    videoView.toggleMediaControlsVisibility();
                    videoView2.toggleMediaControlsVisibility();
                    return true;
                }

                return true;
            }

            @Override
            public boolean onDoubleTap(MotionEvent e) {
                return true;
            }
        };

        detector = new GestureDetector(this, listener);



        videoView.setOnTouchListener(this);
        videoView2.setOnTouchListener(this);
        videoView3.setOnTouchListener(this);
        videoView4.setOnTouchListener(this);

    }

    int offsetX;
    int offsetY;
    private View currentDraggingView; // 当前拖动的控件


    @Override
    protected void onStop() {
        super.onStop();
        videoView.stopPlayback();
        videoView2.stopPlayback();
        videoView3.stopPlayback();
        videoView4.stopPlayback();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();

        if (mScanner != null) {
            mScanner.disconnect();
            mScanner = null;
        }
    }

    private long lastClickTime = 0;
    private float scaleFactor = 1.0f;
    private static final long DOUBLE_CLICK_TIME_DELTA = 300; // 双击事件的时间间隔阈值

    @RequiresApi(api = Build.VERSION_CODES.HONEYCOMB)
    private void toggleZoom(View view) {
        if (scaleFactor == 1.0f) {
            // 缩小控件
            view.setScaleX(0.5f);
            view.setScaleY(0.5f);
            scaleFactor = 0.5f;
        } else {
            // 放大控件
            view.setScaleX(1.0f);
            view.setScaleY(1.0f);
            scaleFactor = 1.0f;
        }
    }


    @RequiresApi(api = Build.VERSION_CODES.O)
    @Override
    public boolean onTouch(View v, MotionEvent event) {
        // 获取控件在父容器中的位置
        int x = (int) event.getRawX();
        int y = (int) event.getRawY();

        switch (event.getAction()) {
            case MotionEvent.ACTION_DOWN:
                // 计算触摸点相对于控件左上角的偏移量
                offsetX = x - v.getLeft();
                offsetY = y - v.getTop();

                long clickTime = System.currentTimeMillis();
                long deltaTime = clickTime - lastClickTime;
                if (deltaTime < DOUBLE_CLICK_TIME_DELTA) {
                    // 在这里处理双击事件
                    toggleZoom(v);
                }
                lastClickTime = clickTime;
                // 点击时将窗口层级提升到最顶层
                currentDraggingView = v; // 记录当前拖动的控件
                // 将当前拖动的控件置顶
                v.bringToFront();
                break;
            case MotionEvent.ACTION_MOVE:
                // 更新控件的位置
                ViewGroup.MarginLayoutParams layoutParams = (ViewGroup.MarginLayoutParams) v.getLayoutParams();
                layoutParams.leftMargin = x - offsetX;
                layoutParams.topMargin = y - offsetY;
                v.setLayoutParams(layoutParams);
                break;
            case MotionEvent.ACTION_UP:
                // 松手后再进行超出边界检查，并删除控件
                int currentX = x - offsetX;
                int currentY = y - offsetY;
                int currentRight = currentX + v.getWidth();
                int currentTop = currentY + v.getHeight();

                if (currentX < 0 || currentRight > line1.getWidth() || currentTop > line1.getHeight()) {
                    line1.removeView(v);
                }
                break;
        }

        return true;
    }
}
