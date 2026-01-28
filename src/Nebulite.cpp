#include "Nebulite.hpp"
#include "Graphics/ImguiHelper.hpp"

namespace Nebulite {

Core::GlobalSpace& Global::instance(){
    static Core::GlobalSpace instance{"Nebulite"};
    return instance;
}

Data::JsonScopeBase const& Global::settings() {
    static auto const& settingsScopeConst = globalDoc().shareManagedScopeBase("settings.");
    return settingsScopeConst;
}

Core::JsonScope& Global::shareScope(ScopeAccessor::BaseAccessToken const& at, std::string const& prefix) {
    return globalDoc().shareManagedScope(at.getPrefix() + prefix);
}

Data::JsonScopeBase& Global::shareScopeBase(ScopeAccessor::BaseAccessToken const& at, std::string const& prefix) {
    return globalDoc().shareManagedScopeBase(at.getPrefix() + prefix);
}

void Global::renderImguiGlobalSpaceWindow(){
    static auto& fullScope = globalDoc().shareManagedScopeBase("");
    Graphics::ImguiHelper::renderJsonScope(fullScope, "GlobalSpace");
}

//------------------------------------------
// Private methods

Data::JSON& Global::globalDoc() {
    static Data::JSON instance;
    return instance;
}

} // namespace Nebulite
