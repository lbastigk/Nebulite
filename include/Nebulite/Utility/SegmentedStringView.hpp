#ifndef NEBULITE_UTILITY_SEGMENTEDSTRINGVIEW_HPP
#define NEBULITE_UTILITY_SEGMENTEDSTRINGVIEW_HPP

//------------------------------------------
// Includes

// Standard library
#include <cstddef>
#include <span>
#include <string>
#include <string_view>

//------------------------------------------

namespace Nebulite::Utility {
/**
 * @class SegmentedStringView
 * @brief Provides basic string functionality for a split string
 * @todo Modify Equality checks to assume a whitespace between each data point
 * @todo Using this class in any FuncTree related parsing could be more powerful, as we avoid recombining
 *       for simple string comparison checks.
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

    [[nodiscard]] std::string recombine() const ;
};

} // namespace Nebulite::Utility
#endif // NEBULITE_UTILITY_SEGMENTEDSTRINGVIEW_HPP
