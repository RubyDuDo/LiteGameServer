//
//  TimeService_test.cpp
//  RubyGameServer
//
//  Created by pinky on 2025-04-18.
//

#include "gtest/gtest.h"
#include "Utils/TimeService.hpp"
#include <set>
#include <chrono>
using namespace std::chrono_literals;

//TEST( Test Suite, Test Case)

TEST(TimeServiceTest, FirstTimeUpdate)
{
    //parepare
    CTimeService timeS;
    TimePoint now = std::chrono::steady_clock::now();
    
    timeS.update( now );
    
    auto getNow = timeS.getCurTime();
    
    EXPECT_EQ( now, getNow );
    EXPECT_EQ( timeS.getDeltaMS().count(), 0 );
}

TEST(TimeServiceTest, TwoTimeUpdate)
{
    //parepare
    CTimeService timeS;
    TimePoint now = std::chrono::steady_clock::now();
    
    timeS.update( now );
    
    auto next = now + 20ms;
    timeS.update(next);
    
    auto getNow = timeS.getCurTime();
    
    EXPECT_EQ( next, getNow );
    EXPECT_EQ( timeS.getDeltaMS().count(), 20 );
}
