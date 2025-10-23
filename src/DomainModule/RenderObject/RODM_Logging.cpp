#include "DomainModule/RenderObject/RODM_Logging.hpp"
#include "Core/RenderObject.hpp"

namespace Nebulite::DomainModule::RenderObject{

const std::string Logging::log_name = "log";
const std::string Logging::log_desc = R"(Logging utilities)";

//------------------------------------------
// Update
Nebulite::Constants::Error Logging::update() {
    // Add Domain-specific updates here!
    // General rule:
    // This is used to update all variables/states that are INTERNAL ONLY
    return Nebulite::Constants::ErrorTable::NONE();
}

//------------------------------------------
// Domain-Bound Functions

Nebulite::Constants::Error Logging::echo(int argc,  char* argv[]){
    std::string args = Nebulite::Utility::StringHandler::recombineArgs(argc - 1, argv + 1);
    Nebulite::Utility::Capture::cout() << args << Nebulite::Utility::Capture::endl;
    return Nebulite::Constants::ErrorTable::NONE();
}
const std::string Logging::echo_name = "echo";
const std::string Logging::echo_desc = R"(Echoes all arguments as string to the standard output.

Usage: echo <string>

This command concatenates all arguments with a whitespace and outputs them to the standard output (cout).
Example:
./bin/Nebulite echo Hello World!
Outputs:
Hello World!
)";

Nebulite::Constants::Error Logging::log_all(int argc,  char* argv[]){
    std::string serialized = domain->serialize();
    if (argc>1){
        for(int i=1; i < argc; i++){
            Nebulite::Utility::FileManagement::WriteFile(argv[i], serialized);
        }
    }
    else{
        std::string id = std::to_string(domain->get(Nebulite::Constants::keyName.renderObject.id.c_str(),0));
        Nebulite::Utility::FileManagement::WriteFile("RenderObject_id"+id+".log.jsonc", serialized);
    }
    return Nebulite::Constants::ErrorTable::NONE();
}
const std::string Logging::log_all_name = "log all";
const std::string Logging::log_all_desc = R"(Logs the entire RenderObject to a file.

Usage: log [filename]

Logs to `RenderObject_id<id>.log.jsonc` if no filename is provided.
)";

Nebulite::Constants::Error Logging::log_key(int argc,  char* argv[]){
    if (argc < 2) {
        return Nebulite::Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }
    std::string key = argv[1];
    std::string file = "RenderObject_id" + std::to_string(domain->get(Nebulite::Constants::keyName.renderObject.id.c_str(), 0)) + ".log.jsonc";
    if(argc > 2){
        file = argv[2];
    }
    std::string value = domain->get<std::string>(key.c_str(), "Key not found");
    Nebulite::Utility::FileManagement::WriteFile(file, value);
    return Nebulite::Constants::ErrorTable::NONE();
}
const std::string Logging::log_key_name = "log key";
const std::string Logging::log_key_desc = R"(Logs a specific value from the RenderObject to a file.

Usage: log key <key> [filename]

Logs the value associated with <key> to the specified [filename], 
or to `RenderObject_id<id>.log.jsonc` if no filename is provided.
)";

} // namespace Nebulite::DomainModule::RenderObject