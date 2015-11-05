package com.gim.msg;

import org.json.JSONException;
import org.json.JSONObject;

import com.gim.GMsg;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

public class GMsgBroadcastReceiver extends BroadcastReceiver  {
	private GMsgPublisher publisher;
	public GMsgBroadcastReceiver(Context context, GMsgPublisher pub)
	{
		GMsgBroadcaseter.register(context, this);
		publisher = pub;
	}
	
	
	@Override
	public void onReceive(final Context context, Intent intent) {
        try {
            String action = intent.getAction();
            String msg = intent.getStringExtra("msg");
            Log.d("fxxxios", "MsgReceiver OnReceive action:" + action + " msg:" + msg);
            
            try {
	        	JSONObject j = new JSONObject (msg);
	        	//String sn = j.getString("sn");
	        	int type = j.getInt("evtype");
	        	
	        	GMsg notify = new GMsg();
	        	notify.type = type;
	        	notify.data = j;
	        	publisher.publish(notify);
	        } catch (JSONException e){
	        	Log.e("fxxxios", e.toString());
	            e.printStackTrace();
	        }
        } catch (Exception ex) {
        	Log.e("fxxxios", ex.toString());
            ex.printStackTrace();
        }
    }
}
