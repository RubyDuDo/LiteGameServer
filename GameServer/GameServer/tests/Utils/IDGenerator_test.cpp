#include "gtest/gtest.h"
#include "Utils/IDGenerator.hpp"
#include <set>

//TEST( Test Suite, Test Case)

TEST(IDGeneratorTest, GeneratesUniqueIDs)
{
    //parepare
    IDGenerator idGen;
    idGen.init(1);

    uint64_t id1 = idGen.getNextID();
    uint64_t id2 = idGen.getNextID();
    uint64_t id3 = idGen.getNextID();

    // Assert
    EXPECT_GT( id2, id1 );
    EXPECT_GT( id3, id2 );
}

//unique between different server
TEST(IDGeneratorTest, UniqueWithTwoServer)
{
    //parepare
    IDGenerator idGen;
    idGen.init(1);
    IDGenerator idGen2;
    idGen2.init(3);

    uint64_t id1 = idGen.getNextID();
    uint64_t id2 = idGen2.getNextID();

    // Assert
    EXPECT_NE( id1, id2 );
}

TEST(IDGeneratorTest, InvalidServerID)
{
    //parepare
    IDGenerator idGen;
    bool bInit = idGen.init(1024);
    
    EXPECT_EQ( bInit, false );
}

//make sure even the peak time, the id generator won't generate same ids
TEST(IDGeneratorTest, TestWaitWhenPeak )
{
    //parepare
    IDGenerator idGen;
    bool bInit = idGen.init(1);
    
    std::set<uint64_t> ids;
    const int genTime = 8096;
    for( int i = 0 ; i < genTime; i++  )
    {
        ids.insert( idGen.getNextID() );
    }
    
    EXPECT_EQ( ids.size(), genTime );
}



// 注意：如果你的测试需要共享设置或清理代码，可以使用 Test Fixtures (TEST_F)
class IDGeneratorFixture : public ::testing::Test {
protected:
   void SetUp() override {
       // 在每个测试用例运行前执行
       idGen_ = new IDGenerator();
   }

   void TearDown() override {
       // 在每个测试用例运行后执行
       delete idGen_;
   }

   IDGenerator* idGen_;
};

TEST_F(IDGeneratorFixture, TestWithFixture) {
    idGen_->init(1);
    uint64_t id = idGen_->getNextID();
    EXPECT_GT(id, 0);
}
