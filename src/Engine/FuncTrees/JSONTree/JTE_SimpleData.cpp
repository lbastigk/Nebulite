#include "JTE_SimpleData.h"
#include "JSON.h"

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



