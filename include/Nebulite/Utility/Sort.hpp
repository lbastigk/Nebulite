#ifndef NEBULITE_UTILITY_SORT_HPP
#define NEBULITE_UTILITY_SORT_HPP

//------------------------------------------
// Includes

// Standard library
#include <algorithm>
#include <cctype>
#include <cstddef>
#include <numeric>
#include <optional>
#include <string_view>
#include <vector>

//------------------------------------------
namespace Nebulite::Utility {
/**
 * @class Nebulite::Utility::Sort
 * @brief Sorting utilities
 */
class Sort {
public:
    static constexpr struct CaseSensitiveLess {
        template <class L, class R>
        requires requires (L const& l, R const& r){
            std::string_view{l};
            std::string_view{r};
        }

        constexpr bool operator()(L const& lhs, R const& rhs) const {
            std::string_view const a{lhs};
            std::string_view const b{rhs};

            return std::ranges::lexicographical_compare(a, b, [](unsigned char const& x, unsigned char const& y) {
                    return x < y;
                }
            );
        }
    } caseSensitiveLess{};

    static constexpr struct CaseInsensitiveLess {
        template <class L, class R>
        requires requires (L const& l, R const& r) {
            std::string_view{l};
            std::string_view{r};
        }

        constexpr bool operator()(L const& lhs, R const& rhs) const {
            std::string_view const a{lhs};
            std::string_view const b{rhs};

            return std::ranges::lexicographical_compare(
                a, b,
                [](unsigned char const x, unsigned char const y) {
                    return std::tolower(x) < std::tolower(y);
                }
            );
        }
    } caseInsensitiveLess{};

    /**
     * @brief Produces a stable ordering of indices using a fallible comparator.
     * @details The comparator must return std::optional<bool>, with true meaning
     *          its first index precedes its second. Both directions are checked;
     *          missing results and mutually preceding pairs fail the sort.
     *          The comparator should define a strict weak ordering: transitivity
     *          and all other violations are not exhaustively checked.
     * @param size Number of indices, from zero to size - 1.
     * @param compare Comparator for two indices.
     * @return Sorted indices, or std::nullopt if a comparison fails or contradicts its reverse.
     */
    template <typename Comparator>
    static std::optional<std::vector<std::size_t>> stableIndexOrder(std::size_t const size, Comparator const& compare) {
        std::vector<std::size_t> order(size);
        std::ranges::iota(order, std::size_t{0});
        std::vector<std::size_t> merged;
        merged.reserve(size);
        for (std::size_t width = 1; width < size;) {
            merged.clear();
            for (std::size_t start = 0; start < size;) {
                auto const middle = start + std::min(width, size - start);
                auto const end = middle + std::min(width, size - middle);
                if (!mergeRuns(order, merged, start, middle, end, compare)) return std::nullopt;
                start = end;
            }
            order.swap(merged);
            if (width > size / 2) break;
            width *= 2;
        }
        return order;
    }

private:
    /**
     * @brief Stably merges two adjacent runs of indices into an output vector.
     * @details The runs are [start, middle) and [middle, end). Equal elements retain
     *          their input order. Each comparison is checked in both directions;
     *          failure can leave a partially appended run in merged.
     * @param order Indices containing the two sorted runs.
     * @param merged Output vector to which the merged indices are appended.
     * @param start Beginning of the first run.
     * @param middle End of the first run and beginning of the second.
     * @param end End of the second run.
     * @param compare Comparator returning an optional boolean for two indices.
     * @return False if a comparison fails or both indices compare less than each other; true otherwise.
     */
    template <typename Comparator>
    static bool mergeRuns(std::vector<std::size_t> const& order, std::vector<std::size_t>& merged, std::size_t const start, std::size_t const middle, std::size_t const end, Comparator const& compare) {
        std::size_t left = start;
        std::size_t right = middle;
        while (left < middle && right < end) {
            auto const rightBeforeLeft = compare(order[right], order[left]);
            auto const leftBeforeRight = compare(order[left], order[right]);
            if (!rightBeforeLeft || !leftBeforeRight || (*rightBeforeLeft && *leftBeforeRight)) return false;
            if (*rightBeforeLeft) {
                merged.push_back(order[right]);
                right++;
            }
            else {
                merged.push_back(order[left]);
                left++;
            }
        }
        while (left < middle) {
            merged.push_back(order[left]);
            left++;
        }
        while (right < end) {
            merged.push_back(order[right]);
            right++;
        }
        return true;
    }
};
} // namespace Nebulite::Utility
#endif // NEBULITE_UTILITY_SORT_HPP
