#include "Expansion/JSON/JTE_ComplexData.h"
#include "Utility/JSON.h"

//-------------------------------
// Update
void Nebulite::Expansion::JSON::ComplexData::update() {
    // Add FuncTree-specific updates here!
    // General rule:
    // This is used to update all variables/states that are INTERNAL ONLY
}

//-------------------------------
// FuncTree-Bound Functions

Nebulite::Constants::ERROR_TYPE Nebulite::Expansion::JSON::ComplexData::set_from_query(int argc, char* argv[]){
    return Nebulite::Constants::ERROR_TYPE::CRITICAL_FUNCTION_NOT_IMPLEMENTED;
}
Nebulite::Constants::ERROR_TYPE Nebulite::Expansion::JSON::ComplexData::set_from_json(int argc, char* argv[]){
    // Since we have no access to the global space, we cant use the JSON doc cache
    // Instead, we manually load the document to retrieve the key
    if(argc < 3){
        std::cerr << "Error: Too few arguments for set-from-json command." << std::endl;
        return Nebulite::Constants::ERROR_TYPE::TOO_FEW_ARGS;
    }
    if(argc > 3){
        std::cerr << "Error: Too many arguments for set-from-json command." << std::endl;
        return Nebulite::Constants::ERROR_TYPE::TOO_MANY_ARGS;
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
        std::cerr << "Error: Could not load file from link: " << link << std::endl;
        return Nebulite::Constants::ERROR_TYPE::FILE_NOT_FOUND;
    }
    Nebulite::Utility::JSON jsonDoc;
    jsonDoc.deserialize(link.c_str());
    
    // Depending on the type of docKey, we retrieve the value
    Nebulite::Utility::JSON::KeyType type = jsonDoc.memberCheck(docKey.c_str());

    // === DOCUMENT ===
    if(type == Nebulite::Utility::JSON::KeyType::document){
        // Retrieve the sub-document
        Nebulite::Utility::JSON subdoc = jsonDoc.get_subdoc(docKey.c_str());

        // Set the sub-document in the current JSON tree
        domain->set_subdoc(myKey.c_str(), subdoc);
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


    return Nebulite::Constants::ERROR_TYPE::NONE;
}