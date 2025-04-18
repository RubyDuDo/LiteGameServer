# tests/CMakeLists.txt

# 查找测试源文件 (相对于 tests 目录)
set(TEST_SOURCES
    Utils/IDGenerator_test.cpp
    # ... 添加其他测试文件 ...
)

# define test executable 
add_executable(run_tests ${TEST_SOURCES})

# link GameServerLib and GoogleTest Lib
# link GameServerLib
target_link_libraries(run_tests PRIVATE GameServerLib GTest::gtest GTest::gtest_main)

# CMAKE_SOURCE_DIR point to root dir
target_include_directories(run_tests PRIVATE ${CMAKE_SOURCE_DIR}) 

# 自动发现测试
include(GoogleTest)
gtest_discover_tests(run_tests)