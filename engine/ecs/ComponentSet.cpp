#include "ComponentSet.h"

namespace conductor {
    ComponentSet::ComponentSet() {
        const size_t component_count =
            ComponentRegistry::size();

        const size_t word_count = 
            (component_count + BITS_PER_WORD - 1) /
            BITS_PER_WORD;

        m_bits.resize(word_count);
    }

    void ComponentSet::add(ComponentTypeId id) {
        const size_t word = word_index(id);

        if(word >= m_bits.size()) {
            m_bits.resize(word + 1, 0);
        }

        const uint64_t mask = bit_mask(id);

        if((m_bits[word] & mask) == 0) {
            m_bits[word] |= mask;
            ++m_size;
        }
    }

    bool ComponentSet::contains(ComponentTypeId id) const noexcept {        
        const size_t word = word_index(id);

        if(word >= m_bits.size()) {
            return false;
        }

        const uint64_t mask = bit_mask(id);

        return (m_bits[word] & mask) != 0;
    }

    bool ComponentSet::contains(const ComponentSet& other) const noexcept {
        if(m_bits.size() != other.m_bits.size()) {
            return false;
        }
        
        for(size_t i = 0; i < m_bits.size(); i++) {
            if((m_bits[i] & other.m_bits[i]) != other.m_bits[i]) {
                return false;
            }
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

    size_t ComponentSet::word_index(ComponentTypeId id) noexcept {
        return id / BITS_PER_WORD;
    }

    uint64_t ComponentSet::bit_mask(ComponentTypeId id) noexcept {
        return uint64_t{1} << (id % BITS_PER_WORD);
    }
}