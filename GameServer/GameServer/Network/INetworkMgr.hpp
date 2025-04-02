//
//  INetworkMgr.hpp
//  GameServer
//
//  Created by pinky on 2025-03-31.
//

#ifndef INetworkMgr_hpp
#define INetworkMgr_hpp

#include <stdio.h>
#include <string>
#include "TcpSocket.hpp"
#include <memory>
#include "../Utils/MsgQueue.hpp"
#include "NetSlot.hpp"
#include <map>
#include <optional>
#include <thread>

class INetHandler{
public:
    virtual void onReceiveMsg( int fd, const std::string& msg ) = 0;
    virtual void onDisconnect( int fd ) = 0;
    virtual void onConnect( const TcpSocket& sock ) = 0;
};

class INetworkMgr
{
public:
    static INetworkMgr* getInstance();
public:
    bool initNetwork( unsigned short svr_port ) ;
    void shutdownNetwork();
    
    virtual void  registerHandler(INetHandler* handler);
    void networkThread();
    
    
    
    void sendMsg( int fd, const std::string& msg );
    
    virtual ~INetworkMgr();
    
protected:
    bool onReceiveMsg( std::shared_ptr<TcpSocket> sock );
    void onDisconnect( int fd );
    void onConnect( shared_ptr<TcpSocket> sock ) ;
    
private:
    virtual void onReceiveMsgInner( int fd, const std::string& msg ) = 0;
    virtual void onDisconnectInner( int fd ) = 0;
    virtual void onConnectInner( shared_ptr<TcpSocket> sock  ) = 0;
    
private:
    virtual bool innerInit() = 0;
    virtual void innerRun() = 0 ;
    virtual void innerShutdown() = 0;
    virtual void innerSendMsg( int fd, const std::string& msg ) ;
private:
    static unique_ptr<INetworkMgr> m_pInstance;
protected:
    INetHandler* m_netHandler;

    bool m_bRunning = false;
    
    std::shared_ptr<TcpSocket> m_listenSock;
    std::vector< TcpSocketPtr > m_setSocks;
    std::map< int, TcpSocketPtr > m_mapSocks;
    
    MsgQueue< std::pair<int, std::string> > m_msgQueue;
    std::map<int, NetSlot> m_mapSlot;
    std::optional<std::thread> m_runThread;
    
};

class INetworkMgrFactory
{
public:
    static std::unique_ptr<INetworkMgr> createNetworkMgr();
};


#endif /* INetworkMgr_hpp */
