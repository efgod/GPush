
LOCAL_PATH := $(call my-dir)
#jsoncpp######################################################################################
include $(CLEAR_VARS)

CLIENTSDK_SRC_PATH := ../../clientsdk
CLIENTSDK_INCLUDE_PATH := ../../clientsdk
LOCAL_C_INCLUDES += $(LOCAL_PATH)/$(CLIENTSDK_INCLUDE_PATH)

 LOCAL_CPPFLAGS += -frtti -fexceptions
 LOCAL_CPPFLAGS += -DGOOGLE_PROTOBUF_NO_THREAD_SAFETY
LOCAL_MODULE    := libjsoncpp

CLIENTSDK_SRC_PATH := ../../clientsdk
CLIENTSDK_INCLUDE_PATH := ../../clientsdk

LOCAL_SRC_FILES := $(CLIENTSDK_SRC_PATH)/json/json_value.cpp $(CLIENTSDK_SRC_PATH)/json/json_reader.cpp $(CLIENTSDK_SRC_PATH)/json/json_writer.cpp 

include $(BUILD_STATIC_LIBRARY)


# google protobuf#############################################################################
include $(CLEAR_VARS)

CLIENTSDK_SRC_PATH := ../../clientsdk
CLIENTSDK_INCLUDE_PATH := ../../clientsdk
LOCAL_C_INCLUDES += $(LOCAL_PATH)/$(CLIENTSDK_INCLUDE_PATH)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/$(CLIENTSDK_INCLUDE_PATH)/include

 LOCAL_CPPFLAGS += -frtti -fexceptions
 LOCAL_CPPFLAGS += -DGOOGLE_PROTOBUF_NO_THREAD_SAFETY
LOCAL_MODULE    := libprotobuflite
LOCAL_SRC_FILES := $(CLIENTSDK_SRC_PATH)/google/protobuf/io/zero_copy_stream_impl_lite.cc \
$(CLIENTSDK_SRC_PATH)/google/protobuf/io/zero_copy_stream.cc \
$(CLIENTSDK_SRC_PATH)/google/protobuf/stubs/common.cc \
$(CLIENTSDK_SRC_PATH)/google/protobuf/generated_message_util.cc \
$(CLIENTSDK_SRC_PATH)/google/protobuf/wire_format_lite.cc \
$(CLIENTSDK_SRC_PATH)/google/protobuf/repeated_field.cc \
$(CLIENTSDK_SRC_PATH)/google/protobuf/stubs/once.cc \
$(CLIENTSDK_SRC_PATH)/google/protobuf/message_lite.cc \
$(CLIENTSDK_SRC_PATH)/google/protobuf/io/coded_stream.cc \
$(CLIENTSDK_SRC_PATH)/google/protobuf/extension_set.cc \
$(CLIENTSDK_SRC_PATH)/google/protobuf/stubs/atomicops_internals_x86_gcc.cc

include $(BUILD_STATIC_LIBRARY)

#efnfw#########################################################################################
include $(CLEAR_VARS) 

CLIENTSDK_SRC_PATH := ../../clientsdk
CLIENTSDK_INCLUDE_PATH := ../../clientsdk
LOCAL_C_INCLUDES += $(LOCAL_PATH)/$(CLIENTSDK_INCLUDE_PATH)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/$(CLIENTSDK_INCLUDE_PATH)/efnfw

 LOCAL_CPPFLAGS += -frtti -fexceptions
LOCAL_MODULE    := libefnfw
LOCAL_SRC_FILES := $(CLIENTSDK_SRC_PATH)/efnfw/net/ef_sock.cpp \
$(CLIENTSDK_SRC_PATH)/efnfw/base/ef_loop_buf.cpp \
$(CLIENTSDK_SRC_PATH)/efnfw/base/ef_thread.cpp \
$(CLIENTSDK_SRC_PATH)/efnfw/base/ef_utility.cpp \
$(CLIENTSDK_SRC_PATH)/efnfw/base/ef_md5.cpp \
$(CLIENTSDK_SRC_PATH)/efnfw/base/ef_hex.cpp \
$(CLIENTSDK_SRC_PATH)/efnfw/base/ef_base64.cpp
include $(BUILD_STATIC_LIBRARY)


#client##########################################################################################
include $(CLEAR_VARS)

CLIENTSDK_SRC_PATH := ../../clientsdk
CLIENTSDK_INCLUDE_PATH := ../../clientsdk
LOCAL_C_INCLUDES += $(LOCAL_PATH)/$(CLIENTSDK_INCLUDE_PATH)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/$(CLIENTSDK_INCLUDE_PATH)/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/$(CLIENTSDK_INCLUDE_PATH)/efnfw

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

LOCAL_SHARED_LIBRARIES := liblog libcutils libandroid_runtime libnativehelper

LOCAL_SRC_FILES :=  $(CLIENTSDK_SRC_PATH)/proto/message.pb.cpp \
$(CLIENTSDK_SRC_PATH)/proto/pair.pb.cpp \
$(CLIENTSDK_SRC_PATH)/proto/session.pb.cpp \
$(CLIENTSDK_SRC_PATH)/proto/gpush.pb.cpp \
$(CLIENTSDK_SRC_PATH)/client/eventloop.cpp \
$(CLIENTSDK_SRC_PATH)/client/opbase.cpp \
$(CLIENTSDK_SRC_PATH)/client/ops.cpp \
$(CLIENTSDK_SRC_PATH)/client/client_conn.cpp \
$(CLIENTSDK_SRC_PATH)/client/client.cpp \
$(CLIENTSDK_SRC_PATH)/client/client_log.cpp \
androidclient.cpp \
client_jni.cpp 

LOCAL_STATIC_LIBRARIES := libjsoncpp libprotobuflite libefnfw

LOCAL_LDLIBS+= -L$(SYSROOT)/usr/lib -llog

include $(BUILD_SHARED_LIBRARY) 
