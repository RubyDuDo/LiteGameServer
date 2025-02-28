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

bool GameLoop::Init()
{
    bool ret = NetworkMgr::getInstance()->InitNetwork();
    if( ret )
    {
        NetworkMgr::getInstance()->registerReceiveMsgHandle(
                                                            std::bind( &GameLoop::onReceiveMsg, this,
                                                                      std::placeholders::_1,
                                                                      std::placeholders::_2)
                                                            );
    }
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
    }
    
    //
    return true;
}

void GameLoop::onReceiveMsg( int sockID, const Msg& packet )
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
    cout<<" Get RoleID:"<<m_nextRoleID<<endl;
    
    ResponseLogin rsp;
    rsp.set_roleid( m_nextRoleID );
    m_nextRoleID++;
    
    Msg outMsg;
    outMsg.set_allocated_head( ProtobufHelp::CreatePacketHead( MsgType_Login));
    outMsg.mutable_payload()->PackFrom( rsp );
    
    NetworkMgr::getInstance()->addTcpQueue( sockID, outMsg);
    
    
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
    
    ResponseAct rsp;
    rsp.set_action( act.action() );
    
    Msg outMsg;
    outMsg.set_allocated_head( ProtobufHelp::CreatePacketHead( MsgType_Act));
    outMsg.mutable_payload()->PackFrom( rsp );
    
    NetworkMgr::getInstance()->addTcpQueue( sockID, outMsg);
    
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
    
    ResponseLogout rsp;
    rsp.set_roleid( logout.roleid() );
    
    Msg outMsg;
    outMsg.set_allocated_head( ProtobufHelp::CreatePacketHead( MsgType_Logout));
    outMsg.mutable_payload()->PackFrom( rsp );
    
    NetworkMgr::getInstance()->addTcpQueue( sockID, outMsg);
}
