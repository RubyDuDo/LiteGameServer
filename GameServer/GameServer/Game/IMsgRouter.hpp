//
//  IMsgRouter.hpp
//  RubyGameServer
//
//  Created by pinky on 2025-04-16.
//
#ifndef IMSGROUTER_HPP
#define IMSGROUTER_HPP

#include <stdio.h>
#include <memory>
#include "../proto/dbmsg.pb.h"
#include "Event.hpp"

class IMsgRouter{
public:
    //Inner Event
    virtual void sendEvent( std::unique_ptr<Event>&& evt ) = 0;
    //DBQuery
    virtual void addDBQuery( std::unique_ptr<DBRequest>&& req, std::function<void( const DBResponse&)> func ) = 0;
    
    virtual ~IMsgRouter() = default;
};

#endif /*Event_hpp*/
