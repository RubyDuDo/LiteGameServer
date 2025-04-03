//
//  Logger.cpp
//  GameServer
//
//  Created by pinky on 2025-04-02.
//

#include "LoggerHelper.hpp"
#include <spdlog/sinks/stdout_color_sinks.h> // for console sink
#include <spdlog/sinks/basic_file_sink.h>    // for basic file sink
#include <spdlog/sinks/rotating_file_sink.h> // for rotating file sink
#include <spdlog/sinks/daily_file_sink.h>
#include <vector>
#include <iostream> // for std::cerr
#include <exception> // for std::exception

// Function implementation
std::shared_ptr<spdlog::logger> LoggerHelper::setupLogger(
    const std::string& logger_name,
    bool log_to_console,
    LogFileType file_type,
    const std::string& file_path,
    spdlog::level::level_enum level,
    spdlog::level::level_enum flush_level,
    bool set_as_default,
    size_t max_file_size_mb,
    size_t max_files,
    bool truncate_basic_file
) {
    std::string concole_pattern ="[%H:%M:%S.%e] [%^%l%$] [%s-%!] %v";
    
    std::string file_pattern = "%Y-%m-%d %H:%M:%S.%e [%l] [%s-%!] %v";
    // First, try to get an existing logger with the same name to avoid duplicates
    auto existing_logger = spdlog::get(logger_name);
    if (existing_logger) {
        // If needed, configuration of the existing logger could be modified here, or just return it directly
        std::cout << "Logger '" << logger_name << "' already exists. Returning existing instance." << std::endl;
        return existing_logger;
    }

    try {
        std::vector<spdlog::sink_ptr> sinks;

        // Add console Sink
        if (log_to_console) {
            auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            // Can set the level for the console sink individually if needed
            // console_sink->set_level(spdlog::level::trace);
            sinks.push_back(console_sink);
            console_sink->set_pattern(concole_pattern);
        }

        // Add file Sink
        if (file_type != LogFileType::NONE) {
            spdlog::sink_ptr file_sink = nullptr;
            switch (file_type) {
                case LogFileType::BASIC:
                    file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(file_path, truncate_basic_file);
                    break;
                case LogFileType::ROTATING:
                    file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
                        file_path,
                        max_file_size_mb * 1024 * 1024, // Convert MB to bytes
                        max_files
                    );
                    break;
                case LogFileType::DAILY:
                    {
                        int rotation_hour = 0;
                        int rotation_minute = 0;
        
                        file_sink = std::make_shared<spdlog::sinks::daily_file_sink_mt>(file_path, rotation_hour, rotation_minute);
                    }
                    
                    break;
                default:
                    // Handle unknown types, or this won't be executed in the case of NONE
                    break;
            }
            if (file_sink) {
                 // Can set the level for the file sink individually if needed
                // file_sink->set_level(spdlog::level::info);
                sinks.push_back(file_sink);
                file_sink->set_pattern(file_pattern);
            }
        }

        // If no sinks are configured, cannot create the logger
        if (sinks.empty()) {
            std::cerr << "Error setting up logger '" << logger_name << "': No sinks configured." << std::endl;
            return nullptr;
        }

        // Create the Logger
        auto logger = std::make_shared<spdlog::logger>(logger_name, sinks.begin(), sinks.end());

        // Set the Logger's overall level
        logger->set_level(level);

        // Set the Logger's flush level
        logger->flush_on(flush_level);

        // Register the Logger
        spdlog::register_logger(logger);

        // Set as default Logger (if needed)
        if (set_as_default) {
            spdlog::set_default_logger(logger);
        }

        std::cout << "Logger '" << logger_name << "' setup successfully." << std::endl;
        return logger;

    } catch (const spdlog::spdlog_ex& ex) {
        std::cerr << "Log initialization failed for logger '" << logger_name << "': " << ex.what() << std::endl;
        // Can optionally remove the logger that might have been partially registered
        spdlog::drop(logger_name);
        return nullptr;
    } catch (const std::exception& ex) {
         std::cerr << "An unexpected error occurred during logger '" << logger_name << "' setup: " << ex.what() << std::endl;
         spdlog::drop(logger_name);
         return nullptr;
    }
}
