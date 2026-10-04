//------------------------------------------
// Includes

// Standard library
#include <string>

// Nebulite
#include "Nebulite/Constants/Event.hpp"
#include "Nebulite/Constants/StandardCapture.hpp"
#include "Nebulite/Interaction/Execution/Domain.hpp"
#include "Nebulite/Module/Domain/Common/Filesystem.hpp"
#include "Nebulite/Utility/Args/SegmentedStringView.hpp"
#include "Nebulite/Utility/Io/FileManagement.hpp"
#include "Nebulite/Utility/StringHandler.hpp"

//------------------------------------------
namespace Nebulite::Module::Domain::Common {

//------------------------------------------
// Update
Constants::Event Filesystem::updateHook() {
    // Add Domain-specific updates here!
    // General rule:
    // This is used to update all variables/states that are INTERNAL ONLY
    return Constants::Event::success;
}

//------------------------------------------
// Domain-Bound Functions

Constants::Event Filesystem::cat(Utility::Args::SegmentedStringView const& args) const{
    if (args.size() < 2) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(domain.capture);
    }

    auto const filePath = args.recombineSubspan(1);
    auto const fileContent = Utility::Io::FileManagement::loadFile(filePath);
    domain.capture.log.println(fileContent);
    return Constants::Event::success;
}

Constants::Event Filesystem::ls(Utility::Args::SegmentedStringView const& args) const {
    std::string const directoryPath = args.size() >= 2 ? args.recombineSubspan(1) : ".";
    auto const entries = Utility::Io::FileManagement::listContentInDirectory(directoryPath);
    domain.capture.log.println(Utility::StringHandler::createPaddedTable(entries, 80));
    return Constants::Event::success;
}

} // namespace Nebulite::Module::Domain::Common
