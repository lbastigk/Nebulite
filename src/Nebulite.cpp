#include "Nebulite.hpp"
#include "Graphics/ImguiHelper.hpp"

namespace Nebulite {

void Global::renderImguiGlobalSettingsWindow(){
    static auto& fullScope = globalDoc().shareManagedScopeBase("");
    Graphics::ImguiHelper::renderJsonScope(fullScope, "GlobalSpace");
}

} // namespace Nebulite
