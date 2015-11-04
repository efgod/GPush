#include <jni.h>
#ifndef _CLIENT_JNI_H_
#define _CLIENT_JNI_H_
#ifdef __cplusplus
extern "C" {
#endif

	JNIEXPORT jint JNICALL Java_com_gim_client_init
		(JNIEnv *, jobject, jobject);

	JNIEXPORT jint JNICALL Java_com_gim_client_stop
		(JNIEnv *, jobject);

	JNIEXPORT jint JNICALL Java_com_gim_client_login
		(JNIEnv* env, jobject job, jstring jsrvip, jint srvport, jstring jcliver, jstring jcid);

	JNIEXPORT jint JNICALL Java_com_gim_client_logout
		(JNIEnv *, jobject, jstring cid);

	JNIEXPORT jint JNICALL Java_com_gim_client_loglevel(JNIEnv *, jobject, jint lvl);


#ifdef __cplusplus
}
#endif
#endif
