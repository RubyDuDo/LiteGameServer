#ifndef NetworkMgrEpoll_hpp
#define NetworkMgrEpoll_hpp

#include <stdio.h>
#include "INetworkMgr.hpp"




class NetworkMgrEpoll : public INetworkMgr
{
public:
    NetworkMgrEpoll() = default;
    
    virtual ~NetworkMgrEpoll();
    
private:
    virtual void innerShutdown() ;
    virtual bool innerInit();
    virtual void innerRun();
    
    virtual void onReceiveMsgInner( int fd, const std::string& msg );
    virtual void onDisconnectInner( int fd );
    virtual void onConnectInner( shared_ptr<TcpSocket> sock  );

    virtual void innerSendMsg( int fd, const std::string& msg ) ;

    void onNewSendMsg();

    void onSockEvent( const struct epoll_event& ev );

    void handleSendMsg( TcpSocket& sock );
    
    
private:
    void clearInvalidSock();
    
private:
    int m_epollFd = -1;
    int m_eventFd = -1;

    
};

#endif /* NetworkMgrEpoll_hpp */