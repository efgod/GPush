package com.gim.msg;

import java.util.LinkedList;
import java.util.List;

import com.gim.GMsg;

import android.util.Log;

public class GMsgPublisher {
	private List<Observer> mObs = new LinkedList<Observer>();
	public void addOb(Observer ob){
		Log.d("addob", ob.getClass().getName());
		if(!mObs.contains(ob)){
			mObs.add(ob);
		}
	}
	public void removeOb(Observer ob){
		mObs.remove(ob);
	}
	
    public void publish(GMsg notify) {
        for (int i = 0; i < mObs.size(); i++) {
        	Observer ob = mObs.get(i);
        	ob.onNotify(notify);
        }
    }
}
