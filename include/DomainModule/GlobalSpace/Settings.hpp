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
    Constants::Error update() override;
    void reinit() override {}


    //------------------------------------------
    // Available Functions

    Constants::Error saveSettings();
    static auto constexpr saveSettings_name = "settings save";
    static auto constexpr saveSettings_desc = "Saves the current global settings to the default filename.\n"
        "\n"
        "Usage: settings save\n";

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
        static auto constexpr unscoped_resolutionX = "renderer.resolutionX";
        static auto constexpr unscoped_resolutionY = "renderer.resolutionY";
        static auto constexpr unscoped_resolutionScaling = "renderer.resolutionScaling";
    public:
        // Use scoped keys to set and access from GlobalSpace
        static auto constexpr scope = "settings.";
        static auto constexpr resolutionX = Data::ScopedKeyView::create<scope>(unscoped_resolutionX);
        static auto constexpr resolutionY = Data::ScopedKeyView::create<scope>(unscoped_resolutionY);
        static auto constexpr resolutionScaling = Data::ScopedKeyView::create<scope>(unscoped_resolutionScaling);

    };

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables. 
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Core::GlobalSpace, Settings) {
        loadSettings(defaultSettingsFile);

        (void)bindCategory(settings_name, "Functions for managing global settings.");
        BINDFUNCTION(&Settings::saveSettings, saveSettings_name, saveSettings_desc);
    }

private:
    void loadSettings(std::string const& filename);
};
} // namespace Nebulite::DomainModule::GlobalSpace
#endif // NEBULITE_GSDM_SETTINGS_HPP
