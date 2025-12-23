/**
 * @file TextInput.hpp
 * @brief Header file for the TextInput class.
 */

#ifndef NEBULITE_UTILITY_TEXTINPUT_HPP
#define NEBULITE_UTILITY_TEXTINPUT_HPP

//------------------------------------------
// Includes

// Standard library
#include <deque>
#include <utility>
#include <vector>
#include <string>
#include <cstdint>

// Nebulite
#include "Utility/Time.hpp"

//------------------------------------------

namespace Nebulite::Utility {
/**
 * @class Nebulite::Utility::TextInput
 * @brief Helper class to handle text input.
 * 
 * Includes:
 * 
 * - Current input buffer
 * 
 * - Command history browsing
 * 
 * - Cursor movement
 * 
 * - Output log
 * 
 * - Submit handling
 */
class TextInput{
public:
    /**
     * @brief Constructor for TextInput.
     */
    TextInput();

    /**
     * @brief Submits the current input buffer as type INPUT.
     * @return The submitted command.
     */
    std::string submit();

    /**
     * @brief A line entry with metadata.
     */
    struct LineEntry{
        enum class LineType{
            INPUT,
            COUT,
            CERR
        };

        LineType type;
        std::string content;
        std::string timestamp;

        LineEntry(std::string cont, LineType const& t)
            : type(t), 
              content(std::move(cont)),
              timestamp(Time::TimeIso8601(Time::ISO8601Format::YYYY_MM_DD_HH_MM_SS, true))
              {}
    };

    /**
     * @brief Inserts a line into the text output.
     * @param line The line of text to insert.
     * @param type The type of submission.
     * Default is COUT.
     */
    void insertLine(std::string const& line, LineEntry::LineType const& type = LineEntry::LineType::COUT);

    /**
     * @brief Handles backspace input.
     */
    void backspace() const ;

    /**
     * @brief Navigates up the command history.
     */
    void history_up();

    /**
     * @brief Navigates down the command history.
     */
    void history_down();
    
    /**
     * @brief Moves the input cursor to the left
     */
    void moveCursorLeft();

    /**
     * @brief Moves the input cursor to the right
     */
    void moveCursorRight();

    /**
     * @brief Appends a character to the input buffer.
     * @param c The character array to append.
     */
    void append(char const* c) const ;

    /**
     * @brief Gets the current input buffer.
     * @return The current input buffer as a string pointer.
     */
    [[nodiscard]] std::string* getInputBuffer() const {
        return consoleInputBuffer;
    }

    /**
     * @brief Gets the queue of output lines.
     */
    std::deque<LineEntry>* getOutput(){
        return &consoleOutput;
    }

    /**
     * @brief Gets the cursor offset in the input buffer.
     * @return The cursor offset as an unsigned 16-bit integer.
     */
    [[nodiscard]] uint16_t getCursorOffset() const {
        return cursorOffset;
    }

private:
    /**
     * @brief Pointer to the current input buffer string.
     * The string that is being shown and modified.
     */
    std::string* consoleInputBuffer;

    /**
     * @brief Buffer for the command at index 0 in the history.
     * This is the command that is being written as unfinished input.
     * 
     * If selectedCommandIndex is 0, consoleInputBuffer points to this string.
     * If selectedCommandIndex is > 0, consoleInputBuffer points to an entry in commandHistory.
     */
    std::string commandIndexZeroBuffer;

    /**
     * @brief Output log of the console.
     */
    std::deque<LineEntry> consoleOutput;

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
     */
    std::vector<std::string> commandHistory;    // Dynamic, is modified

    /**
     * @brief Index of the currently selected command in history.
     * 0 means no selection, latest input.
     */
    size_t selectedCommandIndex = 0;

    /**
     * @brief Offset of the cursor in the input buffer.
     */
    uint16_t cursorOffset = 0;

};
} // namespace Nebulite::Utility 
#endif // NEBULITE_UTILITY_TEXTINPUT_HPP
