/**
 * @file Settings.hpp
 * @brief Contains the declaration of the Settings DomainModule for the GlobalSpace domain.
 */

#ifndef NEBULITE_GSDM_SETTINGS_HPP
#define NEBULITE_GSDM_SETTINGS_HPP

//------------------------------------------
// Includes

// Standard library
#include <memory>

// Nebulite
#include "Constants/ErrorTypes.hpp"
#include "Interaction/Execution/DomainModule.hpp"


//------------------------------------------
// Forward declarations
namespace Nebulite::Core {
class GlobalSpace; // Forward declaration of domain class GlobalSpace
} // namespace Nebulite::Core


//------------------------------------------
namespace Nebulite::DomainModule::GlobalSpace {
/**
 * @class Nebulite::DomainModule::GlobalSpace::Settings
 * @brief DomainModule for loading settings into GlobalSpace for other Domains/DomainModules to use.
 */
NEBULITE_DOMAINMODULE(Nebulite::Core::GlobalSpace, Settings) {
public:
    [[nodiscard]] Constants::Error update() override;
    void reinit() override {}


    //------------------------------------------
    // Available Functions

    [[nodiscard]] Constants::Error saveSettings();
    static auto constexpr saveSettings_name = "settings save";
    static auto constexpr saveSettings_desc = "Saves the current global settings to the default filename.\n"
        "\n"
        "Usage: settings save\n";

    [[nodiscard]] Constants::Error overWriteSettingsFile();
    static auto constexpr overWriteSettingsFile_name = "settings save-standards";
    static auto constexpr overWriteSettingsFile_desc = "Overwrites the settings file with default settings.\n"
        "\n"
        "Usage: settings save-standards\n";

    [[nodiscard]] Constants::Error setSettingStr(std::span<std::string const> const& args) const ;
    static auto constexpr setSetting_name = "settings set-string";
    static auto constexpr setSetting_desc = "Sets a global setting to a specified value.\n"
        "\n"
        "Usage: settings set-string <key> <value>\n";

    [[nodiscard]] Constants::Error setSettingInt(std::span<std::string const> const& args) const ;
    static auto constexpr setSettingInt_name = "settings set-integer";
    static auto constexpr setSettingInt_desc = "Sets a global setting to a specified integer value.\n"
        "\n"
        "Usage: settings set-integer <key> <value>\n";

    //------------------------------------------
    // Category names
    static auto constexpr settings_name = "settings";

    //------------------------------------------
    // Other constants
    static auto constexpr defaultSettingsFile = "Resources/settings.jsonc";

    //------------------------------------------
    // Settings keys
    struct Key {
    private:
        friend class Settings;

        // Use unscoped keys to load from file
        static auto constexpr unscoped_resolutionX = "renderer.resolutionX"; // TODO: change to resolution.w
        static auto constexpr unscoped_resolutionY = "renderer.resolutionY"; // TODO: change to resolution.h
        static auto constexpr unscoped_resolutionScaling = "renderer.resolutionScaling";
        static auto constexpr unscoped_targetFPS = "renderer.targetFPS";

        // What to parse on different scenarios
        static auto constexpr unscoped_parseOnStartup = "parse.onStartup"; // What to always parse on startup
        static auto constexpr unscoped_parseIfNoArgs = "parse.ifNoArgs"; // What to parse on startup if no command line args provided
    public:
        // Use scoped keys to set and access from GlobalSpace
        static auto constexpr scope = "settings.";
        static auto constexpr resolutionX = Data::ScopedKeyView::create<scope>(unscoped_resolutionX);
        static auto constexpr resolutionY = Data::ScopedKeyView::create<scope>(unscoped_resolutionY);
        static auto constexpr resolutionScaling = Data::ScopedKeyView::create<scope>(unscoped_resolutionScaling);
        static auto constexpr targetFPS = Data::ScopedKeyView::create<scope>(unscoped_targetFPS);
        static auto constexpr parseOnStartup = Data::ScopedKeyView::create<scope>(unscoped_parseOnStartup);
        static auto constexpr parseIfNoArgs = Data::ScopedKeyView::create<scope>(unscoped_parseIfNoArgs);
    };

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables. 
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Core::GlobalSpace, Settings) {
        loadSettings(defaultSettingsFile);

        bindCategory(settings_name, "Functions for managing global settings.");
        BIND_FUNCTION(&Settings::saveSettings, saveSettings_name, saveSettings_desc);
        BIND_FUNCTION(&Settings::overWriteSettingsFile, overWriteSettingsFile_name, overWriteSettingsFile_desc);
        BIND_FUNCTION(&Settings::setSettingStr, setSetting_name, setSetting_desc);
        BIND_FUNCTION(&Settings::setSettingInt, setSettingInt_name, setSettingInt_desc);
    }

private:
    void loadSettings(std::string const& filename);
};
} // namespace Nebulite::DomainModule::GlobalSpace
#endif // NEBULITE_GSDM_SETTINGS_HPP
