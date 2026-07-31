#ifndef NEBULITE_MODULE_DOMAIN_GLOBALSPACE_SETTINGS_HPP
#define NEBULITE_MODULE_DOMAIN_GLOBALSPACE_SETTINGS_HPP

//------------------------------------------
// Includes

// Standard library
#include <span>
#include <string>
#include <string_view>

// Nebulite
#include "Nebulite/Constants/Event.hpp"
#include "Nebulite/Data/Document/KeyGroup.hpp"
#include "Nebulite/Module/Base/DomainModule.hpp"

//------------------------------------------
// Forward declarations

namespace Nebulite::Core {
class GlobalSpace;
} // namespace Nebulite::Core

//------------------------------------------
namespace Nebulite::Module::Domain::GlobalSpace {
/**
 * @class Nebulite::Module::Domain::GlobalSpace::Settings
 * @brief DomainModule for loading settings into GlobalSpace for other Domains/DomainModules to use.
 */
class Settings final : public Base::DomainModule<Core::GlobalSpace> {
public:
    [[nodiscard]] Constants::Event updateHook() override;
    void reinit() override {}


    //------------------------------------------
    // Available Functions

    [[nodiscard]] Constants::Event saveSettings() const ;
    static auto constexpr saveSettingsName = "settings save";
    static auto constexpr saveSettingsDesc = "Saves the current global settings to the default filename.\n"
        "\n"
        "Usage: settings save\n";

    [[nodiscard]] Constants::Event overWriteSettingsFile() const ;
    static auto constexpr overWriteSettingsFileName = "settings save-standards";
    static auto constexpr overWriteSettingsFileDesc = "Overwrites the settings file with default settings.\n"
        "\n"
        "Usage: settings save-standards\n";

    [[nodiscard]] Constants::Event setSettingStr(std::span<std::string_view const> const& args) const ;
    static auto constexpr setSettingName = "settings set-string";
    static auto constexpr setSettingDesc = "Sets a global setting to a specified value.\n"
        "\n"
        "Usage: settings set-string <key> <value>\n";

    [[nodiscard]] Constants::Event setSettingInt(std::span<std::string_view const> const& args) const ;
    static auto constexpr setSettingIntName = "settings set-integer";
    static auto constexpr setSettingIntDesc = "Sets a global setting to a specified integer value.\n"
        "\n"
        "Usage: settings set-integer <key> <value>\n";

    //------------------------------------------
    // Categories

    static auto constexpr settingsName = "settings";
    static auto constexpr settingsDesc = "Functions for managing global settings.";

    //------------------------------------------
    // Other constants
    static auto constexpr defaultSettingsFile = "Resources/settings.jsonc";

    //------------------------------------------
    // Settings keys
    struct Key : Data::KeyGroup<"settings."> {
        // Custom settings
        static auto constexpr customSettings = makeScoped("custom");

        // Use scoped keys to set and access from GlobalSpace

        // Renderer-related settings
        static auto constexpr resolutionX = makeScoped("renderer.resolution.w");
        static auto constexpr resolutionY = makeScoped("renderer.resolution.h");
        static auto constexpr resolutionScaling = makeScoped("renderer.resolution.scale");
        static auto constexpr targetFps = makeScoped("renderer.targetFPS");
        static auto constexpr fontMono = makeScoped("renderer.font.mono");
        static auto constexpr fontStandard = makeScoped("renderer.font.standard");
        static auto constexpr cursor = makeScoped("renderer.cursor");

        static auto constexpr fontScale = makeScoped("renderer.font.scale");
        static auto constexpr fontSize1 = makeScoped("renderer.font.size[0]");
        static auto constexpr fontSize2 = makeScoped("renderer.font.size[1]");
        static auto constexpr fontSize3 = makeScoped("renderer.font.size[2]");

        // Startup-related settings
        static auto constexpr parseOnStartup = makeScoped("parse.onStartup");
        static auto constexpr parseIfNoArgs = makeScoped("parse.ifNoArgs");

        // Input-Mapping
        static auto constexpr inputMapping = makeScoped("inputMapping");
    };

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables. 
     */
    explicit Settings(ConstructorParams const& params) : DomainModule(params) {
        if (loadSettings(defaultSettingsFile) != Constants::Event::Success) {
            logInitError();
        }

        bindCategory(settingsName, settingsDesc);
        bindFunction(&Settings::saveSettings, saveSettingsName, saveSettingsDesc);
        bindFunction(&Settings::overWriteSettingsFile, overWriteSettingsFileName, overWriteSettingsFileDesc);
        bindFunction(&Settings::setSettingStr, setSettingName, setSettingDesc);
        bindFunction(&Settings::setSettingInt, setSettingIntName, setSettingIntDesc);
    }

private:
    [[nodiscard]] Constants::Event loadSettings(std::string const& filename) const ;

    void logInitError() const ;
};
} // namespace Nebulite::Module::Domain::GlobalSpace
#endif // NEBULITE_MODULE_DOMAIN_GLOBALSPACE_SETTINGS_HPP
