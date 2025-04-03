//
//  Logger.hpp
//  GameServer
//
//  Created by pinky on 2025-04-02.
//

#ifndef Logger_hpp
#define Logger_hpp

#include <stdio.h>

#include <spdlog/spdlog.h>
#include <string>
#include <memory> // for std::shared_ptr

// Define file log types
enum class LogFileType {
    NONE,    // No file logging
    BASIC,   // Simple file log
    ROTATING, // Rotating file log
    DAILY   // Daily file log
};


class LoggerHelper
{
public:
    // Initialization function declaration
    /**
     * @brief Initializes an spdlog logger instance
     *
     * @param logger_name The name of the logger to create or retrieve
     * @param log_to_console Whether to enable console logging (stdout)
     * @param file_type The type of file logging (NONE, BASIC, ROTATING)
     * @param file_path The path for the log file (used only if file_type != NONE)
     * @param level Minimum logging level for the logger (default: debug)
     * @param flush_level Minimum logging level to trigger automatic flush (default: info)
     * @param set_as_default Whether to set this logger as the global default logger (default: true)
     * @param max_file_size_mb Maximum size of a single rotating log file in MB (used only if file_type == ROTATING, default: 5)
     * @param max_files Maximum number of rotating log files to keep (used only if file_type == ROTATING, default: 3)
     * @param truncate_basic_file Whether to truncate the basic log file on startup (used only if file_type == BASIC, default: true)
     * @return std::shared_ptr<spdlog::logger> The created or retrieved logger instance, or nullptr on failure
     */
    static std::shared_ptr<spdlog::logger> setupLogger(
        const std::string& logger_name,
        bool log_to_console,
        LogFileType file_type,
        const std::string& file_path = "logs/default_log.txt", // Default path
        spdlog::level::level_enum level = spdlog::level::debug,
        spdlog::level::level_enum flush_level = spdlog::level::info,
        bool set_as_default = true,
        size_t max_file_size_mb = 5,
        size_t max_files = 3,
        bool truncate_basic_file = true
    );


};

#endif /* Logger_hpp */
