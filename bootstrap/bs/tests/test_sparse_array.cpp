#include "sparse_array.hpp"

#include <cassert>

#include <iostream>
#include <string>

// Simple test component
struct Position {
    float x, y;
    Position(float x_ = 0, float y_ = 0) : x(x_), y(y_) {}
};

struct Velocity {
    float dx, dy;
    Velocity(float dx_ = 0, float dy_ = 0) : dx(dx_), dy(dy_) {}
};

void test_basic_operations() {
    std::cout << "=== Test basic operations ===" << std::endl;
    sparse_array<Position> positions;
    // Test insert_at with lvalue
    Position pos1(10.0f, 20.0f);
    auto& ref1 = positions.insert_at(0, pos1);
    assert(ref1.has_value());
    assert(ref1->x == 10.0f);
    assert(ref1->y == 20.0f);
    std::cout << "✓ insert_at (lvalue) works" << std::endl;
    // Test insert_at with rvalue
    auto& ref2 = positions.insert_at(2, Position(30.0f, 40.0f));
    assert(ref2.has_value());
    assert(ref2->x == 30.0f);
    std::cout << "✓ insert_at (rvalue) works" << std::endl;
    // Test operator[] access
    assert(positions[0].has_value());
    assert(positions[0]->x == 10.0f);
    assert(positions[1].has_value() == false);  // No component at index 1
    assert(positions[2].has_value());
    std::cout << "✓ operator[] works" << std::endl;
    // Test size
    assert(positions.size() == 3);
    std::cout << "✓ size() works: " << positions.size() << std::endl;
}

void test_emplace_at() {
    std::cout << "\n=== Test emplace_at ===" << std::endl;
    sparse_array<Velocity> velocities;
    // Test emplace_at on empty slot
    auto& ref1 = velocities.emplace_at(5, 100.0f, 200.0f);
    assert(ref1.has_value());
    assert(ref1->dx == 100.0f);
    assert(ref1->dy == 200.0f);
    std::cout << "✓ emplace_at (empty slot) works" << std::endl;
    // Test emplace_at on existing slot (should replace)
    auto& ref2 = velocities.emplace_at(5, 300.0f, 400.0f);
    assert(ref2.has_value());
    assert(ref2->dx == 300.0f);
    assert(ref2->dy == 400.0f);
    std::cout << "✓ emplace_at (replace existing) works" << std::endl;
}

void test_erase() {
    std::cout << "\n=== Test erase ===" << std::endl;
    sparse_array<Position> positions;
    positions.insert_at(0, Position(1.0f, 2.0f));
    positions.insert_at(1, Position(3.0f, 4.0f));
    positions.insert_at(2, Position(5.0f, 6.0f));
    assert(positions[1].has_value());
    positions.erase(1);
    assert(positions[1].has_value() == false);
    assert(positions[0].has_value());
    assert(positions[2].has_value());
    std::cout << "✓ erase() works" << std::endl;
}

void test_get_index() {
    std::cout << "\n=== Test get_index ===" << std::endl;
    sparse_array<Position> positions;
    positions.insert_at(0, Position(10.0f, 20.0f));
    positions.insert_at(3, Position(30.0f, 40.0f));
    // IMPORTANT: Don't keep references across operations that might resize!
    // Get fresh references after all insertions are done
    auto& ref0 = positions[0];
    auto& ref3 = positions[3];
    // Test get_index with std::addressof
    auto idx0 = positions.get_index(ref0);
    auto idx3 = positions.get_index(ref3);
    assert(idx0 == 0);
    assert(idx3 == 3);
    std::cout << "✓ get_index() works (idx0=" << idx0 << ", idx3=" << idx3 << ")" << std::endl;
}

void test_iterators() {
    std::cout << "\n=== Test iterators ===" << std::endl;
    sparse_array<Position> positions;
    positions.insert_at(0, Position(1.0f, 1.0f));
    positions.insert_at(2, Position(2.0f, 2.0f));
    positions.insert_at(4, Position(3.0f, 3.0f));
    int count = 0;
    int with_value = 0;
    for (const auto& opt : positions) {
        count++;
        if (opt.has_value()) {
            with_value++;
            std::cout << "  Position at slot: x=" << opt->x << ", y=" << opt->y << std::endl;
        }
    }
    assert(count == 5);       // 0,1,2,3,4
    assert(with_value == 3);  // only 0,2,4 have values
    std::cout << "✓ iterators work (total=" << count << ", with_value=" << with_value << ")"
              << std::endl;
}

int main() {
    std::cout << "Testing sparse_array implementation...\n" << std::endl;
    try {
        test_basic_operations();
        test_emplace_at();
        test_erase();
        test_get_index();
        test_iterators();
        std::cout << "\n✅ All tests passed!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\n❌ Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}
