//------------------------------------------
// Includes

// Nebulite
#include "Core/RenderObject.hpp"
#include "Module/Domain/RenderObject/Logging.hpp"
#include "Nebulite.hpp"
#include "Utility/IO/FileManagement.hpp"

//------------------------------------------
namespace Nebulite::Module::Domain::RenderObject {

//------------------------------------------
// Update
Constants::Event Logging::updateHook() {
    // Add Domain-specific updates here!
    // General rule:
    // This is used to update all variables/states that are INTERNAL ONLY
    return Constants::Event::Success;
}

//------------------------------------------
// Domain-Bound Functions

Constants::Event Logging::log_all(std::span<std::string const> const& args, Interaction::Context const& ctx, Interaction::ContextScope const& ctxScope) const {
    std::string const serialized = ctxScope.self.serialize();
    if (args.size() > 1) {
        for (auto const& arg : args.subspan(1)) {
            if (!Utility::IO::FileManagement::WriteFile(arg, serialized)) {
                return Constants::StandardCapture::Error::File::couldNotWriteFile(domain.capture);
            }
        }
    } else {
        if (!Utility::IO::FileManagement::WriteFile("RenderObject_id" + std::to_string(ctx.self.getId()) + ".log.jsonc", serialized)) {
            return Constants::StandardCapture::Error::File::couldNotWriteFile(domain.capture);
        }
    }
    return Constants::Event::Success;
}

Constants::Event Logging::log_key(std::span<std::string const> const& args, Interaction::Context const& ctx, Interaction::ContextScope const& ctxScope) const {
    if (args.size() < 2) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(ctx.self.capture);
    }
    auto const key = ctxScope.self.getRootScope() + args[1];
    std::string file = "RenderObject_id" + std::to_string(ctx.self.getId()) + ".log.jsonc";
    if (args.size() > 2) {
        file = args[2];
    }
    auto const value = ctxScope.self.get<std::string>(key.view()).value_or("Key not found");
    if (!Utility::IO::FileManagement::WriteFile(file, value)) {
        return Constants::StandardCapture::Error::File::couldNotWriteFile(domain.capture);
    }
    return Constants::Event::Success;
}

} // namespace Nebulite::Module::Domain::RenderObject
