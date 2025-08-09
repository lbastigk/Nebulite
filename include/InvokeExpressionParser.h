#pragma once

#include "tinyexpr.h"
#include "VirtualDouble.h"
#include "JSON.h"
#include <string>
#include <vector>
#include <memory>

namespace Nebulite {

// Forward declarations
class InvokeNode;
class Invoke;

/**
 * InvokeExpressionParser - Helper class for TinyExpr integration with InvokeNode
 * 
 * This class handles the conversion of Nebulite's $(variable) syntax to TinyExpr format,
 * manages variable references, and provides optimized expression evaluation using TinyExpr.
 * 
 * Key responsibilities:
 * - Extract variable references from expressions ($(self.var), $(global.time), etc.)
 * - Convert Nebulite syntax to TinyExpr compatible format
 * - Setup and manage TinyExpr variables and compilation
 * - Handle VirtualDouble objects for dynamic variable access
 */
class InvokeExpressionParser {
public:
    /**
     * Constructor
     * @param invoke Pointer to the Invoke instance for accessing global methods
     */
    explicit InvokeExpressionParser(Nebulite::Invoke* invoke);

    /**
     * Extract all variable references from an expression string
     * Finds all $(variable.path) patterns and returns the variable paths
     * 
     * @param expression The expression string to parse
     * @return Vector of variable paths (e.g., "self.x", "global.time.dt")
     */
    std::vector<std::string> extractVariableReferences(const std::string& expression);

    /**
     * Convert expression with $(var) syntax to TinyExpr format
     * Replaces $(variable.path) with variable_path for TinyExpr compatibility
     * Also converts dots to underscores in variable names
     * 
     * @param expression The expression with Nebulite $(var) syntax
     * @return Expression string compatible with TinyExpr
     */
    std::string convertToTinyExprFormat(const std::string& expression);

    /**
     * Setup TinyExpr variables for faster evaluation of Mix_eval expressions
     * Prepares the InvokeNode with variable names and te_variable structures
     * Does not create VirtualDouble objects yet - that's done during evaluation
     * 
     * @param varNode The InvokeNode to setup for TinyExpr evaluation
     * @param expression The expression string to analyze for variables
     */
    void setupTinyExprVariables(Nebulite::InvokeNode& varNode, const std::string& expression);

    /**
     * Update variable pointers for TinyExpr evaluation
     * Creates VirtualDouble objects and compiles the TinyExpr expression
     * Must be called before each evaluation with current JSON document pointers
     * 
     * @param varNode The InvokeNode containing TinyExpr setup
     * @param self Pointer to self JSON document
     * @param other Pointer to other JSON document  
     * @param global Pointer to global JSON document
     */
    void updateTinyExprPointers(Nebulite::InvokeNode& varNode, 
                               Nebulite::JSON* self, 
                               Nebulite::JSON* other, 
                               Nebulite::JSON* global);

private:
    Nebulite::Invoke* invoke; ///< Pointer to the Invoke instance for accessing global variables and methods
};

} // namespace Nebulite
