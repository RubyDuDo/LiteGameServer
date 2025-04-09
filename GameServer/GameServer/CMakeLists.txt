cmake_minimum_required(VERSION 3.20)
project(RubyGameServer)

set(CMAKE_CXX_STANDARD 20)

message(STATUS "CMake version: ${CMAKE_VERSION}")
message(STATUS "CMake generator: ${CMAKE_GENERATOR}")

message(STATUS " platform: ${CMAKE_SYSTEM_NAME}-${CMAKE_CXX_COMPILER_ID}-${CMAKE_GENERATOR}")


add_executable(GameServer)




target_sources(GameServer
    PRIVATE
    GameLoop.cpp
    GameLoop.hpp
    main.cpp
    GameNetHelper.cpp 
    GameNetHelper.hpp
    DBQueryHandler.cpp 
    DBQueryHandler.hpp
    EventLogs.cpp 
    EventLogs.hpp
)
source_group("Game" FILES
    GameLoop.cpp
    GameLoop.hpp
    main.cpp
    GameNetHelper.cpp 
    GameNetHelper.hpp
    DBQueryHandler.cpp 
    DBQueryHandler.hpp
    EventLogs.cpp 
    EventLogs.hpp
)

target_sources(GameServer
    PRIVATE
    Game/DataMgr/DataDefine.cpp 
    Game/DataMgr/DataDefine.hpp
    Game/DataMgr/DataMgr.cpp
    Game/DataMgr/DataMgr.hpp
)

source_group( "Game\\DataMgr" FILES
    Game/DataMgr/DataDefine.cpp 
    Game/DataMgr/DataDefine.hpp
    Game/DataMgr/DataMgr.cpp
    Game/DataMgr/DataMgr.hpp
)

target_sources(GameServer
    PRIVATE
    Utils/Buffer.hpp
    Utils/Buffer.cpp
    Utils/MsgQueue.cpp 
    Utils/MsgQueue.hpp 
    Utils/LoggerHelper.cpp 
    Utils/LoggerHelper.hpp
    Utils/json.hpp
    Utils/IDGenerator.cpp 
    Utils/IDGenerator.hpp
)

source_group("Utils" FILES
    Utils/Buffer.hpp
    Utils/Buffer.cpp
    Utils/MsgQueue.cpp 
    Utils/MsgQueue.hpp 
    Utils/LoggerHelper.cpp 
    Utils/LoggerHelper.hpp
    Utils/json.hpp
    Utils/IDGenerator.cpp 
    Utils/IDGenerator.hpp
)

target_sources(GameServer
    PRIVATE
    Network/TcpSocket.hpp
    Network/TcpSocket.cpp
    Network/INetworkMgr.cpp 
    Network/INetworkMgr.hpp 
    Network/NetSlot.cpp 
    Network/NetSlot.hpp 
    # Network/NetworkMgrSelect.cpp 
    # Network/NetworkMgrSelect.hpp 
)

source_group("Network" FILES
    Network/TcpSocket.hpp
    Network/TcpSocket.cpp
    Network/INetworkMgr.cpp 
    Network/INetworkMgr.hpp 
    Network/NetSlot.cpp 
    Network/NetSlot.hpp 
    # Network/NetworkMgrSelect.cpp 
    # Network/NetworkMgrSelect.hpp 
)

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
    target_compile_definitions(GameServer
    PRIVATE
    -D__MACOS__
    )

    target_sources(GameServer
    PRIVATE
    Network/NetworkMgrKQueue.cpp 
    Network/NetworkMgrKQueue.hpp)

    source_group("Network" FILES
    Network/NetworkMgrKQueue.cpp
    Network/NetworkMgrKQueue.hpp
    )
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    target_compile_definitions(GameServer
        PRIVATE
        -D__LINUX__
    )

    target_sources(GameServer
        PRIVATE
        Network/NetworkMgrEpoll.cpp 
        Network/NetworkMgrEpoll.hpp)

    source_group("Network" FILES
        Network/NetworkMgrEpoll.cpp
        Network/NetworkMgrEpoll.hpp
    )
else()
    message(FATAL_ERROR "Unsupported platform: ${CMAKE_SYSTEM_NAME}")
endif()

# add dependencies
include(FetchContent)
set(FETCHCONTENT_BASE_DIR "${CMAKE_SOURCE_DIR}/../_deps/${CMAKE_SYSTEM_NAME}-${CMAKE_CXX_COMPILER_ID}-${CMAKE_GENERATOR}")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(SetupProtobuf)
include(SetupSqdlog)
include(SetupMysqlCpp)
# add dependencies end

target_include_directories(GameServer
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/proto
    ${CMAKE_CURRENT_SOURCE_DIR}/DataReader
    ${CMAKE_CURRENT_SOURCE_DIR}/DB
    ${CMAKE_CURRENT_SOURCE_DIR}/Player)




if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    set_target_properties(GameServer PROPERTIES
    BUILD_WITH_INSTALL_RPATH TRUE
    INSTALL_RPATH "$ORIGIN")
endif()



# set_target_properties(GameServer PROPERTIES
#     RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../bin
# )