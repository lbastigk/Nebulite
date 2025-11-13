#include "DomainModule/RenderObject/RODM_Logging.hpp"
#include "Core/RenderObject.hpp"
#include "Utility/FileManagement.hpp"

#include "Nebulite.hpp"

namespace Nebulite::DomainModule::RenderObject{

std::string const Logging::log_name = "log";
std::string const Logging::log_desc = R"(Logging utilities)";

//------------------------------------------
// Update
Constants::Error Logging::update(){
    // Add Domain-specific updates here!
    // General rule:
    // This is used to update all variables/states that are INTERNAL ONLY
    return Constants::ErrorTable::NONE();
}

//------------------------------------------
// Domain-Bound Functions

// NOLINTNEXTLINE
Constants::Error Logging::echo(int argc,  char** argv){
    std::string const args = Utility::StringHandler::recombineArgs(argc - 1, argv + 1);
    Nebulite::cout() << args << Nebulite::endl;
    return Constants::ErrorTable::NONE();
}
std::string const Logging::echo_name = "echo";
std::string const Logging::echo_desc = R"(Echoes all arguments as string to the standard output.

Usage: echo <string>

This command concatenates all arguments with a whitespace and outputs them to the standard output (cout).
Example:
./bin/Nebulite echo Hello World!
Outputs:
Hello World!
)";

// NOLINTNEXTLINE
Constants::Error Logging::log_all(int argc,  char** argv){
    std::string const serialized = domain->serialize();
    if (argc>1){
        for(int i=1; i < argc; i++){
            Utility::FileManagement::WriteFile(argv[i], serialized);
        }
    }
    else{
        std::string const id = std::to_string(domain->getDoc()->get(Constants::keyName.renderObject.id.c_str(),0));
        Utility::FileManagement::WriteFile("RenderObject_id"+id+".log.jsonc", serialized);
    }
    return Constants::ErrorTable::NONE();
}
std::string const Logging::log_all_name = "log all";
std::string const Logging::log_all_desc = R"(Logs the entire RenderObject to a file.

Usage: log [filename]

Logs to `RenderObject_id<id>.log.jsonc` if no filename is provided.
)";

// NOLINTNEXTLINE
Constants::Error Logging::log_key(int argc,  char** argv){
    if (argc < 2){
        return Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }
    std::string const key = argv[1];
    std::string file = "RenderObject_id" + std::to_string(domain->getDoc()->get(Constants::keyName.renderObject.id.c_str(), 0)) + ".log.jsonc";
    if(argc > 2){
        file = argv[2];
    }
    auto const value = domain->getDoc()->get<std::string>(key.c_str(), "Key not found");
    Utility::FileManagement::WriteFile(file, value);
    return Constants::ErrorTable::NONE();
}
std::string const Logging::log_key_name = "log key";
std::string const Logging::log_key_desc = R"(Logs a specific value from the RenderObject to a file.

Usage: log key <key> [filename]

Logs the value associated with <key> to the specified [filename], 
or to `RenderObject_id<id>.log.jsonc` if no filename is provided.
)";

} // namespace Nebulite::DomainModule::RenderObject