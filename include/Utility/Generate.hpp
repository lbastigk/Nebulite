/**
 * @file Generate.hpp
 * @brief Provides functions generating data structures.
 */

#ifndef NEBULITE_UTILITY_INITIALIZER_HPP
#define NEBULITE_UTILITY_INITIALIZER_HPP

#include <array>
#include <functional>
#include <string>

namespace Nebulite::Utility {

class Generate {
public:
    template<typename T, std::size_t N, typename F>
    static constexpr std::array<T, N> array(F&& generator) {
        return impl<std::array<T, N>,T>(std::make_index_sequence<N>{}, std::forward<F>(generator));
    }

    template<std::size_t N, typename F>
    static constexpr std::string string(F&& generator) {
        return impl<std::string, char>(std::make_index_sequence<N>{}, std::forward<F>(generator));
    }

private:
    template<class C, typename T, std::size_t... Is, typename F>
    static constexpr C impl(std::index_sequence<Is...>, F&& generator) {
        return { { static_cast<T>(std::invoke(generator, Is))... } };
    }
};

} // namespace Nebulite::Utility
#endif // NEBULITE_UTILITY_INITIALIZER_HPP
