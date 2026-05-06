//------------------------------------------
// Includes

// Nebulite
#include "Interaction/Rules/StaticRulesetMap.hpp"
#include "Nebulite.hpp"


namespace Nebulite::Interaction::Rules {

// NOLINTNEXTLINE
void StaticRulesetMap::help(Interaction::Context const& context, double**& /*slf*/, double**& /*otr*/) const {
    Global::capture().log.println("[Available static rulesets:");
    auto list = getList();
    std::ranges::sort(list, [](auto const& metadata1, auto const& metadata2) { return metadata1.topic < metadata2.topic; });

    for (auto const& metadata : list) {
        context.self.capture.log.println();
        context.self.capture.log.println(metadata.topic);
        context.self.capture.log.println(metadata.description);
    }
}

} // namespace Nebulite::Interaction::Rules


