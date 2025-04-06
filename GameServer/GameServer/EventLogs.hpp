//
//  EventLogs.hpp
//  GameServer
//
//  Created by pinky on 2025-04-05.
//

#ifndef EventLogs_hpp
#define EventLogs_hpp

#include "spdlog/spdlog.h"

#include <stdio.h>

class EventLogs{
public:
    EventLogs() = default;
    ~EventLogs() = default;
    static EventLogs* getInstance();
    
    void initEventLogs();
private:
    static EventLogs* m_pInstance;

public:
    void onEventLogin( int ret, int roleID );
    void onEventLogout( int ret, int roleID );
    
private:
    void initLoggers( const std::string& logName, std::shared_ptr<spdlog::logger>& logger, const std::string& logFileName );
    
    int64_t getTimeStamp();

// using separate loggers
//    std::shared_ptr<spdlog::logger> m_loginLogger = nullptr;
//    std::shared_ptr<spdlog::logger> m_logoutLogger = nullptr;
   
    //using single logger
    std::shared_ptr<spdlog::logger> m_eventLogger = nullptr;

};
#endif /* EventLogs_hpp */
