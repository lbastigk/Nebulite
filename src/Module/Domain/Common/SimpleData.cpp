#include "Nebulite.hpp"
#include "Module/Domain/Common/SimpleData.hpp"

namespace Nebulite::Module::Domain::Common {

Constants::Event SimpleData::updateHook() {return Constants::Event::Success;} // No periodic update needed, SimpleData is stateless

//------------------------------------------
// Domain-Bound Functions

//------------------------------------------
// General set/get/remove functions

Constants::Event SimpleData::set(std::span<std::string const> const& args, Interaction::Context const& ctx, Interaction::ContextScope const& ctxScope) {
    auto lock = ctxScope.self.lock(); // Lock the domain for thread-safe access
    if (args.size() < 3) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(ctx.self.capture);
    }

    auto const key = ctxScope.self.getRootScope() + args[1];
    std::string const value = Utility::StringHandler::recombineArgs(args.subspan(2));
    ctxScope.self.set(key, value);
    return Constants::Event::Success;
}

Constants::Event SimpleData::assign(std::span<std::string const> const& args, Interaction::Context const& ctx, Interaction::ContextScope const& ctxScope){
    auto lock = ctxScope.self.lock(); // Lock the domain for thread-safe access
    if (args.size() < 2) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(ctx.self.capture);
    }
    auto const assignmentString = Utility::StringHandler::recombineArgs(args.subspan(1));
    Interaction::Logic::Assignment assignment;
    if (!assignment.parse(assignmentString)) {
        ctx.self.capture.error.println("Error: Failed to parse assignment string '", assignmentString, "'.");
        return Constants::Event::Warning;
    }
    assignment.apply(ctxScope);
    return Constants::Event::Success;
}

Constants::Event SimpleData::move(std::span<std::string const> const& args, Interaction::Context const& ctx, Interaction::ContextScope const& ctxScope) {
    auto lock = ctxScope.self.lock(); // Lock the domain for thread-safe access
    if (args.size() < 3) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(ctx.self.capture);
    }
    if (args.size() > 3) {
        return Constants::StandardCapture::Warning::Functional::tooManyArgs(ctx.self.capture);
    }
    auto const sourceKey = ctxScope.self.getRootScope() + args[1];
    auto const targetKey = ctxScope.self.getRootScope() + args[2];
    ctxScope.self.moveMember(sourceKey, targetKey);
    return Constants::Event::Success;
}

Constants::Event SimpleData::copy(std::span<std::string const> const& args, Interaction::Context const& ctx, Interaction::ContextScope const& ctxScope) {
    auto lock = ctxScope.self.lock(); // Lock the domain for thread-safe access
    if (args.size() < 3) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(ctx.self.capture);
    }
    if (args.size() > 3) {
        return Constants::StandardCapture::Warning::Functional::tooManyArgs(ctx.self.capture);
    }
    auto const sourceKey = ctxScope.self.getRootScope() + args[1];
    auto const targetKey = ctxScope.self.getRootScope() + args[2];
    ctxScope.self.copyMember(sourceKey, targetKey);
    return Constants::Event::Success;
}

Constants::Event SimpleData::keyDelete(std::span<std::string const> const& args, Interaction::Context const& ctx, Interaction::ContextScope const& ctxScope) {
    auto lock = ctxScope.self.lock(); // Lock the domain for thread-safe access
    if (args.size() < 2) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(ctx.self.capture);
    }
    if (args.size() > 2) {
        return Constants::StandardCapture::Warning::Functional::tooManyArgs(ctx.self.capture);
    }
    auto const key = ctxScope.self.getRootScope() + args[1];
    ctxScope.self.removeMember(key);
    return Constants::Event::Success;
}

//------------------------------------------
// Array manipulation functions

Constants::Event SimpleData::ensureArray(std::span<std::string const> const& args, Interaction::Context const& ctx, Interaction::ContextScope const& ctxScope) {
    auto lock = ctxScope.self.lock(); // Lock the domain for thread-safe access
    if (args.size() < 2) {
        ctx.self.capture.error.println("Error: Too few arguments for ensureArray command.");
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(ctx.self.capture);
    }
    if (args.size() > 2) {
        ctx.self.capture.error.println("Error: Too many arguments for ensureArray command.");
        return Constants::StandardCapture::Warning::Functional::tooManyArgs(ctx.self.capture);
    }

    if (auto const key = ctxScope.self.getRootScope() + args[1]; ctxScope.self.memberType(key) != Data::KeyType::array) {
        ctxScope.self.moveMember(key, key + "[0]"); // Move existing value to array index 0
    }
    return Constants::Event::Success;
}

Constants::Event SimpleData::push_back(std::span<std::string const> const& args, Interaction::Context& ctx, Interaction::ContextScope& ctxScope){
    auto lock = ctxScope.self.lock(); // Lock the domain for thread-safe access
    if (args.size() > 3) {
        ctx.self.capture.error.println("Error: Too many arguments for push_front command.");
        return Constants::StandardCapture::Warning::Functional::tooManyArgs(ctx.self.capture);
    }
    auto const key = ctxScope.self.getRootScope() + args[1];
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
        command += " " + std::string(ensureArray_name);
        command += " " + std::string(args[1]);
        if (Constants::Event const result = ctx.self.parseStr(command, ctx, ctxScope); result != Constants::Event::Success) {
            ctx.self.capture.error.println("Error: Failed to ensure array for key '", std::string(args[1]), "'.");
            return result;
        }
    }

    size_t const size = ctxScope.self.memberSize(key);
    auto const itemKey = key + "[" + std::to_string(size) + "]";
    ctxScope.self.set(itemKey, value);
    return Constants::Event::Success;
}

Constants::Event SimpleData::pop_back(std::span<std::string const> const& args, Interaction::Context& ctx, Interaction::ContextScope& ctxScope) {
    auto lock = ctxScope.self.lock(); // Lock the domain for thread-safe access
    if (args.size() < 2) {
        ctx.self.capture.error.println("Error: Too few arguments for push_back command.");
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(ctx.self.capture);
    }
    if (args.size() > 2) {
        ctx.self.capture.error.println("Error: Too many arguments for push_back command.");
        return Constants::StandardCapture::Warning::Functional::tooManyArgs(ctx.self.capture);
    }
    auto const key = ctxScope.self.getRootScope() + args[1];

    if (ctxScope.self.memberType(key) != Data::KeyType::array) {
        std::string command = __FUNCTION__;
        command += " " + std::string(ensureArray_name);
        command += " " + std::string(args[1]);
        if (Constants::Event const result = ctx.self.parseStr(command, ctx, ctxScope); result != Constants::Event::Success) {
            ctx.self.capture.error.println("Error: Failed to ensure array for key '", std::string(args[1]), "'.");
            return result;
        }
    }

    size_t const size = ctxScope.self.memberSize(key);
    if (size == 0) {
        // nothing to pop out, not seen as error
        return Constants::Event::Success;
    }

    auto const itemKey = key + "[" + std::to_string(size - 1) + "]";
    ctxScope.self.removeMember(itemKey);
    return Constants::Event::Success;
}

Constants::Event SimpleData::push_front(std::span<std::string const> const& args, Interaction::Context& ctx, Interaction::ContextScope& ctxScope) {
    auto lock = ctxScope.self.lock(); // Lock the domain for thread-safe access
    if (args.size() > 3) {
        ctx.self.capture.error.println("Error: Too many arguments for push_front command.");
        return Constants::StandardCapture::Warning::Functional::tooManyArgs(ctx.self.capture);
    }
    auto const key = ctxScope.self.getRootScope() + args[1];
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
        command += " " + std::string(ensureArray_name);
        command += " " + std::string(args[1]);
        if (Constants::Event const result = ctx.self.parseStr(command, ctx, ctxScope); result != Constants::Event::Success) {
            ctx.self.capture.error.println("Error: Failed to ensure array for key '", std::string(args[1]), "'.");
            return result;
        }
    }

    size_t const size = ctxScope.self.memberSize(key);

    //------------------------------------------
    // Security check:
    // if any array item is a document, throw error
    // This feature is yet to be implemented!
    for (size_t i = 0; i < size; ++i) {
        auto itemKey = key + "[" + std::to_string(i) + "]";
        if (Data::KeyType const itemType = ctxScope.self.memberType(itemKey); itemType == Data::KeyType::object) {
            ctx.self.capture.error.println("Error: Cannot push_front into an array containing documents.");
            return Constants::StandardCapture::Error::Functional::functionNotImplemented(ctx.self.capture);
        }
    }

    //------------------------------------------
    // Move all existing items one step forward
    for (size_t i = size; i > 0; --i) {
        auto itemKey = key + "[" + std::to_string(i - 1) + "]";
        auto itemValue = ctxScope.self.get<std::string>(itemKey).value_or("");
        auto newItemKey = key + "[" + std::to_string(i) + "]";
        ctxScope.self.set(newItemKey, itemValue);
    }
    auto const itemKey = key + "[0]";
    ctxScope.self.set(itemKey, value);
    return Constants::Event::Success;
}

Constants::Event SimpleData::pop_front(std::span<std::string const> const& args, Interaction::Context& ctx, Interaction::ContextScope& ctxScope) {
    auto lock = ctxScope.self.lock(); // Lock the domain for thread-safe access
    if (args.size() < 2) {
        ctx.self.capture.error.println("Error: Too few arguments for pop_front command.");
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(ctx.self.capture);
    }
    if (args.size() > 2) {
        ctx.self.capture.error.println("Error: Too many arguments for pop_front command.");
        return Constants::StandardCapture::Warning::Functional::tooManyArgs(ctx.self.capture);
    }
    auto const key = ctxScope.self.getRootScope() + args[1];

    if (ctxScope.self.memberType(key) != Data::KeyType::array) {
        std::string command = __FUNCTION__;
        command += " " + std::string(ensureArray_name);
        command += " " + std::string(args[1]);
        if (Constants::Event const result = ctx.self.parseStr(command, ctx, ctxScope); result != Constants::Event::Success) {
            ctx.self.capture.error.println("Error: Failed to ensure array for key '", std::string(args[1]), "'.");
            return result;
        }
    }

    size_t const size = ctxScope.self.memberSize(key);

    //------------------------------------------
    // Security check:
    // if any array item is a document, throw error
    // This feature is yet to be implemented!
    for (size_t i = 0; i < size; ++i) {
        if (ctxScope.self.memberType(key + "[" + std::to_string(i) + "]") == Data::KeyType::object) {
            ctx.self.capture.error.println("Error: Cannot push_front into an array containing documents.");
            return Constants::StandardCapture::Error::Functional::functionNotImplemented(ctx.self.capture);
        }
    }

    //------------------------------------------
    // Move all existing items one step back
    for (size_t i = 1; i < size; i++) {
        auto itemKey = key + "[" + std::to_string(i) + "]";
        auto itemValue = ctxScope.self.get<std::string>(itemKey).value_or("");
        auto newItemKey = key + "[" + std::to_string(i - 1) + "]";
        ctxScope.self.set(newItemKey, itemValue);
    }
    // Remove the last item
    auto const lastItemKey = key + "[" + std::to_string(size - 1) + "]";
    ctxScope.self.removeMember(lastItemKey);
    return Constants::Event::Success;
}

} // namespace Nebulite::Module::Domain::Common
