# config protobuf
message("Configuring protobuf")
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
target_include_directories(GameServerLib PUBLIC
  ${protobuf_SOURCE_DIR}/../src
)

target_link_libraries(GameServer
  PUBLIC
  protobuf::libprotobuf  # 使用 FetchContent 的现代 CMake 方式
)


FetchContent_MakeAvailable(protobuf)