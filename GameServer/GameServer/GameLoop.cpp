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

#include "Utils/LoggerHelper.hpp"

#include "proto/msg.pb.h"
using namespace MyGame;

const unsigned short SVR_PORT_DEF = 8081;
const string DB_HOST_DEF = "tcp://127.0.0.1:3306";
const string DB_USER_DEF = "admin";
const string DB_PASSWD_DEF = "111111";
const string DB_NAME_DEF = "MyGame";

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
    m_db.registerRspHandle(
                           std::bind( &GameLoop::onReceiveDBRsp, this ,
                                     std::placeholders::_1,
                                     std::placeholders::_2));

    
    return ret;
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
    
    while(true)
    {
        std::this_thread::sleep_for( 20ms );
        update();
    }
    
    SPDLOG_INFO("Stop Run!");
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
    SPDLOG_INFO("onDisconnect fd:{}", fd );
    
}

void GameLoop::onConnect( const TcpSocket& sock )
{
    SPDLOG_INFO("onConnect fd:{}", sock.m_sock );
    
}

void GameLoop::update()
{
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
        auto rsp = itdb->second;
        auto itCb = m_mapDBRspFuns.find( queryID );
        if( itCb != m_mapDBRspFuns.end())
        {
            auto callback = itCb->second;
            callback( rsp );
            
            m_mapDBRspFuns.erase( itCb );
        }
        
        itdb = m_dbmsgRsp.try_pop();
    }
    
    //todo update gamelogic
}

void GameLoop::addDBQuery( const DBRequest& req, std::function<void( const DBResponse&)> func )
{
    int queryID = m_db.getNextID();
    m_db.addDBQuery( queryID, req );
    m_mapDBRspFuns[queryID] = func;
    SPDLOG_DEBUG("addDBQuery:{}", (int)req.head().type() );
}

void GameLoop::onReceiveDBRsp( int queryID,  const DBResponse& rsp )
{
    SPDLOG_DEBUG("onReceiveDBRsp:{0},{1}", (int)rsp.head().type(), queryID );
    m_dbmsgRsp.push( make_pair( queryID, rsp ) );
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
        DBRequest req;
        req.mutable_head()->set_type( DBReqType_AddRole );
        
        DBReqAddRole addRole;
        addRole.set_name( query.account() );

        req.set_payload( addRole.SerializeAsString() );
        
        addDBQuery( req , [sockID, this](const DBResponse& rsp ){
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
        DBRequest req;
        req.mutable_head()->set_type( DBReqType_QueryRole );
        
        DBReqQueryRole queryRole;
        queryRole.set_roleid( query.roleid() );

        req.set_payload( queryRole.SerializeAsString() );
        
        addDBQuery( req , [sockID, this](const DBResponse& rsp ){
            dealQueryRole( sockID ,  rsp );
        });
        
    }
    
    return ;
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
    
    ResponseLogin rspLogin;
    rspLogin.set_roleid( query.roleid() );
    rspLogin.set_rolelevel( query.level() );
    NetSendHelper::addTcpQueue( sockID, MsgType_Login, MsgErr_OK, rspLogin);
    
    
    
}

void GameLoop::dealQueryRole( int sockID, const DBResponse& rsp  )
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
    
    ResponseLogin rspLogin;
    rspLogin.set_roleid( query.roleid() );
    rspLogin.set_rolelevel( query.level() );
    NetSendHelper::addTcpQueue( sockID, MsgType_Login, MsgErr_OK, rspLogin);
    
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
    
//    m_playerMgr.onPlayerLogin( sockID, login.strname(), login.strpass() );
    DBRequest req;
    req.mutable_head()->set_type( DBReqType_QueryAccount );
    DBReqQueryAccount account;
    account.set_account( login.strname() );
    string strData = account.SerializeAsString();
    req.set_payload( strData );
    
    addDBQuery( req , [sockID, login, this](const DBResponse& rsp ){
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
    
    if( !m_playerMgr.isPlayerOnline( m_playerMgr.getPlayerIDFromSock( sockID ) ))
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
    
    m_playerMgr.onPlayerLogout( sockID, logout.roleid() );
}

GameLoop::~GameLoop()
{
    INetworkMgr::getInstance()->shutdownNetwork();
    // m_db.shutdown();
    
    SPDLOG_INFO("GameLoop Shutdown!");
}
