package com.gim;

public class GMsg {
	public static final int GIM_EVTYPE_LOGIN_OK = 0;
	public static final int GIM_EVTYPE_LOGIN_FAIL = 1;
	public static final int GIM_EVTYPE_LOGOUT = 2;
	public static final int GIM_EVTYPE_PUSH = 10000;

	public static final int GIM_OK = 0;
	public static final int GIM_ERROR = -1;
	public static final int GIM_NETWORK_ERROR = -9999;
	public static final int GIM_PROBUF_FORMAT_ERROR = -9998;
	public static final int GIM_TOO_LONG_PACKET = -9997;
	public static final int GIM_JNI_ERROR = -9996;
	public static final int GIM_JSON_ERROR = -9995;
	public static final int GIM_UNDEFINED_CMD = -9994;
	public static final int GIM_NOT_LOGGED = -9993;
	public static final int GIM_TIMEOUT = -9992;

	public int type;
	public Object data;
}
