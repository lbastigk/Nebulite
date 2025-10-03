#include "DomainModule/JSON/JDM_ForceValue.hpp"

#include "Utility/JSON.hpp"

Nebulite::Constants::Error Nebulite::DomainModule::JSON::ForceValue::force_set(int argc, char* argv[]) {
    std::lock_guard<std::recursive_mutex> mtx = domain->lock(); // Lock the domain for thread-safe access
    if (argc < 3) {
        return Nebulite::Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }
    if (argc > 3) {
        return Nebulite::Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS();
    }

    std::string key = argv[1];
    std::string value = argv[2];
    forced_global_values[key] = value;
    return Nebulite::Constants::ErrorTable::NONE();
}

Nebulite::Constants::Error Nebulite::DomainModule::JSON::ForceValue::force_clear(int argc, char* argv[]) {
    std::lock_guard<std::recursive_mutex> mtx = domain->lock(); // Lock the domain for thread-safe access
    forced_global_values.clear();
    return Nebulite::Constants::ErrorTable::NONE();
}