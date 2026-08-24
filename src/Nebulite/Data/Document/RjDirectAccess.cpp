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

rapidjson::Value* traverseIntoObject(std::string_view const member, [[clang::lifetimebound]] rapidjson::Value* current) {
    if (member.empty()) return current;
    if (!current->IsObject()) {
        return nullptr;
    }
    auto const memberValue = rapidjson::Value{
        rapidjson::StringRef(member.data(), member.size())
    };
    auto it = current->FindMember(memberValue);
    if (it == current->MemberEnd()) {
        return nullptr;
    }
    return &it->value;
}

rapidjson::Value* traverseIntoArray(std::string_view& keyView, rapidjson::Value* current) {
    // Find closing character
    auto const closeBracket = keyView.find(SpecialCharacter::arrayClose);
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
std::string_view extractKeyPart(std::string_view& keyView) {
    // Find dot or array opening char as next separators
    auto const dotPos = keyView.find(SpecialCharacter::dot);
    auto const bracketPos = keyView.find(SpecialCharacter::arrayOpen);

    auto const nextSep = [&] {
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

    // Remove the extracted part from keyView and return it
    auto const extracted = keyView.substr(0, nextSep);
    keyView.remove_prefix(nextSep);
    return extracted;
}

} // namespace

// TODO: does this work if the key starts with an array? Function isn't as clear as it could be, please refactor to make it more readable and understandable.
rapidjson::Value* traversePath(std::string_view const key, [[clang::lifetimebound]] rapidjson::Value& val) {
    rapidjson::Value* current = &val;
    std::string_view keyView(key);

    while (!keyView.empty()) {
        // Extract current key part (object key)
        auto const keyPart = extractKeyPart(keyView);

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

traverseResult traverseToParent(std::string_view keyStr, [[clang::lifetimebound]] rapidjson::Value& root) {
    auto const lastDot = keyStr.find_last_of(SpecialCharacter::dot);
    auto const lastBracket = keyStr.find_last_of(SpecialCharacter::arrayOpen);

    if (lastBracket != std::string::npos && (lastDot == std::string::npos || lastBracket > lastDot)) {
        // Last access is array index: var.subVar[2] or var[2]
        auto const openBracket = keyStr.find_last_of(SpecialCharacter::arrayOpen);
        if (auto const closeBracket = keyStr.find_last_of(SpecialCharacter::arrayClose); openBracket != std::string::npos && closeBracket != std::string::npos && closeBracket > openBracket) {
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

namespace {

rapidjson::Value* ensurePathIntoObject(std::string_view const member, [[clang::lifetimebound]] rapidjson::Value* current, rapidjson::Document::AllocatorType& allocator) {
    if (member.empty()) return current;
    if (!current->IsObject()) current->SetObject();

    rapidjson::Value const lookup(rapidjson::StringRef(member.data(), member.size()));
    auto it = current->FindMember(lookup);
    if (it != current->MemberEnd()) {
        return &it->value;
    }

    rapidjson::Value key;
    key.SetString(member.data(), static_cast<unsigned int>(member.size()), allocator);
    current->AddMember(
        key,
        rapidjson::Value(rapidjson::kObjectType),
        allocator
    );
    return &current->MemberEnd()[-1].value;
}

rapidjson::Value* ensurePathIntoArray(std::string_view& keyView, rapidjson::Value* current, rapidjson::Document::AllocatorType& allocator) {
    // Find closing character
    auto const closeBracket = keyView.find(SpecialCharacter::arrayClose);
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
        auto const keyPart = extractKeyPart(keyView);

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
    if (Utility::StringHandler::isNullTerminated(jsonString)) {
        auto const* nullTerminatedData = jsonString.data(); // Safe to use directly since it's null-terminated
        if (rapidjson::ParseResult const res = doc.Parse<rapidjsonParseFlags>(nullTerminatedData); !res) {
            Global::capture().error.println("JSON Parse Error at offset ", res.Offset(), ". String is:");
            Global::capture().error.println(nullTerminatedData);
        }
        return;
    }
    auto strCopy = std::string(jsonString); // Make a copy to ensure null-termination
    if (rapidjson::ParseResult const res = doc.Parse<rapidjsonParseFlags>(strCopy.c_str()); !res) {
        Global::capture().error.println("JSON Parse Error at offset ", res.Offset(), ". String is:");
        Global::capture().error.println(strCopy);
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

bool isJsonOrJsonc(std::string_view const str) {
    rapidjson::Document doc;
    if (Utility::StringHandler::isNullTerminated(str)) {
        auto const* nullTerminatedData = str.data(); // Safe to use directly since it's null-terminated
        return !doc.Parse<rapidjsonParseFlags>(nullTerminatedData).HasParseError();
    }
    auto strCopy = std::string(str); // Make a copy to ensure null-termination
    return !doc.Parse<rapidjsonParseFlags>(strCopy.c_str()).HasParseError();
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
        auto const keyStr = std::string(key); // Convert to std::string for rapidjson compatibility
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
                auto const finalKeyStr = std::string(poppedMember); // Convert to std::string for rapidjson compatibility
                parent[poppedIndex].RemoveMember(finalKeyStr.c_str());
            // NOLINTNEXTLINE
            } else if (parent->IsArray() && poppedIndex < static_cast<int>(parent->Size())) {
                parent->Erase(parent->Begin() + poppedIndex);
            }
        } else if (!poppedMember.empty()) {
            auto const finalKeyStr = std::string(poppedMember); // Convert to std::string for rapidjson compatibility
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
        if (auto const keyPart = extractKeyPart(keyView); !keyPart.empty()) {
            // Check for invalid characters in keyPart
            if (keyPart.find_first_of("[]") != std::string_view::npos) {
                return false; // Invalid character found
            }
        }

        // Now handle zero or more array indices if they appear next
        while (!keyView.empty() && keyView[0] == SpecialCharacter::arrayOpen) {
            // Find closing character
            auto const closeBracket = keyView.find(SpecialCharacter::arrayClose);
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
        auto const arrSize = val.Size();
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
