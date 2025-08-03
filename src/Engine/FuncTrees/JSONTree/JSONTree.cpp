#include "JSONTree.h"

Nebulite::JSONTree::JSONTree(JSON* self)
    : FuncTreeWrapper<Nebulite::ERROR_TYPE>("JSONTree", Nebulite::ERROR_TYPE::NONE, Nebulite::ERROR_TYPE::CRITICAL_FUNCTIONCALL_INVALID), self(self) {
    // Initialize JSON specific function calls here
    // Example: attachFunction(&Nebulite::JSON::someFunction, "someFunction", "Description of someFunction");
    simpleData = createExpansionOfType<Nebulite::JSONTreeExpansion::SimpleData>();
    complexData = createExpansionOfType<Nebulite::JSONTreeExpansion::ComplexData>();
}