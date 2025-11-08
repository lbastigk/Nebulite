#include "DomainModule/JSON/JSDM_ForceValue.hpp"
#include "Utility/JSON.hpp"

namespace Nebulite::DomainModule::JSON {

std::string const ForceValue::force_name = "force";
std::string const ForceValue::force_desc = R"(Category for forcing variables to specific values.
This is useful for testing or overriding configuration values.
)";

Nebulite::Constants::Error ForceValue::update(){
    // This might not be enough, as between updates, that value might be changed again.
    // But its a good start.
    // Perhaps later on it's good to find a way to lock values directly in the JSON document?
    for (auto const& [key, value] : forced_global_values){
        domain->set(key, value);
    }
    return Nebulite::Constants::ErrorTable::NONE();
}

Nebulite::Constants::Error ForceValue::force_set(int argc,  char** argv){
    std::scoped_lock<std::recursive_mutex> mtx = domain->lock(); // Lock the domain for thread-safe access
    if (argc < 3){
        return Nebulite::Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }
    if (argc > 3){
        return Nebulite::Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS();
    }

    std::string key = argv[1];
    std::string value = argv[2];
    forced_global_values[key] = value;
    return Nebulite::Constants::ErrorTable::NONE();
}
std::string const ForceValue::force_set_name = "force set";
std::string const ForceValue::force_set_desc = R"(Force a variable to a value.

Usage: force set <key> <value>
)";

Nebulite::Constants::Error ForceValue::force_clear(int argc,  char** argv){
    std::scoped_lock<std::recursive_mutex> mtx = domain->lock(); // Lock the domain for thread-safe access
    forced_global_values.clear();
    return Nebulite::Constants::ErrorTable::NONE();
}
std::string const ForceValue::force_clear_name = "force clear";
std::string const ForceValue::force_clear_desc = R"(Clear all forced variables.

Usage: force clear
)";

} // namespace Nebulite::DomainModule::JSON