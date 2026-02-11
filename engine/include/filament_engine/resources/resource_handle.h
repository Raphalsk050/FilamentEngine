#pragma once

#include <cstdint>
#include <functional>

namespace fe {

// Type-safe handle to a resource managed by ResourceManager.
// Does not own the resource — the ResourceManager handles lifetime.
template <typename T>
class ResourceHandle {
public:
    ResourceHandle() = default;
    explicit ResourceHandle(uint32_t id) : m_id(id) {}

    bool isValid() const { return m_id != INVALID_ID; }
    uint32_t getId() const { return m_id; }

    bool operator==(const ResourceHandle& other) const { return m_id == other.m_id; }
    bool operator!=(const ResourceHandle& other) const { return m_id != other.m_id; }
    explicit operator bool() const { return isValid(); }

    static constexpr uint32_t INVALID_ID = 0;

private:
    uint32_t m_id = INVALID_ID;
};

} // namespace fe

// Hash support for use in containers
namespace std {
template <typename T>
struct hash<fe::ResourceHandle<T>> {
    size_t operator()(const fe::ResourceHandle<T>& handle) const {
        return std::hash<uint32_t>{}(handle.getId());
    }
};
} // namespace std
