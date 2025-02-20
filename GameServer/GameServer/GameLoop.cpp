//
//  GameLoop.cpp
//  GameServer
//
//  Created by pinky on 2025-02-19.
//

#include "GameLoop.hpp"
#include <iostream>
using namespace std;

bool GameLoop::Init()
{
    return true;
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
    
    //
    return true;
}
