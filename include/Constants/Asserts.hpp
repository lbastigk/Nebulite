#ifndef CONSTANTS_ASSERTS_HPP
#define CONSTANTS_ASSERTS_HPP

#include <string_view>

bool constexpr endsWithNewline(std::string_view const& str) {
    return !str.empty() && str.back() == '\n';
}

#endif // CONSTANTS_ASSERTS_HPP
