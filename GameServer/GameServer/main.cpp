//
//  main.cpp
//  GameServer
//
//  Created by pinky on 2025-02-19.
//

#include <iostream>
#include "GameLoop.hpp"

int main(int argc, const char * argv[]) {
    // insert code here...
    std::cout << "Hello, World!\n";
    
    GameLoop game;
    game.run();
    
    return 0;
}
