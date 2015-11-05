package com.gim.msg;


import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;

public class GMsgBroadcaseter {
	public final static String TAG = "G_CLIENT_MSG";
	
	public static void broadcast(Context context, String msg){
		Intent intent = new Intent();
		intent.setAction(TAG);
		intent.putExtra("msg", msg);
		context.sendBroadcast(intent);
	}
	
	public static void register(Context context, BroadcastReceiver r){

	    IntentFilter filter = new IntentFilter();
	    filter.addAction(GMsgBroadcaseter.TAG);
	    filter.setPriority(Integer.MAX_VALUE);
	    context.registerReceiver(r, filter);
	}
}
