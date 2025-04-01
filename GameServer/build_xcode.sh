mkdir build_xcode
cd build_xcode
cmake  -G Xcode ../GameServer

cmake --build ./

cp ./GameServer ../bin