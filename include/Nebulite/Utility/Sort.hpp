#ifndef NEBULITE_UTILITY_SORT_HPP
#define NEBULITE_UTILITY_SORT_HPP

//------------------------------------------
// Includes

// Standard library
#include <algorithm>
#include <cctype>
#include <string_view>

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
            const std::string_view a{lhs};
            const std::string_view b{rhs};

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
            const std::string_view a{lhs};
            const std::string_view b{rhs};

            return std::ranges::lexicographical_compare(
                a, b,
                [](unsigned char const x, unsigned char const y) {
                    return std::tolower(x) < std::tolower(y);
                }
            );
        }
    } caseInsensitiveLess{};
};

} // namespace Nebulite::Utility
#endif // NEBULITE_UTILITY_SORT_HPP
