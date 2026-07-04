//------------------------------------------
// Includes

// Standard library
#include <algorithm>
#include <ranges>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

// Nebulite
#include "Interaction/Context.hpp"
#include "Interaction/Execution/Domain.hpp"
#include "Interaction/Rules/Construction/Initializer.hpp"
#include "Interaction/Rules/StaticRulesetMap.hpp"
#include "Nebulite.hpp"

//------------------------------------------
// Due to lifetime issues, we need to keep track of the singleton
// with an outside variable.

namespace {
struct StatusTracker {
    bool mapDeleted = false;
} statusTracker;
} // namespace

//------------------------------------------
namespace Nebulite::Interaction::Rules {

StaticRulesetMap::StaticRulesetMap(){
    Construction::rulesetMapInit(this);
    bindStaticRuleset(StaticRulesetWithMetadata{
        .type=StaticRulesetWithMetadata::Type::Local,
        .topic=helpName,
        .description=helpDesc,
        .function=[this](const Context& context, double** slf, double** otr) { help(context, slf, otr); },
        .baseListFunc=helpBaseListFunc
    });
}

StaticRulesetMap::~StaticRulesetMap() {
    statusTracker.mapDeleted = true;
}

StaticRulesetMap& StaticRulesetMap::getInstance() {
    static StaticRulesetMap instance;
    return instance;
}

std::vector<StaticRulesetMap::StaticRulesetMetadata> StaticRulesetMap::getList() {
    std::vector<StaticRulesetMetadata> list;
    for (auto const& rule : getInstance().container | std::views::values) {
        if (rule.type != StaticRulesetWithMetadata::Type::invalid) {
            list.push_back(StaticRulesetMetadata{
                .type=rule.type,
                .topic=std::string(rule.topic),
                .description=std::string(rule.description)
            });
        }
    }
    return list;
}

StaticRulesetMap::StaticRulesetWithMetadata& StaticRulesetMap::getStaticRulesetByName(std::string_view const name) {
    // NOLINTBEGIN
    if (statusTracker.mapDeleted) {
        // Using a custom static invalid entry, just in case the in-class one becomes invalid!
        static auto invalid = StaticRulesetWithMetadata{
            .type=StaticRulesetWithMetadata::Type::invalid,
            .topic="",
            .description="",
            .function=nullptr
        };
        return invalid;
    }
    // NOLINTEND
    if (container.contains(name)) {
        return container[name];
    }
    return invalidEntry;
}

void StaticRulesetMap::bindStaticRuleset(StaticRulesetWithMetadata const& func) {
    // Exit program if duplicate
    if (container.contains(func.topic)) {
        throw std::runtime_error("Duplicate static ruleset name: " + std::string(func.topic));
    }
    container[func.topic] = func;
}

// NOLINTNEXTLINE
void StaticRulesetMap::help(Interaction::Context const& context, double** /*slf*/, double** /*otr*/) const {
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


