//
//  main.cpp
//  GameServer
//
//  Created by pinky on 2025-02-19.
//

#include <iostream>
#include <unistd.h>   // for getcwd
#include <limits.h>   // for PATH_MAX
#include "GameLoop.hpp"

GameLoop* m_pGame = nullptr;

int main(int argc, const char * argv[]) {
    // insert code here...
    std::cout << "Hello, World!\n";

    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    printf("Current working dir: %s\n", cwd);
    
    GameLoop game;
    m_pGame = &game;
    game.run();
    
    return 0;
}
