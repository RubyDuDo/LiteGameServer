//
//  NetworkMgr.cpp
//  GameServer
//
//  Created by pinky on 2025-02-19.
//

#include "NetworkMgr.hpp"
#include <thread>
#include <iostream>

NetworkMgr* NetworkMgr::m_pMgr = nullptr;

NetworkMgr* NetworkMgr::getInstance()
{
    if( !m_pMgr )
    {
        m_pMgr = new NetworkMgr();
    }
    return m_pMgr;
}

bool NetworkMgr::InitNetwork()
{
    m_listenSock = NetUtil::createTcpSocket();
    
    m_listenSock->Bind( SVR_PORT );
    
    m_listenSock->Listen();
    
    std::thread th(&NetworkMgr::networkThread, this );
    th.detach();
    
    return true;
}

void NetworkMgr::networkThread()
{
    std::cout<<"network Thread started!"<<std::endl;
    while( true )
    {
        auto playersock = m_listenSock->Accept();
        if( playersock )
        {
            auto playerThread = std::thread(&NetworkMgr::playerThread, this, playersock);
            playerThread.detach();
        }
    }
    
}

constexpr int RECV_BUFF = 1500;
void NetworkMgr::playerThread( std::shared_ptr<TcpSocket> sock )
{
    while( true )
    {
        char recvBuff[RECV_BUFF]{};
        int ret = sock->RecvData( recvBuff, RECV_BUFF );
        if( ret == 0 )
        {
            break;
        }
        
        std::string msg( recvBuff );
        std::cout<<"receive Msg :"<<msg<<std::endl;
        
        int sendLen = msg.length() + 1;
        int sendedLen = 0;
        
        while( sendedLen != sendLen )
        {
            ret = sock->SendData( msg.c_str(),  sendLen  );
            if( ret < 0 )
            {
                break;
            }
            else{
                sendedLen += ret;
            }
            
        }
    }
}
