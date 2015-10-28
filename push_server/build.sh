#!/bin/bash

#cd ../efnfw
#sh build.sh
#cd -

#cd ../common
#sh build.sh
#cd -

protoc  ../proto/pair.proto --cpp_out=./src -I=../proto
mv -f ./src/pair.pb.cc ./src/pair.pb.cpp
mv -f ./src/pair.pb.h ./include/pair.pb.h

protoc  ../proto/gpush.proto --cpp_out=./src -I=../proto
mv -f ./src/gpush.pb.cc ./src/gpush.pb.cpp
mv -f ./src/gpush.pb.h ./include/gpush.pb.h

rm -fr CMakeCache.txt
rm -fr CMakeFiles
rm -fr cmake_install.cmake

mkdir -p bin 

cmake .
make clean;make
rm -fr CMakeCache.txt
rm -fr CMakeFiles
rm -fr cmake_install.cmake
