cmake_minimum_required(VERSION 3.20)
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

if(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")  # macOS
    message(STATUS "Detected platform: macOS")
    set( platform "mac" )
    message(STATUS "Detected platform: ${platform}")
    set(MYSQL_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../thirdlibs/${platform}/mysql-connector-c++)

    target_link_libraries(GameServer
        PRIVATE
        ${MYSQL_ROOT}/lib64/libssl.dylib
        ${MYSQL_ROOT}/lib64/libcrypto.dylib
        ${MYSQL_ROOT}/lib64/libmysqlcppconn-static.a
    )
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    message(STATUS "Detected platform: Linux")
    set( platform "linux" )
    message(STATUS "Detected platform: ${platform}")

    set(MYSQL_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../thirdlibs/${platform}/mysql-connector-c++)
        
    target_link_libraries(GameServer
        PRIVATE
        ${MYSQL_ROOT}/lib64/libmysqlcppconn.so.9.8.2.0
    )



else()
    message(FATAL_ERROR "Unsupported platform: ${CMAKE_SYSTEM_NAME}")
endif()

set_target_properties(GameServer PROPERTIES
  INSTALL_RPATH "$ORIGIN/thirdlibs/lib"
  BUILD_WITH_INSTALL_RPATH TRUE
)


include(FetchContent)

set(FETCHCONTENT_BASE_DIR "${CMAKE_SOURCE_DIR}/../_deps/${CMAKE_SYSTEM_NAME}")


set(protobuf_BUILD_SHARED_LIBS OFF)
set(protobuf_INSTALL OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)  # 如果你需要支持 -fPIC
# Disable building protoc and tests
set(protobuf_BUILD_PROTOC_BINARIES OFF CACHE BOOL "Do not build protoc")
set(protobuf_BUILD_TESTS OFF CACHE BOOL "Do not build tests")
set(protobuf_INSTALL_EXAMPLES OFF CACHE BOOL "Do not install examples")


FetchContent_Declare(
  protobuf
  GIT_REPOSITORY https://github.com/protocolbuffers/protobuf.git
  GIT_TAG v3.20.1-rc1
  SOURCE_SUBDIR cmake
)

# 获取 protobuf 的源路径
FetchContent_GetProperties(protobuf)

# 添加 protobuf 的 src 目录到你的项目 include 路径，解决 port_def.inc 找不到的问题
target_include_directories(GameServer PRIVATE
  ${protobuf_SOURCE_DIR}/../src
)

target_link_libraries(GameServer
  PRIVATE
  protobuf::libprotobuf  # 使用 FetchContent 的现代 CMake 方式
)


FetchContent_MakeAvailable(protobuf)

target_include_directories(GameServer
    PRIVATE
    ${MYSQL_ROOT}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/proto
    ${CMAKE_CURRENT_SOURCE_DIR}/DataReader
    ${CMAKE_CURRENT_SOURCE_DIR}/DB
    ${CMAKE_CURRENT_SOURCE_DIR}/Player)


target_link_libraries(GameServer
    PRIVATE
    resolv)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    set_target_properties(GameServer PROPERTIES
    BUILD_WITH_INSTALL_RPATH TRUE
    INSTALL_RPATH "$ORIGIN")
endif()

# set_target_properties(GameServer PROPERTIES
#     BUILD_RPATH "/usr/local/mysql-connector-c++/lib64"
# )

# set_target_properties(GameServer PROPERTIES
#     RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../bin
# )