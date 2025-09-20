#include "DomainModule/JSON/JDM_ForceValue.hpp"

#include "Utility/JSON.hpp"

Nebulite::Constants::ERROR_TYPE Nebulite::DomainModule::JSON::ForceValue::force(int argc, char* argv[]) {
    std::lock_guard<std::recursive_mutex> mtx = domain->lock(); // Lock the domain for thread-safe access
    if (argc < 3) {
        return Nebulite::Constants::ERROR_TYPE::TOO_FEW_ARGS;
    }
    if (argc > 3) {
        return Nebulite::Constants::ERROR_TYPE::TOO_MANY_ARGS;
    }

    std::string key = argv[1];
    std::string value = argv[2];
    forced_global_values[key] = value;
    return Nebulite::Constants::ERROR_TYPE::NONE;
}

Nebulite::Constants::ERROR_TYPE Nebulite::DomainModule::JSON::ForceValue::forceClear(int argc, char* argv[]) {
    std::lock_guard<std::recursive_mutex> mtx = domain->lock(); // Lock the domain for thread-safe access
    forced_global_values.clear();
    return Nebulite::Constants::ERROR_TYPE::NONE;
}