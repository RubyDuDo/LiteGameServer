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

#include "NetworkMgr.hpp"
#include "proto/msg.pb.h"
using namespace MyGame;

constexpr unsigned short SVR_PORT_DEF = 8081;
constexpr string DB_HOST_DEF = "tcp://127.0.0.1:3306";
constexpr string DB_USER_DEF = "admin";
constexpr string DB_PASSWD_DEF = "111111";
constexpr string DB_NAME_DEF = "MyGame";

constexpr string config_dir = "./config.ini";
bool GameLoop::Init()
{
    int res = m_config.ParseFile( config_dir );
    
    unsigned short port = (unsigned short)m_config.getInt( "Network" , "port", SVR_PORT_DEF );
    bool ret = NetworkMgr::getInstance()->InitNetwork( port );
    if( ret )
    {
        NetworkMgr::getInstance()->registerReceiveMsgHandle(
                                                            std::bind( &GameLoop::onReceiveMsg, this,
                                                                      std::placeholders::_1,
                                                                      std::placeholders::_2)
                                                            );
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
        cout<<"Init Failed!"<<endl;
        return ret;
    }
    else{
        cout<<"Init succeed!"<<endl;
    }
    
    
    
    cout<<"Run ... "<<endl;
    
    while(true)
    {
        std::this_thread::sleep_for( 20ms );
        update();
    }
    
    //
    return true;
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
    cout<<"addDBQuery:"<<req.head().type()<<endl;
}

void GameLoop::onReceiveDBRsp( int queryID,  const DBResponse& rsp )
{
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
        NetworkMgr::getInstance()->addTcpQueue( sockID, MsgType_Login, MsgErr_NotExist, outMsg);
    }
    
    DBRspAccout query;
    if( !query.ParseFromString( rsp.payload()))
    {
        cerr<<"DBRspQueryAccount parse Fail"<<endl;
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
            cout<<"Password mismatch!"<<strPasswd<<"_"<<query.passwd()<<endl;
            //notify client fail
            
            ResponseLogin outMsg;
            NetworkMgr::getInstance()->addTcpQueue( sockID, MsgType_Login, MsgErr_PasswdWrong, outMsg);
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
        NetworkMgr::getInstance()->addTcpQueue( sockID, MsgType_Login, MsgErr_NotExist, outMsg);
    }
    
    DBRspRole query;
    if( !query.ParseFromString( rsp.payload()))
    {
        cerr<<"DBRspQueryAccount parse Fail"<<endl;
        return;
    }
    
    m_playerMgr.addPlayer( sockID,  query.name() ,  query.roleid(),  query.level() );
    
    ResponseLogin rspLogin;
    rspLogin.set_roleid( query.roleid() );
    rspLogin.set_rolelevel( query.level() );
    NetworkMgr::getInstance()->addTcpQueue( sockID, MsgType_Login, MsgErr_OK, rspLogin);
    
    
    
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
        NetworkMgr::getInstance()->addTcpQueue( sockID, MsgType_Login, MsgErr_NotExist, outMsg);
    }
    
    DBRspRole query;
    if( !query.ParseFromString( rsp.payload()))
    {
        cerr<<"DBRspQueryAccount parse Fail"<<endl;
        return;
    }
    
    m_playerMgr.addPlayer( sockID,  query.name() ,  query.roleid(),  query.level() );
    
    ResponseLogin rspLogin;
    rspLogin.set_roleid( query.roleid() );
    rspLogin.set_rolelevel( query.level() );
    NetworkMgr::getInstance()->addTcpQueue( sockID, MsgType_Login, MsgErr_OK, rspLogin);
    
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
        cout<<"Parse Login fail"<<endl;
        return;
    }
    
    cout<<"Login From:"<< login.strname()<<" _ "<<login.strpass()<<endl;
    
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
        cout<<"Parse act fail"<<endl;
        return;
    }
    cout<<"Receive Act "<< act.action()<<endl;
    
    if( !m_playerMgr.isPlayerOnline( m_playerMgr.getPlayerIDFromSock( sockID ) ))
    {
        cout<<"This player is not online"<<endl;
        return;
    }
    
    
    ResponseAct rsp;
    rsp.set_action( act.action() );
    
    NetworkMgr::getInstance()->addTcpQueue( sockID, MsgType_Act, MsgErr_OK, rsp);
    
}

void GameLoop::dealLogout( int sockID, const Msg& msg )
{
    RequestLogout logout;
    if( !msg.payload().UnpackTo( &logout ) )
    {
        cout<<"Parse act fail"<<endl;
        return;
    }
    cout<<"Receive Logout "<< logout.roleid()<<"_ sock:"<<sockID <<endl;
    
    m_playerMgr.onPlayerLogout( sockID, logout.roleid() );
}
