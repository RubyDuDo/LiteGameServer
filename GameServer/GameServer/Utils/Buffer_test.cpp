//
//  Buffer_test.cpp
//  run_tests
//
//  Created by pinky on 2025-04-18.
//

#include <stdio.h>
#include <stdio.h>
#include "gtest/gtest.h"
#include "Utils/Buffer.hpp"

#include <string>
using namespace std;

TEST(BufferTest, AddData)
{
    const int MAX_BUFF_SIZE = 2048;
    char arr[MAX_BUFF_SIZE]  = {0};
    RingBuffer buff(MAX_BUFF_SIZE);
    //parepare
    
    string str("Hello world!");
    int inputLength = str.length();

    EXPECT_EQ( buff.isEmpty(), true );
    //test after addData
    buff.addData( str.c_str(),  inputLength );

    EXPECT_EQ( buff.getSize(), str.length());
    EXPECT_EQ( buff.isEmpty(), false );
    
    //test getData
    int size = buff.getData( arr, MAX_BUFF_SIZE );
    EXPECT_EQ( size ,  inputLength );
    
    size = buff.getSize();
    auto leftSize = buff.getFreeSpaceSize();
    EXPECT_EQ( size, inputLength );
    EXPECT_EQ( leftSize, MAX_BUFF_SIZE - inputLength );
    
    string strGet(arr);
    EXPECT_EQ( strGet, str);
    
    //test after consume  data
    buff.consumeData( size );
    size = buff.getSize();
    leftSize = buff.getFreeSpaceSize();
    
    EXPECT_EQ( size, 0 );
    EXPECT_EQ( leftSize, MAX_BUFF_SIZE );
    EXPECT_EQ( buff.isEmpty(), true );
}

TEST(BufferTest, MultiAddData)
{
    const int MAX_BUFF_SIZE = 2048;
    char arr[MAX_BUFF_SIZE]  = {0};
    RingBuffer buff(MAX_BUFF_SIZE);
    //parepare
    
    string str("Hello world!");
    string str2("Ruby");
    int inputLength =str.length();
    int inputLength2 = str2.length();
    int totalLength = inputLength + inputLength2;

    //test after addData
    buff.addData( str.c_str(),  inputLength );
    buff.addData( str2.c_str(), inputLength2 );
    
    //test getData
    int size = buff.getData( arr, MAX_BUFF_SIZE );
    EXPECT_EQ( size ,  totalLength );
    
    size = buff.getSize();
    auto leftSize = buff.getFreeSpaceSize();
    EXPECT_EQ( size, totalLength );
    EXPECT_EQ( leftSize, MAX_BUFF_SIZE - totalLength );
    
    string strGet(arr);
    EXPECT_EQ( strGet, str + str2 );
    
    //test after consume  data
    buff.consumeData( size );
    size = buff.getSize();
    leftSize = buff.getFreeSpaceSize();
    
    EXPECT_EQ( size, 0 );
    EXPECT_EQ( leftSize, MAX_BUFF_SIZE );

}
//test multi add cause exceedLength, but every single is within the length
TEST(BufferTest, ExceedMaxLength)
{
    const int MAX_BUFF_SIZE = 10;
    char arr[MAX_BUFF_SIZE]  = {0};
    RingBuffer buff(MAX_BUFF_SIZE);
    //parepare
    
    string str("Hello!");
    int inputLength =str.length();

    buff.addData( str.c_str(),  inputLength );
    
    int size = buff.getData( arr , MAX_BUFF_SIZE );
    buff.consumeData( size );
    
    buff.addData( str.c_str(),  inputLength );
    size = buff.getData( arr , MAX_BUFF_SIZE );
    buff.consumeData( size );
    
    EXPECT_EQ( size, inputLength );
    string strGet(arr);
    EXPECT_EQ( strGet, str );
}

//test the total length exceed, Does it need to extend dynamically?
TEST(BufferTest, TotalExceedMaxLength)
{
    //todo, right now ,it doesn't support
}
