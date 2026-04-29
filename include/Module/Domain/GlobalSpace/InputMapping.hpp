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

// Nebulite
#include "Interaction/Execution/DomainModule.hpp"
#include "Module/Domain/Renderer/Input.hpp"

//------------------------------------------
// Forward declarations

namespace Nebulite::Core {
class GlobalSpace;
} // namespace Nebulite::Core

//------------------------------------------
namespace Nebulite::Module::Domain::GlobalSpace {
/**
 * @class Nebulite::Module::Domain::GlobalSpace::InputMapping
 * @brief DomainModule for mapping inputs to actions within the GlobalSpace.
 */
NEBULITE_DOMAINMODULE(Nebulite::Core::GlobalSpace, InputMapping) {
public:
    /**
     * @brief Applies the input mappings, processing any new input events (via the Global Document) and updating the binding states.
     */
    [[nodiscard]] Constants::Event updateHook() override;

    //------------------------------------------
    // Available Functions

    [[nodiscard]] Constants::Event lockOnce(std::span<std::string const> const& args);
    static auto constexpr lockOnceName = "input-mapping lock once";
    static auto constexpr lockOnceDesc = "Locks an action for the current frame, preventing it from being triggered by any of its associated keys.\n"
        "Allows locking of entire categories of actions by using a structured action name, e.g. 'movement::up' or 'combat::primaryAttack' can be locked with 'movement' or 'combat'.\n"
        "Usage: input-mapping lock once <actionName>\n";

    [[nodiscard]] Constants::Event lockOn(std::span<std::string const> const& args);
    static auto constexpr lockOnName = "input-mapping lock on";
    static auto constexpr lockOnDesc = "Locks an action until it is unlocked, preventing it from being triggered by any of its associated keys.\n"
        "Allows locking of entire categories of actions by using a structured action name, e.g. 'movement::up' or 'combat::primaryAttack' can be locked with 'movement' or 'combat'.\n"
        "Usage: input-mapping lock on <actionName>\n";

    [[nodiscard]] Constants::Event unlock(std::span<std::string const> const& args);
    static auto constexpr unlockName = "input-mapping lock off";
    static auto constexpr unlockDesc = "Unlocks an action, allowing it to be triggered by its associated keys again.\n"
        "Allows locking of entire categories of actions by using a structured action name, e.g. 'movement::up' or 'combat::primaryAttack' can be locked with 'movement' or 'combat'.\n"
        "Usage: input-mapping unlock <actionName>\n";

    // TODO: Modify/reload input mapping at runtime
    //       store mappings in settings scope!

    // TODO: showMappings <on/off> function that displays an imgui window with the current mappings
    //       later on, this may be expanded to a full input mapping editor for actual GUI configuration of input

    //------------------------------------------
    // Categories

    static auto constexpr inputMappingName = "input-mapping";
    static auto constexpr inputMappingDesc = "Functions for mapping inputs to actions within the GlobalSpace.";

    static auto constexpr inputMappingLockName = "input-mapping lock";
    static auto constexpr inputMappingLockDesc = "Functions for locking and unlocking input actions.";

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables. 
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Core::GlobalSpace, InputMapping){
        // Setup pointer to polled input key in Renderer::Input module, to sync our updates with it and avoid missing deltas
        sdlPolledInput = moduleScope.getStableDoublePointer(Renderer::Input::Key::routineActivated);

        // Load initial mappings from settings
        reloadMappings();

        // Bind functions
        bindCategory(inputMappingName, inputMappingDesc);
        bindCategory(inputMappingLockName, inputMappingLockDesc);
        bindFunction(&InputMapping::lockOnce, lockOnceName, lockOnceDesc);
        bindFunction(&InputMapping::lockOn, lockOnName, lockOnDesc);
        bindFunction(&InputMapping::unlock, unlockName, unlockDesc);
    }

    struct Key : Data::KeyGroup<""> {
        static auto constexpr mappingLocation = makeScoped("input.");
    };

    //------------------------------------------
    // Other public functions

    /**
     * @brief Loads default input mappings into a given scope.
     */
    static void loadDefaultMappings(Data::JsonScope& scope);

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
            onRelease,
            onChange // Any change in state, either press or release
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
        if (typeStr == "onChange") {
            return association::action::onChange;
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

        enum LockState {
            unlocked,   // The action is not locked and can be triggered by its associated keys.
            lockOnce,   // The action is locked for the current frame, preventing it from being triggered by any of its associated keys. It will be automatically unlocked in the next frame.
            lockOn      // The action is locked until it is manually unlocked, preventing it from being triggered by any of its associated keys.
        } lockState = unlocked; // The current lock state of the action
    };

    /**
     * @brief Maps input actions to their associated keys.
     */
    absl::flat_hash_map<std::string, mapEntry> mappings;

    /**
     * @brief Reloads all input mappings from the settings scope.
     */
    void reloadMappings();

    /**
     * @brief Processes all input mappings
     */
    void processMappings();

    /**
     * @brief Helper function to add a mapping to a given scope, used for loading default mappings and potentially for runtime mapping configuration in the future.
     * @param scope The scope to which the mapping should be added, e.g. settingsScope for default mappings or a temporary scope for runtime configuration.
     * @param action The name of the action to be mapped, e.g. "jump" or "movement::up".
     * @param slots An array of pairs representing the key associations for the action, where each pair consists of the SDL key name and the association type
     *              (e.g. {{"space", "onPress"}, {"w", "current"}, {"", "empty"}} for a jump action that can be triggered by pressing space or holding w).
     */
    static void addMappingToScope(Data::JsonScope& scope, std::string const& action, std::array<std::pair<std::string, std::string>,3> const& slots);
};
}   // namespace Nebulite::Module::Domain::GlobalSpace
