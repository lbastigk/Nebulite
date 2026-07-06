#ifndef UTILITY_RANGES_HPP
#define UTILITY_RANGES_HPP

//------------------------------------------
// Includes

// Standard library
#include <concepts>
#include <optional>
#include <ranges>
#include <type_traits>
#include <vector>

//------------------------------------------
// Concepts

template <typename T>
concept OptionalLike = requires(T x) {
    typename std::remove_cvref_t<T>::value_type;
    { x.has_value() } -> std::convertible_to<bool>;
    *x;
};

//------------------------------------------
namespace Nebulite::Utility {

class Ranges {
public:
    /**
     * @brief Collects a range of optional values into an optional vector. If any value in the range is empty, the result will be an empty optional.
     */
    struct collect_optional_fn : std::ranges::range_adaptor_closure<collect_optional_fn> {
        /**
         * @brief Collects a range of optional values into an optional vector. If any value in the range is empty, the result will be an empty optional.
         * @tparam R The type of the input range.
         * @param r The input range of optional values.
         * @return An optional vector containing the collected values, or an empty optional if any value in the range is empty.
         */
        template <std::ranges::input_range R> requires OptionalLike<std::ranges::range_value_t<R>>
        auto operator()(R&& r) const{
            using Optional = std::remove_cvref_t<std::ranges::range_value_t<R>>;
            using T = Optional::value_type;

            std::vector<T> result;
            if constexpr (std::ranges::sized_range<R>) {
                result.reserve(std::ranges::size(r));
            }
            for (auto&& opt : std::forward<R>(r)) {
                if (!opt) {
                    return std::optional<std::vector<T>>{std::nullopt};
                }
                result.push_back(*std::forward<decltype(opt)>(opt));
            }
            return std::optional<std::vector<T>>{std::move(result)};
        }
    };

    static collect_optional_fn constexpr collectOptional{};

};

} // namespace Nebulite::Utility
#endif // UTILITY_RANGES_HPP
