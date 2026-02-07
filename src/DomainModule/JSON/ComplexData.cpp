#include "Nebulite.hpp"
#include "Core/JsonScope.hpp"
#include "DomainModule/JsonScope/ComplexData.hpp"

namespace Nebulite::DomainModule::JsonScope {

//------------------------------------------
// Update
Constants::Error ComplexData::update() {
    // Add Domain-specific updates here!
    // General rule:
    // This is used to update all variables/states that are INTERNAL ONLY
    return Constants::ErrorTable::NONE();
}

//------------------------------------------
// Domain-Bound Functions

// NOLINTNEXTLINE
Constants::Error ComplexData::querySet() {
    std::scoped_lock<std::recursive_mutex> mtx = domain.lock(); // Lock the domain for thread-safe access
    return Constants::ErrorTable::FUNCTIONAL::CRITICAL_FUNCTION_NOT_IMPLEMENTED();
}

// NOLINTNEXTLINE
Constants::Error ComplexData::jsonSet(std::span<std::string const> const& args, Interaction::Execution::Domain& caller, Data::JsonScopeBase& callerScope) {
    std::scoped_lock<std::recursive_mutex> mtx = callerScope.lock(); // Lock the domain for thread-safe access
    // Since we have no access to the global space, we cant use the JSON doc cache
    // Instead, we manually load the document to retrieve the key
    if (args.size() < 3) {
        return Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }
    if (args.size() > 3) {
        return Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS();
    }
    std::string const& myKey = args[1];
    std::string const& docKey = args[2];

    // Depending on the type of docKey, we retrieve the value

    // === DOCUMENT ===
    if (Data::KeyType const type = Global::instance().getDocCache().memberType(docKey); type == Data::KeyType::object) {
        // Retrieve the sub-document
        Data::JSON const subDoc = Global::instance().getDocCache().getSubDoc(docKey);

        // Set the sub-document in the current JSON tree
        callerScope.setSubDoc(callerScope.getRootScope() + myKey, subDoc);
    }
    // === VALUE ===
    else if (type == Data::KeyType::value) {
        callerScope.set(callerScope.getRootScope() + myKey, Global::instance().getDocCache().get<std::string>(docKey));
    }
    // === ARRAY ===
    else if (type == Data::KeyType::array) {
        size_t const size = Global::instance().getDocCache().memberSize(docKey);
        for (size_t i = 0; i < size; ++i) {
            std::string itemKey = docKey + "[" + std::to_string(i) + "]";
            auto itemValue = Global::instance().getDocCache().get<std::string>(itemKey);
            std::string newItemKey = myKey + "[" + std::to_string(i) + "]";
            callerScope.set(callerScope.getRootScope() + newItemKey, itemValue);
        }
    }
    (void)caller; // Unused parameter
    return Constants::ErrorTable::NONE();
}
} // namespace Nebulite::DomainModule::JsonScope
