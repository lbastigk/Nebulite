#include "StringHandler.h"

std::string StringHandler::uint64ToStringWithPadding(UINT64 value, int length) {
    std::ostringstream oss;
    oss << std::setw(length) << std::setfill('0') << value;
    return oss.str();
}

std::string StringHandler::uint64ToStringWithGroupingAndPadding(UINT64 value, int length) {
    std::ostringstream oss;
    oss << std::setw(length) << std::setfill('0') << value;

    // Insert a space between every 3rd character from the end
    std::string result = oss.str();
    for (int i = result.length() - 3; i > 0; i -= 3) {
        result.insert(i, " ");
    }

    return result;
}

std::string StringHandler::replaceAll(std::string target, std::string toReplace, std::string replacer) {
    std::string::size_type pos = 0u;
    while ((pos = target.find(toReplace, pos)) != std::string::npos) {
        target.replace(pos, toReplace.length(), replacer);
        pos += toReplace.length();
    }
    return target;
}

//use %i in before/after for line count
std::string StringHandler::parseArray(std::vector<std::string> arr, std::string before, std::string after) {
    std::stringstream ss;
    int i = 1;
    for (const auto& line : arr) {
        //replace
        std::string bef = replaceAll(before,"%i",std::to_string(i));
        std::string aft = replaceAll(after, "%i", std::to_string(i));

        //add text
        ss << bef << line << aft;

        i++;
    }
    //delete extra newline
    ss.seekp(-1, std::ios_base::end);
    return ss.str();
}

std::wstring StringHandler::stringToWstring(const std::string& str) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.from_bytes(str);
}

std::string StringHandler::wstringToString(const std::wstring& wstr) {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.to_bytes(wstr);
}

std::string StringHandler::getBinaryString(int toConvert){
    std::string out = "0b";
    for (int i = 0; i < sizeof(int)*8; i++){
        if(i%8 == 0 && i != 0){
            out += " ";
        }
        out += std::to_string((toConvert >> i) && 1);
    }
    return out;
}

// Returns string start up until a certain char appears
std::string StringHandler::untilSpecialChar(std::string input, char specialChar){
    size_t pos = input.find(specialChar);
    if (pos != std::string::npos && pos + 1 < input.size()) {
        return input.substr(0,pos);
    }
    return input;
}

std::string StringHandler::afterSpecialChar(std::string input, char specialChar){
    size_t pos = input.find(specialChar);
    if (pos != std::string::npos && pos + 1 < input.size()) {
        return input.substr(pos+1);
    }
    return input;
}

std::string StringHandler::lstrip(const std::string& input, char specialChar) {
    size_t start = input.find_first_not_of(specialChar);
    return (start == std::string::npos) ? "" : input.substr(start);
}