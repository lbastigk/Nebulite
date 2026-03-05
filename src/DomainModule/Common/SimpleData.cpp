#include "Nebulite.hpp"
#include "DomainModule/Common/SimpleData.hpp"

namespace Nebulite::DomainModule::Common {

Constants::Error SimpleData::update() {return Constants::ErrorTable::NONE();} // No periodic update needed, SimpleData is stateless

//------------------------------------------
// Domain-Bound Functions

//------------------------------------------
// General set/get/remove functions

// NOLINTNEXTLINE
Constants::Error SimpleData::set(std::span<std::string const> const& args, Interaction::Execution::Domain& /*caller*/, Data::JsonScopeBase& callerScope) {
    auto lock = callerScope.lock(); // Lock the domain for thread-safe access
    if (args.size() < 3) {
        return Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }

    auto const key = callerScope.getRootScope() + args[1];
    std::string const value = Utility::StringHandler::recombineArgs(args.subspan(2));
    callerScope.set(key, value);
    return Constants::ErrorTable::NONE();
}

// NOLINTNEXTLINE
Constants::Error SimpleData::move(std::span<std::string const> const& args, Interaction::Execution::Domain& /*caller*/, Data::JsonScopeBase& callerScope) {
    auto lock = callerScope.lock(); // Lock the domain for thread-safe access
    if (args.size() < 3) {
        return Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }
    if (args.size() > 3) {
        return Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS();
    }
    auto const sourceKey = callerScope.getRootScope() + args[1];
    auto const targetKey = callerScope.getRootScope() + args[2];
    callerScope.moveMember(sourceKey, targetKey);
    return Constants::ErrorTable::NONE();
}

// NOLINTNEXTLINE
Constants::Error SimpleData::copy(std::span<std::string const> const& args, Interaction::Execution::Domain& /*caller*/, Data::JsonScopeBase& callerScope) {
    auto lock = callerScope.lock(); // Lock the domain for thread-safe access
    if (args.size() < 3) {
        return Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }
    if (args.size() > 3) {
        return Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS();
    }
    auto const sourceKey = callerScope.getRootScope() + args[1];
    auto const targetKey = callerScope.getRootScope() + args[2];
    callerScope.copyMember(sourceKey, targetKey);
    return Constants::ErrorTable::NONE();
}

// NOLINTNEXTLINE
Constants::Error SimpleData::keyDelete(std::span<std::string const> const& args, Interaction::Execution::Domain& /*caller*/, Data::JsonScopeBase& callerScope) {
    auto lock = callerScope.lock(); // Lock the domain for thread-safe access
    if (args.size() < 2) {
        return Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }
    if (args.size() > 2) {
        return Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS();
    }
    auto const key = callerScope.getRootScope() + args[1];
    callerScope.removeMember(key);
    return Constants::ErrorTable::NONE();
}

//------------------------------------------
// Array manipulation functions

// TODO: JSON::ensureArray could be a useful function

// NOLINTNEXTLINE
Constants::Error SimpleData::ensureArray(std::span<std::string const> const& args, Interaction::Execution::Domain& /*caller*/, Data::JsonScopeBase& callerScope) {
    auto lock = callerScope.lock(); // Lock the domain for thread-safe access
    if (args.size() < 2) {
        Error::println("Error: Too few arguments for ensureArray command.");
        return Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }
    if (args.size() > 2) {
        Error::println("Error: Too many arguments for ensureArray command.");
        return Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS();
    }

    if (auto const key = callerScope.getRootScope() + args[1]; callerScope.memberType(key) != Data::KeyType::array) {
        callerScope.moveMember(key, key + "[0]"); // Move existing value to array index 0
    }
    return Constants::ErrorTable::NONE();
}

Constants::Error SimpleData::push_back(std::span<std::string const> const& args, Interaction::Execution::Domain& caller, Data::JsonScopeBase& callerScope){
    auto lock = callerScope.lock(); // Lock the domain for thread-safe access
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
    callerScope.set(itemKey, value);
    return Constants::ErrorTable::NONE();
}

Constants::Error SimpleData::pop_back(std::span<std::string const> const& args, Interaction::Execution::Domain& caller, Data::JsonScopeBase& callerScope) {
    auto lock = callerScope.lock(); // Lock the domain for thread-safe access
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
    callerScope.removeMember(itemKey);
    return Constants::ErrorTable::NONE();
}

Constants::Error SimpleData::push_front(std::span<std::string const> const& args, Interaction::Execution::Domain& caller, Data::JsonScopeBase& callerScope) {
    auto lock = callerScope.lock(); // Lock the domain for thread-safe access
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
        auto itemValue = callerScope.get<std::string>(itemKey).value_or("");
        auto newItemKey = key + "[" + std::to_string(i) + "]";
        callerScope.set(newItemKey, itemValue);
    }
    auto const itemKey = key + "[0]";
    callerScope.set(itemKey, value);
    return Constants::ErrorTable::NONE();
}

Constants::Error SimpleData::pop_front(std::span<std::string const> const& args, Interaction::Execution::Domain& caller, Data::JsonScopeBase& callerScope) {
    auto lock = callerScope.lock(); // Lock the domain for thread-safe access
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
        auto itemValue = callerScope.get<std::string>(itemKey).value_or("");
        auto newItemKey = key + "[" + std::to_string(i - 1) + "]";
        callerScope.set(newItemKey, itemValue);
    }
    // Remove the last item
    auto const lastItemKey = key + "[" + std::to_string(size - 1) + "]";
    callerScope.removeMember(lastItemKey);
    return Constants::ErrorTable::NONE();
}

} // namespace Nebulite::DomainModule::Common
