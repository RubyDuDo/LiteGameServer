//
//  NetworkMgr.hpp
//  GameServer
//
//  Created by pinky on 2025-02-19.
//

#ifndef NetworkMgr_hpp
#define NetworkMgr_hpp

#include <stdio.h>
#include "TcpSocket.hpp"

constexpr short SVR_PORT = 8081;

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
};

#endif /* NetworkMgr_hpp */
