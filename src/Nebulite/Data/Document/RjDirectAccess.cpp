//------------------------------------------
// Includes

// Standard library
#include <algorithm>
#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

// External
#include <rapidjson/document.h>
#include <rapidjson/error/error.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/rapidjson.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

// Nebulite
#include "Nebulite/Core/GlobalSpace.hpp"
#include "Nebulite/Data/Document/RjDirectAccess.hpp"
#include "Nebulite/Nebulite.hpp"
#include "Nebulite/Utility/Convert/Cast.hpp"
#include "Nebulite/Utility/Io/Capture.hpp"
#include "Nebulite/Utility/StringHandler.hpp"

//------------------------------------------
namespace Nebulite::Data::RjDirectAccess {

// NOTE: While RjDirectAccess uses string_view for key traversal, they are converted to std::string when interacting with rapidjson,
// as rapidjson requires null-terminated strings for keys.
// Later on we should look for ways to avoid this conversion, perhaps a fork of rapidjson is required to support string_view directly.

//------------------------------------------
// Static Public Helper Functions

std::optional<SimpleValue> getSimpleValue(rapidjson::Value const* val) {
    // Integers
    if (val->IsInt())    return val->GetInt();
    if (val->IsInt64())  return val->GetInt64();
    if (val->IsUint())   return val->GetUint();
    if (val->IsUint64()) return val->GetUint64();
    if (val->IsDouble()) return val->GetDouble();
    if (val->IsString()) return std::string(val->GetString(), val->GetStringLength());
    if (val->IsBool())   return val->GetBool();
    // Convert directly unsupported types to double or float if possible
    if (val->IsLosslessDouble()) return val->GetDouble();
    if (val->IsLosslessFloat())  return val->GetDouble();
    if (val->IsFloat())  return val->GetDouble();
    return  std::nullopt;
}

//------------------------------------------
// Document traversal

namespace {

rapidjson::Value* traverseIntoObject(std::string_view const keyPart, [[clang::lifetimebound]] rapidjson::Value* current) {
    if (!keyPart.empty()) {
        auto keyPartStr = std::string(keyPart); // Convert to std::string for rapidjson compatibility

        if (!current->IsObject()) {
            return nullptr;
        }
        if (!current->HasMember(keyPartStr.c_str())) {
            return nullptr;
        }
        return &(*current)[keyPartStr.c_str()];
    }
    return current;
}

rapidjson::Value* traverseIntoArray(std::string_view& keyView, rapidjson::Value* current) {
    // Find closing character
    std::size_t const closeBracket = keyView.find(SpecialCharacter::arrayClose);
    if (closeBracket == std::string_view::npos) {
        // Malformed key - missing closing character
        return nullptr;
    }

    // Extract index string between open and close array character
    auto index = Utility::Convert::Cast::String::to<unsigned int>(keyView.substr(1, closeBracket - 1));
    if (!index.has_value()) {
        return nullptr; // invalid number
    }

    // Make sure current is an array
    if (!current->IsArray()) {
        return nullptr;
    }

    // Check if array size is high enough
    if (current->Size() <= index.value()) {
        return nullptr;
    }

    // Remove processed '[index]'
    keyView.remove_prefix(closeBracket + 1);
    return &(*current)[index.value()];
}

/**
 * @brief Extracts the next part of a key from a dot/bracket notation key string.
 *        Moves keyView forward past the extracted part.
 * @param keyView View to extract from and modify.
 * @return The extracted key part as a std::string.
 */
std::string extractKeyPart(std::string_view& keyView) {
    // Find dot or array opening char as next separators
    std::size_t const dotPos = keyView.find(SpecialCharacter::dot);
    std::size_t const bracketPos = keyView.find(SpecialCharacter::arrayOpen);

    std::size_t const nextSep = [&] {
        if (dotPos == std::string_view::npos && bracketPos == std::string_view::npos) {
            return keyView.size(); // No separator - last key
        }
        if (dotPos == std::string_view::npos) {
            return bracketPos;
        }
        if (bracketPos == std::string_view::npos) {
            return dotPos;
        }
        return std::min(dotPos, bracketPos);
    }();

    // Build the result string from the current data/length before modifying the input view.
    auto const result = std::string(keyView.substr(0, nextSep));
    keyView.remove_prefix(nextSep);
    return result;
}

} // namespace

rapidjson::Value* traversePath(std::string_view const key, [[clang::lifetimebound]] rapidjson::Value& val) {
    rapidjson::Value* current = &val;
    std::string_view keyView(key);

    while (!keyView.empty()) {
        // Extract current key part (object key)
        std::string const keyPart = extractKeyPart(keyView);

        // Handle object key part if non-empty
        current = traverseIntoObject(keyPart, current);
        if (!current) return nullptr;

        // Now handle zero or more array indices if they appear next
        while (!keyView.empty() && keyView[0] == SpecialCharacter::arrayOpen) {
            current = traverseIntoArray(keyView, current);
            if (!current) return nullptr;
        }

        // If next character is dot, skip it and continue
        if (!keyView.empty() && keyView[0] == SpecialCharacter::dot) {
            keyView.remove_prefix(1);
        }
    }
    return current;
}

namespace {

rapidjson::Value* ensurePathIntoObject(std::string const& keyPart, [[clang::lifetimebound]] rapidjson::Value* current, rapidjson::Document::AllocatorType& allocator) {
    if (!keyPart.empty()) {
        if (!current->IsObject()) {
            current->SetObject();
        }

        if (!current->HasMember(keyPart.c_str())) {
            rapidjson::Value keyVal(keyPart.c_str(), allocator);
            rapidjson::Value newObj(rapidjson::kObjectType);
            current->AddMember(keyVal, newObj, allocator);
        }
        return &(*current)[keyPart.c_str()];
    }
    return current;
}

rapidjson::Value* ensurePathIntoArray(std::string_view& keyView, rapidjson::Value* current, rapidjson::Document::AllocatorType& allocator) {
    // Find closing character
    std::size_t const closeBracket = keyView.find(SpecialCharacter::arrayClose);
    if (closeBracket == std::string_view::npos) {
        // Malformed key - missing closing character
        return nullptr;
    }

    // Extract index string between open and close array character
    auto index = Utility::Convert::Cast::String::to<unsigned int>(keyView.substr(1, closeBracket - 1));
    if (!index.has_value()) {
        return nullptr; // invalid number
    }

    // Make sure current is an array
    if (!current->IsArray()) {
        current->SetArray();
    }

    // Expand array if needed
    while (current->Size() <= index.value()) {
        rapidjson::Value emptyObj(rapidjson::kObjectType);
        current->PushBack(emptyObj, allocator);
    }

    // Remove processed '[index]'
    keyView.remove_prefix(closeBracket + 1);
    return &(*current)[index.value()];
}

} // namespace

rapidjson::Value* ensurePath(std::string_view const key, [[clang::lifetimebound]] rapidjson::Value& val, rapidjson::Document::AllocatorType& allocator) {
    rapidjson::Value* current = &val;
    std::string_view keyView(key);

    while (!keyView.empty()) {
        // Extract current key part (object key)
        std::string const keyPart = extractKeyPart(keyView);

        // Handle object key part if non-empty
        current = ensurePathIntoObject(keyPart, current, allocator);
        if (!current) return nullptr;

        // Now handle zero or more array indices if they appear next
        while (!keyView.empty() && keyView[0] == SpecialCharacter::arrayOpen) {
            current = ensurePathIntoArray(keyView, current, allocator);
            if (!current) return nullptr;
        }

        // If next character is dot, skip it and continue
        if (!keyView.empty() && keyView[0] == SpecialCharacter::dot) {
            keyView.remove_prefix(1);
        }
    }

    return current;
}

traverseResult traverseToParent(std::string_view keyStr, [[clang::lifetimebound]] rapidjson::Value& root) {
    std::size_t const lastDot = keyStr.find_last_of(SpecialCharacter::dot);
    std::size_t const lastBracket = keyStr.find_last_of(SpecialCharacter::arrayOpen);

    if (lastBracket != std::string::npos && (lastDot == std::string::npos || lastBracket > lastDot)) {
        // Last access is array index: var.subVar[2] or var[2]
        std::size_t const openBracket = keyStr.find_last_of(SpecialCharacter::arrayOpen);
        if (std::size_t const closeBracket = keyStr.find_last_of(SpecialCharacter::arrayClose); openBracket != std::string::npos && closeBracket != std::string::npos && closeBracket > openBracket) {
            auto const parentPath = keyStr.substr(0, openBracket);
            auto const indexStr = keyStr.substr(openBracket + 1, closeBracket - openBracket - 1);

            auto idx = Utility::Convert::Cast::String::to<int>(indexStr);
            if (!idx.has_value()){ // Invalid index
                return {
                    .parent=nullptr,
                    .poppedMember="",
                    .poppedIndex=-1
                };
            }
            if (parentPath.empty()) { // Parent is root
                return {
                    .parent=&root,
                    .poppedMember="",
                    .poppedIndex=idx.value()
                };
            }
            return {
                .parent=traversePath(parentPath, root),
                .poppedMember="",
                .poppedIndex=idx.value()
            };
        }
    }
    if (lastDot != std::string::npos) {
        // Last access is object member: var.subVar.finalKey
        return {
            .parent=traversePath(keyStr.substr(0, lastDot), root),
            .poppedMember=keyStr.substr(lastDot + 1),
            .poppedIndex=-1
        };
    }
    return {
        .parent=nullptr,
        .poppedMember="",
        .poppedIndex=-1
    };
}

//------------------------------------------
// Serialization/Deserialization


std::string serialize(rapidjson::Document const& doc, SerializationType type) {
    // Determine writer type
    rapidjson::StringBuffer buffer;

    switch (type) {
    case SerializationType::pretty: {
        rapidjson::PrettyWriter writer(buffer);

        // If root is not object or array, wrap root as object/array
        if (!doc.IsObject() && !doc.IsArray()) {
            rapidjson::Document tmp;
            tmp.SetObject();
            rapidjson::Value sortedRoot = sortRecursive(doc, tmp.GetAllocator());
            tmp.Swap(sortedRoot);
            tmp.Accept(writer);
        } else {
            rapidjson::Document sortedDoc;
            sortedDoc.SetObject();
            rapidjson::Value sortedRoot = sortRecursive(doc, sortedDoc.GetAllocator());
            sortedDoc.Swap(sortedRoot);
            sortedDoc.Accept(writer);
        }
        break;
    }
    case SerializationType::compact: {
        rapidjson::Writer writer(buffer);

        if (!doc.IsObject() && !doc.IsArray()) {
            rapidjson::Document tmp;
            tmp.SetObject();
            rapidjson::Value sortedRoot = sortRecursive(doc, tmp.GetAllocator());
            tmp.Swap(sortedRoot);
            tmp.Accept(writer);
        } else {
            rapidjson::Document sortedDoc;
            sortedDoc.SetObject();
            rapidjson::Value sortedRoot = sortRecursive(doc, sortedDoc.GetAllocator());
            sortedDoc.Swap(sortedRoot);
            sortedDoc.Accept(writer);
        }
        break;
    }
    default:
        std::unreachable();
    }

    return buffer.GetString();
}

std::string serialize(rapidjson::Value const& val, SerializationType const type) {
    rapidjson::Document tempDoc;
    tempDoc.SetObject(); // Required before Swap or adding values
    rapidjson::Value sortedVal = sortRecursive(val, tempDoc.GetAllocator());
    tempDoc.Swap(sortedVal); // Efficiently replace contents
    return serialize(tempDoc, type);
}

void deserialize(rapidjson::Document& doc, std::string_view const serialOrLink) {
    std::string jsonString;

    // TODO: remove the serialization of a link, should only be part of Data::Json

    // Check if the input is already a serialized JSON string
    if (isJsonOrJsonc(serialOrLink)) {
        jsonString = serialOrLink;
    }
    // If not, treat it as a file path
    else {
        //------------------------------------------
        // Load the JSON file
        // First token is the path or serialized JSON
        jsonString = Global::instance().getDocCache().getDocString(serialOrLink);
    }

    // Strip JSONC comments before parsing
    std::string const cleanJson = stripComments(jsonString);
    if (rapidjson::ParseResult const res = doc.Parse(cleanJson.c_str()); !res) {
        Global::capture().error.println("JSON Parse Error at offset ", res.Offset(), ". String is:");
        Global::capture().error.println(cleanJson);
    }
}

//------------------------------------------
// Helper functions

rapidjson::Value sortRecursive(rapidjson::Value const& value, rapidjson::Document::AllocatorType& allocator) {
    if (value.IsObject()) {
        // Sort object keys
        std::vector<std::pair<std::string, rapidjson::Value const*>> members;
        for (auto itr = value.MemberBegin(); itr != value.MemberEnd(); ++itr) {
            members.emplace_back(itr->name.GetString(), &itr->value);
        }
        std::ranges::sort(members, [](auto const& a, auto const& b) { return a.first < b.first; });
        rapidjson::Value sortedObj(rapidjson::kObjectType);
        for (auto const& [key, unsortedValue] : members) {
            rapidjson::Value name(key.c_str(), allocator);
            rapidjson::Value sortedVal = sortRecursive(*unsortedValue, allocator);
            sortedObj.AddMember(name, sortedVal, allocator);
        }
        return sortedObj;
    }
    if (value.IsArray()) {
        // Preserve array order; sort internal objects if any
        rapidjson::Value newArr(rapidjson::kArrayType);
        for (auto const& v : value.GetArray()) {
            newArr.PushBack(sortRecursive(v, allocator), allocator);
        }
        return newArr;
    }
    // Primitive value: return a deep copy
    return {value, allocator};
}

/**
 * @brief Helpers for comment stripping
 */
namespace {
struct ParseState {
    bool inString = false;
    bool inSingleComment = false;
    bool inMultiComment = false;
    bool escaped = false;
};

bool handleSingleLineComment(char const c, ParseState& state, std::string& result) {
    if (c == '\n') {
        state.inSingleComment = false;
        result += c; // Preserve newline for line counting
    }
    return true; // Character was handled
}

bool handleMultiLineComment(char const c, char const next, ParseState& state, std::size_t& skipNext) {
    if (c == '*' && next == '/') {
        state.inMultiComment = false;
        skipNext = 1; // Skip the '/'
    }
    return true; // Character was handled
}

bool handleStringContent(char const c, ParseState& state, std::string& result) {
    result += c;
    if (state.escaped) {
        state.escaped = false;
    } else if (c == '\\') {
        state.escaped = true;
    } else if (c == '"') {
        state.inString = false;
    }
    return true; // Character was handled
}

bool handleRegularContent(char const c, char const next, ParseState& state, std::string& result, std::size_t& skipNext) {
    if (c == '"') {
        state.inString = true;
        result += c;
    } else if (c == '/' && next == '/') {
        state.inSingleComment = true;
        skipNext = 1; // Skip the second '/'
    } else if (c == '/' && next == '*') {
        state.inMultiComment = true;
        skipNext = 1; // Skip the '*'
    } else {
        result += c;
    }
    return true; // Character was handled
}
} // namespace

std::string stripComments(std::string_view const jsonc) {
    std::string result;
    result.reserve(jsonc.size());
    ParseState state;
    for (std::size_t i = 0; i < jsonc.size(); ++i) {
        char const c = jsonc[i];
        char const next = i + 1 < jsonc.size() ? jsonc[i + 1] : '\0';
        std::size_t skipNext = 0;

        if (state.inSingleComment) {
            handleSingleLineComment(c, state, result);
        } else if (state.inMultiComment) {
            handleMultiLineComment(c, next, state, skipNext);
        } else if (state.inString) {
            handleStringContent(c, state, result);
        } else {
            handleRegularContent(c, next, state, result, skipNext);
        }

        i += skipNext; // Skip additional characters if needed
    }
    return result;
}

bool isJsonOrJsonc(std::string_view const str) {
    // Complicated check using RapidJSON parsing
    // Simpler check is just not worth it due to various valid JSON formats
    rapidjson::Document doc;
    std::string const cleanJson = stripComments(str);
    return !doc.Parse(cleanJson.c_str()).HasParseError();
}

//------------------------------------------
// Member management

void empty(rapidjson::Document& doc) {
    doc.SetNull();
}

void removeMember(std::string_view const key, rapidjson::Value& val) {
    // Edge case: empty key
    if (key.empty()) {
        // Remove entire document content
        val.SetNull();
        return;
    }

    // Handle simple case: direct member of root document
    if (!key.contains(SpecialCharacter::dot) && !key.contains(SpecialCharacter::arrayOpen)) {
        std::string const keyStr(key);
        if (val.HasMember(keyStr.c_str())) {
            val.RemoveMember(keyStr.c_str());
        }
        return;
    }

    // For complex paths, find the parent and final key/index

    // Now we find the parent value to delete the child from:
    // - parent.child
    // - parent[index]
    // - parent.child[index]
    // Remove the final key/index from parent
    if (auto [parent, poppedMember, poppedIndex] = traverseToParent(key, val); parent != nullptr) {
        if (poppedIndex >= 0) {
            // Remove an array element
            if (!poppedMember.empty()) {
                std::string const finalKeyStr(poppedMember);
                parent[poppedIndex].RemoveMember(finalKeyStr.c_str());
            // NOLINTNEXTLINE
            } else if (parent->IsArray() && poppedIndex < static_cast<int>(parent->Size())) {
                parent->Erase(parent->Begin() + poppedIndex);
            }
        } else if (!poppedMember.empty()) {
            std::string const finalKeyStr(poppedMember);
            // Remove object member
            if (parent->IsObject() && parent->HasMember(finalKeyStr.c_str())) {
                parent->RemoveMember(finalKeyStr.c_str());
            }
        }
    }
}

bool isValidKey(std::string_view const key) {
    std::string_view keyView(key);
    while (!keyView.empty()) {
        // Extract current key part (object key)
        // Validate object key part if non-empty
        if (std::string const keyPart = extractKeyPart(keyView); !keyPart.empty()) {
            // Check for invalid characters in keyPart
            if (keyPart.find_first_of("[]") != std::string_view::npos) {
                return false; // Invalid character found
            }
        }

        // Now handle zero or more array indices if they appear next
        while (!keyView.empty() && keyView[0] == SpecialCharacter::arrayOpen) {
            // Find closing character
            std::size_t const closeBracket = keyView.find(SpecialCharacter::arrayClose);
            if (closeBracket == std::string_view::npos) {
                return false; // Malformed key - missing closing character
            }

            // Extract index string between open and close character
            if (std::string_view const idxStr = keyView.substr(1, closeBracket - 1); !Utility::StringHandler::isNumber(idxStr)) {
                return false; // invalid number
            }

            // Remove processed '[index]'
            keyView.remove_prefix(closeBracket + 1);
        }

        // If next character is dot, skip it and continue
        if (!keyView.empty() && keyView[0] == SpecialCharacter::dot) {
            keyView.remove_prefix(1);
        }
    }
    return true;
}

std::vector<std::string> listAvailableMembers(rapidjson::Value const& val){
    std::vector<std::string> keys;
    if (val.IsArray()) {
        // Generate a list of array keys: [0], [1], ...
        std::size_t const arrSize = val.Size();
        keys.reserve(arrSize);
        for (std::size_t i = 0; i < arrSize; ++i) {
            keys.emplace_back("[" + std::to_string(i) + "]");
        }
        // Note: array keys are inherently ordered by index, no need to sort
        //       If we were to sort, this would cause issues as "[10]" would come before "[2]"
    }
    else if (val.IsObject()) {
        // Generate a list of object member keys
        for (auto itr = val.MemberBegin(); itr != val.MemberEnd(); ++itr) {
            keys.emplace_back(itr->name.GetString());
        }

        // Sort keys alphabetically
        std::ranges::sort(keys);
    }

    // For any other type, we return an empty list
    return keys;
}

} // namespace Nebulite::Data::RjDirectAccess
