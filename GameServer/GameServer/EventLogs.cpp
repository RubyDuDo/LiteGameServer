//
//  EventLogs.cpp
//  GameServer
//
//  Created by pinky on 2025-04-05.
//

#include <iostream>
#include <chrono>
#include "EventLogs.hpp"
#include "Utils/json.hpp"
#include "Utils/LoggerHelper.hpp"

using namespace nlohmann;

EventLogs* EventLogs::m_pInstance = nullptr;
EventLogs* EventLogs::getInstance()
{
    if( !m_pInstance)
    {
        m_pInstance = new EventLogs();
    }
    
    return m_pInstance;
}

void EventLogs::initEventLogs()
{
    //using separate loggers for login and logout events
//    initLoggers( "login", m_loginLogger, "loginEvent.txt");
//    initLoggers( "logout", m_logoutLogger, "logoutEvent.txt");
    
    //using a single logger for all events
    initLoggers( "events", m_eventLogger, "events.txt");
    
}

void EventLogs::initLoggers( const std::string& logName,
                            std::shared_ptr<spdlog::logger>& logger, const std::string& logFileName )
{
    try{
        // Add file Sink
        logger = LoggerHelper::setupLogger(
            logName,
            false,
                                           LogFileType::BASIC,
            logFileName,
            spdlog::level::level_enum::info,
            spdlog::level::level_enum::info,
            false);
        
        std::string file_pattern = "%v";
        logger->set_pattern(file_pattern);
        
    } catch (const spdlog::spdlog_ex& ex) {
        std::cerr << "Event Log initialization failed for logger '" << logName << "': " << ex.what() << std::endl;
        // Can optionally remove the logger that might have been partially registered
        spdlog::drop(logName);
        return ;
    } catch (const std::exception& ex) {
         std::cerr << "An unexpected error occurred during logger '" << logName << "' setup: " << ex.what() << std::endl;
         spdlog::drop(logName);
         return ;
    }
}

void EventLogs::onEventLogin( int ret,  int roleID )
{
    if( m_eventLogger )
    {
        json info;
        info["evtType"] = "Login";
        info["t"] = getTimeStamp();
        
        info["ret"] = ret;
        info["rID"] = roleID;
        m_eventLogger->info( info.dump() );
    }
}

void EventLogs::onEventLogout( int ret, int roleID )
{
    if( m_eventLogger )
    {
        json info;
        info["evtType"] = "Logout";
        info["t"] = getTimeStamp();
        
        info["rID"] = roleID;
        info["ret"] = ret;
        m_eventLogger->info(info.dump());
    }
}

int64_t EventLogs::getTimeStamp()
{
    auto now = std::chrono::system_clock::now();
    auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
    int64_t timestamp_ms = duration_ms.count();
}
