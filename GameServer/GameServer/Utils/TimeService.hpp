//
//  TimeService.hpp
//  GameServer
//
//  Created by pinky on 2025-04-16.
//

#ifndef TimeService_hpp
#define TimeService_hpp

#include <chrono>
#include <stdio.h>
using TimePoint = std::chrono::steady_clock::time_point;
using TimeDuration = std::chrono::milliseconds;

class ITimeService
{
public:
    virtual void update() = 0 ;
    virtual TimePoint getCurTime() = 0 ;
    virtual TimeDuration getDeltaMS() = 0 ;
    
    virtual ~ITimeService() =default;
};

class CTimeService : public ITimeService
{
public:
    virtual void update() override;
    virtual TimePoint getCurTime() override;
    virtual TimeDuration getDeltaMS() override;
private:
    TimePoint m_curTime;
    TimeDuration m_delta;
};

#endif /* TimeService_hpp */
