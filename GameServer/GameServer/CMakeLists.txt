cmake_minimum_required(VERSION 3.25)
project(RubyGameServer)

set(CMAKE_CXX_STANDARD 20)

message(STATUS "CMake version: ${CMAKE_VERSION}")
message(STATUS "CMake generator: ${CMAKE_GENERATOR}")

add_executable(GameServer)

target_sources(GameServer
    PRIVATE
    Buffer.cpp
    Buffer.hpp
    GameLoop.cpp
    GameLoop.hpp
    main.cpp
    MsgQueue.cpp 
    MsgQueue.hpp 
    NetworkMgr.cpp 
    NetworkMgr.hpp
    TcpSocket.cpp
    TcpSocket.hpp)

target_sources(GameServer
    PRIVATE
    DataReader/INIReader.cpp 
    DataReader/INIReader.hpp)

source_group("DataReader" FILES
    DataReader/INIReader.cpp
    DataReader/INIReader.hpp)

target_sources(GameServer
    PRIVATE
    DB/DBMgr.cpp 
    DB/DBMgr.hpp)

source_group("DB" FILES
    DB/DBMgr.cpp
    DB/DBMgr.hpp)

target_sources(GameServer
    PRIVATE
    Player/Player.cpp 
    Player/Player.hpp
    Player/PlayerMgr.cpp 
    Player/PlayerMgr.hpp)

source_group("Player" FILES
    Player/Player.cpp
    Player/Player.hpp
    Player/PlayerMgr.cpp)

target_sources(GameServer
    PRIVATE
    proto/msg.pb.cc  
    proto/msg.pb.h 
    proto/dbmsg.pb.cc  
    proto/dbmsg.pb.h )

source_group("proto" FILES
    proto/msg.pb.cc
    proto/msg.pb.h
    proto/dbmsg.pb.cc
    proto/dbmsg.pb.h)


get_target_property(TARGET_SOURCES GameServer SOURCES)
message(STATUS "Sources for GameServer: ${TARGET_SOURCES}")

set(PROTOBUF_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../thirdlibs/mac/protobuf)
set(MYSQL_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../thirdlibs/mac/mysql-connector-c++)

target_include_directories(GameServer
    PRIVATE
    ${PROTOBUF_ROOT}/include
    ${MYSQL_ROOT}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/proto
    ${CMAKE_CURRENT_SOURCE_DIR}/DataReader
    ${CMAKE_CURRENT_SOURCE_DIR}/DB
    ${CMAKE_CURRENT_SOURCE_DIR}/Player)

target_link_libraries(GameServer
    PRIVATE
    ${PROTOBUF_ROOT}/libs/libprotobuf.a
    ${MYSQL_ROOT}/lib64/libssl.dylib
    ${MYSQL_ROOT}/lib64/libcrypto.dylib
    ${MYSQL_ROOT}/lib64/libmysqlcppconn-static.a
)

target_link_libraries(GameServer
    PRIVATE
    resolv)

set_target_properties(GameServer PROPERTIES
    BUILD_RPATH "/usr/local/mysql-connector-c++/lib64"
)

# set_target_properties(GameServer PROPERTIES
#     RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../bin
# )