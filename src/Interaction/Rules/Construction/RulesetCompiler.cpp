//------------------------------------------
// Includes

// Standard library
#include <memory>
#include <optional>
#include <ranges>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

// Nebulite
#include "Nebulite/Constants/KeyNames.hpp"
#include "Nebulite/Core/GlobalSpace.hpp"
#include "Nebulite/Data/Document/JsonScope.hpp"
#include "Nebulite/Data/Document/KeyType.hpp"
#include "Nebulite/Interaction/Context.hpp"
#include "Nebulite/Interaction/Execution/Domain.hpp"
#include "Nebulite/Interaction/Logic/Assignment.hpp"
#include "Nebulite/Interaction/Logic/Expression.hpp"
#include "Nebulite/Interaction/Rules/Construction/RulesetCompiler.hpp"
#include "Nebulite/Interaction/Rules/Ruleset.hpp"
#include "Nebulite/Interaction/Rules/StaticRulesetMap.hpp"
#include "Nebulite/Nebulite.hpp"
#include "Nebulite/Utility/Ranges.hpp"
#include "Nebulite/Utility/StringHandler.hpp"

//------------------------------------------
namespace Nebulite::Interaction::Rules::Construction {

void RulesetCompiler::getFunctionCalls(Data::JsonScope const& entryDoc, JsonRuleset& Ruleset) {
    auto getCalls = [&entryDoc](Data::ScopedKeyView const& key, std::vector<Logic::Expression>& target) {
        if (entryDoc.memberType(key) == Data::KeyType::array) {
            for (auto const& indexKey : entryDoc.arrayKeys(key)) {
                target.emplace_back(entryDoc.get<std::string>(indexKey).value_or(""));
            }
        }
    };
    getCalls(Constants::KeyNames::Ruleset::parseOnGlobal, Ruleset.functioncalls_global);
    getCalls(Constants::KeyNames::Ruleset::parseOnSelf, Ruleset.functioncalls_self);
    getCalls(Constants::KeyNames::Ruleset::parseOnOther, Ruleset.functioncalls_other);
}

bool RulesetCompiler::getAssignments(std::shared_ptr<JsonRuleset> const& Ruleset, Data::JsonScope const& entry) {
    if (entry.memberType(Constants::KeyNames::Ruleset::assignments) == Data::KeyType::array) {
        for (auto const& key : entry.arrayKeys(Constants::KeyNames::Ruleset::assignments)) {
            if (Logic::Assignment assignment; assignment.parse(entry.get<std::string>(key).value_or(""))) {
                Ruleset->assignments.emplace_back(std::move(assignment));
            }
        }
    } else {
        // No expressions
        return false;
    }
    return true;
}

std::string RulesetCompiler::getCondition(Data::JsonScope const& entry) {
    std::string logicalArg;
    if (entry.memberType(Constants::KeyNames::Ruleset::condition) == Data::KeyType::array) {
        logicalArg = "$(";
        for (auto const& key : entry.arrayKeys(Constants::KeyNames::Ruleset::condition)) {
            logicalArg += "(" + entry.get<std::string>(key).value_or("0") + ")";
            logicalArg += "*"; // Arguments in vector need to be all true: &-logic -> Multiplication
        }
        if (logicalArg.ends_with("*")) {
            logicalArg.pop_back(); // Remove last *
        }
        logicalArg += ")";
    } else {
        // Assume simple value, string:
        logicalArg = entry.get<std::string>(Constants::KeyNames::Ruleset::condition).value_or("0");
        // Ensure logicalArg is encapsulated in an evaluation
        if (!logicalArg.starts_with("$(")) {
            logicalArg = "$(" + logicalArg + ")";
        }
    }
    return logicalArg;
}

bool RulesetCompiler::getJsonRuleset(Data::JsonScope const& doc, Data::JsonScope& entry, Data::ScopedKeyView const& key) {
    if (doc.memberType(key) == Data::KeyType::object) {
        std::string const& serial = doc.serialize(key);
        entry.deserialize(serial);
    } else {
        // Is perhaps link to document
        auto const potentialLink = doc.get<std::string>(key).value_or("");
        if (potentialLink.starts_with("::")) {
            return false; // Is a static ruleset
        }
        std::string const file = Global::instance().getDocCache().getDocString(potentialLink);
        if (file.empty()) {
            return false;
        }
        entry.deserialize(file);
    }
    return true;
}

void RulesetCompiler::setMetaData(RulesetVector const& rulesets) {
    for (auto [i, ruleset] : rulesets | Utility::Ranges::enumerate) {
        ruleset->index = i;
    }
}

void RulesetCompiler::parse(RulesetVector& rulesetsGlobal, RulesetVector& rulesetsLocal, Execution::Domain& self, Data::JsonScope const& rulesetArray) {
    // Clean up existing entries - shared pointers will automatically handle cleanup
    rulesetsGlobal.clear();
    rulesetsLocal.clear();

    // Check if doc is valid
    if (rulesetArray.memberType(rulesetArray.getRootScope()) != Data::KeyType::array) {
        return;
    }

    // Iterate through all entries
    for (auto const& key : rulesetArray.arrayKeys(rulesetArray.getRootScope())) {
        auto Ruleset = getRuleset(rulesetArray, key.view(), self);

        if (std::holds_alternative<std::monostate>(Ruleset)) {
            // Skip invalid entry
            continue;
        }

        if (std::holds_alternative<std::shared_ptr<StaticRuleset>>(Ruleset)) {
            // Static ruleset, push directly
            auto staticRulesetPtr = std::get<std::shared_ptr<StaticRuleset>>(Ruleset);
            staticRulesetPtr->estimatedCost = 1; // Static rulesets have minimal cost
            if (staticRulesetPtr->isGlobal()) {
                rulesetsGlobal.push_back(staticRulesetPtr);
            } else {
                rulesetsLocal.push_back(staticRulesetPtr);
            }
            continue;
        }

        if (std::holds_alternative<std::shared_ptr<JsonRuleset>>(Ruleset)) {
            // Optimize json-defined ruleset and push
            auto jsonRulesetPtr = std::get<std::shared_ptr<JsonRuleset>>(Ruleset);
            optimize(jsonRulesetPtr, self.domainScope);
            jsonRulesetPtr->estimateComputationalCost();
            if (jsonRulesetPtr->isGlobal()) {
                // If topic is empty, it is a local invoke
                rulesetsGlobal.push_back(jsonRulesetPtr);
            } else {
                rulesetsLocal.push_back(jsonRulesetPtr);
            }
        }
    }

    // Set necessary metadata
    setMetaData(rulesetsGlobal);
    setMetaData(rulesetsLocal);
}

void RulesetCompiler::optimize(std::shared_ptr<JsonRuleset> const& entry, Data::JsonScope& self) {
    ContextScope const context{self, self, Global::instance().domainScope};
    for (auto& assignment : entry->assignments) {
        assignment.optimize(context);
    }
}

RulesetCompiler::AnyRuleset RulesetCompiler::getRuleset(Data::JsonScope const& doc, Data::ScopedKeyView const& key, Execution::Domain& self) {
    Data::JsonScope entry;
    if (!getJsonRuleset(doc, entry, key)) {
        // See if it's a static ruleset
        auto const staticFunctionName = doc.get<std::string>(key).value_or("");

        if (
            auto const& staticRulesetEntry = StaticRulesetMap::getInstance().getStaticRulesetByName(staticFunctionName);
            staticRulesetEntry.type != StaticRuleset::Type::invalid
        ) {
            // Is a valid static ruleset
            auto Ruleset = std::make_shared<StaticRuleset>(self);
            if (staticRulesetEntry.type == StaticRuleset::Type::Global) {
                Ruleset->topic = staticRulesetEntry.topic;
            } else {
                Ruleset->topic = ""; // Local rulesets have no topic
            }
            Ruleset->staticFunction = staticRulesetEntry.function;
            Ruleset->baseListFunction = staticRulesetEntry.baseListFunc;
            Ruleset->slf = staticRulesetEntry.baseListFunc(self);
            return Ruleset;
        }
        // Skip this entry if it cannot be parsed
        // Warn user of invalid entry
        Global::capture().error.println("Could not parse Ruleset entry with string '", staticFunctionName, "'. Skipping entry.");
        return std::monostate{};
    }
    // Is a valid JSON-defined ruleset
    auto Ruleset = std::make_shared<JsonRuleset>(self);
    Ruleset->topic = entry.get<std::string>(Constants::KeyNames::Ruleset::topic).value_or("all");

    std::string const logicalArgStr = getCondition(entry);
    std::string_view lsa = logicalArgStr;
    Utility::StringHandler::strip(lsa);
    Ruleset->logicalArg = std::make_unique<Logic::Expression>(lsa);

    // Remove whitespaces at start and end from topic:
    std::string_view top = Ruleset->topic;
    Utility::StringHandler::strip(top);
    Ruleset->topic = top;

    // Get and parse all assignments
    getAssignments(Ruleset, entry);

    // Parse all function calls
    getFunctionCalls(entry, *Ruleset);

    // Push into vector
    return Ruleset;
}

std::optional<std::shared_ptr<Ruleset>> RulesetCompiler::parseSingle(std::string_view const identifier, Execution::Domain& self) {
    Data::JsonScope tempDoc;
    auto const root = Data::ScopedKey("");
    tempDoc.set(root, identifier);
    auto rs = getRuleset(tempDoc, root.view(), self);
    if (std::holds_alternative<std::shared_ptr<StaticRuleset>>(rs)) {
        return std::get<std::shared_ptr<StaticRuleset>>(rs);
    }
    if (std::holds_alternative<std::shared_ptr<JsonRuleset>>(rs)) {
        return std::get<std::shared_ptr<JsonRuleset>>(rs);
    }
    return std::nullopt;
}

} // namespace Nebulite::Interaction::Rules::Construction
