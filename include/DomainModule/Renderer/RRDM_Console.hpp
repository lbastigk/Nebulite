/**
 * @file GSDM_Console.hpp
 * 
 * This file contains the DomainModule of the GlobalSpace to provide console capabilities.
 */

#pragma once

//------------------------------------------
// Includes

// External
#include <SDL.h>
#include <SDL_ttf.h>

// Nebulite
#include "Constants/ErrorTypes.hpp"
#include "Interaction/Execution/DomainModule.hpp"
#include "Utility/JSON.hpp"
#include "Utility/TextInput.hpp"

//------------------------------------------
// Forward declarations
namespace Nebulite{
    namespace Core{
        class Renderer; // Forward declaration of domain class GlobalSpace
    }
}

//------------------------------------------
namespace Nebulite {
namespace DomainModule {
namespace Renderer {
/**
 * @class Nebulite::DomainModule::Renderer::Console
 * @brief DomainModule for Console capabilities within the Renderer.
 */
NEBULITE_DOMAINMODULE(Nebulite::Core::Renderer, Console) {
public:
    /**
     * @brief Override of update.
     */
    Nebulite::Constants::Error update() override;

    //------------------------------------------
    // Available Functions

    // None, only update to check for console toggle and render to screen

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables. 
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Core::Renderer, Console){
        // we cannot do much here, since renderer might not be initialized yet
        // so we do the actual initialization in update() when needed
    }
private: 
    //------------------------------------------
    // Configuration

    // Key to toggle console
    std::string toggleKey = "input.keyboard.delta.tab";

    uint8_t MINIMUM_LINES = 5; // Minimum number of lines to show, including input line
    uint8_t LINE_PADDING = 10; // Padding between lines in pixels
    uint8_t FONT_MAX_SIZE = 24; // Maximum font size

    // y positions of each line, derived from console height
    std::vector<uint16_t> line_y_pos;
    
    //------------------------------------------
    // State

    // Whether the console has been initialized
    bool initialized = false;

    /**
     * @brief Initializes the console, setting up the font and other necessary components.
     */
    void init();

    //------------------------------------------
    // Texture and Font related

    // Font for console text
	TTF_Font* consoleFont;

    //SDL_Rect rect;
	SDL_Rect textRect;

    // Texture for console rendering
    struct SDL_Texture_Wrapper{
        SDL_Rect rect;
        SDL_Texture* texture_ptr = nullptr;
    }consoleTexture;

    //------------------------------------------
    // Methods

    /**
	 * @brief Renders the console to the screen.
     * 
     * @todo Improve rendering by always rendering a fixed number of lines,
     * or less if size starts to become an issue.
     * Setting size and spacing automatically based on console height.
	 */
	void renderConsole();

    /**
     * @brief Populates vector line_y_pos with the y positions of each line,
     * and sets font size accordingly.
     * 
     * @return The calculated line height.
     */
    uint8_t calculateTextAlignment(uint16_t rect_height);

    /**
     * @brief Processes input events for the console.
     */
    void processEvents();

    /**
     * @brief Processes the current mode of the console.
     */
    void processMode();

    //------------------------------------------
    // Mirrored from Renderer

    // Whether the console is currently active
    bool consoleMode = false; 

    // Event for handling input
    std::vector<SDL_Event>* events = nullptr;

    //------------------------------------------
    // References
    SDL_Renderer* renderer = nullptr;
    Nebulite::Utility::JSON* globalDoc = nullptr;

    //------------------------------------------
    // Text input handling
    Nebulite::Utility::TextInput textInput;
};
}   // namespace GlobalSpace
}   // namespace DomainModule
}   // namespace Nebulite