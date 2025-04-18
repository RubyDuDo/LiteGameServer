cmake_minimum_required(VERSION 3.20)
project(RubyGameServer)

set(CMAKE_CXX_STANDARD 20)

enable_testing()

message(STATUS "CMake version: ${CMAKE_VERSION}")
message(STATUS "CMake generator: ${CMAKE_GENERATOR}")

message(STATUS " platform: ${CMAKE_SYSTEM_NAME}-${CMAKE_CXX_COMPILER_ID}-${CMAKE_GENERATOR}")


add_executable(GameServer)
add_library( GameServerLib STATIC )


set( SOURCE_Game 
    GameLoop.cpp
    GameLoop.hpp
    GameNetHelper.cpp 
    GameNetHelper.hpp
    DBQueryHandler.cpp 
    DBQueryHandler.hpp
    EventLogs.cpp 
    EventLogs.hpp
    Game/Event.cpp 
    Game/Event.hpp
    Game/SessionMgr.cpp 
    Game/SessionMgr.hpp)

source_group("Game" FILES
    ${SOURCE_Game}
    main.cpp
)

set( SOURCE_DataMgr
    Game/DataMgr/DataDefine.cpp 
    Game/DataMgr/DataDefine.hpp
    Game/DataMgr/DataMgr.cpp
    Game/DataMgr/DataMgr.hpp
)

source_group( "Game\\DataMgr" FILES
    ${SOURCE_DataMgr}
)

set( SOURCE_Utils
    Utils/Buffer.hpp
    Utils/Buffer.cpp
    Utils/MsgQueue.cpp 
    Utils/MsgQueue.hpp 
    Utils/LoggerHelper.cpp 
    Utils/LoggerHelper.hpp
    Utils/json.hpp
    Utils/IDGenerator.cpp 
    Utils/IDGenerator.hpp
    Utils/TimeService.cpp 
    Utils/TimeService.hpp
)

source_group("Utils" FILES
    ${SOURCE_Utils}
)

set( SOURCE_Network
    Network/TcpSocket.hpp
    Network/TcpSocket.cpp
    Network/INetworkMgr.cpp 
    Network/INetworkMgr.hpp 
    Network/NetSlot.cpp 
    Network/NetSlot.hpp 
    # Network/NetworkMgrSelect.cpp 
    # Network/NetworkMgrSelect.hpp 
)

set( SOURCE_DataReader
    DataReader/INIReader.cpp 
    DataReader/INIReader.hpp)

source_group("DataReader" FILES
    ${SOURCE_DataReader}
)

set( SOURCE_DB
    DB/DBMgr.cpp 
    DB/DBMgr.hpp
)

source_group("DB" FILES
    ${SOURCE_DB}
)

set( SOURCE_Player
    Player/Player.cpp 
    Player/Player.hpp
    Player/PlayerMgr.cpp 
    Player/PlayerMgr.hpp
)

source_group("Player" FILES
    ${SOURCE_Player}
)

set( SOURCE_proto
    proto/msg.pb.cc  
    proto/msg.pb.h 
    proto/dbmsg.pb.cc  
    proto/dbmsg.pb.h )

source_group("proto" FILES
    ${SOURCE_proto}
    proto/msg.proto 
    proto/dbmsg.proto)




get_target_property(TARGET_SOURCES GameServer SOURCES)
message(STATUS "Sources for GameServer: ${TARGET_SOURCES}")


if(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")  # macOS
    target_compile_definitions(GameServer
    PRIVATE
    -D__MACOS__
    )

    list( APPEND SOURCE_Network
        Network/NetworkMgrKQueue.cpp 
        Network/NetworkMgrKQueue.hpp
    )
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    target_compile_definitions(GameServer
        PRIVATE
        -D__LINUX__
    )

    list( APPEND SOURCE_Network
        Network/NetworkMgrEpoll.cpp
        Network/NetworkMgrEpoll.hpp
    )


else()
    message(FATAL_ERROR "Unsupported platform: ${CMAKE_SYSTEM_NAME}")
endif()


set( GAMESERVER_SOURCE 
    ${SOURCE_Game}
    ${SOURCE_DataMgr}
    ${SOURCE_Utils}
    ${SOURCE_Network}
    ${SOURCE_DataReader}
    ${SOURCE_DB}
    ${SOURCE_Player}
    ${SOURCE_proto}    
)

source_group("Network" FILES
    ${SOURCE_Network}
)

target_sources( GameServer 
    PRIVATE
    main.cpp
)

# add dependencies
include(FetchContent)
set(FETCHCONTENT_BASE_DIR "${CMAKE_SOURCE_DIR}/../_deps/${CMAKE_SYSTEM_NAME}-${CMAKE_CXX_COMPILER_ID}-${CMAKE_GENERATOR}")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(SetupProtobuf)
include(SetupSqdlog)
include(SetupMysqlCpp)
# add dependencies end

set(GAME_SERVER_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/proto
    ${CMAKE_CURRENT_SOURCE_DIR}/DataReader
    ${CMAKE_CURRENT_SOURCE_DIR}/DB
    ${CMAKE_CURRENT_SOURCE_DIR}/Player
)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    set_target_properties(GameServer PROPERTIES
    BUILD_WITH_INSTALL_RPATH TRUE
    INSTALL_RPATH "$ORIGIN")
endif()


# ================= test config ============================
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip 
  # or use Git Repository:
  # GIT_REPOSITORY https://github.com/google/googletest.git
  # GIT_TAG        release-1.14.0 
)

FetchContent_MakeAvailable(googletest)

target_sources( GameServerLib
    PRIVATE
    ${GAMESERVER_SOURCE})

target_include_directories(GameServerLib
    PUBLIC
    ${GAME_SERVER_INCLUDE_DIRS}
)


target_link_libraries( GameServer PRIVATE GameServerLib)

add_subdirectory(tests)



# set_target_properties(GameServer PROPERTIES
#     RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../bin
# )