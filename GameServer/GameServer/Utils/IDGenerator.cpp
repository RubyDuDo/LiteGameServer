//
//  IDGenerator.cpp
//  RubyGameServer
//
//  Created by pinky on 2025-04-07.
//

#include "IDGenerator.hpp"
#include "LoggerHelper.hpp"
#include <chrono>

bool IDGenerator::init( int serverID, int64_t epochTimestamp )
{
    if (serverID < 0 || serverID > maxServerId) {
        SPDLOG_ERROR("Server ID must be between 0 and {}, cur is:{}", maxServerId, serverID);
        return false;
    }
    if (epochTimestamp > getCurrentTimeMillis()) {
        SPDLOG_ERROR("Epoch timestamp cannot be in the future.");
        return false;
    }
    
    m_serverID = serverID;
    m_epochTimestamp = epochTimestamp;
    
    return true;
}

uint64_t IDGenerator::getNextID()
{
    std::lock_guard<std::mutex> lock(mutex_);
    int64_t currentTimestamp = getCurrentTimeMillis();
    // Clock moved backwards check
    if (currentTimestamp < m_lastTimestamp) {
        // Optional: Log this event. A small difference might be NTP sync,
        // a large one indicates a bigger problem. Depending on requirements,
        // you might wait instead of throwing for small discrepancies.
        SPDLOG_ERROR("Clock moved backwards. Refusing to generate ID for {} milliseconds", currentTimestamp);

        throw std::runtime_error("Clock moved backwards. Refusing to generate ID for "
                                 + std::to_string(m_lastTimestamp - currentTimestamp) + " milliseconds");
    }
    
    if (currentTimestamp == m_lastTimestamp) {
        // Same millisecond, increment sequence
        m_sequeceID = (m_sequeceID + 1) & sequenceMask; // Wrap around using mask
        if (m_sequeceID == 0) {
            SPDLOG_INFO("Sequence overflowed, waiting for next millisecond");
            // Sequence overflowed (4096 IDs generated in this millisecond!)
            // Block until next millisecond
            currentTimestamp = waitNextMillis(currentTimestamp);
        }
    } else {
        // New millisecond, reset sequence
        m_sequeceID = 0LL;
    }
    
    // Update last timestamp
    m_lastTimestamp = currentTimestamp;
    
    

    // Assemble the 64-bit ID
    // (currentTimestamp - epoch_) fits in 41 bits
    // workerId_ fits in 10 bits
    // sequence_ fits in 12 bits
    uint64_t id = ((currentTimestamp - m_epochTimestamp) << timestampShift) |
    ( m_serverID << serverIdShift) | m_sequeceID;

    return id;
}


int64_t IDGenerator::getCurrentTimeMillis() const {
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
}


int64_t IDGenerator::waitNextMillis(int64_t lastTs) const {
    int64_t currentTimestamp = getCurrentTimeMillis();
    while (currentTimestamp <= lastTs) {
        // Consider a very brief sleep to avoid pegging the CPU,
        // though pure busy-wait gives lowest latency *if* the wait is short.
        // std::this_thread::sleep_for(std::chrono::microseconds(100)); // Optional small sleep
        currentTimestamp = getCurrentTimeMillis();
    }
    return currentTimestamp;
}

