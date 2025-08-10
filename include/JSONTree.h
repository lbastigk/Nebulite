/*
===========================================================
GlobalSpaceTree - Function Tree for JSON Modification
===========================================================

This class extends FuncTreeWrapper<ERROR_TYPE> to provide
a focused, self-contained parsing interface (functioncalls)
for Nebulite's JSON data container.
*/

#pragma once

//----------------------------------------------------------
// Basic includes
#include "ErrorTypes.h"  // Basic Return Type: enum ERROR_TYPE
#include "FuncTree.h"    // All FuncTrees inherit from this for ease of use

//----------------------------------------------------------
// Include Expansions of JSONTree
#include "JTE_SimpleData.h"
#include "JTE_ComplexData.h"

// TODO: Allow JSONTree to access the global space tree
// This way, we have access to the document cache and can use it to retrieve keys

namespace Nebulite{
    
// Forward declaration of JSON class
class JSON;
class JSONTree : public FuncTree<Nebulite::ERROR_TYPE> {
public:
    JSONTree(JSON* self);    // Using a raw Pointer itself is fine here, as the Tree is initialized inside JSON
private:
    // Self-reference to the JSON is needed within the base class to simplify the factory method
    JSON* self;  // Store reference to self

    // Factory method for creating expansion instances with proper linkage
    // Improves readability and maintainability
    template<typename ExpansionType>
    std::unique_ptr<ExpansionType> createExpansionOfType() {
        auto expansion = std::make_unique<ExpansionType>(self, this);
        // Initializing is currently done on construction of the expansion
        // However, if any additional setup is needed later on that can't be done on construction,
        // this simplifies the process
        return expansion;
    }

    //---------------------------------------
    // Commands to the JSONTree are added via Expansion files to keep the JSONTree clean
    // and allow for easy implementation and removal of collaborative features.
    // Maintainers can separately implement their own features and merge them into the JSONTree.
    //
    // 1.) Create a new Class by inheriting from Nebulite::JSONTreeExpansion::Wrapper ( .h file in ./include and .cpp file in ./src)
    // 2.) Implement the setupBindings() method to bind functions
    // 3.) Insert the new object here as a unique pointer
    // 4.) Initialize via make_unique in the JSONTree constructor
    //---------------------------------------
    std::unique_ptr<JSONTreeExpansion::SimpleData> simpleData;
    std::unique_ptr<JSONTreeExpansion::ComplexData> complexData;

};
}