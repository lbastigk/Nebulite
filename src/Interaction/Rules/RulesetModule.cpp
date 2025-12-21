#include "Nebulite.hpp"
#include "Interaction/Rules/RulesetModule.hpp"

namespace Nebulite::Interaction::Rules {
RulesetModule::RulesetModule(std::string_view const& moduleName)
    : id(Nebulite::global().getUniqueId(std::string(moduleName), Core::GlobalSpace::UniqueIdType::expression))
{}

} // namespace Nebulite::Interaction::Rules