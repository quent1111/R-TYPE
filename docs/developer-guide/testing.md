# Testing Guide

## Overview

R-TYPE uses **Google Test (GTest)** for unit and integration testing. The project has **57 test files** organized by component, covering ECS, game logic, networking, rendering, and administration features.

**Test Statistics:**
- 456 total tests
- Organized in 7 categories
- 100% passing rate required for CI/CD

## Quick Start

### Running All Tests

```bash
# Using the convenience script (recommended)
./r-type.sh test

# Using CMake/CTest directly
cd build
ctest --output-on-failure

# Run specific test suite
./bin/test_registry
./bin/test_collision
./bin/test_compression
```

### Building Tests

Tests are automatically built with the main project:

```bash
# Build everything including tests
./r-type.sh build

# Build only tests
cmake --build build --target all
```

## Test Organization

### Directory Structure

```
tests/
├── ecs/                    # ECS core tests
│   ├── test_entity.cpp
│   ├── test_component.cpp
│   ├── test_registry.cpp
│   └── test_system.cpp
├── game/                   # Game logic tests
│   ├── test_collision.cpp
│   ├── test_movement.cpp
│   ├── test_health.cpp
│   ├── test_weapon.cpp
│   ├── test_powerups.cpp
│   ├── test_level_manager.cpp
│   └── ...
├── network/               # Network protocol tests
│   ├── test_serialization.cpp
│   ├── test_compression.cpp
│   ├── test_protocol.cpp
│   ├── test_packet_reliability.cpp
│   └── ...
├── admin/                 # Admin client tests
│   └── test_admin_complete.cpp
├── render/                # Rendering tests
│   └── test_animation.cpp
├── integration/           # Integration tests
│   └── (end-to-end tests)
└── boostrap/              # Bootstrap tests
```

### Test Categories

#### 1. ECS Tests (`tests/ecs/`)

Tests for the Entity Component System core:

- **Entity Management**: Creation, destruction, ID assignment
- **Component Storage**: `sparse_array<T>`, registration, add/remove
- **Registry Operations**: Entity-component associations
- **System Execution**: System registration and updates

**Example:**
```cpp
// tests/ecs/test_entity.cpp
TEST(Entity, Creation) {
    entity e(42);
    EXPECT_EQ(e._id, 42);
}
```

#### 2. Game Logic Tests (`tests/game/`)

Tests for game mechanics and components:

**Collision Detection:**
```cpp
// tests/game/test_collision.cpp
// - AABB collision
// - Circle collision
// - Collision callbacks
// - Damage application
```

**Movement System:**
```cpp
// tests/game/test_movement.cpp
// - Position updates
// - Velocity application
// - Boundary checking
```

**Weapon System:**
```cpp
// tests/game/test_weapon.cpp
TEST(Weapon, ShootingCooldown) {
    weapon w;
    w.fire_timer = 0.0f;
    w.fire_rate = 0.2f;
    
    EXPECT_TRUE(w.can_shoot());
    w.fire_timer = 0.1f;
    EXPECT_FALSE(w.can_shoot());
}
```

**Power-up System:**
```cpp
// tests/game/test_powerups.cpp
// - Power-up spawning
// - Power-up collection
// - Effect application
// - Duration tracking
```

**Level Manager:**
```cpp
// tests/game/test_level_manager.cpp
// - Enemy counting
// - Level progression
// - Difficulty scaling
// - Boss spawning
```

#### 3. Network Tests (`tests/network/`)

Tests for UDP networking and serialization:

**Binary Serialization:**
```cpp
// tests/network/test_serialization.cpp
TEST(BinarySerializer, WriteRead) {
    BinarySerializer serializer;
    serializer.write<uint32_t>(12345);
    serializer.write<float>(3.14f);
    
    uint32_t val1 = serializer.read<uint32_t>();
    float val2 = serializer.read<float>();
    
    EXPECT_EQ(val1, 12345);
    EXPECT_FLOAT_EQ(val2, 3.14f);
}
```

**Compression:**
```cpp
// tests/network/test_compression.cpp
TEST(CompressionSerializer, CompressionDecompression) {
    CompressionSerializer encoder;
    // Write data...
    bool compressed = encoder.compress();
    
    CompressionSerializer decoder;
    decoder.set_data(encoder.data());
    bool decompressed = decoder.decompress();
    
    EXPECT_TRUE(compressed);
    EXPECT_TRUE(decompressed);
    // Verify data integrity...
}
```

**Protocol Tests:**
```cpp
// tests/network/test_protocol.cpp
// - Opcode validation
// - Packet structure
// - Magic number verification
// - Payload encoding/decoding
```

**Reliability Layer:**
```cpp
// tests/network/test_packet_reliability.cpp
// - ACK handling
// - Retry mechanism
// - Sequence IDs
// - Timeout detection
```

#### 4. Admin Tests (`tests/admin/`)

Tests for the admin client UI:

```cpp
// tests/admin/test_admin_complete.cpp
TEST(LoginScreen, TextInput) {
    LoginScreen login;
    login.handle_text_input('a');
    login.handle_text_input('d');
    
    EXPECT_EQ(login.get_username(), "ad");
}

TEST(AdminClient, GetServerStatus) {
    AdminClient client;
    auto status = client.get_server_status();
    
    EXPECT_GE(status.uptime, 0);
    EXPECT_GE(status.connected_clients, 0);
}
```

#### 5. Render Tests (`tests/render/`)

Tests for rendering components (currently disabled):

```cpp
// tests/render/test_animation.cpp
TEST_F(DISABLED_AnimationTest, FrameProgression) {
    // Tests animation frame updates
    GTEST_SKIP() << "Requires SFML textures";
}
```

**Note:** Render tests are disabled in CI because they require graphics context and SFML texture loading.

## Writing Tests

### Test Structure

```cpp
#include <gtest/gtest.h>
#include "YourComponent.hpp"

// Simple test
TEST(TestSuiteName, TestName) {
    // Arrange
    MyComponent component;
    
    // Act
    component.do_something();
    
    // Assert
    EXPECT_EQ(component.value, expected_value);
}

// Test with fixture
class MyComponentTest : public ::testing::Test {
protected:
    void SetUp() override {
        component = MyComponent();
    }
    
    void TearDown() override {
        // Cleanup if needed
    }
    
    MyComponent component;
};

TEST_F(MyComponentTest, SpecificBehavior) {
    component.method();
    ASSERT_TRUE(component.is_valid());
}
```

### Common Assertions

```cpp
// Equality
EXPECT_EQ(actual, expected);
ASSERT_EQ(actual, expected);  // Stops test on failure

// Boolean
EXPECT_TRUE(condition);
EXPECT_FALSE(condition);

// Floating point
EXPECT_FLOAT_EQ(actual, expected);
EXPECT_NEAR(actual, expected, tolerance);

// Comparison
EXPECT_LT(val1, val2);  // Less than
EXPECT_LE(val1, val2);  // Less or equal
EXPECT_GT(val1, val2);  // Greater than
EXPECT_GE(val1, val2);  // Greater or equal

// Exceptions
EXPECT_THROW(statement, exception_type);
EXPECT_NO_THROW(statement);
```

### Best Practices

#### 1. Test Naming

```cpp
// Good: Descriptive names
TEST(WeaponSystem, CooldownPreventsRapidFire)
TEST(CollisionSystem, PlayerProjectileHitsEnemy)
TEST(NetworkSerializer, HandlesEmptyStrings)

// Bad: Vague names
TEST(Weapon, Test1)
TEST(Collision, Works)
```

#### 2. Arrange-Act-Assert Pattern

```cpp
TEST(HealthSystem, DamageReducesHealth) {
    // Arrange - Set up test data
    health player_health;
    player_health.current = 100;
    player_health.max = 100;
    
    // Act - Perform action
    player_health.current -= 25;
    
    // Assert - Verify result
    EXPECT_EQ(player_health.current, 75);
    EXPECT_LT(player_health.current, player_health.max);
}
```

#### 3. Test Independence

```cpp
// Good: Each test is independent
TEST(LevelManager, InitialState) {
    level_manager manager;
    EXPECT_EQ(manager.current_level, 1);
}

TEST(LevelManager, AdvanceLevel) {
    level_manager manager;
    manager.advance_to_next_level();
    EXPECT_EQ(manager.current_level, 2);
}

// Bad: Tests depend on execution order
static level_manager shared_manager;  // Don't do this!
```

#### 4. Edge Cases

```cpp
TEST(Weapon, ZeroCooldown) {
    weapon w;
    w.fire_rate = 0.0f;
    w.fire_timer = 0.0f;
    EXPECT_TRUE(w.can_shoot());  // Edge case: instant fire
}

TEST(Serializer, EmptyString) {
    BinarySerializer s;
    s.write_string("");
    std::string result = s.read_string();
    EXPECT_EQ(result, "");  // Edge case: empty data
}
```

## Running Specific Tests

### Filter by Name

```bash
# Run tests matching pattern
./bin/test_game --gtest_filter="Weapon*"
./bin/test_network --gtest_filter="*Compression*"

# Run single test
./bin/test_collision --gtest_filter="CollisionTest.AABBCollision"
```

### Repeat Tests

```bash
# Repeat 10 times (useful for race conditions)
./bin/test_network --gtest_repeat=10

# Shuffle test order
./bin/test_game --gtest_shuffle
```

### Verbose Output

```bash
# Show all test names
./bin/test_game --gtest_list_tests

# Verbose output
./bin/test_game --gtest_verbose
```

## Continuous Integration

### GitHub Actions

Tests run automatically on every push and pull request:

```yaml
# .github/workflows/tests.yml
- name: Run tests
  run: ./r-type.sh test
```

**Requirements:**
- All 456 tests must pass
- No skipped tests (except DISABLED_ tests)
- Build must succeed on Linux

### Test Coverage

Generate coverage report:

```bash
./r-type.sh coverage
```

View HTML report:
```bash
open build/coverage/index.html
```

## Debugging Tests

### Run with GDB

```bash
gdb --args ./bin/test_collision --gtest_filter="CollisionTest.AABBCollision"
(gdb) run
(gdb) bt  # Backtrace on crash
```

### Valgrind Memory Check

```bash
valgrind --leak-check=full ./bin/test_registry
```

### Print Debug Info

```cpp
TEST(MyTest, Debug) {
    std::cout << "Debug value: " << my_value << std::endl;
    EXPECT_EQ(my_value, expected);
}
```

## Test Maintenance

### Disabled Tests

Some tests are disabled because they require graphics context:

```cpp
TEST_F(DISABLED_AnimationTest, FrameProgression) {
    GTEST_SKIP() << "Requires SFML textures";
}
```

These tests can be run manually when graphics context is available.

### Adding New Tests

1. Create test file in appropriate directory:
   ```bash
   touch tests/game/test_my_feature.cpp
   ```

2. Add to `tests/CMakeLists.txt`:
   ```cmake
   add_executable(test_my_feature game/test_my_feature.cpp)
   target_link_libraries(test_my_feature PRIVATE gtest_main game_lib)
   add_test(NAME test_my_feature COMMAND test_my_feature)
   ```

3. Write tests:
   ```cpp
   #include <gtest/gtest.h>
   
   TEST(MyFeature, BasicBehavior) {
       // Test code
   }
   ```

4. Run tests:
   ```bash
   ./r-type.sh test
   ```

## Troubleshooting

### Tests Not Found

```bash
# Rebuild tests
./r-type.sh rebuild
./r-type.sh test
```

### Linking Errors

Check `tests/CMakeLists.txt` for correct library dependencies:
```cmake
target_link_libraries(test_name PRIVATE 
    gtest_main 
    game_lib 
    engine_lib
)
```

### Timeout Issues

Increase CTest timeout:
```bash
ctest --timeout 300  # 5 minutes
```

### SFML/Graphics Tests Fail

These tests require X11/graphics context. They're disabled in CI but can run locally:
```bash
# Enable display
export DISPLAY=:0
./bin/test_render
```

## Related Documentation

- [Architecture Overview](../architecture/overview.md) - System architecture
- [ECS Documentation](../architecture/ecs.md) - ECS design patterns
- [Network Protocol](../architecture/network.md) - Network testing details
- [Contributing Guidelines](../CONTRIBUTING.md) - Code standards

## Further Reading

- [Google Test Documentation](https://google.github.io/googletest/) - Full GTest reference
- [Test-Driven Development](https://martinfowler.com/bliki/TestDrivenDevelopment.html) - TDD principles
- [Unit Testing Best Practices](https://github.com/goldbergyoni/javascript-testing-best-practices) - General testing patterns
