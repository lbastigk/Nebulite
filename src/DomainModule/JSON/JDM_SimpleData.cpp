#include "DomainModule/JSON/JDM_SimpleData.hpp"
#include "Utility/JSON.hpp"

namespace Nebulite::DomainModule::JSON {

//------------------------------------------
// Update
void SimpleData::update() {
    std::lock_guard<std::recursive_mutex> mtx = domain->lock(); // Lock the domain for thread-safe access
    // Add Domain-specific updates here!
    // General rule:
    // This is used to update all variables/states that are INTERNAL ONLY
}

//------------------------------------------
// Domain-Bound Functions

//------------------------------------------
// General set/get/remove functions

Nebulite::Constants::Error SimpleData::set(int argc, char* argv[]) {
    std::lock_guard<std::recursive_mutex> mtx = domain->lock(); // Lock the domain for thread-safe access
    if(argc < 3) {
        std::cerr << "Error: Too few arguments for set command." << std::endl;
        return Nebulite::Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }
    
    std::string key = argv[1];
    std::string value = argv[2];
    for (int i = 3; i < argc; ++i) {
        value += " " + std::string(argv[i]);
    }
    domain->set(key.c_str(), value);
    return Nebulite::Constants::ErrorTable::NONE();
}
const std::string SimpleData::set_name = "set";
const std::string SimpleData::set_desc = R"(Set a key to a value in the JSON document.

Usage: set <key> <value>

Note: All values are stored as strings.
)";

Nebulite::Constants::Error SimpleData::move(int argc, char* argv[]){
    std::lock_guard<std::recursive_mutex> mtx = domain->lock(); // Lock the domain for thread-safe access
    if (argc != 3) {
        std::cerr << "Error: Too few arguments for move command." << std::endl;
        return Nebulite::Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }

    std::string sourceKey = argv[1];
    std::string targetKey = argv[2];

    if(domain->memberCheck(sourceKey) == Nebulite::Utility::JSON::KeyType::null) {
        std::cerr << "Error: Source key '" << sourceKey << "' does not exist." << std::endl;
        return Nebulite::Constants::ErrorTable::FUNCTIONAL::UNKNOWN_ARG();
    }
    if(domain->memberCheck(sourceKey) == Nebulite::Utility::JSON::KeyType::document) {
        Nebulite::Utility::JSON subdoc = domain->get_subdoc(sourceKey.c_str());
        domain->remove_key(targetKey.c_str());
        domain->set_subdoc(targetKey.c_str(), &subdoc);
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
    return Nebulite::Constants::ErrorTable::NONE();
}
const std::string SimpleData::move_name = "move";
const std::string SimpleData::move_desc = R"(Move data from one key to another.

Usage: move <source_key> <destination_key>
)";

Nebulite::Constants::Error SimpleData::copy(int argc, char* argv[]){
    std::lock_guard<std::recursive_mutex> mtx = domain->lock(); // Lock the domain for thread-safe access
    if (argc != 3) {
        std::cerr << "Error: Too few arguments for copy command." << std::endl;
        return Nebulite::Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }

    std::string sourceKey = argv[1];
    std::string targetKey = argv[2];

    if(domain->memberCheck(sourceKey) == Nebulite::Utility::JSON::KeyType::null) {
        std::cerr << "Error: Source key '" << sourceKey << "' does not exist." << std::endl;
        return Nebulite::Constants::ErrorTable::FUNCTIONAL::UNKNOWN_ARG();
    }
    if(domain->memberCheck(sourceKey) == Nebulite::Utility::JSON::KeyType::document) {
        Nebulite::Utility::JSON subdoc = domain->get_subdoc(sourceKey.c_str());
        domain->remove_key(targetKey.c_str());
        domain->set_subdoc(targetKey.c_str(), &subdoc);
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
    return Nebulite::Constants::ErrorTable::NONE();
}
const std::string SimpleData::copy_name = "copy";
const std::string SimpleData::copy_desc = R"(Copy data from one key to another.

Usage: copy <source_key> <destination_key>
)";

Nebulite::Constants::Error SimpleData::keyDelete(int argc, char* argv[]){
    std::lock_guard<std::recursive_mutex> mtx = domain->lock(); // Lock the domain for thread-safe access
    if (argc != 2) {
        std::cerr << "Error: Too few arguments for delete command." << std::endl;
        return Nebulite::Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }
    std::string key = argv[1];
    domain->remove_key(key.c_str());
    return Nebulite::Constants::ErrorTable::NONE();
}
const std::string SimpleData::keyDelete_name = "keyDelete";
const std::string SimpleData::keyDelete_desc = R"(Delete a key from the JSON document.

Usage: keyDelete <key>
)";

//------------------------------------------
// Array manipulation functions
Nebulite::Constants::Error SimpleData::ensureArray(int argc, char* argv[]) {
    std::lock_guard<std::recursive_mutex> mtx = domain->lock(); // Lock the domain for thread-safe access
    if (argc < 2) {
        std::cerr << "Error: Too few arguments for ensureArray command." << std::endl;
        return Nebulite::Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }
    if (argc > 2) {
        std::cerr << "Error: Too many arguments for ensureArray command." << std::endl;
        return Nebulite::Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS();
    }

    std::string key = argv[1];

    Nebulite::Utility::JSON::KeyType keyType = domain->memberCheck(key);

    if (keyType == Nebulite::Utility::JSON::KeyType::array) {
        // Already an array, nothing to do
        return Nebulite::Constants::ErrorTable::NONE();
    }

    if(keyType == Nebulite::Utility::JSON::KeyType::value) {
        // pop out value
        std::string existingValue = domain->get<std::string>(key.c_str());
        domain->remove_key(key.c_str());

        // Set as new value
        std::string arrayKey = key + "[0]";
        domain->set(arrayKey.c_str(), existingValue);

        // All done
        return Nebulite::Constants::ErrorTable::NONE();
    }

    std::cerr << "Error: Key '" << key << "' is unsupported type " << static_cast<int>(keyType) << ", cannot convert to array." << std::endl;
    return Nebulite::Constants::ErrorTable::FUNCTIONAL::CRITICAL_FUNCTION_NOT_IMPLEMENTED();
    
}
const std::string SimpleData::ensureArray_name = "ensure-array";
const std::string SimpleData::ensureArray_desc = R"(Ensure that a key is an array, converting a value to an array if necessary.

Usage: ensure-array <key>
)";

Nebulite::Constants::Error SimpleData::push_back(int argc, char* argv[]){
    std::lock_guard<std::recursive_mutex> mtx = domain->lock(); // Lock the domain for thread-safe access
    if (argc > 3) {
        std::cerr << "Error: Too many arguments for push_front command." << std::endl;
        return Nebulite::Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS();
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
        std::string command = __FUNCTION__;
        command += " " + SimpleData::ensureArray_name;
        command += " " + key;
        Nebulite::Constants::Error result = domain->parseStr(command);
        if (result != Nebulite::Constants::ErrorTable::NONE()) {
            std::cerr << "Error: Failed to ensure array for key '" << key << "'." << std::endl;
            return result;
        }
    }

    size_t size = domain->memberSize(key);
    std::string itemKey = key + "[" + std::to_string(size) + "]";
    domain->set(itemKey.c_str(), value);
    return Nebulite::Constants::ErrorTable::NONE();
}
const std::string SimpleData::push_back_name = "push-back";
const std::string SimpleData::push_back_desc = R"(Push a value to the back of an array.

Usage: push-back <key> <value>
)";

Nebulite::Constants::Error SimpleData::pop_back(int argc, char* argv[]){
    std::lock_guard<std::recursive_mutex> mtx = domain->lock(); // Lock the domain for thread-safe access
    if (argc < 2) {
        std::cerr << "Error: Too few arguments for push_back command." << std::endl;
        return Nebulite::Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }
    if (argc > 2) {
        std::cerr << "Error: Too many arguments for push_back command." << std::endl;
        return Nebulite::Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS();
    }
    std::string key = argv[1];

    if (domain->memberCheck(key) != Nebulite::Utility::JSON::KeyType::array) {
        std::string command = __FUNCTION__;
        command += " " + SimpleData::ensureArray_name;
        command += " " + key;
        Nebulite::Constants::Error result = domain->parseStr(command);
        if (result != Nebulite::Constants::ErrorTable::NONE()) {
            std::cerr << "Error: Failed to ensure array for key '" << key << "'." << std::endl;
            return result;
        }
    }

    size_t size = domain->memberSize(key);
    if (size == 0) {
        // nothing to pop out, not seen as error
        return Nebulite::Constants::ErrorTable::NONE();
    }

    std::string itemKey = key + "[" + std::to_string(size-1) + "]";
    domain->remove_key(itemKey.c_str());
    return Nebulite::Constants::ErrorTable::NONE();
}
const std::string SimpleData::pop_back_name = "pop-back";
const std::string SimpleData::pop_back_desc = R"(Pop a value from the back of an array.

Usage: pop-back <key>
)";

Nebulite::Constants::Error SimpleData::push_front(int argc, char* argv[]){
    std::lock_guard<std::recursive_mutex> mtx = domain->lock(); // Lock the domain for thread-safe access
    if (argc > 3) {
        std::cerr << "Error: Too many arguments for push_front command." << std::endl;
        return Nebulite::Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS();
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
        std::string command = __FUNCTION__;
        command += " " + SimpleData::ensureArray_name;
        command += " " + key;
        Nebulite::Constants::Error result = domain->parseStr(command);
        if (result != Nebulite::Constants::ErrorTable::NONE()) {
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
            return Nebulite::Constants::ErrorTable::FUNCTIONAL::CRITICAL_FUNCTION_NOT_IMPLEMENTED();
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
    return Nebulite::Constants::ErrorTable::NONE();
}
const std::string SimpleData::push_front_name = "push-front";
const std::string SimpleData::push_front_desc = R"(Push a value to the front of an array.

Usage: push-front <key> <value>
)";

Nebulite::Constants::Error SimpleData::pop_front(int argc, char* argv[]){
    std::lock_guard<std::recursive_mutex> mtx = domain->lock(); // Lock the domain for thread-safe access
    if (argc < 2) {
        std::cerr << "Error: Too few arguments for pop_front command." << std::endl;
        return Nebulite::Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }
    if (argc > 2) {
        std::cerr << "Error: Too many arguments for pop_front command." << std::endl;
        return Nebulite::Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS();
    }
    std::string key = argv[1];

    if (domain->memberCheck(key) != Nebulite::Utility::JSON::KeyType::array) {
        std::string command = __FUNCTION__;
        command += " " + SimpleData::ensureArray_name;
        command += " " + key;
        Nebulite::Constants::Error result = domain->parseStr(command);
        if (result != Nebulite::Constants::ErrorTable::NONE()) {
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
            return Nebulite::Constants::ErrorTable::FUNCTIONAL::CRITICAL_FUNCTION_NOT_IMPLEMENTED();
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

    return Nebulite::Constants::ErrorTable::NONE();
}
const std::string SimpleData::pop_front_name = "pop-front";
const std::string SimpleData::pop_front_desc = R"(Pop a value from the front of an array.

Usage: pop-front <key>
)";

} // namespace Nebulite::DomainModule::JSON