#ifndef INTERACTION_RULES_CONSTRUCTION_INITIALIZER_HPP
#define INTERACTION_RULES_CONSTRUCTION_INITIALIZER_HPP

namespace Nebulite::Interaction::Rules {
class StaticRulesetMap;

namespace Construction {
/**
 * @brief Initializes the StaticRulesetMap with all available static rulesets.
 * @param srm The StaticRulesetMap to initialize.
 */
void rulesetMapInit(StaticRulesetMap* srm);
} // namespace Construction
} // namespace Nebulite::Interaction::Rules
#endif // INTERACTION_RULES_CONSTRUCTION_INITIALIZER_HPP
