/**
 * @file InputMapping.hpp
 * @brief Provides input binding utilities for the Nebulite engine.
 *
 * This file contains a GlobalTree DomainModule to handle input bindings.
 * Note that this file is a work in progress!
 */

#pragma once

//------------------------------------------
// Includes

// General
#include "Interaction/Execution/DomainModule.hpp"

//------------------------------------------
// Forward declarations
namespace Nebulite::Core {
class GlobalSpace;
} // namespace Nebulite::Core

//------------------------------------------
namespace Nebulite::DomainModule::GlobalSpace {
/**
 * @class Nebulite::DomainModule::GlobalSpace::InputMapping
 * @brief DomainModule for mapping inputs to actions within the GlobalSpace.
 */
NEBULITE_DOMAINMODULE(Nebulite::Core::GlobalSpace, InputMapping) {
public:
    /**
     * @brief Applies the input mappings, processing any new input events (via the Global Document) and updating the binding states.
     */
    Constants::Error update() override;

    //------------------------------------------
    // Available Functions

    // TODO: Modify/reload input mapping at runtime
    //       store mappings in settings scope!

    // TODO: showMappings <on/off> function that displays an imgui window with the current mappings
    //       later on, this may be expanded to a full input mapping editor for actual GUI configuration of input

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables. 
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Core::GlobalSpace, InputMapping){
        // Setup pointer to polled input key in Renderer::Input module, to sync our updates with it and avoid missing deltas
        auto const sdlPolledInputKey = moduleScope.getRootScope() + "renderer.input.polled";
        sdlPolledInput = moduleScope.getStableDoublePointer(sdlPolledInputKey);

        // Load initial mappings from settings
        reloadMappings();
    }

    struct Key {
        DECLARE_SCOPE("")
        static auto constexpr mappingLocation = MAKE_SCOPED("input.");
    };

    //------------------------------------------
    // Other public functions

    /**
     * @brief Loads default input mappings into a given scope.
     */
    static void loadDefaultMappings(Data::JsonScopeBase& scope);

private:
    struct InputMappingSlot {
        // Just strings, as we need to combine them with the inputMapping key to form the full key for each slot of each mapping, e.g. "settings.inputMapping.jump.slotA.key"

        // The SDL_ key associated with the slot, e.g. "space"
        static auto constexpr associationA = "slotA.association";
        static auto constexpr associationB = "slotB.association";
        static auto constexpr associationC = "slotC.association";

        // The type of association, e.g. "current", "onPress", "onRelease"
        static auto constexpr actionA = "slotA.action";
        static auto constexpr actionB = "slotB.action";
        static auto constexpr actionC = "slotC.action";
    };

    double* sdlPolledInput = nullptr; // Key for checking if the Renderer::Input module has polled new input, to sync our updates with it and avoid missing deltas

    /**
     * @brief Represents a key association for input mapping.
     * @details The struct represents the Association between a key and its input type.
     *          - key: The name of the SDL key (e.g., "space", "w", "up").
     *          - type: The type of association, which can be one of the following:
     *                  - action::current: The current state of the key (pressed or not).
     *                  - action::onPress: Whether the key was pressed in the current frame.
     *                  - action::onRelease: Whether the key was released in the current frame.
     */
    struct association{
        std::string key; // e.g. "space"
        enum class action {
            empty,
            current,
            onPress,
            onRelease
            // TODO: onPressOrRelease
        } type = action::empty;
    };

    static association::action stringToAssociationType(std::string const& typeStr) {
        if (typeStr == "current") {
            return association::action::current;
        }
        if (typeStr == "onPress") {
            return association::action::onPress;
        }
        if (typeStr == "onRelease") {
            return association::action::onRelease;
        }
        return association::action::empty;
    }

    /**
     * @brief Represents a mapping entry for input actions.
     * @details Any input action can be associated with up to three keys.
     */
    struct mapEntry{
        association slotA{"", association::action::empty}; // First key associated with the action
        association slotB{"", association::action::empty}; // Second key associated with the action
        association slotC{"", association::action::empty}; // Third key associated with the action
    };

    /**
     * @brief Maps input actions to their associated keys.
     */
    absl::flat_hash_map<std::string, mapEntry> mappings;

    /**
     * @brief Reloads all input mappings from the settings scope.
     * @todo Not implemented yet, perhaps this could be done in the update function with a timed routine?
     *       Or a complex trigger when a settings menu is closed?
     */
    void reloadMappings();

    /**
     * @brief Processes all input mappings
     */
    void processMappings();
};
}   // namespace Nebulite::DomainModule::GlobalSpace
