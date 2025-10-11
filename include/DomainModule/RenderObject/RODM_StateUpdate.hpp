/**
 * @file RODM_StateUpdate.hpp
 * 
 * Contains the RenderObject DomainModule for updating the state of render objects.
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
    namespace Core{
        class RenderObject; // Forward declaration of domain class RenderObject
    }
}

//------------------------------------------
namespace Nebulite{
namespace DomainModule{
namespace RenderObject{
/**
 * @class Nebulite::DomainModule::RenderObject::StateUpdate
 * @brief State update DomainModule of the RenderObject tree.
 */
NEBULITE_DOMAINMODULE(Nebulite::Core::RenderObject, StateUpdate) {
public:
    /**
     * @brief Override of update.
     */
    Nebulite::Constants::Error update() override;

    //------------------------------------------
    // Available Functions

    /**
     * @brief Marks the object for deletion.
     * 
     * @param argc The argument count
     * @param argv The argument vector: no arguments available
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error func_delete(int argc, char* argv[]);
    static const std::string func_delete_name;
    static const std::string func_delete_desc;

    /**
     * @brief Updates the text texture of the object.
     * 
     * @param argc The argument count
     * @param argv The argument vector: no arguments available
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error updateText(int argc, char* argv[]);
    static const std::string updateText_name;
    static const std::string updateText_desc;

    //------------------------------------------
    // TODOs

    /**
     * @brief Reloads all invokes for the object.
     * 
     * @param argc The argument count
     * @param argv The argument vector
     * @return Potential errors that occured on command execution
     * 
     * @todo Not implemented yet
     */
    Nebulite::Constants::Error invoke_reload(int argc, char* argv[]);
    static const std::string invoke_reload_name;
    static const std::string invoke_reload_desc;

    /**
     * @brief Adds an invoke for the object.
     * 
     * @param argc The argument count
     * @param argv The argument vector
     * @return Potential errors that occured on command execution
     * 
     * @todo Not implemented yet
     */
    Nebulite::Constants::Error invoke_add(int argc, char* argv[]);
    static const std::string invoke_add_name;
    static const std::string invoke_add_desc;

    /**
     * @brief Removes an invoke from the object.
     * 
     * @param argc The argument count
     * @param argv The argument vector
     * @return Potential errors that occured on command execution
     * 
     * @todo Not implemented yet
     */
    Nebulite::Constants::Error invoke_remove(int argc, char* argv[]);
    static const std::string invoke_remove_name;
    static const std::string invoke_remove_desc;

    /**
     * @brief Removes all invokes from the object.
     * 
     * @param argc The argument count
     * @param argv The argument vector
     * @return Potential errors that occured on command execution
     * 
     * @todo Not implemented yet
     */
    Nebulite::Constants::Error invoke_purge(int argc, char* argv[]);
    static const std::string invoke_purge_name;
    static const std::string invoke_purge_desc;

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables. 
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Core::RenderObject, StateUpdate) {
        bindFunction(&StateUpdate::func_delete,         func_delete_name,         &func_delete_desc);
        bindFunction(&StateUpdate::updateText,          updateText_name,          &updateText_desc);

        // TODO: Move to separate Invoke module
        /*
        bindSubtree("invoke", "Functions for managing invokes");
        bindFunction(&StateUpdate::invoke_reload,       "reload-invokes",       "Reload all invokes");
        bindFunction(&StateUpdate::invoke_add,          "add-invoke",           "Add an invoke from a file");
        bindFunction(&StateUpdate::invoke_remove,       "remove-invoke",        "Remove an invoke by name");
        bindFunction(&StateUpdate::invoke_purge,        "remove-all-invokes",   "Remove all invokes");
        */
    }
};
}   // namespace DomainModule
}   // namespace RenderObject
}   // namespace Nebulite