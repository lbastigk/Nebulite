#include "Interaction/Context.hpp"
#include "Interaction/Execution/Domain.hpp"
#include "Core/JsonScope.hpp"

namespace Nebulite::Interaction {

ContextScopeBase ContextBase::demote() const {
    return ContextScopeBase{
        self.shareDocumentScopeBase(""),
        other.shareDocumentScopeBase(""),
        global.shareDocumentScopeBase("")
    };
}

} // namespace Nebulite::Interaction
