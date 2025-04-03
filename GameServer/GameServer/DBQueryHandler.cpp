//
//  DBQueryHandler.cpp
//  GameServer
//
//  Created by pinky on 2025-04-03.
//

#include "DBQueryHandler.hpp"
#include "Utils/LoggerHelper.hpp"

DBQueryHandler::DBQueryHandler()
{
    
}

DBQueryHandler::~DBQueryHandler()
{
    
}

void DBQueryHandler::onQuery( int queryID, std::unique_ptr<DBRequest>&& req)
{
    if( getSqlConn() == nullptr )
    {
        SPDLOG_ERROR("DBConn is nullptr!");
        return;
    }
    
    switch( req->head().type() )
    {
        case DBReqType_QueryAccount:
            queryAccount( queryID,  *req );
            break;
        case DBReqType_ModAccount:
            break;
        case DBReqType_QueryRole:
            queryRole( queryID,  *req );
            break;
        case DBReqType_AddRole:
            addRole( queryID, *req );
            break;
        default:
            break;
    }
}

void DBQueryHandler::queryAccount( int queryID , const DBRequest& req )
{
    DBReqQueryAccount query;
    if( !query.ParseFromString( req.payload() ))
    {
        SPDLOG_ERROR(" Query Account Parse Fail!");
        return;
    }
    
    try {
        
        std::unique_ptr<sql::Statement> stmt( getSqlConn()->createStatement());
        
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
            
            onRsp( queryID, req.head().type(), DBErr_OK, rspQuery );
        }
        else{
            onRsp( queryID, req.head().type(), DBErr_NotExist );
        }
        
        
    } catch ( sql::SQLException& e ) {
        SPDLOG_ERROR("Err: SQLException in: {}, {}", e.getErrorCode(), e.getSQLState());
        return ;
    }
}

void DBQueryHandler::queryRole( int queryID, const DBRequest& req )
{
    DBReqQueryRole query;
    DBRspRole rspQuery;
    if( !query.ParseFromString( req.payload() ))
    {
        SPDLOG_ERROR(" Query Role Parse Fail!");
        onRsp( queryID, req.head().type(), DBErr_Fail, rspQuery );
        return;
    }
    
    try {
        std::unique_ptr<sql::Statement> stmt( getSqlConn()->createStatement());
        
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
        SPDLOG_ERROR("Err: SQLException in: {}, {}", e.getErrorCode(), e.getSQLState());
        onRsp( queryID, req.head().type(), DBErr_Fail, rspQuery );
        return ;
    }
    
}



void DBQueryHandler::addRole( int queryID, const DBRequest& req)
{
    DBReqAddRole query;
    DBRspRole rspQuery;
    
    DBReqType reqType = req.head().type();
    DBErrorType res = [&]() -> DBErrorType
    {
        if( !query.ParseFromString( req.payload() ))
        {
            SPDLOG_ERROR(" Query Add Role Parse Fail!");
            return DBErr_Fail;
        }
        
        try {

            //step 1： insert a new role
            string strQuery = std::format("insert into roles (name) values ('{}') ", query.name() );
            cout<<strQuery<<endl;
            
            std::unique_ptr<sql::Statement> stmt( getSqlConn()->createStatement());
            
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
                SPDLOG_ERROR("Query Add Role insert Fail!");
                return DBErr_Fail;
            }
            
            //step 3: modify the account table
            strQuery = std::format("update accounts set role_id = {} where account = '{}'", roleid, query.name() );
            cout<<strQuery<<endl;
            rowAffected =  stmt->executeUpdate( strQuery );
            if( rowAffected == 0 )
            {
                SPDLOG_ERROR("Update account fail!");
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
            SPDLOG_ERROR("Err: SQLException in: {}, {}", e.getErrorCode(), e.getSQLState());
            return DBErr_Fail;
        }
        
    }();
    
    onRsp( queryID, reqType, res, rspQuery );
}


