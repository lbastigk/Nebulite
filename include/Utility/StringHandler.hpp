/**
 * @file StringHandler.hpp
 * @brief Utility functions for string manipulation and parsing.
 */

#ifndef UTILITY_STRINGHANDLER_HPP
#define UTILITY_STRINGHANDLER_HPP

//------------------------------------------
// Includes

// Standard library
#include <string>
#include <vector>

//------------------------------------------
namespace Nebulite::Utility {
/**
 * @class Nebulite::Utility::StringHandler
 * @brief Utility functions for string manipulation and parsing.
 */
class StringHandler {
public:
    // [GENERATE]

    /**
     * @brief Repeats a given string a specified number of times.
     * @param str The string to repeat.
     * @param count The number of times to repeat the string.
     * @return The resulting string after repetition.
     */
    static std::string repeat(std::string_view const& str, size_t const& count);

    /**
     * @brief Pads a list of words with spaces to create a table-like view.
     * @details The padding is based on the longest word.
     * @param words The list of words to pad
     * @param rowSize The maximum character size per row. Set to zero for no newlines
     */
    static std::string createPaddedTable(std::vector<std::string> const& words, size_t const& rowSize = 0);

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
     * @param cmd The command string to parse.
     */
    static ParseResult parseQuotedArguments(std::string_view const& cmd);

    /**
     * @brief Recombines a span of strings into a single string with spaces.
     *        Ensures that no extra spaces are added if the original arguments contained spaces.
     * @param args The span of argument strings.
     * @return The recombined argument string.
     */
    static std::string recombineArgs(std::span<std::string const> const& args);

    // [SPLIT]

    // TODO: modify split functions to return a vector of views
    //       -> re-evaluate the usage inside nebulite first

    /**
     * @brief Splits a string into tokens based on a delimiter.
     * @param input The original string.
     * @param delimiter The character to split the string on.
     * @param keepDelimiter Whether to keep the delimiter in the tokens. (Optional, default: false)
     * @return A vector of tokens extracted from the input string.
     */
    static std::vector<std::string> split(std::string_view const& input, char const& delimiter, bool const& keepDelimiter = false);

    /**
     * @brief Splits a string at a given delimiter, only if the depth of braces, parentheses and brackets is 0.
     * @param input The original string.
     * @param delimiter The delimiter to split on
     * @return A vector of strings split on the same depth of parentheses.
     */
    static std::vector<std::string> splitOnSameDepth(std::string_view const& input, char const& delimiter);

    /**
     * @brief Type of delimiter:
     * @details - parentheses: '(' and ')'
     *          - brace: '{' and '}'
     *          - bracket: '[' and ']'
     */
    enum class Delimiter : uint8_t {parentheses, brace, bracket};

    /**
     * @brief Converts a Delimiter enum value to its corresponding opening character.
     * @param delimiter The Delimiter enum value to convert.
     * @return The corresponding opening character for the given Delimiter.
     */
    static char delimiterToOpeningChar(Delimiter const& delimiter);

    /**
     * @brief Converts a Delimiter enum value to its corresponding closing character.
     * @param delimiter The Delimiter enum value to convert.
     * @return The corresponding closing character for the given Delimiter.
     */
    static char delimiterToClosingChar(Delimiter const& delimiter);

    /**
     * @brief Splits a string at a given delimiter, not respecting the depth of any other characters.
     * @param input The input string to split
     * @param delimiter The delimiter to split on
     * @return A vector of strings split on the same depth of the given delimiter
     */
    static std::vector<std::string> splitOnSameDepthOf(std::string_view const& input, Delimiter const& delimiter);
};
}   // namespace Nebulite::Utility
#endif // UTILITY_STRINGHANDLER_HPP
