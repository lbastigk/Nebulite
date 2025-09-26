#include "DomainModule/GlobalSpace/GDM_Debug.hpp"
#include "Core/GlobalSpace.hpp"       // Global Space for Nebulite

//------------------------------------------
// Update
void Nebulite::DomainModule::GlobalSpace::Debug::update() {
    // Add Domain-specific updates here!
    // General rule:
    // This is used to update all variables/states that are INTERNAL ONLY
}

//------------------------------------------
// Domain-Bound Functions

Nebulite::Constants::Error Nebulite::DomainModule::GlobalSpace::Debug::printGlobal(int argc, char* argv[]){
    std::cout << domain->getDoc()->serialize() << std::endl;
    return Nebulite::Constants::ErrorTable::NONE();
}

Nebulite::Constants::Error Nebulite::DomainModule::GlobalSpace::Debug::printState(int argc, char* argv[]){
    std::cout << domain->getRenderer()->serialize() << std::endl;
    return Nebulite::Constants::ErrorTable::NONE();
}

Nebulite::Constants::Error Nebulite::DomainModule::GlobalSpace::Debug::logGlobal(int argc, char* argv[]){
    std::string serialized = domain->getDoc()->serialize();
    if (argc>1){
        for(int i=1; i < argc; i++){
            Nebulite::Utility::FileManagement::WriteFile(argv[i],serialized);
        }
    }
    else{
        Nebulite::Utility::FileManagement::WriteFile("global.log.jsonc",serialized);
    }
    return Nebulite::Constants::ErrorTable::NONE();
}

Nebulite::Constants::Error Nebulite::DomainModule::GlobalSpace::Debug::logState(int argc, char* argv[]){
    std::string serialized = domain->getRenderer()->serialize();
    if (argc>1){
        for(int i=1; i < argc; i++){
            Nebulite::Utility::FileManagement::WriteFile(argv[i],serialized);
        }
    }
    else{
        Nebulite::Utility::FileManagement::WriteFile("state.log.jsonc",serialized);
    }
    return Nebulite::Constants::ErrorTable::NONE();
}

Nebulite::Constants::Error Nebulite::DomainModule::GlobalSpace::Debug::render_object(int argc, char** argv){
    Nebulite::Core::RenderObject ro(domain);
    Nebulite::Utility::FileManagement::WriteFile("./Resources/Renderobjects/standard.jsonc",ro.serialize());
    return Nebulite::Constants::ErrorTable::NONE();
}

Nebulite::Constants::Error Nebulite::DomainModule::GlobalSpace::Debug::errorlog(int argc, char* argv[]){
    // Initialize the error logging buffer
    if(!originalCerrBuf) {
        // Handle the case where originalCerrBuf is not set
        originalCerrBuf = std::cerr.rdbuf();
    }

    if(argc == 2){
        if(!strcmp(argv[1], "on")){
            if(!errorLogStatus){
                try {
                    // Create ofstream only when needed (lazy initialization)
                    if (!errorFile) {
                        errorFile = std::make_unique<std::ofstream>();
                    }
                    
                    // Log errors in separate file
                    errorFile->open("errors.log");
                    if (!(*errorFile)) {
                        std::cerr << "Failed to open error file." << std::endl;
                        return Nebulite::Constants::ErrorTable::FILE::CRITICAL_INVALID_FILE();
                    }
                    
                    originalCerrBuf = std::cerr.rdbuf(); // Store the original cerr buffer
                    std::cerr.rdbuf(errorFile->rdbuf()); // Redirect to file
                    errorLogStatus = true;
                    
                } catch (const std::exception& e) {
                    std::cerr << "Failed to create error log: " << e.what() << std::endl;
                    return Nebulite::Constants::ErrorTable::FILE::CRITICAL_INVALID_FILE();
                } catch (...) {
                    std::cerr << "Failed to create error log: unknown error" << std::endl;
                    return Nebulite::Constants::ErrorTable::FILE::CRITICAL_INVALID_FILE();
                }
            }
        }
        else if (!strcmp(argv[1], "off")){
            if(errorLogStatus){
                // Close error log
                std::cerr.flush();                    // Flush before restoring
                std::cerr.rdbuf(originalCerrBuf);     // Restore the original buffer
                
                if (errorFile) {
                    errorFile->close();
                    // Keep the unique_ptr for potential reuse
                }
                
                errorLogStatus = false;
            }
        } 
    }
    else{
        if(argc > 2){
            return Nebulite::Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS();
        }
        else{
            return Nebulite::Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
        }
    }
    return Nebulite::Constants::ErrorTable::NONE();
}

Nebulite::Constants::Error Nebulite::DomainModule::GlobalSpace::Debug::clearConsole(int argc, char* argv[]){
    if (argc > 1) {
        return Nebulite::Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS();
    }
    int error = 0;
    #if _WIN32
        error = std::system("cls");
    #else
        error = std::system("clear");
    #endif

    if (error != 0) {
        return Nebulite::Constants::ErrorTable::CRITICAL_GENERAL();
    }
    return Nebulite::Constants::ErrorTable::NONE();
}

Nebulite::Constants::Error Nebulite::DomainModule::GlobalSpace::Debug::crash(int argc, char** argv) {
    // If an argument is provided, use it to select crash type
    if (argc > 1 && argv[1]) {
        std::string crashType = argv[1];
        if (crashType == "segfault") {
            // Cause a segmentation fault
            int* p = nullptr;
            *p = 42;
        } else if (crashType == "abort") {
            // Abort the program
            std::abort();
        } else if (crashType == "terminate") {
            // Terminate with std::terminate
            std::terminate();
        } else if (crashType == "throw") {
            // Throw an uncaught exception
            throw std::runtime_error("Intentional crash: uncaught exception");
        } else {
            std::cerr << "Unknown crash type requested: " << crashType << std::endl;
            std::cerr << "Defaulting to segmentation fault" << std::endl;
        }
    } else {
        // Default: segmentation fault
        int* p = nullptr;
        *p = 42;
    }
    // Should never reach here
    return Nebulite::Constants::ErrorTable::NONE();
}

Nebulite::Constants::Error Nebulite::DomainModule::GlobalSpace::Debug::error(int argc, char* argv[]) {
    for (int i = 1; i < argc; ++i) {
        std::cerr << argv[i];
        if (i < argc - 1) {
            std::cerr << " ";
        }
    }
    std::cerr << std::endl;

    // No further error to return
    return Nebulite::Constants::ErrorTable::NONE();
}

Nebulite::Constants::Error Nebulite::DomainModule::GlobalSpace::Debug::warn(int argc, char* argv[]){
    if (argc < 2) {
        return Nebulite::Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }

    std::string str = "";
    for(int i = 1; i < argc; ++i){
        str += argv[i];
        if(i < argc - 1){
            str += " ";
        }
    }
    return Nebulite::Constants::ErrorTable::addError(str, Nebulite::Constants::Error::NON_CRITICAL);
}

Nebulite::Constants::Error Nebulite::DomainModule::GlobalSpace::Debug::critical(int argc, char* argv[]){
    if (argc < 2) {
        return Nebulite::Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }

    std::string str = "";
    for(int i = 1; i < argc; ++i){
        str += argv[i];
        if(i < argc - 1){
            str += " ";
        }
    }
    return Nebulite::Constants::ErrorTable::addError(str, Nebulite::Constants::Error::CRITICAL);
}