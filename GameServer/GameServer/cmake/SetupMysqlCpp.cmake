message(STATUS "Configuring MySQL++ for GameServer")

if(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")  # macOS
    message(STATUS "Detected platform: macOS")
    set( platform "mac" )
    message(STATUS "Detected platform: ${platform}")
    set(MYSQL_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../thirdlibs/${platform}/mysql-connector-c++)

    target_link_libraries(GameServer
        PUBLIC
        ${MYSQL_ROOT}/lib64/libssl.dylib
        ${MYSQL_ROOT}/lib64/libcrypto.dylib
        ${MYSQL_ROOT}/lib64/libmysqlcppconn-static.a
    )

    set_target_properties(GameServer PROPERTIES
        BUILD_RPATH  ${MYSQL_ROOT}/lib64
    )

elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    message(STATUS "Detected platform: Linux")
    set( platform "linux" )
    message(STATUS "Detected platform: ${platform}")

    set(MYSQL_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../thirdlibs/${platform}/mysql-connector-c++)
        
    target_link_libraries(GameServer
        PUBLIC
        ${MYSQL_ROOT}/lib64/libmysqlcppconn.so.9.8.2.0
    )
else()
    message(FATAL_ERROR "Unsupported platform: ${CMAKE_SYSTEM_NAME}-${CMAKE_CXX_COMPILER_ID}")
endif()

target_include_directories(GameServerLib
    PUBLIC
    ${MYSQL_ROOT}/include
)

target_link_libraries(GameServer
    PRIVATE
    resolv)