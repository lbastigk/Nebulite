#include "Nebulite.hpp"
#include "Interaction/Rules/RulesetModule.hpp"


namespace Nebulite::Interaction::Rules {
RulesetModule::RulesetModule(std::string_view const& moduleName)
: id{Data::MappedOrderedDoublePointers::generateUniqueId(moduleName)}
{}

} // namespace Nebulite::Interaction::Rules
