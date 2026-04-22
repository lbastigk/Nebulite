#include "Nebulite.hpp"
#include "Module/Domain/Common/Debug.hpp"
#include "Utility/IO/FileManagement.hpp"
#include "Utility/StringHandler.hpp"

namespace Nebulite::DomainModule::Common {

//------------------------------------------
// Update
Constants::Event Debug::updateHook() {
    // Add Domain-specific updates here!
    // General rule:
    // This is used to update all variables/states that are INTERNAL ONLY
    return Constants::Event::Success;
}

//------------------------------------------
// Domain-Bound Functions

// NOLINTNEXTLINE
Constants::Event Debug::print(std::span<std::string const> const& args, Interaction::Context& ctx, Interaction::ContextScope& ctxScope) {
    if (args.size() > 2) {
        return Constants::StandardCapture::Warning::Functional::tooManyArgs(ctx.self.capture);
    }
    if (args.size() == 2) {
        auto const scopedKey = Data::ScopedKey(args[1]);
        auto const memberType = ctxScope.self.memberType(scopedKey);
        if (memberType == Data::KeyType::null) {
            ctx.self.capture.log.println("{}");
            return Constants::Event::Success;
        }
        if (memberType == Data::KeyType::object || memberType == Data::KeyType::array) {
            ctx.self.capture.log.println(ctxScope.self.serialize(scopedKey));
            return Constants::Event::Success;
        }
        if (memberType == Data::KeyType::value) {
            ctx.self.capture.log.println(ctxScope.self.get<std::string>(scopedKey).value_or(""));
            return Constants::Event::Success;
        }
    }
    ctx.self.capture.log.println(ctxScope.self.serialize());
    return Constants::Event::Success;
}

// NOLINTNEXTLINE
Constants::Event Debug::printId(std::span<std::string const> const& /*args*/, Interaction::Context& ctx, Interaction::ContextScope& /*ctxScope*/) {
    ctx.self.capture.log.println(ctx.self.getId());
    return Constants::Event::Success;
}

// NOLINTNEXTLINE
Constants::Event Debug::error(std::span<std::string const> const& args, Interaction::Context& ctx, Interaction::ContextScope& /*ctxScope*/) {
    auto const& argStr = Utility::StringHandler::recombineArgs(args.subspan(1));
    ctx.self.capture.error.println(argStr);
    return Constants::Event::Success;
}

// NOLINTNEXTLINE
Constants::Event Debug::warn(std::span<std::string const> const& args) const {
    if (args.size() < 2) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(domain.capture);
    }
    std::string const argStr = Utility::StringHandler::recombineArgs(args.subspan(1));
    domain.capture.warning.println(argStr);
    return Constants::Event::Warning;
}

Constants::Event Debug::critical(std::span<std::string const> const& args) const {
    if (args.size() < 2) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(domain.capture);
    }

    std::string const argStr = Utility::StringHandler::recombineArgs(args.subspan(1));
    domain.capture.error.println(argStr);
    return Constants::Event::Error;
}

Constants::Event Debug::cat(std::span<std::string const> const& args) const{
    if (args.size() < 2) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(domain.capture);
    }

    auto const filePath = Utility::StringHandler::recombineArgs(args.subspan(1));
    auto const fileContent = Utility::IO::FileManagement::LoadFile(filePath);
    domain.capture.log.println(fileContent);
    return Constants::Event::Success;
}

} // namespace Nebulite::DomainModule::Common
