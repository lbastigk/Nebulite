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

Constants::Error ComplexData::querySet() {
    std::scoped_lock<std::recursive_mutex> mtx = domain.lock(); // Lock the domain for thread-safe access
    return Constants::ErrorTable::FUNCTIONAL::CRITICAL_FUNCTION_NOT_IMPLEMENTED();
}

Constants::Error ComplexData::jsonSet(int argc, char** argv) {
    std::scoped_lock<std::recursive_mutex> mtx = domain.lock(); // Lock the domain for thread-safe access
    // Since we have no access to the global space, we cant use the JSON doc cache
    // Instead, we manually load the document to retrieve the key
    if (argc < 3) {
        return Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }
    if (argc > 3) {
        return Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS();
    }
    std::string const myKey = argv[1];
    std::string const docKey = argv[2];

    // Depending on the type of docKey, we retrieve the value

    // === DOCUMENT ===
    if (Data::KeyType const type = Nebulite::global().getDocCache().memberType(docKey); type == Data::KeyType::object) {
        // Retrieve the sub-document
        Data::JSON const subDoc = Nebulite::global().getDocCache().getSubDoc(docKey);

        // Set the sub-document in the current JSON tree
        domain.setSubDoc(moduleScope.getRootScope() + myKey, subDoc);
    }
    // === VALUE ===
    else if (type == Data::KeyType::value) {
        domain.set(moduleScope.getRootScope() + myKey, Nebulite::global().getDocCache().get<std::string>(docKey));
    }
    // === ARRAY ===
    else if (type == Data::KeyType::array) {
        size_t const size = Nebulite::global().getDocCache().memberSize(docKey);
        for (size_t i = 0; i < size; ++i) {
            std::string itemKey = docKey + "[" + std::to_string(i) + "]";
            auto itemValue = Nebulite::global().getDocCache().get<std::string>(itemKey);
            std::string newItemKey = myKey + "[" + std::to_string(i) + "]";
            domain.set(moduleScope.getRootScope() + newItemKey, itemValue);
        }
    }
    return Constants::ErrorTable::NONE();
}
} // namespace Nebulite::DomainModule::JsonScope
