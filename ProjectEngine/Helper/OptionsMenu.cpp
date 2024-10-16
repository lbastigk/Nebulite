#include "OptionsMenu.h"



void OptionsMenu::setTextBefore(std::string text){
    textBefore = text;
}

void OptionsMenu::setTextAfter(std::string text){
    textAfter = text;
}

int OptionsMenu::update(bool renderScreen) {
    int val = -1;
    int input = Platform::getCharacter();
    switch(input){
        case Platform::KeyPress::arrowUp:
            currentOption = (currentOption - 1 + functions.size() + 1) % (functions.size() + 1);
            break;
        case Platform::KeyPress::arrowDown:
            currentOption = (currentOption + 1 + functions.size() + 1) % (functions.size() + 1);
            break;
        case Platform::KeyPress::Enter:
            val = currentOption;
            functions.at(currentOption).first();
            break;
    }
    if(renderScreen){
        render();
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
            ss << "->\t" << functions[i].second << std::endl;  // Arrow for the current option
        }
        else{
            ss << "  \t" << functions[i].second << std::endl;  // Display option text
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
void OptionsMenu::attachFunction(FunctionPtr func, const std::string& text) {
    functions.push_back({func, text});  // Add the function and its text to the vector
    currentOption = 0;  // Reset current option to the first one
}

// Clear all function entries
void OptionsMenu::clearEntries() {
    functions.clear();
    currentOption = 0;  // Reset current option
}