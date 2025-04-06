//
//  DataDefine.hpp
//  GameServer
//
//  Created by pinky on 2025-04-05.
//

#ifndef DataDefine_hpp
#define DataDefine_hpp

#include <stdio.h>
#include <string>
#include "../../Utils/json.hpp"

class IInfo{
public:
    int getID() const { return 0; }
};

class MonsterInfo : public IInfo
{
public:
    int MonsterID = 0;
    int Level = 0;
    int HP = 0;
    std::string name;
    std::string Res;

    int getID() const { return MonsterID; }
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MonsterInfo, MonsterID, Level, HP, name, Res)

#endif /* DataDefine_hpp */
