#include <gtest/gtest.h>
#include <chrono>
#include <cmath>

// Mock Entity structure for testing
struct MockEntity {
    float x{0.f}, y{0.f};
    float vx{0.f}, vy{0.f};
    float prev_x{0.f}, prev_y{0.f};
    std::chrono::steady_clock::time_point prev_time;
    std::chrono::steady_clock::time_point curr_time;
    
    MockEntity() : prev_time(std::chrono::steady_clock::now()),
                   curr_time(prev_time) {}
};

// Helper function to calculate alpha (same logic as GameRenderer)
static float calculate_alpha(const MockEntity& e, 
                     std::chrono::steady_clock::time_point render_time) {
    if (e.curr_time <= e.prev_time) {
        return 1.0f;
    }
    
    const float total_ms =
        std::chrono::duration_cast<std::chrono::duration<float, std::milli>>(
            e.curr_time - e.prev_time
        ).count();
    
    const float elapsed_ms =
        std::chrono::duration_cast<std::chrono::duration<float, std::milli>>(
            render_time - e.prev_time
        ).count();
    
    return (total_ms > 0.0f) ? (elapsed_ms / total_ms) : 1.0f;
}

// Helper function to interpolate position
static float interpolate(float prev, float curr, float alpha) {
    alpha = std::max(0.0f, std::min(1.0f, alpha));
    return prev + (curr - prev) * alpha;
}

// ============================================================================
// BASIC INTERPOLATION TESTS
// ============================================================================

TEST(Interpolation, AlphaZeroAtStart) {
    MockEntity e;
    auto now = std::chrono::steady_clock::now();
    
    e.prev_time = now;
    e.curr_time = now + std::chrono::milliseconds(16);
    
    // Render exactly at prev_time
    float alpha = calculate_alpha(e, now);
    
    EXPECT_FLOAT_EQ(alpha, 0.0f);
}

TEST(Interpolation, AlphaOneAtEnd) {
    MockEntity e;
    auto now = std::chrono::steady_clock::now();
    
    e.prev_time = now;
    e.curr_time = now + std::chrono::milliseconds(16);
    
    // Render exactly at curr_time
    float alpha = calculate_alpha(e, e.curr_time);
    
    EXPECT_FLOAT_EQ(alpha, 1.0f);
}

TEST(Interpolation, AlphaHalfAtMiddle) {
    MockEntity e;
    auto now = std::chrono::steady_clock::now();
    
    e.prev_time = now;
    e.curr_time = now + std::chrono::milliseconds(16);
    
    // Render at midpoint (8ms)
    auto render_time = now + std::chrono::milliseconds(8);
    float alpha = calculate_alpha(e, render_time);
    
    EXPECT_NEAR(alpha, 0.5f, 0.01f);
}

TEST(Interpolation, LinearInterpolationHorizontal) {
    MockEntity e;
    e.prev_x = 100.0f;
    e.x = 200.0f;
    
    float alpha = 0.5f;
    float result = interpolate(e.prev_x, e.x, alpha);
    
    EXPECT_FLOAT_EQ(result, 150.0f);
}

TEST(Interpolation, LinearInterpolationVertical) {
    MockEntity e;
    e.prev_y = 50.0f;
    e.y = 150.0f;
    
    float alpha = 0.25f;
    float result = interpolate(e.prev_y, e.y, alpha);
    
    EXPECT_FLOAT_EQ(result, 75.0f);
}

TEST(Interpolation, NoMovementStaysInPlace) {
    MockEntity e;
    e.prev_x = 100.0f;
    e.x = 100.0f;
    
    for (float alpha = 0.0f; alpha <= 1.0f; alpha += 0.1f) {
        float result = interpolate(e.prev_x, e.x, alpha);
        EXPECT_FLOAT_EQ(result, 100.0f);
    }
}

// ============================================================================
// TIMING TESTS
// ============================================================================

TEST(Interpolation, StandardFrameTime16ms) {
    MockEntity e;
    auto now = std::chrono::steady_clock::now();
    
    e.prev_time = now;
    e.curr_time = now + std::chrono::milliseconds(16); // 60 FPS
    
    auto render_time = now + std::chrono::milliseconds(8);
    float alpha = calculate_alpha(e, render_time);
    
    EXPECT_NEAR(alpha, 0.5f, 0.01f);
}

TEST(Interpolation, SlowerFrameTime33ms) {
    MockEntity e;
    auto now = std::chrono::steady_clock::now();
    
    e.prev_time = now;
    e.curr_time = now + std::chrono::milliseconds(33); // 30 FPS
    
    auto render_time = now + std::chrono::milliseconds(11);
    float alpha = calculate_alpha(e, render_time);
    
    EXPECT_NEAR(alpha, 0.33f, 0.01f);
}

TEST(Interpolation, FasterFrameTime8ms) {
    MockEntity e;
    auto now = std::chrono::steady_clock::now();
    
    e.prev_time = now;
    e.curr_time = now + std::chrono::milliseconds(8); // 120 FPS
    
    auto render_time = now + std::chrono::milliseconds(4);
    float alpha = calculate_alpha(e, render_time);
    
    EXPECT_NEAR(alpha, 0.5f, 0.01f);
}

// ============================================================================
// EDGE CASES
// ============================================================================

TEST(Interpolation, AlphaNegativeClamped) {
    // Should never happen in practice, but test clamping
    float result = interpolate(100.0f, 200.0f, -0.5f);
    EXPECT_FLOAT_EQ(result, 100.0f); // Clamped to prev
}

TEST(Interpolation, AlphaAboveOneClamped) {
    // Alpha > 1.0 should be clamped for interpolation
    float result = interpolate(100.0f, 200.0f, 1.5f);
    EXPECT_FLOAT_EQ(result, 200.0f); // Clamped to curr
}

TEST(Interpolation, ZeroTimeDelta) {
    MockEntity e;
    auto now = std::chrono::steady_clock::now();
    
    e.prev_time = now;
    e.curr_time = now; // Same time
    
    float alpha = calculate_alpha(e, now);
    
    // Should return 1.0 when total_ms = 0
    EXPECT_FLOAT_EQ(alpha, 1.0f);
}

TEST(Interpolation, ReverseTimeOrder) {
    MockEntity e;
    auto now = std::chrono::steady_clock::now();
    
    e.prev_time = now + std::chrono::milliseconds(16);
    e.curr_time = now; // curr < prev (shouldn't happen)
    
    float alpha = calculate_alpha(e, now);
    
    // Should return 1.0 when curr_time <= prev_time
    EXPECT_FLOAT_EQ(alpha, 1.0f);
}

// ============================================================================
// MOVEMENT SCENARIOS
// ============================================================================

TEST(Interpolation, FastMovingEntityHorizontal) {
    MockEntity e;
    e.prev_x = 0.0f;
    e.x = 300.0f; // 300 pixels in 16ms = ~18750 px/s
    e.vx = 18750.0f;
    
    auto now = std::chrono::steady_clock::now();
    e.prev_time = now;
    e.curr_time = now + std::chrono::milliseconds(16);
    
    // Render at 8ms (halfway)
    auto render_time = now + std::chrono::milliseconds(8);
    float alpha = calculate_alpha(e, render_time);
    
    float draw_x = interpolate(e.prev_x, e.x, alpha);
    
    EXPECT_NEAR(draw_x, 150.0f, 1.0f); // Halfway = 150px
}

TEST(Interpolation, SlowMovingEntityVertical) {
    MockEntity e;
    e.prev_y = 100.0f;
    e.y = 105.0f; // 5 pixels in 16ms = 312.5 px/s
    e.vy = 312.5f;
    
    auto now = std::chrono::steady_clock::now();
    e.prev_time = now;
    e.curr_time = now + std::chrono::milliseconds(16);
    
    // Render at 4ms (quarter)
    auto render_time = now + std::chrono::milliseconds(4);
    float alpha = calculate_alpha(e, render_time);
    
    float draw_y = interpolate(e.prev_y, e.y, alpha);
    
    EXPECT_NEAR(draw_y, 101.25f, 0.1f); // Quarter = 1.25px moved
}

TEST(Interpolation, DiagonalMovement) {
    MockEntity e;
    e.prev_x = 100.0f;
    e.prev_y = 100.0f;
    e.x = 200.0f;
    e.y = 200.0f;
    
    auto now = std::chrono::steady_clock::now();
    e.prev_time = now;
    e.curr_time = now + std::chrono::milliseconds(16);
    
    auto render_time = now + std::chrono::milliseconds(8);
    float alpha = calculate_alpha(e, render_time);
    
    float draw_x = interpolate(e.prev_x, e.x, alpha);
    float draw_y = interpolate(e.prev_y, e.y, alpha);
    
    EXPECT_NEAR(draw_x, 150.0f, 0.1f);
    EXPECT_NEAR(draw_y, 150.0f, 0.1f);
}

// ============================================================================
// REALISTIC GAME SCENARIOS
// ============================================================================

TEST(Interpolation, ProjectileMovement) {
    MockEntity e;
    e.prev_x = 100.0f;
    e.x = 150.0f; // 50 pixels = ~3125 px/s à 16ms
    e.vx = 700.0f; // Typical projectile speed
    
    auto now = std::chrono::steady_clock::now();
    e.prev_time = now;
    e.curr_time = now + std::chrono::milliseconds(16);
    
    // Test multiple render points
    std::vector<std::pair<int, float>> test_points = {
        {0, 100.0f},   // Start
        {4, 112.5f},   // 25%
        {8, 125.0f},   // 50%
        {12, 137.5f},  // 75%
        {16, 150.0f}   // End
    };
    
    for (const auto& [ms, expected] : test_points) {
        auto render_time = now + std::chrono::milliseconds(ms);
        float alpha = calculate_alpha(e, render_time);
        float draw_x = interpolate(e.prev_x, e.x, alpha);
        
        EXPECT_NEAR(draw_x, expected, 0.5f) << "Failed at " << ms << "ms";
    }
}

TEST(Interpolation, EnemyPatrolMovement) {
    MockEntity e;
    // Enemy moving slowly back and forth
    e.prev_x = 200.0f;
    e.x = 195.0f; // Moving left 5 pixels
    e.vx = -300.0f;
    
    auto now = std::chrono::steady_clock::now();
    e.prev_time = now;
    e.curr_time = now + std::chrono::milliseconds(16);
    
    auto render_time = now + std::chrono::milliseconds(8);
    float alpha = calculate_alpha(e, render_time);
    
    float draw_x = interpolate(e.prev_x, e.x, alpha);
    
    EXPECT_NEAR(draw_x, 197.5f, 0.1f); // Halfway back
}

TEST(Interpolation, PlayerMovementStutter) {
    // Simulate player stopping and starting
    MockEntity e;
    
    auto now = std::chrono::steady_clock::now();
    
    // Frame 1: Moving right
    e.prev_x = 100.0f;
    e.x = 105.0f;
    e.prev_time = now;
    e.curr_time = now + std::chrono::milliseconds(16);
    
    auto render_time = now + std::chrono::milliseconds(8);
    float alpha = calculate_alpha(e, render_time);
    float draw_x = interpolate(e.prev_x, e.x, alpha);
    
    EXPECT_NEAR(draw_x, 102.5f, 0.1f);
    
    // Frame 2: Stopped
    e.prev_x = 105.0f;
    e.x = 105.0f; // No movement
    e.prev_time = e.curr_time;
    e.curr_time = e.prev_time + std::chrono::milliseconds(16);
    
    render_time = e.prev_time + std::chrono::milliseconds(8);
    alpha = calculate_alpha(e, render_time);
    draw_x = interpolate(e.prev_x, e.x, alpha);
    
    EXPECT_FLOAT_EQ(draw_x, 105.0f); // Should stay at same position
}

// ============================================================================
// PRECISION TESTS
// ============================================================================

TEST(Interpolation, SubPixelPrecision) {
    MockEntity e;
    e.prev_x = 100.123f;
    e.x = 100.456f;
    
    float alpha = 0.5f;
    float result = interpolate(e.prev_x, e.x, alpha);
    
    EXPECT_NEAR(result, 100.2895f, 0.0001f);
}

TEST(Interpolation, LargeDistanceInterpolation) {
    MockEntity e;
    e.prev_x = 0.0f;
    e.x = 1920.0f; // Full screen width
    
    float alpha = 0.1f;
    float result = interpolate(e.prev_x, e.x, alpha);
    
    EXPECT_NEAR(result, 192.0f, 0.1f);
}

TEST(Interpolation, NegativeCoordinates) {
    MockEntity e;
    e.prev_x = -50.0f;
    e.x = 50.0f;
    
    float alpha = 0.5f;
    float result = interpolate(e.prev_x, e.x, alpha);
    
    EXPECT_FLOAT_EQ(result, 0.0f);
}

// ============================================================================
// PERFORMANCE TESTS
// ============================================================================

TEST(Interpolation, MultipleEntitiesPerformance) {
    const int NUM_ENTITIES = 1000;
    std::vector<MockEntity> entities(NUM_ENTITIES);
    
    auto now = std::chrono::steady_clock::now();
    
    // Setup entities
    for (std::size_t i = 0; i < NUM_ENTITIES; ++i) {
        entities[i].prev_x = static_cast<float>(i);
        entities[i].x = static_cast<float>(i + 100);
        entities[i].prev_time = now;
        entities[i].curr_time = now + std::chrono::milliseconds(16);
    }
    
    auto render_time = now + std::chrono::milliseconds(8);
    
    // Time the interpolation
    auto start = std::chrono::high_resolution_clock::now();
    
    for (const auto& e : entities) {
        float alpha = calculate_alpha(e, render_time);
        float draw_x = interpolate(e.prev_x, e.x, alpha);
        (void)draw_x; // Prevent optimization
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    // Should complete in less than 1ms for 1000 entities
    EXPECT_LT(duration.count(), 1000) << "Interpolation too slow: " 
                                       << duration.count() << "us for " 
                                       << NUM_ENTITIES << " entities";
}

TEST(Interpolation, RepeatedCalculations) {
    MockEntity e;
    e.prev_x = 100.0f;
    e.x = 200.0f;
    
    auto now = std::chrono::steady_clock::now();
    e.prev_time = now;
    e.curr_time = now + std::chrono::milliseconds(16);
    
    auto render_time = now + std::chrono::milliseconds(8);
    
    // Multiple calculations should give same result
    float alpha1 = calculate_alpha(e, render_time);
    float alpha2 = calculate_alpha(e, render_time);
    float alpha3 = calculate_alpha(e, render_time);
    
    EXPECT_FLOAT_EQ(alpha1, alpha2);
    EXPECT_FLOAT_EQ(alpha2, alpha3);
    
    float result1 = interpolate(e.prev_x, e.x, alpha1);
    float result2 = interpolate(e.prev_x, e.x, alpha2);
    
    EXPECT_FLOAT_EQ(result1, result2);
}

// ============================================================================
// JITTER SIMULATION
// ============================================================================

TEST(Interpolation, NetworkJitterSimulation) {
    MockEntity e;
    auto now = std::chrono::steady_clock::now();
    
    // Frame 1: Normal timing (16ms)
    e.prev_x = 100.0f;
    e.x = 110.0f;
    e.prev_time = now;
    e.curr_time = now + std::chrono::milliseconds(16);
    
    auto render_time = now + std::chrono::milliseconds(8);
    float alpha = calculate_alpha(e, render_time);
    EXPECT_NEAR(alpha, 0.5f, 0.01f);
    
    // Frame 2: Jitter (25ms instead of 16ms)
    e.prev_x = 110.0f;
    e.x = 120.0f;
    e.prev_time = e.curr_time;
    e.curr_time = e.prev_time + std::chrono::milliseconds(25);
    
    render_time = e.prev_time + std::chrono::milliseconds(12);
    alpha = calculate_alpha(e, render_time);
    EXPECT_NEAR(alpha, 0.48f, 0.01f); // 12/25
    
    // Frame 3: Recovered (16ms)
    e.prev_x = 120.0f;
    e.x = 130.0f;
    e.prev_time = e.curr_time;
    e.curr_time = e.prev_time + std::chrono::milliseconds(16);
    
    render_time = e.prev_time + std::chrono::milliseconds(8);
    alpha = calculate_alpha(e, render_time);
    EXPECT_NEAR(alpha, 0.5f, 0.01f);
}

// ============================================================================
// MAIN - Removed (using shared test_main_client.cpp)
// ============================================================================

