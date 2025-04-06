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

int PlayerManager::getRoleID( const string& strName ) const
{
    //
    int roleID = m_nextRoleID;
    m_nextRoleID++;
    return roleID;
}

void PlayerManager::addPlayer( int sockID, const string& strName, int roleid, int level )
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

void PlayerManager::onPlayerLogin( int sockID, const string& strName, const string& strPass )
{
    
    if( !checkPass( strName,  strPass ))
    {
        cout<<"Password not match"<<endl;
        return;
    }
    
    int roleID = getRoleID( strName );
    
    auto it = m_mapPlayers.find(roleID);
    
    if( it == m_mapPlayers.end())
    {
        Player player;
        player.m_roleID = roleID;
        player.m_strName = strName;
        player.m_level = 1;
        player.m_state = Player_Login;
        
        m_mapPlayers[ roleID ] = player;
    }
    else{
        it->second.m_state = Player_Login;
        
        //here need to clear the original role sock map
        
    }
    
    m_mapRoleToSock[ roleID ] = sockID;
    m_mapSockToRole[ sockID ] = roleID;
    
    ResponseLogin rsp;
    rsp.set_roleid( roleID );
    
    NetSendHelper::addTcpQueue( sockID, MsgType_Login, MsgErr_OK, rsp);
}

void PlayerManager::onPlayerLogout( int sockID, int roleID )
{
    if( !isMatchSockAndRole(sockID, roleID ) )
    {
        cout<<"Mismatch sockID roleID :"<<sockID<<"_"<<roleID<<endl;
    }

    auto it = m_mapPlayers.find(roleID);
    if( it == m_mapPlayers.end() )
    {
        cout<<"This player not exist"<<endl;
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

bool PlayerManager::isPlayerOnline( int roleID )
{
    auto it = m_mapPlayers.find( roleID );
    if( it != m_mapPlayers.end() )
    {
        if( it->second.m_state == Player_Login )
            return true;
    }

    return false;
}

int PlayerManager::getPlayerIDFromSock( int sockID )
{
    return m_mapSockToRole[sockID];
}

Player* PlayerManager::getPlayer( int roleID )
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

void PlayerManager::removePlayer( int roleID )
{
    m_mapPlayers.erase( roleID );
    
    int sockID = m_mapRoleToSock[roleID];
    m_mapRoleToSock.erase( roleID );
    m_mapSockToRole.erase( sockID );
}

bool PlayerManager::isMatchSockAndRole( int sockID, int roleID )
{
    if( m_mapSockToRole[sockID] == roleID )
    {
        return true;
    }
    else{
        return false;
    }
}
