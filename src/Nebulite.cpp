#include "Nebulite.hpp"
#include "Graphics/ImguiHelper.hpp"

namespace Nebulite {

Core::GlobalSpace& Global::instance(){
    static Core::GlobalSpace instance{"Nebulite"}; // Pass the global capture to the GlobalSpace constructor
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
    if (auto const result = instance().parseStr(__FUNCTION__ + std::string(" ") + "imgui-view on"); result.isError()) {
        capture().error.println("Error enabling ImGui view for GlobalSpace: " + std::string(result.getDescription()));
    }
}

//------------------------------------------
// Private methods

Data::JSON& Global::globalDoc() {
    static Data::JSON instance;
    return instance;
}

} // namespace Nebulite
