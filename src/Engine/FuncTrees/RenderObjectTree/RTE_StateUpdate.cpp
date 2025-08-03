#include "RTE_StateUpdate.h"
#include "RenderObject.h"

Nebulite::ERROR_TYPE Nebulite::RenderObjectTreeExpansion::StateUpdate::deleteObject(int argc, char* argv[]){
    self->flag.deleteFromScene = true;
    return Nebulite::ERROR_TYPE::NONE;
}
Nebulite::ERROR_TYPE Nebulite::RenderObjectTreeExpansion::StateUpdate::updateText(int argc, char* argv[]){
    self->flag.calculateText = true;
    return Nebulite::ERROR_TYPE::NONE;
}

Nebulite::ERROR_TYPE Nebulite::RenderObjectTreeExpansion::StateUpdate::reloadInvokes(int argc, char* argv[]){
    return Nebulite::ERROR_TYPE::CRITICAL_FUNCTION_NOT_IMPLEMENTED;
}

Nebulite::ERROR_TYPE Nebulite::RenderObjectTreeExpansion::StateUpdate::addInvoke(int argc, char* argv[]){
    return Nebulite::ERROR_TYPE::CRITICAL_FUNCTION_NOT_IMPLEMENTED;
}
Nebulite::ERROR_TYPE Nebulite::RenderObjectTreeExpansion::StateUpdate::removeInvoke(int argc, char* argv[]){
    return Nebulite::ERROR_TYPE::CRITICAL_FUNCTION_NOT_IMPLEMENTED;
}
Nebulite::ERROR_TYPE Nebulite::RenderObjectTreeExpansion::StateUpdate::removeAllInvokes(int argc, char* argv[]){
    return Nebulite::ERROR_TYPE::CRITICAL_FUNCTION_NOT_IMPLEMENTED;
}