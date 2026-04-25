#include "Nebulite.hpp"
#include "Module/Domain/Common/Filesystem.hpp"
#include "Utility/IO/FileManagement.hpp"
#include "Utility/StringHandler.hpp"

namespace Nebulite::Module::Domain::Common {

//------------------------------------------
// Update
Constants::Event Filesystem::updateHook() {
    // Add Domain-specific updates here!
    // General rule:
    // This is used to update all variables/states that are INTERNAL ONLY
    return Constants::Event::Success;
}

//------------------------------------------
// Domain-Bound Functions

Constants::Event Filesystem::cat(std::span<std::string const> const& args) const{
    if (args.size() < 2) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(domain.capture);
    }

    auto const filePath = Utility::StringHandler::recombineArgs(args.subspan(1));
    auto const fileContent = Utility::IO::FileManagement::LoadFile(filePath);
    domain.capture.log.println(fileContent);
    return Constants::Event::Success;
}

Constants::Event Filesystem::ls(std::span<std::string const> const& args) const {
    std::string const directoryPath = args.size() >= 2 ? Utility::StringHandler::recombineArgs(args.subspan(1)) : ".";
    auto const entries = Utility::IO::FileManagement::listFilesAndDirectoriesInPath(directoryPath);
    domain.capture.log.println(Utility::StringHandler::createPaddedTable(entries));
    return Constants::Event::Success;
}

} // namespace Nebulite::Module::Domain::Common
