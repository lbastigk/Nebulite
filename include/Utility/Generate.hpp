/**
 * @file Generator.hpp
 * @brief Provides functions generating data structures.
 */

#ifndef NEBULITE_DOMAINMODULE_INITIALIZER_HPP
#define NEBULITE_DOMAINMODULE_INITIALIZER_HPP

#include <functional>

namespace Nebulite::Utility {

class Generate {
public:
    template<typename T, std::size_t N, typename F>
    static std::array<T,N> array(F&& generator) {
        std::array<T,N> result{};

        // Apply generator on each member of the array
        for (auto&& [index, element] : enumerate(result)) {
            element = static_cast<T>(std::invoke(std::forward<F>(generator), index));
        }
        return result;
    }

private:
    template<typename Range>
    static auto enumerate(Range&& range) {
        return std::views::zip(std::views::iota(size_t{0}), range);
    }
};

} // namespace Nebulite::Utility
#endif // NEBULITE_DOMAINMODULE_INITIALIZER_HPP
