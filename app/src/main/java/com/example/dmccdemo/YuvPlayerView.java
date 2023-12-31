package com.example.dmccdemo;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;
import android.view.SurfaceHolder;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class YuvPlayerView extends GLSurfaceView implements Runnable, SurfaceHolder.Callback, GLSurfaceView.Renderer{
    private int mVideoWidth;
    private int mVideoHeight;

    public YuvPlayerView(Context context) {
        super(context);
        setRenderer(this);
    }

    public YuvPlayerView(Context context, AttributeSet attrs) {
        super(context, attrs);
        setRenderer(this);
    }

    public void setWidthAndHeight(int width,int height){
        this.mVideoWidth = width;
        this.mVideoHeight = height;
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        new Thread(this).start();
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {

    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int w, int h) {
        holder.setFixedSize(w,h);
    }


    @Override
    public void onSurfaceCreated(GL10 gl10, EGLConfig eglConfig) {

    }

    @Override
    public void onSurfaceChanged(GL10 gl10, int i, int i1) {

    }

    @Override
    public void onDrawFrame(GL10 gl10) {

    }

    @Override
    public void run() {
        TestActivity.PlayffmpegRTPYUV(getHolder().getSurface(), mVideoWidth, mVideoHeight);
        TestActivity.ReleaseFFMPEGRTP();
    }
}
