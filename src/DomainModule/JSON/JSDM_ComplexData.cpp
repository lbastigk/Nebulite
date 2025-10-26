#include "DomainModule/JSON/JSDM_ComplexData.hpp"
#include "Utility/JSON.hpp"
#include "Core/GlobalSpace.hpp"

namespace Nebulite::DomainModule::JSON{

std::string const ComplexData::query_name = "query";
std::string const ComplexData::query_desc = R"(Functions to manipulate JSON data via SQL query results)";

std::string const ComplexData::json_name = "json";
std::string const ComplexData::json_desc = R"(Functions to manipulate JSON data via read-only JSON documents)";

//------------------------------------------
// Update
Nebulite::Constants::Error ComplexData::update(){
    // Add Domain-specific updates here!
    // General rule:
    // This is used to update all variables/states that are INTERNAL ONLY
    return Nebulite::Constants::ErrorTable::NONE();
}

//------------------------------------------
// Domain-Bound Functions

Nebulite::Constants::Error ComplexData::query_set(int argc,  char* argv[]){
    std::lock_guard<std::recursive_mutex> mtx = domain->lock(); // Lock the domain for thread-safe access
    return Nebulite::Constants::ErrorTable::FUNCTIONAL::CRITICAL_FUNCTION_NOT_IMPLEMENTED();
}
std::string const ComplexData::query_set_name = "query set";
std::string const ComplexData::query_set_desc = R"(Sets a key from a SQL query result.
Not implemented yet.
)";

Nebulite::Constants::Error ComplexData::json_set(int argc,  char* argv[]){
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
    std::string doc_key = argv[2];
    
    // Depending on the type of docKey, we retrieve the value
    Nebulite::Utility::JSON::KeyType type = global->getDocCache()->memberCheck(doc_key);

    // === DOCUMENT ===
    if(type == Nebulite::Utility::JSON::KeyType::document){
        // Retrieve the sub-document
        Nebulite::Utility::JSON subdoc = global->getDocCache()->get_subdoc(doc_key);

        // Set the sub-document in the current JSON tree
        domain->set_subdoc(myKey.c_str(), &subdoc);
    }
    // === VALUE ===
    else if(type == Nebulite::Utility::JSON::KeyType::value){
        // Retrieve the value
        std::string value = global->getDocCache()->get<std::string>(doc_key);

        // Set the value in the current JSON tree
        domain->set(myKey.c_str(), value);
    }
    // === ARRAY ===
    else if(type == Nebulite::Utility::JSON::KeyType::array){
        uint16_t size = global->getDocCache()->memberSize(doc_key);
        for (uint16_t i = 0; i < size; ++i){
            std::string itemKey = doc_key + "[" + std::to_string(i) + "]";
            std::string itemValue = global->getDocCache()->get<std::string>(itemKey);
            std::string newItemKey = myKey + "[" + std::to_string(i) + "]";
            domain->set(newItemKey.c_str(), itemValue);
        }
    }
    return Nebulite::Constants::ErrorTable::NONE();
}
std::string const ComplexData::json_set_name = "json set";
std::string const ComplexData::json_set_desc = R"(Sets a key from a JSON document.

Usage: json set <key_to_set> <link:key>

Where <link:key> is a link to a JSON document.
The document is dynamically loaded and cached for future use.
)";

} // namespace Nebulite::DomainModule::JSON