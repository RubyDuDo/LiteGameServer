//
//  SessionMgr.cpp
//  GameServer
//
//  Created by pinky on 2025-04-17.
//

#include "SessionMgr.hpp"
#include "../Utils/LoggerHelper.hpp"
SessionInfo::SessionInfo( int sockID, uint64_t roleID, TimePoint curTime ):m_sockID( sockID )
, m_roleID( roleID )
, m_lastHeartbeatTime( curTime )
{
    
}

void SessionMgr::setTimeService( ITimeService* pTimeService )
{
    m_pTimeService = pTimeService;
}

//void SessionMgr::setTimeService( ITimeService* pTimeService );
int SessionMgr::getSockIDFromRoleID( uint64_t roleID )
{
    auto it = m_mapRoleToSocks.find( roleID );
    if( it == m_mapRoleToSocks.end()  )
    {
        return 0;
    }
    else{
        return it->second;
    }
}

uint64_t SessionMgr::getRoleIDFromSockID( int sockID )
{
    auto it = m_mapSessions.find( sockID );
    if( it == m_mapSessions.end() )
    {
        return 0;
    }
    else{
        return it->second.m_roleID;
    }
    
    
}

void SessionMgr::addSessionInfo( int sockID, uint64_t roleID )
{
    SPDLOG_DEBUG("s_{},r_{}", sockID, roleID);
    if( sockID <= 0 || roleID <= 0  )
    {
        //Invalid param
        SPDLOG_DEBUG("Invalid param:s{}_r{}", sockID, roleID );
    }
    //check if these roleID exist
    auto preSockID = getSockIDFromRoleID( roleID );
    //check if this sockID exist
    auto preRoleID = getRoleIDFromSockID( sockID );
    
    if( preSockID == sockID  && preRoleID == roleID )
    {
        //pre exist, do nothing
    }
    else if( preSockID != 0 )
    {
        //have unmatch data, remove previous ones
        //needs to clear there information
        m_mapRoleToSocks.erase( roleID );
    }
    else if( preRoleID != 0 )
    {
        //have unmatch data, remove previous ones
        //needs to clear there information
        m_mapSessions.erase( sockID );
    }
    
    m_mapSessions.insert( std::make_pair( sockID, SessionInfo( sockID,  roleID ,  m_pTimeService->getCurTime() )) );
    
}

bool SessionMgr::isMatchSockAndRole( int sockID, uint64_t roleID )
{
    auto preSockID = getSockIDFromRoleID( roleID );
    //check if this sockID exist
    auto preRoleID = getRoleIDFromSockID( sockID );
    
    if( preSockID == sockID && preRoleID == roleID )
    {
        return true;
    }
    else{
        return false;
    }
}

void SessionMgr::removeSession( int sockID )
{
    SPDLOG_DEBUG("{}", sockID);
    auto roleID = getRoleIDFromSockID( sockID );
    
    m_mapSessions.erase( sockID );
    if( roleID != 0 )
    {
        m_mapRoleToSocks.erase( roleID );
    }
    
}

TimePoint SessionMgr::getCurTime()
{
    if( m_pTimeService){
        return m_pTimeService->getCurTime();
    }
    else{
        return std::chrono::steady_clock::now();
    }
    
}

void SessionMgr::refreshHeartbeat( int sockID )
{
    SPDLOG_DEBUG("{}", sockID);
    auto it = m_mapSessions.find( sockID );
    if( it != m_mapSessions.end() )
    {
        it->second.m_lastHeartbeatTime = m_pTimeService->getCurTime();
    }
    
}
