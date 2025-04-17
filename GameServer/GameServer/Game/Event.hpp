//
//  Event.hpp
//  GameServer
//
//  Created by pinky on 2025-04-16.
//

#ifndef Event_hpp
#define Event_hpp

#include <stdio.h>

enum class EventType{
    Evt_InValid,
    Evt_Connect,
    Evt_Disconnect,
};

class Event
{
public:
    Event():m_type(EventType::Evt_InValid){};
    Event( EventType type ): m_type(type){};
    EventType m_type;
};

class EventConnect : public Event
{
public:
    EventConnect( int sockID ): Event( EventType::Evt_Connect), m_sockID(sockID){};
    int m_sockID;
};

class EventDisconnect : public Event
{
public:
    EventDisconnect( int sockID ): Event( EventType::Evt_Disconnect), m_sockID(sockID){};
    int m_sockID;
};

#endif /* Event_hpp */
