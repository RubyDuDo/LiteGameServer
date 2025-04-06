//
//  DataMgr.hpp
//  GameServer
//
//  Created by pinky on 2025-04-05.
//

#ifndef DataMgr_hpp
#define DataMgr_hpp

#include <stdio.h>
#include <memory>
#include <fstream>
#include "DataDefine.hpp"
#include "../../Utils/LoggerHelper.hpp"
#include "../../Utils/json.hpp"
using json = nlohmann::json;

class DataMgr
{
public:
    static DataMgr* getInstance();
    
    DataMgr() = default;
    ~DataMgr() = default;
    
    bool initData();
    void clear();
    
    template <typename T>
    std::shared_ptr<T> getData( int id )
    {
        auto mapData = getMapForType<T>();
        if( mapData == nullptr)
        {
            return nullptr;
        }
        
        auto it = mapData->find(id);
        if(it != mapData->end())
        {
            return it->second;
        
        }
        else{
            SPDLOG_ERROR("Data not found for id: {}", id);
            return nullptr;
        }
    }
    
private:
    template <typename T>
    bool loadDatas( const std::string& fileName ){
        json jdatas;
        try{
            std::ifstream file(fileName);
            if(!file.is_open()) {
                SPDLOG_ERROR("Failed to open file: {}", fileName);
                return false;
            }
            
            auto mapData = getMapForType<T>();
            if( mapData == nullptr)
            {
                SPDLOG_ERROR("Data map not found for type");
                return false;
            }
            
            
            jdatas = json::parse( file);
            SPDLOG_INFO("Load config file: {}", fileName);
            
            for (auto& jdata : jdatas ) {
                T data = jdata.get<T>();
                (*mapData)[data.getID()] = std::make_shared<T>(data);
            }
            
            return true;
            
        }catch (json::parse_error& e) {
            SPDLOG_ERROR("JSON parse error: {},exception id:{},byte position of error:{}"  , e.what(), e.id, e.byte);
            return false;
        } catch (std::exception& e) {
            SPDLOG_ERROR("Error reading JSON file: {}", e.what());
            return 1;
        }
    }

    
    template <typename T>
    auto* getMapForType(){
        if constexpr (std::is_same_v<T, MonsterInfo>) {
            return &m_mapMonsterData;
            //                } else if constexpr (std::is_same_v<T, Skill>) {
            //                    return &m_mapSkillData;
            //                } else if constexpr (std::is_same_v<T, Item>) {
            //                    return &m_mapItemData;
        } else {
            static_assert(false, "Unsupported type requested in getData");
            return nullptr;
        }
    }
private:
    static DataMgr* m_pInstance;
    
    std::map<int, std::shared_ptr<MonsterInfo> > m_mapMonsterData;
  
    //just to support getMapForType
    std::map<int,int> m_mapInvalid;
};

#endif /* DataMgr_hpp */
