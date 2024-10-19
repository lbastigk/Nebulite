#include "OptionsMenu.h"

void OptionsMenu::setTextBefore(std::string text){
    textBefore = text;
}

void OptionsMenu::setTextAfter(std::string text){
    textAfter = text;
}

int OptionsMenu::update(bool renderScreen) {
    int size = functions.size();
    int val = 0;

    // Depending on type
    if(menuType == typeScrollingMenu){
        // Change happens only if keyboard has input
        if(Platform::hasKeyBoardInput()){
            int input = Platform::getCharacter();
            switch(input){
                case Platform::KeyPress::arrowUp:
                case Platform::KeyPress::W:
                    currentOption = (currentOption - 1 + size + 1) % (size + 1);
                    break;
                case Platform::KeyPress::arrowDown:
                case Platform::KeyPress::S:
                    currentOption = (currentOption + 1 + size + 1) % (size + 1);
                    break;
                case Platform::KeyPress::Enter:
                    val = currentOption;
                    if(currentOption < size){
                        std::get<0>(functions.at(currentOption))();
                    }
                    else{
                        val = statusExit;
                    }
                    break;
                default:
                    break;
            }
            if(renderScreen){
                render();
            }
        }
    }
    else if(menuType == typeConsole){
        console.refresh();
        if(console.hasInput()){
            std::string arg = console.getInput();

            // Check all functions
            // linear search should be enough, for larger menues an std::map might be preferred
            for(auto& function : functions){
                
                val++;
                if(std::get<1>(function) == arg){
                    // execute function
                    std::get<0>(function)();
                    break;
                }
            }
            if(arg == std::string("help")){
                std::stringstream list;
                list << std::endl;

                // Step 1: Find the largest function name length
                size_t maxFunctionNameLength = 0;
                for (auto& function : functions) {
                    size_t functionNameLength = std::get<1>(function).size();
                    if (functionNameLength > maxFunctionNameLength) {
                        maxFunctionNameLength = functionNameLength;
                    }
                }

                // Step 2: List all functions and their descriptions with padding
                for (auto& function : functions) {
                    std::string functionName = std::get<1>(function);

                    // Add space padding to make all function names the same size
                    list << functionName;
                    list << std::string(maxFunctionNameLength - functionName.size(), ' '); // Pad with spaces

                    list << " - ";
                    list << std::get<2>(function); // Function description
                    list << std::endl;
                }

                // Step 3: output
                std::cout << list.str();

                val = 0;
            }
            else if(arg == std::string("exit")){
                val = -1;
            }
            else{
                std::cout <<"Command not found" << "\n";
                val = 0;
            }
        }
    }
    return val;
}

// Renders all available options, where the current option has an arrow next to it
void OptionsMenu::render() {
    Platform::clearScreen();
    std::cout << createText();
}

std::string OptionsMenu::createText() {
    std::stringstream ss;
    ss << textBefore << std::endl;
    for (size_t i = 0; i < functions.size(); ++i) {
        if (currentOption == (int)i) {

            ss << "->\t" << std::get<1>(functions[i]) << std::endl;  // Arrow for the current option
        }
        else{
            ss << "  \t" << std::get<1>(functions[i]) << std::endl;  // Display option text
        }
        
    }
    if (showExitEntry) {
        if (currentOption == (int)functions.size()) {
            ss << "->\t" << "Exit";
        }
        else{
            ss << "  \t" << "Exit";
        }
    }
    ss << textAfter << std::endl;
    return ss.str();
}

// Attach a function to the menu
void OptionsMenu::attachFunction(FunctionPtr func, const std::string& text, const std::string& description) {
    functions.push_back({func, text, description});  // Add the function and its text to the vector
    currentOption = 0;  // Reset current option to the first one
}

// Clear all function entries
void OptionsMenu::clearEntries() {
    functions.clear();
    currentOption = 0;  // Reset current option
}

void OptionsMenu::setOption(int opt){
    currentOption = opt;
}

void OptionsMenu::changeType(int type){
    menuType = type;
    Platform::clearScreen();
}