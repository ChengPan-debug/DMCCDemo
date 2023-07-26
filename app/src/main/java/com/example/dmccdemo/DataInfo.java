package com.example.dmccdemo;

/**
 * @author zed
 * @date 2017/12/12 下午2:08
 * @desc
 */

public class DataInfo {

	public volatile byte[] mDataBytes;
	public volatile long receivedDataTime;

	public synchronized byte[] getmDataBytes() {
		return mDataBytes;
	}

	public synchronized void setmDataBytes(byte[] mDataBytes) {
		this.mDataBytes = mDataBytes;
	}

	public synchronized long getReceivedDataTime() {
		return receivedDataTime;
	}

	public synchronized void setReceivedDataTime(long receivedDataTime) {
		this.receivedDataTime = receivedDataTime;
	}
}
