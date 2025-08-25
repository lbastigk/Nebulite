/**
 * @file RTE_Parenting.h
 * @brief Header file for the Parenting expansion of the RenderObject tree.
 * 
 * @note WORK IN PROGRESS, this expansion is in the ideas stage.
 */

#pragma once

#include "ErrorTypes.h"
#include "FuncTreeExpansionWrapper.h"

namespace Nebulite {
class RenderObject; // Forward declaration of Domain class RenderObject

namespace RenderObjectTreeExpansion {
class Parenting : public Nebulite::FuncTreeExpansion::Wrapper<Nebulite::RenderObject, Parenting> {
public:
    using Wrapper<Nebulite::RenderObject, Parenting>::Wrapper; // Templated constructor from Wrapper, call this->setupBindings()

    void update();

    //----------------------------------------
    // Available Functions

    /**
     * @brief Adds a child to the RenderObject
     * 
     * @param argc The argument count
     * @param argv The argument vector: ...
     * @return Potential errors that occured on command execution
     * 
     * @todo Not implemented yet
     */
    Nebulite::ERROR_TYPE addChildren(int argc, char* argv[]);

    /**
     * @brief Removes a child from the RenderObject
     * 
     * @param argc The argument count
     * @param argv The argument vector: ...
     * @return Potential errors that occured on command execution
     * 
     * @todo Not implemented yet
     */
    Nebulite::ERROR_TYPE removeChildren(int argc, char* argv[]);

    /**
     * @brief Removes all children from the RenderObject
     * 
     * @param argc The argument count
     * @param argv The argument vector: ...
     * @return Potential errors that occured on command execution
     * 
     * @todo Not implemented yet
     */
    Nebulite::ERROR_TYPE removeAllChildren(int argc, char* argv[]);


    //-------------------------------------------
    // Setup

    /**
     * @brief Sets up the functions bindings in the domains function tree
     * 
     * Is called automatically by the inherited Wrappers constructor.
     */
    void setupBindings() {
        bindFunction(&Parenting::addChildren, "add-children", "Adds children of the RenderObject by name");
        bindFunction(&Parenting::removeChildren, "remove-children", "Removes children from the RenderObject");
        bindFunction(&Parenting::removeAllChildren, "remove-all-children", "Removes all children from the RenderObject");
    }
};
}   // namespace RenderObjectTreeExpansion
}   // namespace Nebulite