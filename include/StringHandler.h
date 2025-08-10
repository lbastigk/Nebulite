#pragma once

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

typedef uint64_t UINT64;

namespace Nebulite{
class StringHandler {
public:
    static bool containsAnyOf(const std::string& str, const std::string& chars);

    static bool isNumber(std::string str);

    static std::string uint64ToStringWithPadding(UINT64 value, int length);

    static std::string uint64ToStringWithGroupingAndPadding(UINT64 value, int length);

    static std::string replaceAll(std::string target, const std::string& toReplace, const std::string& replacer);

    //use %i in before/after for line count
    static std::string parseArray(std::vector<std::string> arr, std::string before = "", std::string after = "\n");

    static std::wstring stringToWstring(const std::string& str);

    static std::string wstringToString(const std::wstring& wstr);

    static std::string getBinaryString(int toConvert);

    static std::string untilSpecialChar(std::string input, char specialChar);

    static std::string afterSpecialChar(std::string input, char specialChar);

    static std::string lstrip(const std::string& input, char specialChar = ' ');

    static std::string rstrip(const std::string& input, char specialChar = ' ');

    static std::vector<std::string> split(const std::string& input, char delimiter, bool keepDelimiter = false);

    // Splits a string on the same depth of parentheses
    // - for '()', '[]', '{}' according to delimiter
    static std::vector<std::string> splitOnSameDepth(const std::string& input, char delimiter);
};
}
