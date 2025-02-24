#include <gtest/gtest.h>

// En enkel testfall
TEST(SampleTest, AssertTrue) {
    ASSERT_TRUE(true);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
