#ifndef NEBULITE_UTILITY_ARGS_SEGMENTEDSTRINGVIEW_HPP
#define NEBULITE_UTILITY_ARGS_SEGMENTEDSTRINGVIEW_HPP

//------------------------------------------
// Includes

// Standard library
#include <cstddef>
#include <functional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

// Nebulite
#include "Nebulite/Utility/Args/StringIterators.hpp"
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
    explicit CharacterCount(std::span<std::string_view const> strings);

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

public:
    SegmentedStringView();

    // TODO: remove, only construct from string_view via algorithm defined in StringHandler::parseQuotedArguments
    //       but then we would need an external allocator like a vector of string_views!
    //       Inside FuncTree::parse, we could create the allocator and then pass the SegmentedStringView by const reference.
    explicit SegmentedStringView(std::span<std::string_view const> args);

    //------------------------------------------
    // Operators

    bool operator==(SegmentedStringView const& other) const;
    bool operator!=(SegmentedStringView const& other) const;

    bool operator==(std::string_view other) const; // Passing a string_view with quotes will return false. Consider turning into a SegmentedStringView first!
    bool operator!=(std::string_view other) const; // Passing a string_view with quotes will return false. Consider turning into a SegmentedStringView first!

    [[nodiscard]] decltype(data[0])& operator[](std::size_t index) const ;

    //------------------------------------------
    // Range

    [[nodiscard]] decltype(data.begin()) begin() const ;

    [[nodiscard]] decltype(data.end()) end() const ;

    //------------------------------------------
    // Size

    [[nodiscard]] bool empty() const ;

    [[nodiscard]] std::size_t segmentCount() const ;
    [[nodiscard]] std::size_t characterCount() const ;

    //------------------------------------------
    // Span

    [[nodiscard]] SegmentedStringView subspan(std::size_t index) const ;
    [[nodiscard]] SegmentedStringView subspan(std::size_t startIndex, std::size_t count) const ;

    /**
     * @brief Finds a given named argument and returns it.
     * @details If the name is provided multiple times, the first given value is returned.
     * @param name The name of the argument. Must start with "--"
     * @return The argument. If the given argument name is not in the list,
     *         an empty string_view is returned.
     */
    std::string_view getNamedArgument(std::string_view name) const ;

    /**
     * @brief Returns a subspan of arguments starting from a named argument.
     * @details The subspan goes until the next '--' or until the end.
     * @param name The name of the argument. Must start with "--"
     * @return The subspan of arguments. If the given argument name is not in the list,
     *         an empty SegmentedStringView is returned.
     */
    SegmentedStringView getNamedSpannedArgument(std::string_view name) const ;

    //------------------------------------------
    // Substring

    std::string substring(std::size_t startIndex, std::size_t count) const ;

    //------------------------------------------
    // Copy

    void copy(std::vector<std::string_view>& other) const ;
    void copySubspan(std::vector<std::string_view>& other, std::size_t index) const ;
    void copySubspan(std::vector<std::string_view>& other, std::size_t startIndex, std::size_t count) const ;

    //------------------------------------------
    // Generate

    [[nodiscard]] std::string recombine() const ;

    //------------------------------------------
    // Compare

    bool beginsWith(std::string_view other) const;
    bool beginsWith(SegmentedStringView const& other) const;

    bool endsWith(std::string_view other) const;
    bool endsWith(SegmentedStringView const& other) const;

    bool contains(std::string_view other) const;
    bool contains(SegmentedStringView const& other) const;

    //------------------------------------------
    // Iterate

    template<typename F>
    void forEachCharacter(F&& f) const {
        static_assert(std::is_invocable_v<F, char>);
        auto it = SpanIterator{data};
        while (!it.endReached()) {
            std::invoke(std::forward<F>(f)(it.get()));
            ++it;
        }
    }
};

} // namespace Nebulite::Utility::Args
#endif // NEBULITE_UTILITY_ARGS_SEGMENTEDSTRINGVIEW_HPP
