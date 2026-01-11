#include "Nebulite.hpp"
#include "Core/JsonScope.hpp"
#include "DomainModule/JsonScope/SimpleData.hpp"

namespace Nebulite::DomainModule::JsonScope {

Constants::Error SimpleData::update() {return Constants::ErrorTable::NONE();} // No periodic update needed, SimpleData is stateless

//------------------------------------------
// Domain-Bound Functions

//------------------------------------------
// General set/get/remove functions

Constants::Error SimpleData::set(int argc, char** argv) {
    std::scoped_lock<std::recursive_mutex> mtx = domain.lock(); // Lock the domain for thread-safe access
    if (argc < 3) {
        Nebulite::cerr() << "Error: Too few arguments for set command." << Nebulite::endl;
        return Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }

    auto const key = moduleScope.getRootScope() + argv[1];
    std::string value = argv[2];
    for (int i = 3; i < argc; ++i) {
        value += " " + std::string(argv[i]);
    }
    domain.set(key, value);
    return Constants::ErrorTable::NONE();
}

Constants::Error SimpleData::move(int argc, char** argv) {
    std::scoped_lock<std::recursive_mutex> mtx = domain.lock(); // Lock the domain for thread-safe access
    if (argc != 3) {
        Nebulite::cerr() << "Error: Too few arguments for move command." << Nebulite::endl;
        return Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }

    auto const sourceKey = moduleScope.getRootScope() + argv[1];
    auto const targetKey = moduleScope.getRootScope() + argv[2];

    if (domain.memberType(sourceKey) == Data::KeyType::null) {
        Nebulite::cerr() << "Error: Source key '" << argv[1] << "' does not exist." << Nebulite::endl;
        return Constants::ErrorTable::FUNCTIONAL::UNKNOWN_ARG();
    }
    if (domain.memberType(sourceKey) == Data::KeyType::object) {
        Data::JSON subDoc = domain.getSubDoc(sourceKey);
        domain.removeKey(targetKey);
        domain.setSubDoc(targetKey, subDoc);
        domain.removeKey(sourceKey);
    } else if (domain.memberType(sourceKey) == Data::KeyType::array) {
        // Careful handling required:
        domain.removeKey(targetKey);

        size_t const size = domain.memberSize(sourceKey);
        for (size_t i = 0; i < size; ++i) {
            auto itemKey = sourceKey + "[" + std::to_string(i) + "]";
            auto itemValue = domain.get<std::string>(itemKey.view());
            auto targetItemKey = targetKey + "[" + std::to_string(i) + "]";
            domain.set(targetItemKey, itemValue);
        }
    } else {
        // Move the value from sourceKey to targetKey
        auto const value = domain.get<std::string>(sourceKey);
        domain.removeKey(targetKey);
        domain.set(targetKey, value);
        domain.removeKey(sourceKey);
    }
    return Constants::ErrorTable::NONE();
}

Constants::Error SimpleData::copy(int argc, char** argv) {
    std::scoped_lock<std::recursive_mutex> mtx = domain.lock(); // Lock the domain for thread-safe access
    if (argc != 3) {
        Nebulite::cerr() << "Error: Too few arguments for copy command." << Nebulite::endl;
        return Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }

    auto const sourceKey = moduleScope.getRootScope() + argv[1];
    auto const targetKey = moduleScope.getRootScope() + argv[2];

    if (domain.memberType(sourceKey) == Data::KeyType::null) {
        Nebulite::cerr() << "Error: Source key '" << std::string(argv[1]) << "' does not exist." << Nebulite::endl;
        return Constants::ErrorTable::FUNCTIONAL::UNKNOWN_ARG();
    }
    if (domain.memberType(sourceKey) == Data::KeyType::object) {
        Data::JSON subDoc = domain.getSubDoc(sourceKey);
        domain.removeKey(targetKey);
        domain.setSubDoc(targetKey, subDoc);
    } else if (domain.memberType(sourceKey) == Data::KeyType::array) {
        // Careful handling required:
        domain.removeKey(targetKey);

        size_t const size = domain.memberSize(sourceKey);
        for (size_t i = 0; i < size; ++i) {
            auto itemKey = sourceKey + "[" + std::to_string(i) + "]";
            auto itemValue = domain.get<std::string>(itemKey);
            auto targetItemKey = targetKey + "[" + std::to_string(i) + "]";
            domain.set(targetItemKey, itemValue);
        }
    } else {
        // Move the value from sourceKey to targetKey
        auto const value = domain.get<std::string>(sourceKey);
        domain.removeKey(targetKey);
        domain.set(targetKey, value);
    }
    return Constants::ErrorTable::NONE();
}

Constants::Error SimpleData::keyDelete(int argc, char** argv) {
    std::scoped_lock<std::recursive_mutex> mtx = domain.lock(); // Lock the domain for thread-safe access
    if (argc != 2) {
        Nebulite::cerr() << "Error: Too few arguments for delete command." << Nebulite::endl;
        return Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }
    auto const key = moduleScope.getRootScope() + argv[1];
    domain.removeKey(key);
    return Constants::ErrorTable::NONE();
}

//------------------------------------------
// Array manipulation functions

Constants::Error SimpleData::ensureArray(int argc, char** argv) {
    std::scoped_lock<std::recursive_mutex> mtx = domain.lock(); // Lock the domain for thread-safe access
    if (argc < 2) {
        Nebulite::cerr() << "Error: Too few arguments for ensureArray command." << Nebulite::endl;
        return Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }
    if (argc > 2) {
        Nebulite::cerr() << "Error: Too many arguments for ensureArray command." << Nebulite::endl;
        return Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS();
    }

    auto const key = moduleScope.getRootScope() + argv[1];

    Data::KeyType keyType = domain.memberType(key);

    if (keyType == Data::KeyType::array) {
        // Already an array, nothing to do
        return Constants::ErrorTable::NONE();
    }

    if (keyType == Data::KeyType::value) {
        // pop out value
        auto const existingValue = domain.get<std::string>(key);
        domain.removeKey(key);

        // Set as new value
        auto const arrayKey = key + "[0]";
        domain.set(arrayKey, existingValue);

        // All done
        return Constants::ErrorTable::NONE();
    }

    Nebulite::cerr() << "Error: Key '" << argv[1] << "' is unsupported type " << static_cast<int>(keyType) << ", cannot convert to array." << Nebulite::endl;
    return Constants::ErrorTable::FUNCTIONAL::CRITICAL_FUNCTION_NOT_IMPLEMENTED();
}

Constants::Error SimpleData::push_back(int argc, char** argv) {
    std::scoped_lock<std::recursive_mutex> mtx = domain.lock(); // Lock the domain for thread-safe access
    if (argc > 3) {
        Nebulite::cerr() << "Error: Too many arguments for push_front command." << Nebulite::endl;
        return Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS();
    }
    auto const key = moduleScope.getRootScope() + argv[1];
    std::string value;
    if (argc < 3) {
        // Trying to push an empty value
        // Usually, this would be "{}" but we use an empty string for simplicity
        value = "";
    } else {
        value = argv[2];
    }

    if (domain.memberType(key) != Data::KeyType::array) {
        std::string command = __FUNCTION__;
        command += " " + std::string(ensureArray_name);
        command += " " + std::string(argv[1]);
        if (Constants::Error const result = domain.parseStr(command); result != Constants::ErrorTable::NONE()) {
            Nebulite::cerr() << "Error: Failed to ensure array for key '" << std::string(argv[1]) << "'." << Nebulite::endl;
            return result;
        }
    }

    size_t const size = domain.memberSize(key);
    auto const itemKey = key + "[" + std::to_string(size) + "]";
    domain.set(itemKey, value);
    return Constants::ErrorTable::NONE();
}

Constants::Error SimpleData::pop_back(int argc, char** argv) {
    std::scoped_lock<std::recursive_mutex> mtx = domain.lock(); // Lock the domain for thread-safe access
    if (argc < 2) {
        Nebulite::cerr() << "Error: Too few arguments for push_back command." << Nebulite::endl;
        return Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }
    if (argc > 2) {
        Nebulite::cerr() << "Error: Too many arguments for push_back command." << Nebulite::endl;
        return Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS();
    }
    auto const key = moduleScope.getRootScope() + argv[1];

    if (domain.memberType(key) != Data::KeyType::array) {
        std::string command = __FUNCTION__;
        command += " " + std::string(ensureArray_name);
        command += " " + std::string(argv[1]);
        if (Constants::Error const result = domain.parseStr(command); result != Constants::ErrorTable::NONE()) {
            Nebulite::cerr() << "Error: Failed to ensure array for key '" << std::string(argv[1]) << "'." << Nebulite::endl;
            return result;
        }
    }

    size_t const size = domain.memberSize(key);
    if (size == 0) {
        // nothing to pop out, not seen as error
        return Constants::ErrorTable::NONE();
    }

    auto const itemKey = key + "[" + std::to_string(size - 1) + "]";
    domain.removeKey(itemKey);
    return Constants::ErrorTable::NONE();
}

Constants::Error SimpleData::push_front(int argc, char** argv) {
    std::scoped_lock<std::recursive_mutex> mtx = domain.lock(); // Lock the domain for thread-safe access
    if (argc > 3) {
        Nebulite::cerr() << "Error: Too many arguments for push_front command." << Nebulite::endl;
        return Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS();
    }
    auto const key = moduleScope.getRootScope() + argv[1];
    std::string value;
    if (argc < 3) {
        // Trying to push an empty value
        // Usually, this would be "{}" but we use an empty string for simplicity
        value = "";
    } else {
        value = argv[2];
    }

    if (domain.memberType(key) != Data::KeyType::array) {
        std::string command = __FUNCTION__;
        command += " " + std::string(ensureArray_name);
        command += " " + std::string(argv[1]);
        if (Constants::Error const result = domain.parseStr(command); result != Constants::ErrorTable::NONE()) {
            Nebulite::cerr() << "Error: Failed to ensure array for key '" << std::string(argv[1]) << "'." << Nebulite::endl;
            return result;
        }
    }

    size_t const size = domain.memberSize(key);

    //------------------------------------------
    // Security check:
    // if any array item is a document, throw error
    // This feature is yet to be implemented!
    for (size_t i = 0; i < size; ++i) {
        auto itemKey = key + "[" + std::to_string(i) + "]";
        if (Data::KeyType const itemType = domain.memberType(itemKey); itemType == Data::KeyType::object) {
            Nebulite::cerr() << "Error: Cannot push_front into an array containing documents." << Nebulite::endl;
            return Constants::ErrorTable::FUNCTIONAL::CRITICAL_FUNCTION_NOT_IMPLEMENTED();
        }
    }

    //------------------------------------------
    // Move all existing items one step forward
    for (size_t i = size; i > 0; --i) {
        auto itemKey = key + "[" + std::to_string(i - 1) + "]";
        auto itemValue = domain.get<std::string>(itemKey);
        auto newItemKey = key + "[" + std::to_string(i) + "]";
        domain.set(newItemKey, itemValue);
    }
    auto const itemKey = key + "[0]";
    domain.set(itemKey, value);
    return Constants::ErrorTable::NONE();
}

Constants::Error SimpleData::pop_front(int argc, char** argv) {
    std::scoped_lock<std::recursive_mutex> mtx = domain.lock(); // Lock the domain for thread-safe access
    if (argc < 2) {
        Nebulite::cerr() << "Error: Too few arguments for pop_front command." << Nebulite::endl;
        return Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }
    if (argc > 2) {
        Nebulite::cerr() << "Error: Too many arguments for pop_front command." << Nebulite::endl;
        return Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS();
    }
    auto const key = moduleScope.getRootScope() + argv[1];

    if (domain.memberType(key) != Data::KeyType::array) {
        std::string command = __FUNCTION__;
        command += " " + std::string(ensureArray_name);
        command += " " + std::string(argv[1]);
        if (Constants::Error const result = domain.parseStr(command); result != Constants::ErrorTable::NONE()) {
            Nebulite::cerr() << "Error: Failed to ensure array for key '" << std::string(argv[1]) << "'." << Nebulite::endl;
            return result;
        }
    }

    size_t const size = domain.memberSize(key);

    //------------------------------------------
    // Security check:
    // if any array item is a document, throw error
    // This feature is yet to be implemented!
    for (size_t i = 0; i < size; ++i) {
        if (domain.memberType(key + "[" + std::to_string(i) + "]") == Data::KeyType::object) {
            Nebulite::cerr() << "Error: Cannot push_front into an array containing documents." << Nebulite::endl;
            return Constants::ErrorTable::FUNCTIONAL::CRITICAL_FUNCTION_NOT_IMPLEMENTED();
        }
    }

    //------------------------------------------
    // Move all existing items one step back
    for (size_t i = 1; i < size; i++) {
        auto itemKey = key + "[" + std::to_string(i) + "]";
        auto itemValue = domain.get<std::string>(itemKey);
        auto newItemKey = key + "[" + std::to_string(i - 1) + "]";
        domain.set(newItemKey, itemValue);
    }
    // Remove the last item
    auto const lastItemKey = key + "[" + std::to_string(size - 1) + "]";
    domain.removeKey(lastItemKey);

    return Constants::ErrorTable::NONE();
}

} // namespace Nebulite::DomainModule::JsonScope
