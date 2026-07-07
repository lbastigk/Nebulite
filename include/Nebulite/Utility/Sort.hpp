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
    struct CaseSensitiveLess {
        template <class L, class R>
        requires requires (const L& l, const R& r){
            std::string_view{l};
            std::string_view{r};
        }

        constexpr bool operator()(const L& lhs, const R& rhs) const {
            const std::string_view a{lhs};
            const std::string_view b{rhs};

            return std::ranges::lexicographical_compare(a, b, [](unsigned char const& x, unsigned char const& y) {
                    return x < y;
                }
            );
        }
    };
    static CaseSensitiveLess caseSensitiveLess;

    struct CaseInsensitiveLess {
        template <class L, class R>
        requires requires (const L& l, const R& r) {
            std::string_view{l};
            std::string_view{r};
        }

        constexpr bool operator()(const L& lhs, const R& rhs) const {
            const std::string_view a{lhs};
            const std::string_view b{rhs};

            return std::ranges::lexicographical_compare(
                a, b,
                [](unsigned char const& x, unsigned char const& y) {
                    return std::tolower(x) < std::tolower(y);
                }
            );
        }
    };
    static CaseInsensitiveLess caseInsensitiveLess;
};

} // namespace Nebulite::Utility
#endif // NEBULITE_UTILITY_SORT_HPP
