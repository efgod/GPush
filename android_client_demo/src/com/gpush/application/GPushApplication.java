package com.gpush.application;
import com.gim.msg.*;

import android.app.Application;

public class GPushApplication extends Application {
	@Override
	public void onCreate() {
		super.onCreate();
		GClientBox.instance().init(getApplicationContext());
	}
	
    @Override
    public void onTerminate() {
        super.onTerminate();
    }
}
