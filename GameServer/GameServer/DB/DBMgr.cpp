//
//  DBMgr.cpp
//  GameServer
//
//  Created by pinky on 2025-02-28.
//

#include "DBMgr.hpp"

//#include <jdbc/mysql_driver.h>
//#include <jdbc/mysql_connection.h>
//#include <jdbc/cppconn/statement.h>
//#include <jdbc/cppconn/resultset.h>
//#include <jdbc/cppconn/prepared_statement.h>
//#include <jdbc/cppconn/exception.h>

#include "mysql/jdbc.h"

void DBMgr::InitDB( const std::string& url, const std::string& user,
            const std::string& passwd, const std::string& database)
{
    
    try {
        //connect to db
        sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
        
        std::unique_ptr<sql::Connection> conn( driver->connect(url, user, passwd));
        
        conn->setSchema( database );
        
        std::unique_ptr<sql::Statement> stmt( conn->createStatement());
        
        string query = "Select * from accounts; ";
        
        std::unique_ptr<sql::ResultSet> res( stmt->executeQuery(query));
        
        while( res->next() )
        {
            std::string name = res->getString( "account" );
            cout<<"Account Name :"<<name<<endl;
        }
        
    } catch ( sql::SQLException& e ) {
        std:cerr<<"Err: SQLException in:"<<e.getErrorCode()<<"_"<<e.getSQLState()<<endl;
    }
    
    //start query thread
}

void DBMgr::QueryThread()
{
    
}

void DBMgr::queryAccount( const std::string& strName )
{
    
}

void DBMgr::insertRole()
{
    
}
