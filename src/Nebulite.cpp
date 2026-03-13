#include "Nebulite.hpp"
#include "Graphics/ImguiHelper.hpp"

namespace Nebulite {

Core::GlobalSpace& Global::instance(){
    static Core::GlobalSpace instance{"Nebulite", capture()}; // Pass the global capture to the GlobalSpace constructor
    return instance;
}

Data::JsonScope const& Global::settings() {
    static auto const& settingsScopeConst = globalDoc().shareManagedScopeBase("settings.");
    return settingsScopeConst;
}

Data::JsonScope& Global::shareScopeBase(ScopeAccessor::BaseAccessToken const& at, std::string const& prefix) {
    return globalDoc().shareManagedScopeBase(at.getPrefix() + prefix);
}

void Global::renderImguiGlobalSpaceWindow(){
    static auto& fullScope = globalDoc().shareManagedScopeBase("");
    //Graphics::ImguiHelper::renderJsonScope(fullScope, "GlobalSpace");
    Graphics::ImguiHelper::renderDomain(instance(), capture(), fullScope, "GlobalSpace Domain");
}

//------------------------------------------
// Private methods

Data::JSON& Global::globalDoc() {
    static Data::JSON instance;
    return instance;
}

} // namespace Nebulite
