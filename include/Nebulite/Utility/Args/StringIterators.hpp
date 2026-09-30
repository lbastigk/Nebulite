#ifndef NEBULITE_UTILITY_ARGS_STRINGITERATORS_HPP
#define NEBULITE_UTILITY_ARGS_STRINGITERATORS_HPP

//------------------------------------------
// Includes

// Standard library
#include <concepts>
#include <cstddef>
#include <span>
#include <string_view>

//------------------------------------------
// Concepts

template <typename T>
concept StringIteratorLike = requires(T t) {
    { t.get() } -> std::convertible_to<char>;
    { t.endReached() } -> std::convertible_to<bool>;
    t.operator++();
};

//------------------------------------------
namespace Nebulite::Utility::Args {

class SpanIterator {
    std::span<std::string_view const> data;
    std::span<std::string_view const>::iterator it;
    std::size_t pos = 0;
    bool advancedIt = false;
public:
    SpanIterator(std::span<std::string_view const> d) : data(d), it(d.begin()) {}

    char get() {
        return advancedIt ? ' ' : (*it)[pos];
    }

    void operator++() {
        if (pos == data.size()) {
            ++it;
            pos = 0;
            advancedIt = true;
        }
        else {
            ++pos;
            advancedIt = false;
        }
    }

    [[nodiscard]] bool endReached() const {
        return it == data.end();
    }
};

class StringViewIterator {
    std::string_view data;
    std::string_view::iterator it;
public:
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
