//------------------------------------------
// Includes

// Nebulite
#include "Interaction/Execution/Domain.hpp"
#include "Interaction/Rules/RulesetModule.hpp"
#include "Nebulite.hpp"

//------------------------------------------
namespace Nebulite::Interaction::Rules {
RulesetModule::RulesetModule(std::string_view const& moduleName)
: id{Data::MappedOrderedCacheList::generateUniqueId(moduleName)}
{}

BaseListFunction RulesetModule::generateBaseListFunction(std::vector<Data::ScopedKeyView> const& baseKeys) const {
    return [this, baseKeys](const Execution::Domain& domain) -> double** {
        return domain.ensureOrderedCacheList(id, baseKeys);
    };
}

} // namespace Nebulite::Interaction::Rules
