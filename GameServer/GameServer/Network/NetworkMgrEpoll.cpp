#include "NetworkMgrEpoll.hpp"

#include <sys/epoll.h>  // For epoll_create1
#include <sys/eventfd.h> // For eventfd
#include <unistd.h>    // For close()
#include <stdio.h>     // For perror()
#include <stdlib.h>    // For EXIT_FAILURE
#include <iostream>

#include "../Utils/LoggerHelper.hpp"

constexpr int RECV_BUFF = 1500;
const int MAX_EVENTS = 64;

    
NetworkMgrEpoll::~NetworkMgrEpoll()
{
    shutdownNetwork();
}

void NetworkMgrEpoll::innerShutdown()
{
    if( m_epollFd != -1 )
    {
        close(m_epollFd);
        m_epollFd = -1;
    }
    if( m_eventFd!= -1  )
    {
        close(m_eventFd);
        m_eventFd = -1;
    }
}

bool NetworkMgrEpoll::innerInit()
{
    m_epollFd = epoll_create1( EPOLL_CLOEXEC );
    if (m_epollFd == -1) {
        SPDLOG_ERROR("epoll_create1 fail");
        return false;
    }

    m_eventFd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC );
    if (m_eventFd == -1) {
        SPDLOG_ERROR("eventfd create fail");
        return false;
    }

    struct epoll_event evListen;
    evListen.data.fd = m_listenSock->m_sock;
    evListen.events = EPOLLIN ;
    if (epoll_ctl(m_epollFd, EPOLL_CTL_ADD, m_listenSock->m_sock, &evListen) == -1) {
        SPDLOG_ERROR("epoll_ctl: listen_sock");
        return false;
    }

    struct epoll_event evEvent;
    evEvent.data.fd = m_eventFd;
    evEvent.events = EPOLLIN ;
    if( epoll_ctl(m_epollFd, EPOLL_CTL_ADD, m_eventFd, &evEvent) == -1 )
    {
        SPDLOG_ERROR("epoll_ctl: event_fd");
        return false;
    }

    return true;
}

void NetworkMgrEpoll::innerRun()
{
    SPDLOG_INFO("Enter");
    
    std::vector<struct epoll_event> events(MAX_EVENTS);
    while( m_bRunning )
    {
        
        events.clear();
        int nfds = epoll_wait(m_epollFd, events.data(), MAX_EVENTS, -1);
        if (nfds == -1) {
            SPDLOG_ERROR("epoll_wait");

            if( errno == EINTR )
            {
                continue; // Interrupted by signal, retry
            }
            else{
                break; // Other error, exit loop
            }
        }
        else if( nfds == 0 )
        {
            continue; // No events, continue
        }

        for (int n = 0; n < nfds; ++n) {
            int fd = events[n].data.fd;
            if (fd == m_listenSock->m_sock) {
                // Handle new connection
                while (true)
                {
                    /* code */
                    auto newSock = m_listenSock->Accept();
                    if (!newSock) {
                        SPDLOG_ERROR("Accept Error");
                        continue;
                    }
                
                    onConnect( newSock );
                }
                

                

            } else if (fd == m_eventFd) {
                // Handle eventfd notification
                uint64_t notification;
                ssize_t s = read(m_eventFd, &notification, sizeof(notification));
                if (s != sizeof(notification)) {
                    SPDLOG_ERROR("read");
                    continue;
                }
                else{
                    onNewSendMsg();
                }
            } else {
                // Handle data from connected socket
                onSockEvent( events[n] );
            }
        }

    }
}

void NetworkMgrEpoll::onSockEvent( const struct epoll_event& ev )
{
    int fd = ev.data.fd;
    uint32_t events = ev.events;

    auto it = m_mapSocks.find(fd);
    if( it == m_mapSocks.end() )
    {
        return;
    }

    auto sock = it->second;
    if( sock == nullptr )
    {
        return;
    }

    if ( (events & EPOLLERR) || (events & EPOLLHUP) ) {
        // Handle error or hangup
        onDisconnect(fd);
        return;
    }

    if (events & EPOLLIN) {
        onReceiveMsg( sock );
    }

    if (events & EPOLLOUT) {
        // Handle outgoing data
        handleSendMsg(*sock);

    }    
}

void NetworkMgrEpoll::handleSendMsg( TcpSocket& sock )
{
    SPDLOG_DEBUG("handleSendMsg:{}", sock.m_sock);
    auto it = m_mapSlot.find( sock.m_sock );
    if( it == m_mapSlot.end() )
    {
        SPDLOG_DEBUG("onReceiveMsg, sock not found:{}", sock.m_sock);
        return ;
    }
    NetSlot& slot = it->second;

    char buff[RECV_BUFF] = {};
    int sendSize = slot.m_sendBuff.getData(buff, RECV_BUFF);
    while( sendSize > 0 )
    {
        int sended = sock.SendData(buff, sendSize);
        slot.m_sendBuff.consumeData(sended);

        if( sended < sendSize )
        {
            break; // Not all data sent, wait for next EPOLLOUT
        }
        else{
            sendSize = slot.m_sendBuff.getData(buff, RECV_BUFF);
        }
    }

    if( sendSize <= 0 )
    {
        // No more data to send, remove EPOLLOUT
        struct epoll_event ev;
        ev.data.fd = sock.m_sock;
        ev.events = EPOLLIN  ;
        if (epoll_ctl(m_epollFd, EPOLL_CTL_MOD, sock.m_sock, &ev) == -1) {
            SPDLOG_ERROR("epoll_ctl: fd");
        }
    }

}

void NetworkMgrEpoll::onNewSendMsg()
{
    while( true )
    {
        auto it = m_msgQueue.try_pop();
        if( it )
        {
            auto itSlot = m_mapSlot.find( it->first );
            if( itSlot == m_mapSlot.end() )
            {
                SPDLOG_ERROR("sendMsg: slot not found:{}", it->first);
                continue;
            }

            SPDLOG_TRACE("sendMsg: sock:{}, size:{}", it->first, it->second.size());

            NetSlot& slot = itSlot->second;
            slot.sendMsg( it->second );

            struct epoll_event ev;
            ev.data.fd = it->first;
            ev.events = EPOLLIN  | EPOLLOUT;
            if (epoll_ctl(m_epollFd, EPOLL_CTL_MOD, it->first, &ev) == -1) {
                SPDLOG_ERROR("epoll_ctl: fd");
                continue;
            }
        }
        else{
            break;
        }
    }
}

void NetworkMgrEpoll::onReceiveMsgInner( int fd, const std::string& msg )
{
}

void NetworkMgrEpoll::onDisconnectInner( int fd )
{
    struct epoll_event ev;
    ev.data.fd = fd;

    if( epoll_ctl( m_epollFd, EPOLL_CTL_DEL, fd, &ev ) == -1 )
    {
        SPDLOG_ERROR("epoll_ctl: fd");
        return;
    }

}
void NetworkMgrEpoll::onConnectInner( shared_ptr<TcpSocket> sock  )
{
    struct epoll_event ev;
    ev.data.fd = sock->m_sock;
    ev.events = EPOLLIN  ;
    if (epoll_ctl(m_epollFd, EPOLL_CTL_ADD, sock->m_sock, &ev) == -1) {
        SPDLOG_ERROR("epoll_ctl: fd");
        return;
    }
}

void NetworkMgrEpoll::innerSendMsg( int fd, const std::string& msg )
{
    notifyThread();
}

void NetworkMgrEpoll::notifyThread()
{
    uint64_t notification = 1;
    ssize_t ret = write( m_eventFd, &notification, sizeof(notification) );
    if( ret < 0 )
    {
        SPDLOG_ERROR("write err: {}", strerror(errno));
        return;
    }
    else if( ret != sizeof(notification) )
    {
        SPDLOG_ERROR("write err: {}", strerror(errno));
        return;
    }
    else{
        //success
    }
}

void NetworkMgrEpoll::innerNotifyThreadExit(){
    notifyThread();
}


void NetworkMgrEpoll::clearInvalidSock()
{

}