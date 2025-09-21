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
class Console : public Nebulite::Interaction::Execution::DomainModule<Nebulite::Core::GlobalSpace> {
public:
    /**
     * @brief Overridden update function.
     */
    void update();

    //------------------------------------------
    // Available Functions

    // None, only update to check for console toggle and render to screen

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes references to the domain and FuncTree, 
     * and binds functions to the FuncTree.
     */
    Console(std::string moduleName, Nebulite::Core::GlobalSpace* domain, Nebulite::Interaction::Execution::FuncTree<Nebulite::Constants::ERROR_TYPE>* funcTreePtr);

private: 
    //------------------------------------------
    // Configuration

    // Key to toggle console
    std::string toggleKey = "input.keyboard.delta.tab";

    //------------------------------------------
    // State

    // Whether the console has been initialized
    bool initialized = false;

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
	 */
	void renderConsole();

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
         */
        static void submit(Console *console);

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