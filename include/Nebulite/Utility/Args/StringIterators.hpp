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

// TODO: Optimize with chunked comparison
/*

// Shared API:
std::size_t contiguousMemoryAvailable() const;
const char* data() const;

// Specialized API:
void advanceToNext(); // For spaniterator, essentially ++it (+necessary variable updates)
void advance(std::size_t n); // For stringviewiterator

// Then we can do:
const auto n = std::min(
    itA.contiguousMemoryAvailable(),
    itB.contiguousMemoryAvailable()
);

if(n == 0){
    // Legacy compare
    if (itA.get() != itB.get()) {
        return false;
    }
    ++itA;
    ++itB;
}
else{
    if (std::memcmp(a.data(), b.data(), n) != 0){
        return false;
    }
    a.advance(n); // or advanceToNext
    b.advance(n); // or advanceToNext
}
*/

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
    bool atWhitespacePosition = false;

    void adjustIterator() {
        if (it+1 == data.end()) {
            it = data.end();
        }
    }
public:
    explicit SpanIterator([[clang::lifetimebound]] std::span<std::string_view const> d) : data(d), it(d.begin()) {
        if (it != data.end() && it->empty()) {
            atWhitespacePosition = true;
        }
    }

    char get() {
        assert(it != data.end());
        if (atWhitespacePosition) {
            return ' ';
        }
        assert(pos < it->size());
        assert(it->at(pos) != ' ');
        return (*it)[pos];
    }

    void operator++() {
        if (atWhitespacePosition) {
            ++it;
            pos = 0;
            atWhitespacePosition = it != data.end() && it->empty();
            if (atWhitespacePosition) {
                adjustIterator();
            }
        }
        else if (pos + 1 == it->size()) {
            atWhitespacePosition = true;
            adjustIterator();
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
    explicit StringViewIterator([[clang::lifetimebound]] std::string_view const d) : data(d), it(d.begin()) {}

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
