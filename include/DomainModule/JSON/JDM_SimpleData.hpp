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
     * @brief Override of update.
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
    static const std::string set_name;
    static const std::string set_desc;

    /**
     * @brief Move data from one key to another
     * 
     * @param argc The argument count
     * @param argv The argument vector: <source_key> <destination_key>
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error move(int argc, char* argv[]);
    static const std::string move_name;
    static const std::string move_desc;

    /**
     * @brief Copy data from one key to another
     * 
     * @param argc The argument count
     * @param argv The argument vector: <source_key> <destination_key>
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error copy(int argc, char* argv[]);
    static const std::string copy_name;
    static const std::string copy_desc;

    /**
     * @brief Delete a key from the JSON document
     * 
     * @param argc The argument count
     * @param argv The argument vector: <key>
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error keyDelete(int argc, char* argv[]);
    static const std::string keyDelete_name;
    static const std::string keyDelete_desc;

    /**
     * @brief Ensures that a key is an array, converting a value to an array if necessary
     * 
     * @param argc The argument count
     * @param argv The argument vector: <key>
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error ensureArray(int argc, char* argv[]);
    static const std::string ensureArray_name;
    static const std::string ensureArray_desc;

    /**
     * @brief Pushes a value to the back of an array
     * 
     * @param argc The argument count
     * @param argv The argument vector: <key> <value>
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error push_back(int argc, char* argv[]);
    static const std::string push_back_name;
    static const std::string push_back_desc;

    /**
     * @brief Pops a value from the back of an array
     * 
     * @param argc The argument count
     * @param argv The argument vector: <key>
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error pop_back(int argc, char* argv[]);
    static const std::string pop_back_name;
    static const std::string pop_back_desc;

    /**
     * @brief Pushes a value to the front of an array
     * 
     * @param argc The argument count
     * @param argv The argument vector: <key> <value>
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error push_front(int argc, char* argv[]);
    static const std::string push_front_name;
    static const std::string push_front_desc;

    /**
     * @brief Pops a value from the front of an array
     * 
     * @param argc The argument count
     * @param argv The argument vector: <key>
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error pop_front(int argc, char* argv[]);
    static const std::string pop_front_name;
    static const std::string pop_front_desc;

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables. 
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Utility::JSON, SimpleData){
        // Bind functions specific to complex data handling
        bindFunction(&SimpleData::set,          set_name,           &set_desc);

        // Internal move/copy
        bindFunction(&SimpleData::move,         move_name,          &move_desc);
        bindFunction(&SimpleData::copy,         copy_name,          &copy_desc);

        // Internal key deletion
        bindFunction(&SimpleData::keyDelete,    keyDelete_name,     &keyDelete_desc);

        // Array manipulation
        bindFunction(&SimpleData::push_back,    push_back_name,     &push_back_desc);
        bindFunction(&SimpleData::pop_back,     pop_back_name,      &pop_back_desc);
        bindFunction(&SimpleData::push_front,   push_front_name,    &push_front_desc);
        bindFunction(&SimpleData::pop_front,    pop_front_name,     &pop_front_desc);
        bindFunction(&SimpleData::ensureArray,  ensureArray_name,   &ensureArray_desc);
    }
};
}   // namespace DomainModule
}   // namespace JSON
}   // namespace Nebulite