#include "Nebulite.hpp"
#include "DomainModule/JsonScope/SimpleData.hpp"

namespace Nebulite::DomainModule::JsonScope {

Constants::Error SimpleData::update() {return Constants::ErrorTable::NONE();} // No periodic update needed, SimpleData is stateless

//------------------------------------------
// Domain-Bound Functions

//------------------------------------------
// General set/get/remove functions

// NOLINTNEXTLINE
Constants::Error SimpleData::set(std::span<std::string const> const& args, Interaction::Execution::DomainBase& caller, Data::JsonScopeBase& callerScope) {
    std::scoped_lock<std::recursive_mutex> mtx = callerScope.lock(); // Lock the domain for thread-safe access
    if (args.size() < 3) {
        Error::println("Error: Too few arguments for set command.");
        return Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }

    auto const key = callerScope.getRootScope() + args[1];
    std::string value = args[2];
    for (auto const& arg : args.subspan(3)) {
        value += " " + std::string(arg);
    }
    (void)caller;
    callerScope.set(key, value);
    return Constants::ErrorTable::NONE();
}

// NOLINTNEXTLINE
Constants::Error SimpleData::move(std::span<std::string const> const& args, Interaction::Execution::DomainBase& caller, Data::JsonScopeBase& callerScope) {
    std::scoped_lock<std::recursive_mutex> mtx = callerScope.lock(); // Lock the domain for thread-safe access
    if (args.size() != 3) {
        Error::println("Error: Too few arguments for move command.");
        return Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }

    auto const sourceKey = callerScope.getRootScope() + args[1];
    auto const targetKey = callerScope.getRootScope() + args[2];

    if (callerScope.memberType(sourceKey) == Data::KeyType::null) {
        Error::println("Error: Source key '", args[1], "' does not exist.");
        return Constants::ErrorTable::FUNCTIONAL::UNKNOWN_ARG();
    }
    if (callerScope.memberType(sourceKey) == Data::KeyType::object) {
        Data::JSON const subDoc = callerScope.getSubDoc(sourceKey);
        callerScope.removeMember(targetKey);
        callerScope.setSubDoc(targetKey, subDoc);
        callerScope.removeMember(sourceKey);
    } else if (callerScope.memberType(sourceKey) == Data::KeyType::array) {
        // Careful handling required:
        callerScope.removeMember(targetKey);

        size_t const size = callerScope.memberSize(sourceKey);
        for (size_t i = 0; i < size; ++i) {
            auto itemKey = sourceKey + "[" + std::to_string(i) + "]";
            auto itemValue = callerScope.get<std::string>(itemKey.view());
            auto targetItemKey = targetKey + "[" + std::to_string(i) + "]";
            callerScope.set(targetItemKey, itemValue);
        }
    } else {
        // Move the value from sourceKey to targetKey
        auto const value = callerScope.get<std::string>(sourceKey);
        callerScope.removeMember(targetKey);
        callerScope.set(targetKey, value);
        callerScope.removeMember(sourceKey);
    }
    (void)caller;
    return Constants::ErrorTable::NONE();
}

// NOLINTNEXTLINE
Constants::Error SimpleData::copy(std::span<std::string const> const& args, Interaction::Execution::DomainBase& caller, Data::JsonScopeBase& callerScope) {
    std::scoped_lock<std::recursive_mutex> mtx = callerScope.lock(); // Lock the domain for thread-safe access
    if (args.size() != 3) {
        Error::println("Error: Too few arguments for copy command.");
        return Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }

    auto const sourceKey = callerScope.getRootScope() + args[1];
    auto const targetKey = callerScope.getRootScope() + args[2];

    if (callerScope.memberType(sourceKey) == Data::KeyType::null) {
        Error::println("Error: Source key '", std::string(args[1]), "' does not exist.");
        return Constants::ErrorTable::FUNCTIONAL::UNKNOWN_ARG();
    }
    if (callerScope.memberType(sourceKey) == Data::KeyType::object) {
        Data::JSON const subDoc = callerScope.getSubDoc(sourceKey);
        callerScope.removeMember(targetKey);
        callerScope.setSubDoc(targetKey, subDoc);
    } else if (callerScope.memberType(sourceKey) == Data::KeyType::array) {
        // Careful handling required:
        callerScope.removeMember(targetKey);

        size_t const size = callerScope.memberSize(sourceKey);
        for (size_t i = 0; i < size; ++i) {
            auto itemKey = sourceKey + "[" + std::to_string(i) + "]";
            auto itemValue = callerScope.get<std::string>(itemKey);
            auto targetItemKey = targetKey + "[" + std::to_string(i) + "]";
            callerScope.set(targetItemKey, itemValue);
        }
    } else {
        // Move the value from sourceKey to targetKey
        auto const value = callerScope.get<std::string>(sourceKey);
        callerScope.removeMember(targetKey);
        callerScope.set(targetKey, value);
    }
    (void)caller;
    return Constants::ErrorTable::NONE();
}

// NOLINTNEXTLINE
Constants::Error SimpleData::keyDelete(std::span<std::string const> const& args, Interaction::Execution::DomainBase& caller, Data::JsonScopeBase& callerScope) {
    std::scoped_lock<std::recursive_mutex> mtx = callerScope.lock(); // Lock the domain for thread-safe access
    if (args.size() != 2) {
        Error::println("Error: Too few arguments for delete command.");
        return Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }
    auto const key = callerScope.getRootScope() + args[1];
    (void)caller;
    callerScope.removeMember(key);
    return Constants::ErrorTable::NONE();
}

//------------------------------------------
// Array manipulation functions

// NOLINTNEXTLINE
Constants::Error SimpleData::ensureArray(std::span<std::string const> const& args, Interaction::Execution::DomainBase& caller, Data::JsonScopeBase& callerScope) {
    std::scoped_lock<std::recursive_mutex> mtx = callerScope.lock(); // Lock the domain for thread-safe access
    if (args.size() < 2) {
        Error::println("Error: Too few arguments for ensureArray command.");
        return Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }
    if (args.size() > 2) {
        Error::println("Error: Too many arguments for ensureArray command.");
        return Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS();
    }

    auto const key = callerScope.getRootScope() + args[1];

    Data::KeyType keyType = callerScope.memberType(key);

    if (keyType == Data::KeyType::array) {
        // Already an array, nothing to do
        return Constants::ErrorTable::NONE();
    }

    if (keyType == Data::KeyType::value) {
        // pop out value
        auto const existingValue = callerScope.get<std::string>(key);
        callerScope.removeMember(key);

        // Set as new value
        auto const arrayKey = key + "[0]";
        callerScope.set(arrayKey, existingValue);

        // All done
        (void)caller;
        return Constants::ErrorTable::NONE();
    }

    Error::println("Error: Key '", args[1], "' is unsupported type ", static_cast<int>(keyType), ", cannot convert to array.");
    return Constants::ErrorTable::FUNCTIONAL::CRITICAL_FUNCTION_NOT_IMPLEMENTED();
}

Constants::Error SimpleData::push_back(std::span<std::string const> const& args, Interaction::Execution::DomainBase& caller, Data::JsonScopeBase& callerScope){
    std::scoped_lock<std::recursive_mutex> mtx = callerScope.lock(); // Lock the domain for thread-safe access
    if (args.size() > 3) {
        Error::println("Error: Too many arguments for push_front command.");
        return Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS();
    }
    auto const key = callerScope.getRootScope() + args[1];
    std::string value;
    if (args.size() < 3) {
        // Trying to push an empty value
        // Usually, this would be "{}" but we use an empty string for simplicity
        value = "";
    } else {
        value = args[2];
    }

    if (callerScope.memberType(key) != Data::KeyType::array) {
        std::string command = __FUNCTION__;
        command += " " + std::string(ensureArray_name);
        command += " " + std::string(args[1]);
        if (Constants::Error const result = caller.parseStr(command); result != Constants::ErrorTable::NONE()) {
            Error::println("Error: Failed to ensure array for key '", std::string(args[1]), "'.");
            return result;
        }
    }

    size_t const size = callerScope.memberSize(key);
    auto const itemKey = key + "[" + std::to_string(size) + "]";
    (void)caller;
    callerScope.set(itemKey, value);
    return Constants::ErrorTable::NONE();
}

Constants::Error SimpleData::pop_back(std::span<std::string const> const& args, Interaction::Execution::DomainBase& caller, Data::JsonScopeBase& callerScope) {
    std::scoped_lock<std::recursive_mutex> mtx = callerScope.lock(); // Lock the domain for thread-safe access
    if (args.size() < 2) {
        Error::println("Error: Too few arguments for push_back command.");
        return Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }
    if (args.size() > 2) {
        Error::println("Error: Too many arguments for push_back command.");
        return Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS();
    }
    auto const key = callerScope.getRootScope() + args[1];

    if (callerScope.memberType(key) != Data::KeyType::array) {
        std::string command = __FUNCTION__;
        command += " " + std::string(ensureArray_name);
        command += " " + std::string(args[1]);
        if (Constants::Error const result = caller.parseStr(command); result != Constants::ErrorTable::NONE()) {
            Error::println("Error: Failed to ensure array for key '", std::string(args[1]), "'.");
            return result;
        }
    }

    size_t const size = callerScope.memberSize(key);
    if (size == 0) {
        // nothing to pop out, not seen as error
        return Constants::ErrorTable::NONE();
    }

    auto const itemKey = key + "[" + std::to_string(size - 1) + "]";
    (void)caller;
    callerScope.removeMember(itemKey);
    return Constants::ErrorTable::NONE();
}

Constants::Error SimpleData::push_front(std::span<std::string const> const& args, Interaction::Execution::DomainBase& caller, Data::JsonScopeBase& callerScope) {
    std::scoped_lock<std::recursive_mutex> mtx = callerScope.lock(); // Lock the domain for thread-safe access
    if (args.size() > 3) {
        Error::println("Error: Too many arguments for push_front command.");
        return Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS();
    }
    auto const key = callerScope.getRootScope() + args[1];
    std::string value;
    if (args.size() < 3) {
        // Trying to push an empty value
        // Usually, this would be "{}" but we use an empty string for simplicity
        value = "";
    } else {
        value = args[2];
    }

    if (callerScope.memberType(key) != Data::KeyType::array) {
        std::string command = __FUNCTION__;
        command += " " + std::string(ensureArray_name);
        command += " " + std::string(args[1]);
        if (Constants::Error const result = caller.parseStr(command); result != Constants::ErrorTable::NONE()) {
            Error::println("Error: Failed to ensure array for key '", std::string(args[1]), "'.");
            return result;
        }
    }

    size_t const size = callerScope.memberSize(key);

    //------------------------------------------
    // Security check:
    // if any array item is a document, throw error
    // This feature is yet to be implemented!
    for (size_t i = 0; i < size; ++i) {
        auto itemKey = key + "[" + std::to_string(i) + "]";
        if (Data::KeyType const itemType = callerScope.memberType(itemKey); itemType == Data::KeyType::object) {
            Error::println("Error: Cannot push_front into an array containing documents.");
            return Constants::ErrorTable::FUNCTIONAL::CRITICAL_FUNCTION_NOT_IMPLEMENTED();
        }
    }

    //------------------------------------------
    // Move all existing items one step forward
    for (size_t i = size; i > 0; --i) {
        auto itemKey = key + "[" + std::to_string(i - 1) + "]";
        auto itemValue = callerScope.get<std::string>(itemKey);
        auto newItemKey = key + "[" + std::to_string(i) + "]";
        callerScope.set(newItemKey, itemValue);
    }
    auto const itemKey = key + "[0]";
    (void)caller;
    callerScope.set(itemKey, value);
    return Constants::ErrorTable::NONE();
}

Constants::Error SimpleData::pop_front(std::span<std::string const> const& args, Interaction::Execution::DomainBase& caller, Data::JsonScopeBase& callerScope) {
    std::scoped_lock<std::recursive_mutex> mtx = callerScope.lock(); // Lock the domain for thread-safe access
    if (args.size() < 2) {
        Error::println("Error: Too few arguments for pop_front command.");
        return Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }
    if (args.size() > 2) {
        Error::println("Error: Too many arguments for pop_front command.");
        return Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS();
    }
    auto const key = callerScope.getRootScope() + args[1];

    if (callerScope.memberType(key) != Data::KeyType::array) {
        std::string command = __FUNCTION__;
        command += " " + std::string(ensureArray_name);
        command += " " + std::string(args[1]);
        if (Constants::Error const result = caller.parseStr(command); result != Constants::ErrorTable::NONE()) {
            Error::println("Error: Failed to ensure array for key '", std::string(args[1]), "'.");
            return result;
        }
    }

    size_t const size = callerScope.memberSize(key);

    //------------------------------------------
    // Security check:
    // if any array item is a document, throw error
    // This feature is yet to be implemented!
    for (size_t i = 0; i < size; ++i) {
        if (callerScope.memberType(key + "[" + std::to_string(i) + "]") == Data::KeyType::object) {
            Error::println("Error: Cannot push_front into an array containing documents.");
            return Constants::ErrorTable::FUNCTIONAL::CRITICAL_FUNCTION_NOT_IMPLEMENTED();
        }
    }

    //------------------------------------------
    // Move all existing items one step back
    for (size_t i = 1; i < size; i++) {
        auto itemKey = key + "[" + std::to_string(i) + "]";
        auto itemValue = callerScope.get<std::string>(itemKey);
        auto newItemKey = key + "[" + std::to_string(i - 1) + "]";
        callerScope.set(newItemKey, itemValue);
    }
    // Remove the last item
    auto const lastItemKey = key + "[" + std::to_string(size - 1) + "]";
    (void)caller;
    callerScope.removeMember(lastItemKey);
    return Constants::ErrorTable::NONE();
}

} // namespace Nebulite::DomainModule::JsonScope
