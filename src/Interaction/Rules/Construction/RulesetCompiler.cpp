#include "Nebulite.hpp"
#include "Interaction/Rules/Ruleset.hpp"
#include "Interaction/Rules/Construction/RulesetCompiler.hpp"

namespace Nebulite::Interaction::Rules::Construction {

void RulesetCompiler::getFunctionCalls(Data::JsonScope const& entryDoc, JsonRuleset& Ruleset) {
    // Get function calls: GLOBAL, SELF, OTHER
    if (entryDoc.memberType(Constants::KeyNames::Ruleset::parseOnGlobal) == Data::KeyType::array) {
        size_t const funcSize = entryDoc.memberSize(Constants::KeyNames::Ruleset::parseOnGlobal);
        for (size_t j = 0; j < funcSize; ++j) {
            auto const funcKey = Constants::KeyNames::Ruleset::parseOnGlobal + "[" + std::to_string(j) + "]";
            auto funcCall = entryDoc.get<std::string>(funcKey).value_or("");

            // Create a new Expression, parse the function call
            Logic::Expression invokeExpr(funcCall);
            Ruleset.functioncalls_global.emplace_back(std::move(invokeExpr));
        }
    }
    if (entryDoc.memberType(Constants::KeyNames::Ruleset::parseOnSelf) == Data::KeyType::array) {
        size_t const funcSize = entryDoc.memberSize(Constants::KeyNames::Ruleset::parseOnSelf);
        for (size_t j = 0; j < funcSize; ++j) {
            auto const funcKey = Constants::KeyNames::Ruleset::parseOnSelf + "[" + std::to_string(j) + "]";
            auto funcCall = entryDoc.get<std::string>(funcKey).value_or("");

            // The first arg has to be some reference of where the function is called
            // Global functions explicitly place the Binary name on the front in the global space
            // Here, we just reference "self" as the first argument
            if (!funcCall.starts_with("self ")) {
                funcCall.insert(0, "self ");
            }

            // Create a new Expression, parse the function call
            Logic::Expression invokeExpr(funcCall);
            Ruleset.functioncalls_self.emplace_back(std::move(invokeExpr));
        }
    }
    if (entryDoc.memberType(Constants::KeyNames::Ruleset::parseOnOther) == Data::KeyType::array) {
        size_t const funcSize = entryDoc.memberSize(Constants::KeyNames::Ruleset::parseOnOther);
        for (size_t j = 0; j < funcSize; ++j) {
            auto const funcKey = Constants::KeyNames::Ruleset::parseOnOther + "[" + std::to_string(j) + "]";
            auto funcCall = entryDoc.get<std::string>(funcKey).value_or("");

            // The first arg has to be some reference of where the function is called
            // Global functions explicitly place the Binary name on the front in the global space
            // Here, we just reference "other" as the first argument
            if (!funcCall.starts_with("other ")) {
                funcCall.insert(0, "other ");
            }
            // Create a new Expression, parse the function call
            Logic::Expression invokeExpr(funcCall);
            Ruleset.functioncalls_other.emplace_back(std::move(invokeExpr));
        }
    }
}

std::optional<Logic::Assignment> RulesetCompiler::getAssignment(Data::JsonScope const& entry, size_t const& index) {
    auto const exprKey = Constants::KeyNames::Ruleset::assignments + "[" + std::to_string(index) + "]";
    if (Logic::Assignment assignment; assignment.parse(entry.get<std::string>(exprKey).value_or(""))) {
        return assignment;
    }
    return std::nullopt;
}

bool RulesetCompiler::getAssignments(std::shared_ptr<JsonRuleset> const& Ruleset, Data::JsonScope const& entry) {
    if (entry.memberType(Constants::KeyNames::Ruleset::assignments) == Data::KeyType::array) {
        size_t const exprSize = entry.memberSize(Constants::KeyNames::Ruleset::assignments);
        for (size_t j = 0; j < exprSize; ++j) {
            if (auto v = getAssignment(entry, j); v.has_value()) {
                Ruleset->assignments.emplace_back(std::move(*v));
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
        size_t const logicalArgSize = entry.memberSize(Constants::KeyNames::Ruleset::condition);
        for (size_t j = 0; j < logicalArgSize; ++j) {
            auto logicalArgKey = Constants::KeyNames::Ruleset::condition + "[" + std::to_string(j) + "]";
            logicalArg += "(" + entry.get<std::string>(logicalArgKey).value_or("0") + ")";
            if (j < logicalArgSize - 1) {
                logicalArg += "*"; // Arguments in vector need to be all true: &-logic -> Multiplication
            }
        }
    } else {
        // Assume simple value, string:
        logicalArg = entry.get<std::string>(Constants::KeyNames::Ruleset::condition).value_or("0");
    }

    // Add $()
    if (!logicalArg.starts_with("$(")) {
        logicalArg = "$(" + logicalArg + ")";
    }

    return logicalArg;
}

bool RulesetCompiler::getJsonRuleset(Data::JsonScope const& doc, Data::JsonScope const& entry, Data::ScopedKeyView const& key) {
    if (doc.memberType(key) == Data::KeyType::object) {
        std::string const& serial = doc.serialize(key);
        entry.deserialize(serial);
    } else {
        // Is perhaps link to document
        auto const potentialLink = doc.get<std::string>(key).value_or("");
        if (potentialLink.starts_with("::")) {
            // Is a static ruleset, return false
            return false;
        }
        std::string const file = Global::instance().getDocCache().getDocString(potentialLink);

        if (file.empty()) {
            return false;
        }
        entry.deserialize(file);
    }
    return true;
}

void RulesetCompiler::setMetaData(RulesetVector const& rulesetsLocal, RulesetVector const& rulesetsGlobal) {
    // Set indices
    for (uint32_t i = 0; i < rulesetsLocal.size(); ++i) {
        rulesetsLocal[i]->index = i;
    }
    for (uint32_t i = 0; i < rulesetsGlobal.size(); ++i) {
        rulesetsGlobal[i]->index = i;
    }
}

void RulesetCompiler::parse(RulesetVector& rulesetsGlobal, RulesetVector& rulesetsLocal, Execution::Domain& self) {
    // Clean up existing entries - shared pointers will automatically handle cleanup
    rulesetsGlobal.clear();
    rulesetsLocal.clear();

    // Check if doc is valid
    if (self.domainScope.memberType(Constants::KeyNames::RenderObject::Ruleset::list) != Data::KeyType::array) {
        return;
    }

    // Get size of entries
    size_t const size = self.domainScope.memberSize(Constants::KeyNames::RenderObject::Ruleset::list);
    if (size == 0) {
        // Object has no rulesets
        return;
    }

    // Iterate through all entries
    for (size_t idx = 0; idx < size; ++idx) {
        // Parse entry into separate JSON object
        auto const key = Constants::KeyNames::RenderObject::Ruleset::list + "[" + std::to_string(idx) + "]";
        auto Ruleset = getRuleset(self.domainScope, key.view(), self);

        if (std::holds_alternative<std::monostate>(Ruleset)) {
            // Skip invalid entry
            continue;
        }

        if (std::holds_alternative<std::shared_ptr<StaticRuleset>>(Ruleset)) {
            // Static ruleset, push directly
            auto staticRulesetPtr = std::get<std::shared_ptr<StaticRuleset>>(Ruleset);
            staticRulesetPtr->estimatedCost = 1; // Static rulesets have minimal cost
            if (staticRulesetPtr->_isGlobal) {
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
            if (jsonRulesetPtr->_isGlobal) {
                // If topic is empty, it is a local invoke
                rulesetsGlobal.push_back(jsonRulesetPtr);
            } else {
                rulesetsLocal.push_back(jsonRulesetPtr);
            }
        }
    }

    // Set necessary metadata
    setMetaData(rulesetsGlobal, rulesetsLocal);
}

void RulesetCompiler::optimize(std::shared_ptr<JsonRuleset> const& entry, Data::JsonScope& self) {
    ContextScope const context{self, self, Global::instance().domainScope};
    for (auto& assignment : entry->assignments) {
        assignment.optimize(context);
    }
}

RulesetCompiler::AnyRuleset RulesetCompiler::getRuleset(Data::JsonScope const& doc, Data::ScopedKeyView const& key, Execution::Domain& self) {
    Data::JsonScope const entry;
    if (!getJsonRuleset(doc, entry, key)) {
        // See if it's a static ruleset
        auto const staticFunctionName = doc.get<std::string>(key).value_or("");

        if (
            auto const staticRulesetEntry = StaticRulesetMap::getInstance().getStaticRulesetByName(staticFunctionName);
            staticRulesetEntry.type != StaticRulesetMap::StaticRuleSetWithMetaData::Type::invalid
        ) {
            // Is a valid static ruleset
            auto Ruleset = std::make_shared<StaticRuleset>(self);
            Ruleset->topic = staticRulesetEntry.topic;
            Ruleset->_isGlobal = staticRulesetEntry.type == StaticRulesetMap::StaticRuleSetWithMetaData::Type::Global;
            Ruleset->staticFunction = staticRulesetEntry.function;
            Ruleset->slf = staticRulesetEntry.baseListFunc(self);
            return Ruleset;
        }
        // Skip this entry if it cannot be parsed
        // Warn user of invalid entry
        Global::capture().error.println("Warning: could not parse Ruleset entry with string '", staticFunctionName, "'. Skipping entry.");
        return std::monostate{};
    }
    // Is a valid JSON-defined ruleset
    auto Ruleset = std::make_shared<JsonRuleset>(self);
    Ruleset->topic = entry.get<std::string>(Constants::KeyNames::Ruleset::topic).value_or("all");
    Ruleset->_isGlobal = !Ruleset->topic.empty(); // If topic is empty, it is a local invoke
    std::string logicalArgStr = getCondition(entry);
    logicalArgStr = Utility::StringHandler::rStrip(Utility::StringHandler::lStrip(logicalArgStr));
    Ruleset->logicalArg = std::make_unique<Logic::ExpressionPool>(logicalArgStr);

    // Remove whitespaces at start and end from topic and logicalArg:
    Ruleset->topic = Utility::StringHandler::rStrip(Utility::StringHandler::lStrip(Ruleset->topic));

    // If topic becomes empty after stripping, treat as local-only
    if (Ruleset->topic.empty()) {
        Ruleset->topic = ""; // Keep empty for local identification
    }

    // Get and parse all assignments
    getAssignments(Ruleset, entry);

    // Parse all function calls
    getFunctionCalls(entry, *Ruleset);

    // Push into vector
    return Ruleset;
}

std::optional<std::shared_ptr<Ruleset>> RulesetCompiler::parseSingle(std::string const& identifier, Execution::Domain& self) {
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

} // namespace Nebulite::Interaction::Rules
