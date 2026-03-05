#include "Core/JsonScope.hpp"
#include "DomainModule/JsonScope/ForceValue.hpp"


namespace Nebulite::DomainModule::JsonScope {

// TODO: remove forcevalue entirely
Constants::Error ForceValue::update() {
    // On each update, re-apply forced values
    for (auto const& [key, value] : forced_global_values) {
        auto const scopedKey = moduleScope.getRootScope() + key;
        domain.set(scopedKey, value);
    }
    return Constants::ErrorTable::NONE();
}

Constants::Error ForceValue::force_set(int const argc, char** argv) {
    auto lock = domain.lock(); // Lock the domain for thread-safe access
    if (argc < 3) {
        return Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }
    if (argc > 3) {
        return Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS();
    }

    std::string const key = argv[1];
    std::string const value = argv[2];
    forced_global_values[key] = value;
    return Constants::ErrorTable::NONE();
}

Constants::Error ForceValue::force_clear() {
    auto lock = domain.lock(); // Lock the domain for thread-safe access
    forced_global_values.clear();
    return Constants::ErrorTable::NONE();
}
} // namespace Nebulite::DomainModule::JsonScope
