#include "DomainModule/JSON/JSDM_ComplexData.hpp"
#include "Utility/JSON.hpp"

namespace Nebulite::DomainModule::JSON{

const std::string ComplexData::query_name = "query";
const std::string ComplexData::query_desc = R"(Functions to manipulate JSON data via SQL query results)";

const std::string ComplexData::json_name = "json";
const std::string ComplexData::json_desc = R"(Functions to manipulate JSON data via read-only JSON documents)";

//------------------------------------------
// Update
Nebulite::Constants::Error ComplexData::update() {
    // Add Domain-specific updates here!
    // General rule:
    // This is used to update all variables/states that are INTERNAL ONLY
    return Nebulite::Constants::ErrorTable::NONE();
}

//------------------------------------------
// Domain-Bound Functions

Nebulite::Constants::Error ComplexData::query_set(int argc, char* argv[]){
    std::lock_guard<std::recursive_mutex> mtx = domain->lock(); // Lock the domain for thread-safe access
    return Nebulite::Constants::ErrorTable::FUNCTIONAL::CRITICAL_FUNCTION_NOT_IMPLEMENTED();
}
const std::string ComplexData::query_set_name = "query set";
const std::string ComplexData::query_set_desc = R"(Sets a key from a SQL query result.
Not implemented yet.
)";

Nebulite::Constants::Error ComplexData::json_set(int argc, char* argv[]){
    std::lock_guard<std::recursive_mutex> mtx = domain->lock(); // Lock the domain for thread-safe access
    // Since we have no access to the global space, we cant use the JSON doc cache
    // Instead, we manually load the document to retrieve the key
    if(argc < 3){
        return Nebulite::Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }
    if(argc > 3){
        return Nebulite::Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS();
    }
    std::string myKey = argv[1];
    std::string link_and_key = argv[2];

    //Split link_and_key into link and key
    size_t colonPos = link_and_key.find(':');
    std::string link, docKey;
    if (colonPos == std::string::npos) {
        // Assuming the link is the whole string and key is empty
        link = link_and_key;
        docKey = "";
    } else {
        link = link_and_key.substr(0, colonPos);
        docKey = link_and_key.substr(colonPos + 1);
    }

    // Load the JSON document from the link
    std::string file = Nebulite::Utility::FileManagement::LoadFile(link);
    if(file.empty()){
        return Nebulite::Constants::ErrorTable::FILE::CRITICAL_INVALID_FILE();
    }
    Nebulite::Utility::JSON jsonDoc(global);
    jsonDoc.deserialize(link.c_str());
    
    // Depending on the type of docKey, we retrieve the value
    Nebulite::Utility::JSON::KeyType type = jsonDoc.memberCheck(docKey.c_str());

    // === DOCUMENT ===
    if(type == Nebulite::Utility::JSON::KeyType::document){
        // Retrieve the sub-document
        Nebulite::Utility::JSON subdoc = jsonDoc.get_subdoc(docKey.c_str());

        // Set the sub-document in the current JSON tree
        domain->set_subdoc(myKey.c_str(), &subdoc);
    }
    // === VALUE ===
    else if(type == Nebulite::Utility::JSON::KeyType::value){
        // Retrieve the value
        std::string value = jsonDoc.get<std::string>(docKey.c_str());

        // Set the value in the current JSON tree
        domain->set(myKey.c_str(), value);
    }
    // === ARRAY ===
    else if(type == Nebulite::Utility::JSON::KeyType::array){
        uint16_t size = jsonDoc.memberSize(docKey.c_str());
        for (uint16_t i = 0; i < size; ++i) {
            std::string itemKey = docKey + "[" + std::to_string(i) + "]";
            std::string itemValue = jsonDoc.get<std::string>(itemKey.c_str());
            std::string newItemKey = myKey + "[" + std::to_string(i) + "]";
            domain->set(newItemKey.c_str(), itemValue);
        }
    }


    return Nebulite::Constants::ErrorTable::NONE();
}
const std::string ComplexData::json_set_name = "json set";
const std::string ComplexData::json_set_desc = R"(Sets a key from a JSON document.

Usage: json set <key_to_set> <link:key>

Where <link:key> is a link to a JSON document.
The document is dynamically loaded and cached for future use.
)";

} // namespace Nebulite::DomainModule::JSON