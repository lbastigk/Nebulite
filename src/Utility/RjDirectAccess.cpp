#include "Utility/RjDirectAccess.hpp"

#include "Utility/FileManagement.hpp"

//------------------------------------------
// Static Public Helper Functions

rapidjson::Value* Nebulite::Utility::RjDirectAccess::traverse_path(const char* key, rapidjson::Value& val){
    rapidjson::Value* current = &val;
    std::string_view keyView(key);

    while (!keyView.empty()) {
        // Extract current key part (object key)
        std::string_view keyPart = extractKeyPart(&keyView);

        // Handle object key part if non-empty
        if (!keyPart.empty()) {
            if (!current->IsObject()) {
                return nullptr;
            }
            if (!current->HasMember(std::string(keyPart).c_str())) {
                return nullptr;
            }
            current = &(*current)[std::string(keyPart).c_str()];
        }

        // Now handle zero or more array indices if they appear next
        while (!keyView.empty() && keyView[0] == '[') {
            // Find closing ']'
            size_t closeBracket = keyView.find(']');
            if (closeBracket == std::string_view::npos) {
                // Malformed key - missing ']'
                return nullptr;
            }

            // Extract index string between '[' and ']'
            std::string_view idxStr = keyView.substr(1, closeBracket - 1);
            int index = 0;
            try {
                index = std::stoi(std::string(idxStr));
            } catch (...) {
                return nullptr; // invalid number
            }

            // Make sure current is array
            if (!current->IsArray()) {
                return nullptr;
            }

            // Check if array size is high enough
            if (current->Size() <= static_cast<rapidjson::SizeType>(index)) {
                return nullptr;
            }

            current = &(*current)[index];

            // Remove processed '[index]'
            keyView.remove_prefix(closeBracket + 1);
        }

        // If next character is '.', skip it and continue
        if (!keyView.empty() && keyView[0] == '.') {
            keyView.remove_prefix(1);
        }
    }
    return current;
}

rapidjson::Value* Nebulite::Utility::RjDirectAccess::ensure_path(const char* key, rapidjson::Value& val, rapidjson::Document::AllocatorType& allocator) {
    rapidjson::Value* current = &val;
    std::string_view keyView(key);

    while (!keyView.empty()) {
        // Extract current key part (object key)
        std::string_view keyPart = extractKeyPart(&keyView);

        // Handle object key part if non-empty
        if (!keyPart.empty()) {
            if (!current->IsObject()) {
                current->SetObject();
            }

            if (!current->HasMember(std::string(keyPart).c_str())) {
                rapidjson::Value keyVal(std::string(keyPart).c_str(), allocator);
                rapidjson::Value newObj(rapidjson::kObjectType);
                current->AddMember(keyVal, newObj, allocator);
            }
            current = &(*current)[std::string(keyPart).c_str()];
        }

        // Now handle zero or more array indices if they appear next
        while (!keyView.empty() && keyView[0] == '[') {
            // Find closing ']'
            size_t closeBracket = keyView.find(']');
            if (closeBracket == std::string_view::npos) {
                // Malformed key - missing ']'
                return nullptr;
            }

            // Extract index string between '[' and ']'
            std::string_view idxStr = keyView.substr(1, closeBracket - 1);
            int index = 0;
            try {
                index = std::stoi(std::string(idxStr));
            } catch (...) {
                return nullptr; // invalid number
            }

            // Make sure current is array
            if (!current->IsArray()) {
                current->SetArray();
            }

            // Expand array if needed
            while (current->Size() <= static_cast<rapidjson::SizeType>(index)) {
                rapidjson::Value emptyObj(rapidjson::kObjectType);
                current->PushBack(emptyObj, allocator);
            }

            current = &(*current)[index];

            // Remove processed '[index]'
            keyView.remove_prefix(closeBracket + 1);
        }

        // If next character is '.', skip it and continue
        if (!keyView.empty() && keyView[0] == '.') {
            keyView.remove_prefix(1);
        }
    }

    return current;
}

rapidjson::Value Nebulite::Utility::RjDirectAccess::sortRecursive(const rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) {
    if (value.IsObject()) {
        // Sort object keys
        std::vector<std::pair<std::string, const rapidjson::Value*>> members;
        for (auto itr = value.MemberBegin(); itr != value.MemberEnd(); ++itr) {
            members.emplace_back(itr->name.GetString(), &itr->value);
        }

        std::sort(members.begin(), members.end(),
                  [](const auto& a, const auto& b) {
                      return a.first < b.first;
                  });

        rapidjson::Value sortedObj(rapidjson::kObjectType);
        for (const auto& pair : members) {
            rapidjson::Value name(pair.first.c_str(), allocator);
            rapidjson::Value sortedVal = sortRecursive(*pair.second, allocator);
            sortedObj.AddMember(name, sortedVal, allocator);
        }
        return sortedObj;

    } else if (value.IsArray()) {
        // Preserve array order; sort internal objects if any
        rapidjson::Value newArr(rapidjson::kArrayType);
        for (const auto& v : value.GetArray()) {
            newArr.PushBack(sortRecursive(v, allocator), allocator);
        }
        return newArr;

    } else {
        // Primitive value: return a deep copy
        return rapidjson::Value(value, allocator);
    }
}

std::string Nebulite::Utility::RjDirectAccess::serialize(const rapidjson::Document& doc) {
    if (!doc.IsObject() && !doc.IsArray()) {
        std::cerr << "Serialization only supports JSON objects or arrays!" << std::endl;
        return "{}";
    }

    rapidjson::Document sortedDoc;
    sortedDoc.SetObject(); // Required before Swap or adding values

    rapidjson::Value sortedRoot = sortRecursive(doc, sortedDoc.GetAllocator());
    sortedDoc.Swap(sortedRoot); // Efficiently replace contents

    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
    sortedDoc.Accept(writer);
    return buffer.GetString();
}

void Nebulite::Utility::RjDirectAccess::deserialize(rapidjson::Document& doc, std::string serialOrLink) {
    std::string jsonString;
    
    // Check if the input is already a serialized JSON string
    if (serialOrLink.starts_with("{") || serialOrLink.starts_with("//") || serialOrLink.starts_with("/*") || serialOrLink.starts_with("\n")) {
        jsonString = serialOrLink;
    } 
    // If not, treat it as a file path
    else {
        //------------------------------------------
        // Load the JSON file
        // First token is the path or serialized JSON
        jsonString = Nebulite::Utility::FileManagement::LoadFile(serialOrLink.c_str());
    }
    
    // Strip JSONC comments before parsing
    std::string cleanJson = stripComments(jsonString);
    
    rapidjson::ParseResult res = doc.Parse(cleanJson.c_str());
    if (!res) {
        std::cerr << "JSON Parse Error at offset " << res.Offset() << std::endl;
    }
}

void Nebulite::Utility::RjDirectAccess::empty(rapidjson::Document &doc) {
    doc.SetNull();
    //doc.GetAllocator().Clear();
    //doc.Swap(rapidjson::Value(rapidjson::kObjectType).Move());
}

std::string Nebulite::Utility::RjDirectAccess::stripComments(const std::string& jsonc) {
    std::string result;
    result.reserve(jsonc.size());
    
    bool inString = false;
    bool inSingleComment = false;
    bool inMultiComment = false;
    bool escaped = false;
    
    for (size_t i = 0; i < jsonc.size(); ++i) {
        char c = jsonc[i];
        char next = (i + 1 < jsonc.size()) ? jsonc[i + 1] : '\0';
        
        if (inSingleComment) {
            if (c == '\n') {
                inSingleComment = false;
                result += c; // Preserve newline for line counting
            }
            continue;
        }
        
        if (inMultiComment) {
            if (c == '*' && next == '/') {
                inMultiComment = false;
                ++i; // Skip the '/'
            }
            continue;
        }
        
        if (inString) {
            result += c;
            if (escaped) {
                escaped = false;
            } else if (c == '\\') {
                escaped = true;
            } else if (c == '"') {
                inString = false;
            }
            continue;
        }
        
        // Not in string or comment
        if (c == '"') {
            inString = true;
            result += c;
        } else if (c == '/' && next == '/') {
            inSingleComment = true;
            ++i; // Skip the second '/'
        } else if (c == '/' && next == '*') {
            inMultiComment = true;
            ++i; // Skip the '*'
        } else {
            result += c;
        }
    }
    
    return result;
}

rapidjson::Value* Nebulite::Utility::RjDirectAccess::traverse_to_parent(const char* fullKey, rapidjson::Value& root, std::string& finalKey, int& arrayIndex) {
    std::string keyStr(fullKey);
    size_t lastDot = keyStr.find_last_of('.');
    size_t lastBracket = keyStr.find_last_of('[');

    rapidjson::Value* parent = nullptr;
    if (lastBracket != std::string::npos && (lastDot == std::string::npos || lastBracket > lastDot)) {
        // Last access is array index: var.subvar[2] or var[2]
        size_t openBracket = keyStr.find_last_of('[');
        size_t closeBracket = keyStr.find_last_of(']');
        
        if (openBracket != std::string::npos && closeBracket != std::string::npos && closeBracket > openBracket) {
            std::string parentPath = keyStr.substr(0, openBracket);
            std::string indexStr = keyStr.substr(openBracket + 1, closeBracket - openBracket - 1);
            
            try {
                arrayIndex = std::stoi(indexStr);
                if (parentPath.empty()) {
                    parent = &root;
                } else {
                    parent = Nebulite::Utility::RjDirectAccess::traverse_path(parentPath.c_str(), root);
                }
            } catch (...) {
                return nullptr; // Invalid index
            }
        }
    } else if (lastDot != std::string::npos) {
        // Last access is object member: var.subvar.finalkey
        std::string parentPath = keyStr.substr(0, lastDot);
        finalKey = keyStr.substr(lastDot + 1);
        parent = Nebulite::Utility::RjDirectAccess::traverse_path(parentPath.c_str(), root);
    }
    return parent;
}

void Nebulite::Utility::RjDirectAccess::remove_member(const char* key, rapidjson::Value& val) {
    std::string keyStr(key);
    
    // Handle simple case: direct member of root document
    if (keyStr.find('.') == std::string::npos && keyStr.find('[') == std::string::npos) {
        if (val.HasMember(key)) {
            val.RemoveMember(key);
        }
        return;
    }
    
    // For complex paths, find the parent and final key/index
    
    
    // Now we find the parent value to delete the child from:
    // - parent.child
    // - parent[index]
    // - parent.child[index]
    std::string finalKey;
    int arrayIndex = -1;
    rapidjson::Value* parent = Nebulite::Utility::RjDirectAccess::traverse_to_parent(key, val, finalKey, arrayIndex);
    
    // Remove the final key/index from parent
    if (parent != nullptr) {
        if (arrayIndex >= 0) {
            // Remove an array element
            if(finalKey != ""){
                parent[arrayIndex].RemoveMember(finalKey.c_str());
            }
            else if (parent->IsArray() && arrayIndex < static_cast<int>(parent->Size())) {
                parent->Erase(parent->Begin() + arrayIndex);
            }
        } else if (!finalKey.empty()) {
            // Remove object member
            if (parent->IsObject() && parent->HasMember(finalKey.c_str())) {
                parent->RemoveMember(finalKey.c_str());
            }
        }
    }
}

//------------------------------------------
// Static Private Helper Functions

std::string_view Nebulite::Utility::RjDirectAccess::extractKeyPart(std::string_view* keyView) {
    // Find '.' or '[' as next separators
    size_t dotPos = keyView->find('.');
    size_t bracketPos = keyView->find('[');

    size_t nextSep;
    if (dotPos == std::string_view::npos && bracketPos == std::string_view::npos) {
        nextSep = keyView->size();  // No separator - last key
    } else if (dotPos == std::string_view::npos) {
        nextSep = bracketPos;
    } else if (bracketPos == std::string_view::npos) {
        nextSep = dotPos;
    } else {
        nextSep = std::min(dotPos, bracketPos);
    }

    // Extract current key part (object key)
    std::string_view keyPart = keyView->substr(0, nextSep);
    keyView->remove_prefix(nextSep);
    return keyPart;
}