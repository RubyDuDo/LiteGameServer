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
}
