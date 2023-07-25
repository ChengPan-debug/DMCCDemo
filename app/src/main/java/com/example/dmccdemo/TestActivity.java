package com.example.dmccdemo;

import android.graphics.Color;
import android.media.MediaPlayer;
import android.os.Bundle;
import android.text.Editable;
import android.text.InputFilter;
import android.text.TextWatcher;
import android.view.KeyEvent;
import android.view.MotionEvent;
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

    private EditText rtspTxt;
    private ImageView jianImg;
    private EditText numEdit;
    private ImageView jiaImg;
    private Switch switchBtn;
    VideoView mVideoView;
    private static int NUM_VIDEOS = 10; // 定义要创建的视频数量
    private int videoWidth = 100; // 设置视频宽度
    private int videoHeight = 100; // 设置视频高度
    private long lastClickTime = 0;
    private static final long DOUBLE_CLICK_TIME_DELTA = 300; // 双击事件的时间间隔阈值


    private String uri = "rtsp://192.168.10.151/signal/cmd/H321T_____192.168.31.13-0";


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
//        numEdit.setOnEditorActionListener((v, actionId, event) -> false);
        switchBtn.addTextChangedListener(textWatcher);

        //开关按钮设置监听状态改变事件
        switchBtn.setOnCheckedChangeListener((compoundButton, b) -> {
            System.out.println("开关按钮的状态 = " + b);
            if(b){
                kaiChung();
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

    public void kaiChung(){
        RelativeLayout container = findViewById(R.id.container);
        NUM_VIDEOS = Integer.parseInt(numEdit.getText().toString());
        // 循环创建多个 VideoView
        for (int i = 0; i < NUM_VIDEOS; i++) {
            VideoView videoView = new VideoView(this);
            int videoWidth = 200; // 设置视频宽度
            int videoHeight = 150; // 设置视频高度

            // 设置 VideoView 的 ID，方便后续引用
            videoView.setId(View.generateViewId());

            // 设置 VideoView 的位置和大小
            RelativeLayout.LayoutParams params = new RelativeLayout.LayoutParams(videoWidth, videoHeight);
            params.leftMargin = i * 100; // 设置每个 VideoView 的水平间距
            videoView.setLayoutParams(params);

            // 设置 VideoView 的路径或 URL
            videoView.setVideoPath(uri);

            videoView.setOnTouchListener(TestActivity.this); // 为每个 VideoView 添加触摸事件监听器

            container.addView(videoView);

        }
    }

    @Override
    public void onClick(View v) {
        if(v.getId() == R.id.jian_img){
            if(NUM_VIDEOS > 1){
                NUM_VIDEOS --;
                numEdit.setText(NUM_VIDEOS + "");
            }else {
                Toast.makeText(this,"开窗数量不能少于0",Toast.LENGTH_SHORT).show();
            }
        }else if (v.getId() == R.id.jia_img){
            if(NUM_VIDEOS <= 100){
                NUM_VIDEOS ++;
                numEdit.setText(NUM_VIDEOS + "");
            }else {
                Toast.makeText(this,"开窗数量不能大于100",Toast.LENGTH_SHORT).show();
            }
        }
    }

    @Override
    public boolean onTouch(View v, MotionEvent event) {
        switch (event.getAction()) {
            case MotionEvent.ACTION_DOWN:
                // 当手指按下时，记录下初始位置，用于后续计算偏移量
                v.setTag(R.id.tag_video_view_origin_x, event.getRawX());
                v.setTag(R.id.tag_video_view_origin_y, event.getRawY());
                long clickTime = System.currentTimeMillis();
                // 计算两次点击的时间间隔
                long deltaTime = clickTime - lastClickTime;
                if (deltaTime < DOUBLE_CLICK_TIME_DELTA) {
                    // 在这里处理双击事件
                    handleDoubleClick();
                    int videoWidth = 200; // 设置视频宽度
                    int videoHeight = 150; // 设置视频高度
                    // 设置 VideoView 的位置和大小
                    RelativeLayout.LayoutParams params = new RelativeLayout.LayoutParams(videoWidth, videoHeight);
                    v.setLayoutParams(params);
                }
                lastClickTime = clickTime;
                break;
            case MotionEvent.ACTION_MOVE:
                // 当手指移动时，计算偏移量，并移动 VideoView
                float dX = event.getRawX() - (float) v.getTag(R.id.tag_video_view_origin_x);
                float dY = event.getRawY() - (float) v.getTag(R.id.tag_video_view_origin_y);
                RelativeLayout.LayoutParams layoutParams = (RelativeLayout.LayoutParams) v.getLayoutParams();
                layoutParams.leftMargin += dX;
                layoutParams.topMargin += dY;
                v.setLayoutParams(layoutParams);
                // 更新初始位置
                v.setTag(R.id.tag_video_view_origin_x, event.getRawX());
                v.setTag(R.id.tag_video_view_origin_y, event.getRawY());
                break;
        }
        return true;
    }

    private void handleDoubleClick() {
        // 在这里处理双击事件的逻辑
        // 例如，执行双击后的操作或显示提示
    }

}