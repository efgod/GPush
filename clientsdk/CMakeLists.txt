cmake_minimum_required(VERSION 2.6)
project(clientsdk)
set(CMAKE_VERBOSE_MAKEFILE ON)
add_definitions("-Wall -g -fPIC -frtti -fexceptions -DGOOGLE_PROTOBUF_NO_THREAD_SAFETY")
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

include_directories(. efnfw include)

set(SRC_LIST google/protobuf/io/zero_copy_stream_impl_lite.cc 
google/protobuf/io/zero_copy_stream.cc 
google/protobuf/stubs/common.cc 
google/protobuf/generated_message_util.cc 
google/protobuf/wire_format_lite.cc 
google/protobuf/repeated_field.cc 
google/protobuf/stubs/once.cc 
google/protobuf/message_lite.cc 
google/protobuf/io/coded_stream.cc 
google/protobuf/extension_set.cc 
google/protobuf/stubs/atomicops_internals_x86_gcc.cc
json/json_value.cpp
json/json_reader.cpp
json/json_writer.cpp
efnfw/net/ef_sock.cpp
efnfw/base/ef_loop_buf.cpp 
efnfw/base/ef_thread.cpp 
efnfw/base/ef_utility.cpp 
efnfw/base/ef_md5.cpp 
efnfw/base/ef_hex.cpp 
efnfw/base/ef_base64.cpp
proto/message.pb.cpp 
proto/pair.pb.cpp 
proto/session.pb.cpp 
proto/gpush.pb.cpp 
client/eventloop.cpp 
client/opbase.cpp 
client/ops.cpp 
client/client_conn.cpp 
client/client.cpp 
client/client_log.cpp 
)

add_library(clientsdk SHARED ${SRC_LIST})
#file(GLOB_RECURSE EF_LIST *.cpp)
#add_library(cachegroup STATIC ${EF_LIST})
