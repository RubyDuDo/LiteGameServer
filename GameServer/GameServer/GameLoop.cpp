//
//  GameLoop.cpp
//  GameServer
//
//  Created by pinky on 2025-02-19.
//

#include "GameLoop.hpp"
#include <iostream>
#include <thread>
using namespace std;
using namespace std::chrono;

#include "Utils/LoggerHelper.hpp"
#include "EventLogs.hpp"

#include "Game/DataMgr/DataMgr.hpp"

#include "proto/msg.pb.h"
using namespace MyGame;

const unsigned short SVR_PORT_DEF = 8081;
const string DB_HOST_DEF = "tcp://127.0.0.1:3306";
const string DB_USER_DEF = "admin";
const string DB_PASSWD_DEF = "111111";
const string DB_NAME_DEF = "MyGame";
const int  ServerID_DEF = 1024;

const int HEARTBEAT_CHECK_INTERVAl_DEF = 1;
const int HEARTBEAT_SEND_INTERVAl_DEF = 2;
const int HEARTBEAT_DISCONNECT_INTERVAL_DEF = 10;

const string LOG_TYPE_DEF = "basic";
const string LOG_PATH_DEF = "logs/app_basic.log";

constexpr string config_dir = "./config.ini";


bool GameLoop::Init()
{
    int res = m_config.ParseFile( config_dir );
    
    std::string logFileType = m_config.getString("Log", "type", LOG_TYPE_DEF);
    std::string logFilePath = m_config.getString("Log", "path", LOG_PATH_DEF);
    LogFileType logType = LogFileType::BASIC;
    if( logFileType == "rotate")
    {
        logType = LogFileType::ROTATING;
    }
    else if( logFileType == "daily")
    {
        logType = LogFileType::DAILY;
    }
    else if( logFileType == "none")
    {
        logType = LogFileType::NONE;
    }

    LoggerHelper::setupLogger( "main_logger", true, logType, logFilePath, spdlog::level::debug, spdlog::level::info, true );
    
    DataMgr::getInstance()->initData();
    auto moster = DataMgr::getInstance()->getData<MonsterInfo>( 1 );
    if( moster )
    {
        SPDLOG_INFO("test:Monster ID: {}, Name: {}", moster->getID(), moster->name );
    }
    
    EventLogs::getInstance()->initEventLogs();
    
    unsigned short port = (unsigned short)m_config.getInt( "Network" , "port", SVR_PORT_DEF );
    
    bool ret = true;
    ret = INetworkMgr::getInstance()->initNetwork( port );
    if( ret )
    {
        INetworkMgr::getInstance()->registerHandler( this );
    }
    else{
        SPDLOG_ERROR("Init network failed!");
        return false;
    }
    
    string host = m_config.getString("DB", "host", DB_HOST_DEF );
    string user = m_config.getString("DB", "user", DB_USER_DEF );
    string passwd = m_config.getString("DB", "passwd", DB_PASSWD_DEF );
    string dbname = m_config.getString("DB", "dbname", DB_NAME_DEF );
    m_db.InitDB(host, user, passwd, dbname);
    m_db.registerQueryHandler( &m_dbQueryHandler);
    m_db.registerResponseHandler( this );
   
    int serverID = m_config.getInt("Server", "serverID", ServerID_DEF );
    ret = m_idGen.init( serverID );
    if( !ret ){
        SPDLOG_ERROR("IDGenerator init failed!");
    }
    
    ret = m_playerMgr.init();
    if( !ret ){
        SPDLOG_ERROR("PlayerManager init failed!");
    }
    
    m_heartbeatCheckInterval = m_config.getInt("HeartBeat","check_interval",HEARTBEAT_CHECK_INTERVAl_DEF);
    m_heartbeatSendInterval = m_config.getInt("HeartBeat","send_interval", HEARTBEAT_SEND_INTERVAl_DEF);
    m_heartbeatDisconnectInterval = m_config.getInt("HeartBeat","disconnect_interval", HEARTBEAT_DISCONNECT_INTERVAL_DEF );
    
    m_bRunning = true;
    
    return ret;
}

void GameLoop::stop()
{
    m_bRunning = false;
}

void GameLoop::reloadConfigure()
{
    //todo , reload configure
}

bool GameLoop::run()
{
    bool ret = Init();
    if( !ret )
    {
        SPDLOG_ERROR("Init Failed!");
        return ret;
    }
    else{
        SPDLOG_INFO("Init succeed!");
    }
    
    
    SPDLOG_INFO("Start Run!");
    auto step = milliseconds( 20 );
    
    while( m_bRunning)
    {
        auto now = steady_clock::now();
        update( now );
        auto next = now + step ;
        std::this_thread::sleep_until( next );
        
    }
    
    SPDLOG_INFO("Stop Run!");
    INetworkMgr::getInstance()->shutdownNetwork();
    SPDLOG_INFO("Stop Run: shut Network End!");
    m_db.shutdownDB();
    
    SPDLOG_INFO("Stop Run End!");
    
    return true;
}

void GameLoop::onReceiveMsg( int fd, const std::string& msg )
{
    Msg packet;
    if( !packet.ParseFromString( msg ))
    {
        SPDLOG_WARN("Parse head fail!");
        return;
    }
    m_recvMsgs.push( make_pair( fd, packet ));
}

void GameLoop::onDisconnect( int fd )
{
    auto evt = new EventDisconnect( fd );
    sendEvent( std::unique_ptr<Event>( evt ) );
    SPDLOG_DEBUG("onDisconnect fd:{}", fd );
    
}

void GameLoop::onConnect( const TcpSocket& sock )
{
    auto evt = new EventConnect( sock.m_sock );
    sendEvent( std::unique_ptr<Event>( evt ) );
    SPDLOG_DEBUG("onConnect fd:{}", sock.m_sock );
    
}

void GameLoop::update( const TimePoint& now)
{
    m_timeService.update();
    
    //deal network msg
    auto itMsg = m_recvMsgs.try_pop();
    while( itMsg )
    {
        dealReceiveMsg( itMsg->first, itMsg->second );
        itMsg = m_recvMsgs.try_pop();
    }
    
    //deal db msg
    auto itdb = m_dbmsgRsp.try_pop();
    while( itdb )
    {
        int queryID = itdb->first;
        auto& rsp = itdb->second;
        auto itCb = m_mapDBRspFuns.find( queryID );
        if( itCb != m_mapDBRspFuns.end())
        {
            auto callback = itCb->second;
            callback( *rsp );
            
            m_mapDBRspFuns.erase( itCb );
        }
        
        itdb = m_dbmsgRsp.try_pop();
    }
    
    //deal Inner Event
    auto itEvt = m_innerEvts.try_pop();
    while( itEvt )
    {
        dealRecvEvent( *itEvt );
        
        itEvt = m_innerEvts.try_pop();
    }
    
    heartBeatCheck();
    
    //todo update gamelogic
}

void GameLoop::addDBQuery( std::unique_ptr<DBRequest>&& req, std::function<void( const DBResponse&)> func )
{
    SPDLOG_DEBUG("addDBQuery:{}", (int)req->head().type() );
    int queryID = m_db.getNextID();
    m_db.addDBQuery( queryID, std::move(req) );
    m_mapDBRspFuns[queryID] = func;
    
}

void GameLoop::onReceiveDBRsp( int queryID, std::unique_ptr<DBResponse>&& rsp )
{
    SPDLOG_DEBUG("onReceiveDBRsp:{0},{1}", (int)rsp->head().type(), queryID );
    m_dbmsgRsp.push( make_pair( queryID, std::move(rsp) ) );
}



//void GameLoop::dealDBRsp( const DBResponse& rsp )
//{
//    switch( rsp.head().type() )
//    {
//        case DBReqType_QueryAccount:
//            dealQueryAccount( rsp );
//            break;
//        case DBReqType_ModAccount:
//            break;
//        case DBReqType_QueryRole:
//            break;
//        case DBReqType_AddRole:
//            break;
//        default:
//            break;
//    }
//    
//}

void GameLoop::dealQueryAccount( int sockID, const string& strPasswd, const DBResponse& rsp )
{
    if( rsp.head().res() == DBErr_Fail )
    {
        return;
    }
    
    if( rsp.head().res() == DBErr_NotExist )
    {
        ResponseLogin outMsg;
        NetSendHelper::addTcpQueue( sockID, MsgType_Login, MsgErr_NotExist, outMsg);
    }
    
    DBRspAccout query;
    if( !query.ParseFromString( rsp.payload()))
    {
        SPDLOG_WARN("DBRspQueryAccount parse Fail");
        return;
    }
    
    if( query.roleid() == 0 )
    {
        //send message to db to create a new role for this account
        auto req = std::make_unique<DBRequest>();
        req->mutable_head()->set_type( DBReqType_AddRole );
        
        DBReqAddRole addRole;
        addRole.set_roleid( m_idGen.getNextID() );
        addRole.set_name( query.account() );

        req->set_payload( addRole.SerializeAsString() );
        
        addDBQuery( std::move(req) , [sockID, this](const DBResponse& rsp ){
            dealAddRole( sockID ,  rsp );
        });
        
    }
    else{
        if( strPasswd != query.passwd() )
        {
            SPDLOG_INFO("Password mismatch! {}_{}", strPasswd, query.passwd());
            
            ResponseLogin outMsg;
            NetSendHelper::addTcpQueue( sockID, MsgType_Login, MsgErr_PasswdWrong, outMsg);
            return;
        }
    
        //send message to db to query role info
        auto req = std::make_unique<DBRequest>();
        req->mutable_head()->set_type( DBReqType_QueryRole );
        
        DBReqQueryRole queryRole;
        queryRole.set_roleid( query.roleid() );

        req->set_payload( queryRole.SerializeAsString() );
        
        addDBQuery( std::move(req) , [sockID, this](const DBResponse& rsp ){
            dealQueryRole( sockID ,  rsp );
        });
        
    }
    
    return ;
}

void GameLoop::heartBeatCheck()
{
    static TimePoint lastUpdateTime = m_timeService.getCurTime();
    auto curTime = m_timeService.getCurTime();
    
    auto duration = curTime - lastUpdateTime;
    if( duration < std::chrono::seconds( m_heartbeatCheckInterval ) )
    {
        return;
    }
    
    auto vecSocks = m_sessionMgr.getSockIDs();
    for( auto sockID : vecSocks )
    {
        uint64_t roleID = 0;
        bool bNeedToRemove = false;
        auto optSession = m_sessionMgr.getSessionInfo( sockID );
        if( !optSession.has_value())
        {
            SPDLOG_DEBUG("Disconnect detected by heartbeat, Invalid sock?:s_{}", sockID);
            bNeedToRemove = true;
        }
        else {
            SessionInfo& sessInfo = optSession.value();
            roleID = sessInfo.m_roleID;
            if( curTime - sessInfo.m_lastHeartbeatTime >= std::chrono::seconds( m_heartbeatDisconnectInterval ) )
            {
                SPDLOG_DEBUG("Disconnect detected by heartbeat:s_{},r_{}", sockID, roleID);
                bNeedToRemove = true;
            }
            else{
                bNeedToRemove = false;
            }
            
        }
        
        if( bNeedToRemove )
        {
            m_sessionMgr.removeSession( sockID );
            
            if( roleID != 0 )
            {
                m_playerMgr.removePlayer( roleID );
            }
            
            INetworkMgr::getInstance()->closeSock( sockID );
        }
        
    }
}
void GameLoop::dealAddRole( int sockID, const DBResponse& rsp )
{
    if( rsp.head().res() == DBErr_Fail )
    {
        return;
    }
    
    if( rsp.head().res() == DBErr_NotExist )
    {
        ResponseLogin outMsg;
        NetSendHelper::addTcpQueue( sockID, MsgType_Login, MsgErr_NotExist, outMsg);
    }
    
    DBRspRole query;
    if( !query.ParseFromString( rsp.payload()))
    {
        SPDLOG_WARN("DBRspQueryAccount parse Fail");
        return;
    }
    
    m_playerMgr.addPlayer( sockID,  query.name() ,  query.roleid(),  query.level() );
    
    m_sessionMgr.addSessionInfo( sockID,  query.roleid(), m_timeService.getCurTime());
    
    ResponseLogin rspLogin;
    rspLogin.mutable_roleinfo()->set_roleid( query.roleid() );
    rspLogin.mutable_roleinfo()->set_rolelevel( query.level() );
    rspLogin.mutable_configinfo()->set_heartbeatsendinterval( m_heartbeatSendInterval );
    NetSendHelper::addTcpQueue( sockID, MsgType_Login, MsgErr_OK, rspLogin);
    
    EventLogs::getInstance()->onEventLogin( MsgErr_OK, query.roleid());
    
}

void GameLoop::dealQueryRole( int sockID, const DBResponse& rsp  )
{
    SPDLOG_DEBUG("Enter: sock:{}", sockID );
    if( rsp.head().res() == DBErr_Fail )
    {
        return;
    }
    
    if( rsp.head().res() == DBErr_NotExist )
    {
        ResponseLogin outMsg;
        NetSendHelper::addTcpQueue( sockID, MsgType_Login, MsgErr_NotExist, outMsg);
    }
    
    DBRspRole query;
    if( !query.ParseFromString( rsp.payload()))
    {
        SPDLOG_WARN("DBRspQueryAccount parse Fail");
        return;
    }
    
    m_playerMgr.addPlayer( sockID,  query.name() ,  query.roleid(),  query.level() );
    m_sessionMgr.addSessionInfo( sockID,  query.roleid(), m_timeService.getCurTime() );
    
    ResponseLogin rspLogin;
    rspLogin.mutable_roleinfo()->set_roleid( query.roleid() );
    rspLogin.mutable_roleinfo()->set_rolelevel( query.level() );
    rspLogin.mutable_configinfo()->set_heartbeatsendinterval( m_heartbeatSendInterval );
    NetSendHelper::addTcpQueue( sockID, MsgType_Login, MsgErr_OK, rspLogin);
    
    EventLogs::getInstance()->onEventLogin(MsgErr_OK,  query.roleid());
    
}


void GameLoop::onReceiveMsg( int sockID, const Msg& packet )
{
    m_recvMsgs.push( make_pair( sockID, packet ));
}

void GameLoop::dealReceiveMsg( int sockID, const Msg& packet )
{
    switch( packet.head().type() )
    {
        case MsgType_Login:
            dealLogin( sockID, packet );
            break;
        case MsgType_Act:
            dealAction( sockID, packet );
            break;
        case MsgType_Logout:
            dealLogout( sockID, packet );
            break;
        case MsgType_HeartBeat:
            dealHeartBeat( sockID, packet );
            break;
        default:
            break;
    }
}

void GameLoop::dealLogin( int sockID, const Msg& msg )
{
    RequestLogin login;
    if( !msg.payload().UnpackTo( &login )  )
    {
        SPDLOG_WARN("Parse Login fail");
        return;
    }
    
    SPDLOG_INFO("Receive Login {} _ {}", login.strname(), login.strpass());
    
    auto req = std::make_unique<DBRequest>();
    req->mutable_head()->set_type( DBReqType_QueryAccount );
    DBReqQueryAccount account;
    account.set_account( login.strname() );
    string strData = account.SerializeAsString();
    req->set_payload( strData );
    
    addDBQuery( std::move(req) , [sockID, login, this](const DBResponse& rsp ){
        dealQueryAccount( sockID, login.strpass(),  rsp );
    });
}

void GameLoop::dealAction( int sockID, const Msg& msg )
{
    RequestAct act;
    if( !msg.payload().UnpackTo( &act ) )
    {
        SPDLOG_WARN("Parse act fail");
        return;
    }

    SPDLOG_DEBUG("Receive Act {}", act.action());
    
    if( !m_playerMgr.isPlayerOnline( m_sessionMgr.getRoleIDFromSockID( sockID ) ))
    {
        SPDLOG_WARN("This player is not online");
        return;
    }
    
    
    ResponseAct rsp;
    rsp.set_action( act.action() );
    
    NetSendHelper::addTcpQueue( sockID, MsgType_Act, MsgErr_OK, rsp);
    
}

void GameLoop::dealLogout( int sockID, const Msg& msg )
{
    RequestLogout logout;
    if( !msg.payload().UnpackTo( &logout ) )
    {
        SPDLOG_WARN("Parse Logout fail");
        return;
    }
    
    SPDLOG_INFO("Receive Logout {}, sock:{}", logout.roleid(), sockID );
    
    auto roleID = logout.roleid() ;
    if( !m_sessionMgr.isMatchSockAndRole(sockID, roleID ) )
    {
        SPDLOG_ERROR("Mismatch sockID roleID:{},{}", sockID, roleID );
        return;
    }

    m_playerMgr.removePlayer( roleID );
    
    ResponseLogout rsp;
    rsp.set_roleid( roleID );
    
    NetSendHelper::addTcpQueue( sockID,MsgType_Logout, MsgErr_OK,  rsp);
    EventLogs::getInstance()->onEventLogout( MsgErr_OK, roleID );
    
    m_sessionMgr.removeSession( sockID );

    INetworkMgr::getInstance()->closeSock( sockID );
}

void GameLoop::dealHeartBeat( int sockID, const Msg& msg )
{
    RequestHeartBeat heartbeat;
    if( !msg.payload().UnpackTo( &heartbeat ) )
    {
        SPDLOG_WARN("Parse Logout fail");
        return;
    }
    
    m_sessionMgr.refreshHeartbeat( sockID,  m_timeService.getCurTime() );
    
    SPDLOG_TRACE("Receive HeartBeat roleid:{}, sock:{}", heartbeat.roleid(), sockID );
}

void GameLoop::sendEvent( std::unique_ptr<Event>&& evt )
{
    m_innerEvts.push( std::move(evt) );
}

void GameLoop::dealRecvEvent( Event& evt )
{
    switch( evt.m_type )
    {
        case EventType::Evt_Connect:
            dealEvtConnect( evt );
            break;
        case EventType::Evt_Disconnect:
            dealEvtDisconnect( evt );
            break;
        default:
            SPDLOG_ERROR("Invalid type:{}", (int)evt.m_type);
            break;
    }
}

void GameLoop::dealEvtConnect( Event& evt )
{
    EventConnect& evtConnect = static_cast<EventConnect&>(evt);
    //there is nothing todo currently...
}

void GameLoop::dealEvtDisconnect( Event& evt )
{
    EventDisconnect& evtDisconnect = static_cast<EventDisconnect&>(evt);
    //there should remove the player
    
    int64_t roleID = m_sessionMgr.getRoleIDFromSockID( evtDisconnect.m_sockID );
    if( roleID != 0 )
    {
        m_playerMgr.removePlayer( roleID );
    }
    m_sessionMgr.removeSession( evtDisconnect.m_sockID );
}

GameLoop::~GameLoop()
{
    INetworkMgr::getInstance()->shutdownNetwork();
    // m_db.shutdown();
    
    SPDLOG_INFO("GameLoop Shutdown!");
}
