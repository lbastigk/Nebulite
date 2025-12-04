#include "Interaction/RulesetCompiler.hpp"
#include "Interaction/Ruleset.hpp"
#include "Nebulite.hpp"

void Nebulite::Interaction::RulesetCompiler::getFunctionCalls(
    Utility::JSON& entryDoc,
    Ruleset& Ruleset,
    Core::RenderObject const* self
    ) {
    // Get function calls: GLOBAL, SELF, OTHER
    if (entryDoc.memberType(Constants::keyName.invoke.functioncalls_global) == Utility::JSON::KeyType::array) {
        size_t const funcSize = entryDoc.memberSize(Constants::keyName.invoke.functioncalls_global);
        for (size_t j = 0; j < funcSize; ++j) {
            std::string funcKey = Constants::keyName.invoke.functioncalls_global + "[" + std::to_string(j) + "]";
            auto funcCall = entryDoc.get<std::string>(funcKey, "");

            // Create a new Expression, parse the function call
            Logic::ExpressionPool invokeExpr;
            invokeExpr.parse(funcCall, self->getDoc());
            Ruleset.functioncalls_global.emplace_back(std::move(invokeExpr));
        }
    }
    if (entryDoc.memberType(Constants::keyName.invoke.functioncalls_self) == Utility::JSON::KeyType::array) {
        size_t const funcSize = entryDoc.memberSize(Constants::keyName.invoke.functioncalls_self);
        for (size_t j = 0; j < funcSize; ++j) {
            std::string funcKey = Constants::keyName.invoke.functioncalls_self + "[" + std::to_string(j) + "]";
            auto funcCall = entryDoc.get<std::string>(funcKey, "");

            // The first arg has to be some reference of where the function is called
            // Global functions explicitly place the Binary name on the front in the global space
            // Here, we just reference "self" as the first argument
            if (!funcCall.starts_with("self ")) {
                funcCall.insert(0, "self");
            }

            // Create a new Expression, parse the function call
            Logic::ExpressionPool invokeExpr;
            invokeExpr.parse(funcCall, self->getDoc());
            Ruleset.functioncalls_self.emplace_back(std::move(invokeExpr));
        }
    }
    if (entryDoc.memberType(Constants::keyName.invoke.functioncalls_other) == Utility::JSON::KeyType::array) {
        size_t const funcSize = entryDoc.memberSize(Constants::keyName.invoke.functioncalls_other);
        for (size_t j = 0; j < funcSize; ++j) {
            std::string funcKey = Constants::keyName.invoke.functioncalls_other + "[" + std::to_string(j) + "]";
            auto funcCall = entryDoc.get<std::string>(funcKey, "");

            // The first arg has to be some reference of where the function is called
            // Global functions explicitly place the Binary name on the front in the global space
            // Here, we just reference "other" as the first argument
            if (!funcCall.starts_with("other ")) {
                funcCall.insert(0, "other");
            }
            // Create a new Expression, parse the function call
            Logic::ExpressionPool invokeExpr;
            invokeExpr.parse(funcCall, self->getDoc());
            Ruleset.functioncalls_other.emplace_back(std::move(invokeExpr));
        }
    }
}

bool Nebulite::Interaction::RulesetCompiler::getExpression(Logic::Assignment& assignmentExpr, Utility::JSON& entry, size_t const& index) {
    auto const exprKey = Constants::keyName.invoke.exprVector + "[" + std::to_string(index) + "]";

    // Get expression
    auto expr = entry.get<std::string>(exprKey, "");

    // needs to start with "self.", "other." or "global."
    std::string prefix;
    if (expr.starts_with(Constants::keyName.invoke.typeSelf + ".")) {
        assignmentExpr.onType = Logic::Assignment::Type::Self;
        prefix = Constants::keyName.invoke.typeSelf + ".";
    } else if (expr.starts_with(Constants::keyName.invoke.typeOther + ".")) {
        assignmentExpr.onType = Logic::Assignment::Type::Other;
        prefix = Constants::keyName.invoke.typeOther + ".";
    } else if (expr.starts_with(Constants::keyName.invoke.typeGlobal + ".")) {
        assignmentExpr.onType = Logic::Assignment::Type::Global;
        prefix = Constants::keyName.invoke.typeGlobal + ".";
    } else {
        // Invalid expression
        assignmentExpr.onType = Logic::Assignment::Type::null;
        return false;
    }

    // Find the operator position in the full expression, set operation, key and value
    if (size_t pos; (pos = expr.find("+=")) != std::string::npos) {
        assignmentExpr.operation = Logic::Assignment::Operation::add;
        assignmentExpr.value = expr.substr(pos + 2);
        assignmentExpr.key = expr.substr(prefix.length(), pos - prefix.length());
    } else if ((pos = expr.find("*=")) != std::string::npos) {
        assignmentExpr.operation = Logic::Assignment::Operation::multiply;
        assignmentExpr.value = expr.substr(pos + 2);
        assignmentExpr.key = expr.substr(prefix.length(), pos - prefix.length());
    } else if ((pos = expr.find("|=")) != std::string::npos) {
        assignmentExpr.operation = Logic::Assignment::Operation::concat;
        assignmentExpr.value = expr.substr(pos + 2);
        assignmentExpr.key = expr.substr(prefix.length(), pos - prefix.length());
    } else if ((pos = expr.find("=")) != std::string::npos) {
        assignmentExpr.operation = Logic::Assignment::Operation::set;
        assignmentExpr.value = expr.substr(pos + 1);
        assignmentExpr.key = expr.substr(prefix.length(), pos - prefix.length());
    } else {
        return false;
    }
    return true;
}

bool Nebulite::Interaction::RulesetCompiler::getExpressions(std::shared_ptr<Ruleset> const& Ruleset, Utility::JSON* entry) {
    if (entry->memberType(Constants::keyName.invoke.exprVector) == Utility::JSON::KeyType::array) {
        size_t const exprSize = entry->memberSize(Constants::keyName.invoke.exprVector);
        for (size_t j = 0; j < exprSize; ++j) {
            if (Logic::Assignment assignmentExpr; getExpression(assignmentExpr, *entry, j)) {
                // Successfully parsed expression

                // Remove whitespaces at start and end of key and value
                assignmentExpr.key = Utility::StringHandler::rStrip(Utility::StringHandler::lStrip(assignmentExpr.key));
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

std::string Nebulite::Interaction::RulesetCompiler::getLogicalArg(Utility::JSON& entry) {
    std::string logicalArg;
    if (entry.memberType("logicalArg") == Utility::JSON::KeyType::array) {
        size_t const logicalArgSize = entry.memberSize("logicalArg");
        for (size_t j = 0; j < logicalArgSize; ++j) {
            std::string logicalArgKey = "logicalArg[" + std::to_string(j) + "]";
            logicalArg += "(" + entry.get<std::string>(logicalArgKey, "0") + ")";
            if (j < logicalArgSize - 1) {
                logicalArg += "*"; // Arguments in vector need to be all true: &-logic -> Multiplication
            }
        }
    } else {
        // Assume simple value, string:
        logicalArg = entry.get<std::string>("logicalArg", "0");
    }

    // Add $()
    if (!logicalArg.starts_with("$(")) {
        logicalArg = "$(" + logicalArg + ")";
    }

    return logicalArg;
}

bool Nebulite::Interaction::RulesetCompiler::getRuleset(Utility::JSON& doc, Utility::JSON& entry, size_t const& index) {
    if (std::string const key = Constants::keyName.renderObject.invokes + "[" + std::to_string(index) + "]"; doc.memberType(key) == Utility::JSON::KeyType::object) {
        entry = doc.getSubDoc(key);
    } else {
        // Is link to document
        auto const link = doc.get<std::string>(key, "");
        std::string const file = Nebulite::global().getDocCache()->getDocString(link);

        if (file.empty()) {
            return false;
        }
        entry.deserialize(file);
    }
    return true;
}

// Basic helpers for parsing
// Some of the functions above are probably better suited in the anonymous namespace as well to reduce the size of the header file.
// Do so later on if the RulesetCompiler class gets more complex.
// Especially once pre-compiled rulesets are implemented and need to be linked, which could drastically increase complexity of this class.
namespace {
/**
 * @brief Sets metadata in the object itself and in each Ruleset entry, including IDs, indices, and estimated computational cost.
 *
 * @param self The RenderObject that owns the entries.
 * @param entries_local The local Ruleset objects.
 * @param entries_global The global Ruleset objects.
 */
void setMetaData(
    Nebulite::Core::RenderObject* self,
    std::vector<std::shared_ptr<Nebulite::Interaction::Ruleset>> const& entries_local,
    std::vector<std::shared_ptr<Nebulite::Interaction::Ruleset>> const& entries_global
    ) {
    // Set IDs
    auto const id = self->getDoc()->get<uint32_t>(Nebulite::Constants::keyName.renderObject.id.c_str(), 0);
    for (auto const& entry : entries_local) {
        entry->id = id;
    }
    for (auto const& entry : entries_global) {
        entry->id = id;
    }

    // Set indices
    for (uint32_t i = 0; i < entries_local.size(); ++i) {
        entries_local[i]->index = i;
    }
    for (uint32_t i = 0; i < entries_global.size(); ++i) {
        entries_global[i]->index = i;
    }

    // Estimate full cost of each entry
    for (auto const& entry : entries_local) {
        entry->estimateComputationalCost();
    }
    for (auto const& entry : entries_global) {
        entry->estimateComputationalCost();
    }
}
}

void Nebulite::Interaction::RulesetCompiler::parse(std::vector<std::shared_ptr<Ruleset>>& entries_global, std::vector<std::shared_ptr<Ruleset>>& entries_local, Core::RenderObject* self) {
    // Clean up existing entries - shared pointers will automatically handle cleanup
    entries_global.clear();
    entries_local.clear();

    Utility::JSON* doc = self->getDoc();

    // Check if doc is valid
    if (doc->memberType(Constants::keyName.renderObject.invokes) != Utility::JSON::KeyType::array) {
        return;
    }

    // Get size of entries
    size_t const size = doc->memberSize(Constants::keyName.renderObject.invokes);
    if (size == 0) {
        // Object has no invokes
        return;
    }

    // Iterate through all entries
    for (size_t idx = 0; idx < size; ++idx) {
        // Parse entry into separate JSON object
        Utility::JSON entry;
        if (!getRuleset(*doc, entry, idx)) {
            continue; // Skip this entry
        }

        // Parse into a structure
        auto Ruleset = std::make_shared<Interaction::Ruleset>();
        Ruleset->topic = entry.get<std::string>(Constants::keyName.invoke.topic, "all");
        Ruleset->logicalArg.parse(getLogicalArg(entry), self->getDoc());

        // Remove whitespaces at start and end from topic and logicalArg:
        Ruleset->topic = Utility::StringHandler::rStrip(Utility::StringHandler::lStrip(Ruleset->topic));

        // If topic becomes empty after stripping, treat as local-only
        if (Ruleset->topic.empty()) {
            Ruleset->topic = ""; // Keep empty for local identification
        }

        std::string str = *Ruleset->logicalArg.getFullExpression();
        str = Utility::StringHandler::rStrip(Utility::StringHandler::lStrip(str));
        Ruleset->logicalArg.parse(str, self->getDoc());

        // Get expressions
        if (bool const exprSuccess = getExpressions(Ruleset, &entry); !exprSuccess) {
            continue; // Skip this entry if no expressions are found
        }

        // Parse all assignments
        for (auto& assignment : Ruleset->assignments) {
            assignment.expression.parse(assignment.value, self->getDoc());
        }

        // Parse all function calls
        getFunctionCalls(entry, *Ruleset, self);

        // Push into vector
        Ruleset->selfPtr = self; // Set self pointer
        if (Ruleset->topic.empty()) {
            // If topic is empty, it is a local invoke
            Ruleset->isGlobal = false; // Set isGlobal to false for local invokes
            entries_local.push_back(Ruleset);
        } else {
            entries_global.push_back(Ruleset);
        }
    }

    // See if we can assign a permanent target double pointer for each assignment
    optimizeParsedEntries(entries_global, self->getDoc());
    optimizeParsedEntries(entries_local, self->getDoc());

    // Set necessary metadata: IDs, indices, cost estimation
    setMetaData(self, entries_global, entries_local);
}

void Nebulite::Interaction::RulesetCompiler::optimizeParsedEntries(
    std::vector<std::shared_ptr<Ruleset>> const& entries,
    Utility::JSON* self
    ) {
    // Valid operations for direct double pointer assignment
    auto& ops = numeric_operations;

    // Checking all created entries, if any assignment is a numeric operation on self or global
    // we try to get a direct stable double pointer from the corresponding JSON document
    // If successful, we store the pointer in targetValuePtr of the assignment
    for (auto const& entry : entries) {
        for (auto& assignment : entry->assignments) {
            if (assignment.onType == Logic::Assignment::Type::Self) {
                if (std::ranges::find(ops, assignment.operation) != std::ranges::end(ops)) {
                    // Numeric operation on self, try to get a direct pointer
                    if (double* ptr = self->getStableDoublePointer(assignment.key); ptr != nullptr) {
                        assignment.targetValuePtr = ptr;
                    }
                }
            }
            if (assignment.onType == Logic::Assignment::Type::Global) {
                if (std::ranges::find(ops, assignment.operation) != std::ranges::end(ops)) {
                    // Numeric operation on global, try to get a direct pointer
                    if (double* ptr = Nebulite::global().getDoc()->getStableDoublePointer(assignment.key); ptr != nullptr) {
                        assignment.targetValuePtr = ptr;
                    }
                }
            }
        }
    }
}