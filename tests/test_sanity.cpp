/**
 * @file test_sanity.cpp
 * @brief Sanity test to verify the build system and test framework are working
 */

#include <gtest/gtest.h>

/**
 * @brief Basic sanity test
 * 
 * This test verifies that:
 * - The build system is properly configured
 * - Google Test is correctly linked
 * - The test infrastructure is functional
 */
TEST(SanityTest, BasicAssertion) {
    EXPECT_EQ(1 + 1, 2);
    EXPECT_TRUE(true);
    EXPECT_FALSE(false);
}

/**
 * @brief Test that strings work
 */
TEST(SanityTest, StringComparison) {
    std::string hello = "Hello";
    std::string world = "World";
    
    EXPECT_EQ(hello, "Hello");
    EXPECT_NE(hello, world);
    EXPECT_EQ(hello + " " + world, "Hello World");
}

/**
 * @brief Test that basic math works
 */
TEST(SanityTest, BasicMath) {
    EXPECT_EQ(2 * 2, 4);
    EXPECT_GT(5, 3);
    EXPECT_LT(3, 5);
    EXPECT_GE(5, 5);
    EXPECT_LE(3, 3);
}

/**
 * @brief Main entry point for the test executable
 */
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
