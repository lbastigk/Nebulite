#include "Data/Document/JsonScope.hpp"
#include "Interaction/Context.hpp"
#include "Interaction/Execution/Domain.hpp"

namespace Nebulite::Interaction {

ContextScopeBase Context::demote() const {
    return ContextScopeBase{
        self.domainScope.shareScopeBase(""),
        other.domainScope.shareScopeBase(""),
        global.domainScope.shareScopeBase("")
    };
}

} // namespace Nebulite::Interaction
