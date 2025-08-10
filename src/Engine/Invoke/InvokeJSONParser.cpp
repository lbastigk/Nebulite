#include "InvokeJSONParser.h"

void Nebulite::InvokeJSONParser::getFunctionCalls(Nebulite::JSON& entryDoc, Nebulite::InvokeEntry& invokeEntry, Nebulite::DocumentCache* docCache) {
    // Get function calls: GLOBAL, SELF, OTHER
    if (entryDoc.memberCheck(keyName.invoke.functioncalls_global) == Nebulite::JSON::KeyType::array) {
        uint32_t funcSize = entryDoc.memberSize(keyName.invoke.functioncalls_global);
        for (uint32_t j = 0; j < funcSize; ++j) {
            std::string funcKey = keyName.invoke.functioncalls_global + "[" + std::to_string(j) + "]";
            std::string funcCall = entryDoc.get<std::string>(funcKey.c_str(), "");

            // Create a new InvokeExpression, parse the function call
            Nebulite::InvokeExpression invokeExpr;
            invokeExpr.parse(funcCall, *docCache);
            invokeEntry.functioncalls_global.push_back(invokeExpr);
        }
    }
    if (entryDoc.memberCheck(keyName.invoke.functioncalls_self) == Nebulite::JSON::KeyType::array) {
        uint32_t funcSize = entryDoc.memberSize(keyName.invoke.functioncalls_self);
        for (uint32_t j = 0; j < funcSize; ++j) {
            std::string funcKey = keyName.invoke.functioncalls_self + "[" + std::to_string(j) + "]";
            std::string funcCall = entryDoc.get<std::string>(funcKey.c_str(), "");

            // The first arg has to be some reference of where the function is called
            // Global functions explicitly place the Binary name on the front in the global space
            // Here, we just reference "self" as the first argument
            if (!funcCall.starts_with("self ")) {
                funcCall = "self " + funcCall;
            }

            // Create a new InvokeExpression, parse the function call
            Nebulite::InvokeExpression invokeExpr;
            invokeExpr.parse(funcCall, *docCache);
            invokeEntry.functioncalls_self.push_back(invokeExpr);
        }
    }
    if (entryDoc.memberCheck(keyName.invoke.functioncalls_other) == Nebulite::JSON::KeyType::array) {
        uint32_t funcSize = entryDoc.memberSize(keyName.invoke.functioncalls_other);
        for (uint32_t j = 0; j < funcSize; ++j) {
            std::string funcKey = keyName.invoke.functioncalls_other + "[" + std::to_string(j) + "]";
            std::string funcCall = entryDoc.get<std::string>(funcKey.c_str(), "");

            // The first arg has to be some reference of where the function is called
            // Global functions explicitly place the Binary name on the front in the global space
            // Here, we just reference "other" as the first argument
            if (!funcCall.starts_with("other ")) {
                funcCall = "other " + funcCall;
            }
            // Create a new InvokeExpression, parse the function call
            Nebulite::InvokeExpression invokeExpr;
            invokeExpr.parse(funcCall, *docCache);
            invokeEntry.functioncalls_other.push_back(invokeExpr);
        }
    }
}

bool Nebulite::InvokeJSONParser::getExpression(Nebulite::InvokeAssignmentExpression& assignmentExpr, Nebulite::JSON& entry, int index){
    std::string exprKey = keyName.invoke.exprVector + "[" + std::to_string(index) + "]";

    // Get expression
    std::string expr = entry.get<std::string>(exprKey.c_str(), "");

    // needs to start with "self.", "other." or "global."
    std::string prefix;
    if (expr.starts_with(keyName.invoke.typeSelf + ".")) {
        assignmentExpr.onType = Nebulite::InvokeAssignmentExpression::Type::Self;
        prefix = keyName.invoke.typeSelf + ".";
    } else if (expr.starts_with(keyName.invoke.typeOther + ".")) {
        assignmentExpr.onType = Nebulite::InvokeAssignmentExpression::Type::Other;
        prefix = keyName.invoke.typeOther + ".";
    } else if (expr.starts_with(keyName.invoke.typeGlobal + ".")) {
        assignmentExpr.onType = Nebulite::InvokeAssignmentExpression::Type::Global;
        prefix = keyName.invoke.typeGlobal + ".";
    } else {
        // Invalid expression
        assignmentExpr.onType = Nebulite::InvokeAssignmentExpression::Type::null;
        return false;
    }

    // Find the operator position in the full expression, set operation, key and value
    size_t pos = std::string::npos;
    if ((pos = expr.find("+=")) != std::string::npos) {
        assignmentExpr.operation = Nebulite::InvokeAssignmentExpression::Operation::add;
        assignmentExpr.value = expr.substr(pos + 2);
        assignmentExpr.key = expr.substr(prefix.length(), pos - prefix.length());
    } else if ((pos = expr.find("*=")) != std::string::npos) {
        assignmentExpr.operation = Nebulite::InvokeAssignmentExpression::Operation::multiply;
        assignmentExpr.value = expr.substr(pos + 2);
        assignmentExpr.key = expr.substr(prefix.length(), pos - prefix.length());
    } else if ((pos = expr.find("|=")) != std::string::npos) {
        assignmentExpr.operation = Nebulite::InvokeAssignmentExpression::Operation::concat;
        assignmentExpr.value = expr.substr(pos + 2);
        assignmentExpr.key = expr.substr(prefix.length(), pos - prefix.length());
    } else if ((pos = expr.find("=")) != std::string::npos) {
        assignmentExpr.operation = Nebulite::InvokeAssignmentExpression::Operation::set;
        assignmentExpr.value = expr.substr(pos + 1);
        assignmentExpr.key = expr.substr(prefix.length(), pos - prefix.length());
    } else {
        std::cerr << "No operation found in expression: " << expr << std::endl;
        return false;
    }

    return true;
}

std::string Nebulite::InvokeJSONParser::getLogicalArg(Nebulite::JSON& entry) {
    std::string logicalArg = "";
    if(entry.memberCheck("logicalArg") == Nebulite::JSON::KeyType::array){
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

bool Nebulite::InvokeJSONParser::getInvokeEntry(Nebulite::JSON& doc, Nebulite::JSON& entry, int index) {
    std::string key = keyName.renderObject.invokes + "[" + std::to_string(index) + "]";
    if(doc.memberCheck(key.c_str()) == Nebulite::JSON::KeyType::document) {
        entry = doc.get_subdoc(key.c_str());
    }
    else{
        // Is link to document
        std::string link = doc.get<std::string>(key.c_str(), "");
        std::string file = FileManagement::LoadFile(link);
        if (file.empty()) {
            return false;
        }
        entry.deserialize(file);
    }
    return true;
}

void Nebulite::InvokeJSONParser::parse(Nebulite::JSON& doc, std::vector<std::shared_ptr<Nebulite::InvokeEntry>>& entries_global, std::vector<std::shared_ptr<Nebulite::InvokeEntry>>& entries_local, Nebulite::RenderObject* self, Nebulite::DocumentCache* docCache) {
    // Clean up existing entries - shared pointers will automatically handle cleanup
    entries_global.clear();
    entries_local.clear();

    // Check if doc is valid
    if (doc.memberCheck(keyName.renderObject.invokes) != Nebulite::JSON::KeyType::array) {
        std::cerr << "Invokes field is not an array!" << std::endl;
        return;
    }

    // Get size of entries
    uint32_t size = doc.memberSize(keyName.renderObject.invokes);
    if (size == 0) {
        // Object has no invokes
        return;
    }

    // Iterate through all entries
    for (int i = 0; i < size; ++i) {
        // Parse entry into separate JSON object
        Nebulite::JSON entry;
        if (!InvokeJSONParser::getInvokeEntry(doc, entry, i)) {
            std::cerr << "Failed to get invoke entry at index " << i << std::endl;
            continue; // Skip this entry
        }

        // Parse into a structure
        auto invokeEntry = std::make_shared<Nebulite::InvokeEntry>();
        invokeEntry->topic = entry.get<std::string>(keyName.invoke.topic.c_str(), "all");
        invokeEntry->logicalArg.parse(InvokeJSONParser::getLogicalArg(entry), *docCache);

        // Remove whitespaces at start and end from topic and logicalArg:
        invokeEntry->topic = Nebulite::StringHandler::rstrip(Nebulite::StringHandler::lstrip(invokeEntry->topic));
        
        // If topic becomes empty after stripping, treat as local-only
        if (invokeEntry->topic.empty()) {
            invokeEntry->topic = ""; // Keep empty for local identification
        }

        std::string str = Nebulite::StringHandler::rstrip(Nebulite::StringHandler::lstrip(invokeEntry->logicalArg.getFullExpression()));
        invokeEntry->logicalArg.parse(str, *docCache);

        // Get expressions
        if (entry.memberCheck(keyName.invoke.exprVector) == Nebulite::JSON::KeyType::array) {
            uint32_t exprSize = entry.memberSize(keyName.invoke.exprVector);
            for (uint32_t j = 0; j < exprSize; ++j) {
                Nebulite::InvokeAssignmentExpression assignmentExpr;
                if (InvokeJSONParser::getExpression(assignmentExpr, entry, j)){
                    // Successfully parsed expression

                    // Remove whitespaces at start and end of key and value
                    assignmentExpr.key = Nebulite::StringHandler::rstrip(Nebulite::StringHandler::lstrip(assignmentExpr.key));
                    assignmentExpr.value = Nebulite::StringHandler::rstrip(Nebulite::StringHandler::lstrip(assignmentExpr.value));

                    // Add assignmentExpr to invokeEntry
                    invokeEntry->exprs.push_back(assignmentExpr);
                }
            }
        }
        else {
            std::cerr << "No expressions found in entry at index " << i << std::endl;
            continue; // Skip this entry if no expressions are found
        }

        // Parse all expressions
        uint32_t exprSize = entry.memberSize(keyName.invoke.exprVector);
        for (auto& expr : invokeEntry->exprs) {
            expr.expression.parse(expr.value, *docCache);
        }

        // Parse all function calls
        Nebulite::InvokeJSONParser::getFunctionCalls(entry, *invokeEntry, docCache);

        // Push into vector
        invokeEntry->selfPtr = self; // Set self pointer
        if(invokeEntry->topic.empty()){
            // If topic is empty, it is a local invoke
            invokeEntry->isGlobal = false; // Set isGlobal to false for local invokes
            entries_local.push_back(invokeEntry);
        } else {
            entries_global.push_back(invokeEntry);
        }
    }
}
