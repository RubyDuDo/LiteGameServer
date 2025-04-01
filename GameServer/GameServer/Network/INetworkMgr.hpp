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

class INetHandler{
public:
    virtual void onReceiveMsg( int fd, const std::string& msg ) = 0;
    virtual void onDisconnect( int fd ) = 0;
    virtual void onConnect( int fd ) = 0;
};

class INetworkMgr
{
public:
    static INetworkMgr* getInstance();
public:
    bool initNetwork( unsigned short svr_port ) ;
    
    virtual void  registerHandler(INetHandler* handler);
    void networkThread();
    
    virtual void shutdown() = 0;
    
    void sendMsg( int fd, const std::string& msg );
    
    virtual ~INetworkMgr();
    
protected:
    void onReceiveMsg( std::shared_ptr<TcpSocket> sock );
    void onDisconnect( int fd );
    void onConnect( int fd ) ;
    
private:
    virtual void onReceiveMsgInner( int fd, const std::string& msg ) = 0;
    virtual void onDisconnectInner( int fd ) = 0;
    virtual void onConnectInner( int fd ) = 0;
    
private:
    virtual void innerInit() = 0;
    virtual void innerRun() = 0 ;
private:
    static unique_ptr<INetworkMgr> m_pInstance;
protected:
    INetHandler* m_netHandler;
    
    std::shared_ptr<TcpSocket> m_listenSock;
    std::vector< TcpSocketPtr > m_setSocks;
    
    MsgQueue< std::pair<int, std::string> > m_msgQueue;
    std::map<int, NetSlot> m_mapSlot;
    
};

class INetworkMgrFactory
{
public:
    static std::unique_ptr<INetworkMgr> createNetworkMgr();
};


#endif /* INetworkMgr_hpp */
