#include "registry.hpp"
#include <iostream>
#include <cassert>

struct Position {
	float x, y;
	Position(float x_ = 0, float y_ = 0) : x(x_), y(y_) {}
};

void test_entity_reuse() {
	std::cout << "=== Test Entity ID Reuse ===" << std::endl;
	
	registry reg;
	
	// Create 5 entities
	auto e0 = reg.spawn_entity();
	auto e1 = reg.spawn_entity();
	auto e2 = reg.spawn_entity();
	auto e3 = reg.spawn_entity();
	auto e4 = reg.spawn_entity();
	
	std::cout << "Created entities: " << e0.id() << ", " << e1.id() << ", " 
	          << e2.id() << ", " << e3.id() << ", " << e4.id() << std::endl;
	
	assert(e0.id() == 0);
	assert(e1.id() == 1);
	assert(e2.id() == 2);
	assert(e3.id() == 3);
	assert(e4.id() == 4);
	
	// Add components to some entities
	reg.add_component(e1, Position(10, 20));
	reg.add_component(e2, Position(30, 40));
	reg.add_component(e3, Position(50, 60));
	
	// Kill entities 1 and 3
	reg.kill_entity(e1);
	reg.kill_entity(e3);
	
	std::cout << "Killed entities: " << e1.id() << " and " << e3.id() << std::endl;
	
	// Verify they no longer have components
	assert(!reg.has_component<Position>(e1));
	assert(reg.has_component<Position>(e2));  // e2 should still have component
	assert(!reg.has_component<Position>(e3));
	
	// Spawn new entities - should reuse killed IDs
	auto e5 = reg.spawn_entity();
	auto e6 = reg.spawn_entity();
	auto e7 = reg.spawn_entity();  // This one gets a new ID
	
	std::cout << "New entities after reuse: " << e5.id() << ", " << e6.id() << ", " << e7.id() << std::endl;
	
	// e5 and e6 should reuse IDs 1 and 3 (order might vary based on queue)
	// e7 should get ID 5 (next available)
	assert((e5.id() == 1 || e5.id() == 3));
	assert((e6.id() == 1 || e6.id() == 3));
	assert(e5.id() != e6.id());  // They should be different
	assert(e7.id() == 5);
	
	std::cout << "✓ Entity ID reuse works correctly!" << std::endl;
	std::cout << "  Reused IDs: " << e5.id() << ", " << e6.id() << std::endl;
	std::cout << "  New ID: " << e7.id() << std::endl;
}

void test_entity_operations() {
	std::cout << "\n=== Test Entity Operations ===" << std::endl;
	
	registry reg;
	
	auto e1 = reg.spawn_entity();
	auto e2 = reg.spawn_entity();
	
	// Test comparison operators
	assert(e1 == e1);
	assert(e1 != e2);
	assert(e1 < e2);
	
	// Test conversion to size_t
	std::size_t id1 = e1;
	std::size_t id2 = e2;
	assert(id1 == 0);
	assert(id2 == 1);
	
	// Test entity_from_index
	auto e3 = reg.entity_from_index(42);
	assert(e3.id() == 42);
	
	std::cout << "✓ Entity comparison operators work" << std::endl;
	std::cout << "✓ Entity conversion to size_t works" << std::endl;
	std::cout << "✓ entity_from_index works" << std::endl;
}

void test_erase_functions() {
	std::cout << "\n=== Test Erase Functions ===" << std::endl;
	
	registry reg;
	
	// Register multiple component types
	reg.register_component<Position>();
	
	auto e = reg.spawn_entity();
	reg.add_component(e, Position(100, 200));
	
	assert(reg.has_component<Position>(e));
	
	// Kill entity should use the stored erase function
	reg.kill_entity(e);
	
	assert(!reg.has_component<Position>(e));
	
	std::cout << "✓ Erase functions created by register_component work" << std::endl;
}

int main() {
	std::cout << "Testing new registry features...\n" << std::endl;
	
	try {
		test_entity_reuse();
		test_entity_operations();
		test_erase_functions();
		
		std::cout << "\n✅ All new feature tests passed!" << std::endl;
		return 0;
	} catch (const std::exception& e) {
		std::cerr << "\n❌ Test failed with exception: " << e.what() << std::endl;
		return 1;
	}
}
