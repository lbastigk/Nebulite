#ifndef NEBULITE_UTILITY_STRINGITERATORS_HPP
#define NEBULITE_UTILITY_STRINGITERATORS_HPP

//------------------------------------------
// Includes

// Standard library
#include <cstddef>
#include <span>
#include <string_view>

//------------------------------------------
namespace Nebulite::Utility {

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
        advancedIt = false;
        if (pos == data.size()) {
            ++it;
            pos = 0;
            advancedIt = true;
        }
        else {
            pos++;
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

} // namespace Nebulite::Utility
#endif // NEBULITE_UTILITY_STRINGITERATORS_HPP
