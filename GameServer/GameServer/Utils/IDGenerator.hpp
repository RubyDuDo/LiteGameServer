//
//  IDGenerator.hpp
//  RubyGameServer
//
//  Created by pinky on 2025-04-07.
//

#ifndef IDGenerator_hpp
#define IDGenerator_hpp

#include <stdio.h>
#include <cstdint>
#include <mutex>

class IDGenerator
{
public:
    IDGenerator() = default;
    ~IDGenerator() = default;
    
    uint64_t getNextID();
    //default epochTimestamp is milliseconds since Unix epoch (Jan 1 1970 UTC)
    bool init( int serverID, int64_t epochTimestamp = 1672531200000LL );
    
public:
    static constexpr uint8_t timestampBits = 41;
    static constexpr uint8_t serverIdBits = 10;
    static constexpr uint8_t sequenceBits = 12;
    
    static constexpr uint8_t maxServerId = (1 << serverIdBits) - 1;
    static constexpr uint8_t sequenceMask = (1 << sequenceBits) - 1;
    
    // Bit Shifts
    static constexpr uint8_t serverIdShift = sequenceBits;
    static constexpr uint8_t timestampShift = sequenceBits + serverIdBits;
    
private:
    int64_t m_serverID;
    int64_t m_epochTimestamp;
    int64_t m_lastTimestamp;
    int64_t m_sequeceID;
    
    std::mutex mutex_;
    
    
    // --- Helper Methods ---
    int64_t getCurrentTimeMillis() const;
    // Busy-waits until the current time is greater than the last recorded timestamp
    int64_t waitNextMillis(int64_t lastTs) const;
};

#endif /* IDGenerator_hpp */
