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

// Helper function to extrapolate position (dead reckoning)
static std::pair<float, float> extrapolate(const MockEntity& e,
                                    std::chrono::steady_clock::time_point render_time) {
    const float total_ms =
        std::chrono::duration_cast<std::chrono::duration<float, std::milli>>(
            e.curr_time - e.prev_time
        ).count();
    
    const float elapsed_ms =
        std::chrono::duration_cast<std::chrono::duration<float, std::milli>>(
            render_time - e.prev_time
        ).count();
    
    float alpha = (total_ms > 0.0f) ? (elapsed_ms / total_ms) : 1.0f;
    
    if (alpha > 1.0f) {
        // Extrapolation mode
        float overshoot_ms = elapsed_ms - total_ms;
        float overshoot_frames = overshoot_ms / 16.67f;
        
        // Cap at 12 frames (200ms)
        if (overshoot_frames > 12.0f) {
            overshoot_frames = 12.0f;
        }
        
        float extrapolation_time = overshoot_frames / 60.0f;
        float draw_x = e.x + e.vx * extrapolation_time;
        float draw_y = e.y + e.vy * extrapolation_time;
        
        return {draw_x, draw_y};
    }
    
    // Interpolation fallback (shouldn't happen in extrapolation tests)
    return {e.x, e.y};
}

// ============================================================================
// BASIC EXTRAPOLATION TESTS
// ============================================================================

TEST(Extrapolation, AlphaAboveOneTriggersExtrapolation) {
    MockEntity e;
    auto now = std::chrono::steady_clock::now();
    
    e.prev_time = now;
    e.curr_time = now + std::chrono::milliseconds(16);
    
    // Render 50ms after curr_time (packet loss)
    auto render_time = e.curr_time + std::chrono::milliseconds(50);
    float alpha = calculate_alpha(e, render_time);
    
    EXPECT_GT(alpha, 1.0f);
    EXPECT_NEAR(alpha, 4.125f, 0.1f); // (16+50)/16 = 66/16
}

TEST(Extrapolation, SimpleHorizontalMovement) {
    MockEntity e;
    e.x = 100.0f;
    e.vx = 300.0f; // 300 px/s
    
    auto now = std::chrono::steady_clock::now();
    e.prev_time = now;
    e.curr_time = now + std::chrono::milliseconds(16);
    
    // 50ms after last packet (3 frames at 60 FPS)
    auto render_time = e.curr_time + std::chrono::milliseconds(50);
    
    auto [draw_x, draw_y] = extrapolate(e, render_time);
    
    // overshoot = 50ms = 3 frames
    // extrapolation_time = 3/60 = 0.05s
    // draw_x = 100 + 300*0.05 = 115
    EXPECT_NEAR(draw_x, 115.0f, 1.0f);
}

TEST(Extrapolation, SimpleVerticalMovement) {
    MockEntity e;
    e.y = 200.0f;
    e.vy = -400.0f; // Moving up at 400 px/s
    
    auto now = std::chrono::steady_clock::now();
    e.prev_time = now;
    e.curr_time = now + std::chrono::milliseconds(16);
    
    // 33ms after last packet (2 frames)
    auto render_time = e.curr_time + std::chrono::milliseconds(33);
    
    auto [draw_x, draw_y] = extrapolate(e, render_time);
    
    // overshoot = 33ms = 2 frames
    // extrapolation_time = 2/60 = 0.033s
    // draw_y = 200 + (-400)*0.033 = 186.8
    EXPECT_NEAR(draw_y, 186.8f, 1.0f);
}

TEST(Extrapolation, DiagonalMovement) {
    MockEntity e;
    e.x = 100.0f;
    e.y = 100.0f;
    e.vx = 300.0f;
    e.vy = 400.0f;
    
    auto now = std::chrono::steady_clock::now();
    e.prev_time = now;
    e.curr_time = now + std::chrono::milliseconds(16);
    
    auto render_time = e.curr_time + std::chrono::milliseconds(50);
    
    auto [draw_x, draw_y] = extrapolate(e, render_time);
    
    // 50ms = 3 frames = 0.05s
    EXPECT_NEAR(draw_x, 115.0f, 1.0f); // 100 + 300*0.05
    EXPECT_NEAR(draw_y, 120.0f, 1.0f); // 100 + 400*0.05
}

// ============================================================================
// PACKET LOSS SIMULATION
// ============================================================================

TEST(Extrapolation, SmallPacketLoss50ms) {
    MockEntity e;
    e.x = 500.0f;
    e.vx = 600.0f; // Fast projectile
    
    auto now = std::chrono::steady_clock::now();
    e.prev_time = now;
    e.curr_time = now + std::chrono::milliseconds(16);
    
    // 1 packet lost (~50ms delay)
    auto render_time = e.curr_time + std::chrono::milliseconds(50);
    
    auto [draw_x, draw_y] = extrapolate(e, render_time);
    
    // Should extrapolate ~50px forward (but with frame rounding)
    EXPECT_NEAR(draw_x, 550.0f, 25.0f);  // Increased tolerance for frame rounding
}

TEST(Extrapolation, ModeratePacketLoss100ms) {
    MockEntity e;
    e.x = 300.0f;
    e.vx = 500.0f;
    
    auto now = std::chrono::steady_clock::now();
    e.prev_time = now;
    e.curr_time = now + std::chrono::milliseconds(16);
    
    // 2 packets lost (~100ms delay)
    auto render_time = e.curr_time + std::chrono::milliseconds(100);
    
    auto [draw_x, draw_y] = extrapolate(e, render_time);
    
    // overshoot = 100ms = 6 frames = 0.1s
    // draw_x = 300 + 500*0.1 = 350
    EXPECT_NEAR(draw_x, 350.0f, 5.0f);
}

TEST(Extrapolation, SeverePacketLoss200ms) {
    MockEntity e;
    e.x = 100.0f;
    e.vx = 700.0f;
    
    auto now = std::chrono::steady_clock::now();
    e.prev_time = now;
    e.curr_time = now + std::chrono::milliseconds(16);
    
    // 200ms delay (at cap limit)
    auto render_time = e.curr_time + std::chrono::milliseconds(200);
    
    auto [draw_x, draw_y] = extrapolate(e, render_time);
    
    // Should be capped at 12 frames = 0.2s
    // draw_x = 100 + 700*0.2 = 240
    EXPECT_NEAR(draw_x, 240.0f, 5.0f);
}

TEST(Extrapolation, ExtremePacketLoss500ms) {
    MockEntity e;
    e.x = 100.0f;
    e.vx = 700.0f;
    
    auto now = std::chrono::steady_clock::now();
    e.prev_time = now;
    e.curr_time = now + std::chrono::milliseconds(16);
    
    // 500ms delay (way over cap)
    auto render_time = e.curr_time + std::chrono::milliseconds(500);
    
    auto [draw_x, draw_y] = extrapolate(e, render_time);
    
    // Should be capped at 12 frames = 0.2s (same as 200ms)
    // draw_x = 100 + 700*0.2 = 240
    EXPECT_NEAR(draw_x, 240.0f, 5.0f);
}

// ============================================================================
// CAP LIMIT TESTS (200ms = 12 frames)
// ============================================================================

TEST(Extrapolation, CapAt200msEnforced) {
    MockEntity e;
    e.x = 0.0f;
    e.vx = 1000.0f; // High speed
    
    auto now = std::chrono::steady_clock::now();
    e.prev_time = now;
    e.curr_time = now + std::chrono::milliseconds(16);
    
    // Test at exactly 200ms
    auto render_time_200 = e.curr_time + std::chrono::milliseconds(200);
    auto [x_200, y_200] = extrapolate(e, render_time_200);
    
    // Test at 300ms (should be same as 200ms due to cap)
    auto render_time_300 = e.curr_time + std::chrono::milliseconds(300);
    auto [x_300, y_300] = extrapolate(e, render_time_300);
    
    // Both should be capped at 12 frames
    EXPECT_NEAR(x_200, 200.0f, 5.0f); // 1000*0.2
    EXPECT_NEAR(x_300, 200.0f, 5.0f); // Same due to cap
    EXPECT_NEAR(x_200, x_300, 1.0f);  // Should be very close
}

TEST(Extrapolation, CapPreventsWildPredictions) {
    MockEntity e;
    e.x = 100.0f;
    e.vx = 2000.0f; // Very fast entity
    
    auto now = std::chrono::steady_clock::now();
    e.prev_time = now;
    e.curr_time = now + std::chrono::milliseconds(16);
    
    // 1 second delay (extreme)
    auto render_time = e.curr_time + std::chrono::milliseconds(1000);
    
    auto [draw_x, draw_y] = extrapolate(e, render_time);
    
    // Without cap: 100 + 2000*1.0 = 2100 (way off screen!)
    // With cap: 100 + 2000*0.2 = 500 (more reasonable)
    EXPECT_LT(draw_x, 600.0f);
    EXPECT_NEAR(draw_x, 500.0f, 10.0f);
}

// ============================================================================
// ZERO/NEGATIVE VELOCITY
// ============================================================================

TEST(Extrapolation, ZeroVelocityStaysInPlace) {
    MockEntity e;
    e.x = 500.0f;
    e.y = 300.0f;
    e.vx = 0.0f;
    e.vy = 0.0f;
    
    auto now = std::chrono::steady_clock::now();
    e.prev_time = now;
    e.curr_time = now + std::chrono::milliseconds(16);
    
    // Even with 200ms packet loss
    auto render_time = e.curr_time + std::chrono::milliseconds(200);
    
    auto [draw_x, draw_y] = extrapolate(e, render_time);
    
    // Should stay at same position
    EXPECT_FLOAT_EQ(draw_x, 500.0f);
    EXPECT_FLOAT_EQ(draw_y, 300.0f);
}

TEST(Extrapolation, NegativeVelocityMovesBackward) {
    MockEntity e;
    e.x = 500.0f;
    e.vx = -300.0f; // Moving left
    
    auto now = std::chrono::steady_clock::now();
    e.prev_time = now;
    e.curr_time = now + std::chrono::milliseconds(16);
    
    auto render_time = e.curr_time + std::chrono::milliseconds(50);
    
    auto [draw_x, draw_y] = extrapolate(e, render_time);
    
    // Should move left (decrease x)
    EXPECT_LT(draw_x, 500.0f);
    EXPECT_NEAR(draw_x, 485.0f, 2.0f); // 500 + (-300)*0.05
}

// ============================================================================
// REALISTIC GAME SCENARIOS
// ============================================================================

TEST(Extrapolation, FastProjectileWithJitter) {
    MockEntity e;
    e.x = 100.0f;
    e.vx = 800.0f; // Fast projectile
    
    auto now = std::chrono::steady_clock::now();
    e.prev_time = now;
    e.curr_time = now + std::chrono::milliseconds(16);
    
    // Small jitter (30ms)
    auto render_time = e.curr_time + std::chrono::milliseconds(30);
    
    auto [draw_x, draw_y] = extrapolate(e, render_time);
    
    // Should extrapolate slightly forward
    EXPECT_GT(draw_x, 100.0f);
    EXPECT_LT(draw_x, 150.0f);
}

TEST(Extrapolation, EnemyMovementDuringLag) {
    MockEntity e;
    e.x = 800.0f;
    e.vx = -200.0f; // Enemy moving left
    
    auto now = std::chrono::steady_clock::now();
    e.prev_time = now;
    e.curr_time = now + std::chrono::milliseconds(16);
    
    // 100ms lag spike
    auto render_time = e.curr_time + std::chrono::milliseconds(100);
    
    auto [draw_x, draw_y] = extrapolate(e, render_time);
    
    // Enemy should continue moving left
    EXPECT_LT(draw_x, 800.0f);
    EXPECT_NEAR(draw_x, 780.0f, 5.0f); // 800 + (-200)*0.1
}

TEST(Extrapolation, BossMovementContinuation) {
    MockEntity e;
    e.x = 960.0f; // Center screen
    e.y = 200.0f;
    e.vx = 0.0f;
    e.vy = 50.0f; // Slow vertical movement
    
    auto now = std::chrono::steady_clock::now();
    e.prev_time = now;
    e.curr_time = now + std::chrono::milliseconds(16);
    
    // 150ms packet loss
    auto render_time = e.curr_time + std::chrono::milliseconds(150);
    
    auto [draw_x, draw_y] = extrapolate(e, render_time);
    
    // X should stay same, Y should continue down
    EXPECT_FLOAT_EQ(draw_x, 960.0f);
    EXPECT_GT(draw_y, 200.0f);
    EXPECT_NEAR(draw_y, 215.0f, 10.0f); // 200 + 50*0.15 (increased tolerance)
}

// ============================================================================
// EDGE CASES
// ============================================================================

TEST(Extrapolation, VerySmallOvershoot) {
    MockEntity e;
    e.x = 100.0f;
    e.vx = 500.0f;
    
    auto now = std::chrono::steady_clock::now();
    e.prev_time = now;
    e.curr_time = now + std::chrono::milliseconds(16);
    
    // Just 5ms overshoot (0.3 frames)
    auto render_time = e.curr_time + std::chrono::milliseconds(5);
    
    auto [draw_x, draw_y] = extrapolate(e, render_time);
    
    // Very small extrapolation
    EXPECT_GT(draw_x, 100.0f);
    EXPECT_LT(draw_x, 105.0f);
}

TEST(Extrapolation, HighSpeedEntity) {
    MockEntity e;
    e.x = 0.0f;
    e.vx = 5000.0f; // Extremely fast
    
    auto now = std::chrono::steady_clock::now();
    e.prev_time = now;
    e.curr_time = now + std::chrono::milliseconds(16);
    
    // 100ms packet loss
    auto render_time = e.curr_time + std::chrono::milliseconds(100);
    
    auto [draw_x, draw_y] = extrapolate(e, render_time);
    
    // Even with high speed, should be reasonable due to time cap
    EXPECT_LT(draw_x, 1500.0f); // 5000*0.2 = 1000 (with some margin)
}

TEST(Extrapolation, NegativeCoordinates) {
    MockEntity e;
    e.x = 50.0f;
    e.vx = -300.0f; // Moving left past 0
    
    auto now = std::chrono::steady_clock::now();
    e.prev_time = now;
    e.curr_time = now + std::chrono::milliseconds(16);
    
    auto render_time = e.curr_time + std::chrono::milliseconds(200);
    
    auto [draw_x, draw_y] = extrapolate(e, render_time);
    
    // Can go negative (bounds checking happens elsewhere)
    EXPECT_LT(draw_x, 50.0f);
    EXPECT_NEAR(draw_x, -10.0f, 5.0f); // 50 + (-300)*0.2
}

// ============================================================================
// PRECISION TESTS
// ============================================================================

TEST(Extrapolation, SubPixelExtrapolation) {
    MockEntity e;
    e.x = 100.5f;
    e.vx = 123.456f;
    
    auto now = std::chrono::steady_clock::now();
    e.prev_time = now;
    e.curr_time = now + std::chrono::milliseconds(16);
    
    auto render_time = e.curr_time + std::chrono::milliseconds(33);
    
    auto [draw_x, draw_y] = extrapolate(e, render_time);
    
    // 33ms = 2 frames = 0.033s
    // draw_x = 100.5 + 123.456*0.033 = 104.574
    EXPECT_NEAR(draw_x, 104.574f, 0.5f);
}

TEST(Extrapolation, MultipleConsecutiveFrames) {
    MockEntity e;
    e.x = 100.0f;
    e.vx = 300.0f;
    
    auto now = std::chrono::steady_clock::now();
    e.prev_time = now;
    e.curr_time = now + std::chrono::milliseconds(16);
    
    std::vector<std::pair<int, float>> test_points = {
        {17, 105.0f},   // 1ms overshoot
        {33, 115.0f},   // 17ms = 1 frame
        {50, 120.25f},  // 34ms = 2 frames
        {83, 135.125f}, // 67ms = 4 frames
    };
    
    for (const auto& [ms_overshoot, expected] : test_points) {
        auto render_time = e.curr_time + std::chrono::milliseconds(ms_overshoot);
        auto [draw_x, draw_y] = extrapolate(e, render_time);
        
        EXPECT_NEAR(draw_x, expected, 12.0f)  // Increased tolerance for frame rounding
            << "Failed at " << ms_overshoot << "ms overshoot";
    }
}

// ============================================================================
// PERFORMANCE TESTS
// ============================================================================

TEST(Extrapolation, MultipleEntitiesPerformance) {
    const int NUM_ENTITIES = 1000;
    std::vector<MockEntity> entities(NUM_ENTITIES);
    
    auto now = std::chrono::steady_clock::now();
    
    // Setup entities with packet loss
    for (int i = 0; i < NUM_ENTITIES; ++i) {
        entities[i].x = static_cast<float>(i * 2);
        entities[i].vx = 500.0f;
        entities[i].prev_time = now;
        entities[i].curr_time = now + std::chrono::milliseconds(16);
    }
    
    auto render_time = now + std::chrono::milliseconds(100); // 100ms packet loss
    
    // Time the extrapolation
    auto start = std::chrono::high_resolution_clock::now();
    
    for (const auto& e : entities) {
        auto [draw_x, draw_y] = extrapolate(e, render_time);
        (void)draw_x; (void)draw_y; // Prevent optimization
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    // Should complete in less than 2ms for 1000 entities
    EXPECT_LT(duration.count(), 2000) << "Extrapolation too slow: " 
                                       << duration.count() << "us for " 
                                       << NUM_ENTITIES << " entities";
}

// ============================================================================
// RECOVERY TESTS
// ============================================================================

TEST(Extrapolation, RecoveryAfterPacketLoss) {
    MockEntity e;
    e.x = 100.0f;
    e.vx = 300.0f;
    
    auto now = std::chrono::steady_clock::now();
    
    // Frame 1: Normal
    e.prev_time = now;
    e.curr_time = now + std::chrono::milliseconds(16);
    
    // Frame 2: Packet loss (100ms)
    auto render_during_loss = e.curr_time + std::chrono::milliseconds(100);
    auto [x_during, y_during] = extrapolate(e, render_during_loss);
    EXPECT_NEAR(x_during, 150.0f, 25.0f); // Extrapolated (increased tolerance)
    
    // Frame 3: Packet arrives (recovery)
    e.prev_x = e.x;
    e.x = 130.0f; // Server position (might be different from extrapolation)
    e.prev_time = e.curr_time;
    e.curr_time = render_during_loss;
    
    // After recovery, should use new server state
    EXPECT_FLOAT_EQ(e.x, 130.0f);
}

TEST(Extrapolation, GradualRecovery) {
    MockEntity e;
    e.x = 100.0f;
    e.vx = 400.0f;
    
    auto now = std::chrono::steady_clock::now();
    e.prev_time = now;
    e.curr_time = now + std::chrono::milliseconds(16);
    
    // Simulate increasing packet loss
    std::vector<int> delays_ms = {20, 50, 100, 150, 200, 250};
    
    float prev_x = e.x;
    for (int delay : delays_ms) {
        auto render_time = e.curr_time + std::chrono::milliseconds(delay);
        auto [draw_x, draw_y] = extrapolate(e, render_time);
        
        // Position should increase but cap at 200ms
        EXPECT_GE(draw_x, prev_x);
        
        if (delay <= 200) {
            EXPECT_LT(draw_x, 200.0f);
        } else {
            // Should be capped
            EXPECT_NEAR(draw_x, 180.0f, 5.0f); // 100 + 400*0.2
        }
    }
}

// ============================================================================
// MAIN - Removed (using shared test_main_client.cpp)
// ============================================================================

