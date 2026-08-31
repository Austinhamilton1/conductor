#include "ComponentStorageRegistry.h"

#include <stdexcept>

namespace conductor {
    ComponentStorageRegistry::~ComponentStorageRegistry() {
        for(auto& entry : m_storages) {
            if(entry.valid()) {
                entry.destroy(entry.storage);

                entry.storage = nullptr;
                entry.destroy = nullptr;
                entry.add = nullptr;
                entry.remove = nullptr;
                entry.flush = nullptr;
            }
        }
    }

    void ComponentStorageRegistry::ensure_capacity(ComponentTypeId id) {
        const auto required =
            static_cast<size_t>(id) + 1;

        if(m_storages.size() < required) {
            m_storages.resize(required);
        }
    }

    bool ComponentStorageRegistry::contains(ComponentTypeId id) const noexcept {
        const auto index =
            static_cast<size_t>(id);

        return index < m_storages.size()
            && m_storages[index].valid();
    }

    void ComponentStorageRegistry::destroy(ComponentTypeId id) {
        auto& entry = get_entry(id);

        if(!entry.valid()) {
            throw std::logic_error(
                "Component storage does not exist"
            );
        }

        entry.destroy(entry.storage);

        entry.storage = nullptr;
        entry.destroy = nullptr;

        --m_size;
    }

    ComponentStorageRegistry::StorageEntry&
    ComponentStorageRegistry::entry(ComponentTypeId id) {
        return get_entry(id);
    }

    const ComponentStorageRegistry::StorageEntry&
    ComponentStorageRegistry::entry(ComponentTypeId id) const {
        return get_entry(id);
    }

    ComponentStorageRegistry::StorageEntry&
    ComponentStorageRegistry::get_entry(ComponentTypeId id) {
        const auto index =
            static_cast<size_t>(id);

        if(index >= m_storages.size()) {
            throw std::out_of_range(
                "Invalid component type ID"
            );
        }

        auto& entry = m_storages[index];

        if(!entry.valid()) {
            throw std::logic_error(
                "Component storage does not exist"
            );
        }

        return entry;
    }

    const ComponentStorageRegistry::StorageEntry&
    ComponentStorageRegistry::get_entry(ComponentTypeId id) const {
        const auto index =
            static_cast<size_t>(id);

        if(index >= m_storages.size()) {
            throw std::out_of_range(
                "Invalid component type ID"
            );
        }

        const auto& entry = m_storages[index];

        if(!entry.valid()) {
            throw std::logic_error(
                "Component storage does not exist"
            );
        }

        return entry;
    }
}