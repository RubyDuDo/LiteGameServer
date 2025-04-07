//
//  GameLoop.hpp
//  GameServer
//
//  Created by pinky on 2025-02-19.
//

#ifndef GameLoop_hpp
#define GameLoop_hpp

#include <stdio.h>
#include <chrono>
#include "Player/PlayerMgr.hpp"
#include "DB/DBMgr.hpp"
#include "DBQueryHandler.hpp"
#include "proto/dbmsg.pb.h"
#include "DataReader/INIReader.hpp"

#include "Network/INetworkMgr.hpp"
#include "GameNetHelper.hpp"
#include "../Utils/IDGenerator.hpp"

using TimePoint = std::chrono::steady_clock::time_point;


class GameLoop : public INetHandler, public IDBResponseHandler
{
public:
    bool Init();
    bool run();
    
    void update( const TimePoint& now);
    
    void stop();
    
    void reloadConfigure();

    virtual ~GameLoop();
    
public:
    void onReceiveMsg( int sockID,const Msg& packet );
    void dealReceiveMsg( int sockID,const Msg& packet );
    
    
    void dealLogin( int sockID, const Msg& packet );
    void dealAction( int sockID, const Msg& packet );
    void dealLogout( int sockID, const Msg& packet );
    
public:
    virtual void onReceiveDBRsp( int queryID, std::unique_ptr<DBResponse>&& rsp );
    
//    void dealDBRsp( const DBResponse& rsp );
    void dealQueryAccount( int sockID, const string& strPasswd,  const DBResponse& rsp );
    void dealAddRole( int sockID, const DBResponse& rsp );
    
    void dealQueryRole( int sockID, const DBResponse& rsp  );
    
    void addDBQuery( std::unique_ptr<DBRequest>&& req, std::function<void( const DBResponse&)> func );
    
public:
    
    virtual void onReceiveMsg( int fd, const std::string& msg );
    virtual void onDisconnect( int fd ) ;
    virtual void onConnect( const TcpSocket& sock ) ;
    
private:
    int m_nextRoleID = 1;
    PlayerManager m_playerMgr;
    
    DBMgr m_db;
    DBQueryHandler m_dbQueryHandler;
    
    INIReader m_config;
    

    MsgQueue< pair<int, Msg> > m_recvMsgs;
    
    MsgQueue< pair<int, std::unique_ptr<DBResponse> >> m_dbmsgRsp;
    
    map<int, std::function<void( const DBResponse&)> > m_mapDBRspFuns;
    
    std::atomic<bool> m_bRunning = false;
    
    IDGenerator m_idGen;
    
    
};


#endif /* GameLoop_hpp */
