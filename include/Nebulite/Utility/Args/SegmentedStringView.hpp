#ifndef NEBULITE_UTILITY_ARGS_SEGMENTEDSTRINGVIEW_HPP
#define NEBULITE_UTILITY_ARGS_SEGMENTEDSTRINGVIEW_HPP

//------------------------------------------
// Includes

// Standard library
#include <cstddef>
#include <span>
#include <string>
#include <string_view>
#include <vector>

// Nebulite
#include "Nebulite/Utility/Coordination/LazyInit.hpp"

//------------------------------------------
namespace Nebulite::Utility::Args {
/**
 * @brief Helper wrapper for LazyInit of CharacterCount
 * @details Instead of expanding LazyInit with get()-overloads that accept lambdas for custom initialization,
 *          We just use a wrapper type with constructors.
 */
struct CharacterCount {
    std::size_t count;

    /**
     * @brief Gets size from the given span
     * @param strings The span of string views
     */
    CharacterCount(std::span<std::string_view const> strings);

    /**
     * @brief Forces a character count
     * @param c The count to set
     */
    explicit CharacterCount(std::size_t c);
};

/**
 * @class SegmentedStringView
 * @brief Provides basic string functionality for a split string
 * @details The comparison functions offer a more complex functionality, as they ignore
 *          any quotes marks and just compare the content;
 *          `My name is "Bjarne Stroustrup"` is equal to `My name is Bjarne Stroustrup`
 * @todo Using this class in any FuncTree related parsing could be more powerful, as we avoid recombining
 *       for simple string comparison checks.
 * @todo Add tests in FeatureTest module
 */
class SegmentedStringView {
    std::span<std::string_view const> const data;

    mutable Coordination::LazyInitOptional<CharacterCount, std::span<std::string_view const> const> charCount;

    SegmentedStringView(std::span<std::string_view const> args, std::size_t characterCount);

public:
    // TODO: remove, only construct from string_view via algorithm defined in StringHandler::parseQuotedArguments
    //       but then we would need an external allocator like a vector of string_views!
    //       Inside FuncTree::parse, we could create the allocator and then pass the SegmentedStringView by const reference.
    explicit SegmentedStringView(std::span<std::string_view const> args);

    bool operator==(SegmentedStringView const& other) const;
    bool operator!=(SegmentedStringView const& other) const;

    bool operator==(std::string_view other) const; // Passing a string_view with quotes will return false. Consider turning into a SegmentedStringView first!
    bool operator!=(std::string_view other) const; // Passing a string_view with quotes will return false. Consider turning into a SegmentedStringView first!

    [[nodiscard]] auto begin() const {
        return data.begin();
    }

    [[nodiscard]] auto end() const {
        return data.end();
    }

    [[nodiscard]] auto& operator[](std::size_t const index) const {
        return data[index];
    }

    [[nodiscard]] std::size_t segmentCount() const;
    [[nodiscard]] std::size_t characterCount() const;

    [[nodiscard]] SegmentedStringView subspan(std::size_t index) const ;
    [[nodiscard]] SegmentedStringView subspan(std::size_t startIndex, std::size_t count) const ;

    void appendSubspan(std::vector<std::string_view>& other, std::size_t index) const ;
    void appendSubspan(std::vector<std::string_view>& other, std::size_t startIndex, std::size_t count) const ;

    // TODO: beginsWith, endsWith, contains

    [[nodiscard]] std::string recombine() const ;
};

} // namespace Nebulite::Utility::Args
#endif // NEBULITE_UTILITY_ARGS_SEGMENTEDSTRINGVIEW_HPP
