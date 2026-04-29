//------------------------------------------
// Includes

// Nebulite
#include "Graphics/ImguiHelper.hpp"
#include "Nebulite.hpp"

//------------------------------------------
namespace Nebulite {

Core::GlobalSpace& Global::instance(){
    static Core::GlobalSpace instance{"Nebulite"}; // Pass the global capture to the GlobalSpace constructor
    return instance;
}

Data::JsonScope const& Global::settings() {
    static auto const& settingsScopeConst = globalDoc().shareManagedScopeBase("settings.");
    return settingsScopeConst;
}

Data::JsonScope& Global::shareScope(ScopeAccessor::BaseAccessToken const& at, std::string const& prefix) {
    return globalDoc().shareManagedScopeBase(at.getPrefix() + prefix);
}

//------------------------------------------
// Private methods

Data::JSON& Global::globalDoc() {
    static Data::JSON instance;
    return instance;
}

} // namespace Nebulite
