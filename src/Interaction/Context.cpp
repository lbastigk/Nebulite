#include "Data/Document/JsonScope.hpp"
#include "Interaction/Context.hpp"
#include "Interaction/Execution/Domain.hpp"

namespace Nebulite::Interaction {

ContextScope Context::demote() const {
    return ContextScope{
        self.domainScope.shareScope(""),
        other.domainScope.shareScope(""),
        global.domainScope.shareScope("")
    };
}

} // namespace Nebulite::Interaction
