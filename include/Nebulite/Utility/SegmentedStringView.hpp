#ifndef NEBULITE_UTILITY_SEGMENTEDSTRINGVIEW_HPP
#define NEBULITE_UTILITY_SEGMENTEDSTRINGVIEW_HPP

//------------------------------------------
// Includes

// Standard library
#include <cstddef>
#include <span>
#include <string_view>

//------------------------------------------

namespace Nebulite::Utility {
/**
 * @class SegmentedStringView
 * @brief Provides basic string functionality for a split string
 * @todo Requires policy setting if between each point a whitespace is assumed
 *       This is important for recombination and comparison!
 *       e.g. ["Hello" "world"] might mean "Helloword" or "Hello world", depending on the policy setting
 *       Perhaps it's best to always assume a single whitespace inbetween! Check if arg split works that way...
 */
class SegmentedStringView {
    std::span<std::string_view const> const data;

    std::size_t const charCount; // Pre-computed, as it's used for many comparisons

public:
    explicit SegmentedStringView(std::span<std::string_view const> sv);

    bool operator==(SegmentedStringView const& other) const;
    bool operator!=(SegmentedStringView const& other) const;

    bool operator==(std::string_view other) const;
    bool operator!=(std::string_view other) const;

    [[nodiscard]] std::size_t segmentCount() const;
    [[nodiscard]] std::size_t charactorCount() const;

    [[nodiscard]] SegmentedStringView subspan(std::size_t index) const ;
    [[nodiscard]] SegmentedStringView subspan(std::size_t startIndex, std::size_t count) const ;

    [[nodiscard]] auto begin() const {
        return data.begin();
    }

    [[nodiscard]] auto end() const {
        return data.end();
    }

    // TODO: beginsWith, endsWith, contains
};

} // namespace Nebulite::Utility
#endif // NEBULITE_UTILITY_SEGMENTEDSTRINGVIEW_HPP
