//------------------------------------------
// Includes

// Standard library
#include <string_view>
#include <vector>

// Nebulite
#include "Interaction/Execution/Domain.hpp"
#include "Interaction/Rules/StaticRulesetMap.hpp"
#include "Module/Base/RulesetModule.hpp"
#include "Nebulite.hpp"

//------------------------------------------
namespace Nebulite::Module::Base {

RulesetModule::RulesetModule(std::string_view const& moduleName)
: id{Data::MappedOrderedCacheList::generateUniqueId(moduleName)}
{}

Interaction::Rules::BaseListFunction RulesetModule::generateBaseListFunction(std::vector<Data::ScopedKeyView> const& baseKeys) const {
    return [this, baseKeys](const Interaction::Execution::Domain& domain) -> double** {
        return domain.ensureOrderedCacheList(id, baseKeys);
    };
}

} // namespace Nebulite::Module::Base
