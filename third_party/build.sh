#/bin/bash

function check_bin()
{
        libname="$1";
        r=`ls /usr/bin|grep $libname`
        if [ -n "$r" ]; then
                return 1;
        fi

        r=`ls /usr/local/bin|grep $libname`
        if [ -n "$r" ]; then
                return 1;
        fi

        echo "bin $libname do not exit";
        return 0;
}

function check_lib()
{
	libname="$1";
	r=`ls /usr/lib|grep $libname`
	if [ -n "$r" ]; then
		return 1;
	fi

	r=`ls /usr/lib64|grep $libname`
	if [ -n "$r" ]; then
		return 1;
	fi

	r=`ls /usr/local/lib|grep $libname`
	if [ -n "$r" ]; then
		return 1;
	fi

	r=`ls /usr/local/lib64|grep $libname`
	if [ -n "$r" ]; then
		return 1;
	fi
	echo "lib $libname do not exit";
	return 0;
}

base_dir=`pwd`

dst_bin_dir="/usr/local/bin"
dst_lib_dir="/usr/local/lib"
dst_inc_dir="/usr/local/include"

echo "checking protobuf..."
check_lib protobuf
libret=$?
check_bin protoc
binret=$?
let checkret=$libret*$binret

if [ $checkret -eq 0 ]; then
	#1. install protobuf
	pb_dir="${base_dir}/protobuf-2.6.1"
	rm -fr $pb_dir
	echo "Compiling protobuf2.6.1 and installing....."
	tar xzvf protobuf-2.6.1.tar.gz > /dev/null 
	cd ${pb_dir} && ./configure --prefix=/usr/local > protobuf.log 2>&1
	cd ${pb_dir} && make clean >> protobuf.log 2>&1 
	cd ${pb_dir} && make >> protobuf.log 2>&1 
	cd ${pb_dir} && sudo make install  >> protobuf.log 2>&1

	file=${dst_lib_dir}/libprotobuf.a
	#check
	if [ ! -f "$file" ]; then
		echo "Compile protobuf2.6.1. pls check protobuf.log"
		exit 0
	fi

	echo "Compile and install protobuf succeed. install dir on ${dst_lib_dir}"
else
	echo "protobuf has been installed."
fi

echo "checking redis..."
check_lib hiredis
checkret=$?

if [ $checkret -eq 0 ]; then

	#2. install redis
	echo "Compiling redis......"

	redis_dir="${base_dir}/redis-2.8.23"
	rm -fr $redis_dir
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

	sudo cp ${redis_dir}/deps/hiredis/hiredis.h ${hir_inc_dir}/
	sudo cp ${redis_dir}/src/redis-*  ${dst_bin_dir}
	sudo cp ${redis_dir}/deps/hiredis/libhiredis.a ${dst_lib_dir}

	echo "Compile and install redis successed. install dir on ${dst_lib_dir}"
else
	echo "redis has been installed."

fi

echo "checking zookeeper..."
check_lib zookeeper 
checkret=$?

if [ $checkret -eq 0 ]; then
	#3. install zookeeper
	zk_dir="${base_dir}/zookeeper-3.4.6"

	rm -fr $zk_dir
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
else
	echo "zookeeper has been installed."
fi

echo "checking jsoncpp..."
check_lib jsoncpp 
checkret=$?

if [ $checkret -eq 0 ]; then
	#4. install scons
	scons_dir="${base_dir}/scons-2.1.0"
	rm -fr $scons_dir
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

	rm -fr $jsoncpp_dir
	echo "Compiling jsoncpp lib....."
	cd ${base_dir} && tar xzvf jsoncpp-src-0.5.0.tar.gz > /dev/null
	cd $jsoncpp_dir && ${scons_dir}/bin/scons platform=linux-gcc > jsoncpp.log
	cp -r ${jsoncpp_dir}/include/json ${hir_inc_dir}/
	cp ${jsoncpp_dir}/libs/linux-gcc*/libjson_linux-gcc*libmt.a ${dst_lib_dir}/libjsoncpp.a
	cp ${jsoncpp_dir}/libs/linux-gcc*/libjson_linux-gcc*libmt.so ${dst_lib_dir}/libjsoncpp.so

	#check
	file="${dst_lib_dir}/libjsoncpp_mt.a"
	if [ ! -f "$file" ]; then
		echo "Compile jsoncpp. pls check jsoncpp.log"
		exit 0
	fi

	echo "Compile and install jsoncpp successed."
else
	echo "jsoncpp has been installed."
fi
