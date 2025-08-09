#include "RTE_Logging.h"
#include "RenderObject.h"

Nebulite::ERROR_TYPE Nebulite::RenderObjectTreeExpansion::Logging::echo(int argc, char* argv[]){
    for (int i = 1; i < argc; i++) {
        std::cout << argv[i] << " ";
    }
    std::cout << std::endl;
    return Nebulite::ERROR_TYPE::NONE;
}
Nebulite::ERROR_TYPE Nebulite::RenderObjectTreeExpansion::Logging::log(int argc, char* argv[]){
    std::string serialized = self->serialize();
    if (argc>1){
        for(int i=1; i < argc; i++){
            FileManagement::WriteFile(argv[i],serialized);
        }
    }
    else{
        std::string id = std::to_string(self->valueGet(keyName.renderObject.id.c_str(),0));
        FileManagement::WriteFile("RenderObject_id"+id+".log.jsonc",serialized);
    }
    return Nebulite::ERROR_TYPE::NONE;
}

Nebulite::ERROR_TYPE Nebulite::RenderObjectTreeExpansion::Logging::logValue(int argc, char* argv[]){
    return Nebulite::ERROR_TYPE::CRITICAL_FUNCTION_NOT_IMPLEMENTED;
}