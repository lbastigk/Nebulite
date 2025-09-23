/**
 * @file TDM_Fill.hpp
 * 
 * This file contains the DomainModule of the Texture for fill functions.
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
 * @class Nebulite::DomainModule::Texture::Fill
 * @brief DomainModule for fill functions within the Texture.
 */
class Fill : public Nebulite::Interaction::Execution::DomainModule<Nebulite::Core::Texture> {
public:
    void update();

    //------------------------------------------
    // Available Functions

    /**
     * @brief Fills the texture with a specified color.
     * 
     * @param argc The argument count
     * @param argv The argument vector: "fill <color>" or "fill <R> <G> <B>"
     * @return Potential errors that occurred on command execution
     */
    Nebulite::Constants::Error fill(int argc, char* argv[]);

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes references to the domain and FuncTree, 
     * and binds functions to the FuncTree.
     */
    Fill(std::string moduleName, Nebulite::Core::Texture* domain, Nebulite::Interaction::Execution::FuncTree<Nebulite::Constants::Error>* funcTreePtr) 
    : DomainModule(moduleName, domain, funcTreePtr) {
        bindFunction(&Fill::fill, "fill", "Fill the texture with a color: fill <color> or fill R G B");
    }
};
}   // namespace Texture
}   // namespace DomainModule
}   // namespace Nebulite