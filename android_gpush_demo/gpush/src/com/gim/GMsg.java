package com.gim;

public class GMsg {

	 static public int GIM_EVTYPE_LOGIN_OK = 0;
	 static public int GIM_EVTYPE_LOGIN_FAIL = 1;
	 static public int GIM_EVTYPE_LOGOUT = 2;
	 static public int GIM_EVTYPE_PUSH = 10000;
	 
	 static public int	GIM_OK = 0;
	 static public int	GIM_ERROR = -1;
	 static public int	GIM_NETWORK_ERROR = -9999;
	 static public int	GIM_PROBUF_FORMAT_ERROR = -9998;
	 static public int	GIM_TOO_LONG_PACKET = -9997;
	 static public int	GIM_JNI_ERROR = -9996;
	 static public int	GIM_JSON_ERROR = -9995;
	 static public int	GIM_UNDEFINED_CMD = -9994;
	 static public int	GIM_NOT_LOGGED = -9993;
	 static public int	GIM_TIMEOUT	= -9992;
		
	 public int type;
	 public Object data;
}
