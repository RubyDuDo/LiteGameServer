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
#include "../Utils/LoggerHelper.hpp"

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
    if( m_listenSock == nullptr )
    {
        SPDLOG_ERROR("createTcpSocket fail");
        return false;
    }
    m_listenSock->setReuseAddr(true);
    m_listenSock->setNonBlock( true );
    
    int ret = m_listenSock->Bind( svr_port );
    if( ret < 0 )
    {
        SPDLOG_ERROR("Bind Error");
        return false;
    }
    
    ret = m_listenSock->Listen();
    if( ret < 0 )
    {
        SPDLOG_ERROR("Listen Error!");
        return false;
    }
    
    bool bRet = innerInit();
    if( bRet == false )
    {
        SPDLOG_ERROR("innerInit network fail!");
        return false;
    }

    m_bRunning = true;
    
    m_runThread.emplace(&INetworkMgr::networkThread, this);
    
    return true;
    
}

void INetworkMgr::shutdownNetwork()
{
    m_bRunning = false;
    if( m_runThread && m_runThread->joinable() )
    {
        m_runThread->join();
    }
    
    //clear all the resources should be done after the thread exit
    //in order to avoid the resource is still in use
    //also avoid the resources are used in multi threads
    innerShutdown();
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
    SPDLOG_TRACE("sendMsg: fd:{}, size:{}", fd, msg.size());
    m_msgQueue.push( make_pair( fd, msg ));
    innerSendMsg( fd, msg );
}

INetworkMgr::~INetworkMgr()
{
    
}

bool INetworkMgr::onReceiveMsg( std::shared_ptr<TcpSocket> sock )
{
    auto it = m_mapSocks.find( sock->m_sock );
    if( it == m_mapSocks.end() )
    {
        SPDLOG_ERROR("onReceiveMsg, sock not found:{}", sock->m_sock);
        return false;
    }
    NetSlot& slot = it->second;
    //do receive
    char buff[RECV_BUFF]{};
    int ret = sock->RecvData( buff, RECV_BUFF );
    SPDLOG_TRACE("receiveData: ret_{}, fd_{}", ret, sock->m_sock);
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
                SPDLOG_DEBUG("client close:{}", sock->m_sock);
            }
            else
            {
                //error
                bClose = true;
                SPDLOG_DEBUG("recv error:{}", strerror(err));
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

    if( bClose )
    {
        onDisconnect( sock->m_sock );
    }

    return bClose;
}

void INetworkMgr::onDisconnect( int fd )
{
    if( m_mapSocks.find( fd ) == m_mapSocks.end() )
    {
        return;
    }

    SPDLOG_DEBUG("onDisconnect:{}", fd);

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
    SPDLOG_DEBUG("onConnect:{}", sock->m_sock);
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

