#ifndef NEBULITE_UTILITY_ARGS_SEGMENTEDSTRINGVIEW_HPP
#define NEBULITE_UTILITY_ARGS_SEGMENTEDSTRINGVIEW_HPP

//------------------------------------------
// Includes

// Standard library
#include <cstddef>
#include <functional>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

// Nebulite
#include "Nebulite/Data/OptionalFixedString.hpp"
#include "Nebulite/Utility/Args/StringIterators.hpp"
#include "Nebulite/Utility/Convert/Cast.hpp"
#include "Nebulite/Utility/Coordination/LazyInit.hpp"

//------------------------------------------
namespace Nebulite::Utility::Args {
/**
 * @brief The underlying type of the Nebulite Argument abstraction.
 */
using FoundationalType = std::span<std::string_view const>;

/**
 * @brief Helper wrapper for LazyInit of CharacterCount
 * @details Instead of expanding LazyInit with get()-overloads that accept lambdas for custom initialization,
 *          We just use a wrapper type with constructors.
 */
struct CharacterCount {
    std::size_t count;

    /**
     * @brief Gets size from the given foundational arguments
     * @param args The arguments
     */
    explicit CharacterCount(FoundationalType args);

    /**
     * @brief Forces a character count
     * @param c The count to set
     */
    explicit CharacterCount(std::size_t c);
};

class ArgsTransitionCompatibilityLayer;

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
    FoundationalType data;

    mutable Coordination::LazyInitOptional<CharacterCount, FoundationalType> charCount;

    friend class ArgsTransitionCompatibilityLayer;

    /**
     * @brief Finds a given named argument and returns it.
     * @details If the name is provided multiple times, the first given value is returned.
     * @param name The name of the argument. Must start with "--"
     * @return The argument. If the given argument name is not in the list,
     *         an empty string_view is returned.
     */
    std::string_view getNamedArgumentImpl(std::string_view name) const ;

    /**
     * @brief Returns a subspan of arguments starting from a named argument.
     * @details The subspan goes until the next '--' or until the end.
     * @param name The name of the argument. Must start with "--"
     * @return The subspan of arguments. If the given argument name is not in the list,
     *         an empty SegmentedStringView is returned.
     */
    SegmentedStringView getNamedSpannedArgumentImpl(std::string_view name) const ;
public:
    SegmentedStringView();

    // TODO: remove, only construct from string_view via algorithm defined in StringHandler::parseQuotedArguments
    //       but then we would need an external allocator like a vector of string_views!
    //       Inside FuncTree::parse, we could create the allocator and then pass the SegmentedStringView by const reference.
    explicit SegmentedStringView(FoundationalType args);

    template <typename Data, typename Size>
    explicit SegmentedStringView(Data d, Size s) : data(FoundationalType{d,s}){}

    explicit SegmentedStringView(std::vector<std::string_view> const& argsData) : data(argsData.data(), argsData.size()) {}

    ~SegmentedStringView() = default;

    SegmentedStringView(SegmentedStringView const& other) = default;
    SegmentedStringView(SegmentedStringView&& other) = default;
    SegmentedStringView& operator=(SegmentedStringView const& other) = default;
    SegmentedStringView& operator=(SegmentedStringView&& other) = default;

    //------------------------------------------
    // Operators

    bool operator==(SegmentedStringView const& other) const;
    bool operator!=(SegmentedStringView const& other) const;

    bool operator==(std::string_view other) const; // Passing a string_view with quotes will return false. Consider turning into a SegmentedStringView first!
    bool operator!=(std::string_view other) const; // Passing a string_view with quotes will return false. Consider turning into a SegmentedStringView first!

    [[nodiscard]] std::string_view operator[](std::size_t index) const ;

    //------------------------------------------
    // Range

    [[nodiscard]] decltype(data.at(0)) at(std::size_t index) const ;

    [[nodiscard]] decltype(data.begin()) begin() const ;

    [[nodiscard]] decltype(data.end()) end() const ;

    [[nodiscard]] decltype(data.front()) front() const ;

    [[nodiscard]] decltype(data.back()) back() const ;

    //------------------------------------------
    // Size

    [[nodiscard]] bool empty() const ;

    [[nodiscard]] std::size_t size() const ;

    [[nodiscard]] std::size_t characterCount() const ;

    //------------------------------------------
    // Span

    [[nodiscard]] SegmentedStringView subspan(std::size_t index) const ;
    [[nodiscard]] SegmentedStringView subspan(std::size_t startIndex, std::size_t count) const ;

    //------------------------------------------
    // Argument handling

    template<typename T>
    std::optional<T> tryGetAs(std::size_t const index) const {
        if (index >= size()) {
            return std::nullopt;
        }
        return Convert::Cast::String::to<T>(data[index]);
    }

    /**
     * @brief Finds a given named argument and returns it.
     * @details If the name is provided multiple times, the first given value is returned.
     * @todo Return last given value instead
     * @tparam Name The name of the argument. Must start with "--"
     * @return The argument. If the given argument name is not in the list,
     *         an empty string_view is returned.
     */
    template<Data::OptionalFixedString Name = Data::FixedStringState::noFixedStringProvided>
    std::string_view getNamedArgument() const {
        static_assert(Name.hasValue(), "Please provide a name via template argument");
        static_assert(Name.startsWith("--"), "Name must start with \"--\"");
        return getNamedArgumentImpl(Name.view());
    }

    /**
     * @brief Finds a given named argument and returns it as a SegmentedStringView.
     * @details If the name is provided multiple times, the first given value is returned.
     * @todo Return last given value instead
     * @tparam Name The name of the argument. Must start with "--"
     * @return The argument. If the given argument name is not in the list,
     *         an empty SegmentedStringView is returned.
     */
    template<Data::OptionalFixedString Name = Data::FixedStringState::noFixedStringProvided>
    SegmentedStringView getNamedSpannedArgument() const {
        static_assert(Name.hasValue(), "Please provide a name via template argument");
        static_assert(Name.startsWith("--"), "Name must start with \"--\"");
        return getNamedSpannedArgumentImpl(Name.view());
    }

    //------------------------------------------
    // Substring

    std::string substring(std::size_t startIndex, std::size_t count) const ;

    //------------------------------------------
    // Copy

    void copy(std::vector<std::string_view>& other) const ;
    void copySubspan(std::vector<std::string_view>& other, std::size_t startIndex) const ;
    void copySubspan(std::vector<std::string_view>& other, std::size_t startIndex, std::size_t count) const ;

    //------------------------------------------
    // Generate

    [[nodiscard]] std::string recombine() const ;
    [[nodiscard]] std::string recombineSubspan(std::size_t startIndex) const ;
    [[nodiscard]] std::string recombineSubspan(std::size_t startIndex, std::size_t count) const ;

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
