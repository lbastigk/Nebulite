#include "DomainModule/GlobalSpace/Settings.hpp"
#include "Nebulite.hpp"
#include "Utility/FileManagement.hpp"

namespace Nebulite::DomainModule::GlobalSpace {

Constants::Error Settings::update() {
    return Constants::ErrorTable::NONE();
}

//------------------------------------------
// Available Functions

// Functree currently does not support static methods with no args...
// NOLINTNEXTLINE
Constants::Error Settings::saveSettings() {
    // Create JSON document with current settings
    std::string const settings = Nebulite::globalDoc().settings().serialize();
    Nebulite::Utility::FileManagement::WriteFile(defaultSettingsFile, settings);
    return Constants::ErrorTable::NONE();
}

//------------------------------------------
// Private methods

void Settings::loadSettings(std::string const& filename) {
    // Load settings file and only set known settings
    Nebulite::Data::JSON settings;
    settings.deserialize(filename);

    // Cherry-Pick values to set in global settings
    // Fallback to default values if not present
    // Any unknown settings are ignored
    // Later on, we may want to save all and ensure all known settings are present
    moduleScope.set<uint16_t>(Key::resolutionX, settings.get<uint16_t>(Key::unscoped_resolutionX, 1000));
    moduleScope.set<uint16_t>(Key::resolutionY, settings.get<uint16_t>(Key::unscoped_resolutionY, 1000));

    if (settings.memberType("") != Nebulite::Data::KeyType::object) {
        // Settings file does not exist!
        // Write default settings to file
        saveSettings();
    }
}

} // namespace Nebulite::DomainModule::GlobalSpace
