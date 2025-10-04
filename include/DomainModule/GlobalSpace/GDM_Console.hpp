/**
 * @file GDM_Console.hpp
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

//------------------------------------------
// Forward declarations
namespace Nebulite{
    namespace Core{
        class GlobalSpace; // Forward declaration of domain class GlobalSpace
    }
}

//------------------------------------------
namespace Nebulite {
namespace DomainModule {
namespace GlobalSpace {
/**
 * @class Nebulite::DomainModule::GlobalSpace::Console
 * @brief DomainModule for Consoleging capabilities within the GlobalSpace.
 */
NEBULITE_DOMAINMODULE(Nebulite::Core::GlobalSpace, Console) {
public:
    /**
     * @brief Override of update.
     */
    void update();

    //------------------------------------------
    // Available Functions

    // None, only update to check for console toggle and render to screen

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables. 
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Core::GlobalSpace, Console){
        // we cannot do much here, since renderer might not be initialized yet
        // so we do the actual initialization in update() when needed
        consoleInputBuffer = &commandIndexZeroBuffer;
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
    // IO Buffers

    // What the user is typing
    std::string* consoleInputBuffer;    // What is shown and modified
    std::string commandIndexZeroBuffer; // What is being written as unfinished input
    
    // Optional: Past output log
	std::deque<std::string> consoleOutput;      // Static, is not modified
    std::vector<std::string> commandHistory;    // Dynamic, is modified
    
    // Currently selected command from history
    // 0 means no selection, latest input
    int selectedCommandIndex = 0;

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

    //------------------------------------------
    // Mirrored from Renderer

    // Whether the console is currently active
    bool consoleMode = false; 

    // Event for handling input
    std::vector<SDL_Event>* events = nullptr;

    //------------------------------------------
    // References
    SDL_Renderer* renderer = nullptr;
    Nebulite::Interaction::Invoke* invoke = nullptr;
    Nebulite::Utility::JSON* globalDoc = nullptr;

    //------------------------------------------
    // Text input handling
    /**
     * @class Nebulite::DomainModule::GlobalSpace::Console::TextInput
     * @brief Helper class to handle text input in the console.
     */
    class TextInput{
    public:
        /**
         * @brief Submits the current input buffer as a command.
         * @param console The console instance.
         * @param execute (Optional) Whether to execute the command or just add it to history and output. 
         * Default is true.
         */
        static void submit(Console *console, bool execute = true);

        /**
         * @brief Handles backspace input.
         * @param console The console instance.
         */
        static void backspace(Console *console);

        /**
         * @brief Navigates up the command history.
         * @param console The console instance.
         */
        static void history_up(Console *console);

        /**
         * @brief Navigates down the command history.
         * @param console The console instance.
         */
        static void history_down(Console *console);
    };
    friend class TextInput;
};
}   // namespace GlobalSpace
}   // namespace DomainModule
}   // namespace Nebulite