#ifndef NEBULITE_UTILITY_SEGMENTEDSTRINGVIEW_HPP
#define NEBULITE_UTILITY_SEGMENTEDSTRINGVIEW_HPP

//------------------------------------------
// Includes

// Standard library
#include <cstddef>
#include <span>
#include <string>
#include <string_view>
#include <vector>

//------------------------------------------
namespace Nebulite::Utility {
/**
 * @class SegmentedStringView
 * @brief Provides basic string functionality for a split string
 * @details The comparison functions offer a more complex functionality, as they ignore
 *          any quotes marks and just compare the content;
 *          `My name is "Bjarne Stroustrup"` is equal to `My name is Bjarne Stroustrup`
 * @todo Using this class in any FuncTree related parsing could be more powerful, as we avoid recombining
 *       for simple string comparison checks.
 */
class SegmentedStringView {
    std::span<std::string_view const> const data;

    std::size_t const charCount; // Pre-computed, as it's used for many comparisons

public:
    // TODO: remove, only construct from string_view via algorithm defined in StringHandler::parseQuotedArguments
    explicit SegmentedStringView(std::span<std::string_view const> sv);

    bool operator==(SegmentedStringView const& other) const;
    bool operator!=(SegmentedStringView const& other) const;

    bool operator==(std::string_view other) const; // Passing a string_view with quotes will almost certainly return false. Consider turning into a SegmentedStringView first!
    bool operator!=(std::string_view other) const; // Passing a string_view with quotes will almost certainly return false. Consider turning into a SegmentedStringView first!

    [[nodiscard]] std::size_t segmentCount() const;
    [[nodiscard]] std::size_t charactorCount() const;

    [[nodiscard]] SegmentedStringView subspan(std::size_t index) const ;
    [[nodiscard]] SegmentedStringView subspan(std::size_t startIndex, std::size_t count) const ;

    void appendSubspan(std::vector<std::string_view>& other, std::size_t index) const ;
    void appendSubspan(std::vector<std::string_view>& other, std::size_t startIndex, std::size_t count) const ;

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
