#pragma once

#include <iostream>
#include <iomanip>
#include <sstream>
#include <locale>
#include <cstdint> // For fixed-width integer types
#include <vector>
#include <locale.h>
#include <codecvt>

typedef uint64_t UINT64;

class StringHandler {
public:
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
};