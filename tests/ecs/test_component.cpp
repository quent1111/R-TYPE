

#include <gtest/gtest.h>

class DISABLED_ComponentTest : public ::testing::Test {
protected:
    void SetUp() override {
    }

    void TearDown() override {
    }
};

TEST_F(DISABLED_ComponentTest, AddComponent) {
    GTEST_SKIP() << "Not implemented yet";
}

TEST_F(DISABLED_ComponentTest, RemoveComponent) {
    GTEST_SKIP() << "Not implemented yet";
}

TEST_F(DISABLED_ComponentTest, GetComponent) {
    GTEST_SKIP() << "Not implemented yet";
}

TEST_F(DISABLED_ComponentTest, HasComponent) {
    GTEST_SKIP() << "Not implemented yet";
}
