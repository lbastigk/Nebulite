//------------------------------------------
// Includes

// Standard library
#include <cstddef>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <vector>

// Nebulite
#include "Nebulite/Constants/Event.hpp"
#include "Nebulite/Constants/StandardCapture.hpp"
#include "Nebulite/Data/Document/KeyType.hpp"
#include "Nebulite/Interaction/Context.hpp"
#include "Nebulite/Interaction/Execution/Domain.hpp"
#include "Nebulite/Interaction/Logic/Assignment.hpp"
#include "Nebulite/Module/Domain/Common/SimpleData.hpp"
#include "Nebulite/Utility/StringHandler.hpp"

//------------------------------------------
namespace Nebulite::Module::Domain::Common {

Constants::Event SimpleData::updateHook() {
    // No periodic update needed, SimpleData is stateless
    return Constants::Event::success;
}

//------------------------------------------
// Domain-Bound Functions

//------------------------------------------
// General set/get/remove functions

Constants::Event SimpleData::set(std::span<std::string_view const> const args, Interaction::Context const& ctx, Interaction::ContextScope const& ctxScope) {
    auto lock = ctxScope.self.lock(); // Lock the domain for thread-safe access
    if (args.size() < 2) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(ctx.self.capture);
    }
    auto const key = ctxScope.self.getRootScope().addMember(args[1]);
    std::string const value = args.size() < 3 ? std::string("") : Utility::StringHandler::recombineArgs(args.subspan(2));
    ctxScope.self.set(key, value);
    return Constants::Event::success;
}

Constants::Event SimpleData::assign(std::span<std::string_view const> const args, Interaction::Context const& ctx, Interaction::ContextScope const& ctxScope){
    auto lock = ctxScope.self.lock(); // Lock the domain for thread-safe access
    if (args.size() < 2) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(ctx.self.capture);
    }
    auto const assignmentString = Utility::StringHandler::recombineArgs(args.subspan(1));
    Interaction::Logic::Assignment assignment;
    if (!assignment.parse(assignmentString)) {
        ctx.self.capture.error.println("Error: Failed to parse assignment string '", assignmentString, "'.");
        return Constants::Event::warning;
    }
    assignment.apply(ctxScope);
    return Constants::Event::success;
}

Constants::Event SimpleData::move(std::span<std::string_view const> const args, Interaction::Context const& ctx, Interaction::ContextScope const& ctxScope) {
    auto lock = ctxScope.self.lock(); // Lock the domain for thread-safe access
    if (args.size() < 3) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(ctx.self.capture);
    }
    if (args.size() > 3) {
        return Constants::StandardCapture::Warning::Functional::tooManyArgs(ctx.self.capture);
    }
    auto const sourceKey = ctxScope.self.getRootScope().addMember(args[1]);
    auto const targetKey = ctxScope.self.getRootScope().addMember(args[2]);
    ctxScope.self.moveMember(sourceKey, targetKey);
    return Constants::Event::success;
}

Constants::Event SimpleData::copy(std::span<std::string_view const> const args, Interaction::Context const& ctx, Interaction::ContextScope const& ctxScope) {
    auto lock = ctxScope.self.lock(); // Lock the domain for thread-safe access
    if (args.size() < 3) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(ctx.self.capture);
    }
    if (args.size() > 3) {
        return Constants::StandardCapture::Warning::Functional::tooManyArgs(ctx.self.capture);
    }
    auto const sourceKey = ctxScope.self.getRootScope().addMember(args[1]);
    auto const targetKey = ctxScope.self.getRootScope().addMember(args[2]);
    ctxScope.self.copyMember(sourceKey, targetKey);
    return Constants::Event::success;
}

Constants::Event SimpleData::keyDelete(std::span<std::string_view const> const args, Interaction::Context const& ctx, Interaction::ContextScope const& ctxScope) {
    auto lock = ctxScope.self.lock(); // Lock the domain for thread-safe access
    if (args.size() < 2) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(ctx.self.capture);
    }
    if (args.size() > 2) {
        return Constants::StandardCapture::Warning::Functional::tooManyArgs(ctx.self.capture);
    }
    auto const key = ctxScope.self.getRootScope().addMember(args[1]);
    ctxScope.self.removeMember(key);
    return Constants::Event::success;
}

//------------------------------------------
// Array manipulation functions

Constants::Event SimpleData::ensureArray(std::span<std::string_view const> const args, Interaction::Context const& ctx, Interaction::ContextScope const& ctxScope) {
    auto lock = ctxScope.self.lock(); // Lock the domain for thread-safe access
    if (args.size() < 2) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(ctx.self.capture);
    }
    if (args.size() > 2) {
        return Constants::StandardCapture::Warning::Functional::tooManyArgs(ctx.self.capture);
    }

    if (auto const key = ctxScope.self.getRootScope().addMember(args[1]); ctxScope.self.memberType(key) != Data::KeyType::array) {
        ctxScope.self.moveMember(key, key.addIndex(0)); // Move existing value to array index 0
    }
    return Constants::Event::success;
}

Constants::Event SimpleData::pushBack(std::span<std::string_view const> const args, Interaction::Context& ctx, Interaction::ContextScope& ctxScope){
    auto lock = ctxScope.self.lock(); // Lock the domain for thread-safe access
    if (args.size() > 3) {
        return Constants::StandardCapture::Warning::Functional::tooManyArgs(ctx.self.capture);
    }
    auto const key = ctxScope.self.getRootScope().addMember(args[1]);
    std::string value;
    if (args.size() < 3) {
        // Trying to push an empty value
        // Usually, this would be "{}" but we use an empty string for simplicity
        value = "";
    } else {
        value = args[2];
    }

    if (ctxScope.self.memberType(key) != Data::KeyType::array) {
        std::string command = __FUNCTION__;
        command += " " + std::string(ensureArrayName);
        command += " " + std::string(args[1]);
        if (Constants::Event const result = ctx.self.parseStr(command, ctx, ctxScope); result != Constants::Event::success) {
            ctx.self.capture.error.println("Error: Failed to ensure array for key '", std::string(args[1]), "'.");
            return result;
        }
    }

    std::size_t const size = ctxScope.self.memberSize(key);
    auto const itemKey = key.addIndex(size);
    ctxScope.self.set(itemKey, value);
    return Constants::Event::success;
}

Constants::Event SimpleData::popBack(std::span<std::string_view const> const args, Interaction::Context const& ctx, Interaction::ContextScope const& ctxScope) {
    auto lock = ctxScope.self.lock(); // Lock the domain for thread-safe access
    if (args.size() < 2) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(ctx.self.capture);
    }
    if (args.size() > 2) {
        return Constants::StandardCapture::Warning::Functional::tooManyArgs(ctx.self.capture);
    }
    auto const key = ctxScope.self.getRootScope().addMember(args[1]);

    if (ctxScope.self.memberType(key) != Data::KeyType::array) {
        std::vector<std::string_view> ensureArrayArgs = {"", args[1]};
        if (Constants::Event const result = ensureArray(ensureArrayArgs, ctx, ctxScope); result != Constants::Event::success) {
            ctx.self.capture.error.println("Error: Failed to ensure array for key '", std::string(args[1]), "'.");
            return result;
        }
    }

    std::size_t const size = ctxScope.self.memberSize(key);
    if (size == 0) {
        // nothing to pop out, not seen as error
        return Constants::Event::success;
    }

    auto const itemKey = key.addIndex(size - 1);
    ctxScope.self.removeMember(itemKey);
    return Constants::Event::success;
}

Constants::Event SimpleData::pushFront(std::span<std::string_view const> const args, Interaction::Context& ctx, Interaction::ContextScope& ctxScope) {
    auto lock = ctxScope.self.lock(); // Lock the domain for thread-safe access
    if (args.size() > 3) {
        return Constants::StandardCapture::Warning::Functional::tooManyArgs(ctx.self.capture);
    }
    auto const key = ctxScope.self.getRootScope().addMember(args[1]);
    std::string value;
    if (args.size() < 3) {
        // Trying to push an empty value
        // Usually, this would be "{}" but we use an empty string for simplicity
        value = "";
    } else {
        value = args[2];
    }

    if (ctxScope.self.memberType(key) != Data::KeyType::array) {
        std::vector<std::string_view> ensureArrayArgs = {"", args[1]};
        if (Constants::Event const result = ensureArray(ensureArrayArgs, ctx, ctxScope); result != Constants::Event::success) {
            ctx.self.capture.error.println("Error: Failed to ensure array for key '", std::string(args[1]), "'.");
            return result;
        }
    }

    std::size_t const size = ctxScope.self.memberSize(key);

    //------------------------------------------
    // Security check:
    // if any array item is a document, throw error
    // This feature is yet to be implemented!
    for (auto itemKey : key.getArrayKeys(size)) {
        if (Data::KeyType const itemType = ctxScope.self.memberType(itemKey); itemType == Data::KeyType::object) {
            ctx.self.capture.error.println("Error: Cannot pushFront into an array containing documents.");
            return Constants::StandardCapture::Error::Functional::functionNotImplemented(ctx.self.capture);
        }
    }

    //------------------------------------------
    // Move all existing items one step forward
    for (std::size_t i = size; i > 0; --i) {
        auto itemKey = key.addIndex(i - 1);
        auto itemValue = ctxScope.self.get<std::string>(itemKey).value_or("");
        auto newItemKey = key.addIndex(i);
        ctxScope.self.set(newItemKey, itemValue);
    }
    auto const itemKey = key.addIndex(0);
    ctxScope.self.set(itemKey, value);
    return Constants::Event::success;
}

Constants::Event SimpleData::popFront(std::span<std::string_view const> const args, Interaction::Context& ctx, Interaction::ContextScope& ctxScope) {
    auto lock = ctxScope.self.lock(); // Lock the domain for thread-safe access
    if (args.size() < 2) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(ctx.self.capture);
    }
    if (args.size() > 2) {
        return Constants::StandardCapture::Warning::Functional::tooManyArgs(ctx.self.capture);
    }
    auto const key = ctxScope.self.getRootScope().addMember(args[1]);

    if (ctxScope.self.memberType(key) != Data::KeyType::array) {
        std::string command = __FUNCTION__;
        command += " " + std::string(ensureArrayName);
        command += " " + std::string(args[1]);
        if (Constants::Event const result = ctx.self.parseStr(command, ctx, ctxScope); result != Constants::Event::success) {
            ctx.self.capture.error.println("Error: Failed to ensure array for key '", std::string(args[1]), "'.");
            return result;
        }
    }

    std::size_t const size = ctxScope.self.memberSize(key);

    //------------------------------------------
    // Security check:
    // if any array item is a document, throw error
    // This feature is yet to be implemented!
    for (auto itemKey : key.getArrayKeys(size)) {
        if (ctxScope.self.memberType(itemKey) == Data::KeyType::object) {
            ctx.self.capture.error.println("Error: Cannot pushFront into an array containing documents.");
            return Constants::StandardCapture::Error::Functional::functionNotImplemented(ctx.self.capture);
        }
    }

    //------------------------------------------
    // Move all existing items one step back
    for (auto const [newItemKey, itemKey] : key.getArrayKeys(size) | std::views::adjacent<2>) {
        auto itemValue = ctxScope.self.get<std::string>(itemKey).value_or("");
        ctxScope.self.set(newItemKey, itemValue);
    }
    // Remove the last item
    auto const lastItemKey = key.addIndex(size - 1);
    ctxScope.self.removeMember(lastItemKey);
    return Constants::Event::success;
}

} // namespace Nebulite::Module::Domain::Common
