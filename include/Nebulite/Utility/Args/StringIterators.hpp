#ifndef NEBULITE_UTILITY_ARGS_STRINGITERATORS_HPP
#define NEBULITE_UTILITY_ARGS_STRINGITERATORS_HPP

//------------------------------------------
// Includes

// Standard library
#include <cassert>
#include <concepts>
#include <cstddef>
#include <span>
#include <string_view>

//------------------------------------------
// Concepts

/**
 * @brief Concept for StringIterator types used in Argument parsing
 */
template <typename T>
concept StringIteratorLike = requires(T t) {
    { t.get() } -> std::convertible_to<char>;
    { t.endReached() } -> std::convertible_to<bool>;
    t.operator++();
};

//------------------------------------------
namespace Nebulite::Utility::Args {
/**
 * @brief Allows for continuous iteration through a span of string_views.
 * @details Assumes one whitespace inbetween each of the string_views
 */
class SpanIterator {
    std::span<std::string_view const> data;
    std::span<std::string_view const>::iterator it;
    std::size_t pos = 0;
    bool advancedIt = false;
public:
    SpanIterator([[clang::lifetimebound]] std::span<std::string_view const> d) : data(d), it(d.begin()) {}

    char get() {
        assert(it != data.end());
        assert((*it)[pos] != ' ');
        return advancedIt ? ' ' : (*it)[pos];
    }

    void operator++() {
        if (advancedIt) {
            ++it;
            pos = 0;
            advancedIt = false;
        }
        else if (pos + 1 == it->size()) {
            advancedIt = true;
        }
        else {
            ++pos;
        }
    }

    [[nodiscard]] bool endReached() const {
        return it == data.end();
    }
};

/**
 * @brief Helper class with the same interface as SpanIterator
 */
class StringViewIterator {
    std::string_view data;
    std::string_view::iterator it;
public:
    StringViewIterator([[clang::lifetimebound]] std::string_view const d) : data(d), it(d.begin()) {}

    [[nodiscard]] char get() const {
        return *it;
    }

    void operator++() {
        ++it;
    }

    [[nodiscard]] bool endReached() const {
        return it == data.end();
    }
};

} // namespace Nebulite::Utility::Args
#endif // NEBULITE_UTILITY_ARGS_STRINGITERATORS_HPP
