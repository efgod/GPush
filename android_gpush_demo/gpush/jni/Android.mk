
LOCAL_PATH := $(call my-dir)
#jsoncpp######################################################################################
include $(CLEAR_VARS) 
 LOCAL_CPPFLAGS += -frtti -fexceptions
 LOCAL_CPPFLAGS += -DGOOGLE_PROTOBUF_NO_THREAD_SAFETY
LOCAL_MODULE    := libjsoncpp 
LOCAL_SRC_FILES := json/json_value.cpp json/json_reader.cpp json/json_writer.cpp 

include $(BUILD_STATIC_LIBRARY)


# google protobuf#############################################################################
include $(CLEAR_VARS) 
 LOCAL_CPPFLAGS += -frtti -fexceptions
 LOCAL_CPPFLAGS += -DGOOGLE_PROTOBUF_NO_THREAD_SAFETY
LOCAL_MODULE    := libprotobuflite
LOCAL_SRC_FILES := google/protobuf/io/zero_copy_stream_impl_lite.cc \
google/protobuf/io/zero_copy_stream.cc \
google/protobuf/stubs/common.cc \
google/protobuf/generated_message_util.cc \
google/protobuf/wire_format_lite.cc \
google/protobuf/repeated_field.cc \
google/protobuf/stubs/once.cc \
google/protobuf/message_lite.cc \
google/protobuf/io/coded_stream.cc \
google/protobuf/extension_set.cc \
google/protobuf/stubs/atomicops_internals_x86_gcc.cc

include $(BUILD_STATIC_LIBRARY)

#efnfw#########################################################################################
include $(CLEAR_VARS) 
 LOCAL_CPPFLAGS += -frtti -fexceptions
LOCAL_MODULE    := libefnfw
LOCAL_SRC_FILES := net/ef_sock.cpp \
base/ef_loop_buf.cpp \
base/ef_thread.cpp \
base/ef_utility.cpp \
base/ef_md5.cpp \
base/ef_hex.cpp \
base/ef_base64.cpp
include $(BUILD_STATIC_LIBRARY)


#client##########################################################################################
include $(CLEAR_VARS)
LOCAL_ARM_MODE := arm
LOCAL_CPPFLAGS += -frtti -fexceptions
LOCAL_CPPFLAGS += -fPIC

#LOCAL_CPPFLAGS += -DGOOGLE_PROTOBUF_NO_THREAD_SAFETY

#LOCAL_CPPFLAGS += -D_DEBUG
LOCAL_CPPFLAGS += -Wl,-Map=test.map -g 

LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE := clientsdk

LOCAL_MODULE_TAGS := optional

LOCAL_PROGUARD_ENABLED:= disabled

LOCAL_CPP_EXTENSION := .cc .cpp

LOCAL_CPP_INCLUDES += .

LOCAL_SHARED_LIBRARIES := liblog libcutils libandroid_runtime libnativehelper

LOCAL_SRC_FILES :=  proto/message.pb.cpp \
proto/pair.pb.cpp \
proto/session.pb.cpp \
proto/gpush.pb.cpp \
client_sdk/eventloop.cpp \
client_sdk/opbase.cpp \
client_sdk/ops.cpp \
client_sdk/client_conn.cpp \
client_sdk/client.cpp \
client_sdk/client_log.cpp \
android/androidclient.cpp \
android/client_jni.cpp 

LOCAL_STATIC_LIBRARIES := libjsoncpp libprotobuflite libefnfw

LOCAL_LDLIBS+= -L$(SYSROOT)/usr/lib -llog

include $(BUILD_SHARED_LIBRARY) 
