#include "JTE_SimpleData.h"
#include "JSON.h"

//---------------------------------------
// General set/get/remove functions

Nebulite::ERROR_TYPE Nebulite::JSONTreeExpansion::SimpleData::set(int argc, char* argv[]) {
    if(argc < 3) {
        std::cerr << "Error: Too few arguments for set command." << std::endl;
        return Nebulite::ERROR_TYPE::TOO_FEW_ARGS;
    }
    
    std::string key = argv[1];
    std::string value = argv[2];
    for (int i = 3; i < argc; ++i) {
        value += " " + std::string(argv[i]);
    }
    self->set(key.c_str(), value);
    return Nebulite::ERROR_TYPE::NONE;
}

Nebulite::ERROR_TYPE Nebulite::JSONTreeExpansion::SimpleData::move(int argc, char* argv[]){
    if (argc != 3) {
        std::cerr << "Error: Too few arguments for move command." << std::endl;
        return Nebulite::ERROR_TYPE::TOO_FEW_ARGS;
    }

    std::string sourceKey = argv[1];
    std::string targetKey = argv[2];

    if(self->memberCheck(sourceKey) == Nebulite::JSON::KeyType::null) {
        std::cerr << "Error: Source key '" << sourceKey << "' does not exist." << std::endl;
        return Nebulite::ERROR_TYPE::UNKNOWN_ARG;
    }
    if(self->memberCheck(sourceKey) == Nebulite::JSON::KeyType::document) {
        Nebulite::JSON subdoc = self->get_subdoc(sourceKey.c_str());
        self->remove_key(targetKey.c_str());
        self->set_subdoc(targetKey.c_str(), subdoc);
        self->remove_key(sourceKey.c_str());
    }
    else if (self->memberCheck(sourceKey) == Nebulite::JSON::KeyType::array) {
        // Careful handling required:
        self->remove_key(targetKey.c_str());

        uint16_t size = self->memberSize(sourceKey);
        for (uint16_t i = 0; i < size; ++i) {
            std::string itemKey = sourceKey + "[" + std::to_string(i) + "]";
            std::string itemValue = self->get<std::string>(itemKey.c_str());
            std::string targetItemKey = targetKey + "[" + std::to_string(i) + "]";
            self->set(targetItemKey.c_str(), itemValue);
        }
    }
    else {
        // Move the value from sourceKey to targetKey
        std::string value = self->get<std::string>(sourceKey.c_str());
        self->remove_key(targetKey.c_str());
        self->set(targetKey.c_str(), value);
        self->remove_key(sourceKey.c_str());
    }
    return Nebulite::ERROR_TYPE::NONE;
}

Nebulite::ERROR_TYPE Nebulite::JSONTreeExpansion::SimpleData::copy(int argc, char* argv[]){
    if (argc != 3) {
        std::cerr << "Error: Too few arguments for move command." << std::endl;
        return Nebulite::ERROR_TYPE::TOO_FEW_ARGS;
    }

    std::string sourceKey = argv[1];
    std::string targetKey = argv[2];

    if(self->memberCheck(sourceKey) == Nebulite::JSON::KeyType::null) {
        std::cerr << "Error: Source key '" << sourceKey << "' does not exist." << std::endl;
        return Nebulite::ERROR_TYPE::UNKNOWN_ARG;
    }
    if(self->memberCheck(sourceKey) == Nebulite::JSON::KeyType::document) {
        Nebulite::JSON subdoc = self->get_subdoc(sourceKey.c_str());
        self->remove_key(targetKey.c_str());
        self->set_subdoc(targetKey.c_str(), subdoc);
    }
    else if (self->memberCheck(sourceKey) == Nebulite::JSON::KeyType::array) {
        // Careful handling required:
        self->remove_key(targetKey.c_str());

        uint16_t size = self->memberSize(sourceKey);
        for (uint16_t i = 0; i < size; ++i) {
            std::string itemKey = sourceKey + "[" + std::to_string(i) + "]";
            std::string itemValue = self->get<std::string>(itemKey.c_str());
            std::string targetItemKey = targetKey + "[" + std::to_string(i) + "]";
            self->set(targetItemKey.c_str(), itemValue);
        }
    }
    else {
        // Move the value from sourceKey to targetKey
        std::string value = self->get<std::string>(sourceKey.c_str());
        self->remove_key(targetKey.c_str());
        self->set(targetKey.c_str(), value);
    }
    return Nebulite::ERROR_TYPE::NONE;
}

Nebulite::ERROR_TYPE Nebulite::JSONTreeExpansion::SimpleData::keyDelete(int argc, char* argv[]){
    if (argc != 2) {
        std::cerr << "Error: Too few arguments for move command." << std::endl;
        return Nebulite::ERROR_TYPE::TOO_FEW_ARGS;
    }
    std::string key = argv[1];
    self->remove_key(key.c_str());
    return Nebulite::ERROR_TYPE::NONE;
}

//---------------------------------------
// Array manipulation functions

Nebulite::ERROR_TYPE Nebulite::JSONTreeExpansion::SimpleData::push_back(int argc, char* argv[]){
    if (argc > 3) {
        std::cerr << "Error: Too many arguments for push_front command." << std::endl;
        return Nebulite::ERROR_TYPE::TOO_MANY_ARGS;
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

    if (self->memberCheck(key) != Nebulite::JSON::KeyType::array) {
        std::cerr << "Error: Key '" << key << "' is not an array." << std::endl;
        // TODO: Get out value, make key an array, insert value as key[1]
        return Nebulite::ERROR_TYPE::FEATURE_NOT_IMPLEMENTED;
    }

    size_t size = self->memberSize(key);
    std::string itemKey = key + "[" + std::to_string(size) + "]";
    self->set(itemKey.c_str(), value);
    return Nebulite::ERROR_TYPE::NONE;
}

Nebulite::ERROR_TYPE Nebulite::JSONTreeExpansion::SimpleData::pop_back(int argc, char* argv[]){
    if (argc < 2) {
        std::cerr << "Error: Too few arguments for push_back command." << std::endl;
        return Nebulite::ERROR_TYPE::TOO_FEW_ARGS;
    }
    if (argc > 2) {
        std::cerr << "Error: Too many arguments for push_back command." << std::endl;
        return Nebulite::ERROR_TYPE::TOO_MANY_ARGS;
    }
    std::string key = argv[1];

    if (self->memberCheck(key) != Nebulite::JSON::KeyType::array) {
        std::cerr << "Error: Key '" << key << "' is not an array." << std::endl;
        // TODO: Get out value, make key an array, insert value as key[0]
        return Nebulite::ERROR_TYPE::FEATURE_NOT_IMPLEMENTED;
    }

    size_t size = self->memberSize(key);
    if (size == 0) {
        // nothing to pop out, not seen as error
        return Nebulite::ERROR_TYPE::NONE;
    }

    std::string itemKey = key + "[" + std::to_string(size-1) + "]";
    self->remove_key(itemKey.c_str());
    return Nebulite::ERROR_TYPE::NONE;
}

Nebulite::ERROR_TYPE Nebulite::JSONTreeExpansion::SimpleData::push_front(int argc, char* argv[]){
    if (argc > 3) {
        std::cerr << "Error: Too many arguments for push_front command." << std::endl;
        return Nebulite::ERROR_TYPE::TOO_MANY_ARGS;
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

    
    
    if (self->memberCheck(key) != Nebulite::JSON::KeyType::array) {
        std::cerr << "Error: Key '" << key << "' is not an array." << std::endl;
        // TODO: Get out value, make key an array, insert value as key[0]
        return Nebulite::ERROR_TYPE::FEATURE_NOT_IMPLEMENTED;
    }
    size_t size = self->memberSize(key);

    //-----------------------------------------
    // Security check:
    // if any array item is a document, throw error
    // This feature is yet to be implemented!
    for (size_t i = 0; i < size; ++i) {
        std::string itemKey = key + "[" + std::to_string(i) + "]";
        Nebulite::JSON::KeyType itemType = self->memberCheck(itemKey);
        if (itemType == Nebulite::JSON::KeyType::document) {
            std::cerr << "Error: Cannot push_front into an array containing documents." << std::endl;
            return Nebulite::ERROR_TYPE::FEATURE_NOT_IMPLEMENTED;
        }
    }

    //----------------------------------------
    // Move all existing items one step forward
    for (size_t i = size; i > 0; --i) {
        std::string itemKey = key + "[" + std::to_string(i - 1) + "]";
        std::string itemValue = self->get<std::string>(itemKey.c_str());
        std::string newItemKey = key + "[" + std::to_string(i) + "]";
        self->set(newItemKey.c_str(), itemValue);
    }
    std::string itemKey = key + "[0]";
    self->set(itemKey.c_str(), value);
    return Nebulite::ERROR_TYPE::NONE;
}

Nebulite::ERROR_TYPE Nebulite::JSONTreeExpansion::SimpleData::pop_front(int argc, char* argv[]){
    if (argc < 2) {
        std::cerr << "Error: Too few arguments for pop_front command." << std::endl;
        return Nebulite::ERROR_TYPE::TOO_FEW_ARGS;
    }
    if (argc > 2) {
        std::cerr << "Error: Too many arguments for pop_front command." << std::endl;
        return Nebulite::ERROR_TYPE::TOO_MANY_ARGS;
    }
    std::string key = argv[1];

    if (self->memberCheck(key) != Nebulite::JSON::KeyType::array) {
        std::cerr << "Error: Key '" << key << "' is not an array." << std::endl;
        // TODO: Get out value, make key an array, insert value as key[0]
        return Nebulite::ERROR_TYPE::FEATURE_NOT_IMPLEMENTED;
    }

    size_t size = self->memberSize(key);

    //-----------------------------------------
    // Security check:
    // if any array item is a document, throw error
    // This feature is yet to be implemented!
    for (size_t i = 0; i < size; ++i) {
        std::string itemKey = key + "[" + std::to_string(i) + "]";
        Nebulite::JSON::KeyType itemType = self->memberCheck(itemKey);
        if (itemType == Nebulite::JSON::KeyType::document) {
            std::cerr << "Error: Cannot push_front into an array containing documents." << std::endl;
            return Nebulite::ERROR_TYPE::FEATURE_NOT_IMPLEMENTED;
        }
    }

    //----------------------------------------
    // Move all existing items one step back
    for (size_t i = 1; i < size; i++) {
        std::string itemKey = key + "[" + std::to_string(i) + "]";
        std::string itemValue = self->get<std::string>(itemKey.c_str());
        std::string newItemKey = key + "[" + std::to_string(i - 1) + "]";
        self->set(newItemKey.c_str(), itemValue);
    }
    // Remove the last item
    std::string lastItemKey = key + "[" + std::to_string(size-1) + "]";
    self->remove_key(lastItemKey.c_str());

    return Nebulite::ERROR_TYPE::NONE;
}