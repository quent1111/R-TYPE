#pragma once

#include "entity.hpp"
#include "sparse_array.hpp"

#include <any>

#include <functional>
#include <memory>
#include <queue>
#include <stdexcept>
#include <typeindex>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class registry {
public:
    using entity_t = entity;

private:
    class component_array_base {
    public:
        virtual ~component_array_base() noexcept = default;
        virtual void erase_entity(entity_t entity) = 0;
        virtual std::size_t size() const noexcept = 0;
    };

    template <typename Component>
    class component_array : public component_array_base {
    public:
        sparse_array<Component> data;

        void erase_entity(entity_t entity) override {
            data.erase(static_cast<std::size_t>(entity));
        }

        std::size_t size() const noexcept override { return data.size(); }
    };

    std::unordered_map<std::type_index, std::unique_ptr<component_array_base>> _components_arrays;
    std::unordered_map<std::type_index, std::function<void(registry&, entity_t const&)>>
        _erase_functions;
    std::size_t _next_entity_id = 0;
    std::queue<std::size_t> _dead_entities;
    std::unordered_map<std::size_t, std::unordered_set<std::type_index>> _entity_components;

public:
    registry() = default;

    template <typename Component>
    sparse_array<Component>& register_component() {
        std::type_index type_idx(typeid(Component));
        auto it = _components_arrays.find(type_idx);
        if (it != _components_arrays.end()) {
            auto* wrapper = static_cast<component_array<Component>*>(it->second.get());
            return wrapper->data;
        }
        auto wrapper = std::make_unique<component_array<Component>>();
        auto& ref = wrapper->data;
        _components_arrays[type_idx] = std::move(wrapper);
        _erase_functions[type_idx] = [](registry& reg, entity_t const& entity) {
            reg.get_components<Component>().erase(static_cast<std::size_t>(entity));
        };
        return ref;
    }

    template <typename Component>
    sparse_array<Component>& get_components() {
        std::type_index type_idx(typeid(Component));
        auto it = _components_arrays.find(type_idx);
        if (it == _components_arrays.end()) {
            return register_component<Component>();
        }
        auto* wrapper = static_cast<component_array<Component>*>(it->second.get());
        return wrapper->data;
    }

    template <typename Component>
    sparse_array<Component> const& get_components() const {
        std::type_index type_idx(typeid(Component));
        auto it = _components_arrays.find(type_idx);
        if (it == _components_arrays.end()) {
            throw std::runtime_error("Component type not registered");
        }
        auto* wrapper = static_cast<component_array<Component>*>(it->second.get());
        return wrapper->data;
    }

    entity_t spawn_entity() {
        if (!_dead_entities.empty()) {
            std::size_t reused_id = _dead_entities.front();
            _dead_entities.pop();
            return entity(reused_id);
        }
        return entity(_next_entity_id++);
    }

    constexpr entity_t entity_from_index(std::size_t idx) const noexcept { return entity(idx); }

    void kill_entity(entity_t const& e) {
        std::size_t entity_id = e.id();

        auto it = _entity_components.find(entity_id);
        if (it != _entity_components.end()) {
            for (const auto& type_idx : it->second) {
                auto erase_it = _erase_functions.find(type_idx);
                if (erase_it != _erase_functions.end()) {
                    erase_it->second(*this, e);
                }
            }
            _entity_components.erase(it);
        }

        _dead_entities.push(entity_id);
    }

    template <typename Component>
    typename sparse_array<std::remove_reference_t<Component>>::reference_type
    add_component(entity_t entity, Component&& component) {
        using ComponentType = std::remove_reference_t<Component>;
        std::type_index type_idx(typeid(ComponentType));

        _entity_components[entity.id()].insert(type_idx);

        return get_components<ComponentType>().insert_at(static_cast<std::size_t>(entity),
                                                         std::forward<Component>(component));
    }

    template <typename Component, typename... Params>
    typename sparse_array<Component>::reference_type emplace_component(entity_t entity,
                                                                       Params&&... params) {
        std::type_index type_idx(typeid(Component));

        _entity_components[entity.id()].insert(type_idx);

        return get_components<Component>().emplace_at(static_cast<std::size_t>(entity),
                                                      std::forward<Params>(params)...);
    }

    template <typename Component>
    void remove_component(entity_t entity) {
        std::type_index type_idx(typeid(Component));

        auto it = _entity_components.find(entity.id());
        if (it != _entity_components.end()) {
            it->second.erase(type_idx);
            if (it->second.empty()) {
                _entity_components.erase(it);
            }
        }

        get_components<Component>().erase(static_cast<std::size_t>(entity));
    }

    template <typename Component>
    bool has_component(entity_t entity) const {
        auto it = _entity_components.find(entity.id());
        if (it == _entity_components.end())
            return false;

        std::type_index type_idx(typeid(Component));
        return it->second.find(type_idx) != it->second.end();
    }

    template <typename Component>
    typename sparse_array<Component>::reference_type get_component(entity_t entity) {
        return get_components<Component>()[static_cast<std::size_t>(entity)];
    }

    template <typename Component>
    typename sparse_array<Component>::const_reference_type get_component(entity_t entity) const {
        return get_components<Component>()[static_cast<std::size_t>(entity)];
    }
};

namespace std {
template <>
struct hash<entity> {
    std::size_t operator()(const entity& e) const {
        return std::hash<std::size_t>{}(static_cast<std::size_t>(e));
    }
};
}  // namespace std