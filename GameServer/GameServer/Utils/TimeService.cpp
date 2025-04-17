//
//  TimeService.cpp
//  GameServer
//
//  Created by pinky on 2025-04-16.
//

#include "TimeService.hpp"

CTimeService::CTimeService():m_curTime(std::chrono::steady_clock::now())
,m_delta(std::chrono::milliseconds::zero())
{
}
void CTimeService::update()
{
    auto last = m_curTime;
    m_curTime = std::chrono::steady_clock::now();
    m_delta = std::chrono::duration_cast<std::chrono::milliseconds>(m_curTime - last);
}

TimePoint CTimeService::getCurTime()
{
    return m_curTime;
}

TimeDuration CTimeService::getDeltaMS()
{
    return m_delta;
}
