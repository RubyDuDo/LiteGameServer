//
//  DBMgr.hpp
//  GameServer
//
//  Created by pinky on 2025-02-28.
//

#ifndef DBMgr_hpp
#define DBMgr_hpp

#include <stdio.h>
#include <memory>
#include "../MsgQueue.hpp"
#include "../proto/dbmsg.pb.h"
#include "mysql/jdbc.h"
using namespace MyGameDB;

class DBMgr
{
public:
    DBMgr() = default;
    ~DBMgr();
    bool InitDB( const std::string& url, const std::string& user,
                const std::string& passwd, const std::string& database);
    
    void QueryThread();
    
    void addDBQuery( int queryID, const DBRequest& req );
    
    void registerRspHandle( std::function<void(int, const DBResponse&)> recvFun);
public:
    void queryAccount( int queryID,  const DBRequest& req );
    void addRole( int queryID, const DBRequest& req);
    void queryRole( int queryID, const DBRequest& req );
    
public:
    void onRsp( int queryID, const DBResponse& rsp );
    template <typename T >
    void onRsp( int queryID, DBReqType type, DBErrorType res, const T& rsp );
    
public:
    int getNextID();
    
    
    
    
public:
    //处理返回结果
    
private:
    MsgQueue< pair<int, DBRequest> > m_msgReq;
    
    sql::mysql::MySQL_Driver* m_driver;
    std::unique_ptr<sql::Connection> m_conn;
    
    std::function<void( int, const DBResponse&)> m_recvFun;
    
    int m_nextID;
    
};

template <typename T >
void DBMgr::onRsp( int queryID, DBReqType type, DBErrorType res, const T& rspBody )
{
    DBResponse rsp;
    rsp.mutable_head()->set_type( type );
    rsp.mutable_head()->set_res( res );
    
    rsp.set_payload( rspBody.SerializeAsString() );
    
    onRsp( queryID, rsp );
    
}

#endif /* DBMgr_hpp */
