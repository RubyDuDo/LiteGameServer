//
//  Player.hpp
//  GameServer
//
//  Created by pinky on 2025-02-28.
//

#ifndef Player_hpp
#define Player_hpp

#include <stdio.h>
#include <string>
using namespace std;

enum PlayerState
{
    Player_BeforeLogin,
    Player_Login,
    Player_Logout,
};

class Player
{
public:
    Player() = default;
    
public:
    int m_roleID = 0;
    int m_level = 0;
    string m_strName;
    PlayerState m_state = Player_BeforeLogin;
};

#endif /* Player_hpp */
