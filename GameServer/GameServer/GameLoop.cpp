//
//  GameLoop.cpp
//  GameServer
//
//  Created by pinky on 2025-02-19.
//

#include "GameLoop.hpp"
#include <iostream>
#include <thread>
using namespace std;

#include "NetworkMgr.hpp"

bool GameLoop::Init()
{
    bool ret = NetworkMgr::getInstance()->InitNetwork();
    return ret;
}

bool GameLoop::run()
{
    bool ret = Init();
    if( !ret )
    {
        cout<<"Init Failed!"<<endl;
        return ret;
    }
    else{
        cout<<"Init succeed!"<<endl;
    }
    
    cout<<"Run ... "<<endl;
    
    while(true)
    {
        std::this_thread::sleep_for( 20ms );
    }
    
    //
    return true;
}
