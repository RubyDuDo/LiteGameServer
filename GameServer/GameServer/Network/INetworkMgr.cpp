//
//  INetworkMgr.cpp
//  GameServer
//
//  Created by pinky on 2025-03-31.
//

#include "INetworkMgr.hpp"
#include <thread>
#include <iostream>
#include "NetworkMgrSelect.hpp"
#ifdef __LINUX__
#include "NetworkMgrEpoll.hpp"
#endif
using namespace std;

#include <algorithm>

#include <cerrno>
#include <cstring>

constexpr int RECV_BUFF = 1500;

unique_ptr<INetworkMgr> INetworkMgr::m_pInstance = nullptr;

INetworkMgr* INetworkMgr::getInstance()
{
    if( m_pInstance == nullptr )
    {
        m_pInstance = INetworkMgrFactory::createNetworkMgr();
    }
    return m_pInstance.get();
}


std::unique_ptr<INetworkMgr> INetworkMgrFactory::createNetworkMgr()
{
#ifdef __LINUX__
    return make_unique<NetworkMgrEpoll>();
#else
    return make_unique<NetworkMgrSelect>();
#endif
}


bool INetworkMgr::initNetwork( unsigned short svr_port )
{
    m_listenSock = NetUtil::createTcpSocket();
    m_listenSock->setReuseAddr(true);
    cout<<"setReuseAddr"<<endl;
    m_listenSock->setNonBlock( true );
    
    m_listenSock->Bind( svr_port );
    
    m_listenSock->Listen();
    
    bool bRet = innerInit();
    if( bRet == false )
    {
        cout<<"init network fail!"<<endl;
        return false;
    }

    m_bRunning = true;
    
    m_runThread.emplace(&INetworkMgr::networkThread, this);
    
    return true;
    
}

void INetworkMgr::shutdownNetwork()
{
    innerShutdown();

    m_bRunning = false;
    if( m_runThread && m_runThread->joinable() )
    {
        m_runThread->join();
    }
}

void INetworkMgr::registerHandler(INetHandler* handler)
{
    m_netHandler = handler;
}

void INetworkMgr::networkThread()
{
    innerRun();
}

void INetworkMgr::sendMsg( int fd, const std::string& msg )
{
    cout<<"sendMsg:"<<fd<<":"<<msg.size()<<endl;
    m_msgQueue.push( make_pair( fd, msg ));
    innerSendMsg( fd, msg );
}

INetworkMgr::~INetworkMgr()
{
    
}

bool INetworkMgr::onReceiveMsg( std::shared_ptr<TcpSocket> sock )
{
    cout<<"onReceiveMsg:"<<sock->m_sock<<endl;
    NetSlot& slot = m_mapSlot[sock->m_sock];
    //do receive
    char buff[RECV_BUFF]{};
    int ret = sock->RecvData( buff, RECV_BUFF );
    cout<<"receiveData:"<<ret<<": from :"<<sock->m_sock<<endl;
    bool bClose = false;
    while( true )
    {
        if( ret < 0 )
        {
            int err = errno;
            if( err == EAGAIN || err == EWOULDBLOCK )
            {
                //no more data
            }
            else if( err == EINTR )
            {
                //interrupted, just try again in next loop
            }
            else if( err == ECONNRESET )
            {
                //client close
                bClose = true;
                cout<<"client close:"<<sock->m_sock<<endl;
            }
            else
            {
                //error
                bClose = true;
                cout<<"recv error:"<<strerror(err)<<endl;
            }
            //error
            break;
        }
        else if( ret == 0 )
        {
            // client close
            bClose = true;
            break;
        }
        else
        {
            slot.m_recvBuff.addData( buff, ret );
            if( ret < RECV_BUFF )
            {
                break;
            }
            else
            {
                ret = sock->RecvData( buff, RECV_BUFF );
            }
        }
    }

    cout<<"onReceiveMsg, Send to Slot:"<<sock->m_sock<<endl;

    do{
        auto pMsg = slot.getNextRecvMsg();
        if( pMsg )
        {
            onReceiveMsgInner( sock->m_sock, *pMsg );
            
            if( m_netHandler )
            {
                m_netHandler->onReceiveMsg( sock->m_sock, *pMsg );
            }
        }
        else{
            break;
        }
    }while( true );

    cout<<"onReceiveMsg, End:"<<sock->m_sock<<endl;

    if( bClose )
    {
        onDisconnect( sock->m_sock );
    }

    return bClose;
}

void INetworkMgr::onDisconnect( int fd )
{
    cout<<"onDisconnect:"<<fd<<endl;
    if( m_mapSocks.find( fd ) == m_mapSocks.end() )
    {
        return;
    }

    onDisconnectInner(fd);

    m_mapSlot.erase( fd );
    m_mapSocks.erase( fd );

    auto it = std::find_if( m_setSocks.begin(), m_setSocks.end(), [fd]( const TcpSocketPtr& sock ) {
        return sock->m_sock == fd;
    });
    if( it != m_setSocks.end() )
    {
        m_setSocks.erase( it );
    }
    
    if( m_netHandler )
    {
        m_netHandler->onDisconnect( fd );
    }
}

void INetworkMgr::onConnect( shared_ptr<TcpSocket> sock )
{
    cout<<"onConnect:"<<sock->m_sock<<endl;
    sock->setNonBlock( true );
    m_mapSocks[sock->m_sock] = sock;
    m_mapSlot[ sock->m_sock ] = NetSlot();
    
    onConnectInner( sock );
    if( m_netHandler )
    {
        m_netHandler->onConnect( *sock );
    }
}

void INetworkMgr::innerSendMsg( int fd, const std::string& msg )
{

}

