#include "JSONTree.h"

Nebulite::JSONTree::JSONTree()
    : FuncTreeWrapper<Nebulite::ERROR_TYPE>("JSONTree", Nebulite::ERROR_TYPE::NONE, Nebulite::ERROR_TYPE::CRITICAL_FUNCTIONCALL_INVALID) {
    // Initialize JSON specific function calls here
    // Example: attachFunction(&Nebulite::JSON::someFunction, "someFunction", "Description of someFunction");
}