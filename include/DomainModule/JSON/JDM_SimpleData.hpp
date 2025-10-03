/**
 * @file JDM_SimpleData.hpp
 * @brief DomainModule for simple data operations on domain class Nebulite::Utility::JSON
 */

#pragma once

//------------------------------------------
// Includes

// General
#include "Constants/ErrorTypes.hpp"
#include "Interaction/Execution/DomainModule.hpp"

//------------------------------------------
// Forward declarations
namespace Nebulite{
    namespace Utility{
        class JSON; // Forward declaration of domain class JSON
    }
}

//------------------------------------------
namespace Nebulite{
namespace DomainModule{
namespace JSON{
/**
 * @class Nebulite::DomainModule::JSON::SimpleData
 * @brief DomainModule for simple data operations on domain class Nebulite::Utility::JSON
 */
NEBULITE_DOMAINMODULE(Nebulite::Utility::JSON, SimpleData) {
public:
    /**
     * @brief Overwridden update function.
     */
    void update();

    //------------------------------------------
    // Available Functions

    /**
     * @brief Set a key to a value in the JSON document
     * 
     * @param argc The argument count
     * @param argv The argument vector: <key> <value>
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error set(int argc, char* argv[]);
    std::string set_desc = R"(Set a key to a value in the JSON document.

    Usage: set <key> <value>

    Note: All values are stored as strings.
    )";

    /**
     * @brief Move data from one key to another
     * 
     * @param argc The argument count
     * @param argv The argument vector: <source_key> <destination_key>
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error move(int argc, char* argv[]);
    std::string move_desc = R"(Move data from one key to another.

    Usage: move <source_key> <destination_key>
    )";

    /**
     * @brief Copy data from one key to another
     * 
     * @param argc The argument count
     * @param argv The argument vector: <source_key> <destination_key>
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error copy(int argc, char* argv[]);
    std::string copy_desc = R"(Copy data from one key to another.

    Usage: copy <source_key> <destination_key>
    )";

    /**
     * @brief Delete a key from the JSON document
     * 
     * @param argc The argument count
     * @param argv The argument vector: <key>
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error keyDelete(int argc, char* argv[]);
    std::string keyDelete_desc = R"(Delete a key from the JSON document.

    Usage: keyDelete <key>
    )";

    /**
     * @brief Pushes a value to the back of an array
     * 
     * @param argc The argument count
     * @param argv The argument vector: <key> <value>
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error push_back(int argc, char* argv[]);
    std::string push_back_desc = R"(Push a value to the back of an array.

    Usage: push-back <key> <value>
    )";

    /**
     * @brief Pops a value from the back of an array
     * 
     * @param argc The argument count
     * @param argv The argument vector: <key>
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error pop_back(int argc, char* argv[]);
    std::string pop_back_desc = R"(Pop a value from the back of an array.

    Usage: pop-back <key>
    )";

    /**
     * @brief Pushes a value to the front of an array
     * 
     * @param argc The argument count
     * @param argv The argument vector: <key> <value>
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error push_front(int argc, char* argv[]);
    std::string push_front_desc = R"(Push a value to the front of an array.

    Usage: push-front <key> <value>
    )";

    /**
     * @brief Pops a value from the front of an array
     * 
     * @param argc The argument count
     * @param argv The argument vector: <key>
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error pop_front(int argc, char* argv[]);
    std::string pop_front_desc = R"(Pop a value from the front of an array.

    Usage: pop-front <key>
    )";

    /**
     * @brief Ensures that a key is an array, converting a value to an array if necessary
     * 
     * @param argc The argument count
     * @param argv The argument vector: <key>
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error ensureArray(int argc, char* argv[]);
    std::string ensureArray_desc = R"(Ensure that a key is an array, converting a value to an array if necessary.

    Usage: ensure-array <key>
    )";

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables. 
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Utility::JSON, SimpleData){
        // Bind functions specific to complex data handling
        bindFunction(&SimpleData::set, "set", set_desc);

        // Internal move/copy
        bindFunction(&SimpleData::move, "move", move_desc);
        bindFunction(&SimpleData::copy, "copy", copy_desc);

        // Internal key deletion
        bindFunction(&SimpleData::keyDelete, "keyDelete", keyDelete_desc);

        // Array manipulation
        bindFunction(&SimpleData::push_back,   "push-back",    push_back_desc);
        bindFunction(&SimpleData::pop_back,    "pop-back",     pop_back_desc);
        bindFunction(&SimpleData::push_front,  "push-front",   push_front_desc);
        bindFunction(&SimpleData::pop_front,   "pop-front",    pop_front_desc);
        bindFunction(&SimpleData::ensureArray, "ensure-array", ensureArray_desc);   // Note: name 'ensure-array' is hardcoded in some functions! Do not change!
    }
};
}   // namespace DomainModule
}   // namespace JSON
}   // namespace Nebulite