#include "Nebulite.hpp"
#include "DomainModule/GlobalSpace/InputMapping.hpp"
#include "DomainModule/GlobalSpace/Settings.hpp"
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

    // We move the settings into a key to match the same structure as in globalSpace
    // This way, we can use the scoped keys to set and access settings in the same way as we do for global settings
    settings.moveMember("", Key::scope);
    auto const& settingsFile = settings.shareManagedScopeBase(Key::scope);

    // Get custom settings
    moduleScope.setSubDoc(Key::customSettings, settingsFile.getSubDoc(Key::customSettings));
    if (moduleScope.memberType(Key::customSettings) != Data::KeyType::object) { // Load default if not present
        moduleScope.set<std::string>(Key::customSettings + ".__example__", "This is an example custom setting. You can add any settings you want under the 'custom' key in the settings file, and they will always be loaded into the globalspace.");
    }

    //---------------------------------------------------
    // Cherry-Pick all other values to set in global settings
    // Fallback to default values if not present
    // Any unknown settings are ignored
    // This way, we ensure the presence of all expected settings
    // For freeform custom settings, users may use the prefix from Key::customSettings

    // Renderer settings
    moduleScope.set<uint16_t>(Key::resolutionX, settingsFile.get<uint16_t>(Key::resolutionX, 1000));
    moduleScope.set<uint16_t>(Key::resolutionY, settingsFile.get<uint16_t>(Key::resolutionY, 1000));
    moduleScope.set<uint8_t>(Key::resolutionScaling, settingsFile.get<uint8_t>(Key::resolutionScaling, 1));
    moduleScope.set<uint16_t>(Key::targetFPS, settingsFile.get<uint16_t>(Key::targetFPS, 60));

    // Commands: On startup
    moduleScope.setSubDoc(Key::parseOnStartup, settingsFile.getSubDoc(Key::parseOnStartup));
    if (moduleScope.memberType(Key::parseOnStartup) != Data::KeyType::array) { // Load default if not present
        moduleScope.setEmptyArray(Key::parseOnStartup);
    }

    // Commands: When opening Nebulite with no arguments, e.g. by double-clicking the executable
    moduleScope.setSubDoc(Key::parseIfNoArgs, settingsFile.getSubDoc(Key::parseIfNoArgs));
    if (moduleScope.memberType(Key::parseIfNoArgs) != Data::KeyType::array) { // Load default if not present
        moduleScope.set<std::string>(Key::parseIfNoArgs + "[0]", "echo Nebulite opened with no arguments provided. Starting empty renderer.");
        moduleScope.set<std::string>(Key::parseIfNoArgs + "[1]", "echo Open the interactive console with <tab> key and type 'help' for available commands.");
        moduleScope.set<std::string>(Key::parseIfNoArgs + "[2]", "set-fps 60"); // TODO: add an initRenderer command and use that instead of setting fps here
    }

    // Input mappings
    moduleScope.setSubDoc(Key::inputMapping, settingsFile.getSubDoc(Key::inputMapping));
    if (moduleScope.memberType(Key::inputMapping) != Data::KeyType::object) { // Load default if not present
        InputMapping::loadDefaultMappings(moduleScope);
    }

    /**
     * @todo: Add more settings:
     *        - Console settings (like font size, colors, etc.)
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
