/**
 * @file StringHandler.hpp
 * @brief Utility functions for string manipulation and parsing.
 */

#pragma once

//------------------------------------------
// Includes

// General
#include <iostream>
#include <iomanip>
#include <sstream>
#include <locale>
#include <cstdint> // For fixed-width integer types
#include <vector>
#include <locale.h>
#include <codecvt>
#include <algorithm>
#include <unordered_map>

//------------------------------------------
namespace Nebulite {
namespace Utility {
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
    static bool containsAnyOf(const std::string& str, const std::string& chars);

    /**
     * @brief Checks if a string represents a valid number.
     * 
     * @param str The input string to check.
     * @return true if the string is a valid number, false otherwise.
     */
    static bool isNumber(std::string str);

    /**
     * @brief Replaces all occurrences of a substring within a string.
     * 
     * @param target The original string.
     * @param toReplace The substring to replace.
     * @param replacer The replacement substring.
     * @return The modified string with all occurrences replaced.
     */
    static std::string replaceAll(std::string target, const std::string& toReplace, const std::string& replacer);

    /**
     * @brief Extracts the substring from the start of the input until a special character is found.
     * 
     * @param input The original string.
     * @param specialChar The special character to look for.
     * @return The substring from the start of the input until the special character is found,
     * but not including the special character itself.
     */
    static std::string untilSpecialChar(std::string input, char specialChar);

    /**
     * @brief Extracts the substring from the input after a special character is found.
     * 
     * @param input The original string.
     * @param specialChar The special character to look for.
     * @return The substring from the input after the special character is found.
     */
    static std::string afterSpecialChar(std::string input, char specialChar);

    /**
     * @brief Left strips a string of a special character.
     * 
     * @param input The original string.
     * @param specialChar The special character to remove.
     * @return The left-stripped string.
     */
    static std::string lstrip(const std::string& input, char specialChar = ' ');

    /**
     * @brief Right strips a string of a special character.
     * 
     * @param input The original string.
     * @param specialChar The special character to remove.
     * @return The right-stripped string.
     */
    static std::string rstrip(const std::string& input, char specialChar = ' ');

    /**
     * @brief Splits a string into tokens based on a delimiter.
     * 
     * @param input The original string.
     * @param delimiter The character to split the string on.
     * @param keepDelimiter Whether to keep the delimiter in the tokens. 
     * Keeps the delimiter at the start of the token. (Optional, default: false)
     * @return A vector of tokens extracted from the input string.
     */
    static std::vector<std::string> split(const std::string& input, char delimiter, bool keepDelimiter = false);

    /**
     * @brief Splits a string on the same depth of parentheses.
     * 
     * Example:
     * 
     * `"This is a text {with} {some}{!} nested {{paranthesis}}"`
     * 
     * -> `["This is a text ", "{with}", " ", "{some}", "{!}", " nested ", "{{paranthesis}}"]`
     * 
     * @param input The original string.
     * @param delimiter The opening parenthesis to split on.
     * @return A vector of strings split on the same depth of parentheses.
     */
    static std::vector<std::string> splitOnSameDepth(const std::string& input, char delimiter);
};
}   // namespace Utility
}   // namespace Nebulite
