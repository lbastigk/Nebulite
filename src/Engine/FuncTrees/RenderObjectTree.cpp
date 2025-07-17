#include <RenderObjectTree.h>

//===== Layout & Geometry =====//
Nebulite::ERROR_TYPE Nebulite::RenderObjectTree::align_text(int argc, char* argv[]){
    return Nebulite::ERROR_TYPE::CRITICAL_FUNCTION_NOT_IMPLEMENTED;
}


//===== Computation & Internal Updates =====//
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

//===== Complex ideas =====//
Nebulite::ERROR_TYPE sql_call(int argc, char* argv[]){
    return Nebulite::ERROR_TYPE::CRITICAL_FUNCTION_NOT_IMPLEMENTED;
}
Nebulite::ERROR_TYPE json_call(int argc, char* argv[]){
    return Nebulite::ERROR_TYPE::CRITICAL_FUNCTION_NOT_IMPLEMENTED;
}

//===== Generation =====//
Nebulite::ERROR_TYPE make_box(int argc, char* argv[]){
    return Nebulite::ERROR_TYPE::CRITICAL_FUNCTION_NOT_IMPLEMENTED;
}