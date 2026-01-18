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
    std::string const settings = Global::settings().serialize();
    Utility::FileManagement::WriteFile(defaultSettingsFile, settings);
    return Constants::ErrorTable::NONE();
}

Constants::Error Settings::overWriteSettingsFile() {
    // Overwrite settings file with default settings
    loadSettings(defaultSettingsFile);
    if (saveSettings() != Constants::ErrorTable::NONE()) {
        return Constants::ErrorTable::FILE::CRITICAL_INVALID_FILE();
    }
    return Constants::ErrorTable::NONE();
}

Constants::Error Settings::setSettingStr(std::span<std::string const> const& args) const {
    if (args.size() < 2) {
        return Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }
    if (args.size() > 2) {
        return Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS();
    }
    std::string const& key = args[0];
    std::string const& value = args[1];

    // Set string setting in global settings
    moduleScope.set<std::string>(moduleScope.getRootScope() + key, value);
    return Constants::ErrorTable::NONE();
}

Constants::Error Settings::setSettingInt(std::span<std::string const> const& args) const {
    if (args.size() < 2) {
            return Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }
    if (args.size() > 2) {
            return Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS();
    }
    std::string const& key = args[0];
    int const value = std::stoi(args[1]);

    // Set integer setting in global settings
    moduleScope.set<int>(moduleScope.getRootScope() + key, value);
    return Constants::ErrorTable::NONE();
}

//------------------------------------------
// Private methods

void Settings::loadSettings(std::string const& filename) {
    // Load settings file and only set known settings
    Data::JSON settings;
    settings.deserialize(filename);

    //---------------------------------------------------
    // Cherry-Pick values to set in global settings
    // Fallback to default values if not present
    // Any unknown settings are ignored
    // Later on, we may want to save all and ensure all known settings are present

    // Renderer settings
    moduleScope.set<uint16_t>(Key::resolutionX, settings.get<uint16_t>(Key::unscoped_resolutionX, 1000));
    moduleScope.set<uint16_t>(Key::resolutionY, settings.get<uint16_t>(Key::unscoped_resolutionY, 1000));
    moduleScope.set<uint8_t>(Key::resolutionScaling, settings.get<uint8_t>(Key::unscoped_resolutionScaling, 1));
    moduleScope.set<uint16_t>(Key::targetFPS, settings.get<uint16_t>(Key::unscoped_targetFPS, 60));

    // What commands to parse on different scenarios
    moduleScope.setSubDoc(Key::parseOnStartup, settings.getSubDoc(Key::unscoped_parseOnStartup));
    if (moduleScope.memberType(Key::parseOnStartup) != Data::KeyType::array) { // Load default if not present
        moduleScope.setEmptyArray(Key::parseOnStartup);
    }
    moduleScope.setSubDoc(Key::parseIfNoArgs, settings.getSubDoc(Key::unscoped_parseIfNoArgs));
    if (moduleScope.memberType(Key::parseIfNoArgs) != Data::KeyType::array) { // Load default if not present
        moduleScope.set<std::string>(Key::parseIfNoArgs + "[0]", "echo Nebulite opened with no arguments provided. Starting empty renderer.");
        moduleScope.set<std::string>(Key::parseIfNoArgs + "[1]", "echo Open the interactive console with <tab> key and type 'help' for available commands.");
        moduleScope.set<std::string>(Key::parseIfNoArgs + "[2]", "set-fps 60");
    }

    /**
     * @todo: Add more settings:
     *        - Console settings (like font size, colors, etc.)
     *        - Key bindings
     *        - Language/locale
     *        - etc...
     */

    //---------------------------------------------------
    // Check if settings file existed, if not, write default settings back to file
    if (settings.memberType("") != Data::KeyType::object) {
        Error::println("Settings: Settings file is invalid. Loading default values.");
        // Settings file does not exist!
        // Write default settings to file
        if (saveSettings() != Constants::ErrorTable::NONE()) {
            Error::println("Settings: Failed to write default settings to file: ", filename);
        }
    }
}

} // namespace Nebulite::DomainModule::GlobalSpace
