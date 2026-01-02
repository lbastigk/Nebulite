#include "DomainModule/JSON/ForceValue.hpp"
#include "Data/Document/JSON.hpp"

namespace Nebulite::DomainModule::JSON {

Constants::Error ForceValue::update() {
    // On each update, re-apply forced values
    for (auto const& [key, value] : forced_global_values) {
        domain.set(key, value);
    }
    return Constants::ErrorTable::NONE();
}

Constants::Error ForceValue::force_set(int argc, char** argv) {
    std::scoped_lock<std::recursive_mutex> mtx = domain.lock(); // Lock the domain for thread-safe access
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
    std::scoped_lock<std::recursive_mutex> mtx = domain.lock(); // Lock the domain for thread-safe access
    forced_global_values.clear();
    return Constants::ErrorTable::NONE();
}
} // namespace Nebulite::DomainModule::JSON
