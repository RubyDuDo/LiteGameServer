//
//  DataMgr.cpp
//  GameServer
//
//  Created by pinky on 2025-04-05.
//

#include "DataMgr.hpp"
#include "../../Utils/json.hpp"
#include "../../Utils/LoggerHelper.hpp"

DataMgr* DataMgr::m_pInstance = nullptr;
using json = nlohmann::json;

DataMgr* DataMgr::getInstance()
{
    if(!m_pInstance)
    {
        m_pInstance = new DataMgr();
    }
    return m_pInstance;
}

//
//<template typename T>
//bool DataMgr::loadDatas( const std::string& fileName )
//{
//    json jdatas;
//    try{
//        std::ifstream file(fileName);
//        if(!file.is_open()) {
//            SPDLOG_ERROR("Failed to open file: {}", fileName);
//            return false;
//        }
//        
//        auto mapData = getMapForType<Monster>();
//        if( mapData == nullptr)
//        {
//            SPDLOG_ERROR("Data map not found for type");
//            return false;
//        }
//        
//        
//        jdatas = json.parse( file);
//        SPDLOG_INFO("Load config file: {}", fileName);
//        
//        for (auto& jdata : jdatas ) {
//            T data = jdata.get<T>();
//            (*mapData)[data.getID()] = data;
//        }
//        
//        return true;
//        
//    }catch (json::parse_error& e) {
//        SPDLOG_ERROR("JSON parse error: {},exception id:{},byte position of error:{}"  , e.what(), e.id, e.byte);
//        return false;
//    } catch (std::exception& e) {
//        SPDLOG_ERROR("Error reading JSON file: {}", e.what());
//        return 1;
//    }
//}

bool DataMgr::initData()
{
    bool ret = true;
    ret = ret && loadDatas<MonsterInfo>("./data/monster.json");
    
    return ret;
}

void DataMgr::clear()
{
    m_mapMonsterData.clear();
}

