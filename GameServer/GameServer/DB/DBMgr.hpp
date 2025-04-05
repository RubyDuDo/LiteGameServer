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
#include <thread>
#include "../Utils/MsgQueue.hpp"
#include "../proto/dbmsg.pb.h"
#include "mysql/jdbc.h"
#include <optional>
using namespace MyGameDB;

class IDBResponseHandler
{
public:
    virtual void onReceiveDBRsp( int queryID, std::unique_ptr<DBResponse>&& rsp ) = 0;
    virtual ~IDBResponseHandler() = default;
};

class IDBQueryHandler
{
public:
    virtual void onQuery( int queryID, std::unique_ptr<DBRequest>&& req ) = 0;
    virtual ~IDBQueryHandler() = default;
    void registerResponseHandler( IDBResponseHandler* handler );
    void setSqlConn( sql::Connection* conn );
    void onRsp( int queryID, std::unique_ptr<DBResponse>&& rsp );
    
    template <typename T >
    void onRsp( int queryID, DBReqType type, DBErrorType res, const T& rsp );
    
    void onRsp( int queryID, DBReqType type, DBErrorType res);
    
    sql::Connection* getSqlConn() const { return m_conn; }
    
protected:
    IDBResponseHandler* m_rspHandler;
    sql::Connection* m_conn;
};



class DBMgr
{
public:
    DBMgr() = default;
    ~DBMgr();
    bool InitDB( const std::string& url, const std::string& user,
                const std::string& passwd, const std::string& database);
    
    void registerQueryHandler( IDBQueryHandler* handler );
    void registerResponseHandler( IDBResponseHandler* handler );
    
    void QueryThread();
    
    void addDBQuery( int queryID, std::unique_ptr<DBRequest>&& req );
    
    void shutdownDB();

public:
    int getNextID();
    
private:
    MsgQueue< pair<int, std::unique_ptr<DBRequest> > > m_msgReq;
    
    sql::mysql::MySQL_Driver* m_driver;
    std::unique_ptr<sql::Connection> m_conn;
    
    std::optional<std::thread> m_dbThread;
    
    int m_nextID = 0;
    
    IDBQueryHandler* m_queryHandler;
    IDBResponseHandler* m_rspHandler;
    
    bool m_bRunning = false;
    
};

template <typename T >
void IDBQueryHandler::onRsp( int queryID, DBReqType type, DBErrorType res, const T& rspBody )
{
    auto rsp = std::make_unique<DBResponse>();
    rsp->mutable_head()->set_type( type );
    rsp->mutable_head()->set_res( res );
    
    rsp->set_payload( rspBody.SerializeAsString() );
    
    onRsp( queryID, std::move(rsp) );
}

#endif /* DBMgr_hpp */
