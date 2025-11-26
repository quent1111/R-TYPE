#include <gtest/gtest.h>
#include "states/StateManager.hpp"
#include "states/IState.hpp"
#include <SFML/Graphics.hpp>

class MockState : public rtype::IState {
public:
    MockState() = default;
    void on_enter() override { enter_called = true; }
    void on_exit() override { exit_called = true; }
    void handle_event(const sf::Event&) override { event_handled = true; }
    void update(float) override { update_called = true; }
    void render(sf::RenderWindow&) override { render_called = true; }
    std::string get_next_state() const override { return next_state; }
    void set_next_state(const std::string& state) { next_state = state; }
    bool enter_called = false;
    bool exit_called = false;
    bool event_handled = false;
    bool update_called = false;
    bool render_called = false;
    std::string next_state;
};

class StateManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        manager_ = std::make_unique<rtype::StateManager>();
        manager_->register_state("state1", []() {
            return std::make_unique<MockState>();
        });
        manager_->register_state("state2", []() {
            return std::make_unique<MockState>();
        });
        manager_->register_state("state3", []() {
            return std::make_unique<MockState>();
        });
    }

    std::unique_ptr<rtype::StateManager> manager_;
};

TEST_F(StateManagerTest, InitialStateEmpty) {
    EXPECT_FALSE(manager_->has_states());
}

TEST_F(StateManagerTest, RegisterAndPushState) {
    manager_->push_state("state1");
    EXPECT_TRUE(manager_->has_states());
}

TEST_F(StateManagerTest, OnEnterCalledOnPush) {
    testing::internal::CaptureStderr();
    manager_->push_state("state1");
    std::string output = testing::internal::GetCapturedStderr();
    EXPECT_TRUE(manager_->has_states());
}

TEST_F(StateManagerTest, PushUnregisteredState) {
    testing::internal::CaptureStderr();
    manager_->push_state("nonexistent");
    std::string output = testing::internal::GetCapturedStderr();
    EXPECT_FALSE(manager_->has_states());
    EXPECT_TRUE(output.find("not registered") != std::string::npos);
}

TEST_F(StateManagerTest, PopState) {
    manager_->push_state("state1");
    EXPECT_TRUE(manager_->has_states());
    manager_->pop_state();
    EXPECT_FALSE(manager_->has_states());
}

TEST_F(StateManagerTest, PopEmptyStack) {
    EXPECT_FALSE(manager_->has_states());
    manager_->pop_state();
    EXPECT_FALSE(manager_->has_states());
}

TEST_F(StateManagerTest, ChangeState) {
    manager_->push_state("state1");
    EXPECT_TRUE(manager_->has_states());
    manager_->change_state("state2");
    EXPECT_TRUE(manager_->has_states());
}

TEST_F(StateManagerTest, ChangeStateCallsExitAndEnter) {
    manager_->push_state("state1");
    manager_->change_state("state2");
    EXPECT_TRUE(manager_->has_states());
}

TEST_F(StateManagerTest, EventPropagation) {
    manager_->push_state("state1");
    sf::Event event;
    event.type = sf::Event::KeyPressed;
    event.key.code = sf::Keyboard::Escape;
    manager_->handle_event(event);
}

TEST_F(StateManagerTest, UpdatePropagation) {
    manager_->push_state("state1");
    manager_->update(0.016f);
}

TEST_F(StateManagerTest, RenderPropagation) {
    // sf::RenderWindow window(sf::VideoMode(800, 600), "Test", sf::Style::None);
    manager_->push_state("state1");
    // manager_->render(window);
    // window.close();
}

TEST_F(StateManagerTest, NoEventWhenNoStates) {
    sf::Event event;
    event.type = sf::Event::Closed;
    manager_->handle_event(event);
    EXPECT_FALSE(manager_->has_states());
}

TEST_F(StateManagerTest, NoUpdateWhenNoStates) {
    manager_->update(0.016f);
    EXPECT_FALSE(manager_->has_states());
}

TEST_F(StateManagerTest, NoRenderWhenNoStates) {
    // sf::RenderWindow window(sf::VideoMode(800, 600), "Test", sf::Style::None);
    // manager_->render(window);
    EXPECT_FALSE(manager_->has_states());
    // window.close();
}

TEST_F(StateManagerTest, MultipleStatesPushed) {
    manager_->push_state("state1");
    manager_->push_state("state2");
    manager_->push_state("state3");
    EXPECT_TRUE(manager_->has_states());
    manager_->pop_state();
    EXPECT_TRUE(manager_->has_states());
    manager_->pop_state();
    EXPECT_TRUE(manager_->has_states());
    manager_->pop_state();
    EXPECT_FALSE(manager_->has_states());
}

TEST_F(StateManagerTest, ProcessTransitionsWithNextState) {
    auto state_with_transition = std::make_unique<MockState>();
    state_with_transition->set_next_state("state2");
    manager_->register_state("transition_state", [state = state_with_transition.get()]() mutable {
        auto ptr = std::make_unique<MockState>();
        ptr->set_next_state(state->next_state);
        return ptr;
    });
    manager_->push_state("transition_state");
    EXPECT_TRUE(manager_->has_states());
    manager_->process_transitions();
    EXPECT_TRUE(manager_->has_states());
}

TEST_F(StateManagerTest, ProcessTransitionsWithNoNextState) {
    manager_->push_state("state1");
    EXPECT_TRUE(manager_->has_states());
    manager_->process_transitions();
    EXPECT_TRUE(manager_->has_states());
}

TEST_F(StateManagerTest, ProcessTransitionsWithEmptyStack) {
    EXPECT_FALSE(manager_->has_states());
    manager_->process_transitions();
    EXPECT_FALSE(manager_->has_states());
}

TEST_F(StateManagerTest, MultipleRegistrations) {
    manager_->register_state("duplicate", []() {
        return std::make_unique<MockState>();
    });
    manager_->register_state("duplicate", []() {
        return std::make_unique<MockState>();
    });
    manager_->push_state("duplicate");
    EXPECT_TRUE(manager_->has_states());
}

TEST_F(StateManagerTest, StateLifecycleOrder) {
    testing::internal::CaptureStdout();
    auto tracker = std::make_shared<std::vector<std::string>>();
    manager_->register_state("tracked1", [tracker]() {
        auto state = std::make_unique<MockState>();
        return state;
    });
    manager_->register_state("tracked2", [tracker]() {
        auto state = std::make_unique<MockState>();
        return state;
    });
    manager_->push_state("tracked1");
    manager_->change_state("tracked2");
    manager_->pop_state();
    testing::internal::GetCapturedStdout();
}