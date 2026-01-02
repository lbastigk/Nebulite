#include "DomainModule/RenderObject/Logging.hpp"
#include "Core/RenderObject.hpp"
#include "Utility/FileManagement.hpp"

#include "Nebulite.hpp"

namespace Nebulite::DomainModule::RenderObject {

//------------------------------------------
// Update
Constants::Error Logging::update() {
    // Add Domain-specific updates here!
    // General rule:
    // This is used to update all variables/states that are INTERNAL ONLY
    return Constants::ErrorTable::NONE();
}

//------------------------------------------
// Domain-Bound Functions

Constants::Error Logging::echo(int argc, char** argv) {
    std::string const args = Utility::StringHandler::recombineArgs(argc - 1, argv + 1);
    Nebulite::cout() << args << Nebulite::endl;
    return Constants::ErrorTable::NONE();
}

Constants::Error Logging::log_all(int argc, char** argv) {
    std::string const serialized = domain.serialize();
    if (argc > 1) {
        for (int i = 1; i < argc; i++) {
            Utility::FileManagement::WriteFile(argv[i], serialized);
        }
    } else {
        std::string const id = std::to_string(domain.getDoc().get(Constants::KeyNames::RenderObject::id, 0));
        Utility::FileManagement::WriteFile("RenderObject_id" + id + ".log.jsonc", serialized);
    }
    return Constants::ErrorTable::NONE();
}

Constants::Error Logging::log_key(int argc, char** argv) {
    if (argc < 2) {
        return Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }
    std::string const key = argv[1];
    std::string file = "RenderObject_id" + std::to_string(domain.getDoc().get(Constants::KeyNames::RenderObject::id, 0)) + ".log.jsonc";
    if (argc > 2) {
        file = argv[2];
    }
    auto const value = domain.getDoc().get<std::string>(key, "Key not found");
    Utility::FileManagement::WriteFile(file, value);
    return Constants::ErrorTable::NONE();
}

} // namespace Nebulite::DomainModule::RenderObject
