#include "gtest/gtest.h"
#include "Utils/IDGenerator.hpp"

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
    EXPECT_NE( id1, id2 );
    EXPECT_NE( id3, id2 );
    EXPECT_NE( id1, id3 );

    EXPECT_GT(id1, 0); // 期望 id1 大于 0
    EXPECT_GT(id2, 0);
    EXPECT_GT(id3, 0);
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
