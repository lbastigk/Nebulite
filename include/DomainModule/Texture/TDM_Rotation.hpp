/**
 * @file TDM_Rotation.hpp
 * 
 * This file contains the DomainModule of the Texture for rotation functions.
 * 
 * @todo Functionality is not given, errors regarding streaming access / target setting persist.
 * Prioritize later, once some more important features are overhauled.
 */

#pragma once

//------------------------------------------
// Includes

// Nebulite
#include "Constants/ErrorTypes.hpp"
#include "Interaction/Execution/DomainModule.hpp"

//------------------------------------------
// Forward declarations
namespace Nebulite{
    namespace Core{
        class Texture; // Forward declaration of domain class Texture
    }
}

//------------------------------------------
namespace Nebulite {
namespace DomainModule {
namespace Texture {
/**
 * @class Nebulite::DomainModule::Texture::Rotation
 * @brief DomainModule for rotation functions within the Texture.
 */
class Rotation : public Nebulite::Interaction::Execution::DomainModule<Nebulite::Core::Texture> {
public:
    void update();

    //------------------------------------------
    // Available Functions

    /**
     * @brief Evaluates an expression string and executes it
     * 
     * @param argc The argument count
     * @param argv The argument vector: rotation angle in degrees
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::ERROR_TYPE rotate(int argc, char* argv[]);

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes references to the domain and FuncTree, 
     * and binds functions to the FuncTree.
     */
    Rotation(std::string moduleName, Nebulite::Core::Texture* domain, Nebulite::Interaction::Execution::FuncTree<Nebulite::Constants::ERROR_TYPE>* funcTreePtr) 
    : DomainModule(moduleName, domain, funcTreePtr) {
        bindFunction(&Rotation::rotate, "rotate",   "Rotate the texture by a given angle: rotate <angle>");
    }
};
}   // namespace Texture
}   // namespace DomainModule
}   // namespace Nebulite