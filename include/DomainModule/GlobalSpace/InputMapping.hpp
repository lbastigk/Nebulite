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
#include "Utility/TimedRoutine.hpp"

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
     * @brief Updates the input bindings.
     *
     * This function is called to update the state of the input bindings,
     * processing any new input events and updating the binding states.
     * 
     * 1.) Process keyboard input from SDL Renderer
     * 2.) Turn into cross-platform naming
     * 3.) Use keymapping to write state into globalspace
     * 4.) Write deltas into globalspace
     * 5.) Stores last pressed delta, which might become handy if we need to look up the last input state
     *
     * Should use a TimeKeeper so it only updates inputs every n milliseconds
     */
    Constants::Error update() override;

    //------------------------------------------
    // Available Functions

    // TODO: Modify/reload input mapping at runtime
    //       store mappings in settings scope!

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables. 
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Core::GlobalSpace, InputMapping){
        auto const sdlPolledInputKey = moduleScope.getRootScope() + "renderer.input.polled";
        sdlPolledInput = moduleScope.getStableDoublePointer(sdlPolledInputKey);

        // Example mappings:

        mappings["jump"] = mapEntry{
            .slotA = association{"space", association::type::current},
            .slotB = association{"", association::type::empty},
            .slotC = association{"", association::type::empty}
        };
        mappings["up"] = mapEntry{
            .slotA = association{"w", association::type::current},
            .slotB = association{"up", association::type::current},
            .slotC = association{"", association::type::empty}
        };
        mappings["down"] = mapEntry{
            .slotA = association{"s", association::type::current},
            .slotB = association{"down", association::type::current},
            .slotC = association{"", association::type::empty}
        };
        mappings["left"] = mapEntry{
            .slotA = association{"a", association::type::current},
            .slotB = association{"left", association::type::current},
            .slotC = association{"", association::type::empty}
        };
        mappings["right"] = mapEntry{
            .slotA = association{"d", association::type::current},
            .slotB = association{"right", association::type::current},
            .slotC = association{"", association::type::empty}
        };
    }

    struct Key {
        DECLARE_SCOPE("")
        static auto constexpr mappingLocation = MAKE_SCOPED("input.");
    };

private:
    double* sdlPolledInput = nullptr; // Key for checking if the Renderer::Input module has polled new input, to sync our updates with it and avoid missing deltas

    /**
     * @brief Represents a key association for input mapping.
     * 
     * The struct represents the Association between a key and its input type.
     */
    struct association{
        std::string key; // e.g. "space"
        enum class type {
            empty,
            current,
            onPress,
            onRelease
        } type = type::empty;
    };

    /**
     * @brief Represents a mapping entry for input actions.
     * @details Any input action can be associated with up to three keys.
     */
    struct mapEntry{
        association slotA{"", association::type::empty}; // First key associated with the action
        association slotB{"", association::type::empty}; // Second key associated with the action
        association slotC{"", association::type::empty}; // Third key associated with the action
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
    //void reloadMappings();

    /**
     * @brief Processes all input mappings
     */
    void processMappings();

    /**
     * JSON structure for input mapping:
     *
     * global.settings.inputMapping: <object>:
     * key is the name of the action, e.g. "jump"
     * value is an object with:
     * - slotA: <object>:
     *   - key: <string>: the key associated with the action, e.g. "space"
     *   - type: <string>: the type of association, one of "current", "onPress", "onRelease", "empty"
     * - slotB: <object>:
     *   - key: <string>: the key associated with the action, e.g. "w"
     *   - type: <string>: the type of association, one of "current", "onPress", "onRelease", "empty"
     * - slotC: <object>:
     *   - key: <string>: the key associated with the action, e.g. "up"
     *   - type: <string>: the type of association, one of "current", "onPress", "onRelease", "empty"
     */

    std::unique_ptr<Utility::TimedRoutine> inputMapperTimer = nullptr;
};
}   // namespace Nebulite::DomainModule::GlobalSpace
