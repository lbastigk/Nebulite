#include <iostream>
#include <iomanip>
#include <sstream>
#include <locale>
#include <Windows.h>
#include <vector>

class StringHandler {
public:

    static std::string uint64ToStringWithPadding(UINT64 value, int length) {
        std::ostringstream oss;
        oss << std::setw(length) << std::setfill('0') << value;
        return oss.str();
    }

    static std::string uint64ToStringWithGroupingAndPadding(UINT64 value, int length) {
        std::ostringstream oss;
        oss << std::setw(length) << std::setfill('0') << value;

        // Insert a space between every 3rd character from the end
        std::string result = oss.str();
        for (int i = result.length() - 3; i > 0; i -= 3) {
            result.insert(i, " ");
        }

        return result;
    }

    static std::string replaceAll(std::string target, std::string toReplace, std::string replacer) {
        std::string::size_type pos = 0u;
        while ((pos = target.find(toReplace, pos)) != std::string::npos) {
            target.replace(pos, toReplace.length(), replacer);
            pos += toReplace.length();
        }
        return target;
    }

    //use %i in before/after for line count
    static std::string parseArray(std::vector<std::string> arr, std::string before = "", std::string after = "\n") {
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
};