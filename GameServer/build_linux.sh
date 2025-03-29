#!/bin/bash
mkdir -p build_linux
cd build_linux
cmake ../GameServer || { echo "cmake failed"; exit 1; }
make -j4 || { echo "make failed"; exit 1; }
cp GameServer ../bin/
cp ../thirdlibs/linux/mysql-connector-c++/lib64/libmysqlcppconn.so.9.8.2.0 ../bin/