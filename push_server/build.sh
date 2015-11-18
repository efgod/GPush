#!/bin/bash

protoc  ../proto/pair.proto --cpp_out=./src -I=../proto
mv -f ./src/pair.pb.cc ./src/pair.pb.cpp

protoc  ../proto/gpush.proto --cpp_out=./src -I=../proto
mv -f ./src/gpush.pb.cc ./src/gpush.pb.cpp

rm -fr CMakeCache.txt
rm -fr CMakeFiles
rm -fr cmake_install.cmake

mkdir -p bin 

cmake .
make clean;make
rm -fr CMakeCache.txt
rm -fr CMakeFiles
rm -fr cmake_install.cmake
