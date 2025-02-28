//
//  GameLoop.hpp
//  GameServer
//
//  Created by pinky on 2025-02-19.
//

#ifndef GameLoop_hpp
#define GameLoop_hpp

#include <stdio.h>
#include "NetworkMgr.hpp"
#include "Player/PlayerMgr.hpp"

class GameLoop
{
public:
    bool Init();
    bool run();
    
public:
    void onReceiveMsg( int sockID,const Msg& packet );
    
    void dealLogin( int sockID, const Msg& packet );
    void dealAction( int sockID, const Msg& packet );
    void dealLogout( int sockID, const Msg& packet );
    
private:
    int m_nextRoleID = 1;
    PlayerManager m_playerMgr;
};

#endif /* GameLoop_hpp */
