/**
 * @file Generator.hpp
 * @brief Provides functions generating data structures.
 */

#ifndef NEBULITE_UTILITY_INITIALIZER_HPP
#define NEBULITE_UTILITY_INITIALIZER_HPP

#include <functional>

namespace Nebulite::Utility {

class Generate {
public:
    template<typename T, std::size_t N, typename F>
    static constexpr std::array<T, N> array(F&& generator) {
        return array_impl<T>(std::make_index_sequence<N>{}, std::forward<F>(generator));
    }

private:
    template<typename T, std::size_t... Is, typename F>
    static constexpr std::array<T, sizeof...(Is)>
    array_impl(std::index_sequence<Is...>, F&& generator) {
        return { { static_cast<T>(std::invoke(generator, Is))... } };
    }
};

} // namespace Nebulite::Utility
#endif // NEBULITE_UTILITY_INITIALIZER_HPP
