#!/bin/bash

# yum install git
# yum instal cmake
# yum group install "Development Tools"

base_dir=`pwd`


#1. first install third party lib
echo "install third party lib..."
cd third_party && sh ./build.sh > third_party.log 2>&1

source ${HOME}/.bash_profile

#2. compile efnfw and common lib
find ./ -name "*.sh"| xargs chmod u+x

echo "Compiling efnfw lib ......"
cd ${base_dir}/efnfw && sh ./build.sh > efnfw_compile.log 2>&1

file="${base_dir}/efnfw/lib/libefnfw.a"

if [ ! -f "$file" ]; then
  	echo "Compile efnfw lib faield. pls check efnfw_compile.log"
	exit 0
fi

echo "Compile libefnfw.a succeed!"

echo "Compiling common lib ......"
cd ${base_dir}/common && sh ./build.sh > common_compile.log 2>&1

file="${base_dir}/common/lib/libcommon.a"

if [ ! -f "$file" ]; then
  	echo "Compile common lib faield. pls check common_compile.log"
	exit 0
fi


echo "Compile libcommon.a succeed."

#3. install efnfw and common lib

#4. compile connect server
cd ${base_dir}/connect_server && sh ./build.sh > connect_server.log 2>&1
file="${base_dir}/connect_server/bin/CServer"

if [ ! -f "$file" ]; then
  	echo "Compile connect_server faield. pls check connect_server.log"
	exit 0
fi

echo "Compile connect_server succeed."

#5. compile push_server
cd ${base_dir}/push_server && sh ./build.sh > push_server.log 2>&1
file="${base_dir}/push_server/bin/PushServer"

if [ ! -f "$file" ]; then
  	echo "Compile push_server faield. pls check push_server.log"
	exit 0
fi

echo "Compile push server succeed."
#6. test client 
