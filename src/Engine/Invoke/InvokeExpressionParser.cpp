#include "InvokeExpressionParser.h"
#include "InvokeNode.h"
#include "Invoke.h"
#include <algorithm>
#include <iostream>

namespace Nebulite {

InvokeExpressionParser::InvokeExpressionParser(Nebulite::Invoke* invoke) 
    : invoke(invoke) {
}

std::vector<std::string> InvokeExpressionParser::extractVariableReferences(const std::string& expression) {
    std::vector<std::string> variables;
    size_t pos = 0;
    
    while (pos < expression.size()) {
        size_t dollarPos = expression.find("$(", pos);
        if (dollarPos == std::string::npos) break;
        
        // Find matching closing parenthesis
        size_t openPos = dollarPos + 2;
        int depth = 1;
        size_t closePos = openPos;
        
        while (closePos < expression.size() && depth > 0) {
            if (expression[closePos] == '(') depth++;
            else if (expression[closePos] == ')') depth--;
            closePos++;
        }
        
        if (depth == 0) {
            std::string varContent = expression.substr(openPos, closePos - openPos - 1);
            // Only extract simple variable references (not nested expressions)
            if (varContent.find("$(") == std::string::npos) {
                variables.push_back(varContent);
            }
        }
        
        pos = closePos;
    }
    
    return variables;
}

std::string InvokeExpressionParser::convertToTinyExprFormat(const std::string& expression) {
    std::string result = expression;
    
    // Replace $(variable.path) with just variable_path for TinyExpr
    size_t pos = 0;
    while (pos < result.size()) {
        size_t dollarPos = result.find("$(", pos);
        if (dollarPos == std::string::npos) break;
        
        size_t openPos = dollarPos + 2;
        int depth = 1;
        size_t closePos = openPos;
        
        while (closePos < result.size() && depth > 0) {
            if (result[closePos] == '(') depth++;
            else if (result[closePos] == ')') depth--;
            closePos++;
        }
        
        if (depth == 0) {
            std::string varContent = result.substr(openPos, closePos - openPos - 1);
            // Only replace simple variables (no nested expressions)
            if (varContent.find("$(") == std::string::npos) {
                // Replace dots with underscores for TinyExpr variable names
                std::string varName = varContent;
                std::replace(varName.begin(), varName.end(), '.', '_');
                result.replace(dollarPos, closePos - dollarPos, varName);
                pos = dollarPos + varName.length();
            } else {
                pos = closePos;
            }
        } else {
            break;
        }
    }
    
    return result;
}

void InvokeExpressionParser::setupTinyExprVariables(Nebulite::InvokeNode& varNode, const std::string& expression) {
    // Lock the global TinyExpr operations to prevent race conditions
    std::lock_guard<std::mutex> lock(invoke->tinyexpr_mutex);
    
    // 1.) Get all variables: $(...)
    std::vector<std::string> foundVariables = extractVariableReferences(expression);
    
    // 2.) Clear existing data (ONLY touching additional setup variables)
    varNode.te_vars.clear();
    varNode.virtualDoubles.clear();
    varNode.variableNames.clear();
    varNode.te_evaluate = nullptr; // Clear any previous compilation
    
    // 3.) Store both original variable paths and TinyExpr compatible names
    for (const std::string& varPath : foundVariables) {
        // Store the original variable path (for creating VirtualDouble objects)
        varNode.variableNames.push_back(varPath);
        
        // We'll create VirtualDouble objects during evaluation when we have valid JSON pointers
        // For now, just prepare the te_variable structure with null pointer
        // Create TinyExpr compatible name for the te_variable
        std::string tinyExprName = varPath;
        std::replace(tinyExprName.begin(), tinyExprName.end(), '.', '_');
        
        varNode.te_vars.push_back({
            nullptr,  // Will be set during compilation with fresh c_str() pointer
            nullptr   // Will be set during evaluation
        });
    }
    
    // 4.) Store the TinyExpr format for later compilation
    // Don't compile yet - wait until we have valid variable pointers during evaluation
    std::string tinyExprFormat = convertToTinyExprFormat(expression);
    // Store the expression format in the text field or we could add a new field
    // For now, we'll recompute it during evaluation
}

void InvokeExpressionParser::updateTinyExprPointers(Nebulite::InvokeNode& varNode, 
                                                   Nebulite::JSON* self, 
                                                   Nebulite::JSON* other, 
                                                   Nebulite::JSON* global) {
    // Lock the global TinyExpr operations to prevent race conditions between threads
    // This ensures that only one thread can compile and update TinyExpr state at a time
    std::lock_guard<std::mutex> lock(invoke->tinyexpr_mutex);
                                                   
    // Set the JSON document pointers directly
    varNode.self = self;
    varNode.other = other;
    varNode.global = global;
    
    // Use the stored variable names from setupTinyExprVariables instead of re-extracting
    // This avoids potential corruption from re-parsing varNode.text
    
    /*
    std::cerr << "DEBUG: Found " << varNode.variableNames.size() << " variables for expression: " << varNode.text << std::endl;
    for (size_t i = 0; i < varNode.variableNames.size(); ++i) {
        std::cerr << "  Variable " << i << ": " << varNode.variableNames[i] << std::endl;
    }
    
    std::cerr << "DEBUG: Stored " << varNode.variableNames.size() << " variable names from setup" << std::endl;
    for (size_t i = 0; i < varNode.variableNames.size(); ++i) {
        std::cerr << "  Stored name " << i << ": " << varNode.variableNames[i] << std::endl;
    }
    */

    
    // Clear old VirtualDouble objects
    varNode.virtualDoubles.clear();
    
    // Create both VirtualDouble objects and TinyExpr compatible names
    std::vector<std::string> tinyExprNames;
    
    // Create VirtualDouble objects for each variable using stored original names
    for (size_t i = 0; i < varNode.variableNames.size() && i < varNode.te_vars.size(); ++i) {
        const std::string& varPath = varNode.variableNames[i]; // This is the original path like "self.x"
        Nebulite::JSON* targetDoc = nullptr;
        std::string key;
        
        if (varPath.starts_with("self.")) {
            targetDoc = varNode.self;
            key = varPath.substr(5);
        } else if (varPath.starts_with("other.")) {
            targetDoc = varNode.other;
            key = varPath.substr(6);
        } else if (varPath.starts_with("global.")) {
            targetDoc = varNode.global;
            key = varPath.substr(7);
        } else {
            // Skip variables we can't handle
            continue;
        }
        
        // Create VirtualDouble with valid JSON pointers
        varNode.virtualDoubles.push_back(
            std::make_unique<VirtualDouble>(targetDoc, key)
        );
        
        // Create TinyExpr compatible name
        std::string tinyExprName = varPath;
        std::replace(tinyExprName.begin(), tinyExprName.end(), '.', '_');
        tinyExprNames.push_back(tinyExprName);
    }
    
    // Now compile the TinyExpr expression (variables will be set during evaluation)
    std::string tinyExprFormat = convertToTinyExprFormat(varNode.text);
    
    // Free any existing compiled expression
    if (varNode.te_evaluate) {
        te_free(varNode.te_evaluate);
        varNode.te_evaluate = nullptr;
    }
    
    // Update te_variable pointers right before compilation
    // CRITICAL: Update both name and address pointers - names may have been invalidated!
    for (size_t i = 0; i < varNode.virtualDoubles.size() && i < varNode.te_vars.size() && i < tinyExprNames.size(); ++i) {
        varNode.te_vars[i].name = tinyExprNames[i].c_str();  // Fresh c_str() pointer
        
        // Check if VirtualDouble is valid before accessing
        if (varNode.virtualDoubles[i] != nullptr) {
            varNode.te_vars[i].address = varNode.virtualDoubles[i]->ptr();
            //std::cerr << "  te_var[" << i << "]: name='" << varNode.te_vars[i].name 
            //          << "' address=" << varNode.te_vars[i].address << std::endl;
        } else {
            std::cerr << "ERROR: VirtualDouble " << i << " is null for variable '" 
                      << varNode.variableNames[i] << "'" << std::endl;
            varNode.te_vars[i].address = nullptr;
        }
    }
    
    /*
    std::cerr << "DEBUG: About to compile: " << tinyExprFormat << std::endl;
    std::cerr << "DEBUG: With " << varNode.te_vars.size() << " variables" << std::endl;
    */
    
    
    // Compile with valid variable pointers
    int error = 0;
    varNode.te_evaluate = te_compile(
        tinyExprFormat.c_str(),
        varNode.te_vars.data(),
        static_cast<int>(varNode.te_vars.size()),
        &error
    );
    
    if (!varNode.te_evaluate) {
        std::cerr << "TinyExpr compilation failed for expression: " << tinyExprFormat 
                  << " (error code: " << error << ")" << std::endl;
    } else {
        //std::cerr << "TinyExpr compilation successful for: " << tinyExprFormat << std::endl;
    }
}

} // namespace Nebulite
