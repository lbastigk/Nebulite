//------------------------------------------
// Includes

// Standard library
#include <stdexcept>
#include <string_view>
#include <vector>

// Nebulite
#include "Interaction/Execution/Domain.hpp"
#include "Interaction/Rules/StaticRulesetMap.hpp"
#include "Module/Base/RulesetModule.hpp"
#include "Nebulite.hpp"

//------------------------------------------
namespace Nebulite::Module::Base {

RulesetModule::RulesetModule(std::string_view const moduleName)
: id{Data::MappedOrderedCacheList::generateUniqueId(moduleName)}
{}

Interaction::Rules::BaseListFunction RulesetModule::generateBaseListFunction(std::vector<Data::ScopedKeyView> const& baseKeys) const {
    return [this, baseKeys](const Interaction::Execution::Domain& domain) -> double** {
        try {
            return domain.ensureOrderedCacheList(id, baseKeys);
        } catch (...) {
            return nullptr;
        }
    };
}

void RulesetModule::checkGlobalContextCorrectness(Interaction::Context const& context) {
    if (context.global.getId() != Global::instance().getId()) {
        throw std::runtime_error("The global context must be the actual GlobalSpace, as this function relies on pre-cached global variables.");
    }
}

} // namespace Nebulite::Module::Base
