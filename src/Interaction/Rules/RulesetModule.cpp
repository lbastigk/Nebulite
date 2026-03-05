#include "Nebulite.hpp"
#include "Interaction/Execution/Domain.hpp"
#include "Interaction/Rules/RulesetModule.hpp"

namespace Nebulite::Interaction::Rules {
RulesetModule::RulesetModule(std::string_view const& moduleName)
: id{Data::MappedOrderedDoublePointers::generateUniqueId(moduleName)}
{}

void RulesetModule::ensureBaseList(Execution::Domain const& ctx, std::vector<Data::ScopedKeyView> const& keys, double**& arr) const {
    if (arr != nullptr) [[likely]] {
        return; // Already initialized, do nothing
    }
    arr = ctx.ensureOrderedCacheList(id, keys)->data();
}

} // namespace Nebulite::Interaction::Rules
