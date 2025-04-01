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
using namespace std;

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
    //todo
    return make_unique<NetworkMgrSelect>();
}


bool INetworkMgr::initNetwork( unsigned short svr_port )
{
    m_listenSock = NetUtil::createTcpSocket();
    m_listenSock->setReuseAddr(true);
    cout<<"setReuseAddr"<<endl;
    m_listenSock->setNonBlock( true );
    
    m_listenSock->Bind( svr_port );
    
    m_listenSock->Listen();
    
    innerInit();
    
    std::thread th(&INetworkMgr::networkThread, this );
    th.detach();
    
    return true;
    
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
    m_msgQueue.push( make_pair( fd, msg ));
}

INetworkMgr::~INetworkMgr()
{
    
}

int INetworkMgr::onReceiveMsg( std::shared_ptr<TcpSocket> sock )
{
    //do receive
    char buff[RECV_BUFF]{};
    int ret = sock->RecvData( buff, RECV_BUFF );
    cout<<"receiveData:"<<ret<<": from :"<<sock->m_sock<<endl;
    if( ret == 0 )
    {
        return ret;
    }
    else if( ret > 0 )
    {
        NetSlot& slot = m_mapSlot[sock->m_sock];
        slot.m_recvBuff.addData( buff, ret );
        
        auto pMsg = slot.getNextRecvMsg();
        if( pMsg )
        {
            onReceiveMsgInner( sock->m_sock, *pMsg );
            
            if( m_netHandler )
            {
                m_netHandler->onReceiveMsg( sock->m_sock, *pMsg );
            }
        }
    }

    return ret;
}

void INetworkMgr::onDisconnect( int fd )
{
    onDisconnectInner(fd);
    
    if( m_netHandler )
    {
        m_netHandler->onDisconnect( fd );
    }
}

void INetworkMgr::onConnect( shared_ptr<TcpSocket> sock )
{
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

