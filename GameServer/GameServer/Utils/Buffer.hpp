//
//  Buffer.hpp
//  GameClient
//
//  Created by pinky on 2025-02-19.
//

#ifndef Buffer_hpp
#define Buffer_hpp

#include <stdio.h>
#include <vector>

class RingBuffer
{
public:
    RingBuffer( int size = 2048);
    
    bool addData( const char* pData, int len  );
    int getData( char* pBuff, int maxLen);
    void consumeData( int len );
    
    int getCapacity() const;
    int getSize()const;
    int getFreeSpaceSize() const;
    bool isEmpty() const;
    
private:
    int m_capacity;             // Total capacity of the buffer
    std::vector<char> m_buffer; // Underlying data storage
    int m_readPos;             // Read position index
    int m_writePos;            // Write position index
};

#endif /* Buffer_hpp */
