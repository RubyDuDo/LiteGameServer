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
#include "DB/DBMgr.hpp"
#include "proto/dbmsg.pb.h"

class GameLoop
{
public:
    bool Init();
    bool run();
    
    void update();
    
public:
    void onReceiveMsg( int sockID,const Msg& packet );
    void dealReceiveMsg( int sockID,const Msg& packet );
    
    
    void dealLogin( int sockID, const Msg& packet );
    void dealAction( int sockID, const Msg& packet );
    void dealLogout( int sockID, const Msg& packet );
    
public:
    void onReceiveDBRsp( int queryID, const DBResponse& rsp );
    
//    void dealDBRsp( const DBResponse& rsp );
    void dealQueryAccount( int sockID, const string& strPasswd,  const DBResponse& rsp );
    void dealAddRole( int sockID, const DBResponse& rsp );
    
    void dealQueryRole( int sockID, const DBResponse& rsp  );
    
    void addDBQuery( const DBRequest& req, std::function<void( const DBResponse&)> func );
    
private:
    int m_nextRoleID = 1;
    PlayerManager m_playerMgr;
    DBMgr m_db;
    

    MsgQueue< pair<int, Msg> > m_recvMsgs;
    
    MsgQueue< pair<int, DBResponse>> m_dbmsgRsp;
    
    map<int, std::function<void( const DBResponse&)> > m_mapDBRspFuns;
};

#endif /* GameLoop_hpp */
