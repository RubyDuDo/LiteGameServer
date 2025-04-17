//
//  PlayerMgr.cpp
//  GameServer
//
//  Created by pinky on 2025-02-28.
//

#include "PlayerMgr.hpp"
#include "../GameNetHelper.hpp"
#include "../EventLogs.hpp"

#include <iostream>
using namespace std;

bool PlayerManager::checkPass( const string& strName, const string& strPass ) const
{
    //todo
    return true;
}

bool PlayerManager::init(  )
{
    return true;
}

void PlayerManager::addPlayer( int sockID, const string& strName, uint64_t roleid, int level )
{
    Player player;
    player.m_roleID = roleid;
    player.m_strName = strName;
    player.m_level = 1;
    player.m_state = Player_Login;
    
    m_mapPlayers[roleid] = player;
    
    m_mapRoleToSock[ roleid ] = sockID;
    m_mapSockToRole[ sockID ] = roleid;
    
}


void PlayerManager::onPlayerLogout( int sockID, uint64_t roleID )
{
    if( !isMatchSockAndRole(sockID, roleID ) )
    {
        SPDLOG_ERROR("Mismatch sockID roleID:{},{}", sockID, roleID );
        return;
    }

    auto it = m_mapPlayers.find(roleID);
    if( it == m_mapPlayers.end() )
    {
        SPDLOG_ERROR("This player not exist");
        return;
    }
    
    if( it->second.m_state != Player_Login )
    {
        cout<<"This player not login"<<endl;
        return;
    }
    
    removePlayer( roleID );
    
    ResponseLogout rsp;
    rsp.set_roleid( roleID );
    
    NetSendHelper::addTcpQueue( sockID,MsgType_Logout, MsgErr_OK,  rsp);
    EventLogs::getInstance()->onEventLogout( MsgErr_OK, roleID );
}

bool PlayerManager::isPlayerOnline( uint64_t roleID )
{
    auto it = m_mapPlayers.find( roleID );
    if( it != m_mapPlayers.end() )
    {
        if( it->second.m_state == Player_Login )
            return true;
    }

    return false;
}

uint64_t PlayerManager::getPlayerIDFromSock( int sockID )
{
    auto it = m_mapSockToRole.find( sockID );
    if( it != m_mapSockToRole.end() )
    {
        return it->second;
    }
    else{
        return 0;
    }
}

void PlayerManager::onSockDisconnect( int sockID )
{
    int64_t roleID = getPlayerIDFromSock( sockID );
    if( roleID != 0 )
    {
        removePlayer( roleID );
    }
}

Player* PlayerManager::getPlayer( uint64_t roleID )
{
    auto it = m_mapPlayers.find( roleID );
    if( it != m_mapPlayers.end())
    {
        return &it->second;
    }
    else{
        return nullptr;
    }
    
}

void PlayerManager::removePlayer( uint64_t roleID )
{
    SPDLOG_DEBUG("removePlayer:{}", roleID );
    m_mapPlayers.erase( roleID );
    
    int sockID = m_mapRoleToSock[roleID];
    m_mapRoleToSock.erase( roleID );
    m_mapSockToRole.erase( sockID );
}

bool PlayerManager::isMatchSockAndRole( int sockID, uint64_t roleID )
{
    if( m_mapSockToRole[sockID] == roleID )
    {
        return true;
    }
    else{
        return false;
    }
}
