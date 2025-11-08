/**
 * @file StringHandler.hpp
 * @brief Utility functions for string manipulation and parsing.
 */

#ifndef NEBULITE_UTILITY_STRINGHANDLER_HPP
#define NEBULITE_UTILITY_STRINGHANDLER_HPP

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
    /**
     * @brief Checks if a string contains any characters from a given set.
     * 
     * @param str The input string to check.
     * @param chars The set of characters to look for.
     * @return true if any character from chars is found in str, false otherwise.
     */
    static bool containsAnyOf(std::string const& str, std::string const& chars);

    /**
     * @brief Checks if a string represents a valid number.
     * 
     * @param str The input string to check.
     * @return true if the string is a valid number, false otherwise.
     */
    static bool isNumber(std::string const& str);

    /**
     * @brief Replaces all occurrences of a substring within a string.
     * 
     * @param target The original string.
     * @param toReplace The substring to replace.
     * @param replacer The replacement substring.
     * @return The modified string with all occurrences replaced.
     */
    static std::string replaceAll(std::string target, std::string const& toReplace, std::string const& replacer);

    /**
     * @brief Extracts the substring from the start of the input until a special character is found.
     * 
     * @param input The original string.
     * @param specialChar The special character to look for.
     * @return The substring from the start of the input until the special character is found,
     * but not including the special character itself.
     */
    static std::string untilSpecialChar(std::string const& input, char const& specialChar);

    /**
     * @brief Extracts the substring from the input after a special character is found.
     * 
     * @param input The original string.
     * @param specialChar The special character to look for.
     * @return The substring from the input after the special character is found.
     */
    static std::string afterSpecialChar(std::string const& input, char const& specialChar);

    /**
     * @brief Left strips a string of a special character.
     * 
     * @param input The original string.
     * @param specialChar The special character to remove.
     * @return The left-stripped string.
     */
    static std::string lStrip(std::string const& input, char const& specialChar = ' ');

    /**
     * @brief Right strips a string of a special character.
     * 
     * @param input The original string.
     * @param specialChar The special character to remove.
     * @return The right-stripped string.
     */
    static std::string rStrip(std::string const& input, char const& specialChar = ' ');

    /**
     * @brief Splits a string into tokens based on a delimiter.
     * 
     * @param input The original string.
     * @param delimiter The character to split the string on.
     * @param keepDelimiter Whether to keep the delimiter in the tokens. 
     * Keeps the delimiter at the start of the token. (Optional, default: false)
     * @return A vector of tokens extracted from the input string.
     */
    static std::vector<std::string> split(std::string const& input, char const& delimiter, bool const& keepDelimiter = false);

    /**
     * @brief Splits a string on the same depth of parentheses.
     * 
     * Example:
     * 
     * `"This is a text {with} {some}{!} nested {{parenthesis}}"`
     * 
     * -> `["This is a text ", "{with}", " ", "{some}", "{!}", " nested ", "{{parenthesis}}"]`
     * 
     * @param input The original string.
     * @param delimiter The opening parenthesis to split on.
     * @return A vector of strings split on the same depth of parentheses.
     */
    static std::vector<std::string> splitOnSameDepth(std::string const& input, char const& delimiter);

    /**
     * @brief Parses a command string into individual arguments, taking quotes into account.
     * 
     * @param cmd The command string to parse.
     */
    static std::vector<std::string> parseQuotedArguments(std::string const& cmd);

    /**
     * @brief Recombines argc/argv into a single string with spaces.
     * 
     * Ensures that no extra spaces are added if the original arguments contained spaces.
     */
    static std::string recombineArgs(int argc, char* argv[]);
};
}   // namespace Nebulite::Utility
#endif // NEBULITE_UTILITY_STRINGHANDLER_HPP
