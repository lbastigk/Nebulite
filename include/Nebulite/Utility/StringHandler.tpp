#ifndef NEBULITE_UTILITY_STRINGHANDLER_TPP
#define NEBULITE_UTILITY_STRINGHANDLER_TPP

//------------------------------------------
// Includes

// Standard library
#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>

//------------------------------------------
// Conditional includes

#ifndef NEBULITE_UTILITY_STRINGHANDLER_HPP
    #include "Nebulite/Utility/StringHandler.hpp"
#endif // NEBULITE_UTILITY_STRINGHANDLER_HPP

//------------------------------------------
namespace Nebulite::Utility {
/**
 * @brief Prepares and argument for correct string combination.
 * @details If you notice that certain types aren't turned into the correct string representation,
 *          or the compilation fails, you can modify the behaviour here.
 * @tparam T The Type of the argument to prepare
 * @param t The argument to prepare
 * @return The prepared argument for correct string representation.
 */
template<typename T>
static auto prepareArg(T&& t) { // NOLINT
    using U = std::remove_reference_t<T>;

    if constexpr (std::is_array_v<U>) {
        return std::string_view(t); // NOLINT
    } else {
        return std::forward<T>(t);
    }
}

template <typename... Args>
bool StringHandler::startsWithSequence(std::string_view str, Args... args) {
    static_assert(sizeof...(Args) > 0, "At least one sequence argument is required for startsWithSequence");
    auto impl = [&]<typename First, typename... Rest>(auto&& self, std::string_view remaining, First first, Rest... rest) -> bool {
        if (!remaining.starts_with(first)) {
            return false;
        }

        if constexpr (sizeof...(Rest) == 0) {
            return true;
        } else {
            return self(self, remaining.substr(first.size()), rest...);
        }
    };
    return impl(impl, str, args...);
}

template <typename... Args>
std::string StringHandler::combine(Args&&... args) {
    std::ostringstream workingBuffer{};
    if constexpr (sizeof...(args) != 0) {
        (workingBuffer << ... << prepareArg(std::forward<Args>(args)));
    }
    return workingBuffer.str();
}

template <typename... Args>
std::string StringHandler::combineWithNewline(Args&&... args) {
    std::ostringstream workingBuffer{};
    if constexpr (sizeof...(args) != 0) {
        (workingBuffer << ... << prepareArg(std::forward<Args>(args)));
    }
    workingBuffer << '\n';
    return workingBuffer.str();
}

} // namespace Nebulite::Utility
#endif // NEBULITE_UTILITY_STRINGHANDLER_TPP
