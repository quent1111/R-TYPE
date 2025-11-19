/**
 * @file test_sanity.cpp
 * @brief Sanity test to verify the build system and test framework are working
 * 
 * This is a minimal test that ensures:
 * - CMake configuration is correct
 * - Google Test is properly linked
 * - The test infrastructure can compile and run
 * 
 * @note This test should ALWAYS pass. If it fails, the build system is broken.
 */

#include <gtest/gtest.h>

/**
 * @brief Minimal sanity check
 * 
 * Verifies that the test framework is functional.
 * This test intentionally has no real logic - it just needs to compile and run.
 */
TEST(SanityTest, FrameworkWorks) {
    EXPECT_TRUE(true) << "If this fails, something is very wrong!";
}

/**
 * @brief Main entry point for the test executable
 */
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
