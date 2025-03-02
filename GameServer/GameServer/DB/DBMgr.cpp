//
//  DBMgr.cpp
//  GameServer
//
//  Created by pinky on 2025-02-28.
//

#include "DBMgr.hpp"
#include <thread>

// 定义一个 ScopeGuard 类，当对象销毁时执行传入的 lambda 函数
class ScopeGuard {
public:
    explicit ScopeGuard(std::function<void()> onExitScope)
        : onExitScope_(onExitScope), dismissed_(false) {}

    ~ScopeGuard() {
        if (!dismissed_) {
            onExitScope_();
        }
    }

    // 如果希望提前取消这个“finally”行为，可以调用此方法
    void dismiss() { dismissed_ = true; }

    // 禁止拷贝构造和拷贝赋值
    ScopeGuard(const ScopeGuard&) = delete;
    ScopeGuard& operator=(const ScopeGuard&) = delete;

private:
    std::function<void()> onExitScope_;
    bool dismissed_;
};


//#include <jdbc/mysql_driver.h>
//#include <jdbc/mysql_connection.h>
//#include <jdbc/cppconn/statement.h>
//#include <jdbc/cppconn/resultset.h>
//#include <jdbc/cppconn/prepared_statement.h>
//#include <jdbc/cppconn/exception.h>

void DBMgr::registerRspHandle( std::function<void( int, const DBResponse&)> recvFun)
{
    m_recvFun = recvFun;

}

DBMgr::~DBMgr()
{
    
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
        std:cerr<<"Err: SQLException in:"<<e.getErrorCode()<<"_"<<e.getSQLState()<<endl;
        
        return false;
    }
    
    auto queryThread = std::thread(&DBMgr::QueryThread, this );
    queryThread.detach();
    
    return true;
    
    //start query thread
}

void DBMgr::addDBQuery(  int queryID,const DBRequest& req )
{
    m_msgReq.push( make_pair(queryID, req ) );
}

void DBMgr::QueryThread()
{
    while(true)
    {
        auto it = m_msgReq.wait_and_pop();
        auto query = it->second;
        switch( query.head().type() )
        {
            case DBReqType_QueryAccount:
                queryAccount( it->first,  query );
                break;
            case DBReqType_ModAccount:
                break;
            case DBReqType_QueryRole:
                queryRole( it->first,  query );
                break;
            case DBReqType_AddRole:
                addRole( it->first, query );
                break;
            default:
                break;
        }
    }
    
}

void DBMgr::queryAccount( int queryID , const DBRequest& req )
{
    DBReqQueryAccount query;
    if( !query.ParseFromString( req.payload() ))
    {
        cerr<<" Query Account Parse Fail!"<<endl;
        return;
    }
    
    try {
        DBResponse rsp;
        rsp.mutable_head()->set_type( req.head().type() );
        
        std::unique_ptr<sql::Statement> stmt( m_conn->createStatement());
        
        string strQuery = std::format("select * from accounts where account = '{}' ", query.account() );
        cout<<strQuery<<endl;
    
        std::unique_ptr<sql::ResultSet> res( stmt->executeQuery(strQuery));
        
        if( res->next() )
        {
            std::string name = res->getString( "account" );
            std::string passwd = res->getString("passwd");
            int roleid = res->getInt("role_id");
            cout<<"Account Name :"<<name<<"_"<<passwd<<"_"<<roleid<<endl;
            
            DBRspAccout rspQuery;
            rspQuery.set_account( name );
            rspQuery.set_passwd( passwd );
            rspQuery.set_roleid( roleid );
            
            string strData = rspQuery.SerializeAsString();
            

            rsp.mutable_head()->set_res( DBErr_OK );
            rsp.set_payload( strData );
            
            onRsp( queryID, rsp );
            
        }
        else{
            rsp.mutable_head()->set_res( DBErr_NotExist );
            onRsp( queryID, rsp );
        }
        
        
    } catch ( sql::SQLException& e ) {
        std:cerr<<"Err: SQLException in:"<<e.getErrorCode()<<"_"<<e.getSQLState()<<endl;
        return ;
    }
}

void DBMgr::queryRole( int queryID, const DBRequest& req )
{
    DBReqQueryRole query;
    DBRspRole rspQuery;
    if( !query.ParseFromString( req.payload() ))
    {
        cerr<<" Query Role Parse Fail!"<<endl;
        onRsp( queryID, req.head().type(), DBErr_Fail, rspQuery );
        return;
    }
    
    try {
        std::unique_ptr<sql::Statement> stmt( m_conn->createStatement());
        
        string strQuery = std::format("select * from roles where role_id = {}", query.roleid() );
        cout<<strQuery<<endl;
    
        std::unique_ptr<sql::ResultSet> res( stmt->executeQuery(strQuery));
        
        if( res->next() )
        {
            std::string name = res->getString( "name" );
            int level = res->getInt("level");
            int roleid = res->getInt("role_id");
            cout<<"Role Name :"<<name<<"_"<<roleid<<"_"<<level<<endl;
            
            rspQuery.set_roleid(  roleid );
            rspQuery.set_name( name );
            rspQuery.set_level( level );
            
            onRsp( queryID, req.head().type(), DBErr_OK, rspQuery );
        }
        else{
            onRsp( queryID, req.head().type(), DBErr_NotExist, rspQuery );
        }
        
        
    } catch ( sql::SQLException& e ) {
        std:cerr<<"Err: SQLException in:"<<e.getErrorCode()<<"_"<<e.getSQLState()<<endl;
        onRsp( queryID, req.head().type(), DBErr_Fail, rspQuery );
        return ;
    }
    
}



void DBMgr::addRole( int queryID, const DBRequest& req)
{
    DBReqAddRole query;
    DBRspRole rspQuery;
    
    DBReqType reqType = req.head().type();
    DBErrorType res = [&]() -> DBErrorType
    {
        if( !query.ParseFromString( req.payload() ))
        {
            cerr<<" Query Add Role Parse Fail!"<<endl;
            return DBErr_Fail;
        }
        
        try {

            //step 1： insert a new role
            string strQuery = std::format("insert into roles (name) values ('{}') ", query.name() );
            cout<<strQuery<<endl;
            
            std::unique_ptr<sql::Statement> stmt( m_conn->createStatement());
            
            int rowAffected =  stmt->executeUpdate(strQuery);
            if( rowAffected == 0 )
            {
                cerr<<"Insert role fail"<<endl;
                return DBErr_Fail;
            }
            
            //step 2: get the new roleid
            int roleid = 0;
            std::unique_ptr<sql::ResultSet> res( stmt->executeQuery( "SELECT LAST_INSERT_ID()" ));
            if (res->next()) {
                    int autoIncrementId = res->getInt(1);
                    cout << "Auto-incremented ID: " << autoIncrementId << endl;
                rspQuery.set_roleid( autoIncrementId);
                roleid = autoIncrementId;
            }
            else{
                cerr<<" Query Add Role insert Fail!"<<endl;
                return DBErr_Fail;
            }
            
            //step 3: modify the account table
            strQuery = std::format("update accounts set role_id = {} where account = '{}'", roleid, query.name() );
            cout<<strQuery<<endl;
            rowAffected =  stmt->executeUpdate( strQuery );
            if( rowAffected == 0 )
            {
                cerr<<"update account fail"<<endl;
                return DBErr_Fail;
            }
            
            //step 4: get the whole roleinfo from db
            strQuery = std::format("select * from roles where role_id = {}", rspQuery.roleid() );
            cout<<strQuery<<endl;
            std::unique_ptr<sql::ResultSet> resQuery( stmt->executeQuery(strQuery));
            
            if( resQuery->next() )
            {
                std::string name = res->getString( "name" );
                int level = res->getInt("level");
                int roleid = res->getInt("role_id");
                cout<<"Role Name :"<<name<<"_"<<roleid<<"_"<<level<<endl;
                
                rspQuery.set_roleid(  roleid );
                rspQuery.set_name( name );
                rspQuery.set_level( level );
                return DBErr_OK;
            }
            else{
                return DBErr_NotExist;
            }
            
        } catch ( sql::SQLException& e ) {
            std:cerr<<"Err: SQLException in:"<<e.getErrorCode()<<"_"<<e.getSQLState()<<endl;
            return DBErr_Fail;
        }
        
    }();
    
    onRsp( queryID, reqType, res, rspQuery );
}

void DBMgr::onRsp( int queryID, const DBResponse& rsp )
{
    if( m_recvFun )
    {
        m_recvFun( queryID, rsp );
    }
}
