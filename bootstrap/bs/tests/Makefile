# Makefile for ECS Tests

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -I..
TARGETS = test_sparse_array test_registry test_entity_reuse

.PHONY: all clean run

all: $(TARGETS)

test_sparse_array: test_sparse_array.cpp ../sparse_array.hpp
	$(CXX) $(CXXFLAGS) -o $@ $<

test_registry: test_registry.cpp ../registry.hpp ../sparse_array.hpp ../entity.hpp
	$(CXX) $(CXXFLAGS) -o $@ $<

test_entity_reuse: test_entity_reuse.cpp ../registry.hpp ../sparse_array.hpp ../entity.hpp
	$(CXX) $(CXXFLAGS) -o $@ $<

run: all
	@echo "=== Running sparse_array tests ==="
	@./test_sparse_array
	@echo ""
	@echo "=== Running registry tests ==="
	@./test_registry
	@echo ""
	@echo "=== Running entity reuse tests ==="
	@./test_entity_reuse

clean:
	rm -f $(TARGETS) *.o
