//------------------------------------------
// Includes

// External
#include <rapidjson/stringbuffer.h>
#include <rapidjson/prettywriter.h>

// Nebulite
#include "Core/GlobalSpace.hpp"
#include "Utility/RjDirectAccess.hpp"
#include "Utility/StringHandler.hpp"

//------------------------------------------
namespace Nebulite::Utility {
//------------------------------------------
// Static Public Helper Functions

rapidjson::Value* RjDirectAccess::traverse_path(char const* key, rapidjson::Value const& val){
    rapidjson::Value const* current = &val;
    std::string_view keyView(key);

    while (!keyView.empty()){
        // Extract current key part (object key)
        std::string keyPart = extractKeyPart(&keyView);

        // Handle object key part if non-empty
        if (!keyPart.empty()){
            if (!current->IsObject()){
                return nullptr;
            }
            if (!current->HasMember(keyPart.c_str())){
                return nullptr;
            }
            current = &(*current)[keyPart.c_str()];
        }

        // Now handle zero or more array indices if they appear next
        while (!keyView.empty() && keyView[0] == '['){
            // Find closing ']'
            size_t const closeBracket = keyView.find(']');
            if (closeBracket == std::string_view::npos){
                // Malformed key - missing ']'
                return nullptr;
            }

            // Extract index string between '[' and ']'
            std::string_view idxStr = keyView.substr(1, closeBracket - 1);
            unsigned int index = 0;
            try {
                index = static_cast<unsigned int>(std::stoul(std::string(idxStr)));
            } catch (...){
                return nullptr; // invalid number
            }

            // Make sure current is an array
            if (!current->IsArray()){
                return nullptr;
            }

            // Check if array size is high enough
            if (current->Size() <= index){
                return nullptr;
            }

            current = &(*current)[index];

            // Remove processed '[index]'
            keyView.remove_prefix(closeBracket + 1);
        }

        // If next character is '.', skip it and continue
        if (!keyView.empty() && keyView[0] == '.'){
            keyView.remove_prefix(1);
        }
    }
    return const_cast<rapidjson::Value*>(current);
}

rapidjson::Value* RjDirectAccess::ensure_path(char const* key, rapidjson::Value& val, rapidjson::Document::AllocatorType& allocator){
    rapidjson::Value* current = &val;
    std::string_view keyView(key);

    while (!keyView.empty()){
        // Extract current key part (object key)
        std::string keyPart = extractKeyPart(&keyView);

        // Handle object key part if non-empty
        if (!keyPart.empty()){
            if (!current->IsObject()){
                current->SetObject();
            }

            if (!current->HasMember(keyPart.c_str())){
                rapidjson::Value keyVal(keyPart.c_str(), allocator);
                rapidjson::Value newObj(rapidjson::kObjectType);
                current->AddMember(keyVal, newObj, allocator);
            }
            current = &(*current)[keyPart.c_str()];
        }

        // Now handle zero or more array indices if they appear next
        while (!keyView.empty() && keyView[0] == '['){
            // Find closing ']'
            size_t closeBracket = keyView.find(']');
            if (closeBracket == std::string_view::npos){
                // Malformed key - missing ']'
                return nullptr;
            }

            // Extract index string between '[' and ']'
            std::string_view idxStr = keyView.substr(1, closeBracket - 1);
            unsigned int index = 0;
            try {
                index = static_cast<unsigned int>(std::stoul(std::string(idxStr)));
            } catch (...){
                return nullptr; // invalid number
            }

            // Make sure current is an array
            if (!current->IsArray()){
                current->SetArray();
            }

            // Expand array if needed
            while (current->Size() <= index){
                rapidjson::Value emptyObj(rapidjson::kObjectType);
                current->PushBack(emptyObj, allocator);
            }

            current = &(*current)[index];

            // Remove processed '[index]'
            keyView.remove_prefix(closeBracket + 1);
        }

        // If next character is '.', skip it and continue
        if (!keyView.empty() && keyView[0] == '.'){
            keyView.remove_prefix(1);
        }
    }

    return current;
}

rapidjson::Value RjDirectAccess::sortRecursive(rapidjson::Value const& value, rapidjson::Document::AllocatorType& allocator){
    if (value.IsObject()){
        // Sort object keys
        std::vector<std::pair<std::string, rapidjson::Value const*>> members;
        for (auto itr = value.MemberBegin(); itr != value.MemberEnd(); ++itr){
            members.emplace_back(itr->name.GetString(), &itr->value);
        }
        std::ranges::sort(members,[](auto const& a, auto const& b){return a.first < b.first;});
        rapidjson::Value sortedObj(rapidjson::kObjectType);
        for (auto const& [key, unsortedValue] : members){
            rapidjson::Value name(key.c_str(), allocator);
            rapidjson::Value sortedVal = sortRecursive(*unsortedValue, allocator);
            sortedObj.AddMember(name, sortedVal, allocator);
        }
        return sortedObj;
    }
    if (value.IsArray()){
        // Preserve array order; sort internal objects if any
        rapidjson::Value newArr(rapidjson::kArrayType);
        for (auto const& v : value.GetArray()){
            newArr.PushBack(sortRecursive(v, allocator), allocator);
        }
        return newArr;
    }
    // Primitive value: return a deep copy
    return {value, allocator};
}

std::string RjDirectAccess::serialize(rapidjson::Document const& doc){
    if (!doc.IsObject() && !doc.IsArray()){
        Capture::cerr() << "Serialization only supports JSON objects or arrays!" << Capture::endl;
        return "{}";
    }

    rapidjson::Document sortedDoc;
    sortedDoc.SetObject(); // Required before Swap or adding values

    rapidjson::Value sortedRoot = sortRecursive(doc, sortedDoc.GetAllocator());
    sortedDoc.Swap(sortedRoot); // Efficiently replace contents

    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter writer(buffer);
    sortedDoc.Accept(writer);
    return buffer.GetString();
}

void RjDirectAccess::deserialize(rapidjson::Document& doc, std::string const& serialOrLink, Core::GlobalSpace* global){
    std::string jsonString;

    // Check if the input is already a serialized JSON string
    if (is_json_or_jsonc(serialOrLink)){
        jsonString = serialOrLink;
    }
    // If not, treat it as a file path
    else {
        //------------------------------------------
        // Load the JSON file
        // First token is the path or serialized JSON
        jsonString = global->getDocCache()->getDocString(serialOrLink);
    }

    // Strip JSONC comments before parsing
    std::string const cleanJson = stripComments(jsonString);
    if (rapidjson::ParseResult const res = doc.Parse(cleanJson.c_str()); !res){
        Capture::cerr() << "JSON Parse Error at offset " << res.Offset() << Capture::endl;
        Capture::cerr() << "String is:\n" << cleanJson << Capture::endl;
    }
}

void RjDirectAccess::empty(rapidjson::Document &doc){
    doc.SetNull();
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

    bool handleSingleLineComment(char const c, ParseState& state, std::string& result){
        if (c == '\n'){
            state.inSingleComment = false;
            result += c; // Preserve newline for line counting
        }
        return true; // Character was handled
    }

    bool handleMultiLineComment(char const c, char const next, ParseState& state, size_t& skipNext){
        if (c == '*' && next == '/'){
            state.inMultiComment = false;
            skipNext = 1; // Skip the '/'
        }
        return true; // Character was handled
    }

    bool handleStringContent(char const c, ParseState& state, std::string& result){
        result += c;
        if (state.escaped){
            state.escaped = false;
        } else if (c == '\\'){
            state.escaped = true;
        } else if (c == '"'){
            state.inString = false;
        }
        return true; // Character was handled
    }

    bool handleRegularContent(char const c, char const next, ParseState& state, std::string& result, size_t& skipNext){
        if (c == '"'){
            state.inString = true;
            result += c;
        } else if (c == '/' && next == '/'){
            state.inSingleComment = true;
            skipNext = 1; // Skip the second '/'
        } else if (c == '/' && next == '*'){
            state.inMultiComment = true;
            skipNext = 1; // Skip the '*'
        } else {
            result += c;
        }
        return true; // Character was handled
    }
}

std::string RjDirectAccess::stripComments(std::string const& jsonc){
    std::string result;
    result.reserve(jsonc.size());
    ParseState state;
    for (size_t i = 0; i < jsonc.size(); ++i){
        char const c = jsonc[i];
        char const next = i + 1 < jsonc.size() ? jsonc[i + 1] : '\0';
        size_t skipNext = 0;

        if (state.inSingleComment){
            handleSingleLineComment(c, state, result);
        } else if (state.inMultiComment){
            handleMultiLineComment(c, next, state, skipNext);
        } else if (state.inString){
            handleStringContent(c, state, result);
        } else {
            handleRegularContent(c, next, state, result, skipNext);
        }

        i += skipNext; // Skip additional characters if needed
    }
    return result;
}

rapidjson::Value* RjDirectAccess::traverse_to_parent(char const* fullKey, rapidjson::Value const& root, std::string& finalKey, int& arrayIndex){
    std::string const keyStr(fullKey);
    size_t const lastDot = keyStr.find_last_of('.');
    size_t const lastBracket = keyStr.find_last_of('[');

    rapidjson::Value const* parent = nullptr;
    if (lastBracket != std::string::npos && (lastDot == std::string::npos || lastBracket > lastDot)){
        // Last access is array index: var.subVar[2] or var[2]
        size_t const openBracket = keyStr.find_last_of('[');
        if (size_t const closeBracket = keyStr.find_last_of(']'); openBracket != std::string::npos && closeBracket != std::string::npos && closeBracket > openBracket){
            std::string const parentPath = keyStr.substr(0, openBracket);
            std::string const indexStr = keyStr.substr(openBracket + 1, closeBracket - openBracket - 1);

            try {
                arrayIndex = std::stoi(indexStr);
                if (parentPath.empty()){
                    parent = &root;
                } else {
                    parent = traverse_path(parentPath.c_str(), root);
                }
            } catch (...){
                return nullptr; // Invalid index
            }
        }
    } else if (lastDot != std::string::npos){
        // Last access is object member: var.subVar.finalKey
        std::string const parentPath = keyStr.substr(0, lastDot);
        finalKey = keyStr.substr(lastDot + 1);
        parent = traverse_path(parentPath.c_str(), root);
    }
    return const_cast<rapidjson::Value*>(parent);
}

void RjDirectAccess::remove_member(char const* key, rapidjson::Value& val){
    // Handle simple case: direct member of root document
    if (std::string const keyStr(key); keyStr.find('.') == std::string::npos && keyStr.find('[') == std::string::npos){
        if (val.HasMember(key)){
            val.RemoveMember(key);
        }
        return;
    }

    // For complex paths, find the parent and final key/index


    // Now we find the parent value to delete the child from:
    // - parent.child
    // - parent[index]
    // - parent.child[index]
    // Remove the final key/index from parent
    std::string finalKey;
    int arrayIndex = -1;
    if (auto* parent = traverse_to_parent(key, val, finalKey, arrayIndex); parent != nullptr){
        if (arrayIndex >= 0){
            // Remove an array element
            if(!finalKey.empty()){
                parent[arrayIndex].RemoveMember(finalKey.c_str());
            }
            else if (parent->IsArray() && arrayIndex < static_cast<int>(parent->Size())){
                parent->Erase(parent->Begin() + arrayIndex);
            }
        } else if (!finalKey.empty()){
            // Remove object member
            if (parent->IsObject() && parent->HasMember(finalKey.c_str())){
                parent->RemoveMember(finalKey.c_str());
            }
        }
    }
}

bool RjDirectAccess::is_json_or_jsonc(std::string const& str){
    // So far, we do a simple check based on starting characters
    bool const check1 = str.starts_with("{") || str.starts_with("//") || str.starts_with("/*") || str.starts_with("\n");
    return check1;
}

bool RjDirectAccess::isValidKey(std::string const& key){
    std::string_view keyView(key);
    while (!keyView.empty()){
        // Extract current key part (object key)
        // Validate object key part if non-empty
        if (std::string keyPart = extractKeyPart(&keyView); !keyPart.empty()){
            // Check for invalid characters in keyPart
            if (keyPart.find_first_of("[]") != std::string_view::npos){
                return false; // Invalid character found
            }
        }

        // Now handle zero or more array indices if they appear next
        while (!keyView.empty() && keyView[0] == '['){
            // Find closing ']'
            size_t const closeBracket = keyView.find(']');
            if (closeBracket == std::string_view::npos){
                return false; // Malformed key - missing ']'
            }

            // Extract index string between '[' and ']'
            if(std::string_view idxStr = keyView.substr(1, closeBracket - 1); !StringHandler::isNumber(std::string(idxStr))){
                return false; // invalid number
            }

            // Remove processed '[index]'
            keyView.remove_prefix(closeBracket + 1);
        }

        // If next character is '.', skip it and continue
        if (!keyView.empty() && keyView[0] == '.'){
            keyView.remove_prefix(1);
        }
    }
    return true;
}

//------------------------------------------
// Static Private Helper Functions

// Helper for key traversal: extracts next key part and advances the view
std::string RjDirectAccess::extractKeyPart(std::string_view* keyView){
    if (!keyView) return std::string{};
    // Find '.' or '[' as next separators
    size_t const dotPos     = keyView->find('.');
    size_t const bracketPos = keyView->find('[');

    size_t nextSep;
    if (dotPos == std::string_view::npos && bracketPos == std::string_view::npos){
        nextSep = keyView->size();  // No separator - last key
    } else if (dotPos == std::string_view::npos){
        nextSep = bracketPos;
    } else if (bracketPos == std::string_view::npos){
        nextSep = dotPos;
    } else {
        nextSep = std::min(dotPos, bracketPos);
    }

    // Build the result string from the current data/length before modifying the input view.
    std::string const result(keyView->data(), nextSep);
    keyView->remove_prefix(nextSep);
    return result;
}
} // namespace Nebulite::Utility