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

#include <csignal>

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
    
//    auto logger = spdlog::stdout_color_mt("console");
//
//        // 设置 pattern，包括 %@ 以打印 source location（文件名:行号）
//        logger->set_pattern("[%@:%#-%!] [%l] %v");
//
//        // 使用 source_loc 显式传入源码信息
//        spdlog::source_loc loc{__FILE_NAME__, __LINE__, SPDLOG_FUNCTION};
//        logger->log(loc, spdlog::level::info, "Hello with source info");
}

void signalHandler(int signum) {
//    const char *msg = "Received SIGHUP, reloading config...\n";
//        write(STDOUT_FILENO, msg, strlen(msg));
//    SPDLOG_INFO("Signal received!");
    if( signum == SIGINT || signum == SIGTERM)
    {
        SPDLOG_INFO("Signal SIGINT/SIGTERM received!");
        //gracefully shutdown
        m_pGame->stop();
    }
    else if( signum == SIGHUP )
    {
        SPDLOG_INFO("Signal SIGHUP received!");
        //hot reload configure
        m_pGame->reloadConfigure();
    }
}

int main(int argc, const char * argv[]) {
    // insert code here...
    std::cout << "Hello, World!\n";
    
    struct sigaction action;
    action.sa_handler = signalHandler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0; // !!! 重要：没有 SA_RESTART !!!

    // 注册 SIGTERM 和 SIGINT
    if (sigaction(SIGTERM, &action, NULL) == -1) {
        SPDLOG_ERROR("sigaction SIGTERM failed");
        return 1;
    }
    if (sigaction(SIGINT, &action, NULL) == -1) {
        SPDLOG_ERROR("sigaction SIGINT failed");
        return 1;
    }

    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    SPDLOG_INFO("Current working dir: %s\n", cwd);
    
    GameLoop game;
    m_pGame = &game;
    game.run();
    
    return 0;
}
