# R-TYPE Test Suite

This directory contains all unit and integration tests for the R-TYPE project using Google Test framework.

## 📁 Directory Structure

```
tests/
├── test_sanity.cpp              # Minimal sanity test (ALWAYS ENABLED)
├── ecs/                         # Entity-Component-System tests
│   ├── test_entity.cpp         # Entity lifecycle tests
│   ├── test_component.cpp      # Component management tests
│   ├── test_system.cpp         # System execution tests
│   └── test_registry.cpp       # Registry tests
├── network/                     # Networking tests
│   ├── test_protocol.cpp       # Protocol encoding/decoding
│   ├── test_serialization.cpp  # Data serialization tests
│   └── test_udp_socket.cpp     # Socket wrapper tests
├── game/                        # Game logic tests
│   ├── test_collision.cpp      # Collision detection tests
│   ├── test_movement.cpp       # Movement system tests
│   └── test_spawning.cpp       # Entity spawning tests
├── render/                      # Rendering tests
│   ├── test_sprite_system.cpp  # Sprite rendering tests
│   └── test_animation.cpp      # Animation system tests
└── integration/                 # Integration tests
    ├── test_client_server.cpp  # Client-server communication
    └── test_multiplayer.cpp    # Multiplayer gameplay tests
```

## 🚀 Running Tests

### Run all enabled tests
```bash
# Linux/macOS
./r-type.sh test

# Windows
r-type.bat test
```

### Run specific test suite
```bash
cd build/build/Release  # or build/bin
./test_ecs              # Run ECS tests only
./test_network          # Run network tests only
./test_game             # Run game logic tests
./test_render           # Run rendering tests
./test_integration      # Run integration tests
```

### Run with verbose output
```bash
./r-type.sh test --verbose
```

## ⚙️ Test Status

| Test Suite | Status | Files |
|------------|--------|-------|
| **Sanity** | ✅ **ENABLED** | `test_sanity.cpp` |
| **ECS** | ⏸️ Disabled (placeholder) | 4 files |
| **Network** | ⏸️ Disabled (placeholder) | 3 files |
| **Game** | ⏸️ Disabled (placeholder) | 3 files |
| **Render** | ⏸️ Disabled (placeholder) | 2 files |
| **Integration** | ⏸️ Disabled (placeholder) | 2 files |

**Total:** 1 enabled test suite, 14 placeholder test files

## 📝 Implementing Tests

### Step 1: Choose a test file
Navigate to the appropriate category and open the test file.

### Step 2: Remove DISABLED_ prefix
Change the test class name from:
```cpp
class DISABLED_EntityTest : public ::testing::Test {
```
to:
```cpp
class EntityTest : public ::testing::Test {
```

### Step 3: Implement the test
Replace `GTEST_SKIP()` with actual test logic:
```cpp
TEST_F(EntityTest, CreateEntity) {
    // Before:
    // GTEST_SKIP() << "Not implemented yet";
    
    // After:
    auto entity = registry.spawn_entity();
    EXPECT_TRUE(entity.is_valid());
}
```

### Step 4: Enable in CMakeLists.txt
Uncomment the corresponding `add_test()` line in `tests/CMakeLists.txt`:
```cmake
# Before:
# add_test(NAME ECSTests COMMAND test_ecs)

# After:
add_test(NAME ECSTests COMMAND test_ecs)
```

### Step 5: Build and verify
```bash
./r-type.sh test
```

## 🧪 Writing Good Tests

### Test Naming Convention
- Use descriptive test names: `TEST_F(ComponentTest, AddingComponentIncreasesCount)`
- Test class names should match the component: `EntityTest`, `RegistryTest`
- Group related tests in the same test fixture

### Test Structure
Follow the **AAA pattern**:
```cpp
TEST_F(EntityTest, SpawnEntity) {
    // Arrange: Setup test data
    Registry registry;
    
    // Act: Perform the action
    auto entity = registry.spawn_entity();
    
    // Assert: Verify the result
    EXPECT_TRUE(entity.is_valid());
    EXPECT_GE(entity.id(), 0);
}
```

### Assertions vs Expectations
- `EXPECT_*`: Test continues after failure
- `ASSERT_*`: Test stops immediately on failure

Use `ASSERT_*` when subsequent code depends on the condition:
```cpp
TEST_F(EntityTest, AddComponent) {
    auto entity = registry.spawn_entity();
    ASSERT_TRUE(entity.is_valid());  // Must pass for next line to work
    
    entity.add_component<Position>(10.0f, 20.0f);
    EXPECT_TRUE(entity.has_component<Position>());
}
```

## 📊 Code Coverage

Generate code coverage report:
```bash
./r-type.sh coverage
```

Coverage report will be in: `build/build/Release/coverage_html/index.html`

## 🔍 Test Categories

### Unit Tests
Test individual components in isolation:
- **ECS tests**: Entity, Component, System, Registry
- **Network tests**: Protocol, Serialization, Sockets
- **Game tests**: Collision, Movement, Spawning
- **Render tests**: Sprites, Animation

### Integration Tests
Test multiple components working together:
- **Client-Server**: Communication between client and server
- **Multiplayer**: Full game sessions with multiple players

## 🐛 Debugging Tests

### Run single test
```bash
./test_ecs --gtest_filter="EntityTest.CreateEntity"
```

### Run with debugger
```bash
gdb ./test_ecs
(gdb) run
```

### Verbose output
```bash
./test_ecs --gtest_verbose
```

## 📚 Google Test Documentation

- [Primer](https://google.github.io/googletest/primer.html)
- [Advanced Guide](https://google.github.io/googletest/advanced.html)
- [Assertions Reference](https://google.github.io/googletest/reference/assertions.html)

## 🎯 Test Coverage Goals

| Component | Target Coverage |
|-----------|----------------|
| ECS Core | 90%+ |
| Network | 85%+ |
| Game Logic | 80%+ |
| Rendering | 70%+ |
| Integration | 60%+ |

## 💡 Tips

1. **Write tests first (TDD)**: Define expected behavior before implementing
2. **Keep tests independent**: Each test should run in isolation
3. **Use fixtures**: Share setup/teardown code with test fixtures
4. **Test edge cases**: Empty inputs, null pointers, boundary values
5. **Mock dependencies**: Use test doubles for complex dependencies
6. **Continuous testing**: Run tests frequently during development

## ⚠️ Common Issues

### Test not found
- Make sure the test is uncommented in `CMakeLists.txt`
- Rebuild the project: `./r-type.sh rebuild`

### Linking errors
- Check that required libraries are linked in `CMakeLists.txt`
- Verify component dependencies are correct

### Test timeout
- Integration tests may take longer - increase timeout in CTest
- Check for infinite loops or blocking operations

---

**Happy Testing! 🧪**
