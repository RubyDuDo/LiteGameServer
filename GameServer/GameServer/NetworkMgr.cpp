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
#include <chrono>
#include <sys/select.h>
using namespace std::chrono_literals;
using namespace std;

constexpr int RECV_BUFF = 1500;

NetworkMgr* NetworkMgr::m_pMgr = nullptr;

void Slot::sendMsg( const Msg& msg )
{
    short sendLen = msg.m_strAction.length() + 1;
    
    short len = htons( sendLen );
    m_sendBuff.addData( (char*)&len, sizeof(short) );
    m_sendBuff.addData( (char*)msg.m_strAction.c_str(), sendLen );
}

std::shared_ptr<Msg> Slot::getNextRecvMsg()
{
    //TCP拆包
    if( m_recvBuff.getSize() > sizeof( short ) )
    {
        char msgbuff[RECV_BUFF]{};
        short len = 0;
        m_recvBuff.getData( (char*)&len, sizeof(short));
        len = ntohs( len );
        if( m_recvBuff.getSize() >= len + sizeof(short) )
        {
            m_recvBuff.consumeData( sizeof( short ) );
            m_recvBuff.getData( msgbuff,  RECV_BUFF );
            m_recvBuff.consumeData( len );
            
            std::string msg( msgbuff );
            
            return std::make_shared<Msg>(msg);
        }
    }
    
    return nullptr;
}

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
    
    m_setSocks.push_back( m_listenSock );
    m_maxFd = m_listenSock->m_sock;
    
    std::thread th(&NetworkMgr::networkNonBlockThread, this );
//    std::thread th(&NetworkMgr::networkThread, this );
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

void NetworkMgr::networkNonBlockThread()
{
    std::cout<<"network Thread(Nonblock Mode) started!"<<std::endl;
    while( true )
    {
        std::this_thread::sleep_for( 10ms );
        std::vector<TcpSocketPtr> outReadSet, outWriteSet, outExceptSet;
    
        int ret = NetUtil::Select( m_maxFd, m_setSocks, outReadSet, m_setSocks, outWriteSet, m_setSocks, outExceptSet);
        if( ret < 0 )
        {
            perror("select error");
            break;
        }
        
        if( !outReadSet.empty())
        {
            for( auto it : outReadSet )
            {
                if( it == m_listenSock )
                {
                    auto sock = m_listenSock->Accept();
                    if( sock )
                    {
                        m_setSocks.push_back( sock );
                        m_mapSlot[sock->m_sock] = Slot();
                        
                        if( sock->m_sock > m_maxFd)
                        {
                            m_maxFd = sock->m_sock;
                        }
                    }
                }
                else{
                    //do receive
                    char buff[RECV_BUFF]{};
                    int ret = it->RecvData( buff, RECV_BUFF );
                    if( ret == 0 )
                    {
                        break;
                    }
                    else if( ret > 0 )
                    {
                        Slot& slot = m_mapSlot[it->m_sock];
                        slot.m_recvBuff.addData( buff, ret );
                        
                        auto pMsg = slot.getNextRecvMsg();
                        if( pMsg )
                        {
                            std::cout<<"Receive:"<<pMsg->m_strAction<<std::endl;
                            //echo back
                            slot.sendMsg( *pMsg );
                        }
                    }
                }
            }
            
        }
        
        if( !outWriteSet.empty() )
        {
            //do write
            for( auto it : outWriteSet )
            {
                if( it == m_listenSock )
                {
                    continue;
                }
                
                Slot& slot = m_mapSlot[it->m_sock];
                char buff[RECV_BUFF]{};
                int sendSize = slot.m_sendBuff.getData( buff , RECV_BUFF );
                if( sendSize > 0 )
                {
                    int sended = it->SendData( buff ,  sendSize);
                    slot.m_sendBuff.consumeData( sended );
                }
            }
        }
    }
}

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
