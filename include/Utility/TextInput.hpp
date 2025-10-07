/**
 * @file TextInput.hpp
 * @brief Header file for the TextInput class.
 */

 #pragma once

//------------------------------------------
// Includes

// General
#include <deque>
#include <vector>
#include <string>

// Nebulite
#include "Core/GlobalSpace.hpp"
#include "Nebulite.hpp"

//------------------------------------------

namespace Nebulite::Utility {
/**
 * @class Nebulite::Utility::TextInput
 * @brief Helper class to handle text input.
 * 
 * Includes:
 * - Current input buffer
 * - Command history
 * - Output log
 * 
 * @todo Add ability to move cursor within input line
 */
class TextInput{
private:
    // What the user is typing
    std::string* consoleInputBuffer;    // What is shown and modified
    std::string commandIndexZeroBuffer; // What is being written as unfinished input

    // Optional: Past output log
    /**
     * @todo Wrap each entry in a struct with extra metadata:
     * - type (input, cout, cerr)
     * - timestamp
     * - string
     */
    std::deque<std::string> consoleOutput;      // Static, is not modified

    /**
     * @brief History of past commands.
     * 
     * If the user presses UP/DOWN, they can cycle through this history.
     * If we write while browsing history, we modify that entry!
     * While the output still shows the original command,
     * the input buffer is modified to show the new command.
     * 
     * Example:
     * - User types "set var1 10" and presses Enter
     * - User presses UP, input buffer shows "set var1 10"
     * - User modifies it to "set var1 20"
     * - User presses Enter, command "set var1 20" is executed and added
     * - The output shows both commands, but the commandHistory shows "set var1 20" twice, as
     * we modified the first entry while browsing history.
     * 
     * @todo: Perhaps we could wrap each entry in a struct with metadata:
     * - original command (for output)
     * - modified command (for input buffer while browsing history)
     * - if we ever move up/down, we reset the modified command to the original command
     */
    std::vector<std::string> commandHistory;    // Dynamic, is modified

    // Currently selected command from history
    // 0 means no selection, latest input
    int selectedCommandIndex = 0;

public:
    /**
     * @brief Constructor for TextInput.
     */
    TextInput();

    /**
     * @brief Types of submission for commands.
     */
    enum class submitType{
        EXECUTE,
        HISTORY_ONLY
    };

    /**
     * @brief Submits the current input buffer as a command.
     * @param globalspace The GlobalSpace instance to submit the command to.
     * @param execute (Optional) Whether to execute the command or just add it to history and output. 
     * Default is true.
     */
    void submit(Nebulite::Core::GlobalSpace* globalspace, submitType type = submitType::EXECUTE);

    /**
     * @brief Handles backspace input.
     * @param console The console instance.
     */
    void backspace();

    /**
     * @brief Navigates up the command history.
     * @param console The console instance.
     */
    void history_up();

    /**
     * @brief Navigates down the command history.
     * @param console The console instance.
     */
    void history_down();

    /**
     * @brief Appends a character to the input buffer.
     * @param console The console instance.
     * @param c The character array to append.
     */
    void append(const char* c){
        consoleInputBuffer->append(c);
    }

    /**
     * @brief Gets the current input buffer.
     * @return The current input buffer as a string pointer.
     */
    std::string* getInputBuffer() const {
        return consoleInputBuffer;
    }

    /**
     * @brief Gets the queue of output lines.
     */
    std::deque<std::string>* getOutput() {
        return &consoleOutput;
    }
};
} // namespace Nebulite::Utility 