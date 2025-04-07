message("Configuring spdlog")
FetchContent_Declare(
  spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog.git
  GIT_TAG        v1.14.1 
)

# fetch spdlog sources and add it to the build system
# FetchContent_MakeAvailable is a macro that adds the target spdlog to the current CMake project.
FetchContent_MakeAvailable(spdlog)

# add the include path
FetchContent_GetProperties(spdlog)
target_include_directories(GameServer PRIVATE
  ${spdlog_SOURCE_DIR}/../include
)
# add the library
target_link_libraries(GameServer PRIVATE spdlog::spdlog)


# using this method to control the log level at compile time could achieve better performance.
target_compile_definitions(GameServer PRIVATE
    # for debug， SPDLOG_ACTIVE_LEVEL 为 TRACE
    $<$<CONFIG:Debug>:SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE>

    # for Release , use the default setting.
    # $<$<CONFIG:Release>:SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_WARN>
)

# target_link_libraries(my_app PRIVATE fmt::fmt)