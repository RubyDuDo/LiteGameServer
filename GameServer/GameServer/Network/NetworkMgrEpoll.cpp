#include "NetworkMgrEpoll.hpp"

#include <sys/epoll.h>  // For epoll_create1
#include <sys/eventfd.h> // For eventfd
#include <unistd.h>    // For close()
#include <stdio.h>     // For perror()
#include <stdlib.h>    // For EXIT_FAILURE
#include <iostream>

constexpr int RECV_BUFF = 1500;
    
NetworkMgrEpoll::~NetworkMgrEpoll()
{
    shutdown();
}

void NetworkMgrEpoll::shutdown()
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

void NetworkMgrEpoll::innerInit()
{
    m_epollFd = epoll_create1( EPOLL_CLOEXEC );
    if (m_epollFd == -1) {
        perror("epoll_create1");
        exit(EXIT_FAILURE);
    }

    m_eventFd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC );
    if (m_eventFd == -1) {
        perror("eventfd");
        close(m_epollFd); 
        exit(EXIT_FAILURE);
    }

    struct epoll_event evListen;
    evListen.data.fd = m_listenSock->m_sock;
    evListen.events = EPOLLIN | EPOLLET;
    if (epoll_ctl(m_epollFd, EPOLL_CTL_ADD, m_listenSock->m_sock, &evListen) == -1) {
        perror("epoll_ctl: listen_sock");
        shutdown();
        exit(EXIT_FAILURE);
    }

    struct epoll_event evEvent;
    evEvent.data.fd = m_eventFd;
    evEvent.events = EPOLLIN | EPOLLET;
    if( epoll_ctl(m_epollFd, EPOLL_CTL_ADD, m_eventFd, &evEvent) == -1 )
    {
        perror("epoll_ctl: event_fd");
        shutdown();
        exit(EXIT_FAILURE);
    }
}

void NetworkMgrEpoll::innerRun()
{
    std::cout<<"network Thread( Epoll: Nonblock Mode) started!"<<std::endl;
    while(true)
    {
        const int MAX_EVENTS = 10;
        std::vector<struct epoll_event> events(MAX_EVENTS);
        int nfds = epoll_wait(m_epollFd, events.data(), MAX_EVENTS, -1);
        if (nfds == -1) {
            perror("epoll_wait");

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
                auto newSock = m_listenSock->Accept();
                if (!newSock) {
                    perror("Accept Error");
                    continue;
                }
            
                onConnect( newSock );
                

            } else if (fd == m_eventFd) {
                // Handle eventfd notification
                uint64_t notification;
                ssize_t s = read(m_eventFd, &notification, sizeof(notification));
                if (s != sizeof(notification)) {
                    perror("read");
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
    auto sock = m_mapSocks[fd];

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
        int ret = onReceiveMsg( sock );
        if( ret == 0 )
        {
            onDisconnect(fd);
            return;
        }
        else if( ret < 0 )
        {
            perror("recv error");
            onDisconnect(fd);
            return; 
        }
    }

    if (events & EPOLLOUT) {
        // Handle outgoing data
        handleSendMsg(*sock);

    }    
}

void NetworkMgrEpoll::handleSendMsg( TcpSocket& sock )
{
    NetSlot& slot = m_mapSlot[ sock.m_sock];

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
        ev.events = EPOLLIN | EPOLLET ;
        if (epoll_ctl(m_epollFd, EPOLL_CTL_MOD, sock.m_sock, &ev) == -1) {
            perror("epoll_ctl: fd");
            return;
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
            NetSlot& slot = m_mapSlot[it->first];
            slot.sendMsg( it->second );

            struct epoll_event ev;
            ev.data.fd = it->first;
            ev.events = EPOLLOUT | EPOLLET | EPOLLOUT;
            if (epoll_ctl(m_epollFd, EPOLL_CTL_MOD, it->first, &ev) == -1) {
                perror("epoll_ctl: fd");
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
        perror("epoll_ctl: fd");
        return;
    }

}
void NetworkMgrEpoll::onConnectInner( shared_ptr<TcpSocket> sock  )
{
    struct epoll_event ev;
    ev.data.fd = sock->m_sock;
    ev.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
    if (epoll_ctl(m_epollFd, EPOLL_CTL_ADD, sock->m_sock, &ev) == -1) {
        perror("epoll_ctl: fd");
        return;
    }
}

void NetworkMgrEpoll::innerSendMsg( int fd, const std::string& msg )
{
    uint64_t notification = 1;
    ssize_t ret = write( m_eventFd, &notification, sizeof(notification) );
    if( ret < 0 )
    {
        perror("innerSendMsg");
        return;
    }
    else if( ret != sizeof(notification) )
    {
        perror("innerSendMsg");
        return;
    }
    else{
        //success
    }
}


void NetworkMgrEpoll::clearInvalidSock()
{

}