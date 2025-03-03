//
//  NetworkMgr.hpp
//  GameServer
//
//  Created by pinky on 2025-02-19.
//

#ifndef NetworkMgr_hpp
#define NetworkMgr_hpp

#include <stdio.h>
#include <string>
#include "TcpSocket.hpp"
#include <map>
#include "proto/msg.pb.h"
using namespace MyGame;

#include "Buffer.hpp"

class GameLoop;

class ProtobufHelp
{
public:
    static MsgHead* CreatePacketHead( MsgType type );
    static MsgRspHead* CreateRspHead( MsgType type, MsgErrCode res );
};

#include <set>

class Slot
{
public:
    Slot():m_recvBuff(), m_sendBuff(){};
    
    void sendMsg( const MsgRsp& msg );
    std::shared_ptr<Msg> getNextRecvMsg();
public:
    Buffer m_recvBuff;
    Buffer m_sendBuff;
};

class NetworkMgr
{
private:
    NetworkMgr() = default;
    static NetworkMgr* m_pMgr;
    std::shared_ptr<TcpSocket> m_listenSock;
    std::vector< TcpSocketPtr > m_setSocks;
    
    std::map<int, Slot> m_mapSlot;
    
    int m_maxFd = 0 ;
    
    std::function<void(int, const Msg&)> m_recvFun;
public:
    static NetworkMgr* getInstance();
    
    bool InitNetwork( unsigned short svr_port );
    
    void networkNonBlockThread();
    void networkThread();
    
    void playerThread( std::shared_ptr<TcpSocket> );
    
    void onReceiveMsg( std::shared_ptr<TcpSocket> sock, const Msg& packet );
    
    void registerReceiveMsgHandle( std::function<void( int, const Msg&)> recvFun);
    
    void addTcpQueue( int sockID,  const MsgRsp& packet );
    
    template< typename T>
    void addTcpQueue( int sockID,  MsgType type, MsgErrCode res,  const T& rsp );

    void clearInvalidSock();
    
};

template< typename T>
void NetworkMgr::addTcpQueue( int sockID, MsgType type, MsgErrCode res, const T& rsp )
{
    MsgRsp outMsg;
    outMsg.set_allocated_head( ProtobufHelp::CreateRspHead( type, res ));
    outMsg.mutable_payload()->PackFrom( rsp );
    
    addTcpQueue( sockID, outMsg );
}

#endif /* NetworkMgr_hpp */
