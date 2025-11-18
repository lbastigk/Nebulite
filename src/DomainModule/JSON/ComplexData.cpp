#include "DomainModule/JSON/ComplexData.hpp"
#include "Utility/JSON.hpp"

#include "Nebulite.hpp"

namespace Nebulite::DomainModule::JSON{

std::string const ComplexData::query_name = "query";
std::string const ComplexData::query_desc = R"(Functions to manipulate JSON data via SQL query results)";

std::string const ComplexData::json_name = "json";
std::string const ComplexData::json_desc = R"(Functions to manipulate JSON data via read-only JSON documents)";

//------------------------------------------
// Update
Constants::Error ComplexData::update(){
    // Add Domain-specific updates here!
    // General rule:
    // This is used to update all variables/states that are INTERNAL ONLY
    return Constants::ErrorTable::NONE();
}

//------------------------------------------
// Domain-Bound Functions

// NOLINTNEXTLINE
Constants::Error ComplexData::querySet(int argc,  char** argv){
    std::scoped_lock<std::recursive_mutex> mtx = domain->lock(); // Lock the domain for thread-safe access
    return Constants::ErrorTable::FUNCTIONAL::CRITICAL_FUNCTION_NOT_IMPLEMENTED();
}
std::string const ComplexData::querySet_name = "query set";
std::string const ComplexData::querySet_desc = R"(Sets a key from a SQL query result.
Not implemented yet.
)";

// NOLINTNEXTLINE
Constants::Error ComplexData::jsonSet(int argc,  char** argv){
    std::scoped_lock<std::recursive_mutex> mtx = domain->lock(); // Lock the domain for thread-safe access
    // Since we have no access to the global space, we cant use the JSON doc cache
    // Instead, we manually load the document to retrieve the key
    if(argc < 3){
        return Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }
    if(argc > 3){
        return Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS();
    }
    std::string const myKey = argv[1];
    std::string const docKey = argv[2];
    
    // Depending on the type of docKey, we retrieve the value

    // === DOCUMENT ===
    if(Utility::JSON::KeyType const type = Nebulite::global().getDocCache()->memberCheck(docKey); type == Utility::JSON::KeyType::document){
        // Retrieve the sub-document
        Utility::JSON subDoc = Nebulite::global().getDocCache()->getSubDoc(docKey);

        // Set the sub-document in the current JSON tree
        domain->setSubDoc(myKey.c_str(), &subDoc);
    }
    // === VALUE ===
    else if(type == Utility::JSON::KeyType::value){
        domain->set(myKey, Nebulite::global().getDocCache()->get<std::string>(docKey));
    }
    // === ARRAY ===
    else if(type == Utility::JSON::KeyType::array){
        size_t const size = Nebulite::global().getDocCache()->memberSize(docKey);
        for (size_t i = 0; i < size; ++i){
            std::string itemKey = docKey + "[" + std::to_string(i) + "]";
            auto itemValue = Nebulite::global().getDocCache()->get<std::string>(itemKey);
            std::string newItemKey = myKey + "[" + std::to_string(i) + "]";
            domain->set(newItemKey, itemValue);
        }
    }
    return Constants::ErrorTable::NONE();
}
std::string const ComplexData::jsonSet_name = "json set";
std::string const ComplexData::jsonSet_desc = R"(Sets a key from a JSON document.

Usage: json set <key_to_set> <link:key>

Where <link:key> is a link to a JSON document.
The document is dynamically loaded and cached for future use.
)";

} // namespace Nebulite::DomainModule::JSON