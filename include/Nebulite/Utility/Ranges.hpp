#ifndef NEBULITE_UTILITY_RANGES_HPP
#define NEBULITE_UTILITY_RANGES_HPP

//------------------------------------------
// Includes

// Standard library
#include <algorithm>
#include <bit>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <functional>
#include <iterator>
#include <optional>
#include <ranges>
#include <type_traits>
#include <vector>

// Nebulite
#include "Nebulite/Utility/Convert/Bits.hpp"
#include "Nebulite/Utility/Generate.hpp"

//------------------------------------------
/**
 * @namespace Nebulite::Utility::Ranges
 * @brief Pipeable range utilities
 */
namespace Nebulite::Utility::Ranges {

//------------------------------------------
// Concepts

template <typename T>
concept OptionalLike = requires(T x) {
    typename std::remove_cvref_t<T>::value_type;
    { x.has_value() } -> std::convertible_to<bool>;
    x.value();
    *x;
};

template<class R>
using index_reference_t = decltype(std::declval<R&>()[std::declval<std::ranges::range_size_t<R>>()]);

template<class R>
concept IndexableRange = std::ranges::range<R>
    && requires(R& r, std::ranges::range_size_t<R> i){
    r[i];
    };

template<class R>
concept MutableRange = IndexableRange<R>
    && requires(R& r, std::ranges::range_size_t<R> i) {
    requires std::is_lvalue_reference_v<index_reference_t<R>>;
    requires std::assignable_from<index_reference_t<R>, std::ranges::range_value_t<R>>;
    requires std::swappable<index_reference_t<R>>;
    };

//------------------------------------------
// Range Utilities

/**
 * @brief Collects a range of optional values into an optional vector. If any value in the range is empty, the result will be an empty optional.
 */
static struct CollectOptional : std::ranges::range_adaptor_closure<CollectOptional> {
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
static struct TryTransform {
    /**
     * @brief Collects a range of values into a vector with a transformation function.
     *        If the transformation function returns an empty optional for any value, the result will be an empty optional.
     * @tparam F The type of the transformation function.
     */
    template <typename F>
    struct Closure : std::ranges::range_adaptor_closure<Closure<F>> {
        F f;

        explicit constexpr Closure(F func) : f(std::move(func)){}

        template <std::ranges::input_range R>
        auto operator()(R&& r) const {
            using Optional = std::remove_cvref_t<std::invoke_result_t<F&, std::ranges::range_reference_t<R>>>;
            using T = Optional::value_type;

            std::vector<T> result;
            if constexpr (std::ranges::sized_range<R>) {
                result.reserve(std::ranges::size(r));
            }

            for (auto&& elem : std::forward<R>(r)) {
                auto value = std::invoke(f, elem);
                if (!value) {
                    return std::optional<std::vector<T>>{std::nullopt};
                }
                result.push_back(std::move(*value));
            }

            return std::optional<std::vector<T>>{std::move(result)};
        }
    };

    template <typename F>
    auto operator()(F f) const {
        return Closure<std::decay_t<F>>{
            std::move(f),
        };
    }
} constexpr tryTransform{};

/**
 * @brief An alternative implementation of std::views::enumerate, where the enumeration index is of type std::size_t
 */
static struct Enumerate : std::ranges::range_adaptor_closure<Enumerate> {
    template <typename T>
    struct EnumerateItem {
        std::size_t index;
        T value;
    };

    // A simple enumerate view implementation that yields EnumerateItem{index, element}
    template <std::ranges::view V>
    requires std::ranges::input_range<V>
    struct EnumerateView : std::ranges::view_interface<EnumerateView<V>> {
        V base = V();

        EnumerateView() = default;
        explicit EnumerateView(V b) : base(std::move(b)) {}

        using BaseIterator = std::ranges::iterator_t<V>;
        using BaseSentinel = std::ranges::sentinel_t<V>;

        struct Iterator {
            BaseIterator it{};
            std::size_t index = 0;

            using iterator_category = std::input_iterator_tag; // NOLINT
            using value_type = EnumerateItem<std::ranges::range_reference_t<V>>; // NOLINT
            using difference_type = std::ptrdiff_t; // NOLINT

            Iterator() = default;
            Iterator(BaseIterator const& i, std::size_t const idx) : it(i), index(idx) {}

            value_type operator*() const {
                return value_type{index, *it};
            }

            Iterator& operator++() {
                ++it;
                ++index;
                return *this;
            }

            void operator++(int) { ++*this; }

            friend bool operator==(Iterator const& a, BaseIterator const& b) { return a.it == b; }
        };

        struct Sentinel {
            BaseSentinel end;
        };

        Iterator begin() { return Iterator{std::ranges::begin(base), 0}; }
        Sentinel end() { return Sentinel{std::ranges::end(base)}; }

        // compare Iterator and sentinel
        friend bool operator==(Iterator const& it, Sentinel const& s) { return it.it == s.end; }
        friend bool operator==(Sentinel const& s, Iterator const& it) { return it == s; }
    };

    template <std::ranges::viewable_range R>
    auto operator()(R&& r) const {
        using V = std::views::all_t<R>;
        return EnumerateView<V>(std::views::all(std::forward<R>(r)));
    }
} constexpr enumerate{};

/**
 * @brief Checks if all elements in a range are equal and satisfy a given predicate.
 */
static struct AllEqualAnd : std::ranges::range_adaptor_closure<AllEqualAnd>{
    template<class Pred>
    struct Closure : range_adaptor_closure<Closure<Pred>>{
        Pred pred;

        explicit Closure(Pred p) : pred(std::move(p)){}

        template<std::ranges::input_range R>
        bool operator()(R&& r) const {
            if (std::ranges::empty(r)) {
                return true;
            }
            auto const& first = *std::ranges::begin(r);
            return std::ranges::all_of(std::forward<R>(r), [this, first](auto const& elem) {
                return std::invoke(pred, elem) && elem == first;
            });
        }
    };

    template<class Pred>
    auto operator()(Pred pred) const {
        return Closure<std::decay_t<Pred>>{
            std::move(pred),
        };
    }
} constexpr allEqualAnd{};

/**
 * @brief Checks if all elements in a range are equal
 */
static struct AllEqual : std::ranges::range_adaptor_closure<AllEqual>{
    template<std::ranges::input_range R>
    bool operator()(R&& r) const {
        if (std::ranges::empty(r)) {
            return true;
        }
        auto const& first = *std::ranges::begin(r);
        return std::ranges::all_of(std::forward<R>(r), [first](auto const& elem) {
            return elem == first;
        });
    }
} constexpr allEqual{};

/**
 * @brief Normalizes a given range by a value n, keeping the type the same.
 */
static struct Normalize {
    struct Closure : std::ranges::range_adaptor_closure<Closure> {
        std::size_t const n;

        // Specialization for mutable ranges: modifies the range in place
        template<MutableRange R>
        auto operator()(R&& r) const {
            auto const dN = static_cast<double>(n);
            for (auto& v : r) {
                v /= dN;
            }
            return std::forward<R>(r);
        }

        // Specialization for input ranges: returns a transformed view
        template<std::ranges::input_range R> requires (!MutableRange<R>)
        auto operator()(R&& r) const {
            auto const dN = static_cast<double>(n);
            return std::forward<R>(r)
                | std::views::transform([dN] (auto const& v) {
                    return v / dN;
                });
        }
    };

    auto operator()(std::size_t const n) const {
        return Closure{
            {}, // NOLINT
            n,
        };
    }
} constexpr normalize{};

struct InplaceBitReversalPermutation {
    struct Closure : std::ranges::range_adaptor_closure<Closure> {
        std::size_t n;

        Closure(std::size_t rangeSize) : n(rangeSize) {}

        template<MutableRange R>
        static void apply(R& r, std::size_t const n) {
            auto const bitCount = static_cast<std::size_t>(std::bit_width(n - 1));
            assert(r.size() == n);
            assert(std::has_single_bit(n)); // n must be a power of two

            // Since we verified the size, we can access directly without bounds checking. This is a performance optimization.
            auto* data = std::ranges::data(r);

            for (auto const i : Generate::indices(n)) {
                if (auto const b = Convert::Bits::reverse(i, bitCount); i < b) {
                    std::ranges::swap(data[i], data[b]);
                }
            }
        }

        template<MutableRange R>
        auto operator()(R&& r) const {
            apply(r, n);
            return std::forward<R>(r);
        }
    };

    auto operator()(std::size_t const n) const {
        return Closure(n);
    }
} constexpr inplaceBitReversalPermutation; // NOLINT

/**
 * @brief Creates a copy of a range with elements permuted according to the bit-reversal permutation.
 */
struct CopyBitReversalPermutation {
    template<typename T>
    struct Closure : std::ranges::range_adaptor_closure<Closure<T>> {
        std::size_t n;
        T t;

        Closure(std::size_t rangeSize, T const defaultValue) : n(rangeSize), t(defaultValue) {}

        template<IndexableRange R>
        static std::vector<T> apply(R const& r, std::size_t const n, T const defaultValue) {
            assert(std::has_single_bit(n)); // n must be a power of two

            std::vector<T> result(n);
            auto const bitCount = static_cast<std::size_t>(std::bit_width(n - 1));

            // Since we verified the size, we can access directly without bounds checking. This is a performance optimization.
            auto* dstData = std::ranges::data(result);
            auto* srcData = std::ranges::data(r);

            for (auto const source : Generate::indices(n)) {
                auto const destination = Convert::Bits::reverse(source, bitCount);

                if (source >= r.size()) {
                    dstData[destination] = defaultValue;
                }
                else {
                    dstData[destination] = static_cast<T>(srcData[source]);
                }
            }
            return std::move(result);
        }

        template<IndexableRange R>
        std::vector<T> operator()(R const& r) const {
            return apply(r, n, t);
        }
    };

    template<typename T>
    auto operator()(std::size_t const n, T const defaultValue = T{}) const {
        return Closure<T>(n, std::move(defaultValue));
    }
} constexpr copyBitReversalPermutation;

} // namespace Nebulite::Utility::Ranges
#endif // NEBULITE_UTILITY_RANGES_HPP
