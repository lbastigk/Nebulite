#include "Nebulite.hpp"
#include "Interaction/Execution/Domain.hpp"
#include "Interaction/Rules/RulesetModule.hpp"

namespace Nebulite::Interaction::Rules {
RulesetModule::RulesetModule(std::string_view const& moduleName)
: id{Data::MappedOrderedDoublePointers::generateUniqueId(moduleName)}
{}

void RulesetModule::ensureBaseList(Execution::Domain const& domain, std::vector<Data::ScopedKeyView> const& keys, double**& arr) const {
    arr = domain.ensureOrderedCacheList(id, keys)->data();
}

} // namespace Nebulite::Interaction::Rules
