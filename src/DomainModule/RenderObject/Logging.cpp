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

// NOLINTNEXTLINE
Constants::Error Logging::echo(std::span<std::string const> const& args) {
    std::string const argStr = Utility::StringHandler::recombineArgs(args.subspan(1));
    Nebulite::cout() << argStr << Nebulite::endl;
    return Constants::ErrorTable::NONE();
}

// NOLINTNEXTLINE
Constants::Error Logging::log_all(std::span<std::string const> const& args, Interaction::Execution::DomainBase& caller, Data::JsonScopeBase& callerScope) {
    std::string const serialized = callerScope.serialize();
    if (args.size() > 1) {
        for (auto const& arg : args.subspan(1)) {
            Utility::FileManagement::WriteFile(arg, serialized);
        }
    } else {
        std::string const id = std::to_string(callerScope.get(Constants::KeyNames::RenderObject::id, 0));
        Utility::FileManagement::WriteFile("RenderObject_id" + id + ".log.jsonc", serialized);
    }
    return Constants::ErrorTable::NONE();
}

// NOLINTNEXTLINE
Constants::Error Logging::log_key(std::span<std::string const> const& args, Interaction::Execution::DomainBase& caller, Data::JsonScopeBase& callerScope) {
    if (args.size() < 2) {
        return Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }
    auto const key = callerScope.getRootScope() + args[1];
    std::string file = "RenderObject_id" + std::to_string(callerScope.get(Constants::KeyNames::RenderObject::id, 0)) + ".log.jsonc";
    if (args.size() > 2) {
        file = args[2];
    }
    auto const value = callerScope.get<std::string>(key.view(), "Key not found");
    Utility::FileManagement::WriteFile(file, value);
    return Constants::ErrorTable::NONE();
}

} // namespace Nebulite::DomainModule::RenderObject
