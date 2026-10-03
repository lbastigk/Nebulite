#ifndef NEBULITE_UTILITY_ARGS_STRINGITERATORS_HPP
#define NEBULITE_UTILITY_ARGS_STRINGITERATORS_HPP

//------------------------------------------
// Includes

// Standard library
#include <algorithm>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <memory>
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
    { t.contiguousMemoryAvailable() } -> std::convertible_to<std::size_t>;
    { t.contiguousData() } -> std::convertible_to<char const*>;
    t.advanceBy(std::size_t{});
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
    bool atWhitespacePosition = false;

    void adjustIterator() {
        if (it+1 == data.end()) {
            it = data.end();
        }
    }

    void advanceToNextWord() {
        ++it;
        pos = 0;
        atWhitespacePosition = it != data.end() && it->empty();
        if (atWhitespacePosition) {
            adjustIterator();
        }
    }

public:
    explicit SpanIterator([[clang::lifetimebound]] std::span<std::string_view const> d) : data(d), it(d.begin()) {
        if (it != data.end() && it->empty()) {
            atWhitespacePosition = true;
        }
    }

    [[nodiscard]] std::size_t contiguousMemoryAvailable() const {
        if (atWhitespacePosition) {
            return 0;
        }
        return it->size() - pos;
    }

    [[nodiscard]] char const* contiguousData() const {
        assert(!atWhitespacePosition);
        return it->data() + pos;
    }

    void advanceBy(std::size_t n) {
        while (n > 0) {
            if (atWhitespacePosition) {
                advanceToNextWord();
                --n;
                return;
            }
            assert(it->size() - pos > 0);
            if (auto const available = it->size() - pos - 1; available == 0) {
                atWhitespacePosition = true;
                adjustIterator();
                --n;
            }
            else {
                auto const advance = std::min(n, available);
                pos += advance;
                n -= advance;
            }
        }
    }

    [[nodiscard]] char get() const {
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
            advanceToNextWord();
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

    [[nodiscard]] std::size_t contiguousMemoryAvailable() const {
        return static_cast<std::size_t>(data.end() - it);
    }

    [[nodiscard]] char const* contiguousData() const {
        return std::to_address(it);
    }

    void advanceBy(std::size_t const n) {
        it += n;
    }

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
