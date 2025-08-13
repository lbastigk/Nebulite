#include "GTE_Debug.h"
#include "GlobalSpace.h"       // Global Space for Nebulite

//-------------------------------
// Update
void Nebulite::GlobalSpaceTreeExpansion::Debug::update() {
    // Add FuncTree-specific updates here!
    // General rule:
    // This is used to update all variables/states that are INTERNAL ONLY
}

//-------------------------------
// FuncTree-Bound Functions

Nebulite::ERROR_TYPE Nebulite::GlobalSpaceTreeExpansion::Debug::printGlobal(int argc, char* argv[]){
    std::cout << self->getRenderer()->serializeGlobal() << std::endl;
    return Nebulite::ERROR_TYPE::NONE;
}

Nebulite::ERROR_TYPE Nebulite::GlobalSpaceTreeExpansion::Debug::printState(int argc, char* argv[]){
    std::cout << self->getRenderer()->serialize() << std::endl;
    return Nebulite::ERROR_TYPE::NONE;
}

Nebulite::ERROR_TYPE Nebulite::GlobalSpaceTreeExpansion::Debug::logGlobal(int argc, char* argv[]){
    std::string serialized = self->getRenderer()->serializeGlobal();
    if (argc>1){
        for(int i=1; i < argc; i++){
            FileManagement::WriteFile(argv[i],serialized);
        }
    }
    else{
        FileManagement::WriteFile("global.log.jsonc",serialized);
    }
    return Nebulite::ERROR_TYPE::NONE;
}

Nebulite::ERROR_TYPE Nebulite::GlobalSpaceTreeExpansion::Debug::logState(int argc, char* argv[]){
    std::string serialized = self->getRenderer()->serialize();
    if (argc>1){
        for(int i=1; i < argc; i++){
            FileManagement::WriteFile(argv[i],serialized);
        }
    }
    else{
        FileManagement::WriteFile("state.log.jsonc",serialized);
    }
    return Nebulite::ERROR_TYPE::NONE;
}

Nebulite::ERROR_TYPE Nebulite::GlobalSpaceTreeExpansion::Debug::render_object(int argc, char** argv){
    RenderObject ro;
    FileManagement::WriteFile("./Resources/Renderobjects/standard.jsonc",ro.serialize());
    return Nebulite::ERROR_TYPE::NONE;
}

Nebulite::ERROR_TYPE Nebulite::GlobalSpaceTreeExpansion::Debug::errorlog(int argc, char* argv[]){
    if(argc == 2){
        if(!strcmp(argv[1], "on")){
            if(!self->errorLogStatus){
                try {
                    // Create ofstream only when needed (lazy initialization)
                    if (!self->errorFile) {
                        self->errorFile = std::make_unique<std::ofstream>();
                    }
                    
                    // Log errors in separate file
                    self->errorFile->open("errors.log");
                    if (!(*self->errorFile)) {
                        std::cerr << "Failed to open error file." << std::endl;
                        return Nebulite::ERROR_TYPE::CRITICAL_INVALID_FILE;
                    }
                    
                    self->originalCerrBuf = std::cerr.rdbuf(); // Store the original cerr buffer
                    std::cerr.rdbuf(self->errorFile->rdbuf()); // Redirect to file
                    self->errorLogStatus = true;
                    
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
            if(self->errorLogStatus){
                // Close error log
                std::cerr.flush();                           // Flush before restoring
                std::cerr.rdbuf(self->originalCerrBuf);     // Restore the original buffer
                
                if (self->errorFile) {
                    self->errorFile->close();
                    // Keep the unique_ptr for potential reuse
                }
                
                self->errorLogStatus = false;
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

Nebulite::ERROR_TYPE Nebulite::GlobalSpaceTreeExpansion::Debug::always(int argc, char* argv[]){
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
                self->tasks_always.taskList.push_back(command);
            }
        }
    }
    return Nebulite::ERROR_TYPE::NONE;
}

Nebulite::ERROR_TYPE Nebulite::GlobalSpaceTreeExpansion::Debug::alwaysClear(int argc, char* argv[]){
    self->tasks_always.taskList.clear();
    return Nebulite::ERROR_TYPE::NONE;
}