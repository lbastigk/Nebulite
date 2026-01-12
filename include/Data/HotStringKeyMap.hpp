// C++
#ifndef DATA_HOT_STRING_KEY_MAP_HPP
#define DATA_HOT_STRING_KEY_MAP_HPP

#include <cstddef>
#include <iterator>
#include <memory>
#include <type_traits>
#include <utility>

// Nebulite
#include "Data/HotKeyMap.hpp"

namespace Nebulite::Data {

template<typename V>
class HotStringKeyMap {
private:
    // Use full 256 buckets for first-byte partitioning
    using MapType = Data::HotKeyMap<std::string, V>;
    static constexpr std::size_t BucketCount =
        static_cast<std::size_t>(std::numeric_limits<unsigned char>::max()) + 1;

    // Array of HotKeyMaps, one per possible first-character value.
    MapType map[BucketCount];

    // (Optional) sanity check
    static_assert(BucketCount == 256, "Expected 256 buckets for HotStringKeyMap");

    // Deduce iterator types from MapType::begin()
    using iterator_type = decltype(std::declval<MapType&>().begin());
    // If MapType has no const begin(), reuse the non-const iterator type for const_iterator.
    using const_iterator_type = iterator_type;
    using value_type = std::remove_reference_t<decltype(*std::declval<iterator_type>())>;

public:
    struct iterator {
        using iterator_category = std::forward_iterator_tag;
        using value_type = HotStringKeyMap::value_type;
        using reference = value_type&;
        using pointer = value_type*;
        using difference_type = std::ptrdiff_t;

        HotStringKeyMap* parent = nullptr;
        std::size_t idx = BucketCount;
        iterator_type cur;

        iterator() = default;
        explicit iterator(HotStringKeyMap* p, std::size_t const start = 0) : parent(p), idx(start) {
            if (idx < BucketCount) cur = parent->map[idx].begin();
            advance_to_valid();
        }

        void advance_to_valid() {
            while (parent && idx < BucketCount) {
                auto e = parent->map[idx].end();
                if (cur != e) return;
                ++idx;
                if (idx < BucketCount) cur = parent->map[idx].begin();
            }
        }

        iterator& operator++() {
            ++cur;
            advance_to_valid();
            return *this;
        }
        iterator operator++(int) { iterator tmp = *this; ++(*this); return tmp; }

        reference operator*() const { return *cur; }
        pointer operator->() const { return std::addressof(*cur); }

        bool operator==(const iterator& o) const {
            if (parent != o.parent) return false;
            if (idx == BucketCount && o.idx == BucketCount) return true;
            return idx == o.idx && cur == o.cur;
        }
        bool operator!=(const iterator& o) const { return !(*this == o); }
    };

    struct const_iterator {
        using iterator_category = std::forward_iterator_tag;
        using value_type = HotStringKeyMap::value_type;
        using reference = const value_type&;
        using pointer = const value_type*;
        using difference_type = std::ptrdiff_t;

        const HotStringKeyMap* parent = nullptr;
        std::size_t idx = BucketCount;
        const_iterator_type cur;

        const_iterator() = default;
        explicit const_iterator(const HotStringKeyMap* p, std::size_t const start = 0) : parent(p), idx(start) {
            if (idx < BucketCount) cur = const_cast<MapType&>(parent->map[idx]).begin();
            advance_to_valid();
        }

        void advance_to_valid() {
            while (parent && idx < BucketCount) {
                auto e = const_cast<MapType&>(parent->map[idx]).end();
                if (cur != e) return;
                ++idx;
                if (idx < BucketCount) cur = const_cast<MapType&>(parent->map[idx]).begin();
            }
        }

        const_iterator& operator++() {
            ++cur;
            advance_to_valid();
            return *this;
        }
        const_iterator operator++(int) { const_iterator tmp = *this; ++(*this); return tmp; }

        reference operator*() const { return *cur; }
        pointer operator->() const { return std::addressof(*cur); }

        bool operator==(const const_iterator& o) const {
            if (parent != o.parent) return false;
            if (idx == BucketCount && o.idx == BucketCount) return true;
            return idx == o.idx && cur == o.cur;
        }
        bool operator!=(const const_iterator& o) const { return !(*this == o); }
    };

    iterator begin() { return iterator(this, 0); }
    iterator end()   { return iterator(this, BucketCount); }

    const_iterator begin() const { return const_iterator(this, 0); }
    const_iterator end()   const { return const_iterator(this, BucketCount); }

    const_iterator cbegin() const { return begin(); }
    const_iterator cend()   const { return end(); }

    V& operator[](std::string const& key) {
        if (key.empty()) {
            return map[0][key]; // Handle empty key case
        }
        auto const firstChar = static_cast<unsigned char>(key[0]);
        return map[firstChar][key];
    }

    iterator find(std::string const& key) {
        if (key.empty()) {
            auto it = map[0].find(key);
            if (it != map[0].end()) {
                return iterator(this, 0);
            } else {
                return end();
            }
        }
        auto const firstChar = static_cast<unsigned char>(key[0]);
        auto it = map[firstChar].find(key);
        if (it != map[firstChar].end()) {
            return iterator(this, firstChar);
        } else {
            return end();
        }
    }
};

} // namespace Nebulite::Data

#endif // DATA_HOT_STRING_KEY_MAP_HPP
