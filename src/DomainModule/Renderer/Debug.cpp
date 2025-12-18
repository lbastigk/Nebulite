//------------------------------------------
// Includes


// Nebulite
#include "DomainModule/Renderer/Debug.hpp"

//------------------------------------------
namespace Nebulite::DomainModule::Renderer {

std::string const Debug::debug_name = "renderer-debug";
std::string const Debug::debug_desc = "Debugging and logging functions for the Renderer.";

Constants::Error Debug::update() {
    return Constants::ErrorTable::NONE();
}

//------------------------------------------

// NOLINTNEXTLINE
Constants::Error Debug::windowStatus(std::span<std::string const> const& args) {
    return Constants::ErrorTable::FUNCTIONAL::FEATURE_NOT_IMPLEMENTED();
}

std::string const Debug::windowStatus_name = "renderer-debug window-status";
std::string const Debug::windowStatus_desc = "Prints the current status of the renderer window.";

} // namespace Nebulite::DomainModule::GlobalSpace