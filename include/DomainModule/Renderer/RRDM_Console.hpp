/**
 * @file GSDM_Console.hpp
 * 
 * This file contains the DomainModule of the GlobalSpace to provide console capabilities.
 */

#ifndef NEBULITE_DOMAINMODULE_RENDERER_RRDM_CONSOLE_HPP
#define NEBULITE_DOMAINMODULE_RENDERER_RRDM_CONSOLE_HPP

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
namespace Nebulite::DomainModule::Renderer {
/**
 * @class Nebulite::DomainModule::Renderer::Console
 * @brief DomainModule for Console capabilities within the Renderer.
 */
NEBULITE_DOMAINMODULE(Nebulite::Core::Renderer, Console){
public:
    /**
     * @brief Override of update.
     */
    Nebulite::Constants::Error update() override;

    //------------------------------------------
    // Available Functions

    /**
     * @brief Zooms the console in or out.
     * @param argc Number of arguments.
     * @param argv Argument values: [in/out/+/-], defaults to in/+.
     * @return Error code.
     */
    Nebulite::Constants::Error consoleZoom(int argc,  char* argv[]);
    static std::string const consoleZoom_name;
    static std::string const consoleZoom_desc;

    /**
     * @brief Sets a background image for the console.
     * @param argc Number of arguments.
     * @param argv Argument values: [image_path].
     * @return Error code.
     */
    Nebulite::Constants::Error consoleSetBackground(int argc,  char* argv[]);
    static std::string const consoleSetBackground_name;
    static std::string const consoleSetBackground_desc;

    //------------------------------------------
    // Category strings

    static std::string const console_name;
    static std::string const console_desc;

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables. 
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Core::Renderer, Console){
        // we cannot do much here, since renderer might not be initialized yet
        // so we do the actual initialization in update() when needed
        bindCategory(console_name, &console_desc);
        bindFunction(&Console::consoleZoom,             consoleZoom_name,           &consoleZoom_desc);
        bindFunction(&Console::consoleSetBackground,    consoleSetBackground_name,  &consoleSetBackground_desc);
    }
private: 
    //------------------------------------------
    // Configuration

    // Key to toggle console
    std::string toggleKey = "input.keyboard.delta.tab";

    uint8_t MINIMUM_LINES = 8;      // Minimum number of lines to show, including input line
    uint8_t LINE_PADDING = 10;      // Padding between lines in pixels
    uint8_t FONT_MAX_SIZE = 24;     // Maximum font size

    // y positions of each line, derived from console height
    std::vector<uint16_t> line_y_positions;
    
    //------------------------------------------
    // State

    // Whether the console has been initialized
    bool initialized = false;

    // Flag to indicate if text alignment needs recalculation
    bool flag_recalculateTextAlignment = true;

    // Scrolling offset for output lines
    uint16_t outputScrollingOffset = 0;

    /**
     * @brief Initializes the console, setting up the font and other necessary components.
     */
    void init();

    //------------------------------------------
    // Texture and Font related

    // Font for console text
	TTF_Font* consoleFont = nullptr;

    /**
     * @brief Rectangle defining the input text area.
     */
	SDL_Rect textInputRect;

    /**
     * @brief Texture for the background image.
     */
    SDL_Texture* backgroundImageTexture = nullptr;

    /**
     * @brief Rectangle defining the highlighted text area.
     */
    SDL_Rect textInputHighlightRect;

    /**
     * @brief Rectangle used for each output line.
     */
    SDL_Rect textOutputRect;

    /**
     * @struct Colors
     * @brief Struct to hold color definitions for the console.
     */
    struct Colors{
        SDL_Color background = { 30,  30, 100, 150};    // Semi-transparent gray-blue
        SDL_Color input      = {200, 200, 200, 255};    // Light gray
        SDL_Color highlight  = {100, 100, 100, 255};    // Dark gray
        SDL_Color cerrStream = {255,  40,  40, 255};    // Light Red
        SDL_Color coutStream = {255, 255, 255, 255};    // White
    }color;

    // Texture for console rendering
    struct SDL_Texture_Wrapper{
        SDL_Rect rect;
        SDL_Texture* texture_ptr = nullptr;
    }consoleTexture;

    // Scale of the window
    unsigned int WindowScale = 1;

    //------------------------------------------
    // Methods

    /**
	 * @brief Renders the console to the screen.
	 */
	void renderConsole();

    /**
     * @brief Populates vector line_y_positions with the y positions of each line,
     * and sets font size accordingly.
     * 
     * @todo Make sure the lines are aligned at the top instead of the bottom to reduce jitter when resizing.
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
    // Processing helpers

    /**
     * @brief Processes a keydown event.
     */
    void processKeyDownEvent(const SDL_KeyboardEvent& key);

    /**
     * @brief Submits the current command in the text input.
     */
    void keyTriggerSubmit();

    /**
     * @brief Scrolls the output up by one line.
     */
    void keyTriggerScrollUp();

    /**
     * @brief Scrolls the output down by one line.
     */
    void keyTriggerScrollDown();

    /**
     * @brief Zooms the console in, if ctrl is held.
     */
    void keyTriggerZoomIn(const SDL_KeyboardEvent& key);

    /**
     * @brief Zooms the console out, if ctrl is held.
     */
    void keyTriggerZoomOut(const SDL_KeyboardEvent& key);

    //------------------------------------------
    // RenderConsole helpers

    /**
     * @brief Ensures the console texture is created and valid.
     * @return true if the created console texture is valid, false if creation failed.
     */
    bool ensureConsoleTexture();

    /**
     * @brief Draws the background of the console.
     */
    void drawBackground();

    /**
     * @brief Draws the input text.
     * 
     * @param lineHeight The height of each line in pixels.
     */
    void drawInput(uint16_t lineHeight);

    /**
     * @brief Draws the output lines.
     * 
     * @param maxLineLength The maximum length of a line before linebreaking, in characters.
     * 
     * @todo Newline handling does not work if a line is split into more than two lines.
     */
    void drawOutput(uint16_t maxLineLength);

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
}   // namespace Nebulite::DomainModule::GlobalSpace
#endif // NEBULITE_DOMAINMODULE_RENDERER_RRDM_CONSOLE_HPP