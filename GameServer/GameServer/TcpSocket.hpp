//
//  TcpSocket.hpp
//  GameServer
//
//  Created by pinky on 2025-02-19.
//

#ifndef TcpSocket_hpp
#define TcpSocket_hpp

#include <stdio.h>
#include <memory>
#include <set>
#include <vector>


constexpr int BACKLOG_D = 500;

class TcpSocket
{
public:
    int m_sock;
public:
    TcpSocket( int sock );
    ~TcpSocket();
    
    int Bind( short port );
    int Listen( int backlog = BACKLOG_D );
    std::shared_ptr<TcpSocket> Accept();
    
    int Connect( const std::string& ip, short port );
    
    int SendData( const char* buff, int len );
    
    int RecvData( char* buff, int maxLen );
    
    int setNonBlock( bool bNonBlock );
    
};

typedef std::shared_ptr<TcpSocket> TcpSocketPtr;

class NetUtil
{
public:
    static TcpSocketPtr createTcpSocket();
    
    static fd_set*  FillSetFromVector( fd_set& out_set, const std::vector< TcpSocketPtr > & insocks);
    static std::vector<TcpSocketPtr> FillVectorFromSet( const std::vector<TcpSocketPtr>& insocks , const fd_set& inSet);
    
    static int Select(  int maxFd, const std::vector<TcpSocketPtr>& inReadSet, std::vector<TcpSocketPtr>& outReadSet,
               const std::vector<TcpSocketPtr>& inWriteSet, std::vector<TcpSocketPtr>& outWriteSet,
               const std::vector<TcpSocketPtr>& inExceptSet, std::vector<TcpSocketPtr>& outExceptSet);
    
};

#endif /* TcpSocket_hpp */
