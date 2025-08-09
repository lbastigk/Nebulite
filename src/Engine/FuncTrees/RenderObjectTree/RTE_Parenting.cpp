#include "RTE_Parenting.h"
#include "RenderObject.h"

Nebulite::ERROR_TYPE Nebulite::RenderObjectTreeExpansion::Parenting::addChildren(int argc, char* argv[]){
    // Idea: <link> <name>
    // Store in a map
    // E.g. add-children Pants/Jeans.jsonc myPants
    return Nebulite::ERROR_TYPE::CRITICAL_FUNCTION_NOT_IMPLEMENTED;
}
Nebulite::ERROR_TYPE Nebulite::RenderObjectTreeExpansion::Parenting::removeChildren(int argc, char* argv[]){
    // Idea: <link> <name>
    // Store in a map
    // E.g. remove-children myPants
    return Nebulite::ERROR_TYPE::CRITICAL_FUNCTION_NOT_IMPLEMENTED;
}
Nebulite::ERROR_TYPE Nebulite::RenderObjectTreeExpansion::Parenting::removeAllChildren(int argc, char* argv[]){
    return Nebulite::ERROR_TYPE::CRITICAL_FUNCTION_NOT_IMPLEMENTED;
}

