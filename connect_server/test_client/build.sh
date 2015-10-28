#!/bin/sh

cd `dirname $0`

cp -fr ../../proto src/

protoc src/proto/pair.proto -I=src/proto/ --cpp_out=src/proto
mv src/proto/pair.pb.cc src/proto/pair.pb.cpp

protoc src/proto/gpush.proto -I=src/proto/ --cpp_out=src/proto
mv src/proto/gpush.pb.cc src/proto/gpush.pb.cpp


echo "clean build files"
rm -fr CMakeCache.txt 
rm -fr CMakeFiles

mkdir -p bin

cmake .
make clean;make

rm -fr CMakeCache.txt
rm -fr CMakeFiles
