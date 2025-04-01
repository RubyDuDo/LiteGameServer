//
//  TcpSocket.cpp
//  GameServer
//
//  Created by pinky on 2025-02-19.
//

#include "TcpSocket.hpp"

#include <unistd.h>     // close()
#include <sys/socket.h> // socket(), bind(), listen(), accept()
#include <arpa/inet.h> // sockaddr_in, inet_addr()

#include <fcntl.h> //O_NONBLOCK, fcntl

#include <iostream>
#include <string>


TcpSocketPtr NetUtil::createTcpSocket()
{
    int sock = socket( AF_INET, SOCK_STREAM, 0);
    if( sock < 0 )
    {
        perror("socket() error");
        return nullptr;
    }
    
    return std::make_shared<TcpSocket>(sock);
}

fd_set*  NetUtil::FillSetFromVector( fd_set& out_set, const std::vector< TcpSocketPtr > & insocks)
{
    FD_ZERO(&out_set);
    for( auto it : insocks )
    {
        FD_SET( it->m_sock, &out_set);
    }
    
    return &out_set;
    
}
std::vector<TcpSocketPtr>  NetUtil::FillVectorFromSet( const std::vector<TcpSocketPtr>& insocks ,const fd_set& inSet )
{
    std::vector<TcpSocketPtr> outSocks;
    for( auto it : insocks )
    {
        if( FD_ISSET( it->m_sock, &inSet) )
        {
            outSocks.push_back( it );
        }
    }
    
    return outSocks;
}

int NetUtil::Select( int maxFd, const std::vector<TcpSocketPtr>& inReadSet, std::vector<TcpSocketPtr>& outReadSet,
           const std::vector<TcpSocketPtr>& inWriteSet, std::vector<TcpSocketPtr>& outWriteSet,
           const std::vector<TcpSocketPtr>& inExceptSet, std::vector<TcpSocketPtr>& outExceptSet)
{
    fd_set read, write, except;
    
    fd_set* readPtr = FillSetFromVector( read, inReadSet );
    fd_set* writePtr = FillSetFromVector( write, inReadSet );
    fd_set* exceptPtr = FillSetFromVector( except, inReadSet );
    
    int ret = select(maxFd + 1 , readPtr, writePtr, exceptPtr, nullptr );
    
    if( ret > 0  )
    {
        outReadSet = FillVectorFromSet( inReadSet, *readPtr);
        outWriteSet = FillVectorFromSet( inWriteSet, *writePtr);
        outExceptSet = FillVectorFromSet( inExceptSet, *exceptPtr);
    }
    
    return ret;
    
}


TcpSocket::TcpSocket( int sock ): m_sock( sock )
{
    
}

TcpSocket::~TcpSocket()
{
    close( m_sock );
}

void TcpSocket::setValid( bool bValid )
{
    m_bValid = bValid;
}

bool TcpSocket::isValid( )
{
    return m_bValid;
}

int TcpSocket::Bind( unsigned short port )
{
    //set resueable, which will be more convenient when debug
    int opt = 1;
    if (setsockopt(m_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt(SO_REUSEADDR) failed");
    }
    
    struct sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons( port );
    
    int ret = bind( m_sock, (struct sockaddr*)&addr, sizeof(addr) );
    if( ret < 0 )
    {
        perror("Bind Error");
        return ret;
    }
    

    return 0;
}

int TcpSocket::Listen( int backlog )
{
    int ret = listen( m_sock, backlog );
    if( ret < 0 )
    {
        perror("Listen Error!");
        return ret;
    }
    return 0;
}

std::shared_ptr<TcpSocket> TcpSocket::Accept()
{
    struct sockaddr_in client_addr{};
    socklen_t client_len = sizeof(client_addr);
    int sock = accept( m_sock, (struct sockaddr*) &client_addr,  &client_len);
    
    if( sock > 0 )
    {
        char ip_str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, ip_str, sizeof(ip_str));
        
        std::cout<<"Connect from:"<<ip_str<<":"<<ntohs(client_addr.sin_port) << std::endl;
        return std::make_shared<TcpSocket>(sock);
    }
    else{
        return nullptr;
    }
}

int TcpSocket::Connect( const std::string& ip, unsigned short port )
{
    struct sockaddr_in addr{};
    addr.sin_family = AF_INET;
    inet_pton( AF_INET, ip.c_str(), &addr.sin_addr );
    addr.sin_port = htons( port );
    int ret = connect( m_sock, (struct sockaddr*)&addr, sizeof(addr) );
    if( ret < 0 )
    {
        perror( "Connect Error");
        return ret;
    }
    return 0;
}

int TcpSocket::SendData( const char* buff, int len )
{
    int ret = send( m_sock, buff, len, 0);
    if( ret == -1 )
    {
        perror( "SendData Error");
    }
    
    return ret;
}

int TcpSocket::RecvData( char* buff, int maxLen )
{
    int ret = recv( m_sock, buff, maxLen, 0);
    if( ret == -1 )
    {
        perror("RecvData Error");
    }
    return ret;
}

int TcpSocket::setNonBlock( bool bNonBlock )
{
    int flags = fcntl( m_sock, F_GETFL, 0 );
    flags = bNonBlock ?( flags | O_NONBLOCK ):( flags & ~O_NONBLOCK );
    int ret = fcntl( m_sock, F_SETFL, flags );
    
    return ret;
    
}

int TcpSocket::setReuseAddr( bool bReuse )
{
    int opt = bReuse ? 1 : 0;
    int ret = setsockopt( m_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt) );
    if( ret < 0 )
    {
        perror("setsockopt(SO_REUSEADDR) failed");
    }
    
    return ret;
}
