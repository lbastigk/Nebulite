#ifndef NEBULITE_UTILITY_GENERATE_HPP
#define NEBULITE_UTILITY_GENERATE_HPP

//------------------------------------------
// Includes

// Standard library
#include <array>
#include <bit>
#include <cstddef>
#include <functional>
#include <ranges>
#include <stdexcept>
#include <string>
#include <utility>

// Nebulite
#include "Nebulite/Utility/CompileTimeEvaluate.hpp"

//------------------------------------------
namespace Nebulite::Utility {

class Generate {
public:
    /**
     * @brief Generates an std::array based on a provided generator function
     * @tparam T The storage type of the array
     * @tparam N The size of the array
     * @tparam F The generator function type
     * @param generator The generator function
     * @return The generated array
     */
    template<typename T, std::size_t N, typename F>
    static constexpr std::array<T, N> array(F&& generator) {
        return impl<std::array<T, N>,T>(
            std::make_index_sequence<N>{},
            std::forward<F>(generator)
        );
    }

    /**
     * @brief Generates an std::string based on a provided generator function
     * @tparam N The size of the string
     * @tparam F The generator function type
     * @param generator The generator function
     * @return The generated string
     */
    template<std::size_t N, typename F>
    static constexpr std::string string(F&& generator) {
        return impl<std::string, char>(
            std::make_index_sequence<N>{},
            std::forward<F>(generator)
        );
    }

    /**
     * @brief Generates a pairing table for N elements, where each element is paired with every other element exactly once across multiple rows
     * @details In each row, each element is paired once. Either with itself or with another element.
     *          In total, there are 2*N rows and N/2 pairs per row, with a total of N*N pairs across the entire table.
     * @tparam N The number of elements to pair. Must be of the form 4*n^k for some n >= 1 and k >= 0 to ensure proper pairing and round generation.
     * @return A 2D array representing the pairing table, where each row contains pairs of indices representing the paired elements for that round.
     */
    template <std::size_t N>
    static consteval auto pairingTable() {
        static_assert(N%4 == 0 && N >=4 && CompileTimeEvaluate::isPowerOfTwo(N/4), "N must be of type 4*n^k for some n >= 1 and k >= 0 to ensure proper pairing and round generation.");

        static auto constexpr roundCount = 2*N;
        static auto constexpr pairsPerRound = N/2;

        std::array<std::array<std::pair<std::size_t, std::size_t>, pairsPerRound>, roundCount> rounds{};
        std::array<std::size_t, roundCount> roundsAssigned{}; // Track how many pairs have been assigned to each round
        static std::array<std::array<std::size_t, 4>, 4> constexpr baseMask = {
            {
                {1, 2, 5, 6},
                {4, 3, 8, 7},
                {5, 6, 1, 2},
                {8, 7, 4, 3},
            },
        };

        for (std::size_t row = 0; row < N ; row++) {
            for (std::size_t col = 0; col < N; col++) {
                // For each pair (row, col), determine which round it belongs to using the base mask and the row/col indices
                auto const localRound = baseMask.at(row % 4).at(col % 4) - 1; // Get the local round index from the base mask
                auto const rowGroup = row / 4; // Determine the group of the row (0 to N/4 - 1)
                auto const colGroup = col / 4; // Determine the group of the column
                auto const totalRound = localRound + 8*(rowGroup ^ colGroup);
                rounds[totalRound][roundsAssigned[totalRound]] = {row, col};
                ++roundsAssigned[totalRound];
            }
        }

        // Additional checks, just to be sure the generated table is correct.
        for (std::size_t r = 0; r < roundCount; ++r) {
            // Each round must have exactly pairsPerRound pairs
            if (roundsAssigned[r] != pairsPerRound) {
                throw std::runtime_error("Invalid pairing: round does not contain the expected number of pairs!");
            }
        }
        for (std::size_t r = 0; r < roundCount; ++r) {
            for (std::size_t i = 0; i < pairsPerRound; ++i) {
                for (std::size_t j = i + 1; j < pairsPerRound; ++j) {
                    if (rounds[r][i] == rounds[r][j]) {
                        throw std::runtime_error("Duplicate pair detected in a round!");
                    }
                }
            }
        }

        return std::move(rounds);
    }

    /**
     * @brief Generates a range of powers of two up to a specified maximum value.
     * @param inclusiveMax The inclusive maximum value for the range of powers of two.
     * @return A view of the powers of two: [2, 4, 8, ..., inclusiveMax]
     * @todo Add option to include 1
     */
    static auto constexpr powersOfTwo(std::size_t const inclusiveMax) {
        auto bitWidth = static_cast<std::size_t>(std::bit_width(inclusiveMax));
        return std::views::iota(1)
            | std::views::take_while([bitWidth](std::size_t const x) {
                return x < bitWidth;
            })
            | std::views::transform([](std::size_t const x) {
                return std::size_t{1} << x;
            });
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

private:
    template<class C, typename T, std::size_t... Is, typename F>
    static constexpr C impl(std::index_sequence<Is...> /*is*/, F&& generator) {
        return {
            {
                static_cast<T>(std::invoke(std::forward<F>(generator), Is))... // NOLINT
            },
        };
    }
};

} // namespace Nebulite::Utility
#endif // NEBULITE_UTILITY_GENERATE_HPP
