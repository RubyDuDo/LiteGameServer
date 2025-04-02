//
//  NetworkMgrSelect.hpp
//  GameServer
//
//  Created by pinky on 2025-03-31.
//

#ifndef NetworkMgrSelect_hpp
#define NetworkMgrSelect_hpp

#include <stdio.h>
#include "INetworkMgr.hpp"



class NetworkMgrSelect : public INetworkMgr
{
public:
    NetworkMgrSelect() = default;
    
    virtual ~NetworkMgrSelect();
    
private:
    virtual void innerShutdown() ;
    virtual bool innerInit();
    virtual void innerRun();
    
    virtual void onReceiveMsgInner( int fd, const std::string& msg );
    virtual void onDisconnectInner( int fd );
    virtual void onConnectInner( shared_ptr<TcpSocket> sock  );
    
    //move msg from msgqueue to individual slots
    void dispatchSendMsg();
    
private:
    void clearInvalidSock();
    
private:
    int m_maxFd = 0 ;
    
    
};

#endif /* NetworkMgrSelect_hpp */
