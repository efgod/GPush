package com.gpush.activity;


import java.util.LinkedList;
import java.util.List;

import org.json.JSONException;
import org.json.JSONObject;

import com.gim.GMsg;
import com.gim.msg.GClientBox;
import com.gim.msg.Observer;
import com.gpush.config.Config;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;
import android.view.Gravity;
import android.widget.ArrayAdapter;
import android.widget.ListAdapter;
import android.widget.ListView;
import android.widget.Toast;


public class MainActivity extends Activity implements Observer{
	 private ListView listView;
	 private ArrayAdapter<String> adpter;
	 private List<String> list;
	    //private List<String> data = new ArrayList<String>();
	    @Override
	    public void onCreate(Bundle savedInstanceState){
	        super.onCreate(savedInstanceState);

	    	GClientBox.instance().getPublisher().addOb(this);
	        GClientBox.instance().getClient().login(Config.srvip, Config.srvport, Config.cliversion, Config.cid);
	        listView = new ListView(this);
	        list = new LinkedList<String>();
	        list.add("GPush Demo");
	        adpter = new  ArrayAdapter<String>(this, android.R.layout.simple_expandable_list_item_1, list);
	        listView.setAdapter(adpter);
	        setContentView(listView);
	    }
	    
	    @Override
	    protected void onDestroy (){
	    	super.onDestroy();
	    	GClientBox.instance().getClient().logout(Config.cid);
	    	GClientBox.instance().getPublisher().removeOb(this);
	    }
	    
	    public int onNotify(GMsg notify){
	    	if(notify.type == GMsg.GIM_EVTYPE_PUSH){
	            
	            try {
		        	JSONObject j = (JSONObject)notify.data;

		    		String m = "msg [sn:" + j.getString("sn") + "] [payload:" + j.getString("payload") + "]";
		    		if(list.size() > 6){
		    			list.clear();
		    			list.add("GPush Demo");
		    		}
		    		list.add(m);
		    		adpter.notifyDataSetChanged();
		        } catch (JSONException e){
		        	Log.e("fxxxios", e.toString());
		            e.printStackTrace();
		        }
	    	}
	    	else if(notify.type == GMsg.GIM_EVTYPE_LOGIN_OK){
	    		Toast toast = Toast.makeText(getApplicationContext(),  "login success", Toast.LENGTH_LONG);
	    		toast.setGravity(Gravity.CENTER, 0, 0);
	    		toast.show();
	    	}else if(notify.type == GMsg.GIM_EVTYPE_LOGIN_FAIL){
	    		Toast toast = Toast.makeText(getApplicationContext(),  "login fail", Toast.LENGTH_LONG);
	    		toast.setGravity(Gravity.CENTER, 0, 0);
	    		toast.show();
	    	}else if(notify.type == GMsg.GIM_EVTYPE_LOGIN_FAIL){
	    		Toast toast = Toast.makeText(getApplicationContext(),  "logout", Toast.LENGTH_LONG);
	    		toast.setGravity(Gravity.CENTER, 0, 0);
	    		toast.show();
	    	}
	    	return 0;
	    }
}
