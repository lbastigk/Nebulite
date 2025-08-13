#include "RTE_Layout.h"
#include "RenderObject.h"

//-------------------------------
// Update
void Nebulite::RenderObjectTreeExpansion::Layout::update() {
    // Add FuncTree-specific updates here!
    // General rule:
    // This is used to update all variables/states that are INTERNAL ONLY
}

//-------------------------------
// FuncTree-Bound Functions

Nebulite::ERROR_TYPE Nebulite::RenderObjectTreeExpansion::Layout::alignText(int argc, char* argv[]){
    return Nebulite::ERROR_TYPE::CRITICAL_FUNCTION_NOT_IMPLEMENTED;
}
Nebulite::ERROR_TYPE Nebulite::RenderObjectTreeExpansion::Layout::makeBox(int argc, char* argv[]){
    return Nebulite::ERROR_TYPE::CRITICAL_FUNCTION_NOT_IMPLEMENTED;
}