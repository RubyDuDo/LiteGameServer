//
//  NetworkMgr.cpp
//  GameServer
//
//  Created by pinky on 2025-02-19.
//

#include "NetworkMgr.hpp"
#include <thread>
#include <iostream>
#include "Buffer.hpp"
#include <chrono>
#include <sys/select.h>
using namespace std::chrono_literals;
using namespace std;

constexpr int RECV_BUFF = 1500;

NetworkMgr* NetworkMgr::m_pMgr = nullptr;

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


void Slot::sendMsg( const MsgRsp& msg )
{
    cout<<"Slot SendMsg:"<<msg.head().type()<<endl;
    std::string strData = msg.SerializeAsString();
    
    short sendLen = strData.length();
    
    short len = htons( sendLen );
    m_sendBuff.addData( (char*)&len, sizeof(short) );
    m_sendBuff.addData( (char*)strData.c_str(), sendLen );
}

std::shared_ptr<Msg> Slot::getNextRecvMsg()
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
            
            std::string data( msgbuff, len );
            Msg msg;
            if( !msg.ParseFromString( data ))
            {
                cout<<"Parse head fail!"<<endl;
                return nullptr;
            }
            
//            cout<<"Parse Msg:" << msg.head().type()<<"_______"<<msg.DebugString()<<endl;
        
            return std::make_shared<Msg>(msg);
        }
    }
    
    return nullptr;
}

NetworkMgr* NetworkMgr::getInstance()
{
    if( !m_pMgr )
    {
        m_pMgr = new NetworkMgr();
    }
    return m_pMgr;
}

bool NetworkMgr::InitNetwork( unsigned short svr_port )
{
    m_listenSock = NetUtil::createTcpSocket();
    
    m_listenSock->Bind( svr_port );
    
    m_listenSock->Listen();
    
    m_setSocks.push_back( m_listenSock );
    m_maxFd = m_listenSock->m_sock;
    
    std::thread th(&NetworkMgr::networkNonBlockThread, this );
//    std::thread th(&NetworkMgr::networkThread, this );
    th.detach();
    
    return true;
}

void NetworkMgr::clearInvalidSock()
{
    auto it = m_setSocks.begin();
    if( it != m_setSocks.end() )
    {
        auto pSock = *it;
        if( pSock->isValid() )
        {
            it++;
        }
        else{
            it = m_setSocks.erase( it );
        }
    }
}

void NetworkMgr::registerReceiveMsgHandle( std::function<void(int,const Msg&)> recvFun)
{
    m_recvFun = recvFun;
}

void NetworkMgr::networkThread()
{
    std::cout<<"network Thread started!"<<std::endl;
    while( true )
    {
        auto playersock = m_listenSock->Accept();
        if( playersock )
        {
            auto playerThread = std::thread(&NetworkMgr::playerThread, this, playersock);
            playerThread.detach();
        }
    }
}

void NetworkMgr::networkNonBlockThread()
{
    std::cout<<"network Thread(Nonblock Mode) started!"<<std::endl;
    while( true )
    {
        std::this_thread::sleep_for( 10ms );
        clearInvalidSock();
        std::vector<TcpSocketPtr> outReadSet, outWriteSet, outExceptSet;
    
        int ret = NetUtil::Select( m_maxFd, m_setSocks, outReadSet, m_setSocks, outWriteSet, m_setSocks, outExceptSet);
        if( ret < 0 )
        {
            perror("select error");
            break;
        }
        
        if( !outReadSet.empty())
        {
            for( auto it : outReadSet )
            {
                if( it == m_listenSock )
                {
                    auto sock = m_listenSock->Accept();
                    if( sock )
                    {
                        m_setSocks.push_back( sock );
                        m_mapSlot[sock->m_sock] = Slot();
                        
                        if( sock->m_sock > m_maxFd)
                        {
                            m_maxFd = sock->m_sock;
                        }
                    }
                }
                else{
                    if( !it->isValid() )
                    {
                        continue;
                    }
                    
                    //do receive
                    char buff[RECV_BUFF]{};
                    int ret = it->RecvData( buff, RECV_BUFF );
                    cout<<"receiveData:"<<ret<<": from :"<<it->m_sock<<endl;
                    if( ret == 0 )
                    {
                        it->setValid( false );
                        break;
                    }
                    else if( ret > 0 )
                    {
                        Slot& slot = m_mapSlot[it->m_sock];
                        slot.m_recvBuff.addData( buff, ret );
                        
                        auto pMsg = slot.getNextRecvMsg();
                        if( pMsg )
                        {
                            onReceiveMsg( it, *pMsg );
                        }
                    }
                }
            }
            
        }
        
        if( !outWriteSet.empty() )
        {
            //do write
            for( auto it : outWriteSet )
            {
                if( it == m_listenSock )
                {
                    continue;
                }
                
                Slot& slot = m_mapSlot[it->m_sock];
                char buff[RECV_BUFF]{};
                int sendSize = slot.m_sendBuff.getData( buff , RECV_BUFF );
                if( sendSize > 0 )
                {
                    int sended = it->SendData( buff ,  sendSize);
                    slot.m_sendBuff.consumeData( sended );
                }
            }
        }
    }
}

void NetworkMgr::onReceiveMsg( std::shared_ptr<TcpSocket> sock, const Msg& packet )
{
    if( m_recvFun )
    {
        m_recvFun( sock->m_sock ,packet );
    }
}

void NetworkMgr::addTcpQueue( int sockID, const MsgRsp& packet )
{
    auto it = m_mapSlot.find( sockID);
    if( it == m_mapSlot.end())
    {
        cout<<"addTcpQueue sockID not found:"<<sockID<<endl;
        return;
    }
    
    it->second.sendMsg( packet );
}





void NetworkMgr::playerThread( std::shared_ptr<TcpSocket> sock )
{
    Buffer recvBuff;
    while( true )
    {
        std::this_thread::sleep_for( 20ms );
        char buff[RECV_BUFF]{};
        int ret = sock->RecvData( buff, RECV_BUFF );
        if( ret == 0 )
        {
            break;
        }
        else if( ret > 0 )
        {
            recvBuff.addData( buff, ret );
        }
        
        //TCP拆包
        if( recvBuff.getSize() > sizeof( short ) )
        {
            char msgbuff[RECV_BUFF]{};
            short len = 0;
            recvBuff.getData( (char*)&len, sizeof(short));
            len = ntohs( len );
            if( recvBuff.getSize() >= len + sizeof(short) )
            {
                recvBuff.consumeData( sizeof( short ) );
                recvBuff.getData( msgbuff,  RECV_BUFF );
                recvBuff.consumeData( len );
                
                std::string msg( msgbuff , len );
                
                
//                onReceiveMsg( sock, msg );
            }
        }
    }
}
