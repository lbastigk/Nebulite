/**
 * @file GTE_InputMapping.h
 * @brief Provides input binding utilities for the Nebulite engine.
 *
 * This file contains a GlobalTree extension to handle input bindings.
 * Note that this file is a work in progress!
 *
 * @author Leo Bastigkeit
 * @date August 15, 2025
 */

#include "FuncTreeExpansionWrapper.h"

/**
 * @class GTE_InputMapping
 * @brief Handles input bindings for the Nebulite engine.
 *
 * The GTE_InputMapping class provides functionality to manage input bindings
 * for various input devices, including keyboard and mouse.
 */
class InputMapping : public Nebulite::FuncTreeExpansion::Wrapper<Nebulite::GlobalSpace,InputMapping> {
public:
    using Wrapper<Nebulite::GlobalSpace, InputMapping>::Wrapper;   // Templated constructor from Wrapper, call this->setupBindings()

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
    void update();

    //----------------------------------------
    // Available Functions

    Nebulite::ERROR_TYPE readMappingsFromFile(int argc, char* argv[]);

    // Useful if we wish to update mappings ingame
    Nebulite::ERROR_TYPE updateInputMappings(int argc, char* argv[]);

    Nebulite::ERROR_TYPE writeMappingsToFile(int argc, char* argv[]);

    //----------------------------------------
    // Binding Functions
    void setupBindings() {
        bindFunction(&InputMapping::readMappingsFromFile,"read-input-mappings-from-file", "Reads Input Mapping from inputs.jsonc file");
        bindFunction(&InputMapping::updateInputMappings,"update-input-mappings", "Updates current input mapping: <key> <slot> <input>");
        bindFunction(&InputMapping::writeMappingsToFile,"write-input-mappings-to-file", "Writes Input Mapping to inputs.jsonc file");
    }

private:
    // Inputs bindings should be parsed into a struct
    // accessible in global document via:
    // global.input.bound.<current/delta>.<action>
    //
    // action: burst, if either is true:
    // association key_1 = {"space" , delta}
    // association key_2 = {"left", current}
    // association key_3 = {"right", current}
    struct association{
        std::string key; // e.g. "space"
        enum class type {
            empty,
            current,
            onPress,
            onRelease
        } type;
    };
    struct mapEntry{
        association slot_1{"", association::type::empty}; // First key associated with the action
        association slot_2{"", association::type::empty}; // Second key associated with the action
        association slot_3{"", association::type::empty}; // Third key associated with the action
    };

    // Map of input actions to their key associations
    absl::flat_hash_map<std::string, mapEntry> mappings;

    /*
    then, we can loop through all mappings:

    for (const auto& [action, entry] : mappings) {
        // Process each mapping
        int current = 0;

        for(const auto& association : {entry.slot_1, entry.slot_2, entry.slot_3}) {
            switch (association.type) {
                case association::type::current:
                    current += global.get<int>("<locationForCurrent>." + association.key);
                    break;
                case association::type::onPress:
                    current += abs(global.get<int>("<locationForDelta>." + association.key)) == 1;
                    break;
                case association::type::onRelease:
                    current += global.get<int>("<locationForDelta>." + association.key) == -1;
                    break;
                case association::type::empty:
                    break;
            }
        }

        // Now we write the state into our mapping location
        global.set<int>("<locationForAction>." + action, current);
    }
    */
};
