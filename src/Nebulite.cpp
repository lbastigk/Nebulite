#include "Nebulite.hpp"
#include "Graphics/ImguiHelper.hpp"

namespace Nebulite {

void Global::renderImguiGlobalSpaceWindow(){
    static auto& fullScope = globalDoc().shareManagedScopeBase("");
    Graphics::ImguiHelper::renderJsonScope(fullScope, "GlobalSpace");
}

} // namespace Nebulite
