//
//  NetSlot.cpp
//  GameServer
//
//  Created by pinky on 2025-03-31.
//

#include "NetSlot.hpp"
#include <string>
#include <iostream>
#include <arpa/inet.h>

constexpr int RECV_BUFF = 1500;

void NetSlot::sendMsg( const std::string& strData )
{
    short sendLen = strData.length();
    
    short len = htons( sendLen );
    m_sendBuff.addData( (char*)&len, sizeof(short) );
    m_sendBuff.addData( (char*)strData.c_str(), sendLen );
}

std::shared_ptr<std::string> NetSlot::getNextRecvMsg()
{
    //TCP拆包
    if( m_recvBuff.getSize() > sizeof( short ) )
    {
        char msgbuff[RECV_BUFF]{};
        short len = 0;
        m_recvBuff.getData( (char*)&len, sizeof(short));
        len = ntohs( len );
        if( m_recvBuff.getSize() >= len + sizeof(short) )
        {
            m_recvBuff.consumeData( sizeof( short ) );
            m_recvBuff.getData( msgbuff,  RECV_BUFF );
            m_recvBuff.consumeData( len );
                    
            return std::make_shared<std::string>( msgbuff, len );
        }
    }
    
    return nullptr;
}
