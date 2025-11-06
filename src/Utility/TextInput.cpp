#include "Utility/TextInput.hpp"

namespace Nebulite::Utility {

TextInput::TextInput(){
    consoleInputBuffer = &commandIndexZeroBuffer;
}

std::string TextInput::submit(){
    if (!consoleInputBuffer->empty()){
        std::string input = *consoleInputBuffer;
        insertLine(input, LineEntry::LineType::INPUT);

        // Like in typical consoles, we clear the output
        commandIndexZeroBuffer.clear();
        consoleInputBuffer = &commandIndexZeroBuffer;

        // Reset cursor
        cursorOffset = 0;

        // Reset history navigation
        selectedCommandIndex = 0;

        return input;
    }
    // Use last command if input is empty
    if(!commandHistory.empty()){
        std::string lastCommand = commandHistory.back();
        insertLine(lastCommand, LineEntry::LineType::INPUT);

        // Reset history navigation
        selectedCommandIndex = 0;

        return lastCommand;
    }
    // Nothing to submit
    return "";
}

void TextInput::insertLine(std::string const& line, LineEntry::LineType const& type){
    switch(type){
        case LineEntry::LineType::COUT: {
            LineEntry entry(line, LineEntry::LineType::COUT);
            consoleOutput.emplace_back(entry);
            break;
        }
        case LineEntry::LineType::CERR: {
            LineEntry entry(line, LineEntry::LineType::CERR);
            consoleOutput.emplace_back(entry);
            break;
        }
        case LineEntry::LineType::INPUT: {
            LineEntry entry(line, LineEntry::LineType::INPUT);
            commandHistory.emplace_back(line);
            consoleOutput.emplace_back(entry);
            break;
        }
    }
}

void TextInput::backspace() const {
    // We can only backspace if the buffer is not empty
    if (!consoleInputBuffer->empty()){
        // We can only backspace if the cursor is not at the start
        if(cursorOffset != consoleInputBuffer->size()){
            if(cursorOffset > 0){
                // Remove character before cursor
                std::string const beforeCursor = consoleInputBuffer->substr(0, consoleInputBuffer->size() - cursorOffset - 1);
                std::string const afterCursor = consoleInputBuffer->substr(consoleInputBuffer->size() - cursorOffset);
                *consoleInputBuffer = beforeCursor + afterCursor;
            }
            else{
                // Remove last character
                consoleInputBuffer->pop_back();
            }
        }
    }
}

void TextInput::history_up(){
    selectedCommandIndex++;
    
    // Get command from history
    if(selectedCommandIndex > commandHistory.size()){
        selectedCommandIndex = commandHistory.size();
    }
    if(selectedCommandIndex > 0){
        consoleInputBuffer = &commandHistory[commandHistory.size() - selectedCommandIndex];
    }
}

void TextInput::history_down(){
    if(selectedCommandIndex > 0) selectedCommandIndex--;
    
    // Get command from buffer or history
    if(selectedCommandIndex == 0){
        consoleInputBuffer = &commandIndexZeroBuffer;
    }
    else{
        consoleInputBuffer = &commandHistory[commandHistory.size() - selectedCommandIndex];
    }
}

void TextInput::moveCursorLeft(){
    // Move cursor left if it's smaller than the input buffer size
    if(cursorOffset < consoleInputBuffer->size()){
        cursorOffset++;
    }
}

void TextInput::moveCursorRight(){
    // Move cursor right if it's greater than 0
    if(cursorOffset > 0){
        cursorOffset--;
    }
}

void TextInput::append(char const* c) const {
    std::string const input(c);
    std::string newBuffer;
    if(cursorOffset > 0){
        // Insert character at cursor position
        std::string const string_before_cursor = consoleInputBuffer->substr(0, consoleInputBuffer->size() - cursorOffset);
        std::string const string_after_cursor = consoleInputBuffer->substr(consoleInputBuffer->size() - cursorOffset);
        newBuffer = string_before_cursor + input + string_after_cursor;
    }
    else{
        // Append character to the end
        newBuffer = *consoleInputBuffer + input;
    }
    *consoleInputBuffer = newBuffer;
}

} // namespace Nebulite::Utility