//
//  DBMgr.hpp
//  GameServer
//
//  Created by pinky on 2025-02-28.
//

#ifndef DBMgr_hpp
#define DBMgr_hpp

#include <stdio.h>
#include "../MsgQueue.hpp"


class DBRequest
{
    
    
};

class DBResponse
{
    
};

class DBMgr
{
public:
    void InitDB( const std::string& url, const std::string& user,
                const std::string& passwd, const std::string& database);
    
    void QueryThread();
    
public:
    void queryAccount( const std::string& strName );
    void insertRole();
    
public:
    //处理返回结果
    
private:
    MsgQueue<DBRequest> m_msgReq;
    MsgQueue<DBResponse> m_msgRsp;
    
};

#endif /* DBMgr_hpp */
