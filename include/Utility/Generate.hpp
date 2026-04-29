/**
 * @file Generate.hpp
 * @brief Provides functions generating data structures.
 */

#ifndef NEBULITE_UTILITY_INITIALIZER_HPP
#define NEBULITE_UTILITY_INITIALIZER_HPP

//------------------------------------------
// Includes

// Standard library
#include <array>
#include <functional>
#include <stdexcept>
#include <string>

// Nebulite
#include "Utility/CompileTimeEvaluate.hpp"

//------------------------------------------
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

    /**
     * @brief Generates a pairing table for N elements, where each element is paired with every other element exactly once across multiple rows
     * @details In each row, each element is paired once. Either with itself or with another element.
     *          In total, there are 2*N rows and N/2 pairs per row, with a total of N*N pairs across the entire table.
     * @tparam N The number of elements to pair. Must be of the form 4*n^k for some n >= 1 and k >= 0 to ensure proper pairing and round generation.
     * @return A 2D array representing the pairing table, where each row contains pairs of indices representing the paired elements for that round.
     */
    template <size_t N>
    static consteval auto pairingTable() {
        static_assert(N%4 == 0 && N >=4 && CompileTimeEvaluate::isPowerOfTwo(N/4), "N must be of type 4*n^k for some n >= 1 and k >= 0 to ensure proper pairing and round generation.");

        static auto constexpr roundCount = 2*N;
        static auto constexpr pairsPerRound = N/2;

        std::array<std::array<std::pair<size_t, size_t>, pairsPerRound>, roundCount> rounds{};
        std::array<size_t, roundCount> roundsAssigned{}; // Track how many pairs have been assigned to each round

        // TODO: should be reduce-able to a 2x2 mask!
        static std::array<std::array<size_t, 4>, 4> constexpr baseMask = {
            {
                {1, 2, 5, 6},
                {4, 3, 8, 7},
                {5, 6, 1, 2},
                {8, 7, 4, 3}
            }
        };

        for (size_t row = 0; row < N ; row++) {
            for (size_t col = 0; col < N; col++) {
                // For each pair (row, col), determine which round it belongs to using the base mask and the row/col indices
                size_t const localRound = baseMask[row % 4][col % 4] - 1; // Get the local round index from the base mask
                size_t const rowGroup = row / 4; // Determine the group of the row (0 to N/4 - 1)
                size_t const colGroup = col / 4; // Determine the group of the column
                size_t const totalRound = localRound + 8*(rowGroup ^ colGroup);
                rounds[totalRound][roundsAssigned[totalRound]] = {row, col};
                ++roundsAssigned[totalRound];
            }
        }

        // Additional checks, just to be sure the generated table is correct.
        for (size_t r = 0; r < roundCount; ++r) {
            // Each round must have exactly pairsPerRound pairs
            if (roundsAssigned[r] != pairsPerRound) {
                throw std::runtime_error("Invalid pairing: round does not contain the expected number of pairs!");
            }
        }
        for (size_t r = 0; r < roundCount; ++r) {
            for (size_t i = 0; i < pairsPerRound; ++i) {
                for (size_t j = i + 1; j < pairsPerRound; ++j) {
                    if (rounds[r][i] == rounds[r][j]) {
                        throw std::runtime_error("Duplicate pair detected in a round!");
                    }
                }
            }
        }

        return std::move(rounds);
    }

private:
    template<class C, typename T, std::size_t... Is, typename F>
    static constexpr C impl(std::index_sequence<Is...>, F&& generator) {
        return { { static_cast<T>(std::invoke(generator, Is))... } };
    }
};

} // namespace Nebulite::Utility
#endif // NEBULITE_UTILITY_INITIALIZER_HPP
