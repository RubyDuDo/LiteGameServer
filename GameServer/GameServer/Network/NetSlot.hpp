//
//  NetSlot.hpp
//  GameServer
//
//  Created by pinky on 2025-03-31.
//

#ifndef NetSlot_hpp
#define NetSlot_hpp

#include <stdio.h>
#include <memory>
#include "../Utils/Buffer.hpp"

class NetSlot
{
public:
    NetSlot():m_recvBuff(), m_sendBuff(){};
    
    void sendMsg( const std::string& strData );
    std::shared_ptr<std::string> getNextRecvMsg();
public:
    RingBuffer m_recvBuff;
    RingBuffer m_sendBuff;
};


#endif /* NetSlot_hpp */
