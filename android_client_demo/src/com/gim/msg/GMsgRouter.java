package com.gim.msg;

import com.gim.listener;

import android.content.Context;

public class GMsgRouter implements listener{
	private Context mContext;
	public GMsgRouter(Context context){
		mContext = context;
	}
	
    public void handleMessage(String msg){
		 GMsgBroadcaseter.broadcast(mContext, msg);
   }
}
