//
//  PlayerMgr.hpp
//  GameServer
//
//  Created by pinky on 2025-02-28.
//

#ifndef PlayerMgr_hpp
#define PlayerMgr_hpp

#include <stdio.h>
#include <map>
using namespace std;

#include "Player.hpp"

class PlayerManager
{
public:
    void onPlayerLogin( int sockID, const string& strName, const string& strPass );
    void onPlayerLogout( int sockID,int roleID );
    
    bool isPlayerOnline( int roleID );
    int getPlayerIDFromSock( int sockID );
    
    Player* getPlayer( int roleID );
    
private:
    bool checkPass( const string& strName, const string& strPass ) const ;
    int getRoleID( const string& strName ) const;
    
    bool isMatchSockAndRole( int sockID, int roleID );
    
    void removePlayer( int roleID );
    
private:
    map<int, Player> m_mapPlayers;
    mutable int m_nextRoleID = 1;
    
    map<int, int> m_mapRoleToSock;
    map<int, int> m_mapSockToRole;
    
    
};

#endif /* PlayerMgr_hpp */
