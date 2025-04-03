//
//  DBQueryHandler.hpp
//  GameServer
//
//  Created by pinky on 2025-04-03.
//

#ifndef DBQueryHandler_hpp
#define DBQueryHandler_hpp

#include <stdio.h>
#include "DB/DBMgr.hpp"

class DBQueryHandler : public IDBQueryHandler
{
public:
    DBQueryHandler();
    virtual ~DBQueryHandler();
    
    virtual void onQuery( int queryID, std::unique_ptr<DBRequest>&& req);

private:
    void queryAccount( int queryID,  const DBRequest& req );
    void addRole( int queryID, const DBRequest& req);
    void queryRole( int queryID, const DBRequest& req );
};

#endif /* DBQueryHandler_hpp */
