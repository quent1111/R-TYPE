#include <iostream>
#include <cassert>
#include "../../bootstrap/bs/sparse_array.hpp"
#include "../../bootstrap/bs/zipper.hpp"
#include "../../bootstrap/bs/indexed_zipper.hpp"

struct position {
    float x, y;
    explicit position(float x = 0, float y = 0) : x(x), y(y) {}
};

struct velocity {
    float vx, vy;
    explicit velocity(float vx = 0, float vy = 0) : vx(vx), vy(vy) {}
};

struct health {
    int hp;
    explicit health(int hp = 100) : hp(hp) {}
};

void test_basic_zipper() {
    std::cout << "\n=== Test: Basic Zipper ===" << std::endl;

    sparse_array<position> positions;
    sparse_array<velocity> velocities;

    positions.insert_at(0, position{10, 20});
    velocities.insert_at(0, velocity{1, 2});

    positions.insert_at(1, position{30, 40});
    velocities.insert_at(1, velocity{3, 4});

    positions.insert_at(2, position{50, 60});
    velocities.insert_at(2, velocity{5, 6});

    std::cout << "Iterating with zipper:" << std::endl;
    int count = 0;
    for (auto &&[pos, vel] : containers::zipper(positions, velocities)) {
        std::cout << "  [" << count << "] Position = { " << pos.x << ", " << pos.y << " }, "
                  << "Velocity = { " << vel.vx << ", " << vel.vy << " }" << std::endl;
        count++;
        if (count > 10) {
            std::cout << "  ERROR: Infinite loop detected!" << std::endl;
            break;
        }
    }

    std::cout << "Total count: " << count << std::endl;
    assert(count == 3);
    std::cout << "✓ Basic zipper works correctly (" << count << " entities)" << std::endl;
}

void test_zipper_with_missing_components() {
    std::cout << "\n=== Test: Zipper Skips Missing Components ===" << std::endl;

    sparse_array<position> positions;
    sparse_array<velocity> velocities;

    positions.insert_at(0, position{10, 20});
    velocities.insert_at(0, velocity{1, 2});

    positions.insert_at(1, position{30, 40});

    positions.insert_at(2, position{50, 60});
    velocities.insert_at(2, velocity{5, 6});

    velocities.insert_at(3, velocity{7, 8});

    positions.insert_at(4, position{90, 100});
    velocities.insert_at(4, velocity{9, 10});

    std::cout << "Entities with BOTH position and velocity:" << std::endl;
    int count = 0;
    for (auto &&[pos, vel] : containers::zipper(positions, velocities)) {
        std::cout << "  Position = { " << pos.x << ", " << pos.y << " }, "
                  << "Velocity = { " << vel.vx << ", " << vel.vy << " }" << std::endl;
        count++;
    }
    
    assert(count == 3);
    std::cout << "✓ Zipper correctly skipped missing components (" << count << "/5 entities)" << std::endl;
}

void test_indexed_zipper() {
    std::cout << "\n=== Test: Indexed Zipper ===" << std::endl;

    sparse_array<position> positions;
    sparse_array<velocity> velocities;

    positions.insert_at(0, position{10, 20});
    velocities.insert_at(0, velocity{1, 2});


    positions.insert_at(2, position{50, 60});
    velocities.insert_at(2, velocity{5, 6});

    positions.insert_at(3, position{70, 80});
    velocities.insert_at(3, velocity{7, 8});

    std::cout << "Iterating with indexed_zipper:" << std::endl;
    int count = 0;
    for (auto &&[i, pos, vel] : containers::indexed_zipper(positions, velocities)) {
        std::cout << "  Entity " << i << ": Position = { " << pos.x << ", " << pos.y << " }, "
                  << "Velocity = { " << vel.vx << ", " << vel.vy << " }" << std::endl;

        if (count == 0) assert(i == 0);
        if (count == 1) assert(i == 2);
        if (count == 2) assert(i == 3);

        count++;
    }

    assert(count == 3);
    std::cout << "✓ Indexed zipper works correctly with indices" << std::endl;
}

void test_three_components() {
    std::cout << "\n=== Test: Zipper with 3 Components ===" << std::endl;

    sparse_array<position> positions;
    sparse_array<velocity> velocities;
    sparse_array<health> healths;

    positions.insert_at(0, position{10, 20});
    velocities.insert_at(0, velocity{1, 2});
    healths.insert_at(0, health{100});

    positions.insert_at(1, position{30, 40});
    velocities.insert_at(1, velocity{3, 4});

    positions.insert_at(2, position{50, 60});
    velocities.insert_at(2, velocity{5, 6});
    healths.insert_at(2, health{75});

    std::cout << "Entities with position, velocity, AND health:" << std::endl;
    int count = 0;
    for (auto &&[pos, vel, hp] : containers::zipper(positions, velocities, healths)) {
        std::cout << "  Position = { " << pos.x << ", " << pos.y << " }, "
                  << "Velocity = { " << vel.vx << ", " << vel.vy << " }, "
                  << "Health = " << hp.hp << std::endl;
        count++;
    }

    assert(count == 2);
    std::cout << "✓ Zipper with 3 components works correctly (" << count << "/3 entities)" << std::endl;
}

void test_indexed_zipper_usage() {
    std::cout << "\n=== Test: Practical Indexed Zipper Usage ===" << std::endl;

    sparse_array<position> positions;
    sparse_array<velocity> velocities;

    for (size_t i = 0; i < 10; ++i) {
        if (i % 2 == 0) {
            positions.insert_at(i, position{static_cast<float>(i * 10), static_cast<float>(i * 10)});
            velocities.insert_at(i, velocity{1, 1});
        }
    }

    std::cout << "Applying movement to entities:" << std::endl;
    for (auto &&[i, pos, vel] : containers::indexed_zipper(positions, velocities)) {
        pos.x += vel.vx;
        pos.y += vel.vy;
        std::cout << "  Moved entity " << i << " to (" << pos.x << ", " << pos.y << ")" << std::endl;
    }

    assert(positions[0]->x == 1.0f);
    assert(positions[0]->y == 1.0f);
    assert(positions[2]->x == 21.0f);
    assert(positions[2]->y == 21.0f);

    std::cout << "✓ Practical indexed zipper usage works correctly" << std::endl;
}

void test_empty_containers() {
    std::cout << "\n=== Test: Empty Containers ===" << std::endl;
    
    sparse_array<position> positions;
    sparse_array<velocity> velocities;
    
    int count = 0;
    for (auto &&[pos, vel] : containers::zipper(positions, velocities)) {
        (void)pos;
        (void)vel;
        count++;
    }
    
    assert(count == 0);
    std::cout << "✓ Empty containers handled correctly (no iterations)" << std::endl;
}

int main() {
    std::cout << "Testing Zipper and Indexed Zipper..." << std::endl;
    
    test_basic_zipper();
    test_zipper_with_missing_components();
    test_indexed_zipper();
    test_three_components();
    test_indexed_zipper_usage();
    test_empty_containers();
    
    std::cout << "\n✅ All zipper tests passed!" << std::endl;
    return 0;
}
