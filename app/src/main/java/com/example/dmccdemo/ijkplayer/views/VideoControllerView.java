package com.example.dmccdemo.ijkplayer.views;

import android.content.Context;
import android.os.Handler;
import android.os.Message;
import android.os.SystemClock;
import android.util.AttributeSet;
import android.util.Log;
import android.view.Gravity;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.widget.FrameLayout;
import android.widget.ImageButton;
import android.widget.LinearLayout;
import android.widget.MediaController;
import android.widget.SeekBar;
import android.widget.SeekBar.OnSeekBarChangeListener;
import android.widget.TextView;


import com.example.dmccdemo.R;

import java.lang.ref.WeakReference;
import java.util.Formatter;
import java.util.Locale;

import tv.danmaku.ijk.media.widget.media.IMediaController;
import tv.danmaku.ijk.media.widget.media.IjkVideoView;

/**
 * 播放控制器
 *
 * Created by jiaozebo on 2017/6/11.
 */
public class VideoControllerView extends FrameLayout implements IMediaController {
    private static final String TAG = "VideoControllerView";

    private MediaController.MediaPlayerControl mPlayer;

    private boolean mShowing;
    private boolean mDragging;

    private static final int sDefaultTimeout = 10000;
    private static final int FADE_OUT = 1;
    private static final int SHOW_PROGRESS = 2;

    private boolean mUseFastForward;
    private boolean mFromXml;

    StringBuilder mFormatBuilder;
    Formatter mFormatter;

    private Context mContext;
    private View mAnchor;
    private View mRoot;
    private View mediaControllerLL;
    private SeekBar mProgress;              // 播放进度滚动条
    private TextView mCurrentTime;          // 当前播放时间点
    private TextView mEndTime;              // 总时长

    private ImageButton mPauseButton;       // 暂停or开始
    private ImageButton mFastButton;        // 快进
    private ImageButton mRewindButton;      // 快退
    private ImageButton mFullscreenButton;  // 全屏
    private ImageButton mRecordButton;      // 录像
    private ImageButton mFastPlay;          // 播放速度加快
    private ImageButton mSlowPlay;          // 播放速度减慢

    private TextView mTVSpeed;
    private TextView mTVRecordDuration;
    private TextView fps, kbps;
    private View mPictureBtn, mChangeModeBtn;

    private long mReceivedBytes;
    private long mReceivedPackets;
    private long lastBitsMillis;
    private long recordBeginTime;

    private Handler mHandler = new MessageHandler(this);

    Runnable mRecordTickTask = new Runnable() {
        @Override
        public void run() {
            long recordSecond = (System.currentTimeMillis() - recordBeginTime) / 1000;

            if (recordSecond >= 300) {  // 分段

            }

            recordSecond %= 3600;
            mTVRecordDuration.setText(String.format("%02d:%02d", recordSecond / 60, recordSecond % 60));
//            mTVRecordDuration.setCompoundDrawablesWithIntrinsicBounds(recordSecond % 2 == 0 ? R.drawable.red_dot : R.drawable.transparent_dot, 0, 0, 0);
            postDelayed(this, 1000);
        }
    };

    // 每一秒更新fps/bps
    Runnable fpsBpsTickTask = new Runnable() {
        long firstTimeStamp = 0l;

        @Override
        public void run() {
            if (firstTimeStamp == 0l)
                firstTimeStamp = System.currentTimeMillis();

            if (mPlayer != null && (mPlayer instanceof IjkVideoView)) {
                IjkVideoView ijk = (IjkVideoView) mPlayer;
                long l = ijk.getReceivedBytes();
                long received = l - mReceivedBytes;

                long packets = ijk.getVideoCachePackets();
                long receivedPackets = packets - mReceivedPackets;
                mReceivedBytes = l;
                mReceivedPackets = packets;

                if (ijk.isPlaying() && lastBitsMillis != 0) {
                    long l1 = SystemClock.uptimeMillis() - lastBitsMillis;

                    if (l1 >= 300) {
                        long time = System.currentTimeMillis() - firstTimeStamp;

                        if (time < 900) {
                            fps.setText("");
                            kbps.setText("");
                        } else {
                            receivedPackets = Math.min(receivedPackets, 30);
                            fps.setText(String.format("%dfps", receivedPackets));
                            kbps.setText(String.format("%3.01fKB/s", received * 1.0f * 1000 / l1 / 1024));
                        }
                    }
                } else {
                    fps.setText("");
                    kbps.setText("");
                }

                lastBitsMillis = SystemClock.uptimeMillis();
            }
            postDelayed(this, 1000);
        }
    };

    private Runnable mSeekingPending;

    // 暂停/开始的点击事件
    private OnClickListener mPauseListener = new OnClickListener() {
        public void onClick(View v) {
            doPauseResume();
            show(sDefaultTimeout);
        }
    };

    // 全屏的点击事件
    private OnClickListener mFullscreenListener = new OnClickListener() {
        public void onClick(View v) {
            show(sDefaultTimeout);
        }
    };

    // 录像的点击事件
    private OnClickListener mRecordingListener = new OnClickListener() {
        public void onClick(View v) {
            show(sDefaultTimeout);
        }
    };

    // 滚动条的点击事件
    private OnSeekBarChangeListener mSeekListener = new OnSeekBarChangeListener() {
        public void onStartTrackingTouch(SeekBar bar) {
            show(3600000);

            mDragging = true;
            mHandler.removeMessages(SHOW_PROGRESS);
        }

        public void onProgressChanged(SeekBar bar, int progress, boolean fromuser) {
            if (mPlayer == null) {
                return;
            }

            if (!fromuser) {
                return;
            }

            if (mSeekingPending != null) {
                removeCallbacks(mSeekingPending);
                mSeekingPending = null;
            }

            if (mPlayer.getDuration() <= 0)
                return;
//            long duration = mPlayer.getDuration();
//            long newPosition = (duration * progress) / 1000L;

            mPlayer.seekTo((int) progress);

            if (mCurrentTime != null)
                mCurrentTime.setText(stringForTime((int) progress));
        }

        public void onStopTrackingTouch(SeekBar bar) {
            mDragging = false;
            setProgress();
            show(sDefaultTimeout);

            mHandler.sendEmptyMessage(SHOW_PROGRESS);
        }
    };

    // 快退的点击事件
    private OnClickListener mRewindListener = new OnClickListener() {
        public void onClick(View v) {
            if (mPlayer == null) {
                return;
            }

            int pos = mPlayer.getCurrentPosition();
            pos -= 5000; // милисекунд
            mPlayer.seekTo(pos);
            setProgress();

            show(sDefaultTimeout);
        }
    };

    // 快进的点击事件
    private OnClickListener mFastListener = new OnClickListener() {
        public void onClick(View v) {
            if (mPlayer == null) {
                return;
            }

            int pos = mPlayer.getCurrentPosition();
            pos += 15000; // милисекунд
            mPlayer.seekTo(pos);
            setProgress();

            show(sDefaultTimeout);
        }
    };

    // 播放速度加快
    private OnClickListener mFastPlayListener = new OnClickListener() {
        @Override
        public void onClick(View v) {
            show(sDefaultTimeout);

            if (mPlayer instanceof FullScreenAbleMediaPlayerControl) {
                FullScreenAbleMediaPlayerControl player = (FullScreenAbleMediaPlayerControl) mPlayer;
                float speed = player.getSpeed();

                if (speed > 2.0) {
                    return;
                }

                if (speed >= 1.0f) {
                    mTVSpeed.setText(String.format("%d倍速", (int) (speed * 2)));
                } else {
                    mTVSpeed.setText(String.format("%.02f倍速", speed * 2));
                }

                if (speed == 0.5) {
                    mTVSpeed.setVisibility(GONE);
                } else {
                    mTVSpeed.setVisibility(VISIBLE);
                }

                player.setSpeed(speed * 2);
            } else {

            }
        }
    };

    private OnClickListener mSlowPlayListener = new OnClickListener() {
        @Override
        public void onClick(View v) {
            if (mPlayer instanceof FullScreenAbleMediaPlayerControl) {
                FullScreenAbleMediaPlayerControl player = (FullScreenAbleMediaPlayerControl) mPlayer;
                float speed = player.getSpeed();

                if (speed < 0.5) {
                    return;
                }

                if (speed >= 2.0f) {
                    mTVSpeed.setText(String.format("%d倍速", (int) (speed * 0.5)));
                } else {
                    mTVSpeed.setText(String.format("%.02f倍速", speed * 0.5));
                }

                if (speed == 2.0) {
                    mTVSpeed.setVisibility(GONE);
                } else {
                    mTVSpeed.setVisibility(VISIBLE);
                }

                player.setSpeed(speed * 0.5f);
            }

            show(sDefaultTimeout);
        }
    };

    private OnClickListener takePicListener = new OnClickListener() {
        @Override
        public void onClick(View v) {
            show(sDefaultTimeout);
            if (mPlayer instanceof FullScreenAbleMediaPlayerControl) {
                FullScreenAbleMediaPlayerControl player = (FullScreenAbleMediaPlayerControl) mPlayer;
                player.takePicture();
            }
        }
    };

    private OnClickListener modeListener = new OnClickListener() {
        @Override
        public void onClick(View v) {
            show(sDefaultTimeout);
            if (mPlayer instanceof FullScreenAbleMediaPlayerControl) {
                FullScreenAbleMediaPlayerControl player = (FullScreenAbleMediaPlayerControl) mPlayer;
                player.toggleMode();
            }
        }
    };

    /** ==================== constructor ==================== */

    public VideoControllerView(Context context, AttributeSet attrs) {
        super(context, attrs);

        mRoot = null;
        mContext = context;
        mUseFastForward = true;
        mFromXml = true;

        Log.i(TAG, TAG);
    }

    public VideoControllerView(Context context, boolean useFastForward) {
        super(context);
        mContext = context;
        mUseFastForward = useFastForward;

        Log.i(TAG, TAG);
    }

    public VideoControllerView(Context context) {
        this(context, true);

        Log.i(TAG, TAG);
    }

    /** ==================== system Override ==================== */
    /*
    * 发生在视图实例化的过程中，一般在activity的oncreate方法中，并且只有在布局文件中实例化才有会这个回调
    * */
    @Override
    public void onFinishInflate() {
        super.onFinishInflate();

    }

    /*
    * 当发生轨迹球事件时触发该方法(貌似轨迹球是过去手机的按键)
    * */
    @Override
    public boolean onTrackballEvent(MotionEvent ev) {
        show(sDefaultTimeout);
        return false;
    }

    /*
    * 当发生触摸屏事件时触发该方法
    * */
    @Override
    public boolean onTouchEvent(MotionEvent event) {
        show(sDefaultTimeout);
        return true;
    }

    @Override
    public boolean dispatchKeyEvent(KeyEvent event) {
        if (mPlayer == null) {
            return true;
        }

        int keyCode = event.getKeyCode();
        final boolean uniqueDown = event.getRepeatCount() == 0 && event.getAction() == KeyEvent.ACTION_DOWN;

        if (keyCode == KeyEvent.KEYCODE_HEADSETHOOK ||
                keyCode == KeyEvent.KEYCODE_MEDIA_PLAY_PAUSE ||
                keyCode == KeyEvent.KEYCODE_SPACE) {

            if (uniqueDown) {
                doPauseResume();
                show(sDefaultTimeout);

                if (mPauseButton != null) {
                    mPauseButton.requestFocus();
                }
            }

            return true;
        } else if (keyCode == KeyEvent.KEYCODE_MEDIA_PLAY) {
            if (uniqueDown && !mPlayer.isPlaying()) {
                mPlayer.start();
                show(sDefaultTimeout);
            }
            return true;
        } else if (keyCode == KeyEvent.KEYCODE_MEDIA_STOP
                || keyCode == KeyEvent.KEYCODE_MEDIA_PAUSE) {
            if (uniqueDown && mPlayer.isPlaying()) {
                mPlayer.pause();
                show(sDefaultTimeout);
            }
            return true;
        } else if (keyCode == KeyEvent.KEYCODE_VOLUME_DOWN ||
                keyCode == KeyEvent.KEYCODE_VOLUME_UP ||
                keyCode == KeyEvent.KEYCODE_VOLUME_MUTE) {
            return super.dispatchKeyEvent(event);
        } else if (keyCode == KeyEvent.KEYCODE_BACK || keyCode == KeyEvent.KEYCODE_MENU) {
            if (uniqueDown) {
                hide();
            }

            return true;
        }

        show(sDefaultTimeout);
        return super.dispatchKeyEvent(event);
    }

    /** ==================== IMediaController ==================== */

    @Override
    public void hide() {
        if (mAnchor == null) {
            return;
        }

        try {
            if (mAnchor instanceof ViewGroup) {
                ViewGroup vg = (ViewGroup) mAnchor;
                vg.removeView(this);
            }

            mHandler.removeMessages(SHOW_PROGRESS);
        } catch (IllegalArgumentException ex) {
            Log.w("MediaController", "already removed");
        }

        mShowing = false;
    }

    public boolean isShowing() {
        return mShowing;
    }

    @Override
    public void setAnchorView(View view) {
        mAnchor = view;

        LayoutParams frameParams = new LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.MATCH_PARENT);

        removeAllViews();

    }

    @Override
    public void setEnabled(boolean enabled) {
        if (mPauseButton != null) {
            mPauseButton.setEnabled(enabled);
        }

        if (mFastButton != null) {
            mFastButton.setEnabled(enabled);
        }

        if (mRewindButton != null) {
            mRewindButton.setEnabled(enabled);
        }

        if (mProgress != null) {
            mProgress.setEnabled(enabled);
        }


        super.setEnabled(enabled);
    }

    @Override
    public void setMediaPlayer(MediaController.MediaPlayerControl player) {
        mPlayer = player;
        updateSpeedCtrl();
    }

    @Override
    public void show() {
        show(sDefaultTimeout);
    }

    @Override
    public void show(int timeout) {
        if (!mShowing && mAnchor != null) {
            setProgress();

            if (mPauseButton != null) {
                mPauseButton.requestFocus();
            }


            LayoutParams tlp = new LayoutParams(
                    ViewGroup.LayoutParams.MATCH_PARENT,
                    ViewGroup.LayoutParams.WRAP_CONTENT,
                    Gravity.BOTTOM);

            if (mAnchor instanceof ViewGroup) {
                ViewGroup vg = (ViewGroup) mAnchor;
                vg.addView(this, tlp);
            }

            mShowing = true;
        }

        updateSpeedCtrl();

        mHandler.sendEmptyMessage(SHOW_PROGRESS);

        Message msg = mHandler.obtainMessage(FADE_OUT);

        if (timeout != 0) {
            mHandler.removeMessages(FADE_OUT);
            mHandler.sendMessageDelayed(msg, timeout);
        }

        if (mPlayer != null && mPlayer.isPlaying()) {
            removeCallbacks(fpsBpsTickTask);
            post(fpsBpsTickTask);
        }
    }

    @Override
    public void showOnce(View view) {

    }

    /** ==================== UI操作 ==================== */

    /**
     * 生成播放控制的布局
     */


    private void updateSpeedCtrl() {
        if (mRoot == null || mRecordButton == null || this.mPlayer == null) {
            return;
        }

        if (this.mPlayer instanceof FullScreenAbleMediaPlayerControl) {
            FullScreenAbleMediaPlayerControl mPlayer = (FullScreenAbleMediaPlayerControl) this.mPlayer;

            if (mPlayer.speedCtrlEnable()) {
                mFastPlay.setVisibility(VISIBLE);
                mSlowPlay.setVisibility(VISIBLE);
                mTVSpeed.setVisibility(VISIBLE);

                kbps.setVisibility(GONE);
                fps.setVisibility(GONE);
            } else {
                mFastPlay.setVisibility(GONE);
                mSlowPlay.setVisibility(GONE);
                mTVSpeed.setVisibility(GONE);

                kbps.setVisibility(VISIBLE);
                fps.setVisibility(VISIBLE);
            }
        }
    }

    /*
     * 暂停/开始播放
     * */
    private void doPauseResume() {
        if (mPlayer == null) {
            return;
        }

        removeCallbacks(fpsBpsTickTask);

        if (mPlayer.isPlaying()) {
            mPlayer.pause();
        } else {
            boolean isCompleted = false;

            if (this.mPlayer instanceof FullScreenAbleMediaPlayerControl) {
                FullScreenAbleMediaPlayerControl mPlayer = (FullScreenAbleMediaPlayerControl) this.mPlayer;
                isCompleted = mPlayer.isCompleted();
            }

            mPlayer.start();

            if (isCompleted) {
                int duration = mPlayer.getDuration();
                final int progress = mProgress.getProgress();
                Log.d(TAG,String.valueOf(duration));

                mSeekingPending = new Runnable() {
                    @Override
                    public void run() {
                        if (mPlayer != null) {
                            mPlayer.seekTo(progress);
                        }
                    }
                };

                postDelayed(mSeekingPending,500);
            }

            post(fpsBpsTickTask);
            mReceivedBytes = 0;
            mReceivedPackets = 0;
        }

    }


    private int setProgress() {
        if (mPlayer == null || mDragging) {
            return 0;
        }

        int position = (int) (mPlayer.getCurrentPosition());

        // 非文件流的duration为0.
        int duration = mPlayer.getDuration();

        if (mProgress != null) {
            if (duration > 0) {
                int max = mProgress.getMax();

                if (max != duration) {
                    mProgress.setMax(duration);
                    mProgress.setProgress(position);
                } else {
                    if (position > mProgress.getProgress()){
                        mProgress.setProgress(position);
                    }
                }
            } else {
                mProgress.setMax(0);
                mProgress.setProgress(0);
            }

            int percent = mPlayer.getBufferPercentage();
            mProgress.setSecondaryProgress(percent * 10);
        }

        if (mEndTime != null)
            mEndTime.setText(stringForTime(duration));

        if (mCurrentTime != null)
            mCurrentTime.setText(stringForTime(position));

        return position;
    }



    private String stringForTime(int timeMs) {
        int totalSeconds = timeMs / 1000;
        int seconds = totalSeconds % 60;
        int minutes = (totalSeconds / 60) % 60;
        int hours = totalSeconds / 3600;

        mFormatBuilder.setLength(0);

        if (hours > 0) {
            return mFormatter.format("%d:%02d:%02d", hours, minutes, seconds).toString();
        } else {
            return mFormatter.format("%02d:%02d", minutes, seconds).toString();
        }
    }

    public interface FullScreenAbleMediaPlayerControl extends MediaController.MediaPlayerControl {
        boolean isFullScreen();

        void toggleFullScreen();

        boolean recordEnable();

        boolean speedCtrlEnable();

        boolean isRecording();

        void toggleRecord();

        float getSpeed();

        void setSpeed(float speed);

        void takePicture();

        void toggleMode();

        boolean isCompleted();
    }

    private static class MessageHandler extends Handler {
        private final WeakReference<VideoControllerView> mView;

        MessageHandler(VideoControllerView view) {
            mView = new WeakReference<VideoControllerView>(view);
        }

        @Override
        public void handleMessage(Message msg) {
            VideoControllerView view = mView.get();

            if (view == null || view.mPlayer == null) {
                return;
            }

            switch (msg.what) {
                case FADE_OUT:
                    view.hide();
                    break;
                case SHOW_PROGRESS:
                    if (!view.mDragging && view.mShowing && view.mPlayer.isPlaying()) {
                        msg = obtainMessage(SHOW_PROGRESS);
                        sendMessageDelayed(msg, 1000 - (view.setProgress() % 1000));
                    }
                    break;
            }
        }
    }
}

