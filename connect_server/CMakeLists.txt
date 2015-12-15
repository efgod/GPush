project(CServer)
set(CMAKE_VERBOSE_MAKEFILE ON)
add_definitions("-Wall -pg -g")
include_directories(${PROJECT_SOURCE_DIR}/../common/include ${PROJECT_SOURCE_DIR}/../efnfw ${PROJECT_SOURCE_DIR}/../head)
LINK_DIRECTORIES(${PROJECT_SOURCE_DIR}/../common/lib
${PROJECT_SOURCE_DIR}/../efnfw/lib /usr/local/lib)
link_libraries(common efnfw hiredis zookeeper_mt protobuf jsoncpp pthread)
file(GLOB_RECURSE SRC_LIST src/*.cpp)
add_executable(bin/CServer ${SRC_LIST})
