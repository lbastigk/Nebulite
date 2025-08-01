#include "MTE_Debug.h"
#include "GlobalSpace.h"       // Global Space for Nebulite
#include "Invoke.h"            // Invoke for parsing expressions

Nebulite::ERROR_TYPE Nebulite::MainTreeExpansion::Debug::printGlobal(int argc, char* argv[]){
    std::cout << global->getRenderer()->serializeGlobal() << std::endl;
    return Nebulite::ERROR_TYPE::NONE;
}

Nebulite::ERROR_TYPE Nebulite::MainTreeExpansion::Debug::printState(int argc, char* argv[]){
    std::cout << global->getRenderer()->serialize() << std::endl;
    return Nebulite::ERROR_TYPE::NONE;
}

Nebulite::ERROR_TYPE Nebulite::MainTreeExpansion::Debug::logGlobal(int argc, char* argv[]){
    std::string serialized = global->getRenderer()->serializeGlobal();
    if (argc>1){
        for(int i=1; i < argc; i++){
            FileManagement::WriteFile(argv[i],serialized);
        }
    }
    else{
        FileManagement::WriteFile("global.log.json",serialized);
    }
    return Nebulite::ERROR_TYPE::NONE;
}

Nebulite::ERROR_TYPE Nebulite::MainTreeExpansion::Debug::logState(int argc, char* argv[]){
    std::string serialized = global->getRenderer()->serialize();
    if (argc>1){
        for(int i=1; i < argc; i++){
            FileManagement::WriteFile(argv[i],serialized);
        }
    }
    else{
        FileManagement::WriteFile("state.log.json",serialized);
    }
    return Nebulite::ERROR_TYPE::NONE;
}

Nebulite::ERROR_TYPE Nebulite::MainTreeExpansion::Debug::render_object(int argc, char** argv){
    RenderObject ro;
    FileManagement::WriteFile("./Resources/Renderobjects/standard.json",ro.serialize());
    return Nebulite::ERROR_TYPE::NONE;
}

Nebulite::ERROR_TYPE Nebulite::MainTreeExpansion::Debug::errorlog(int argc, char* argv[]){
    if(argc == 2){
        if(!strcmp(argv[1], "on")){
            if(!global->errorLogStatus){
                try {
                    // Create ofstream only when needed (lazy initialization)
                    if (!global->errorFile) {
                        global->errorFile = std::make_unique<std::ofstream>();
                    }
                    
                    // Log errors in separate file
                    global->errorFile->open("errors.log");
                    if (!(*global->errorFile)) {
                        std::cerr << "Failed to open error file." << std::endl;
                        return Nebulite::ERROR_TYPE::CRITICAL_INVALID_FILE;
                    }
                    
                    global->originalCerrBuf = std::cerr.rdbuf(); // Store the original cerr buffer
                    std::cerr.rdbuf(global->errorFile->rdbuf()); // Redirect to file
                    global->errorLogStatus = true;
                    
                } catch (const std::exception& e) {
                    std::cerr << "Failed to create error log: " << e.what() << std::endl;
                    return Nebulite::ERROR_TYPE::CRITICAL_INVALID_FILE;
                } catch (...) {
                    std::cerr << "Failed to create error log: unknown error" << std::endl;
                    return Nebulite::ERROR_TYPE::CRITICAL_INVALID_FILE;
                }
            }
        }
        else if (!strcmp(argv[1], "off")){
            if(global->errorLogStatus){
                // Close error log
                std::cerr.flush();                           // Flush before restoring
                std::cerr.rdbuf(global->originalCerrBuf);     // Restore the original buffer
                
                if (global->errorFile) {
                    global->errorFile->close();
                    // Keep the unique_ptr for potential reuse
                }
                
                global->errorLogStatus = false;
            }
        } 
    }
    else{
        if(argc > 2){
            return Nebulite::ERROR_TYPE::TOO_MANY_ARGS;
        }
        else{
            return Nebulite::ERROR_TYPE::TOO_FEW_ARGS;
        }
    }
    return Nebulite::ERROR_TYPE::NONE;
}

Nebulite::ERROR_TYPE Nebulite::MainTreeExpansion::Debug::always(int argc, char* argv[]){
    if (argc > 1) {
        std::ostringstream oss;
        for (int i = 1; i < argc; ++i) {
            if (i > 1) oss << ' ';
            oss << argv[i];
        }

        // Split oss.str() on ';' and push each trimmed command
        std::string argStr = oss.str();
        std::stringstream ss(argStr);
        std::string command;

        while (std::getline(ss, command, ';')) {
            // Trim whitespace from each command
            command.erase(0, command.find_first_not_of(" \t"));
            command.erase(command.find_last_not_of(" \t") + 1);
            if (!command.empty()) {
                global->tasks_always.taskList.push_back(command);
            }
        }
    }
    return Nebulite::ERROR_TYPE::NONE;
}

Nebulite::ERROR_TYPE Nebulite::MainTreeExpansion::Debug::alwaysClear(int argc, char* argv[]){
    global->tasks_always.taskList.clear();
    return Nebulite::ERROR_TYPE::NONE;
}




Nebulite::ERROR_TYPE Nebulite::MainTreeExpansion::Debug::printVar(int argc, char** argv){
    std::cout << "headless: " << global->headless << std::endl;
    return Nebulite::ERROR_TYPE::NONE;
}