#/bin/bash

base_dir=`pwd`
#1. install protobuf

dst_bin_dir="${HOME}/usr/bin"
dst_lib_dir="${HOME}/usr/lib"
dst_inc_dir="${HOME}/usr/include"

profile="${HOME}/.bash_profile"
echo "LD_LIBRARY_PATH=\$LD_LIBRARY_PATH:${dst_lib_dir}" >> ${profile}
echo "LIBRARY_PATH=\$LIBRARY_PATH:${dst_lib_dir}" >> ${profile}

echo "C_INCLUDE_PATH=${dst_inc_dir}" >> ${profile}
echo "CPLUS_INCLUDE_PATH=${dst_inc_dir}" >> ${profile}

echo "PATH=\$PATH:${dst_bin_dir}" >> ${profile}
echo "export LD_LIBRARY_PATH LIBRARY_PATH C_INCLUDE_PATH CPLUS_INCLUDE_PATH PATH" >> ${profile}

export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${dst_lib_dir}
export LIBRARY_PATH=${LIBRARY_PATH}:${dst_lib_dir}
export C_INCLUDE_PATH=${dst_inc_dir}
export CPLUS_INCLUDE_PATH=${dst_inc_dir} 
export PATH=${PATH}:${dst_bin_dir}

source ${profile}
echo "PATH : ${PATH}"

pb_dir="${base_dir}/protobuf-2.6.1"
echo "Compiling protobuf2.6.1 and installing....."
tar xzvf protobuf-2.6.1.tar.gz > /dev/null 
cd ${pb_dir} && ./configure --prefix=${HOME}/usr/ > protobuf.log 2>&1
cd ${pb_dir} && make  >> protobuf.log 2>&1 
cd ${pb_dir} && make install  >> protobuf.log 2>&1

file=${dst_lib_dir}/libprotobuf.a
#check
if [ ! -f "$file" ]; then
  	echo "Compile protobuf2.6.1. pls check protobuf.log"
	exit 0
fi

echo "Compile and install protobuf succeed. install dir on ${dst_lib_dir}"

#2. install redis
echo "Compiling redis......"

redis_dir="${base_dir}/redis-2.8.23"
cd ${base_dir} && tar xzvf redis-2.8.23.tar.gz > /dev/null
cd ${redis_dir} && make > redis.log 2>&1

file=${redis_dir}/src/redis-server
if [ ! -f "$file" ]; then
  	echo "Compile redis2.8.3. pls check redis.log"
	exit 0
fi

hir_inc_dir="${dst_inc_dir}/hiredis"
if [ ! -d "${hir_inc_dir}" ]; then
  cd ${dst_inc_dir} && mkdir -p hiredis
fi

cp ${redis_dir}/deps/hiredis/hiredis.h ${hir_inc_dir}/
cp ${redis_dir}/src/redis-*  ${dst_bin_dir}
cp ${redis_dir}/deps/hiredis/libhiredis.a ${dst_lib_dir}

echo "Compile and install redis successed. install dir on ${dst_lib_dir}"

#3. install zookeeper
zk_dir="${base_dir}/zookeeper-3.4.6"

echo "Compiling zk c client lib....."
cd ${base_dir} && tar xzvf zookeeper-3.4.6.tar.gz > /dev/null
cd ${zk_dir}/src/c && ./configure --prefix=${HOME}/usr/ > zk.log 2>&1
cd ${zk_dir}/src/c && make > zk.log 2>&1
cd ${zk_dir}/src/c && make install > zk.log 2>&1

#check
file="${dst_lib_dir}/libzookeeper_mt.a"
if [ ! -f "$file" ]; then
  	echo "Compile zookeep c client. pls check zk.log"
	exit 0
fi

echo "Compile and install zk successed."

#4. install scons
scons_dir="${base_dir}/scons-2.1.0"

echo "Install scons....."
cd ${base_dir} && tar xzvf scons-2.1.0.tar.gz > /dev/null
python ${scons_dir}/setup.py install --prefix=${scons_dir} > scons.log

echo "Install scons successed."

#check
file="${dst_lib_dir}/libzookeeper_mt.a"
if [ ! -f "$file" ]; then
  	echo "Compile zookeep c client. pls check zk.log"
	exit 0
fi

echo "Compile and install zk succed."
#5. install jsoncpp
jsoncpp_dir="${base_dir}/jsoncpp-src-0.5.0"

echo "Compiling jsoncpp lib....."
cd ${base_dir} && tar xzvf jsoncpp-src-0.5.0.tar.gz > /dev/null
cd $jsoncpp_dir && ${scons_dir}/bin/scons platform=linux-gcc > jsoncpp.log
cp -r ${jsoncpp_dir}/include/json ${hir_inc_dir}/
cp ${jsoncpp_dir}/libs/linux-gcc*/libjson_linux-gcc*libmt.a ${dst_lib_dir}/libjsoncpp_mt.a

#check
file="${dst_lib_dir}/libjson_mt.a"
if [ ! -f "$file" ]; then
  	echo "Compile jsoncpp. pls check jsoncpp.log"
	exit 0
fi

echo "Compile and install jsoncpp successed."

echo "remove so."
rm -fr ${dst_lib_dir}/*.so*

