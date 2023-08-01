package com.example.dmccdemo;

import android.content.Context;
import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaFormat;
import android.os.Build;
import android.os.SystemClock;
import android.util.AttributeSet;
import android.util.Log;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.concurrent.locks.ReentrantLock;

/**
 * @author zed
 * @date 2017/11/22 上午10:38
 * @desc
 */

public class MediaCodecSurfaceViews extends SurfaceView {

    private final String TAG = MediaCodecSurfaceViews.class.getSimpleName();

    //设置解码分辨率
    private int mVideoWidth;
    private int mVideoHeight;

    //解码帧率 1s解码30帧
    private final int FRAME_RATE = 10;

    //支持格式
    private final String VIDEOFORMAT_H264 = "video/avc";

    //默认格式
    private String mMimeType = VIDEOFORMAT_H264;

    //接收的视频帧队列
    private ArrayList<DataInfo> mFrmList = new ArrayList<>();

    //解码支持监听器
    private OnSupportListener mSupportListener;
    //解码结果监听
    private OnDecodeListener mOnDecodeListener;

    private MediaCodec mMediaCodec;
    private DecodeThread mDecodeThread;
    private DrawingThread drawingThread;
    private Surface mSurface;

    private ReentrantLock lock = new ReentrantLock(true);

    public MediaCodecSurfaceViews(Context context) {
        super(context);
        try {
            getHolder().addCallback(mCallback);
        }catch (Exception e){
            Log.e(TAG, "MediaCodecSurfaceViews: ",e );
        }
    }


    public MediaCodecSurfaceViews(Context context, AttributeSet attrs) {
        super(context, attrs);
        getHolder().addCallback(mCallback);
    }

    public void setWidthAndHeight(int width,int height){
        this.mVideoWidth = width;
        this.mVideoHeight = height;
    }

    private SurfaceHolder.Callback mCallback = new SurfaceHolder.Callback() {
        @Override
        public void surfaceCreated(SurfaceHolder holder) {
            mSurface = holder.getSurface();
            init();
        }

        @Override
        public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        }

        @Override
        public void surfaceDestroyed(SurfaceHolder holder) {
            unInit();
        }
    };


    public void init() {
        if (mDecodeThread != null) {
            mDecodeThread.stopThread();
            try {
                mDecodeThread.join();
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
            mDecodeThread = null;
        }
        if (drawingThread != null) {
            drawingThread.stopThread();
            try {
                drawingThread.join();
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
            drawingThread = null;
        }

        if (mMediaCodec != null) {
            mMediaCodec.stop();
            mMediaCodec.release();
            mMediaCodec = null;
        }

        try {
            //通过多媒体格式名创建一个可用的解码器
            mMediaCodec = MediaCodec.createDecoderByType(mMimeType);
        } catch (IOException e) {
            e.printStackTrace();
        }

        //初始化解码器格式
        MediaFormat mediaformat = MediaFormat.createVideoFormat(mMimeType, mVideoWidth, mVideoHeight);
        mediaformat.setInteger(MediaFormat.KEY_BIT_RATE,20000000);
        mediaformat.setInteger(MediaFormat.KEY_BITRATE_MODE, MediaCodecInfo.EncoderCapabilities.BITRATE_MODE_CQ);
        mediaformat.setInteger(MediaFormat.KEY_I_FRAME_INTERVAL, 0);
        //设置帧率
        mediaformat.setInteger(MediaFormat.KEY_FRAME_RATE, FRAME_RATE);
        //crypto:数据加密 flags:编码器/编码器
        mMediaCodec.configure(mediaformat, mSurface, null, 0);
        mMediaCodec.start();
        mDecodeThread = new DecodeThread();
        mDecodeThread.start();
        drawingThread = new DrawingThread();
        drawingThread.start();
        mMediaCodec.setVideoScalingMode(MediaCodec.VIDEO_SCALING_MODE_SCALE_TO_FIT_WITH_CROPPING);
    }

    public void unInit() {
        if (mDecodeThread != null) {
            mDecodeThread.stopThread();
            try {
                mDecodeThread.join(200);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
            mDecodeThread = null;
        }
        if (drawingThread != null) {
            drawingThread.stopThread();
            try {
                drawingThread.join(200);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
            drawingThread = null;
        }
        try {
            if (mMediaCodec != null) {
                mMediaCodec.stop();
                mMediaCodec.release();
                mMediaCodec = null;
            }
        } catch (IllegalStateException e) {
            e.printStackTrace();
        }

        mFrmList.clear();
    }

    /**
     * @author zed
     * @description 解码线程
     * @time 2017/11/22
     */
    private class DecodeThread extends Thread {

        private boolean isRunning = true;

        public synchronized void stopThread() {
            isRunning = false;
        }

        public boolean isRunning() {
            return isRunning;
        }

        @Override
        public void run() {

            //存放目标文件的数据
            ByteBuffer byteBuffer = null;
            //解码后的数据，包含每一个buffer的元数据信息，例如偏差，在相关解码器中有效的数据大小
            MediaCodec.BufferInfo info = new MediaCodec.BufferInfo();
            long startMs = System.currentTimeMillis();
            DataInfo dataInfo;
            byte[] mDataBytes = new byte[1024];
            ArrayList<DataInfo> mList = mFrmList;
            while (isRunning) {
                if (mList.isEmpty()) {
                    try {
                        Thread.sleep(10);
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                    continue;
                }
                Log.d(TAG,"begin decode...");

                try {
                    lock.lock();
                    dataInfo = mList.remove(0);
                    if (dataInfo != null && dataInfo.getmDataBytes() != null) {
                        mDataBytes = dataInfo.getmDataBytes();
                    }
                } catch (Exception e) {
                    e.printStackTrace();
                } finally {
                    lock.unlock();
                }

                //1 准备填充器
                int inIndex = -1;
                try {
                    inIndex = mMediaCodec.dequeueInputBuffer(0);
                } catch (IllegalStateException e) {
                    e.printStackTrace();
                    if (mSupportListener != null) {
                        mSupportListener.UnSupport();
                    }
                }

                if (inIndex >= 0) {
                    //2 准备填充数据
                    Log.d(TAG, "inIndex:" + inIndex);
                    if (Build.VERSION.SDK_INT < Build.VERSION_CODES.LOLLIPOP) {
                        byteBuffer = mMediaCodec.getInputBuffers()[inIndex];
                        byteBuffer.clear();
                    } else {
                        byteBuffer = mMediaCodec.getInputBuffer(inIndex);
                    }

                    if (byteBuffer == null) {
                        continue;
                    }
                    byteBuffer.put(mDataBytes, 0, mDataBytes.length);
                    //3 把数据传给解码器
                    mMediaCodec.queueInputBuffer(inIndex, 0, mDataBytes.length, 0, 0);
                } else {
                    SystemClock.sleep(50);
                    continue;
                }
            }

        }

    }

    public void setData(byte[] data) {

        DataInfo dataInfo = new DataInfo();
        dataInfo.setmDataBytes(data);
        long time = System.currentTimeMillis();
        dataInfo.setReceivedDataTime(time);
        lock.lock();
        try {
            mFrmList.add(dataInfo);
        } catch (Exception e) {
            e.printStackTrace();
        } finally {
            lock.unlock();
        }
    }

    class DrawingThread extends Thread {
        long last = 0;

        private boolean isRunning = true;

        public synchronized void stopThread() {
            isRunning = false;
        }

        public boolean isRunning() {
            return isRunning;
        }

        @Override
        public void run() {

            while (isRunning) {

                long startMs = System.currentTimeMillis();
                //解码后的数据，包含每一个buffer的元数据信息，例如偏差，在相关解码器中有效的数据大小
                MediaCodec.BufferInfo info = new MediaCodec.BufferInfo();
                int outIndex = MediaCodec.INFO_TRY_AGAIN_LATER;
                //4 开始解码
                try {

                    outIndex = mMediaCodec.dequeueOutputBuffer(info, 0);

                } catch (IllegalStateException e) {
                    e.printStackTrace();
                }
               /* while (outIndex >= 0) {
                    mMediaCodec.releaseOutputBuffer(outIndex, true);
                    outIndex = mMediaCodec.dequeueOutputBuffer(info, 0);
                }*/
                if (outIndex >= 0) {
                    //帧控制
                    Log.d(TAG, "surface decoder given buffer " + outIndex + " (size=" + info.size + ")");
                    while (info.presentationTimeUs / 1000 > System.currentTimeMillis() - startMs) {
                        try {
                            Log.e("sleep---261:", "");
                            Thread.sleep(100);
                        } catch (Exception e) {
                            e.printStackTrace();
                        }
                    }
                    boolean doRender = (info.size != 0);
                    try {
                        //对outputbuffer的处理完后，调用这个函数把buffer重新返回给codec类。
                        //调用这个api之后，SurfaceView才有图像
                        mMediaCodec.releaseOutputBuffer(outIndex, true);
                        if (mOnDecodeListener != null) {
                            mOnDecodeListener.decodeResult(mVideoWidth, mVideoHeight);
                        }

                        long curr = System.currentTimeMillis();
                        Log.i(TAG, "DecodeThread delay = " + (curr-last));
                        last = curr;
                    } catch (Exception e) {
                        e.printStackTrace();
                    }

                } else if (outIndex == MediaCodec.INFO_TRY_AGAIN_LATER)
                {
                    // no output available yet
                        Log.d(TAG, "no output from decoder available");
                }
                else if (outIndex == MediaCodec.INFO_OUTPUT_BUFFERS_CHANGED)
                {
                    //decoderOutputBuffers = decoder.getOutputBuffers();
                        Log.d(TAG, "decoder output buffers changed (we don't care)");
                }
                else if (outIndex == MediaCodec.INFO_OUTPUT_FORMAT_CHANGED)
                {
                    // expected before first buffer of data
                    MediaFormat newFormat = mMediaCodec.getOutputFormat();
                        Log.d(TAG, "decoder output format changed: " + newFormat);
                }
                else if (outIndex < 0)
                {
                    throw new RuntimeException("unexpected result from decoder.dequeueOutputBuffer: " + outIndex);
                }
                System.gc();
            }

        }
    }
}