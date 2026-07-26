//------------------------------------------
// Includes

// Standard library
#include <stdexcept>
#include <string_view>

// Nebulite
#include "Nebulite/Core/GlobalSpace.hpp"
#include "Nebulite/Interaction/Execution/Domain.hpp"
#include "Nebulite/Module/Base/RulesetModule.hpp"
#include "Nebulite/Nebulite.hpp"

//------------------------------------------
namespace Nebulite::Module::Base {

RulesetModule::RulesetModule(std::string_view const moduleName)
: id{Data::MappedOrderedCacheList::generateUniqueId(moduleName)}
{}

void RulesetModule::checkGlobalContextCorrectness(Interaction::Context const& context) {
    if (!isGlobalContextCorrect(context)) {
        throw std::runtime_error("The global context must be the actual GlobalSpace, as this function relies on pre-cached global variables.");
    }
}

bool RulesetModule::isGlobalContextCorrect(Interaction::Context const& context){
    return context.global.getId() == Global::instance().getId();
}

} // namespace Nebulite::Module::Base
