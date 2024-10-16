#include "Platform.h"

#include <iostream>
#include <vector>
#include <functional>
#include <string>
#include <sstream>

class OptionsMenu {
public:
    using FunctionPtr = std::function<void()>;  // Define the function pointer type

    OptionsMenu() : currentOption(0) {}

    void setTextBefore(std::string text);
    void setTextAfter(std::string text);

    // Update, get input and manipulate
    // return is 0 if no option was chosen
    // return is positive if option n was chosen and the corresponding function was executed
    // return is -1 if exit was chosen
    int update(bool render = true);

    // Renders all available options, where the current option has an arrow next to it
    void render();

    std::string createText();

    // Attach a function to the menu
    void attachFunction(FunctionPtr func, const std::string& text);

    // Clear all function entries
    void clearEntries();

private:
    std::string textBefore = "";
    std::string textAfter = "";

    std::vector<std::pair<FunctionPtr, std::string>> functions;  // Vector of function-text pairs
    bool showExitEntry = true;  // Whether to show the exit entry
    int currentOption;           // Current option index
};