//
//  GameNetHelper.hpp
//  GameServer
//
//  Created by pinky on 2025-03-31.
//

#ifndef GameNetHelper_hpp
#define GameNetHelper_hpp

#include <stdio.h>
#include "proto/msg.pb.h"
using namespace MyGame;

class ProtobufHelp
{
public:
    static MsgHead* CreatePacketHead( MsgType type );
    static MsgRspHead* CreateRspHead( MsgType type, MsgErrCode res );
};


class NetSendHelper
{
public:
    template< typename T>
    static void addTcpQueue( int sockID, MsgType type, MsgErrCode res, const T& rsp )
    {
        MsgRsp outMsg;
        outMsg.set_allocated_head( ProtobufHelp::CreateRspHead( type, res ));
        outMsg.mutable_payload()->PackFrom( rsp );
        
        addTcpQueue( sockID, outMsg );
    }
    
    static void addTcpQueue( int sockID, const MsgRsp& outMsg);
};

#endif /* GameNetHelper_hpp */
