#pragma once

#include <cstdint>
#include <limits>

namespace conductor {
    class DeferredEntity {
    public:
        using Id = uint32_t;

        static constexpr Id INVALID_ID = std::numeric_limits<Id>::max();

        constexpr DeferredEntity() noexcept = default;

        constexpr explicit DeferredEntity(Id id) noexcept
            : m_id(id) {}

        constexpr Id id() const noexcept {
            return m_id;
        }

        constexpr bool valid() const noexcept {
            return m_id != INVALID_ID;
        }

        constexpr bool operator==(const DeferredEntity&) const noexcept = default;

    private:
        Id m_id = INVALID_ID;
    };
}