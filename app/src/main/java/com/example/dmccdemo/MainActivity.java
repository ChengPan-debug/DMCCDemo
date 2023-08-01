package com.example.dmccdemo;

import android.annotation.SuppressLint;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.view.WindowManager;
import android.widget.LinearLayout;
import android.widget.RelativeLayout;

import androidx.appcompat.app.AppCompatActivity;

public class MainActivity extends AppCompatActivity implements View.OnTouchListener {

    private MediaCodecSurfaceViews mSurfaceView;
    private MediaCodecSurfaceViews mSurfaceView1;
    private static final String TAG = "MainActivity";
    private static final boolean mStopFlag = false;
    private RelativeLayout container;
    private long lastClickTime = 0;
    private static final long DOUBLE_CLICK_TIME_DELTA = 300; // 双击事件的时间间隔阈值

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        initView();
    }

    @SuppressLint("ClickableViewAccessibility")
    private void initView() {
        container = findViewById(R.id.container);
        mSurfaceView = findViewById(R.id.surfaceViews);
        mSurfaceView1 = findViewById(R.id.surfaceViews1);

        mSurfaceView.setOnTouchListener(this);
        mSurfaceView1.setOnTouchListener(this);
        showWindows();
    }

    @SuppressLint("ClickableViewAccessibility")
    public void showWindows(){
        new MyFloatingThread().start();
        mSurfaceView.setWidthAndHeight(1280,800);
        mSurfaceView1.setWidthAndHeight(1280,800);
    }

    @Override
    public boolean onTouch(View v, MotionEvent event) {
        float x = event.getRawX();
        float y = event.getRawY();
        switch (event.getAction()) {
            case MotionEvent.ACTION_DOWN:
                // 当手指按下时，记录下初始位置，用于后续计算偏移量
                v.setTag(R.id.tag_video_view_origin_x, event.getRawX());
                v.setTag(R.id.tag_video_view_origin_y, event.getRawY());
                long clickTime = System.currentTimeMillis();
                //计算两次点击的时间间隔
                long deltaTime = clickTime - lastClickTime;
                if (deltaTime < DOUBLE_CLICK_TIME_DELTA) {
                    // 在这里处理双击事件
//                    handleDoubleClick(v);
                }
                lastClickTime = clickTime;
                break;
            case MotionEvent.ACTION_MOVE:
                // Calculate the delta change in position
                float dX = x - (float) v.getTag(R.id.tag_video_view_origin_x);
                float dY = y - (float) v.getTag(R.id.tag_video_view_origin_y);

                // Calculate the new position for the VideoView
                RelativeLayout.LayoutParams layoutParams = (RelativeLayout.LayoutParams) v.getLayoutParams();
                int newX = (int) (layoutParams.leftMargin + dX);
                int newY = (int) (layoutParams.topMargin + dY);

                // Get the dimensions of the RelativeLayout container
                int containerWidth = container.getWidth();
                int containerHeight = container.getHeight();

                // Calculate the maximum allowed position to stay within the RelativeLayout bounds
                int maxLeft = containerWidth - v.getWidth();
                int maxTop = containerHeight - v.getHeight();

                // Limit the new position to stay within the RelativeLayout bounds
                newX = Math.max(0, Math.min(newX, maxLeft));
                newY = Math.max(0, Math.min(newY, maxTop));

                // Update the layout parameters
                layoutParams.leftMargin = newX;
                layoutParams.topMargin = newY;
                v.setLayoutParams(layoutParams);

                // Update the initial position for the next move
                v.setTag(R.id.tag_video_view_origin_x, x);
                v.setTag(R.id.tag_video_view_origin_y, y);
                break;
        }
        return true;
    }



    class MyFloatingThread extends Thread {
        @Override
        public void run() {
            int times = 0;
            while (times < 30) {
                if (recScreen()) {
                    break;
                } else {
                    times++;
                    Log.e(TAG, "not rec" + times +"次");
                }

            }
        }
    }

    private boolean recScreen() {
        try {
            int receiver = TestActivity.createRtspReceiver("rtsp://192.168.20.222:8554/10002/20230729092638");
            if (receiver != 0) {
                TestActivity.releaseRtspRecv();
                Thread.sleep(2000);
                return false;
            }

            boolean isFirstFrame = true;

            Log.e(TAG, "createReceiver" + receiver);
            int continuous = 0;
            while(!mStopFlag) {

                byte[] streamBuffer = TestActivity.recvRtspVideoPacket(30);
                System.out.println(streamBuffer.length);
                if (streamBuffer.length == 0) {
                    continuous++;
                    Thread.sleep(30);
                    continue;
                } else {
                    continuous = 0;
                    if(isFirstFrame) {
                        isFirstFrame = false;
                        continue;
                    }
                }
                mSurfaceView.setData(streamBuffer);
                mSurfaceView1.setData(streamBuffer);
            }
        } catch (Exception e) {
            Log.e(TAG, "recScreen: ", e);
        }
        return true;
    }

}