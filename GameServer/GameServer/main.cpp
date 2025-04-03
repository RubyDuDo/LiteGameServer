//
//  main.cpp
//  GameServer
//
//  Created by pinky on 2025-02-19.
//

#include <iostream>
#include <unistd.h>   // for getcwd
#include <limits.h>   // for PATH_MAX
#include "GameLoop.hpp"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

GameLoop* m_pGame = nullptr;

void initLog()
{
    spdlog::info("Welcom to spdlog!");
    spdlog::flush_on(spdlog::level::info); // 设置 flush 级别
//    spdlog::error("Some error message with arg:{}", 1 );
    
//    try {
//       auto file_logger = spdlog::basic_logger_mt("basic_logger", "logs/basic-log.txt");
//       spdlog::set_default_logger(file_logger);
//       spdlog::flush_on(spdlog::level::info); // 设置 flush 级别
//       spdlog::info("Logging to a file");
//    } catch (const spdlog::spdlog_ex &ex) {
//       // 处理异常
//    }
}

int main(int argc, const char * argv[]) {
    // insert code here...
    std::cout << "Hello, World!\n";

    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    printf("Current working dir: %s\n", cwd);
    
    initLog();
    
    GameLoop game;
    m_pGame = &game;
    game.run();
    
    return 0;
}
