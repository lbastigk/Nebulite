#include "RenderObjectTree.h"
#include "RenderObject.h"

Nebulite::RenderObjectTree::RenderObjectTree(RenderObject* self)
    : FuncTreeWrapper<Nebulite::ERROR_TYPE>("RenderObjectTree", Nebulite::ERROR_TYPE::NONE, Nebulite::ERROR_TYPE::CRITICAL_FUNCTIONCALL_INVALID),
      self(self)
{
    //===== Layout & Geometry =====//
    bindFunction(funcTree, this, &Nebulite::RenderObjectTree::align_text, "align_text", "Align text to object dimensions");
    bindFunction(funcTree, this, &Nebulite::RenderObjectTree::make_box, "make_box", "Create text box");

    //===== Computation & Internal Updates =====//
    bindFunction(funcTree, this, &Nebulite::RenderObjectTree::deleteObject, "delete", "Marks object for deletion");
    bindFunction(funcTree, this, &Nebulite::RenderObjectTree::calculate_text, "calculate_text", "Calculate text based on object dimensions");
    bindFunction(funcTree, this, &Nebulite::RenderObjectTree::recalculate_all, "recalculate_all", "Recalculate all text and geometry");
    bindFunction(funcTree, this, &Nebulite::RenderObjectTree::reload_invokes, "reload_invokes", "Reload all invokes");

    //===== Data management =====//
    bindFunction(funcTree, this, &Nebulite::RenderObjectTree::store, "store", "Store result of function call in document");
    bindFunction(funcTree, this, &Nebulite::RenderObjectTree::move, "move", "Move part of document from a to b");
    bindFunction(funcTree, this, &Nebulite::RenderObjectTree::copy, "copy", "Copy part of document from a to b");
    bindFunction(funcTree, this, &Nebulite::RenderObjectTree::keydelete, "keydelete", "Delete a key from document");

    //===== Debugging / Logging =====//
    bindFunction(funcTree, this, &Nebulite::RenderObjectTree::echo, "echo", "Echo a message to the console");
    bindFunction(funcTree, this, &Nebulite::RenderObjectTree::log, "log", "Log the current state of the RenderObject");
    bindFunction(funcTree, this, &Nebulite::RenderObjectTree::log_value, "log_value", "Log the value of a specific property");
    bindFunction(funcTree, this, &Nebulite::RenderObjectTree::assert_nonzero, "assert_nonzero", "Assert that a specific property is non-zero"); 

    //===== Children & Invokes =====//
    bindFunction(funcTree, this, &Nebulite::RenderObjectTree::addChildren, "addChildren", "Add children to the RenderObject");
    bindFunction(funcTree, this, &Nebulite::RenderObjectTree::removeChildren, "removeChildren", "Remove specific children from the RenderObject");
    bindFunction(funcTree, this, &Nebulite::RenderObjectTree::removeAllChildren, "removeAllChildren", "Remove all children from the RenderObject");
    bindFunction(funcTree, this, &Nebulite::RenderObjectTree::addInvoke, "addInvoke", "Add an invoke to the RenderObject");
    bindFunction(funcTree, this, &Nebulite::RenderObjectTree::removeInvoke, "removeInvoke", "Remove a specific invoke from the RenderObject");
    bindFunction(funcTree, this, &Nebulite::RenderObjectTree::removeAllInvokes, "removeAllInvokes", "Remove all invokes from the RenderObject");

    //===== Complex ideas =====//
    bindFunction(funcTree, this, &Nebulite::RenderObjectTree::sql_call, "sql_call", "Call a SQL function");
    bindFunction(funcTree, this, &Nebulite::RenderObjectTree::json_call, "json_call", "Call a JSON function");

    //... Add more function bindings as needed
}

//===== Layout & Geometry =====//
Nebulite::ERROR_TYPE Nebulite::RenderObjectTree::align_text(int argc, char* argv[]){
    return Nebulite::ERROR_TYPE::CRITICAL_FUNCTION_NOT_IMPLEMENTED;
}
Nebulite::ERROR_TYPE Nebulite::RenderObjectTree::make_box(int argc, char* argv[]){
    return Nebulite::ERROR_TYPE::CRITICAL_FUNCTION_NOT_IMPLEMENTED;
}

//===== Computation & Internal Updates =====//
Nebulite::ERROR_TYPE Nebulite::RenderObjectTree::deleteObject(int argc, char* argv[]){
    //std::cerr << "RenderObjectTree::deleteObject called, marking object with id " << self->valueGet<int>(keyName.renderObject.id.c_str(),0) << " for deletion." << std::endl;

    // Mark the object for deletion
    self->deleteFlag = true;
    return Nebulite::ERROR_TYPE::NONE;
}
Nebulite::ERROR_TYPE Nebulite::RenderObjectTree::calculate_text(int argc, char* argv[]){
    return Nebulite::ERROR_TYPE::CRITICAL_FUNCTION_NOT_IMPLEMENTED;
}
Nebulite::ERROR_TYPE Nebulite::RenderObjectTree::recalculate_all(int argc, char* argv[]){
    return Nebulite::ERROR_TYPE::CRITICAL_FUNCTION_NOT_IMPLEMENTED;
}
Nebulite::ERROR_TYPE Nebulite::RenderObjectTree::reload_invokes(int argc, char* argv[]){
    return Nebulite::ERROR_TYPE::CRITICAL_FUNCTION_NOT_IMPLEMENTED;
}

//===== Data management =====//
Nebulite::ERROR_TYPE Nebulite::RenderObjectTree::store(int argc, char* argv[]){
    return Nebulite::ERROR_TYPE::CRITICAL_FUNCTION_NOT_IMPLEMENTED;
}
Nebulite::ERROR_TYPE Nebulite::RenderObjectTree::move(int argc, char* argv[]){
    return Nebulite::ERROR_TYPE::CRITICAL_FUNCTION_NOT_IMPLEMENTED;
}
Nebulite::ERROR_TYPE Nebulite::RenderObjectTree::copy(int argc, char* argv[]){
    return Nebulite::ERROR_TYPE::CRITICAL_FUNCTION_NOT_IMPLEMENTED;
}
Nebulite::ERROR_TYPE Nebulite::RenderObjectTree::keydelete(int argc, char* argv[]){
    return Nebulite::ERROR_TYPE::CRITICAL_FUNCTION_NOT_IMPLEMENTED;
}

//===== Debugging / Logging =====//
Nebulite::ERROR_TYPE Nebulite::RenderObjectTree::echo(int argc, char* argv[]){
    for (int i = 1; i < argc; i++) {
        std::cout << argv[i] << " ";
    }
    std::cout << std::endl;
    return Nebulite::ERROR_TYPE::NONE;
}
Nebulite::ERROR_TYPE Nebulite::RenderObjectTree::log(int argc, char* argv[]){
    std::string serialized = self->serialize();
    if (argc>1){
        for(int i=1; i < argc; i++){
            FileManagement::WriteFile(argv[i],serialized);
        }
    }
    else{
        std::string id = std::to_string(self->valueGet(keyName.renderObject.id.c_str(),0));
        FileManagement::WriteFile("RenderObject_id"+id+".log.json",serialized);
    }
    return Nebulite::ERROR_TYPE::NONE;
}
Nebulite::ERROR_TYPE Nebulite::RenderObjectTree::log_value(int argc, char* argv[]){
    return Nebulite::ERROR_TYPE::CRITICAL_FUNCTION_NOT_IMPLEMENTED;
}
Nebulite::ERROR_TYPE Nebulite::RenderObjectTree::assert_nonzero(int argc, char* argv[]){
    return Nebulite::ERROR_TYPE::CRITICAL_FUNCTION_NOT_IMPLEMENTED;
}

//===== Children & Invokes =====//
Nebulite::ERROR_TYPE Nebulite::RenderObjectTree::addChildren(int argc, char* argv[]){
    return Nebulite::ERROR_TYPE::CRITICAL_FUNCTION_NOT_IMPLEMENTED;
}
Nebulite::ERROR_TYPE Nebulite::RenderObjectTree::removeChildren(int argc, char* argv[]){
    return Nebulite::ERROR_TYPE::CRITICAL_FUNCTION_NOT_IMPLEMENTED;
}
Nebulite::ERROR_TYPE Nebulite::RenderObjectTree::removeAllChildren(int argc, char* argv[]){
    return Nebulite::ERROR_TYPE::CRITICAL_FUNCTION_NOT_IMPLEMENTED;
}
Nebulite::ERROR_TYPE Nebulite::RenderObjectTree::addInvoke(int argc, char* argv[]){
    return Nebulite::ERROR_TYPE::CRITICAL_FUNCTION_NOT_IMPLEMENTED;
}
Nebulite::ERROR_TYPE Nebulite::RenderObjectTree::removeInvoke(int argc, char* argv[]){
    return Nebulite::ERROR_TYPE::CRITICAL_FUNCTION_NOT_IMPLEMENTED;
}
Nebulite::ERROR_TYPE Nebulite::RenderObjectTree::removeAllInvokes(int argc, char* argv[]){
    return Nebulite::ERROR_TYPE::CRITICAL_FUNCTION_NOT_IMPLEMENTED;
}

//===== Complex ideas =====//
Nebulite::ERROR_TYPE Nebulite::RenderObjectTree::sql_call(int argc, char* argv[]){
    return Nebulite::ERROR_TYPE::CRITICAL_FUNCTION_NOT_IMPLEMENTED;
}
Nebulite::ERROR_TYPE Nebulite::RenderObjectTree::json_call(int argc, char* argv[]){
    return Nebulite::ERROR_TYPE::CRITICAL_FUNCTION_NOT_IMPLEMENTED;
}
