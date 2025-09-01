#include "DomainModule/JSON/JDM_SimpleData.h"
#include "Utility/JSON.h"

//------------------------------------------
// Update
void Nebulite::DomainModule::JSON::SimpleData::update() {
    // Add FuncTree-specific updates here!
    // General rule:
    // This is used to update all variables/states that are INTERNAL ONLY
}

//------------------------------------------
// FuncTree-Bound Functions

//------------------------------------------
// General set/get/remove functions

Nebulite::Constants::ERROR_TYPE Nebulite::DomainModule::JSON::SimpleData::set(int argc, char* argv[]) {
    if(argc < 3) {
        std::cerr << "Error: Too few arguments for set command." << std::endl;
        return Nebulite::Constants::ERROR_TYPE::TOO_FEW_ARGS;
    }
    
    std::string key = argv[1];
    std::string value = argv[2];
    for (int i = 3; i < argc; ++i) {
        value += " " + std::string(argv[i]);
    }
    domain->set(key.c_str(), value);
    return Nebulite::Constants::ERROR_TYPE::NONE;
}

Nebulite::Constants::ERROR_TYPE Nebulite::DomainModule::JSON::SimpleData::move(int argc, char* argv[]){
    if (argc != 3) {
        std::cerr << "Error: Too few arguments for move command." << std::endl;
        return Nebulite::Constants::ERROR_TYPE::TOO_FEW_ARGS;
    }

    std::string sourceKey = argv[1];
    std::string targetKey = argv[2];

    if(domain->memberCheck(sourceKey) == Nebulite::Utility::JSON::KeyType::null) {
        std::cerr << "Error: Source key '" << sourceKey << "' does not exist." << std::endl;
        return Nebulite::Constants::ERROR_TYPE::UNKNOWN_ARG;
    }
    if(domain->memberCheck(sourceKey) == Nebulite::Utility::JSON::KeyType::document) {
        Nebulite::Utility::JSON subdoc = domain->get_subdoc(sourceKey.c_str());
        domain->remove_key(targetKey.c_str());
        domain->set_subdoc(targetKey.c_str(), subdoc);
        domain->remove_key(sourceKey.c_str());
    }
    else if (domain->memberCheck(sourceKey) == Nebulite::Utility::JSON::KeyType::array) {
        // Careful handling required:
        domain->remove_key(targetKey.c_str());

        uint16_t size = domain->memberSize(sourceKey);
        for (uint16_t i = 0; i < size; ++i) {
            std::string itemKey = sourceKey + "[" + std::to_string(i) + "]";
            std::string itemValue = domain->get<std::string>(itemKey.c_str());
            std::string targetItemKey = targetKey + "[" + std::to_string(i) + "]";
            domain->set(targetItemKey.c_str(), itemValue);
        }
    }
    else {
        // Move the value from sourceKey to targetKey
        std::string value = domain->get<std::string>(sourceKey.c_str());
        domain->remove_key(targetKey.c_str());
        domain->set(targetKey.c_str(), value);
        domain->remove_key(sourceKey.c_str());
    }
    return Nebulite::Constants::ERROR_TYPE::NONE;
}

Nebulite::Constants::ERROR_TYPE Nebulite::DomainModule::JSON::SimpleData::copy(int argc, char* argv[]){
    if (argc != 3) {
        std::cerr << "Error: Too few arguments for move command." << std::endl;
        return Nebulite::Constants::ERROR_TYPE::TOO_FEW_ARGS;
    }

    std::string sourceKey = argv[1];
    std::string targetKey = argv[2];

    if(domain->memberCheck(sourceKey) == Nebulite::Utility::JSON::KeyType::null) {
        std::cerr << "Error: Source key '" << sourceKey << "' does not exist." << std::endl;
        return Nebulite::Constants::ERROR_TYPE::UNKNOWN_ARG;
    }
    if(domain->memberCheck(sourceKey) == Nebulite::Utility::JSON::KeyType::document) {
        Nebulite::Utility::JSON subdoc = domain->get_subdoc(sourceKey.c_str());
        domain->remove_key(targetKey.c_str());
        domain->set_subdoc(targetKey.c_str(), subdoc);
    }
    else if (domain->memberCheck(sourceKey) == Nebulite::Utility::JSON::KeyType::array) {
        // Careful handling required:
        domain->remove_key(targetKey.c_str());

        uint16_t size = domain->memberSize(sourceKey);
        for (uint16_t i = 0; i < size; ++i) {
            std::string itemKey = sourceKey + "[" + std::to_string(i) + "]";
            std::string itemValue = domain->get<std::string>(itemKey.c_str());
            std::string targetItemKey = targetKey + "[" + std::to_string(i) + "]";
            domain->set(targetItemKey.c_str(), itemValue);
        }
    }
    else {
        // Move the value from sourceKey to targetKey
        std::string value = domain->get<std::string>(sourceKey.c_str());
        domain->remove_key(targetKey.c_str());
        domain->set(targetKey.c_str(), value);
    }
    return Nebulite::Constants::ERROR_TYPE::NONE;
}

Nebulite::Constants::ERROR_TYPE Nebulite::DomainModule::JSON::SimpleData::keyDelete(int argc, char* argv[]){
    if (argc != 2) {
        std::cerr << "Error: Too few arguments for move command." << std::endl;
        return Nebulite::Constants::ERROR_TYPE::TOO_FEW_ARGS;
    }
    std::string key = argv[1];
    domain->remove_key(key.c_str());
    return Nebulite::Constants::ERROR_TYPE::NONE;
}

//------------------------------------------
// Array manipulation functions
Nebulite::Constants::ERROR_TYPE Nebulite::DomainModule::JSON::SimpleData::ensureArray(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Error: Too few arguments for ensureArray command." << std::endl;
        return Nebulite::Constants::ERROR_TYPE::TOO_FEW_ARGS;
    }
    if (argc > 2) {
        std::cerr << "Error: Too many arguments for ensureArray command." << std::endl;
        return Nebulite::Constants::ERROR_TYPE::TOO_MANY_ARGS;
    }

    std::string key = argv[1];

    Nebulite::Utility::JSON::KeyType keyType = domain->memberCheck(key);

    if (keyType == Nebulite::Utility::JSON::KeyType::array) {
        // Already an array, nothing to do
        return Nebulite::Constants::ERROR_TYPE::NONE;
    }

    if(keyType == Nebulite::Utility::JSON::KeyType::value) {
        // pop out value
        std::string existingValue = domain->get<std::string>(key.c_str());
        domain->remove_key(key.c_str());

        // Set as new value
        std::string arrayKey = key + "[0]";
        domain->set(arrayKey.c_str(), existingValue);

        // All done
        return Nebulite::Constants::ERROR_TYPE::NONE;
    }

    std::cerr << "Error: Key '" << key << "' is unsupported type " << static_cast<int>(keyType) << ", cannot convert to array." << std::endl;
    return Nebulite::Constants::ERROR_TYPE::FEATURE_NOT_IMPLEMENTED;
    
}

Nebulite::Constants::ERROR_TYPE Nebulite::DomainModule::JSON::SimpleData::push_back(int argc, char* argv[]){
    if (argc > 3) {
        std::cerr << "Error: Too many arguments for push_front command." << std::endl;
        return Nebulite::Constants::ERROR_TYPE::TOO_MANY_ARGS;
    }
    std::string key = argv[1];
    std::string value;
    bool pushEmptyValue = (argc < 3);
    if(pushEmptyValue){
        // Usually, this would be "{}" but we use an empty string for simplicity
        value = "";
    }
    else {
        value = argv[2];
    }

    if (domain->memberCheck(key) != Nebulite::Utility::JSON::KeyType::array) {
        Nebulite::Constants::ERROR_TYPE result = funcTree->parseStr(std::string("Nebulite::DomainModule::JSON::SimpleData::push_back ensure-array " + key));
        if (result != Nebulite::Constants::ERROR_TYPE::NONE) {
            std::cerr << "Error: Failed to ensure array for key '" << key << "'." << std::endl;
            return result;
        }
    }

    size_t size = domain->memberSize(key);
    std::string itemKey = key + "[" + std::to_string(size) + "]";
    domain->set(itemKey.c_str(), value);
    return Nebulite::Constants::ERROR_TYPE::NONE;
}

Nebulite::Constants::ERROR_TYPE Nebulite::DomainModule::JSON::SimpleData::pop_back(int argc, char* argv[]){
    if (argc < 2) {
        std::cerr << "Error: Too few arguments for push_back command." << std::endl;
        return Nebulite::Constants::ERROR_TYPE::TOO_FEW_ARGS;
    }
    if (argc > 2) {
        std::cerr << "Error: Too many arguments for push_back command." << std::endl;
        return Nebulite::Constants::ERROR_TYPE::TOO_MANY_ARGS;
    }
    std::string key = argv[1];

    if (domain->memberCheck(key) != Nebulite::Utility::JSON::KeyType::array) {
        Nebulite::Constants::ERROR_TYPE result = funcTree->parseStr(std::string("Nebulite::DomainModule::JSON::SimpleData::pop_back ensure-array " + key));
        if (result != Nebulite::Constants::ERROR_TYPE::NONE) {
            std::cerr << "Error: Failed to ensure array for key '" << key << "'." << std::endl;
            return result;
        }
    }

    size_t size = domain->memberSize(key);
    if (size == 0) {
        // nothing to pop out, not seen as error
        return Nebulite::Constants::ERROR_TYPE::NONE;
    }

    std::string itemKey = key + "[" + std::to_string(size-1) + "]";
    domain->remove_key(itemKey.c_str());
    return Nebulite::Constants::ERROR_TYPE::NONE;
}

Nebulite::Constants::ERROR_TYPE Nebulite::DomainModule::JSON::SimpleData::push_front(int argc, char* argv[]){
    if (argc > 3) {
        std::cerr << "Error: Too many arguments for push_front command." << std::endl;
        return Nebulite::Constants::ERROR_TYPE::TOO_MANY_ARGS;
    }
    std::string key = argv[1];
    std::string value;
    bool pushEmptyValue = (argc < 3);
    if(pushEmptyValue){
        // Usually, this would be "{}" but we use an empty string for simplicity
        value = "";
    }
    else {
        value = argv[2];
    }

    
    if (domain->memberCheck(key) != Nebulite::Utility::JSON::KeyType::array) {
        Nebulite::Constants::ERROR_TYPE result = funcTree->parseStr(std::string("Nebulite::DomainModule::JSON::SimpleData::push_front ensure-array " + key));
        if (result != Nebulite::Constants::ERROR_TYPE::NONE) {
            std::cerr << "Error: Failed to ensure array for key '" << key << "'." << std::endl;
            return result;
        }
    }

    size_t size = domain->memberSize(key);

    //------------------------------------------
    // Security check:
    // if any array item is a document, throw error
    // This feature is yet to be implemented!
    for (size_t i = 0; i < size; ++i) {
        std::string itemKey = key + "[" + std::to_string(i) + "]";
        Nebulite::Utility::JSON::KeyType itemType = domain->memberCheck(itemKey);
        if (itemType == Nebulite::Utility::JSON::KeyType::document) {
            std::cerr << "Error: Cannot push_front into an array containing documents." << std::endl;
            return Nebulite::Constants::ERROR_TYPE::FEATURE_NOT_IMPLEMENTED;
        }
    }

    //------------------------------------------
    // Move all existing items one step forward
    for (size_t i = size; i > 0; --i) {
        std::string itemKey = key + "[" + std::to_string(i - 1) + "]";
        std::string itemValue = domain->get<std::string>(itemKey.c_str());
        std::string newItemKey = key + "[" + std::to_string(i) + "]";
        domain->set(newItemKey.c_str(), itemValue);
    }
    std::string itemKey = key + "[0]";
    domain->set(itemKey.c_str(), value);
    return Nebulite::Constants::ERROR_TYPE::NONE;
}

Nebulite::Constants::ERROR_TYPE Nebulite::DomainModule::JSON::SimpleData::pop_front(int argc, char* argv[]){
    if (argc < 2) {
        std::cerr << "Error: Too few arguments for pop_front command." << std::endl;
        return Nebulite::Constants::ERROR_TYPE::TOO_FEW_ARGS;
    }
    if (argc > 2) {
        std::cerr << "Error: Too many arguments for pop_front command." << std::endl;
        return Nebulite::Constants::ERROR_TYPE::TOO_MANY_ARGS;
    }
    std::string key = argv[1];

    if (domain->memberCheck(key) != Nebulite::Utility::JSON::KeyType::array) {
        Nebulite::Constants::ERROR_TYPE result = funcTree->parseStr(std::string("Nebulite::DomainModule::JSON::SimpleData::pop_front ensure-array " + key));
        if (result != Nebulite::Constants::ERROR_TYPE::NONE) {
            std::cerr << "Error: Failed to ensure array for key '" << key << "'." << std::endl;
            return result;
        }
    }

    size_t size = domain->memberSize(key);

    //------------------------------------------
    // Security check:
    // if any array item is a document, throw error
    // This feature is yet to be implemented!
    for (size_t i = 0; i < size; ++i) {
        std::string itemKey = key + "[" + std::to_string(i) + "]";
        Nebulite::Utility::JSON::KeyType itemType = domain->memberCheck(itemKey);
        if (itemType == Nebulite::Utility::JSON::KeyType::document) {
            std::cerr << "Error: Cannot push_front into an array containing documents." << std::endl;
            return Nebulite::Constants::ERROR_TYPE::FEATURE_NOT_IMPLEMENTED;
        }
    }

    //------------------------------------------
    // Move all existing items one step back
    for (size_t i = 1; i < size; i++) {
        std::string itemKey = key + "[" + std::to_string(i) + "]";
        std::string itemValue = domain->get<std::string>(itemKey.c_str());
        std::string newItemKey = key + "[" + std::to_string(i - 1) + "]";
        domain->set(newItemKey.c_str(), itemValue);
    }
    // Remove the last item
    std::string lastItemKey = key + "[" + std::to_string(size-1) + "]";
    domain->remove_key(lastItemKey.c_str());

    return Nebulite::Constants::ERROR_TYPE::NONE;
}