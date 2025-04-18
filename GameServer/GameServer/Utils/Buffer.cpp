//
//  Buffer.cpp
//  GameClient
//
//  Created by pinky on 2025-02-19.
//

#include "Buffer.hpp"
#include <cstring>
#include <algorithm>


RingBuffer::RingBuffer( int size ):
m_capacity(size),
m_buffer(size),
m_readPos(0),
m_writePos(0)
{
    if (size <= 0) {
        throw std::invalid_argument("Buffer size must be positive");
    }
}

bool RingBuffer::addData( const char* pData, int len  )
{
    if (len <= 0 || pData == nullptr) {
        return false; // Nothing to add or invalid input
    }

    int availableSize = getFreeSpaceSize();

    if (len > availableSize) {
        return false; // Not enough space
    }

    // Check for wrap-around write
    if (m_writePos + len <= m_capacity) {
        // No wrap-around: write in a single block
        std::memcpy(&m_buffer[m_writePos], pData, len);
    } else {
        // Wrap-around: write in two blocks
        int firstChunckLen = m_capacity - m_writePos;
        std::memcpy(&m_buffer[m_writePos], pData, firstChunckLen);

        int seconChunckLen = len - firstChunckLen;
        std::memcpy(&m_buffer[0], pData + firstChunckLen, seconChunckLen);
    }

    // Update write position
    m_writePos = (m_writePos + len) % m_capacity;

    return true;
    
}

int RingBuffer::getData( char* pBuff, int maxLen)
{
    if (maxLen <= 0 || pBuff == nullptr) {
        return 0; // Nothing to read or invalid input
    }

    int readableSize = getSize();
    int bytesToRead = std::min(maxLen, readableSize);

    if (bytesToRead == 0) {
        return 0; // Buffer is empty
    }

    // Check for wrap-around read
    if ( m_readPos + bytesToRead <= m_capacity) {
        // No wrap-around: read in a single block
        std::memcpy(pBuff, &m_buffer[m_readPos], bytesToRead);
    } else {
        // Wrap-around: read in two blocks
        int firstChunckLen = m_capacity - m_readPos;
        std::memcpy(pBuff, &m_buffer[m_readPos], firstChunckLen);

        int secondChunckLen = bytesToRead - firstChunckLen;
        std::memcpy(pBuff + firstChunckLen, &m_buffer[0], secondChunckLen);
    }

    // NOTE: Does NOT advance read_pos_, as per typical "getData" or "peek" semantics.
    // Use consumeData to advance the read position.
    return bytesToRead;
}

void RingBuffer::consumeData( int len )
{
    if (len <= 0) {
        return; // Nothing to consume
    }

    int bytesToConsume = std::min(len, getSize() ); // Don't consume more than available

    m_readPos = (m_readPos + bytesToConsume) % m_capacity;
}

int RingBuffer::getCapacity() const
{
    return m_capacity;
}

int RingBuffer::getSize()const
{
    if ( m_writePos >= m_readPos) {
        return m_writePos - m_readPos;
    } else {
        return m_capacity - m_readPos + m_writePos;
    }
}
int RingBuffer::getFreeSpaceSize() const
{
    return m_capacity - getSize();
}

bool RingBuffer::isEmpty() const
{
    return (getSize() == 0);
}
