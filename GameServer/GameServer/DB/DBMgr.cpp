//
//  DBMgr.cpp
//  GameServer
//
//  Created by pinky on 2025-02-28.
//

#include "DBMgr.hpp"
#include <thread>
#include <format>
#include "../Utils/LoggerHelper.hpp"

// 定义一个 ScopeGuard 类，当对象销毁时执行传入的 lambda 函数
//class ScopeGuard {
//public:
//    explicit ScopeGuard(std::function<void()> onExitScope)
//        : onExitScope_(onExitScope), dismissed_(false) {}
//
//    ~ScopeGuard() {
//        if (!dismissed_) {
//            onExitScope_();
//        }
//    }
//
//    // 如果希望提前取消这个“finally”行为，可以调用此方法
//    void dismiss() { dismissed_ = true; }
//
//    // 禁止拷贝构造和拷贝赋值
//    ScopeGuard(const ScopeGuard&) = delete;
//    ScopeGuard& operator=(const ScopeGuard&) = delete;
//
//private:
//    std::function<void()> onExitScope_;
//    bool dismissed_;
//};


void IDBQueryHandler::registerResponseHandler( IDBResponseHandler* handler )
{
    m_rspHandler = handler;
}

void IDBQueryHandler::setSqlConn( sql::Connection* conn )
{
    m_conn = conn;
}

void IDBQueryHandler::onRsp( int queryID, std::unique_ptr<DBResponse>&& rsp )
{
    if( m_rspHandler )
    {
        m_rspHandler->onReceiveDBRsp( queryID, std::move(rsp) );
    }
}

void IDBQueryHandler::onRsp( int queryID, DBReqType type, DBErrorType res)
{
    auto rsp = std::make_unique<DBResponse>();
    rsp->mutable_head()->set_type( type );
    rsp->mutable_head()->set_res( res );
    
    onRsp( queryID, std::move(rsp) );
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///
void DBMgr::registerQueryHandler( IDBQueryHandler* handler )
{
    m_queryHandler = handler;
    m_queryHandler->setSqlConn( m_conn.get() );
    if( m_rspHandler){
        m_queryHandler->registerResponseHandler( m_rspHandler );
    }
}

void DBMgr::registerResponseHandler( IDBResponseHandler* handler )
{
    m_rspHandler = handler;
    if( m_queryHandler )
    {
        m_queryHandler->registerResponseHandler( m_rspHandler );
    }
}

DBMgr::~DBMgr()
{
    shutdownDB();
}

int DBMgr::getNextID()
{
    return m_nextID++;
}

bool DBMgr::InitDB( const std::string& url, const std::string& user,
            const std::string& passwd, const std::string& database)
{
    
    try {
        //connect to db
        m_driver = sql::mysql::get_mysql_driver_instance();
        
        m_conn.reset(  m_driver->connect(url, user, passwd) );
        
        m_conn->setSchema( database );
        
    } catch ( sql::SQLException& e ) {
        SPDLOG_ERROR("SQLException: {},{}", e.getErrorCode(), e.getSQLState());
        return false;
    }
    
    m_bRunning = true;
    
    m_dbThread = make_optional<std::thread>(&DBMgr::QueryThread, this );
    
    return true;
    
    //start query thread
}

void DBMgr::shutdownDB()
{
    SPDLOG_INFO(" Enter ");
    m_bRunning = false;
    
    if( m_dbThread && m_dbThread->joinable())
    {
        m_dbThread->join();
    }
}

void DBMgr::addDBQuery( int queryID, std::unique_ptr<DBRequest>&& req )
{
    m_msgReq.push( make_pair(queryID, std::move(req) ) );
}

void DBMgr::QueryThread()
{
    while(m_bRunning)
    {
        auto it = m_msgReq.wait_and_pop();
        if( m_queryHandler)
        {
            m_queryHandler->onQuery( it->first, std::move(it->second) );
        }
    }
    
}
