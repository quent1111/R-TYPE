#pragma once

#include "sparse_array.hpp"
#include "entity.hpp"
#include <unordered_map>
#include <unordered_set>
#include <typeindex>
#include <memory>
#include <any>
#include <stdexcept>
#include <functional>
#include <vector>
#include <queue>

class registry {
public:
	// Type alias for entity (kept for backward compatibility)
	using entity_t = entity;

private:

	// Base class for type-erasure of sparse_array<T>
	class component_array_base {
	public:
		virtual ~component_array_base() noexcept = default;
		// Remove component for a given entity (used when killing entity)
		virtual void erase_entity(entity_t entity) = 0;
		// Get size of the array
		virtual std::size_t size() const noexcept = 0;
	};


	// Template wrapper that holds the actual sparse_array<Component>
    template <typename Component>
	class component_array : public component_array_base {
	public:
		sparse_array<Component> data;

		void erase_entity(entity_t entity) override {
			data.erase(static_cast<std::size_t>(entity));
		}

		std::size_t size() const noexcept override {
			return data.size();
		}
	};

	// Storage: type_index -> pointer to component_array_base
	std::unordered_map<std::type_index, std::unique_ptr<component_array_base>> _components_arrays;
	// Storage for erase functions: type_index -> erase function
	std::unordered_map<std::type_index, std::function<void(registry&, entity_t const&)>> _erase_functions;
	// Entity management
	std::size_t _next_entity_id = 0;
	std::queue<std::size_t> _dead_entities;  // Pool of dead entity IDs for reuse
	// Track which components each entity has (optimization for kill_entity)
	std::unordered_map<std::size_t, std::unordered_set<std::type_index>> _entity_components;
	// Systems storage: vector of functions that take registry& as parameter
	std::vector<std::function<void(registry&)>> _systems;

public:
	// Default constructor
	registry() = default;

	// Explicitly register a component type (creates the sparse_array)
	template <typename Component>
	sparse_array<Component>& register_component()
	{
		std::type_index type_idx(typeid(Component));
		// Check if already registered
		auto it = _components_arrays.find(type_idx);
		if (it != _components_arrays.end()) {
			// Already registered, return existing
			auto* wrapper = static_cast<component_array<Component>*>(it->second.get());
			return wrapper->data;
		}
		// Create new wrapper and sparse_array
		auto wrapper = std::make_unique<component_array<Component>>();
		auto& ref = wrapper->data;
		_components_arrays[type_idx] = std::move(wrapper);
		// Create and store erase function for this component type
		_erase_functions[type_idx] = [](registry& reg, entity_t const& entity) {
			reg.get_components<Component>().erase(static_cast<std::size_t>(entity));
		};
		return ref;
	}

	// Get the sparse_array for a given component type (non-const)
	template <typename Component>
	sparse_array<Component>& get_components()
	{
		std::type_index type_idx(typeid(Component));
		auto it = _components_arrays.find(type_idx);
		if (it == _components_arrays.end()) {
			// Auto-register if not found
			return register_component<Component>();
		}
		// Safe downcast
		auto* wrapper = static_cast<component_array<Component>*>(it->second.get());
		return wrapper->data;
	}

	// Get the sparse_array for a given component type (const)
	template <typename Component>
	sparse_array<Component> const& get_components() const
	{
		std::type_index type_idx(typeid(Component));
		auto it = _components_arrays.find(type_idx);
		if (it == _components_arrays.end()) {
			throw std::runtime_error("Component type not registered");
		}
		auto* wrapper = static_cast<component_array<Component>*>(it->second.get());
		return wrapper->data;
	}

	// Create a new entity and return its ID
	entity_t spawn_entity()
	{
		// Reuse a dead entity ID if available
		if (!_dead_entities.empty()) {
			std::size_t reused_id = _dead_entities.front();
			_dead_entities.pop();
			return entity(reused_id);
		}
		// Otherwise create a new ID
		return entity(_next_entity_id++);
	}

	// Get the entity that will be created next (useful for pre-allocation)
	constexpr entity_t entity_from_index(std::size_t idx) const noexcept
	{
		return entity(idx);
	}

	// Kill an entity (removes all its components from all arrays and adds ID to dead pool)
	void kill_entity(entity_t const& e)
	{
		std::size_t entity_id = e.id();
		
		// OPTIMIZATION: Only erase components that this entity actually has
		auto it = _entity_components.find(entity_id);
		if (it != _entity_components.end()) {
			// Erase only the components this entity has
			for (const auto& type_idx : it->second) {
				auto erase_it = _erase_functions.find(type_idx);
				if (erase_it != _erase_functions.end()) {
					erase_it->second(*this, e);
				}
			}
			// Clear the component tracking for this entity
			_entity_components.erase(it);
		}
		
		// Add entity ID to dead pool for reuse
		_dead_entities.push(entity_id);
	}

	// Add a component to an entity (universal reference)
	template <typename Component>
	typename sparse_array<std::remove_reference_t<Component>>::reference_type 
	add_component(entity_t entity, Component&& component)
	{
		using ComponentType = std::remove_reference_t<Component>;
		std::type_index type_idx(typeid(ComponentType));
		
		// Track that this entity now has this component type
		_entity_components[entity.id()].insert(type_idx);
		
		return get_components<ComponentType>().insert_at(static_cast<std::size_t>(entity), std::forward<Component>(component));
	}

	// Emplace a component for an entity
	template <typename Component, typename... Params>
	typename sparse_array<Component>::reference_type emplace_component(entity_t entity, Params&&... params)
	{
		std::type_index type_idx(typeid(Component));
		
		// Track that this entity now has this component type
		_entity_components[entity.id()].insert(type_idx);
		
		return get_components<Component>().emplace_at(static_cast<std::size_t>(entity), std::forward<Params>(params)...);
	}

	// Remove a specific component from an entity
	template <typename Component>
	void remove_component(entity_t entity)
	{
		std::type_index type_idx(typeid(Component));
		
		// Update tracking
		auto it = _entity_components.find(entity.id());
		if (it != _entity_components.end()) {
			it->second.erase(type_idx);
			// If entity has no more components, remove the entry
			if (it->second.empty()) {
				_entity_components.erase(it);
			}
		}
		
		get_components<Component>().erase(static_cast<std::size_t>(entity));
	}

	// Check if an entity has a specific component
	template <typename Component>
	bool has_component(entity_t entity) const
	{
		// OPTIMIZATION: Check tracking map first (single lookup)
		auto it = _entity_components.find(entity.id());
		if (it == _entity_components.end())
			return false;
		
		std::type_index type_idx(typeid(Component));
		return it->second.find(type_idx) != it->second.end();
	}

	// Get a component for an entity (non-const) - returns optional reference
	template <typename Component>
	typename sparse_array<Component>::reference_type get_component(entity_t entity)
	{
		return get_components<Component>()[static_cast<std::size_t>(entity)];
	}

	// Get a component for an entity (const) - returns optional reference
	template <typename Component>
	typename sparse_array<Component>::const_reference_type get_component(entity_t entity) const
	{
		return get_components<Component>()[static_cast<std::size_t>(entity)];
	}

	// =================== SYSTEMS ===================
	
	// Add a system to the registry
	// The system function will be called with registry& and the requested component arrays
	// Usage: reg.add_system<Position, Velocity>(my_system_function);
	template <typename... Components, typename Function>
	void add_system(Function&& f)
	{
		// Create a lambda that captures the function and calls it with the proper component arrays
		auto system_wrapper = [func = std::forward<Function>(f)](registry& reg) {
			// Get all the component arrays needed by the system
			func(reg, reg.get_components<Components>()...);
		};
		
		_systems.push_back(std::move(system_wrapper));
	}
	
	// Alternative: take function by const reference (useful for function pointers)
	template <typename... Components, typename Function>
	void add_system(Function const& f)
	{
		auto system_wrapper = [func = f](registry& reg) {
			func(reg, reg.get_components<Components>()...);
		};
		
		_systems.push_back(std::move(system_wrapper));
	}
	
	// Run all registered systems
	void run_systems()
	{
		for (auto& system : _systems) {
			system(*this);
		}
	}
};

