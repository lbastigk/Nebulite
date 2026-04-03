#include "Nebulite.hpp"
#include "Interaction/Execution/Domain.hpp"
#include "Interaction/Rules/RulesetModule.hpp"

namespace Nebulite::Interaction::Rules {
RulesetModule::RulesetModule(std::string_view const& moduleName)
: id{Data::MappedOrderedCacheList::generateUniqueId(moduleName)}
{}

void RulesetModule::ensureBaseList(Execution::Domain const& domain, std::vector<Data::ScopedKeyView> const& keys, double**& arr) const {
    arr = domain.ensureOrderedCacheList(id, keys);
}

std::function<double**(const Execution::Domain&)> RulesetModule::generateBaseListFunction(std::vector<Data::ScopedKeyView> const& baseKeys) const {
    std::function<double**(const Execution::Domain&)> const baseListFunc = [this, baseKeys](const Execution::Domain& domain) -> double** {
        double** v;
        ensureBaseList(domain, baseKeys, v);
        return v;
    };
    return baseListFunc;
}

} // namespace Nebulite::Interaction::Rules
