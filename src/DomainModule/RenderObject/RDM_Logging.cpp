#include "DomainModule/RenderObject/RDM_Logging.hpp"
#include "Core/RenderObject.hpp"

//------------------------------------------
// Update
void Nebulite::DomainModule::RenderObject::Logging::update() {
    // Add FuncTree-specific updates here!
    // General rule:
    // This is used to update all variables/states that are INTERNAL ONLY
}

//------------------------------------------
// FuncTree-Bound Functions

Nebulite::Constants::ERROR_TYPE Nebulite::DomainModule::RenderObject::Logging::echo(int argc, char* argv[]){
    for (int i = 1; i < argc; i++) {
        std::cout << argv[i] << " ";
    }
    std::cout << std::endl;
    return Nebulite::Constants::ERROR_TYPE::NONE;
}

Nebulite::Constants::ERROR_TYPE Nebulite::DomainModule::RenderObject::Logging::log(int argc, char* argv[]){
    std::string serialized = domain->serialize();
    if (argc>1){
        for(int i=1; i < argc; i++){
            Nebulite::Utility::FileManagement::WriteFile(argv[i],serialized);
        }
    }
    else{
        std::string id = std::to_string(domain->get(Nebulite::Constants::keyName.renderObject.id.c_str(),0));
        Nebulite::Utility::FileManagement::WriteFile("RenderObject_id"+id+".log.jsonc",serialized);
    }
    return Nebulite::Constants::ERROR_TYPE::NONE;
}

Nebulite::Constants::ERROR_TYPE Nebulite::DomainModule::RenderObject::Logging::logValue(int argc, char* argv[]){
    return Nebulite::Constants::ERROR_TYPE::CRITICAL_FUNCTION_NOT_IMPLEMENTED;
}

Nebulite::Constants::ERROR_TYPE Nebulite::DomainModule::RenderObject::Logging::print(int argc, char* argv[]){
    std::string serialized = domain->serialize();
    std::cout << serialized << std::endl;
    return Nebulite::Constants::ERROR_TYPE::NONE;
}

Nebulite::Constants::ERROR_TYPE Nebulite::DomainModule::RenderObject::Logging::printValue(int argc, char* argv[]){
    if(argc<2){
        std::cerr << "print-value requires a <key> argument" << std::endl;
        return Nebulite::Constants::ERROR_TYPE::TOO_FEW_ARGS;
    }
    std::string key = argv[1];
    auto value = domain->get<std::string>(key.c_str(), "");
    std::cout << value << std::endl;
    return Nebulite::Constants::ERROR_TYPE::NONE;
}