#include "Nebulite.hpp"
#include "Interaction/Rules/Ruleset.hpp"
#include "Interaction/Rules/Construction/RulesetCompiler.hpp"

namespace Nebulite::Interaction::Rules::Construction {

void RulesetCompiler::getFunctionCalls(Core::JsonScope& entryDoc, JsonRuleset& Ruleset, Execution::DomainBase const& self) {
    // Get function calls: GLOBAL, SELF, OTHER
    if (entryDoc.memberType(Constants::KeyNames::Ruleset::parseOnGlobal) == Data::KeyType::array) {
        size_t const funcSize = entryDoc.memberSize(Constants::KeyNames::Ruleset::parseOnGlobal);
        for (size_t j = 0; j < funcSize; ++j) {
            auto const funcKey = Constants::KeyNames::Ruleset::parseOnGlobal + "[" + std::to_string(j) + "]";
            auto funcCall = entryDoc.get<std::string>(funcKey, "");

            // Create a new Expression, parse the function call
            Logic::ExpressionPool invokeExpr;
            invokeExpr.parse(funcCall, self.getDoc());
            Ruleset.functioncalls_global.emplace_back(std::move(invokeExpr));
        }
    }
    if (entryDoc.memberType(Constants::KeyNames::Ruleset::parseOnSelf) == Data::KeyType::array) {
        size_t const funcSize = entryDoc.memberSize(Constants::KeyNames::Ruleset::parseOnSelf);
        for (size_t j = 0; j < funcSize; ++j) {
            auto const funcKey = Constants::KeyNames::Ruleset::parseOnSelf + "[" + std::to_string(j) + "]";
            auto funcCall = entryDoc.get<std::string>(funcKey, "");

            // The first arg has to be some reference of where the function is called
            // Global functions explicitly place the Binary name on the front in the global space
            // Here, we just reference "self" as the first argument
            if (!funcCall.starts_with("self ")) {
                funcCall.insert(0, "self");
            }

            // Create a new Expression, parse the function call
            Logic::ExpressionPool invokeExpr;
            invokeExpr.parse(funcCall, self.getDoc());
            Ruleset.functioncalls_self.emplace_back(std::move(invokeExpr));
        }
    }
    if (entryDoc.memberType(Constants::KeyNames::Ruleset::parseOnOther) == Data::KeyType::array) {
        size_t const funcSize = entryDoc.memberSize(Constants::KeyNames::Ruleset::parseOnOther);
        for (size_t j = 0; j < funcSize; ++j) {
            auto const funcKey = Constants::KeyNames::Ruleset::parseOnOther + "[" + std::to_string(j) + "]";
            auto funcCall = entryDoc.get<std::string>(funcKey, "");

            // The first arg has to be some reference of where the function is called
            // Global functions explicitly place the Binary name on the front in the global space
            // Here, we just reference "other" as the first argument
            if (!funcCall.starts_with("other ")) {
                funcCall.insert(0, "other");
            }
            // Create a new Expression, parse the function call
            Logic::ExpressionPool invokeExpr;
            invokeExpr.parse(funcCall, self.getDoc());
            Ruleset.functioncalls_other.emplace_back(std::move(invokeExpr));
        }
    }
}

bool RulesetCompiler::getExpression(Logic::Assignment& assignmentExpr, Core::JsonScope& entry, size_t const& index) {
    static std::string constexpr startSelf = "self.";
    static std::string constexpr startOther = "other.";
    static std::string constexpr startGlobal = "global.";

    auto const exprKey = Constants::KeyNames::Ruleset::assignments + "[" + std::to_string(index) + "]";

    // Get expression
    auto expr = entry.get<std::string>(exprKey, "");

    // needs to start with "self.", "other." or "global."
    std::string prefix;
    if (expr.starts_with(startSelf)) {
        assignmentExpr.onType = Logic::Assignment::Type::Self;
        prefix = startSelf;
    } else if (expr.starts_with(startOther)) {
        assignmentExpr.onType = Logic::Assignment::Type::Other;
        prefix = startOther;
    } else if (expr.starts_with(startGlobal)) {
        assignmentExpr.onType = Logic::Assignment::Type::Global;
        prefix = startGlobal;
    } else {
        // Invalid expression
        assignmentExpr.onType = Logic::Assignment::Type::null;
        return false;
    }

    // Find the operator position in the full expression, set operation, key and value
    if (size_t pos; (pos = expr.find("+=")) != std::string::npos) {
        assignmentExpr.operation = Logic::Assignment::Operation::add;
        assignmentExpr.value = expr.substr(pos + 2);
        assignmentExpr.keyStr = expr.substr(prefix.length(), pos - prefix.length());
    } else if ((pos = expr.find("*=")) != std::string::npos) {
        assignmentExpr.operation = Logic::Assignment::Operation::multiply;
        assignmentExpr.value = expr.substr(pos + 2);
        assignmentExpr.keyStr = expr.substr(prefix.length(), pos - prefix.length());
    } else if ((pos = expr.find("|=")) != std::string::npos) {
        assignmentExpr.operation = Logic::Assignment::Operation::concat;
        assignmentExpr.value = expr.substr(pos + 2);
        assignmentExpr.keyStr = expr.substr(prefix.length(), pos - prefix.length());
    } else if ((pos = expr.find("=")) != std::string::npos) {
        assignmentExpr.operation = Logic::Assignment::Operation::set;
        assignmentExpr.value = expr.substr(pos + 1);
        assignmentExpr.keyStr = expr.substr(prefix.length(), pos - prefix.length());
    } else {
        return false;
    }
    return true;
}

bool RulesetCompiler::getExpressions(std::shared_ptr<JsonRuleset> const& Ruleset, Core::JsonScope& entry, Core::JsonScope& self) {
    if (entry.memberType(Constants::KeyNames::Ruleset::assignments) == Data::KeyType::array) {
        size_t const exprSize = entry.memberSize(Constants::KeyNames::Ruleset::assignments);
        for (size_t j = 0; j < exprSize; ++j) {
            if (Logic::Assignment assignmentExpr; getExpression(assignmentExpr, entry, j)) {
                // Successfully parsed expression

                // Remove whitespaces at start and end of key and value
                assignmentExpr.key.parse(Utility::StringHandler::rStrip(Utility::StringHandler::lStrip(assignmentExpr.keyStr)), self);
                assignmentExpr.value = Utility::StringHandler::rStrip(Utility::StringHandler::lStrip(assignmentExpr.value));

                // Add assignmentExpr to Ruleset
                Ruleset->assignments.emplace_back(std::move(assignmentExpr));
            }
        }
    } else {
        // No expressions
        return false;
    }
    return true;
}

std::string RulesetCompiler::getCondition(Core::JsonScope& entry) {
    std::string logicalArg;
    if (entry.memberType(Constants::KeyNames::Ruleset::condition) == Data::KeyType::array) {
        size_t const logicalArgSize = entry.memberSize(Constants::KeyNames::Ruleset::condition);
        for (size_t j = 0; j < logicalArgSize; ++j) {
            auto logicalArgKey = Constants::KeyNames::Ruleset::condition + "[" + std::to_string(j) + "]";
            logicalArg += "(" + entry.get<std::string>(logicalArgKey, "0") + ")";
            if (j < logicalArgSize - 1) {
                logicalArg += "*"; // Arguments in vector need to be all true: &-logic -> Multiplication
            }
        }
    } else {
        // Assume simple value, string:
        logicalArg = entry.get<std::string>(Constants::KeyNames::Ruleset::condition, "0");
    }

    // Add $()
    if (!logicalArg.starts_with("$(")) {
        logicalArg = "$(" + logicalArg + ")";
    }

    return logicalArg;
}

bool RulesetCompiler::getJsonRuleset(Core::JsonScope& doc, Core::JsonScope& entry, Data::ScopedKeyView const& key) {
    if (doc.memberType(key) == Data::KeyType::object) {
        std::string const& serial = doc.shareScope(key).serialize();
        entry.deserialize(serial);
    } else {
        // Is perhaps link to document
        auto const potentialLink = doc.get<std::string>(key, "");
        if (potentialLink.starts_with("::")) {
            // Is a static ruleset, return false
            return false;
        }
        std::string const file = Nebulite::global().getDocCache().getDocString(potentialLink);

        if (file.empty()) {
            return false;
        }
        entry.deserialize(file);
    }
    return true;
}

void RulesetCompiler::setMetaData(
    Nebulite::Interaction::Execution::DomainBase& self,
    std::vector<std::shared_ptr<Nebulite::Interaction::Rules::Ruleset>> const& rulesetsLocal,
    std::vector<std::shared_ptr<Nebulite::Interaction::Rules::Ruleset>> const& rulesetsGlobal
    ) {
    // Set IDs
    auto const id = self.getDoc().get<uint32_t>(Nebulite::Constants::KeyNames::RenderObject::id, 0);
    for (auto const& entry : rulesetsLocal) {
        entry->id = id;
    }
    for (auto const& entry : rulesetsGlobal) {
        entry->id = id;
    }

    // Set indices
    for (uint32_t i = 0; i < rulesetsLocal.size(); ++i) {
        rulesetsLocal[i]->index = i;
    }
    for (uint32_t i = 0; i < rulesetsGlobal.size(); ++i) {
        rulesetsGlobal[i]->index = i;
    }
}

void RulesetCompiler::parse(std::vector<std::shared_ptr<Ruleset>>& rulesetsGlobal, std::vector<std::shared_ptr<Ruleset>>& rulesetsLocal, Interaction::Execution::DomainBase& self) {
    // Clean up existing entries - shared pointers will automatically handle cleanup
    rulesetsGlobal.clear();
    rulesetsLocal.clear();

    // Check if doc is valid
    if (self.getDoc().memberType(Constants::KeyNames::RenderObject::Ruleset::broadcast) != Data::KeyType::array) {
        return;
    }

    // Get size of entries
    size_t const size = self.getDoc().memberSize(Constants::KeyNames::RenderObject::Ruleset::broadcast);
    if (size == 0) {
        // Object has no rulesets
        return;
    }

    // Iterate through all entries
    for (size_t idx = 0; idx < size; ++idx) {
        // Parse entry into separate JSON object
        auto const key = Constants::KeyNames::RenderObject::Ruleset::broadcast + "[" + std::to_string(idx) + "]";
        auto Ruleset = getRuleset(self.getDoc(), key, self);

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
            optimize(jsonRulesetPtr, self.getDoc());
            jsonRulesetPtr->estimateComputationalCost();
            if (jsonRulesetPtr->_isGlobal) {
                // If topic is empty, it is a local invoke
                rulesetsGlobal.push_back(jsonRulesetPtr);
            } else {
                rulesetsLocal.push_back(jsonRulesetPtr);
            }
        }
    }

    // Set necessary metadata: IDs, indices
    setMetaData(self, rulesetsGlobal, rulesetsLocal);
}

void RulesetCompiler::optimize(std::shared_ptr<JsonRuleset> const& entry, Core::JsonScope& self) {
    // List of operations that are considered numeric and thus eligible for direct pointer assignment.
    // Any new numeric operation must be added here to benefit from optimization techniques in the Invoke class.
    std::array<Logic::Assignment::Operation,3> const numeric_operations = {
        Logic::Assignment::Operation::set,
        Logic::Assignment::Operation::add,
        Logic::Assignment::Operation::multiply
    };

    for (auto& assignment : entry->assignments) {
        if (assignment.onType == Logic::Assignment::Type::Self) {
            if (std::ranges::find(numeric_operations, assignment.operation) != std::ranges::end(numeric_operations)) {
                // Numeric operation on self, try to get a direct pointer
                if (double* ptr = self.getStableDoublePointer(Data::ScopedKey(assignment.key.eval(self))); ptr != nullptr) {
                    assignment.targetValuePtr = ptr;
                }
            }
        }
        if (assignment.onType == Logic::Assignment::Type::Global) {
            if (std::ranges::find(numeric_operations, assignment.operation) != std::ranges::end(numeric_operations)) {
                // Numeric operation on global, try to get a direct pointer
                if (double* ptr = Nebulite::global().getDoc().getStableDoublePointer(Data::ScopedKey(assignment.key.eval(self))); ptr != nullptr) {
                    assignment.targetValuePtr = ptr;
                }
            }
        }
    }
}

RulesetCompiler::AnyRuleset RulesetCompiler::getRuleset(Core::JsonScope& doc, Data::ScopedKeyView const& key, Execution::DomainBase& self) {
    Core::JsonScope entry;
    if (!getJsonRuleset(doc, entry, key)) {
        // See if it's a static ruleset
        auto staticFunctionName = doc.get<std::string>(key, "");
        auto staticRulesetEntry = StaticRulesetMap::getInstance().getStaticRulesetByName(staticFunctionName);
        if (staticRulesetEntry.type != StaticRulesetMap::StaticRuleSetWithMetaData::Type::invalid) {
            // Is a valid static ruleset
            auto Ruleset = std::make_shared<Interaction::Rules::StaticRuleset>();
            Ruleset->topic = staticRulesetEntry.topic;
            Ruleset->_isGlobal = (staticRulesetEntry.type == StaticRulesetMap::StaticRuleSetWithMetaData::Type::Global);
            Ruleset->staticFunction = staticRulesetEntry.function;
            Ruleset->selfPtr = &self; // Set self pointer, might be helpful even for static rulesets
            return Ruleset;
        }
        // Skip this entry if it cannot be parsed
        // Warn user of invalid entry
        Nebulite::cerr() << "Warning: could not parse Ruleset entry with string '" << staticFunctionName << "'. Skipping entry." << Nebulite::endl;
        return std::monostate{};
    }
    // Is a valid JSON-defined ruleset
    auto Ruleset = std::make_shared<Interaction::Rules::JsonRuleset>();
    Ruleset->topic = entry.get<std::string>(Constants::KeyNames::Ruleset::topic, "all");
    Ruleset->_isGlobal = (!Ruleset->topic.empty()); // If topic is empty, it is a local invoke
    Ruleset->logicalArg.parse(getCondition(entry), self.getDoc());

    // Remove whitespaces at start and end from topic and logicalArg:
    Ruleset->topic = Utility::StringHandler::rStrip(Utility::StringHandler::lStrip(Ruleset->topic));

    // If topic becomes empty after stripping, treat as local-only
    if (Ruleset->topic.empty()) {
        Ruleset->topic = ""; // Keep empty for local identification
    }

    std::string str = *Ruleset->logicalArg.getFullExpression();
    str = Utility::StringHandler::rStrip(Utility::StringHandler::lStrip(str));
    Ruleset->logicalArg.parse(str, self.getDoc());

    // Get and parse all assignments
    getExpressions(Ruleset, entry, self.getDoc());
    for (auto& assignment : Ruleset->assignments) {
        assignment.expression.parse(assignment.value, self.getDoc());
    }

    // Parse all function calls
    getFunctionCalls(entry, *Ruleset, self);

    // Push into vector
    Ruleset->selfPtr = &self; // Set self pointer
    return Ruleset;
}

std::optional<std::shared_ptr<Ruleset>> RulesetCompiler::parseSingle(std::string const& identifier, Interaction::Execution::DomainBase& self) {
    Core::JsonScope tempDoc;
    auto const root = Data::ScopedKey("");
    tempDoc.set(root, identifier);
    auto rs = getRuleset(tempDoc, root, self);
    if (std::holds_alternative<std::shared_ptr<StaticRuleset>>(rs)) {
        return std::get<std::shared_ptr<StaticRuleset>>(rs);
    }
    if (std::holds_alternative<std::shared_ptr<JsonRuleset>>(rs)) {
        return std::get<std::shared_ptr<JsonRuleset>>(rs);
    }
    return std::nullopt;
}

} // namespace Nebulite::Interaction::Rules
