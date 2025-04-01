//
//  GameNetHelper.cpp
//  GameServer
//
//  Created by pinky on 2025-03-31.
//

#include "GameNetHelper.hpp"
#include "Network/INetworkMgr.hpp"

MyGame::MsgHead* ProtobufHelp::CreatePacketHead( MsgType type )
{
    MsgHead* pHead = new MsgHead();
    pHead->set_type( type );
    
    return pHead;
}

MsgRspHead* ProtobufHelp::CreateRspHead( MsgType type, MsgErrCode res )
{
    MsgRspHead* pHead = new MsgRspHead();
    pHead->set_type( type );
    pHead->set_res( res );
    
    return pHead;
    
}

void NetSendHelper::addTcpQueue( int sockID, const MsgRsp& outMsg)
{
    std::string strData = outMsg.SerializeAsString();
    INetworkMgr::getInstance()->sendMsg( sockID, strData );
}
