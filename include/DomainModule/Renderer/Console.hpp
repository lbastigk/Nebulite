/**
 * @file Console.hpp
 * @brief This file contains the DomainModule of the GlobalSpace to provide console capabilities.
 */

#ifndef NEBULITE_RRDM_CONSOLE_HPP
#define NEBULITE_RRDM_CONSOLE_HPP

//------------------------------------------
// Includes

// Standard Library
#include <queue>

// External
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

// Nebulite
#include "Constants/ErrorTypes.hpp"
#include "Constants/KeyNames.hpp"
#include "Interaction/Execution/DomainModule.hpp"
#include "Utility/TextInput.hpp"
#include "Utility/TimeKeeper.hpp"

//------------------------------------------
// Forward declarations
namespace Nebulite::Core {
class Renderer;
} // namespace Nebulite::Core

//------------------------------------------
namespace Nebulite::DomainModule::Renderer {
/**
 * @class Nebulite::DomainModule::Renderer::Console
 * @brief DomainModule for Console capabilities within the Renderer.
 * @todo: Add functions to open/close the console via commands.
 */
NEBULITE_DOMAINMODULE(Nebulite::Core::Renderer, Console) {
public:
    Constants::Error update() override;
    void reinit() override {}

    //------------------------------------------
    // Available Functions

    Constants::Error consoleOpen();
    static auto constexpr consoleOpen_name = "console open";
    static auto constexpr consoleOpen_desc = "Opens the console, allowing it to be rendered and receive input.\n"
        "\n"
        "Usage: console open\n";

    Constants::Error consoleClose();
    static auto constexpr consoleClose_name = "console close";
    static auto constexpr consoleClose_desc = "Closes the console, hiding it and preventing it from receiving input.\n"
        "\n"
        "Usage: console close\n";

    Constants::Error consoleZoom(int argc, char** argv);
    static auto constexpr consoleZoom_name = "console zoom";
    static auto constexpr consoleZoom_desc = "Reduces or increases the console font size.\n"
        "\n"
        "Usage: console zoom [in/out/+/-]\n"
        "- in  / + : Zooms in  (increases font size)\n"
        "- out / - : Zooms out (decreases font size)\n";

    Constants::Error consoleSetBackground(int argc, char** argv);
    static auto constexpr consoleSetBackground_name = "console set-background";
    static auto constexpr consoleSetBackground_desc = "Sets a background image for the console.\n"
        "\n"
        "Usage: console set-background <image_path>\n";

    // Autotype functions

    Constants::Error consoleAutotypeText(std::span<std::string const> const& args);
    static auto constexpr consoleAutotypeText_name = "console autotype text";
    static auto constexpr consoleAutotypeText_desc = "Adds a text input command into the autotype queue.\n"
        "\n"
        "Usage: console autotype text <text>\n";

    Constants::Error consoleAutotypeEnter();
    static auto constexpr consoleAutotypeEnter_name = "console autotype enter";
    static auto constexpr consoleAutotypeEnter_desc = "Puts an enter command into the autotype queue.\n"
        "\n"
        "Usage: console autotype enter\n";

    Constants::Error consoleAutotypeExecute();
    static auto constexpr consoleAutotypeExecute_name = "console autotype execute";
    static auto constexpr consoleAutotypeExecute_desc = "Executes all autotype commands in the queue.\n"
        "\n"
        "Usage: console autotype execute\n";

    Constants::Error consoleAutotypeWait(std::span<std::string const> const& args);
    static auto constexpr consoleAutotypeWait_name = "console autotype wait";
    static auto constexpr consoleAutotypeWait_desc = "Waits for a specified amount of ms before executing the next autotype command.\n"
        "\n"
        "Usage: console autotype wait <milliseconds>\n";

    Constants::Error consoleAutotypeClose();
    static auto constexpr consoleAutotypeClose_name = "console autotype close";
    static auto constexpr consoleAutotypeClose_desc = "Closes the console if the autotype wait counter reaches zero"
        "\n"
        "Usage: console autotype close\n";

    Constants::Error consoleAutotypeHistoryUp();
    static auto constexpr consoleAutotypeHistoryUp_name = "console autotype up";
    static auto constexpr consoleAutotypeHistoryUp_desc = "Adds a history up command into the autotype queue.\n"
        "\n"
        "Usage: console autotype up\n";

    Constants::Error consoleAutotypeHistoryDown();
    static auto constexpr consoleAutotypeHistoryDown_name = "console autotype down";
    static auto constexpr consoleAutotypeHistoryDown_desc = "Adds a history down command into the autotype queue.\n"
        "\n"
        "Usage: console autotype down\n";

    Constants::Error consoleAutotypeClear();
    static auto constexpr consoleAutotypeClear_name = "console autotype clear";
    static auto constexpr consoleAutotypeClear_desc = "Clears the autotype queue without executing the commands.\n"
        "Clears both the queue pending for execution and the active queue currently being executed.\n"
        "\n"
        "Usage: console autotype clear\n";


    //------------------------------------------
    // Category strings

    static auto constexpr console_name = "console";
    static auto constexpr console_desc = "Console commands and settings.\n"
        "\n"
        "Contains commands to manipulate the in-application console.\n";

    static auto constexpr consoleAutotype_name = "console autotype";
    static auto constexpr consoleAutotype_desc = "Utilities to automate typing in the console";

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables.
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Core::Renderer, Console) {
        // we cannot do much here, since renderer might not be initialized yet
        // so we do the actual initialization in update() when needed
        bindCategory(console_name, console_desc);
        BIND_FUNCTION(&Console::consoleOpen, consoleOpen_name, consoleOpen_desc);
        BIND_FUNCTION(&Console::consoleClose, consoleClose_name, consoleClose_desc);
        BIND_FUNCTION(&Console::consoleZoom, consoleZoom_name, consoleZoom_desc);
        BIND_FUNCTION(&Console::consoleSetBackground, consoleSetBackground_name, consoleSetBackground_desc);

        bindCategory(consoleAutotype_name, consoleAutotype_desc);
        BIND_FUNCTION(&Console::consoleAutotypeText, consoleAutotypeText_name, consoleAutotypeText_desc);
        BIND_FUNCTION(&Console::consoleAutotypeEnter, consoleAutotypeEnter_name, consoleAutotypeEnter_desc);
        BIND_FUNCTION(&Console::consoleAutotypeExecute, consoleAutotypeExecute_name, consoleAutotypeExecute_desc);
        BIND_FUNCTION(&Console::consoleAutotypeWait, consoleAutotypeWait_name, consoleAutotypeWait_desc);
        BIND_FUNCTION(&Console::consoleAutotypeClose, consoleAutotypeClose_name, consoleAutotypeClose_desc);
        BIND_FUNCTION(&Console::consoleAutotypeHistoryUp, consoleAutotypeHistoryUp_name, consoleAutotypeHistoryUp_desc);
        BIND_FUNCTION(&Console::consoleAutotypeHistoryDown, consoleAutotypeHistoryDown_name, consoleAutotypeHistoryDown_desc);
        BIND_FUNCTION(&Console::consoleAutotypeClear, consoleAutotypeClear_name, consoleAutotypeClear_desc);
    }

    struct Key : KeyGroup<"renderer."> {
        // No keys for now
    };

private:
    //------------------------------------------
    // Basic Configuration

    // Font to use
    std::string consoleFontPath = "Resources/Fonts/JetBrainsMono-Medium.ttf";

    struct ConsoleLayout {
        uint16_t MINIMUM_LINES = 8; // Minimum number of lines to show, including input line
        double paddingRatio = 0.25; // Padding between lines as a ratio of font size
        uint16_t FONT_MAX_SIZE = 24; // Maximum font size
        double heightRatio = 0.75; // Console takes 75% of the screen height
    } consoleLayout;

    /**
     * @struct Colors
     * @brief Struct to hold color definitions for the console.
     */
    struct Colors {
        SDL_Color background = {30, 30, 100, 150}; // Semi-transparent gray-blue
        SDL_Color input = {200, 200, 200, 255}; // Light gray
        SDL_Color highlight = {100, 100, 100, 255}; // Dark gray
        SDL_Color cerrStream = {255, 40, 40, 255}; // Light Red
        SDL_Color coutStream = {255, 255, 255, 255}; // White
    } color;

    //------------------------------------------
    // State-related variables and functions

    // Whether the console has been initialized
    bool initialized = false;

    // Flag to indicate if text alignment needs recalculation
    bool flag_recalculateTextAlignment = true;

    /**
     * @brief Scrolling offset for output lines
     * Used to scroll through previous console output.
     */
    int32_t outputScrollingOffset = 0;

    /**
     * @brief Initializes the console, setting up the font and other necessary components.
     */
    void init();

    //------------------------------------------
    // Other variables

    // y positions of each line, derived from console height
    std::vector<int32_t> line_y_positions;

    //------------------------------------------
    // Texture and Font related objects

    // Font for console text
    TTF_Font* consoleFont = nullptr;

    /**
     * @brief Rectangle defining the input text area.
     */
    SDL_Rect textInputRect = {0, 0, 0, 0};

    /**
     * @brief Texture for the background image.
     */
    SDL_Texture* backgroundImageTexture = nullptr;

    /**
     * @brief Rectangle defining the highlighted text area.
     */
    SDL_Rect textInputHighlightRect = {0, 0, 0, 0};

    /**
     * @brief Rectangle used for each output line.
     */
    SDL_Rect textOutputRect = {0, 0, 0, 0};

    // Texture for console rendering
    struct SDL_Texture_Wrapper {
        SDL_Rect rect = {0, 0, 0, 0};
        SDL_Texture* texture_ptr = nullptr;
    } consoleTexture;

    //------------------------------------------
    // Methods

    /**
     * @brief Renders the console to the screen.
     */
    void renderConsole();

    /**
     * @brief Populates vector line_y_positions with the y positions of each line,
     *        and sets font size accordingly.
     * @param rect_height The height of the console rectangle in pixels.
     * @return The calculated line height.
     * @todo Make sure the lines are aligned at the top instead of the bottom to reduce jitter when resizing.
     */
    uint16_t calculateTextAlignment(uint16_t const& rect_height);

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
    void processKeyDownEvent(SDL_KeyboardEvent const& key);

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
    void keyTriggerZoomIn() const;

    /**
     * @brief Zooms the console out, if ctrl is held.
     */
    void keyTriggerZoomOut() const;

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
    void drawBackground() const;

    /**
     * @brief Draws the input text.
     * @param lineHeight The height of each line in pixels.
     */
    void drawInput(uint16_t const& lineHeight);

    /**
     * @brief Draws the output lines.
     * @param maxLineLength The maximum length of a line before line breaking happens, in characters.
     * @todo Newline handling does not work if a line is split into more than two lines.
     */
    void drawOutput(uint16_t const& maxLineLength);

    //------------------------------------------
    // Mirrored from Renderer

    // Whether the console is currently active
    bool consoleMode = false;

    // Event for handling input
    std::vector<SDL_Event>* events = nullptr;

    //------------------------------------------
    // Renderer Pointer
    SDL_Renderer* renderer = nullptr;

    //------------------------------------------
    // Text input handling
    Utility::TextInput textInput;

    //------------------------------------------
    // Autotype handling

    /**
     * @brief Struct to manage the autotype system, which allows queuing up commands to be executed in the console with optional wait times between them.
     */
    struct AutoType {
        /**
         * @struct Command
         * @brief Represents a command for the autotype system
         * @details Commands cannot be executed immediately, so we store their representation in a queue instead.
         */
        struct Command {
            enum class Type {
                TEXT,
                ENTER,
                CLOSE,
                WAIT,
                HISTORY_UP,
                HISTORY_DOWN
            } type;
            std::string text; // Additional data for text or wait commands
        };

        // Stores all autotype commands before "autotype execute" is called
        std::queue<Command> queue;

        // Stores autotype commands that are currently being executed (after "autotype execute" is called, until all commands are executed)
        std::queue<Command> activeQueue;

        // Remaining wait time in ms before the next autotype command from autotypeActiveQueue can be executed.
        size_t waitTimeRemaining = 0;

        // Timer to track dt for autotype wait commands
        Utility::TimeKeeper waitTimer;
    }autoType;

    /**
     * @brief Processes the autotype command queue, executing commands one by one while respecting wait times.
     */
    void processAutotypeQueue();
};
} // namespace Nebulite::DomainModule::Renderer
#endif // NEBULITE_RRDM_CONSOLE_HPP
