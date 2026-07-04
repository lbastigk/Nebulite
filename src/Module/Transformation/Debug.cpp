//------------------------------------------
// Includes

// Standard library
#include <ranges>
#include <span>
#include <stdexcept>
#include <string>

// Nebulite
#include "Data/Document/JsonScope.hpp"
#include "Module/Transformation/Debug.hpp"
#include "Nebulite.hpp"
#include "Utility/IO/FileManagement.hpp"
#include "Utility/StringHandler.hpp"

//------------------------------------------
namespace Nebulite::Module::Transformation {

void Debug::bindTransformations() {
    bindTransformation(&Debug::echo, echoName, echoDesc);
    bindTransformation(&Debug::warn, warnName, warnDesc);
    bindTransformation(&Debug::error, errorName, errorDesc);
    bindTransformation(&Debug::print, printName, printDesc);
    bindTransformation(&Debug::unreachable, unreachableName, unreachableDesc);
    bindTransformation(&Debug::store, storeName, storeDesc);
}

// Since this is for debugging only, we pass the output directly to global capture, instead of a local capture

bool Debug::echo(std::span<std::string_view const> const& args) {
    Global::capture().log.println(Utility::StringHandler::recombineArgs(args.subspan(1)));
    return true;
}

bool Debug::warn(std::span<std::string_view const> const& args) {
    Global::capture().warning.println(Utility::StringHandler::recombineArgs(args.subspan(1)));
    return true;
}

bool Debug::error(std::span<std::string_view const> const& args) {
    Global::capture().error.println(Utility::StringHandler::recombineArgs(args.subspan(1)));
    return true;
}

// NOLINTNEXTLINE
bool Debug::print(std::span<std::string_view const> const& args, Data::JsonScope& jsonDoc) {
    // Print to cout, no modifications
    if (args.size() > 1) {
        for (auto const& arg : args | std::views::drop(1)) {
            if (std::string const value = jsonDoc.serialize(rootKey.addMember(arg)); value.ends_with('\n')) {
                Global::capture().log.print(value);
            } else {
                Global::capture().log.println(value);
            }
        }
    } else {
        if (std::string const value = jsonDoc.serialize(); value.ends_with('\n')) {
            Global::capture().log.print(value);
        } else {
            Global::capture().log.println(value);
        }
    }
    return true;
}

bool Debug::unreachable(std::span<std::string_view const> const& args){
    std::string const message = "Unreachable transformation path reached! " + Utility::StringHandler::recombineArgs(args.subspan(1));
    throw std::logic_error(message);
}

bool Debug::store(std::span<std::string_view const> const& args, Data::JsonScope const& jsonDoc){
    if (args.size() < 2) {
        Global::capture().error.println("store transformation requires at least one argument for the file name to store the JSON value under.");
        return false;
    }
    auto const filename = Utility::StringHandler::recombineArgs(args.subspan(1));
    if (!Utility::IO::FileManagement::WriteFile(filename, jsonDoc.serialize())) {
        Global::capture().error.println("Error writing to file.");
        return false;
    }
    return true;
}

} // namespace Nebulite::Module::Transformation
