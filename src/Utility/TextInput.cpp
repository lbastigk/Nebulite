#include "Utility/TextInput.hpp"

namespace Nebulite::Utility {

TextInput::TextInput(){
    consoleInputBuffer = &commandIndexZeroBuffer;
}

void TextInput::submit(Nebulite::Core::GlobalSpace* globalspace, submitType type){
    if (!consoleInputBuffer->empty()) {
        std::string input = *consoleInputBuffer;

        // History and output
        commandHistory.emplace_back(input);
        consoleOutput.emplace_back("> " + input);

        // Add to queue
        if(type == submitType::EXECUTE){
            globalspace->getTaskQueue()->emplace_back(input);
            if(selectedCommandIndex != 0){
                // If we were browsing history, reset to latest input
                selectedCommandIndex = 0;
                consoleInputBuffer = &commandIndexZeroBuffer;
            }
        }

        // Like in typical consoles, we clear the output
        commandIndexZeroBuffer.clear();
    }
}

void TextInput::backspace(){
    if (!consoleInputBuffer->empty()) {
        consoleInputBuffer->pop_back();
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

} // namespace Nebulite::Utility