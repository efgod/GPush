package com.gim;

public class client 
{
	 public static int SDK_EVTYPE_LOGIN_OK = 1;
	 public static int SDK_EVTYPE_LOGIN_FAIL = 2;
	 public static int SDK_EVTYPE_LOGOUT = 3;
	 public static int SDK_EVTYPE_PUSH = 10000;
	 
	 
	 public native int  init(listener lstr);
	 public native int stop();
	 public native int login(String srvip, int srvport, String cliver, String cid);
	 public native int logout(String cid);
	 public native int loglevel(int lvl); // 0:nothing, 1:error, 2:all
	 static {
	        System.loadLibrary("clientsdk");
	    }
}