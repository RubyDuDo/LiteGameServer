//
//  NetworkMgrSelect.cpp
//  GameServer
//
//  Created by pinky on 2025-03-31.
//

#include "NetworkMgrSelect.hpp"
#include <iostream>
constexpr int RECV_BUFF = 1500;


void NetworkMgrSelect::shutdown()
{
    
}

void NetworkMgrSelect::innerInit()
{
    m_setSocks.push_back( m_listenSock );
    m_maxFd = m_listenSock->m_sock + 1 ;
}

void NetworkMgrSelect::onReceiveMsgInner( int fd, const std::string& msg )
{
    
}
void NetworkMgrSelect::onDisconnectInner( int fd )
{
    
}
void NetworkMgrSelect::onConnectInner( shared_ptr<TcpSocket> sock  )
{
    
    if( sock->m_sock >= m_maxFd )
    {
        m_maxFd = sock->m_sock + 1 ;
    }
}

void NetworkMgrSelect::dispatchSendMsg()
{
    while( true )
    {
        auto it = m_msgQueue.try_pop();
        if( it )
        {
            NetSlot& slot = m_mapSlot[it->first];
            slot.sendMsg( it->second );
        }
        else{
            break;
        }
    }
}

void NetworkMgrSelect::innerRun()
{
    std::cout<<"network Thread( Select: Nonblock Mode) started!"<<std::endl;
    while( true )
    {
        clearInvalidSock();
        
        dispatchSendMsg();
        std::vector<TcpSocketPtr> outReadSet, outWriteSet, outExceptSet;
        //todo ,only need to register write set for which have data to send
    
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
                        onConnect( sock );
                    }
                }
                else{
                    if( !it->isValid() )
                    {
                        continue;
                    }
                    
                    onReceiveMsg( it );
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
                
                NetSlot& slot = m_mapSlot[it->m_sock];
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

void NetworkMgrSelect::clearInvalidSock()
{
    auto it = m_setSocks.begin();
    if( it != m_setSocks.end() )
    {
        auto pSock = *it;
        if( pSock->isValid() )
        {
            it++;
        }
        else{
            it = m_setSocks.erase( it );
        }
    }
}

NetworkMgrSelect::~NetworkMgrSelect()
{
    
}

