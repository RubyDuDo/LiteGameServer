//
//  NetworkMgr.cpp
//  GameServer
//
//  Created by pinky on 2025-02-19.
//

#include "NetworkMgr.hpp"
#include <thread>
#include <iostream>
#include "Buffer.hpp"

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
void NetworkMgr::onReceiveMsg( std::shared_ptr<TcpSocket> sock, const Msg& msg )
{
    Buffer sendBuff;
    std::cout<<"onReceiveMsg:"<< msg.m_strAction.length() <<":"<<msg.m_strAction<<std::endl;
    short sendLen = msg.m_strAction.length() + 1;
    
    short len = htons( sendLen );
    sendBuff.addData( (char*)&len, sizeof(short) );
    sendBuff.addData( (char*)msg.m_strAction.c_str(), sendLen );
    //echo back
    
    char buff[RECV_BUFF]{};
    sendLen = sendBuff.getData( buff ,  RECV_BUFF);
    short sendedLen = 0;
    
    std::cout<<"Echo back:"<< sendLen<<std::endl;
    
    while( sendedLen != sendLen )
    {
        int ret = sock->SendData( buff + sendedLen,  sendLen - sendedLen  );
        if( ret < 0 )
        {
            break;
        }
        else{
            sendedLen += ret;
        }
    }
}

#include <chrono>
using namespace std::chrono_literals;

void NetworkMgr::playerThread( std::shared_ptr<TcpSocket> sock )
{
    Buffer recvBuff;
    while( true )
    {
        std::this_thread::sleep_for( 20ms );
        char buff[RECV_BUFF]{};
        int ret = sock->RecvData( buff, RECV_BUFF );
        if( ret == 0 )
        {
            break;
        }
        else if( ret > 0 )
        {
            recvBuff.addData( buff, ret );
        }
        
        //TCP拆包
        if( recvBuff.getSize() > sizeof( short ) )
        {
            char msgbuff[RECV_BUFF]{};
            short len = 0;
            recvBuff.getData( (char*)&len, sizeof(short));
            len = ntohs( len );
            if( recvBuff.getSize() >= len + sizeof(short) )
            {
                recvBuff.consumeData( sizeof( short ) );
                recvBuff.getData( msgbuff,  RECV_BUFF );
                recvBuff.consumeData( len );
                
                std::string msg( msgbuff );
                
                
                onReceiveMsg( sock, Msg(msg) );
            }
        }
        
    }
}
