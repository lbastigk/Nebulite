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
class SimpleDataNebulite::Interaction::Execution::DomainModule<Nebulite::Utility::JSON, SimpleData> {
public:
    using DomainModule<Nebulite::Utility::JSON, SimpleData>::DomainModule; // Templated constructor from Wrapper, call this->setupBindings()

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
    Nebulite::Constants::ERROR_TYPE set(int argc, char* argv[]);

    /**
     * @brief Move data from one key to another
     * 
     * @param argc The argument count
     * @param argv The argument vector: <source_key> <destination_key>
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::ERROR_TYPE move(int argc, char* argv[]);

    /**
     * @brief Copy data from one key to another
     * 
     * @param argc The argument count
     * @param argv The argument vector: <source_key> <destination_key>
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::ERROR_TYPE copy(int argc, char* argv[]);

    /**
     * @brief Delete a key from the JSON document
     * 
     * @param argc The argument count
     * @param argv The argument vector: <key>
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::ERROR_TYPE keyDelete(int argc, char* argv[]);

    /**
     * @brief Pushes a value to the back of an array
     * 
     * @param argc The argument count
     * @param argv The argument vector: <key> <value>
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::ERROR_TYPE push_back(int argc, char* argv[]);

    /**
     * @brief Pops a value from the back of an array
     * 
     * @param argc The argument count
     * @param argv The argument vector: <key>
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::ERROR_TYPE pop_back(int argc, char* argv[]);

    /**
     * @brief Pushes a value to the front of an array
     * 
     * @param argc The argument count
     * @param argv The argument vector: <key> <value>
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::ERROR_TYPE push_front(int argc, char* argv[]);

    /**
     * @brief Pops a value from the front of an array
     * 
     * @param argc The argument count
     * @param argv The argument vector: <key>
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::ERROR_TYPE pop_front(int argc, char* argv[]);

    /**
     * @brief Ensures that a key is an array, converting a value to an array if necessary
     * 
     * @param argc The argument count
     * @param argv The argument vector: <key>
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::ERROR_TYPE ensureArray(int argc, char* argv[]);


    //------------------------------------------
    // Setup

    /**
     * @brief Initializes references to the domain and FuncTree, 
     * and binds functions to the FuncTree.
     */
    Debug(Nebulite::Core::GlobalSpace* domain, Nebulite::Interaction::Execution::FuncTree<Nebulite::Constants::ERROR_TYPE>* funcTreePtr) 
    : DomainModule(domain, funcTreePtr) {
        // Bind functions specific to complex data handling
        bindFunction(&SimpleData::set, "set", "Set a key to a value in the JSON document: <key> <value>");

        // Internal move/copy
        bindFunction(&SimpleData::move, "move", "Move data from one key to another: <source_key> <destination_key>");
        bindFunction(&SimpleData::copy, "copy", "Copy data from one key to another: <source_key> <destination_key>");

        // Internal key deletion
        bindFunction(&SimpleData::keyDelete, "keyDelete", "Delete a key from the JSON document: <key>");

        // Array manipulation
        bindFunction(&SimpleData::push_back,   "push-back",    "Pushes a value to the back of an array:  <key> <value>");
        bindFunction(&SimpleData::pop_back,    "pop-back",     "Pops a value from the back of an array:  <key>");
        bindFunction(&SimpleData::push_front,  "push-front",   "Pushes a value to the front of an array: <key> <value>");
        bindFunction(&SimpleData::pop_front,   "pop-front",    "Pops a value from the front of an array: <key>");
        bindFunction(&SimpleData::ensureArray, "ensure-array", "Ensures that a key is an array, converting a value to an array if necessary: <key>");   // Note: name 'ensure-array' is hardcoded in some functions! Do not change!
    }
};
}   // namespace DomainModule
}   // namespace JSON
}   // namespace Nebulite