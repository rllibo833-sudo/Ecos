#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <stdexcept>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <utility>
#include <vector>

namespace ecos::studio {

// Generation-stamped handle: destroyed entities cannot be accessed through stale IDs.
struct Entity {
    static constexpr std::uint32_t invalid_index = std::numeric_limits<std::uint32_t>::max();

    std::uint32_t index{invalid_index};
    std::uint32_t generation{0};

    [[nodiscard]] constexpr explicit operator bool() const noexcept {
        return index != invalid_index && generation != 0;
    }
    friend constexpr bool operator==(const Entity&, const Entity&) noexcept = default;
};

struct EntityHash {
    [[nodiscard]] std::size_t operator()(Entity entity) const noexcept {
        const auto a = static_cast<std::size_t>(entity.index);
        const auto b = static_cast<std::size_t>(entity.generation);
        return (a << 1U) ^ (b + static_cast<std::size_t>(0x9e3779b9U) + (a << 6U) + (a >> 2U));
    }
};

// Runtime entity/component storage. Components are owned by the registry and are
// removed automatically when their entity is destroyed. This is a native C++20
// foundation; renderer, scene authoring and scripting systems can build on it.
class EntityRegistry {
    struct ComponentPoolBase {
        virtual ~ComponentPoolBase() = default;
        virtual void erase(Entity entity) noexcept = 0;
    };

    template <typename T>
    struct ComponentPool final : ComponentPoolBase {
        std::unordered_map<Entity, T, EntityHash> values;

        void erase(Entity entity) noexcept override {
            values.erase(entity);
        }
    };

public:
    [[nodiscard]] Entity create();
    [[nodiscard]] bool destroy(Entity entity);
    [[nodiscard]] bool alive(Entity entity) const noexcept;
    [[nodiscard]] std::size_t entity_count() const noexcept;

    template <typename T, typename... Args>
    T& emplace(Entity entity, Args&&... args) {
        if (!alive(entity)) throw std::invalid_argument("Cannot add a component to a stale entity");
        auto& values = ensure_pool<T>().values;
        auto [it, inserted] = values.try_emplace(entity, std::forward<Args>(args)...);
        if (!inserted) throw std::logic_error("Entity already has this component type");
        return it->second;
    }

    template <typename T>
    [[nodiscard]] T* get(Entity entity) noexcept {
        if (!alive(entity)) return nullptr;
        auto* storage = find_pool<T>();
        if (!storage) return nullptr;
        const auto it = storage->values.find(entity);
        return it == storage->values.end() ? nullptr : &it->second;
    }

    template <typename T>
    [[nodiscard]] const T* get(Entity entity) const noexcept {
        if (!alive(entity)) return nullptr;
        const auto* storage = find_pool<T>();
        if (!storage) return nullptr;
        const auto it = storage->values.find(entity);
        return it == storage->values.end() ? nullptr : &it->second;
    }

    template <typename T>
    [[nodiscard]] bool has(Entity entity) const noexcept {
        return get<T>(entity) != nullptr;
    }

    template <typename T>
    bool remove(Entity entity) noexcept {
        if (!alive(entity)) return false;
        auto* storage = find_pool<T>();
        return storage && storage->values.erase(entity) != 0;
    }

    // Returns live entities carrying T in stable index order for deterministic systems.
    template <typename T>
    [[nodiscard]] std::vector<Entity> entities_with() const {
        std::vector<Entity> result;
        const auto* storage = find_pool<T>();
        if (!storage) return result;
        result.reserve(storage->values.size());
        for (const auto& [entity, component] : storage->values) {
            (void)component;
            if (alive(entity)) result.push_back(entity);
        }
        std::sort(result.begin(), result.end(), [](Entity a, Entity b) {
            return a.index < b.index;
        });
        return result;
    }

    template <typename T>
    [[nodiscard]] std::size_t component_count() const noexcept {
        const auto* storage = find_pool<T>();
        return storage ? storage->values.size() : 0;
    }

private:
    template <typename T>
    [[nodiscard]] ComponentPool<T>* find_pool() noexcept {
        const auto it = pools_.find(std::type_index(typeid(T)));
        return it == pools_.end() ? nullptr : static_cast<ComponentPool<T>*>(it->second.get());
    }

    template <typename T>
    [[nodiscard]] const ComponentPool<T>* find_pool() const noexcept {
        const auto it = pools_.find(std::type_index(typeid(T)));
        return it == pools_.end() ? nullptr : static_cast<const ComponentPool<T>*>(it->second.get());
    }

    template <typename T>
    ComponentPool<T>& ensure_pool() {
        const auto key = std::type_index(typeid(T));
        auto it = pools_.find(key);
        if (it == pools_.end()) {
            auto storage = std::make_unique<ComponentPool<T>>();
            auto* result = storage.get();
            pools_.emplace(key, std::move(storage));
            return *result;
        }
        return *static_cast<ComponentPool<T>*>(it->second.get());
    }

    std::vector<std::uint32_t> generations_;
    std::vector<std::uint8_t> alive_;
    std::vector<std::uint32_t> free_indices_;
    std::unordered_map<std::type_index, std::unique_ptr<ComponentPoolBase>> pools_;
    std::size_t entity_count_{0};
};

} // namespace ecos::studio
