#include "Interaction/RulesetCompiler.hpp"

#include "Interaction/Ruleset.hpp"

void Nebulite::Interaction::RulesetCompiler::getFunctionCalls(
    Nebulite::Utility::JSON& entryDoc,
    Nebulite::Interaction::Ruleset& Ruleset, 
    Nebulite::Core::RenderObject* self,
    Nebulite::Utility::DocumentCache* docCache,
    Nebulite::Utility::JSON* global
) {
    // Get function calls: GLOBAL, SELF, OTHER
    if (entryDoc.memberCheck(Nebulite::Constants::keyName.invoke.functioncalls_global) == Nebulite::Utility::JSON::KeyType::array) {
        uint32_t funcSize = entryDoc.memberSize(Nebulite::Constants::keyName.invoke.functioncalls_global);
        for (uint32_t j = 0; j < funcSize; ++j) {
            std::string funcKey = Nebulite::Constants::keyName.invoke.functioncalls_global + "[" + std::to_string(j) + "]";
            std::string funcCall = entryDoc.get<std::string>(funcKey.c_str(), "");

            // Create a new Expression, parse the function call
            Nebulite::Interaction::Logic::ExpressionPool invokeExpr;
            invokeExpr.parse(funcCall, *docCache, self->getDoc(), global);
            Ruleset.functioncalls_global.emplace_back(std::move(invokeExpr));
        }
    }
    if (entryDoc.memberCheck(Nebulite::Constants::keyName.invoke.functioncalls_self) == Nebulite::Utility::JSON::KeyType::array) {
        uint32_t funcSize = entryDoc.memberSize(Nebulite::Constants::keyName.invoke.functioncalls_self);
        for (uint32_t j = 0; j < funcSize; ++j) {
            std::string funcKey = Nebulite::Constants::keyName.invoke.functioncalls_self + "[" + std::to_string(j) + "]";
            std::string funcCall = entryDoc.get<std::string>(funcKey.c_str(), "");

            // The first arg has to be some reference of where the function is called
            // Global functions explicitly place the Binary name on the front in the global space
            // Here, we just reference "self" as the first argument
            if (!funcCall.starts_with("self ")) {
                funcCall = "self " + funcCall;
            }

            // Create a new Expression, parse the function call
            Nebulite::Interaction::Logic::ExpressionPool invokeExpr;
            invokeExpr.parse(funcCall, *docCache, self->getDoc(), global);
            Ruleset.functioncalls_self.emplace_back(std::move(invokeExpr));
        }
    }
    if (entryDoc.memberCheck(Nebulite::Constants::keyName.invoke.functioncalls_other) == Nebulite::Utility::JSON::KeyType::array) {
        uint32_t funcSize = entryDoc.memberSize(Nebulite::Constants::keyName.invoke.functioncalls_other);
        for (uint32_t j = 0; j < funcSize; ++j) {
            std::string funcKey = Nebulite::Constants::keyName.invoke.functioncalls_other + "[" + std::to_string(j) + "]";
            std::string funcCall = entryDoc.get<std::string>(funcKey.c_str(), "");

            // The first arg has to be some reference of where the function is called
            // Global functions explicitly place the Binary name on the front in the global space
            // Here, we just reference "other" as the first argument
            if (!funcCall.starts_with("other ")) {
                funcCall = "other " + funcCall;
            }
            // Create a new Expression, parse the function call
            Nebulite::Interaction::Logic::ExpressionPool invokeExpr;
            invokeExpr.parse(funcCall, *docCache, self->getDoc(), global);
            Ruleset.functioncalls_other.emplace_back(std::move(invokeExpr));
        }
    }
}

bool Nebulite::Interaction::RulesetCompiler::getExpression(Nebulite::Interaction::Logic::Assignment& assignmentExpr, Nebulite::Utility::JSON& entry, int index){
    std::string exprKey = Nebulite::Constants::keyName.invoke.exprVector + "[" + std::to_string(index) + "]";

    // Get expression
    std::string expr = entry.get<std::string>(exprKey.c_str(), "");

    // needs to start with "self.", "other." or "global."
    std::string prefix;
    if (expr.starts_with(Nebulite::Constants::keyName.invoke.typeSelf + ".")) {
        assignmentExpr.onType = Nebulite::Interaction::Logic::Assignment::Type::Self;
        prefix = Nebulite::Constants::keyName.invoke.typeSelf + ".";
    } else if (expr.starts_with(Nebulite::Constants::keyName.invoke.typeOther + ".")) {
        assignmentExpr.onType = Nebulite::Interaction::Logic::Assignment::Type::Other;
        prefix = Nebulite::Constants::keyName.invoke.typeOther + ".";
    } else if (expr.starts_with(Nebulite::Constants::keyName.invoke.typeGlobal + ".")) {
        assignmentExpr.onType = Nebulite::Interaction::Logic::Assignment::Type::Global;
        prefix = Nebulite::Constants::keyName.invoke.typeGlobal + ".";
    } else {
        // Invalid expression
        assignmentExpr.onType = Nebulite::Interaction::Logic::Assignment::Type::null;
        return false;
    }

    // Find the operator position in the full expression, set operation, key and value
    size_t pos = std::string::npos;
    if ((pos = expr.find("+=")) != std::string::npos) {
        assignmentExpr.operation = Nebulite::Interaction::Logic::Assignment::Operation::add;
        assignmentExpr.value = expr.substr(pos + 2);
        assignmentExpr.key = expr.substr(prefix.length(), pos - prefix.length());
    } else if ((pos = expr.find("*=")) != std::string::npos) {
        assignmentExpr.operation = Nebulite::Interaction::Logic::Assignment::Operation::multiply;
        assignmentExpr.value = expr.substr(pos + 2);
        assignmentExpr.key = expr.substr(prefix.length(), pos - prefix.length());
    } else if ((pos = expr.find("|=")) != std::string::npos) {
        assignmentExpr.operation = Nebulite::Interaction::Logic::Assignment::Operation::concat;
        assignmentExpr.value = expr.substr(pos + 2);
        assignmentExpr.key = expr.substr(prefix.length(), pos - prefix.length());
    } else if ((pos = expr.find("=")) != std::string::npos) {
        assignmentExpr.operation = Nebulite::Interaction::Logic::Assignment::Operation::set;
        assignmentExpr.value = expr.substr(pos + 1);
        assignmentExpr.key = expr.substr(prefix.length(), pos - prefix.length());
    } else {
        std::cerr << "No operation found in expression: " << expr << std::endl;
        return false;
    }

    return true;
}

bool Nebulite::Interaction::RulesetCompiler::getExpressions(Nebulite::Interaction::Ruleset* Ruleset, Nebulite::Utility::JSON* entry) {
    if (entry->memberCheck(Nebulite::Constants::keyName.invoke.exprVector) == Nebulite::Utility::JSON::KeyType::array) {
        uint32_t exprSize = entry->memberSize(Nebulite::Constants::keyName.invoke.exprVector);
        for (uint32_t j = 0; j < exprSize; ++j) {
            Nebulite::Interaction::Logic::Assignment assignmentExpr;
            if (RulesetCompiler::getExpression(assignmentExpr, *entry, j)){
                // Successfully parsed expression

                // Remove whitespaces at start and end of key and value
                assignmentExpr.key = Nebulite::Utility::StringHandler::rstrip(Nebulite::Utility::StringHandler::lstrip(assignmentExpr.key));
                assignmentExpr.value = Nebulite::Utility::StringHandler::rstrip(Nebulite::Utility::StringHandler::lstrip(assignmentExpr.value));

                // Add assignmentExpr to Ruleset
                Ruleset->assignments.emplace_back(std::move(assignmentExpr));
            }
        }
    }
    else {
        // No expressions
        return false;
    }
    return true;
}

std::string Nebulite::Interaction::RulesetCompiler::getLogicalArg(Nebulite::Utility::JSON& entry) {
    std::string logicalArg = "";
    if(entry.memberCheck("logicalArg") == Nebulite::Utility::JSON::KeyType::array){
        uint32_t logicalArgSize = entry.memberSize("logicalArg");
        for(uint32_t j = 0; j < logicalArgSize; ++j) {
            std::string logicalArgKey = "logicalArg[" + std::to_string(j) + "]";
            logicalArg += "(" + entry.get<std::string>(logicalArgKey.c_str(), "0") + ")";
            if (j < logicalArgSize - 1) {
                logicalArg += "*"; // Arguments in vector need to be all true: &-logic -> Multiplication
            }
        }
    }
    else{
        // Assume simple value, string:
        logicalArg = entry.get<std::string>("logicalArg", "0");
    }

    // Add $()
    if(!logicalArg.starts_with("$(")) {
        logicalArg = "$(" + logicalArg + ")";
    }

    return logicalArg;
}

bool Nebulite::Interaction::RulesetCompiler::getRuleset(Nebulite::Utility::JSON& doc, Nebulite::Utility::JSON& entry, int index) {
    std::string key = Nebulite::Constants::keyName.renderObject.invokes + "[" + std::to_string(index) + "]";
    if(doc.memberCheck(key.c_str()) == Nebulite::Utility::JSON::KeyType::document) {
        entry = doc.get_subdoc(key.c_str());
    }
    else{
        // Is link to document
        std::string link = doc.get<std::string>(key.c_str(), "");
        std::string file = Nebulite::Utility::FileManagement::LoadFile(link);
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
namespace{
    /**
     * @brief Sets metadata in the object itself and in each Ruleset entry, including IDs, indices, and estimated computational cost.
     * 
     * @param self The RenderObject that owns the entries.
     * @param entries_local The local Ruleset objects.
     * @param entries_global The global Ruleset objects.
     */
    void setMetaData(Nebulite::Core::RenderObject* self, std::vector<std::shared_ptr<Nebulite::Interaction::Ruleset>>& entries_local, std::vector<std::shared_ptr<Nebulite::Interaction::Ruleset>>& entries_global){
        // Set IDs
        uint32_t id = self->get<uint32_t>(Nebulite::Constants::keyName.renderObject.id.c_str(), 0);
        for (const auto& entry : entries_local) {
            entry->id = id;
        }
        for (const auto& entry : entries_global) {
            entry->id = id;
        }

        // Set indices
        for (size_t i = 0; i < entries_local.size(); ++i) {
            entries_local[i]->index = static_cast<uint32_t>(i);
        }
        for (size_t i = 0; i < entries_global.size(); ++i) {
            entries_global[i]->index = static_cast<uint32_t>(i);
        }

        // Estimate full cost of each entry
        for (const auto& entry : entries_local) {
            entry->estimateComputationalCost();
        }
        for (const auto& entry : entries_global) {
            entry->estimateComputationalCost();
        }
    }
}

void Nebulite::Interaction::RulesetCompiler::parse(std::vector<std::shared_ptr<Nebulite::Interaction::Ruleset>>& entries_global, std::vector<std::shared_ptr<Nebulite::Interaction::Ruleset>>& entries_local, Nebulite::Core::RenderObject* self, Nebulite::Utility::DocumentCache* docCache, Nebulite::Utility::JSON* global) {
    // Clean up existing entries - shared pointers will automatically handle cleanup
    entries_global.clear();
    entries_local.clear();

    Nebulite::Utility::JSON* doc = self->getDoc();

    // Check if doc is valid
    if (doc->memberCheck(Nebulite::Constants::keyName.renderObject.invokes) != Nebulite::Utility::JSON::KeyType::array) {
        std::cerr << "Invokes field is not an array!" << std::endl;
        return;
    }

    // Get size of entries
    uint32_t size = doc->memberSize(Nebulite::Constants::keyName.renderObject.invokes);
    if (size == 0) {
        // Object has no invokes
        return;
    }

    // Iterate through all entries
    for (int i = 0; i < size; ++i) {
        // Parse entry into separate JSON object
        Nebulite::Utility::JSON entry(self->getGlobalSpace());
        if (!RulesetCompiler::getRuleset(*doc, entry, i)) {
            std::cerr << "Failed to get invoke entry at index " << i << std::endl;
            continue; // Skip this entry
        }

        // Parse into a structure
        auto Ruleset = std::make_shared<Nebulite::Interaction::Ruleset>();
        Ruleset->topic = entry.get<std::string>(Nebulite::Constants::keyName.invoke.topic.c_str(), "all");
        Ruleset->logicalArg.parse(RulesetCompiler::getLogicalArg(entry), *docCache, self->getDoc(), global);

        // Remove whitespaces at start and end from topic and logicalArg:
        Ruleset->topic = Nebulite::Utility::StringHandler::rstrip(Nebulite::Utility::StringHandler::lstrip(Ruleset->topic));
        
        // If topic becomes empty after stripping, treat as local-only
        if (Ruleset->topic.empty()) {
            Ruleset->topic = ""; // Keep empty for local identification
        }

        std::string str = *Ruleset->logicalArg.getFullExpression();
        str = Nebulite::Utility::StringHandler::rstrip(Nebulite::Utility::StringHandler::lstrip(str));
        Ruleset->logicalArg.parse(str, *docCache, self->getDoc(), global);

        // Get expressions
        bool exprSuccess = RulesetCompiler::getExpressions(Ruleset.get(), &entry);
        if (!exprSuccess) {
            std::cerr << "No expressions found in entry at index " << i << std::endl;
            continue; // Skip this entry if no expressions are found
        }

        // Parse all expressions
        uint32_t exprSize = entry.memberSize(Nebulite::Constants::keyName.invoke.exprVector);
        for (auto& assignment : Ruleset->assignments) {
            assignment.expression.parse(assignment.value, *docCache, self->getDoc(), global);
        }

        // Parse all function calls
        Nebulite::Interaction::RulesetCompiler::getFunctionCalls(entry, *Ruleset, self, docCache, global);

        // Push into vector
        Ruleset->selfPtr = self; // Set self pointer
        if(Ruleset->topic.empty()){
            // If topic is empty, it is a local invoke
            Ruleset->isGlobal = false; // Set isGlobal to false for local invokes
            entries_local.push_back(Ruleset);
        } else {
            entries_global.push_back(Ruleset);
        }
    }

    // See if we can assign a permanent target double pointer for each assignment
    optimizeParsedEntries(entries_global, self->getDoc(), global);
    optimizeParsedEntries(entries_local, self->getDoc(), global);

    // Set necessary metadata: IDs, indices, cost estimation
    setMetaData(self, entries_global, entries_local);
}

void Nebulite::Interaction::RulesetCompiler::optimizeParsedEntries(
    std::vector<std::shared_ptr<Nebulite::Interaction::Ruleset>>& entries, 
    Nebulite::Utility::JSON* self,
    Nebulite::Utility::JSON* global
){
    // Valid operations for direct double pointer assignment
    auto ops = Nebulite::Interaction::RulesetCompiler::numeric_operations;

    // Checking all created entries, if any assignment is a numeric operation on self or global
    // we try to get a direct stable double pointer from the corresponding JSON document
    // If successful, we store the pointer in targetValuePtr of the assignment
    for (const auto& entry : entries) {
        for (auto& assignment : entry->assignments) {
            if (assignment.onType == Nebulite::Interaction::Logic::Assignment::Type::Self) {
                auto ops = Nebulite::Interaction::RulesetCompiler::numeric_operations;
                if (ops.end() != std::find(ops.begin(), ops.end(), assignment.operation)) {
                    // Numeric operation on self, try to get a direct pointer
                    double* ptr = self->getDoc()->get_stable_double_ptr(assignment.key);
                    if (ptr != nullptr) {
                        assignment.targetValuePtr = ptr;
                    }
                }
            }
            if (assignment.onType == Nebulite::Interaction::Logic::Assignment::Type::Global) {
                if (ops.end() != std::find(ops.begin(), ops.end(), assignment.operation)) {
                    // Numeric operation on global, try to get a direct pointer
                    double* ptr = global->get_stable_double_ptr(assignment.key);
                    if (ptr != nullptr) {
                        assignment.targetValuePtr = ptr;
                    }
                }
            }
        }
    }
}