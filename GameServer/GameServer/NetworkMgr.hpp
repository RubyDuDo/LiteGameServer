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

constexpr short SVR_PORT = 8081;
#include "Buffer.hpp"

class Msg
{
public:
    Msg( const std::string& str ):m_strAction( str ){};
    std::string m_strAction;
    
};

#include <set>

class Slot
{
public:
    Slot():m_recvBuff(), m_sendBuff(){};
    
    void sendMsg( const Msg& msg );
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
public:
    static NetworkMgr* getInstance();
    
    bool InitNetwork();
    
    void networkNonBlockThread();
    void networkThread();
    
    void playerThread( std::shared_ptr<TcpSocket> );
    
    void onReceiveMsg( std::shared_ptr<TcpSocket> sock, const Msg& msg );

};

#endif /* NetworkMgr_hpp */
