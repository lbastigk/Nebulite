#include "Nebulite.hpp"
#include "Data/Document/JSON.hpp"
#include "DomainModule/JSON/SimpleData.hpp"

namespace Nebulite::DomainModule::JSON {

//------------------------------------------
// Update
Constants::Error SimpleData::update() {
    std::scoped_lock<std::recursive_mutex> mtx = domain->lock(); // Lock the domain for thread-safe access
    // Add Domain-specific updates here!
    // General rule:
    // This is used to update all variables/states that are INTERNAL ONLY
    return Constants::ErrorTable::NONE();
}

//------------------------------------------
// Domain-Bound Functions

//------------------------------------------
// General set/get/remove functions

Constants::Error SimpleData::set(int argc, char** argv) {
    std::scoped_lock<std::recursive_mutex> mtx = domain->lock(); // Lock the domain for thread-safe access
    if (argc < 3) {
        Nebulite::cerr() << "Error: Too few arguments for set command." << Nebulite::endl;
        return Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }

    std::string const key = argv[1];
    std::string value = argv[2];
    for (int i = 3; i < argc; ++i) {
        value += " " + std::string(argv[i]);
    }
    domain->set(key, value);
    return Constants::ErrorTable::NONE();
}

Constants::Error SimpleData::move(int argc, char** argv) {
    std::scoped_lock<std::recursive_mutex> mtx = domain->lock(); // Lock the domain for thread-safe access
    if (argc != 3) {
        Nebulite::cerr() << "Error: Too few arguments for move command." << Nebulite::endl;
        return Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }

    std::string const sourceKey = argv[1];
    std::string const targetKey = argv[2];

    if (domain->memberType(sourceKey) == Data::JSON::KeyType::null) {
        Nebulite::cerr() << "Error: Source key '" << sourceKey << "' does not exist." << Nebulite::endl;
        return Constants::ErrorTable::FUNCTIONAL::UNKNOWN_ARG();
    }
    if (domain->memberType(sourceKey) == Data::JSON::KeyType::object) {
        Data::JSON subDoc = domain->getSubDoc(sourceKey);
        domain->removeKey(targetKey.c_str());
        domain->setSubDoc(targetKey.c_str(), subDoc);
        domain->removeKey(sourceKey.c_str());
    } else if (domain->memberType(sourceKey) == Data::JSON::KeyType::array) {
        // Careful handling required:
        domain->removeKey(targetKey.c_str());

        size_t const size = domain->memberSize(sourceKey);
        for (size_t i = 0; i < size; ++i) {
            std::string itemKey = sourceKey + "[" + std::to_string(i) + "]";
            auto itemValue = domain->get<std::string>(itemKey);
            std::string targetItemKey = targetKey + "[" + std::to_string(i) + "]";
            domain->set(targetItemKey, itemValue);
        }
    } else {
        // Move the value from sourceKey to targetKey
        auto const value = domain->get<std::string>(sourceKey);
        domain->removeKey(targetKey.c_str());
        domain->set(targetKey, value);
        domain->removeKey(sourceKey.c_str());
    }
    return Constants::ErrorTable::NONE();
}

Constants::Error SimpleData::copy(int argc, char** argv) {
    std::scoped_lock<std::recursive_mutex> mtx = domain->lock(); // Lock the domain for thread-safe access
    if (argc != 3) {
        Nebulite::cerr() << "Error: Too few arguments for copy command." << Nebulite::endl;
        return Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }

    std::string const sourceKey = argv[1];
    std::string const targetKey = argv[2];

    if (domain->memberType(sourceKey) == Data::JSON::KeyType::null) {
        Nebulite::cerr() << "Error: Source key '" << sourceKey << "' does not exist." << Nebulite::endl;
        return Constants::ErrorTable::FUNCTIONAL::UNKNOWN_ARG();
    }
    if (domain->memberType(sourceKey) == Data::JSON::KeyType::object) {
        Data::JSON subDoc = domain->getSubDoc(sourceKey);
        domain->removeKey(targetKey.c_str());
        domain->setSubDoc(targetKey.c_str(), subDoc);
    } else if (domain->memberType(sourceKey) == Data::JSON::KeyType::array) {
        // Careful handling required:
        domain->removeKey(targetKey.c_str());

        size_t const size = domain->memberSize(sourceKey);
        for (size_t i = 0; i < size; ++i) {
            std::string itemKey = sourceKey + "[" + std::to_string(i) + "]";
            auto itemValue = domain->get<std::string>(itemKey);
            std::string targetItemKey = targetKey + "[" + std::to_string(i) + "]";
            domain->set(targetItemKey, itemValue);
        }
    } else {
        // Move the value from sourceKey to targetKey
        auto const value = domain->get<std::string>(sourceKey);
        domain->removeKey(targetKey.c_str());
        domain->set(targetKey, value);
    }
    return Constants::ErrorTable::NONE();
}

Constants::Error SimpleData::keyDelete(int argc, char** argv) {
    std::scoped_lock<std::recursive_mutex> mtx = domain->lock(); // Lock the domain for thread-safe access
    if (argc != 2) {
        Nebulite::cerr() << "Error: Too few arguments for delete command." << Nebulite::endl;
        return Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }
    std::string const key = argv[1];
    domain->removeKey(key.c_str());
    return Constants::ErrorTable::NONE();
}

//------------------------------------------
// Array manipulation functions

Constants::Error SimpleData::ensureArray(int argc, char** argv) {
    std::scoped_lock<std::recursive_mutex> mtx = domain->lock(); // Lock the domain for thread-safe access
    if (argc < 2) {
        Nebulite::cerr() << "Error: Too few arguments for ensureArray command." << Nebulite::endl;
        return Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }
    if (argc > 2) {
        Nebulite::cerr() << "Error: Too many arguments for ensureArray command." << Nebulite::endl;
        return Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS();
    }

    std::string const key = argv[1];

    Data::JSON::KeyType keyType = domain->memberType(key);

    if (keyType == Data::JSON::KeyType::array) {
        // Already an array, nothing to do
        return Constants::ErrorTable::NONE();
    }

    if (keyType == Data::JSON::KeyType::value) {
        // pop out value
        auto const existingValue = domain->get<std::string>(key);
        domain->removeKey(key.c_str());

        // Set as new value
        std::string const arrayKey = key + "[0]";
        domain->set(arrayKey, existingValue);

        // All done
        return Constants::ErrorTable::NONE();
    }

    Nebulite::cerr() << "Error: Key '" << key << "' is unsupported type " << static_cast<int>(keyType) << ", cannot convert to array." << Nebulite::endl;
    return Constants::ErrorTable::FUNCTIONAL::CRITICAL_FUNCTION_NOT_IMPLEMENTED();
}

Constants::Error SimpleData::push_back(int argc, char** argv) {
    std::scoped_lock<std::recursive_mutex> mtx = domain->lock(); // Lock the domain for thread-safe access
    if (argc > 3) {
        Nebulite::cerr() << "Error: Too many arguments for push_front command." << Nebulite::endl;
        return Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS();
    }
    std::string const key = argv[1];
    std::string value;
    if (argc < 3) {
        // Trying to push an empty value
        // Usually, this would be "{}" but we use an empty string for simplicity
        value = "";
    } else {
        value = argv[2];
    }

    if (domain->memberType(key) != Data::JSON::KeyType::array) {
        std::string command = __FUNCTION__;
        command += " " + std::string(ensureArray_name);
        command += " " + key;
        if (Constants::Error const result = domain->parseStr(command); result != Constants::ErrorTable::NONE()) {
            Nebulite::cerr() << "Error: Failed to ensure array for key '" << key << "'." << Nebulite::endl;
            return result;
        }
    }

    size_t const size = domain->memberSize(key);
    std::string const itemKey = key + "[" + std::to_string(size) + "]";
    domain->set(itemKey, value);
    return Constants::ErrorTable::NONE();
}

Constants::Error SimpleData::pop_back(int argc, char** argv) {
    std::scoped_lock<std::recursive_mutex> mtx = domain->lock(); // Lock the domain for thread-safe access
    if (argc < 2) {
        Nebulite::cerr() << "Error: Too few arguments for push_back command." << Nebulite::endl;
        return Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }
    if (argc > 2) {
        Nebulite::cerr() << "Error: Too many arguments for push_back command." << Nebulite::endl;
        return Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS();
    }
    std::string const key = argv[1];

    if (domain->memberType(key) != Data::JSON::KeyType::array) {
        std::string command = __FUNCTION__;
        command += " " + std::string(ensureArray_name);
        command += " " + key;
        if (Constants::Error const result = domain->parseStr(command); result != Constants::ErrorTable::NONE()) {
            Nebulite::cerr() << "Error: Failed to ensure array for key '" << key << "'." << Nebulite::endl;
            return result;
        }
    }

    size_t const size = domain->memberSize(key);
    if (size == 0) {
        // nothing to pop out, not seen as error
        return Constants::ErrorTable::NONE();
    }

    std::string const itemKey = key + "[" + std::to_string(size - 1) + "]";
    domain->removeKey(itemKey.c_str());
    return Constants::ErrorTable::NONE();
}

Constants::Error SimpleData::push_front(int argc, char** argv) {
    std::scoped_lock<std::recursive_mutex> mtx = domain->lock(); // Lock the domain for thread-safe access
    if (argc > 3) {
        Nebulite::cerr() << "Error: Too many arguments for push_front command." << Nebulite::endl;
        return Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS();
    }
    std::string const key = argv[1];
    std::string value;
    if (argc < 3) {
        // Trying to push an empty value
        // Usually, this would be "{}" but we use an empty string for simplicity
        value = "";
    } else {
        value = argv[2];
    }

    if (domain->memberType(key) != Data::JSON::KeyType::array) {
        std::string command = __FUNCTION__;
        command += " " + std::string(ensureArray_name);
        command += " " + key;
        if (Constants::Error const result = domain->parseStr(command); result != Constants::ErrorTable::NONE()) {
            Nebulite::cerr() << "Error: Failed to ensure array for key '" << key << "'." << Nebulite::endl;
            return result;
        }
    }

    size_t const size = domain->memberSize(key);

    //------------------------------------------
    // Security check:
    // if any array item is a document, throw error
    // This feature is yet to be implemented!
    for (size_t i = 0; i < size; ++i) {
        std::string itemKey = key + "[" + std::to_string(i) + "]";
        if (Data::JSON::KeyType const itemType = domain->memberType(itemKey); itemType == Data::JSON::KeyType::object) {
            Nebulite::cerr() << "Error: Cannot push_front into an array containing documents." << Nebulite::endl;
            return Constants::ErrorTable::FUNCTIONAL::CRITICAL_FUNCTION_NOT_IMPLEMENTED();
        }
    }

    //------------------------------------------
    // Move all existing items one step forward
    for (size_t i = size; i > 0; --i) {
        std::string itemKey = key + "[" + std::to_string(i - 1) + "]";
        auto itemValue = domain->get<std::string>(itemKey);
        std::string newItemKey = key + "[" + std::to_string(i) + "]";
        domain->set(newItemKey, itemValue);
    }
    std::string const itemKey = key + "[0]";
    domain->set(itemKey, value);
    return Constants::ErrorTable::NONE();
}

Constants::Error SimpleData::pop_front(int argc, char** argv) {
    std::scoped_lock<std::recursive_mutex> mtx = domain->lock(); // Lock the domain for thread-safe access
    if (argc < 2) {
        Nebulite::cerr() << "Error: Too few arguments for pop_front command." << Nebulite::endl;
        return Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }
    if (argc > 2) {
        Nebulite::cerr() << "Error: Too many arguments for pop_front command." << Nebulite::endl;
        return Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS();
    }
    std::string const key = argv[1];

    if (domain->memberType(key) != Data::JSON::KeyType::array) {
        std::string command = __FUNCTION__;
        command += " " + std::string(ensureArray_name);
        command += " " + key;
        if (Constants::Error const result = domain->parseStr(command); result != Constants::ErrorTable::NONE()) {
            Nebulite::cerr() << "Error: Failed to ensure array for key '" << key << "'." << Nebulite::endl;
            return result;
        }
    }

    size_t const size = domain->memberSize(key);

    //------------------------------------------
    // Security check:
    // if any array item is a document, throw error
    // This feature is yet to be implemented!
    for (size_t i = 0; i < size; ++i) {
        if (domain->memberType(key + "[" + std::to_string(i) + "]") == Data::JSON::KeyType::object) {
            Nebulite::cerr() << "Error: Cannot push_front into an array containing documents." << Nebulite::endl;
            return Constants::ErrorTable::FUNCTIONAL::CRITICAL_FUNCTION_NOT_IMPLEMENTED();
        }
    }

    //------------------------------------------
    // Move all existing items one step back
    for (size_t i = 1; i < size; i++) {
        std::string itemKey = key + "[" + std::to_string(i) + "]";
        auto itemValue = domain->get<std::string>(itemKey);
        std::string newItemKey = key + "[" + std::to_string(i - 1) + "]";
        domain->set(newItemKey, itemValue);
    }
    // Remove the last item
    std::string const lastItemKey = key + "[" + std::to_string(size - 1) + "]";
    domain->removeKey(lastItemKey.c_str());

    return Constants::ErrorTable::NONE();
}

} // namespace Nebulite::DomainModule::JSON
