//------------------------------------------
// Includes

// Standard library
#include <span>
#include <stdexcept>
#include <string>

// Nebulite
#include "Constants/Event.hpp"
#include "Constants/StandardCapture.hpp"
#include "Data/Document/KeyType.hpp"
#include "Data/Document/ScopedKey.hpp"
#include "Interaction/Context.hpp"
#include "Module/Domain/Common/Debug.hpp"
#include "Nebulite.hpp"
#include "Utility/StringHandler.hpp"

//------------------------------------------
namespace Nebulite::Module::Domain::Common {

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

// Fetch

Constants::Event Debug::fetchId(std::span<std::string const> const& args, Interaction::Context const& ctx, Interaction::ContextScope const& ctxScope) {
    if (args.size() < 2) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(ctx.self.capture);
    }
    auto const key = Data::ScopedKey(args[1]);
    ctxScope.self.set(key, ctx.self.getId());
    return Constants::Event::Success;
}

Constants::Event Debug::fetchName(std::span<std::string const> const& args, Interaction::Context const& ctx, Interaction::ContextScope const& ctxScope) {
    if (args.size() < 2) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(ctx.self.capture);
    }
    auto const key = Data::ScopedKey(args[1]);
    ctxScope.self.set(key, ctx.self.getName());
    return Constants::Event::Success;
}

Constants::Event Debug::print(std::span<std::string const> const& args, Interaction::Context const& ctx, Interaction::ContextScope const& ctxScope) {
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

Constants::Event Debug::printId(std::span<std::string const> const& /*args*/, Interaction::Context const& ctx, Interaction::ContextScope& /*ctxScope*/) {
    ctx.self.capture.log.println(ctx.self.getId());
    return Constants::Event::Success;
}

// Flow

// Ignore lint: Function warn always returns Constants::Event::Warning
// NOLINTNEXTLINE
Constants::Event Debug::warn(std::span<std::string const> const& args) const {
    if (args.size() < 2) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(domain.capture);
    }
    std::string const argStr = Utility::StringHandler::recombineArgs(args.subspan(1));
    domain.capture.warning.println(argStr);
    return Constants::Event::Warning;
}

Constants::Event Debug::error(std::span<std::string const> const& args, Interaction::Context const& ctx, Interaction::ContextScope& /*ctxScope*/) {
    auto const& argStr = Utility::StringHandler::recombineArgs(args.subspan(1));
    ctx.self.capture.error.println(argStr);
    return Constants::Event::Error;
}

Constants::Event Debug::func_throw(std::span<std::string const> const& args) {
    std::string const message = [&] {
        if (args.size() < 2) return std::string("");
        return Utility::StringHandler::recombineArgs(args.subspan(1));
    }();
    throw std::runtime_error(message);
}

Constants::Event Debug::mustThrow(std::span<std::string const> const& args, Interaction::Context& ctx, Interaction::ContextScope& ctxScope) {
    if (args.size() < 2) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(ctx.self.capture);
    }
    auto const funcCallStr = Utility::StringHandler::recombineArgs(args);
    try {
        (void) ctx.self.parseStr(funcCallStr, ctx, ctxScope);
    } catch (...) {
        // An exception was thrown as expected
        return Constants::Event::Success;
    }
    // No exception was thrown, but one was expected
    throw std::runtime_error("Expected an exception to be thrown, but no exception was thrown.");
}

} // namespace Nebulite::Module::Domain::Common
