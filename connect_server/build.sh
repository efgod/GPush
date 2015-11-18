#!/bin/bash

protoc  ../proto/gpush.proto --cpp_out=src/ -I=../proto 

mv src/gpush.pb.cc src/gpush.pb.cpp

mkdir -p bin		

rm -fr CMakeCache.txt
rm -fr CMakeFiles


cmake .
make clean;make

rm -fr CMakeCache.txt
rm -fr CMakeFiles
