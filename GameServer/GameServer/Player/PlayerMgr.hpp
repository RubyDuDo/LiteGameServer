//
//  PlayerMgr.hpp
//  GameServer
//
//  Created by pinky on 2025-02-28.
//

#ifndef PlayerMgr_hpp
#define PlayerMgr_hpp

#include <stdio.h>
#include <stdint.h>
#include <map>
using namespace std;

#include "Player.hpp"


class PlayerManager
{
public:
    bool init( );
    void addPlayer( int sockID, const string& strName, uint64_t roleid, int level );
    void onPlayerLogout( int sockID, uint64_t roleID );
    
    bool isPlayerOnline( uint64_t roleID );
    uint64_t getPlayerIDFromSock( int sockID );
    
    Player* getPlayer( uint64_t roleID );
    
private:
    bool checkPass( const string& strName, const string& strPass ) const ;
    uint64_t getRoleID( const string& strName ) const;
    
    bool isMatchSockAndRole( int sockID, uint64_t roleID );
    
    void removePlayer( uint64_t roleID );
    
private:
    map<int, Player> m_mapPlayers;
    
    map<uint64_t, int> m_mapRoleToSock;
    map<int, uint64_t> m_mapSockToRole;
};

#endif /* PlayerMgr_hpp */
