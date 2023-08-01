package com.example.dmccdemo;

import static android.content.ContentValues.TAG;

import android.annotation.SuppressLint;
import android.content.Intent;
import android.graphics.Color;
import android.media.MediaPlayer;
import android.os.Bundle;
import android.text.Editable;
import android.text.InputFilter;
import android.text.TextWatcher;
import android.util.Log;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.Surface;
import android.view.SurfaceView;
import android.view.View;
import android.widget.CompoundButton;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.MediaController;
import android.widget.RelativeLayout;
import android.widget.Switch;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.VideoView;

import androidx.appcompat.app.AppCompatActivity;

public class TestActivity extends AppCompatActivity implements View.OnClickListener, View.OnTouchListener {

    static {
        System.loadLibrary("native-lib");
    }

    private EditText rtspTxt;
    private ImageView jianImg;
    private EditText numEdit;
    private ImageView jiaImg;
    private Switch switchBtn;
    VideoView mVideoView;
    private static int NUM_VIDEOS = 5; // 定义要创建的视频数量
    private int videoWidth = 100; // 设置视频宽度
    private int videoHeight = 100; // 设置视频高度
    private long lastClickTime = 0;
    private static final boolean mStopFlag = false;
    private static final long DOUBLE_CLICK_TIME_DELTA = 300; // 双击事件的时间间隔阈值


    private String uri = "rtsp://192.168.20.222:8554/10002/20230801090441";


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_test);
        initView();
    }

    private void initView() {
        rtspTxt = findViewById(R.id.rtsp_txt);
        jianImg = findViewById(R.id.jian_img);
        numEdit = findViewById(R.id.num_edit);
        jiaImg = findViewById(R.id.jia_img);
        switchBtn = findViewById(R.id.switch_btn);
        numEdit.setText(NUM_VIDEOS + "");
        rtspTxt.setText(uri);
        numEdit.setFilters(new InputFilter[]{new InputFilterMinMax(1, 100)});

        jiaImg.setOnClickListener(this);
        jianImg.setOnClickListener(this);
        numEdit.addTextChangedListener(textWatcher);

        //开关按钮设置监听状态改变事件
        switchBtn.setOnCheckedChangeListener((compoundButton, b) -> {
            System.out.println("开关按钮的状态 = " + b);
            if(b){
                startRtspWindows();
            }else {
                closeAllRtspWindows();
            }
            //设置改变字体颜色
            switchBtn.setSwitchTextAppearance(TestActivity.this, b ? R.style.switchStyleCheck : R.style.switchStyleDefault);
        });
    }


    private TextWatcher textWatcher = new TextWatcher() {
        @Override
        public void beforeTextChanged(CharSequence s, int start, int count, int after) {

        }

        @Override
        public void onTextChanged(CharSequence s, int start, int before, int count) {

        }

        @Override
        public void afterTextChanged(Editable s) {
            // 在这里处理输入完成后的逻辑
            String inputText = s.toString().trim();
            if (!inputText.isEmpty()) {
                NUM_VIDEOS = Integer.parseInt(inputText);
                // 在这里可以对输入的值进行处理，比如保存到变量中，显示提示等
            }

        }
    };

    //开多个窗口
    public void startRtspWindows(){
        container = findViewById(R.id.container);
        NUM_VIDEOS = Integer.parseInt(numEdit.getText().toString());
        for (int i = 0; i < NUM_VIDEOS; i++) {
            addRtspWindow(i);
            try {
                Thread.sleep(500);
            } catch (InterruptedException e) {
                throw new RuntimeException(e);
            }
        }
    }

    private String type = "hard";
    RelativeLayout container;

    @SuppressLint("ClickableViewAccessibility")
    private void addRtspWindow(int i) {
        int videoWidth = 800; // 设置视频宽度
        int videoHeight = 600; // 设置视频高度
        //设置 VideoView 的位置和大小
        RelativeLayout.LayoutParams params = new RelativeLayout.LayoutParams(videoWidth, videoHeight);
        params.leftMargin = i * 100; // 设置每个 VideoView 的水平间距
        switch (type){
            case "hard":
                // Create and start a new RTSP window
                MediaCodecSurfaceViews surfaceView = new MediaCodecSurfaceViews(this);
                // 设置 VideoView 的 ID，方便后续引用
                surfaceView.setId(View.generateViewId());
                surfaceView.setWidthAndHeight(1920,1080);
                //设置 VideoView 的位置和大小
                surfaceView.setLayoutParams(params);
                surfaceView.setOnTouchListener(TestActivity.this); // 为每个 VideoView 添加触摸事件监听器
                container.addView(surfaceView);
                new MyFloatingThread(surfaceView).start();
                break;
            case "soft":
                // Create and start a new RTSP window
                YuvPlayerView yuvPlayerView = new YuvPlayerView(this);
                // 设置 VideoView 的 ID，方便后续引用
                yuvPlayerView.setId(View.generateViewId());
                yuvPlayerView.setWidthAndHeight(1920,1080);

                yuvPlayerView.setLayoutParams(params);
                yuvPlayerView.setOnTouchListener(TestActivity.this); // 为每个 VideoView 添加触摸事件监听器
                container.addView(yuvPlayerView);
                new MyFloatingThread().start();
                break;
        }
    }

    private void removeRtspWindow() {
        // Remove the last RTSP window if available
        int childCount = container.getChildCount();
        if (childCount > 0) {
            View lastChild = container.getChildAt(childCount - 1);
            container.removeView(lastChild);
        }
    }

    //关闭所有RTSP窗口
    private void closeAllRtspWindows() {
        container.removeAllViews();
    }

    @Override
    public void onClick(View v) {
        if(v.getId() == R.id.jian_img){
            if(NUM_VIDEOS > 1){
                NUM_VIDEOS --;
                numEdit.setText(NUM_VIDEOS + "");
                if(switchBtn.isChecked()){
                    removeRtspWindow();
                }
            }else {
                Toast.makeText(this,"开窗数量不能少于0",Toast.LENGTH_SHORT).show();
            }
        }else if (v.getId() == R.id.jia_img){
            if(NUM_VIDEOS <= 100){
                NUM_VIDEOS ++;
                numEdit.setText(NUM_VIDEOS + "");
                if(switchBtn.isChecked()){
                    addRtspWindow(0);
                }
            }else {
                Toast.makeText(this,"开窗数量不能大于100",Toast.LENGTH_SHORT).show();
            }
        }
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
                v.setTag(R.id.tag_video_view_origin_y, y);
                break;
        }
        return true;
    }

    class MyFloatingThread extends Thread {
        private  MediaCodecSurfaceViews surfaceView;

        public MyFloatingThread() {
        }
        
        public MyFloatingThread(MediaCodecSurfaceViews surfaceView) {
            this.surfaceView = surfaceView;
        }

        @Override
        public void run() {
            int times = 0;
            while (times < 30) {
                if (type.contains("hard") ? hardRecScreen(surfaceView) :  softRecScreen()) {
                    break;
                } else {
                    times++;
                    Log.e(TAG, "not rec" + times +"次");
                }

            }
        }
    }

    private boolean hardRecScreen(MediaCodecSurfaceViews mSurfaceView) {
        try {
            int receiver = createRtspReceiver(uri);
            if (receiver != 0) {
                releaseRtspRecv();
                Thread.sleep(2000);
                return false;
            }

            boolean isFirstFrame = true;

            Log.e(TAG, "createReceiver" + receiver);
            int continuous = 0;
            while(!mStopFlag) {

                byte[] streamBuffer = recvRtspVideoPacket(30);
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
            }
        } catch (Exception e) {
            Log.e(TAG, "recScreen: ", e);
        }
        return true;
    }

    private boolean softRecScreen() {

        InitffmpegRTP();//初始化软解码
        
        try {
            int receiver = createRtspReceiver(uri);
            if (receiver != 0) {
                ReleaseFFMPEGRTP();
                Thread.sleep(2000);
                return false;
            }

            boolean isFirstFrame = true;

            Log.e(TAG, "createReceiver" + receiver);
            int continuous = 0;
            while(!mStopFlag) {

                byte[] streamBuffer = recvRtspVideoPacket(30);
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
                SetFFMPEGRTPData(streamBuffer, streamBuffer.length);
            }
        } catch (Exception e) {
            Log.e(TAG, "recScreen: ", e);
        }
        return true;
    }

    private void handleDoubleClick(View v) {
        // 在这里处理双击事件的逻辑
        // 例如，执行双击后的操作或显示提示
        int videoWidth = 200; // 设置视频宽度
        int videoHeight = 150; // 设置视频高度
        // 设置 VideoView 的位置和大小
        RelativeLayout.LayoutParams params = new RelativeLayout.LayoutParams(videoWidth, videoHeight);
        v.setLayoutParams(params);
    }

    public static native int createRtspReceiver(String url);

    public static native byte[] recvRtspVideoPacket(int timeout);

    public static native void releaseRtspRecv();

    public static native int InitffmpegRTP();

    public static native int PlayffmpegRTPYUV(Surface surface, int screenWidth, int screenHeight);

    public static native int SetFFMPEGRTPData(byte[] data, int dataLength);

    public static native int ReleaseFFMPEGRTP();

    @Override
    protected void onDestroy() {
        super.onDestroy();
        releaseRtspRecv();
    }
}
