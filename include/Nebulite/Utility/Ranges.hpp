#ifndef NEBULITE_UTILITY_RANGES_HPP
#define NEBULITE_UTILITY_RANGES_HPP

//------------------------------------------
// Includes

// Standard library
#include <concepts>
#include <cstddef>
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

    /**
     * @brief Generates a range of powers of two up to a specified maximum value.
     * @param inclusiveMax The inclusive maximum value for the range of powers of two.
     * @return A view of the powers of two: [2, 4, 8, ..., inclusiveMax]
     */
    static auto constexpr powersOfTwo(std::size_t const inclusiveMax) {
        return std::views::iota(1)
             | std::views::transform([](std::size_t const x) { return x << 1; })
             | std::views::take_while([inclusiveMax](std::size_t const x) { return x <= inclusiveMax; });
    }

    /**
     * @brief The stdlib iota has issues with static analyzers, even though it works fine. This is a workaround
     * @param exclusiveMax The exclusive maximum value for the range
     * @return A view of the iota range: [0, exclusiveMax)
     */
    static auto constexpr iota(std::size_t const exclusiveMax) {
        return std::views::iota(0)
            | std::views::take_while([exclusiveMax](std::size_t const x) { return x < exclusiveMax; })
            | std::views::transform([](auto const idx) { return static_cast<std::size_t>(idx); });
    }

};

} // namespace Nebulite::Utility
#endif // NEBULITE_UTILITY_RANGES_HPP
