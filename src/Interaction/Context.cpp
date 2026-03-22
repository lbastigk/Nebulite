#include "Data/Document/JsonScope.hpp"
#include "Interaction/Context.hpp"
#include "Interaction/Execution/Domain.hpp"

namespace Nebulite::Interaction {

ContextScopeBase Context::demote() const {
    return ContextScopeBase{
        self.domainScope.shareScope(""),
        other.domainScope.shareScope(""),
        global.domainScope.shareScope("")
    };
}

} // namespace Nebulite::Interaction
