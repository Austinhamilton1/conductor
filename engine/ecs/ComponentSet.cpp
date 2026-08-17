#include "ComponentSet.h"

#include <algorithm>

namespace conductor {
    void ComponentSet::add(ComponentTypeId id) {
        const size_t word = word_index(id);

        if (word >= m_bits.size()) {
            m_bits.resize(word + 1, 0);
        }

        const uint64_t mask = bit_mask(id);

        if ((m_bits[word] & mask) == 0) {
            m_bits[word] |= mask;
            ++m_size;
        }
    }

    bool ComponentSet::contains(
        ComponentTypeId id
    ) const noexcept {
        const size_t word = word_index(id);

        if (word >= m_bits.size()) {
            return false;
        }

        const uint64_t mask = bit_mask(id);

        return (m_bits[word] & mask) != 0;
    }

    bool ComponentSet::contains(
        const ComponentSet& other
    ) const noexcept {

        /*
         * Every word in 'other' must be completely contained
         * within this set.
         *
         * If 'other' has words beyond our size, those words
         * must be zero for containment to succeed.
         */
        for (size_t i = 0; i < other.m_bits.size(); ++i) {

            const uint64_t other_word =
                other.m_bits[i];

            if (other_word == 0)
                continue;

            if (i >= m_bits.size())
                return false;

            if ((m_bits[i] & other_word) != other_word)
                return false;
        }

        return true;
    }

    bool ComponentSet::empty() const noexcept {
        return m_size == 0;
    }

    size_t ComponentSet::size() const noexcept {
        return m_size;
    }

    void ComponentSet::clear() {
        std::fill(
            m_bits.begin(),
            m_bits.end(),
            uint64_t{0}
        );

        m_size = 0;
    }

    size_t ComponentSet::word_index(
        ComponentTypeId id
    ) noexcept {
        return id / BITS_PER_WORD;
    }

    uint64_t ComponentSet::bit_mask(
        ComponentTypeId id
    ) noexcept {
        return uint64_t{1} << (id % BITS_PER_WORD);
    }

    bool ComponentSet::operator==(const ComponentSet& other) const noexcept {
        const size_t word_count =
            std::max(
                m_bits.size(),
                other.m_bits.size()
            );

        for (size_t i = 0; i < word_count; ++i) {
            const uint64_t lhs =
                i < m_bits.size()
                    ? m_bits[i]
                    : 0;

            const uint64_t rhs =
                i < other.m_bits.size()
                    ? other.m_bits[i]
                    : 0;

            if (lhs != rhs)
                return false;
        }

        return true;
    }
}