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
        
        std::unique_ptr<sql::PreparedStatement> stmt(
            getSqlConn()->prepareStatement("SELECT * FROM accounts WHERE account = ?")
        );
        stmt->setString(1, query.account());
    
        std::unique_ptr<sql::ResultSet> res( stmt->executeQuery());
        
        if( res->next() )
        {
            std::string name = res->getString( "account" );
            std::string passwd = res->getString("passwd");
            uint64_t roleid = res->getUInt64("roleid");
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
        std::unique_ptr<sql::PreparedStatement> stmt(
            getSqlConn()->prepareStatement("SELECT * FROM roles WHERE roleid = ?")
        );
        stmt->setUInt64(1, query.roleid());
        SPDLOG_DEBUG("QueryRole roleid: {}", query.roleid());
    
        std::unique_ptr<sql::ResultSet> res( stmt->executeQuery());
        
        if( res->next() )
        {
            std::string name = res->getString( "name" );
            int level = res->getInt("level");
            uint64_t roleid = res->getUInt64("roleid");
            SPDLOG_DEBUG("Role Name:{}, ID:{}, level:{}", name, roleid, level );
            
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
        
        uint64_t roleID = query.roleid();
        
        try {

            //step 1： insert a new role
            SPDLOG_DEBUG("Insert role: roleid={}, name={}", query.roleid(), query.name());
            
            std::unique_ptr<sql::PreparedStatement> stmtInsert(
                getSqlConn()->prepareStatement("INSERT INTO roles (roleid, name) VALUES (?, ?)")
            );
            stmtInsert->setUInt64(1, query.roleid());
            stmtInsert->setString(2, query.name());
            
            int rowAffected = stmtInsert->executeUpdate();
            if( rowAffected == 0 )
            {
                SPDLOG_ERROR("Insert role fail");
                return DBErr_Fail;
            }

            
            //step 3: modify the account table
            std::unique_ptr<sql::PreparedStatement> stmtUpdate(
                getSqlConn()->prepareStatement("UPDATE accounts SET roleid = ? WHERE account = ?")
            );
            stmtUpdate->setUInt64(1, query.roleid());
            stmtUpdate->setString(2, query.name());
            SPDLOG_DEBUG("Update account roleid={}, name={}", query.roleid(), query.name());
            rowAffected = stmtUpdate->executeUpdate();
            if( rowAffected == 0 )
            {
                SPDLOG_ERROR("Update account fail!");
                return DBErr_Fail;
            }
            
            //step 4: get the whole roleinfo from db
            std::unique_ptr<sql::PreparedStatement> stmtSelect(
                getSqlConn()->prepareStatement("SELECT * FROM roles WHERE roleid = ?")
            );
            stmtSelect->setUInt64(1, roleID);
            SPDLOG_DEBUG("Query role by roleID={}", roleID);
            std::unique_ptr<sql::ResultSet> resQuery( stmtSelect->executeQuery());
            
            if( resQuery->next() )
            {
                std::string name = resQuery->getString( "name" );
                int level = resQuery->getInt("level");
                uint64_t roleid = resQuery->getInt("roleid");
                SPDLOG_DEBUG("Role Name:{}, ID:{}, level:{}", name, roleid, level );
                
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


