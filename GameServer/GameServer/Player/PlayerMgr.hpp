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
    void removePlayer( uint64_t roleID );
    
    bool isPlayerOnline( uint64_t roleID );
    
    Player* getPlayer( uint64_t roleID );
    
private:
    bool checkPass( const string& strName, const string& strPass ) const ;
    uint64_t getRoleID( const string& strName ) const;

    
    
private:
    map<int, Player> m_mapPlayers;
    
};

#endif /* PlayerMgr_hpp */
