#ifndef DATA_DOCUMENT_TYPECONVERSION_HPP
#define DATA_DOCUMENT_TYPECONVERSION_HPP

//------------------------------------------
// Includes

// Standard library
#include <optional>
#include <string>

// Nebulite
//#include "Module/Base/TransformationModule.hpp"
#include "Utility/StringHandler.hpp"

//------------------------------------------
namespace Nebulite::Data {

// Converter helper functions for convertVariant
class TypeConversion {
public:
    static std::optional<bool> stringToBool(std::string const& stored){
        if (stored == "true") return true;
        if (stored == "false") return false;
        if(Utility::StringHandler::isNumber(stored)){
            try {
                return std::stoi(stored) != 0;
            } catch (...){
                return std::nullopt;
            }
        }
        return stored == "true";
    }

    static std::optional<int> stringToInt(std::string const& stored){
        if (stored == "true") return 1;
        if (stored == "false") return 0;
        try {
            return std::stoi(stored);
        } catch (...){
            return std::nullopt;
        }
    }

    static std::optional<uint8_t> stringToUInt8(std::string const& stored){
        if (stored == "true") return 1;
        if (stored == "false") return 0;
        try {
            return static_cast<uint8_t>(std::stoul(stored));
        } catch (...){
            return std::nullopt;
        }
    }

    static std::optional<int8_t> stringToInt8(std::string const& stored){
        if (stored == "true") return 1;
        if (stored == "false") return 0;
        try {
            return static_cast<int8_t>(std::stol(stored));
        } catch (...){
            return std::nullopt;
        }
    }

    static std::optional<uint16_t> stringToUInt16(std::string const& stored){
        if (stored == "true") return 1;
        if (stored == "false") return 0;
        try {
            return static_cast<uint16_t>(std::stoul(stored));
        } catch (...){
            return std::nullopt;
        }
    }

    static std::optional<int16_t> stringToInt16(std::string const& stored){
        if (stored == "true") return 1;
        if (stored == "false") return 0;
        try {
            return static_cast<int16_t>(std::stol(stored));
        } catch (...){
            return std::nullopt;
        }
    }

    static std::optional<uint32_t> stringToUInt32(std::string const& stored){
        if (stored == "true") return 1;
        if (stored == "false") return 0;
        try {
            return static_cast<uint32_t>(std::stoul(stored));
        } catch (...){
            return std::nullopt;
        }
    }

    static std::optional<int32_t> stringToInt32(std::string const& stored){
        if (stored == "true") return 1;
        if (stored == "false") return 0;
        try {
            return static_cast<int32_t>(std::stol(stored));
        } catch (...){
            return std::nullopt;
        }
    }

    static std::optional<uint64_t> stringToUInt64(std::string const& stored){
        if (stored == "true") return 1;
        if (stored == "false") return 0;
        try {
            return std::stoull(stored);
        } catch (...){
            return std::nullopt;
        }
    }

    static std::optional<int64_t> stringToInt64(std::string const& stored){
        if (stored == "true") return 1;
        if (stored == "false") return 0;
        try {
            return std::stoll(stored);
        } catch (...){
            return std::nullopt;
        }
    }

    static std::optional<double> stringToDouble(std::string const& stored){
        if (stored == "true") return 1.0;
        if (stored == "false") return 0.0;
        try {
            return std::stod(stored);
        } catch (...){
            return std::nullopt;
        }
    }
};

} // namespace Nebulite::Data
#endif // DATA_DOCUMENT_TYPECONVERSION_HPP
