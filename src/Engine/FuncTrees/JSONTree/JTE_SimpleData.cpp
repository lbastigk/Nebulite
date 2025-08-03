#include "JTE_SimpleData.h"
#include "JSON.h"

Nebulite::ERROR_TYPE Nebulite::JSONTreeExpansion::SimpleData::set(int argc, char* argv[]) {
    if(argc < 3) {
        std::cerr << "Error: Too few arguments for set command." << std::endl;
        return Nebulite::ERROR_TYPE::TOO_FEW_ARGS;
    }
    
    std::string key = argv[1];
    std::string value = argv[2];
    for (int i = 3; i < argc; ++i) {
        value += " " + std::string(argv[i]);
    }
    self->set(key.c_str(), value);
    return Nebulite::ERROR_TYPE::NONE;
}

Nebulite::ERROR_TYPE Nebulite::JSONTreeExpansion::SimpleData::store(int argc, char* argv[]){
    return Nebulite::ERROR_TYPE::CRITICAL_FUNCTION_NOT_IMPLEMENTED;
}
Nebulite::ERROR_TYPE Nebulite::JSONTreeExpansion::SimpleData::move(int argc, char* argv[]){
    return Nebulite::ERROR_TYPE::CRITICAL_FUNCTION_NOT_IMPLEMENTED;
}
Nebulite::ERROR_TYPE Nebulite::JSONTreeExpansion::SimpleData::copy(int argc, char* argv[]){
    return Nebulite::ERROR_TYPE::CRITICAL_FUNCTION_NOT_IMPLEMENTED;
}
Nebulite::ERROR_TYPE Nebulite::JSONTreeExpansion::SimpleData::keyDelete(int argc, char* argv[]){
    return Nebulite::ERROR_TYPE::CRITICAL_FUNCTION_NOT_IMPLEMENTED;
}



