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
#include <memory>
#include "Player/PlayerMgr.hpp"
#include "DB/DBMgr.hpp"
#include "DBQueryHandler.hpp"

#include "DataReader/INIReader.hpp"

#include "Network/INetworkMgr.hpp"
#include "GameNetHelper.hpp"
#include "../Utils/IDGenerator.hpp"
#include "../Game/Event.hpp"
#include "../Game/IMsgRouter.hpp"
#include "../Game/SessionMgr.hpp"


#include "../Utils/TimeService.hpp"

class GameLoop : public INetHandler, public IDBResponseHandler, public IMsgRouter
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
    void dealHeartBeat( int sockID, const Msg& packet );
    
public:
    virtual void onReceiveDBRsp( int queryID, std::unique_ptr<DBResponse>&& rsp );
    
//    void dealDBRsp( const DBResponse& rsp );
    void dealQueryAccount( int sockID, const string& strPasswd,  const DBResponse& rsp );
    void dealAddRole( int sockID, const DBResponse& rsp );
    
    void dealQueryRole( int sockID, const DBResponse& rsp  );
    
    virtual void addDBQuery( std::unique_ptr<DBRequest>&& req, std::function<void( const DBResponse&)> func ) override;
    
public:
    
    virtual void onReceiveMsg( int fd, const std::string& msg );
    virtual void onDisconnect( int fd ) ;
    virtual void onConnect( const TcpSocket& sock ) ;
    
//Inner Event
    virtual void sendEvent( std::unique_ptr<Event>&& evt ) override;
    
    void dealRecvEvent( Event& evt );
    void dealEvtConnect( Event& evt );
    void dealEvtDisconnect( Event& evt );
    
private:
    void heartBeatCheck();
private:
    PlayerManager m_playerMgr;
    
    DBMgr m_db;
    DBQueryHandler m_dbQueryHandler;
    
    INIReader m_config;
    

    MsgQueue< pair<int, Msg> > m_recvMsgs;
    
    MsgQueue< pair<int, std::unique_ptr<DBResponse> >> m_dbmsgRsp;
    
    map<int, std::function<void( const DBResponse&)> > m_mapDBRspFuns;
    
    std::atomic<bool> m_bRunning = false;
    
    IDGenerator m_idGen;
    
    //Inner Event
    MsgQueue< Event > m_innerEvts;
    
    //Service
    CTimeService m_timeService;
    
    
    
    //session
    SessionMgr m_sessionMgr;
    int m_heartbeatCheckInterval = 1;
    int m_heartbeatSendInterval = 2;
    int m_heartbeatDisconnectInterval = 10;
    
    
};


#endif /* GameLoop_hpp */
