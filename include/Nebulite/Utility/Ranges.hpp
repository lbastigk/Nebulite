#ifndef NEBULITE_UTILITY_RANGES_HPP
#define NEBULITE_UTILITY_RANGES_HPP

//------------------------------------------
// Includes

// Standard library
#include <concepts>
#include <cstddef>
#include <functional>
#include <iterator>
#include <optional>
#include <ranges>
#include <stdexcept>
#include <type_traits>
#include <vector>

//------------------------------------------
// Concepts

template <typename T>
concept OptionalLike = requires(T x) {
    typename std::remove_cvref_t<T>::value_type;
    { x.has_value() } -> std::convertible_to<bool>;
    x.value();
    *x;
};

//------------------------------------------
namespace Nebulite::Utility {

class Ranges {
public:
    /**
     * @brief Collects a range of optional values into an optional vector. If any value in the range is empty, the result will be an empty optional.
     */
    static struct collect_optional_fn : std::ranges::range_adaptor_closure<collect_optional_fn> {
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
    } constexpr collectOptional{};

    /**
     * @brief Collects a range of values into a vector with a transformation function.
     *        If the transformation function returns an empty optional for any value, the result will be an empty optional.
     */
    static struct try_transform_fn {
        /**
         * @brief Collects a range of values into a vector with a transformation function.
         *        If the transformation function returns an empty optional for any value, the result will be an empty optional.
         * @tparam F The type of the transformation function.
         */
        template <typename F>
        struct closure : std::ranges::range_adaptor_closure<closure<F>> {
            F f;

            explicit constexpr closure(F func) : f(std::move(func)){}

            template <std::ranges::input_range R>
            auto operator()(R&& r) const {
                using Optional = std::remove_cvref_t<std::invoke_result_t<F&, std::ranges::range_reference_t<R>>>;
                using T = Optional::value_type;

                std::vector<T> result;
                if constexpr (std::ranges::sized_range<R>)
                    result.reserve(std::ranges::size(r));

                for (auto&& elem : std::forward<R>(r)) {
                    auto value = std::invoke(f, elem);
                    if (!value)
                        return std::optional<std::vector<T>>{std::nullopt};

                    result.push_back(std::move(*value));
                }

                return std::optional<std::vector<T>>{std::move(result)};
            }
        };

        template <typename F>
        auto operator()(F f) const {
            return closure<std::decay_t<F>>{
                std::move(f)
            };
        }
    } constexpr tryTransform{};

    /**
     * @brief An alternative implementation of std::views::enumerate, where the enumeration index is of type std::size_t
     */
    static struct enumerate_fn : std::ranges::range_adaptor_closure<enumerate_fn> {
        template <typename T>
        struct enumerate_item {
            std::size_t index;
            T value;
        };

        // A simple enumerate view implementation that yields enumerate_item{index, element}
        template <std::ranges::view V>
        requires std::ranges::input_range<V>
        struct enumerate_view : std::ranges::view_interface<enumerate_view<V>> {
            V base = V();

            enumerate_view() = default;
            explicit enumerate_view(V b) : base(std::move(b)) {}

            using base_iterator_t = std::ranges::iterator_t<V>;
            using base_sentinel_t = std::ranges::sentinel_t<V>;

            struct iterator {
                base_iterator_t it{};
                std::size_t index = 0;

                using iterator_category = std::input_iterator_tag;
                using value_type = enumerate_item<std::ranges::range_reference_t<V>>;
                using difference_type = std::ptrdiff_t;

                iterator() = default;
                iterator(base_iterator_t i, std::size_t const idx) : it(i), index(idx) {}

                value_type operator*() const {
                    return value_type{index, *it};
                }

                iterator& operator++() {
                    ++it;
                    ++index;
                    return *this;
                }

                void operator++(int) { ++*this; }

                friend bool operator==(iterator const& a, base_iterator_t const& b) { return a.it == b; }
            };

            struct sentinel {
                base_sentinel_t end;
            };

            iterator begin() { return iterator{std::ranges::begin(base), 0}; }
            sentinel end() { return sentinel{std::ranges::end(base)}; }

            // compare iterator and sentinel
            friend bool operator==(iterator const& it, sentinel const& s) { return it.it == s.end; }
            friend bool operator==(sentinel const& s, iterator const& it) { return it == s; }
        };

        template <std::ranges::viewable_range R>
        auto operator()(R&& r) const {
            using V = std::views::all_t<R>;
            return enumerate_view<V>(std::views::all(std::forward<R>(r)));
        }
    } constexpr enumerate{};

    /**
     * @brief Generates a range of powers of two up to a specified maximum value.
     * @param inclusiveMax The inclusive maximum value for the range of powers of two.
     * @return A view of the powers of two: [2, 4, 8, ..., inclusiveMax]
     */
    static auto constexpr powersOfTwo(std::size_t const inclusiveMax) {
        return std::views::iota(0)
            | std::views::transform([](std::size_t const x) { return std::size_t{2} << x; })
            | std::views::take_while([inclusiveMax](std::size_t const x) { return x <= inclusiveMax; });
    }

    /**
     * @brief The stdlib iota has issues with static analyzers, even though it works fine. This is a workaround
     * @tparam ReturnType The type of the indices to be generated (default is std::size_t)
     * @param start The start value
     * @param exclusiveMax The exclusive maximum value for the range
     * @return A view of the iota range: [0, exclusiveMax) as ReturnType
     */
    template<typename ReturnType = std::size_t>
    static auto constexpr iota(std::size_t const start, std::size_t const exclusiveMax) {
        if constexpr(std::is_same_v<ReturnType, int>) {
            return std::views::iota(static_cast<int>(start), static_cast<int>(exclusiveMax));
        }
        else {
            return std::views::iota(static_cast<int>(start), static_cast<int>(exclusiveMax))
                | std::views::transform([](auto const idx) { return static_cast<ReturnType>(idx); });
        }
    }

    /**
     * @brief Generate indices for a range from 0 to exclusiveMax - 1. This is a workaround for issues with static analyzers and std::views::iota.
     * @tparam ReturnType The type of the indices to be generated (default is std::size_t)
     * @param exclusiveMax The exclusive maximum value for the range
     * @return A view of the iota range: [0, exclusiveMax)
     */
    template<typename ReturnType = std::size_t>
    static auto constexpr indices(std::size_t const exclusiveMax) {
        if (exclusiveMax == 0) {
            throw std::invalid_argument("Exclusive max index must be greater than 0.");
        }
        return iota<ReturnType>(0, exclusiveMax);
    }



    /**
     * @brief Checks if all elements in a range are equal and satisfy a given predicate.
     */
    static struct all_equal_and_fn : std::ranges::range_adaptor_closure<all_equal_and_fn>{
        template<class Pred>
        struct all_equal_and_closure : range_adaptor_closure<all_equal_and_closure<Pred>>{
            Pred pred;

            explicit all_equal_and_closure(Pred p) : pred(std::move(p)){}

            template<std::ranges::input_range R>
            bool operator()(R&& r) const {
                auto it = std::ranges::begin(r);
                auto end = std::ranges::end(r);
                (void)std::forward<R>(r);

                if (it == end)
                    return true;

                auto const& first = *it;

                if (!std::invoke(pred, first))
                    return false;

                for (++it; it != end; ++it) {
                    if (!std::invoke(pred, *it))
                        return false;

                    if (*it != first)
                        return false;
                }

                return true;
            }
        };

        template<class Pred>
        auto operator()(Pred pred) const {
            return all_equal_and_closure<std::decay_t<Pred>>{
                std::move(pred)
            };
        }
    } constexpr all_equal_and{};

    /**
     * @brief Checks if all elements in a range are equal
     */
    static struct all_equal_fn : std::ranges::range_adaptor_closure<all_equal_fn>{
        template<std::ranges::input_range R>
        bool operator()(R&& r) const {
            auto it = std::ranges::begin(r);
            auto end = std::ranges::end(r);
            (void)std::forward<R>(r);

            if (it == end)
                return true;

            auto const& first = *it;

            for (++it; it != end; ++it) {
                if (*it != first)
                    return false;
            }

            return true;
        }
    } constexpr all_equal{};
};

} // namespace Nebulite::Utility
#endif // NEBULITE_UTILITY_RANGES_HPP
