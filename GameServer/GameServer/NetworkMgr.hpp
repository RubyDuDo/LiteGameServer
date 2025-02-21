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

constexpr short SVR_PORT = 8081;

class Msg
{
public:
    Msg( const std::string& str ):m_strAction( str ){};
    std::string m_strAction;
    
};

class NetworkMgr
{
private:
    NetworkMgr() = default;
    static NetworkMgr* m_pMgr;
    std::shared_ptr<TcpSocket> m_listenSock;
public:
    static NetworkMgr* getInstance();
    
    bool InitNetwork();
    
    void networkThread();
    
    void playerThread( std::shared_ptr<TcpSocket> );
    
    void onReceiveMsg( std::shared_ptr<TcpSocket> sock, const Msg& msg );
};

#endif /* NetworkMgr_hpp */
