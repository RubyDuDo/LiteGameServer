//
//  SessionMgr.hpp
//  GameServer
//
//  Created by pinky on 2025-04-17.
//

#ifndef SessionMgr_hpp
#define SessionMgr_hpp

#include <stdio.h>
#include <stdint.h>
#include <map>
#include "../Utils/TimeService.hpp"

class SessionInfo
{
public:
    SessionInfo( int sockID, uint64_t roleID, TimePoint curTime );
    
    int m_sockID = 0;
    uint64_t m_roleID = 0;
    TimePoint m_lastHeartbeatTime;
};

class SessionMgr
{
public:
    void setTimeService( ITimeService* pTimeService );
    
    int getSockIDFromRoleID( uint64_t roleID );
    uint64_t getRoleIDFromSockID( int sockID );
    
    void addSessionInfo( int sockID, uint64_t roleID );
    void removeSession( int sockID );
    
    void refreshHeartbeat( int sockID );
    
    bool isMatchSockAndRole( int sockID, uint64_t roleID );

private:
    TimePoint getCurTime();
    
    
    ITimeService* m_pTimeService;
    std::map<int, SessionInfo>  m_mapSessions;
    std::map<uint64_t, int>  m_mapRoleToSocks;
};

#endif /* SessionMgr_hpp */
