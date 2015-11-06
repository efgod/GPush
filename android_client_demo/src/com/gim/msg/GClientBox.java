package com.gim.msg;

import com.gim.client;

import android.content.Context;
import android.util.Log;

public class GClientBox {
	private static GClientBox ins = null;

	public static GClientBox instance() {
		if (ins == null) {
			ins = new GClientBox();
		}
		return ins;
	}

	public static final String TAG = "GClientBox";
	private client mClient;
	private GMsgRouter mRouter;
	private GMsgPublisher mPublisher;
	private GMsgBroadcastReceiver mReceiver;

	private GClientBox() {
	}

	public client getClient() {
		return mClient;
	}

	public GMsgPublisher getPublisher() {
		return mPublisher;
	}

	public void init(Context context) {
		Log.d(TAG, "init");
		mRouter = new GMsgRouter(context);
		mPublisher = new GMsgPublisher();
		mReceiver = new GMsgBroadcastReceiver(context, mPublisher);

		mClient = new client();
		mClient.init(mRouter);
		mClient.loglevel(1);
	}
}
