package com.gpush.application;
import com.gim.msg.*;

import android.app.Application;
import android.util.Log;

public class GPushApplication extends Application {
	private static final String TAG = "XNetworkInfo";
	@Override
	public void onCreate() {
		Log.d(TAG, "onCreate");
		super.onCreate();
		
		GClientBox.instance().init(getApplicationContext());
	}
	
    @Override
    public void onTerminate() {
    	Log.d(TAG, "onTerminate");
        super.onTerminate();
    }
}
