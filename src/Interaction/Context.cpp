#include "Interaction/Context.hpp"
#include "Interaction/Execution/Domain.hpp"
#include "Core/JsonScope.hpp"

namespace Nebulite::Interaction {

ContextScopeBase ContextBase::demote() const {
    return ContextScopeBase{
        self.domainScope.shareScopeBase(""),
        other.domainScope.shareScopeBase(""),
        global.domainScope.shareScopeBase("")
    };
}

} // namespace Nebulite::Interaction
