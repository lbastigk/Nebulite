//------------------------------------------
// Includes

// Standard library
#include <ranges>
#include <stdexcept>
#include <string>

// Nebulite
#include "Nebulite/Data/Document/JsonScope.hpp"
#include "Nebulite/Module/Transformation/Debug.hpp"
#include "Nebulite/Nebulite.hpp"
#include "Nebulite/Utility/Args/CmdArgs.hpp"
#include "Nebulite/Utility/Io/FileManagement.hpp"
#include "Nebulite/Utility/StringHandler.hpp"

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

bool Debug::echo(Utility::Args::SSV const& args) {
    Global::capture().log.println(Utility::StringHandler::recombineArgs(args.subspan(1)));
    return true;
}

bool Debug::warn(Utility::Args::SSV const& args) {
    Global::capture().warning.println(Utility::StringHandler::recombineArgs(args.subspan(1)));
    return true;
}

bool Debug::error(Utility::Args::SSV const& args) {
    Global::capture().error.println(Utility::StringHandler::recombineArgs(args.subspan(1)));
    return true;
}

// NOLINTNEXTLINE
bool Debug::print(Utility::Args::SSV const& args, Data::JsonScope& jsonDoc) {
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

bool Debug::unreachable(Utility::Args::SSV const& args){
    std::string const message = "Unreachable transformation path reached! " + Utility::StringHandler::recombineArgs(args.subspan(1));
    throw std::logic_error(message);
}

bool Debug::store(Utility::Args::SSV const& args, Data::JsonScope const& jsonDoc){
    if (args.size() < 2) {
        Global::capture().error.println("store transformation requires at least one argument for the file name to store the JSON value under.");
        return false;
    }
    auto const filename = Utility::StringHandler::recombineArgs(args.subspan(1));
    if (!Utility::Io::FileManagement::writeFile(filename, jsonDoc.serialize())) {
        Global::capture().error.println("Error writing to file.");
        return false;
    }
    return true;
}

} // namespace Nebulite::Module::Transformation
