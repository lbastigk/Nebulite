/**
 * @file StringHandler.hpp
 * @brief Utility functions for string manipulation and parsing.
 */

#ifndef UTILITY_STRINGHANDLER_HPP
#define UTILITY_STRINGHANDLER_HPP

//------------------------------------------
// Includes

// Standard library
#include <cstddef> // NOLINT
#include <string>
#include <string_view>
#include <vector>

//------------------------------------------
namespace Nebulite::Utility {
/**
 * @class Nebulite::Utility::StringHandler
 * @brief Utility functions for string manipulation and parsing.
 */
class StringHandler {
public:
    // [COMPARE]

    /**
     * @brief Checks if a given string_view str starts with a given sequence of string_views.
     * @tparam Args The string_views to consider as the start sequence. Any number of arguments is supported.
     * @param str The string to compare to
     * @param args The start sequence
     * @return True if the string starts with the given sequence, false otherwise
     */
    template <typename... Args>
    static bool startsWithSequence(std::string_view const& str, Args... args) {
        static_assert(sizeof...(Args) > 0, "At least one sequence argument is required for startsWithSequence");
        auto impl = [&]<typename First, typename... Rest>(auto&& self, std::string_view remaining, First first, Rest... rest) -> bool {
            if (!remaining.starts_with(first)) {
                return false;
            }

            if constexpr (sizeof...(Rest) == 0) {
                return true;
            } else {
                return self(self, remaining.substr(first.size()), rest...);
            }
        };
        return impl(impl, str, args...);
    }

    // [GENERATE]

    /**
     * @brief Pads a list of words with spaces to create a table-like view.
     * @details The padding is based on the longest word.
     * @param words The list of words to pad
     * @param rowSize The maximum character size per row. Set to zero for no newlines
     */
    static std::string createPaddedTable(std::vector<std::string> const& words, std::size_t rowSize = 0);

    /**
     * @brief Replaces all occurrences of a substring within a string.
     * @param target The original string.
     * @param toReplace The substring to replace.
     * @param replacer The replacement substring.
     * @return The modified string with all occurrences replaced.
     */
    static std::string replaceAll(std::string target, std::string_view const& toReplace, std::string_view const& replacer);

    // [VALIDATE]

    /**
     * @brief Checks if a string contains any characters from a given set.
     * @param str The input string to check.
     * @param chars The set of characters to look for.
     * @return true if any character from chars is found in str, false otherwise.
     */
    static bool containsAnyOf(std::string_view const& str, std::string_view const& chars);

    /**
     * @brief Checks if a string represents a valid number.
     * @param str The input string to check.
     * @return true if the string is a valid number, false otherwise.
     */
    static bool isNumber(std::string_view const& str);

    // [STRIP]

    /**
     * @brief Extracts the substring from the start of the input until a special character is found.
     * @param str The string to modify
     * @param specialChar The special character to look for.
     */
    static void untilSpecialChar(std::string_view& str, char const& specialChar);

    /**
     * @brief Extracts the substring from the input after a special character is found.
     * @param str The string to modify
     * @param specialChar The special character to look for.
     */
    static void afterSpecialChar(std::string_view& str, char const& specialChar);

    /**
     * @brief Left strips a string of a special character.
     * @param str The string to modify
     * @param specialChar The special character to remove.
     */
    static void lStrip(std::string_view& str, char const& specialChar = ' ');

    /**
     * @brief Right strips a string of a special character.
     * @param str The string to modify
     * @param specialChar The special character to remove.
     */
    static void rStrip(std::string_view& str, char const& specialChar = ' ');

    /**
     * @brief Left and right strips a string of a special character.
     * @param str The string to modify
     * @param specialChar The special character to remove.
     */
    static void strip(std::string_view& str, char const& specialChar = ' ');

    // [ARGS]

    struct ParseResult {
        std::vector<std::string> args;
        bool unclosedQuote = false;
    };

    /**
     * @brief Parses a command string into individual arguments, taking quotes into account.
     * @details Due to quote handling, ParseResult::args is a vector of strings, not a vector of string_views.
     * @param cmd The command string to parse.
     * @todo Returning a vector of string_views should be possible. Large refactor might be necessary
     */
    static ParseResult parseQuotedArguments(std::string_view const& cmd);

    /**
     * @brief Recombines a span of string_views into a single string with spaces.
     *        Ensures that no extra spaces are added if the original arguments contained spaces.
     * @param args The span of argument strings.
     * @return The recombined argument string.
     */
    static std::string recombineArgs(std::span<std::string_view const> const& args);

    // [SPLIT]

    /**
     * @brief Splits a string into tokens based on a delimiter.
     * @details The delimiter is kept at the front of the next token
     * @param input The original string.
     * @param delimiter The character to split the string on.
     * @param keepDelimiter Whether to keep the delimiter in the tokens. (Optional, default: false)
     * @return A vector of tokens extracted from the input string.
     */
    static std::vector<std::string_view> split(std::string_view const& input, char delimiter, bool keepDelimiter = false);

    /**
     * @brief Splits a string at a given delimiter, only if the depth of braces, parentheses and brackets is 0.
     * @param input The original string.
     * @param delimiter The delimiter to split on
     * @return A vector of strings split on the same depth of parentheses.
     */
    static std::vector<std::string_view> splitOnSameDepth(std::string_view const& input, char delimiter);

    /**
     * @brief Type of delimiter:
     * @details - parentheses: '(' and ')'
     *          - brace: '{' and '}'
     *          - bracket: '[' and ']'
     */
    enum class Delimiter : std::uint8_t {
        parentheses = 0,
        brace,
        bracket
    };

    /**
     * @brief Converts a Delimiter enum value to its corresponding opening character.
     * @param delimiter The Delimiter enum value to convert.
     * @return The corresponding opening character for the given Delimiter.
     */
    static char delimiterToOpeningChar(Delimiter delimiter);

    /**
     * @brief Converts a Delimiter enum value to its corresponding closing character.
     * @param delimiter The Delimiter enum value to convert.
     * @return The corresponding closing character for the given Delimiter.
     */
    static char delimiterToClosingChar(Delimiter delimiter);

    /**
     * @brief Splits a string at a given delimiter, not respecting the depth of any other characters.
     * @param input The input string to split
     * @param delimiter The delimiter to split on
     * @return A vector of strings split on the same depth of the given delimiter
     */
    static std::vector<std::string_view> splitOnSameDepthOf(std::string_view const& input, Delimiter delimiter);
};
}   // namespace Nebulite::Utility
#endif // UTILITY_STRINGHANDLER_HPP
