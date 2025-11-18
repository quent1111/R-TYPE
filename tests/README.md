# Tests

Unit and integration tests for the R-Type project.

## Running Tests

```bash
cd build
ctest --output-on-failure
```

## Test Structure

- `ecs_tests/`: ECS module tests (entity, sparse_array, registry)
- `net_tests/`: Networking tests (sockets, protocol)
- `integration_tests/`: End-to-end tests

## Writing Tests

Use GoogleTest framework:

```cpp
#include <gtest/gtest.h>

TEST(MyTest, BasicAssertion) {
    EXPECT_EQ(1 + 1, 2);
}
```

Add test file to `tests/CMakeLists.txt`.
